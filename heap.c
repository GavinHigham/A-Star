#include "heap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {GROWTH_FACTOR = 2};

struct heap {
	int len;    //Count of array elements.
	int _len;   //Count of internal array, including empty space.
	int (*compare)(const void *, const void *); //Comparison function.
	void **_arr; //Internal array.
};

HEAP * heap_new(int len, int (*compare)(const void *, const void *))
{
	HEAP *h = (HEAP *)malloc(sizeof(HEAP));
	if (h != NULL) {
		h->_arr = malloc(len * sizeof(void *));
		h->_len = len;
		h->compare = compare;
		h->len = 0;
	}
	return h;
}

void heap_free(HEAP *h)
{
	free(h->_arr);
	free(h);
}

int insert(HEAP *h, void *n)
{
	if (h->len >= h->_len) { //The heap is already full, reallocate storage.
		void **new_storage = (void **)realloc(h->_arr, sizeof(void *) * h->_len * GROWTH_FACTOR);
		if (new_storage != NULL) {
			h->_arr = new_storage;
			h->_len *= GROWTH_FACTOR;
		} else {
			return -1; //Could not grow the internal array.
		}
	}
	h->_arr[h->len++] = n;
	return 0;
}

void * peek(HEAP *h)
{
	if (h->len < 1)
		return NULL;
	return h->_arr[0];
}

static int parent_index(int i)
{
	return (i - 1) / 2;
}

//Left Child Index
static int lci(int i)
{
	return i*2 + 1;
}

//Right Child Index
static int rci(int i)
{
	return i*2 + 2;
}

static void swap(void **p1, void **p2)
{
	void *tmp = *p1;
	*p1 = *p2;
	*p2 = tmp;
}

static void sift(HEAP *h, int i, int (*compare)(const void *, const void *))
{
	if (i == 0)
		return;
	int pi = parent_index(i);
	if (compare(h->_arr[i], h->_arr[pi]) > 0) {
		swap(&h->_arr[i], &h->_arr[pi]);
		sift(h, pi, compare);
	}
}

static void sift_down(HEAP *h, int i, int (*compare)(const void *, const void *))
{
	int l = lci(i);
	int r = rci(i);
	int mi = i;

	if (l <= h->len && compare(h->_arr[l], h->_arr[mi]) > 0)
		mi = l; //If left child is larger than i, swap with that one.
	if (r <= h->len && compare(h->_arr[r], h->_arr[mi]) > 0)
		mi = r; //If right child is larger than i or left child, swap with that one.
	//Essentially, if either child is larger, swap with the largest
	
	if (mi != i) {
		swap(&h->_arr[i], &h->_arr[mi]);
		sift_down(h, mi, compare);
	}
}

int heap_add(HEAP *h, void *data)
{
	int result = insert(h, data);
	if (result == 0)
		sift(h, h->len - 1, h->compare);
	return result;
}

void * heap_remove(HEAP *h)
{
	if (h->len < 1)
		return NULL;
	void *root = peek(h); //Grab the node at the root.
	h->_arr[0] = h->_arr[--h->len]; //Move the last node to the root position.
	sift_down(h, 0, h->compare); //Sift that node down until it sits at the right position in the heap.
	return root;
}
