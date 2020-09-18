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
    Request *r = request_create("PUT",topic,body);  // build request with the body
    queue_push(mq->outgoing, r);                       // push request to outgoing    
}

/**
 * Retrieve one message (by taking Request from incoming queue).
 * @param   mq      Message Queue structure.
 * @return  Newly allocated message body (must be freed).
 */
char * mq_retrieve(MessageQueue *mq) {
    Request *r = request_create("GET", mq->incoming->head->uri, NULL);
    
    mq->incoming->head = mq->incoming->head->next;
    return r->body;
}

/**
 * Subscribe to specified topic.
 * @param   mq      Message Queue structure.
 * @param   topic   Topic string to subscribe to.
 **/
void mq_subscribe(MessageQueue *mq, const char *topic) {
    const char *uri = sprintf("/subscription/%s/%s", mq->name, topic); // create uri
    Request *r = request_create("PUT", uri, NULL);
    queue_push(mq->outgoing, r);
}

/**
 * Unubscribe to specified topic.
 * @param   mq      Message Queue structure.
 * @param   topic   Topic string to unsubscribe from.
 **/
void mq_unsubscribe(MessageQueue *mq, const char *topic) {
    const char *uri = sprintf("/subscription/%s/%s", mq->name, topic);
    Request *r = request_create("DELETE", uri, NULL);
    queue_push(mq->outgoing, r);
}

/**
 * Start running the background threads:
 *  1. First thread should continuously send requests from outgoing queue.
 *  2. Second thread should continuously receive reqeusts to incoming queue.
 * @param   mq      Message Queue structure.
 */

// QUESTIONS
// Where do the things we push / pull go?
// Where do we call this code
// Should we do an infinite loop? Or no
void mq_start(MessageQueue *mq) {
}

/**
 * Stop the message queue client by setting shutdown attribute and sending
 * sentinel messages
 * @param   mq      Message Queue structure.
 */

// QUESTIONS
// What are sentinel messages?
// What do we do with them in other functions / programs?
void mq_stop(MessageQueue *mq) {
    // TODO call mq_subscribe on a SENTINEL topic
    // just call it sentinel and all will be ok
    // Also change mq->shutdown
}

/**
 * Returns whether or not the message queue should be shutdown.
 * @param   mq      Message Queue structure.
 */
bool mq_shutdown(MessageQueue *mq) {
    return (mq->shutdown);
}

/* Internal Functions */

/**
 * Pusher thread takes messages from outgoing queue and sends them to server.
 * @param   arg     Message Queue structure.
 **/

// QUESTIONS
// How do we read the response?
void * mq_pusher(void *arg) {
    MessageQueue *mq = (MessageQueue *) arg;              // set arg
    char buffer[BUFSIZ];                                  // define buffer
    while (!mq_shutdown(mq)){
        Request *r = queue_pop(mq->outgoing);             // pop request
        request_delete(r);                                // free request
        FILE *fs = socket_connect(mq->host, mq->port);    // connect to server
        request_write(r, fs);                             // write request to server

        while(fgets(buffer, BUFSIZ, fs))                  // read response
            continue;
    }

    return NULL;
}

/**
 * Puller thread requests new messages from server and then puts them in
 * incoming queue.
 * @param   arg     Message Queue structure.
 **/

// QUESTIONS
// Do we have to do mq_retrieve or do the Request *new thing?
// How do we read the response?
// What do we do if there is no request body?
void * mq_puller(void *arg) {
    MessageQueue *mq = (MessageQueue *)arg;
    char buffer[BUFSIZ];                                      // set up buffer
    int length;                                               // set up length
    while (!mq_shutdown(mq)){
        Request *r = request_create("GET", mq->name, NULL);   // make empty request
        FILE *fs = socket_connect(mq->host, mq->port);        // connect to server
        request_write(r, fs);                                 // write request
        char *response = fgets(buffer, BUFSIZ, fs);           // gets the response
        char *rCode = strstr(response, "200 OK");             // checks for response
        if (streq(rCode, "200 OK")){                          // checks for 200 OK
            while (fgets(buffer, BUFSIZ, response) && !streq(buffer, "\r\n"))
                sscanf(buffer, "Content-Length:%d", &length);
        
            r->body = malloc(length * sizeof(char) + 1);
            fread(r->body, length, 1, socket);
        }

        if (length != 0)
            queue_push(mq->incoming, r);
    }

    return NULL;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
