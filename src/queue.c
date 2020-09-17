/* queue.c: Concurrent Queue of Requests */

#include "mq/queue.h"

/**
 * Create queue structure.
 * @return  Newly allocated queue structure.
 */
Queue * queue_create() {
    Queue *q = calloc(1, sizeof(Queue));
    if (q){
        //q->head = NULL;
        //q->tail = NULL;
        //q->size = 0;

        mutex_init(&q->lock, NULL);
        cond_init(&q->block, NULL);
        return q;
    } 

    return NULL;
}

/**
 * Delete queue structure.
 * @param   q       Queue structure.
 */
void queue_delete(Queue *q) {
    mutex_lock(&q->lock);
    Request *temp;

    for (Request *r = q->head; r != NULL; r = temp){
        temp = r->next;
        request_delete(r);
    }
    
    mutex_unlock(&q->lock);
    free(q);
}

/**
 * Push request to the back of queue.
 * @param   q       Queue structure.
 * @param   r       Request structure.
 */
void queue_push(Queue *q, Request *r) {
    mutex_lock(&q->lock);
    
    // Check if youre pushing the first element
    if (q->size == 0){
        q->head = r;
        q->tail = r;
    }

    // If its not the first element...
    else{
        q->tail->next = r;
        q->tail = r;
    }

    // Standard for both cases
    r->next = NULL;
    q->size++;
    
    // Concurrency stuff
    mutex_unlock(&q->lock);
}

/**
 * Pop request to the front of queue (block until there is something to return).
 * @param   q       Queue structure.
 * @return  Request structure.
 */

// POP CAN BLOCK... check if the queue is empty
Request * queue_pop(Queue *q) {
    // Block Pop :)
    mutex_lock(&q->lock);
    if (q->size == 0)
        cond_wait(&q->block, &q->lock);

    // Update Queue data
    Request *pop = q->head;
    q->head = q->head->next;
    q->size--;

    // Unlock the lock yo
    
    mutex_unlock(&q->lock);
    return pop;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
