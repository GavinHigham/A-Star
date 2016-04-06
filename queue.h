#ifndef QUEUE_H
#define QUEUE_H

typedef struct node *PNODE; //Forward declaration of PNODE.
typedef struct queue
{
	PNODE *_queue; //Internal array for our circular buffer.
	int length;    //Current size of the array.
	int _length;   //Size of the internal array, including the empty space.
	int _first;    //Pointer to the first process in our queue.
} QUEUE, *QUEUEP;

//Returns a new QUEUE of length "length."
QUEUEP new_queue(int length);

//Frees a dynamically allocated queue.
void free_queue(QUEUEP q);

//Shows us what the next item to be dequeued is without actually dequeueing it.
PNODE peek(QUEUEP q);

//Enqueues a PNODE n into the queue q, percolating it up to the correct level, based on the provided comparison function.
//Returns 0 if p was successfully added to the queue.
//Returns -1 if the queue was full and could not be enlarged. (n was not enqueued in this case)
int heap_enqueue(QUEUEP q, PNODE n, int (*compare)(const void *, const void *));

//Dequeues a PNODE from the queue q, maintaining heapiness.
//Returns a PNODE, or NULL if there are no more items in the queue.
PNODE heap_dequeue(QUEUEP q, int (*compare)(const void *, const void *));

#endif