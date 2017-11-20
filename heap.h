#ifndef HEAP_H
#define HEAP_H

typedef struct heap HEAP;

//Returns a new HEAP of length "length."
HEAP * heap_new(int length, int (*compare)(const void *, const void *));

//Frees a dynamically allocated heap.
void heap_free(HEAP *h);

//Shows us what the top item on the heap is without actually removing it.
void * peek(HEAP *h);

//Adds data to the heap h, percolating it up to the correct level.
//Returns 0 if data was successfully added to the heap.
//Returns -1 if the heap was full and could not be enlarged (data  was not added in this case).
int heap_add(HEAP *h, void *data);

//Removes data from the heap h, maintaining heapiness.
//Returns NULL if there are no more items in the heap.
void * heap_remove(HEAP *h);

#endif