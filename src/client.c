/* client.c: Message Queue Client */

#include "mq/client.h"
#include "mq/logging.h"
#include "mq/socket.h"
#include "mq/string.h"

/* Internal Constants */

#define SENTINEL "SHUTDOWN"

/* Internal Prototypes */

void * mq_pusher(void *);
void * mq_puller(void *);

/* External Functions */

/**
 * Create Message Queue withs specified name, host, and port.
 * @param   name        Name of client's queue.
 * @param   host        Address of server.
 * @param   port        Port of server.
 * @return  Newly allocated Message Queue structure.
 */
MessageQueue * mq_create(const char *name, const char *host, const char *port) {
    MessageQueue *mq = calloc(1, sizeof(MessageQueue));
    if (mq) {
        strcpy(mq->name, (char *) name);
        strcpy(mq->host, (char *) host);
        strcpy(mq->port, (char *) port);
        mq->outgoing = queue_create();
        mq->incoming = queue_create();
        mq->shutdown = false;

        mutex_init(&mq->lock, NULL);

        return mq;
    }
    return NULL;
}

/**
 * Delete Message Queue structure (and internal resources).
 * @param   mq      Message Queue structure.
 */
void mq_delete(MessageQueue *mq) {
    if (mq) {
        free(mq->name);
        free(mq->host);
        free(mq->port);
        queue_delete(mq->outgoing);
        queue_delete(mq->incoming);
        free(mq);
    }
}

/**
 * Publish one message to topic (by placing new Request in outgoing queue).
 * @param   mq      Message Queue structure.
 * @param   topic   Topic to publish to.
 * @param   body    Message body to publish.
 */
void mq_publish(MessageQueue *mq, const char *topic, const char *body) {
    Request *r = request_create("PUT", topic, body);   // build request with the body
    queue_push(mq->outgoing, r);                       // push request to outgoing    
}

/**
 * Retrieve one message (by taking Request from incoming queue).
 * @param   mq      Message Queue structure.
 * @return  Newly allocated message body (must be freed).
 */

// pop stack
// check if r->body is null and contains the sentinel value
// If it does then just return null
char * mq_retrieve(MessageQueue *mq) {
    Request *r = queue_pop(mq->incoming);
    
    if (r->body != NULL){
        char *body = strdup(r->body);
        return body;
    }
    else
        return NULL;
    //test
}

/**
 * Subscribe to specified topic.
 * @param   mq      Message Queue structure.
 * @param   topic   Topic string to subscribe to.
 **/
void mq_subscribe(MessageQueue *mq, const char *topic) {
    char uri[BUFSIZ];
    sprintf(uri, "/subscription/%s/%s", mq->name, topic); // create uri
    Request *r = request_create("PUT", uri, NULL);
    queue_push(mq->outgoing, r);
}

/**
 * Unubscribe to specified topic.
 * @param   mq      Message Queue structure.
 * @param   topic   Topic string to unsubscribe from.
 **/
void mq_unsubscribe(MessageQueue *mq, const char *topic) {
    char uri[BUFSIZ];
    sprintf(uri, "/subscription/%s/%s", mq->name, topic);
    Request *r = request_create("DELETE", uri, NULL);
    queue_push(mq->outgoing, r);
}

/**
 * Start running the background threads:
 *  1. First thread should continuously send requests from outgoing queue.
 *  2. Second thread should continuously receive reqeusts to incoming queue.
 * @param   mq      Message Queue structure.
 */

void mq_start(MessageQueue *mq) {
    // Initialize and start threads 
    thread_create(&mq->pusher, NULL, mq_pusher, mq);
    thread_create(&mq->puller, NULL, mq_puller, mq);    

    // Subscribe to sentinel
    mq_subscribe(mq, SENTINEL);
}

/**
 * Stop the message queue client by setting shutdown attribute and sending
 * sentinel messages
 * @param   mq      Message Queue structure.
 */

void mq_stop(MessageQueue *mq) {
    mq_publish(mq, SENTINEL, SENTINEL);

    // Lock and change the variable
    mutex_lock(&mq->lock);
    mq->shutdown = true;
    mutex_unlock(&mq->lock);

    // TODO join threads pusher and puller
    thread_join(mq->pusher, NULL);
    thread_join(mq->puller, NULL);
}

/**
 * Returns whether or not the message queue should be shutdown.
 * @param   mq      Message Queue structure.
 */
bool mq_shutdown(MessageQueue *mq) {
    mutex_lock(&mq->lock);
    bool status = mq->shutdown;
    mutex_unlock(&mq->lock);
    return (status);
}

/* Internal Functions */

/**
 * Pusher thread takes messages from outgoing queue and sends them to server.
 * @param   arg     Message Queue structure.
 **/

void * mq_pusher(void *arg) {
    MessageQueue *mq = (MessageQueue *) arg;              // set arg
    char buffer[BUFSIZ];                                  // define buffer
    while (!mq_shutdown(mq)){
        Request *r = queue_pop(mq->outgoing);             // pop request
        FILE *fs = socket_connect(mq->host, mq->port);    // connect to server
        request_write(r, fs);                             // write request to server

        while(fgets(buffer, BUFSIZ, fs))                  // read response
            buffer[strlen(buffer)-1] = '\0';                // set last char to null

        
        request_delete(r);
        fclose(fs);
    }

    return NULL;
}

/**
 * Puller thread requests new messages from server and then puts them in
 * incoming queue.
 * @param   arg     Message Queue structure.
 **/

void * mq_puller(void *arg) {
    MessageQueue *mq = (MessageQueue *)arg;
    char buffer[BUFSIZ];                                      // set up buffer
    size_t length;                                               // set up length
    FILE *fs;
    char uri[BUFSIZ];
    while (!mq_shutdown(mq)){
        sprintf(uri, "/queue/%s", mq->name);
        Request *r = request_create("GET", uri, NULL);            // make empty request
        if (r){
            fs = socket_connect(mq->host, mq->port);                 // connect to server
            if (fs){
                request_write(r, fs);                                          // write request
                char *response = fgets(buffer, BUFSIZ, fs);                    // gets the response
                if (strstr(response, "200 OK")){                                   // checks for 200 OK
                    while (fgets(buffer, BUFSIZ, fs) && !streq(buffer, "\r\n"))
                        sscanf(buffer, "Content-Length:%ld", &length);
                    if (length > 0)
                        r->body = calloc(length + 1, sizeof(char));
                    if (r->body)
                        fread(r->body, 1, length, fs);
                }
                else
                    continue;
            }
            else
                continue;
        }
        else
            continue;
        
        // if r->body != NULL
        if (length != 0)
            queue_push(mq->incoming, r);
        else
            request_delete(r);
        fclose(fs);
    }

    return NULL;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
