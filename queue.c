#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "queue.h"
#define GROWTH_FACTOR 2

QUEUEP new_queue(int length)
{
	QUEUEP newQueue = (QUEUEP)malloc(sizeof(QUEUE));
	if (newQueue != NULL) {
		newQueue->_queue = (PNODE *)malloc(sizeof(PNODE) * length);
		newQueue->_length = length;
		newQueue->_first = 0;
		newQueue->length = 0;
	}
	return newQueue;
}

//Frees a dynamically allocated queue.
void free_queue(QUEUEP q)
{
	free(q->_queue);
	free(q);
}

int enqueue(QUEUEP q, PNODE n)
{
	if (q->length >= q->_length) { //The queue is already full, reallocate storage.
		PNODE *new_storage = (PNODE *)realloc(q->_queue, sizeof(PNODE) * q->_length * GROWTH_FACTOR);
		if (new_storage != NULL) {
			q->_queue = new_storage;
			q->_length = 2 * q->_length;
		}
		else
			return -1; //Could not grow the internal array.
	}
	q->_queue[(q->_first+q->length) % q->_length] = n; //The slot "length" distance from _first is now filled.
	q->length++;
	return 0;
}

PNODE peek(QUEUEP q)
{
	if (q->length < 1)
		return NULL;
	return q->_queue[q->_first];
}

static int parentIndex(QUEUEP q, int i) {
	return (((i-q->_first)%q->_length - 1) / 2 + q->_first) % q->_length;
}

static int leftChildIndex(QUEUEP q, int i)
{
	return ((((i-q->_first)%q->_length * 2) + 1) + q->_first) % q->_length;
}

static int rightChildIndex(QUEUEP q, int i)
{
	return ((((i-q->_first)%q->_length * 2) + 2) + q->_first) % q->_length;
}

static void swap(PNODE *p1, PNODE *p2) {
	PNODE tmp = *p1;
	*p1 = *p2;
	*p2 = tmp;
}

static void sift(QUEUEP q, int i, int (*compare)(const void *, const void *))
{
	int pi = parentIndex(q, i);
	if (compare(&q->_queue[i], &q->_queue[pi]) < 0) {
		swap(&q->_queue[i], &q->_queue[pi]);
		sift(q, pi, compare);
	}
}

static void sift_down(QUEUEP q, int i, int (*compare)(const void *, const void *))
{
	int lci = leftChildIndex(q, i);
	int rci = rightChildIndex(q, i);
	int mi;
	if ((((i-q->_first)%q->_length * 2) + 2) >= q->length) {
		if ((((i-q->_first)%q->_length * 2) + 1) >= q->length)
			return;
		else
			mi = lci;
	} else {
		if (compare(&q->_queue[lci], &q->_queue[rci]) < 0)
			mi = lci;
		else
			mi = rci;
	}
	if (compare(&q->_queue[i], &q->_queue[mi]) < 0) {
		swap(&q->_queue[i], &q->_queue[mi]);
		sift_down(q, mi, compare);
	}
}

int heap_enqueue(QUEUEP q, PNODE n, int (*compare)(const void *, const void *))
{
	int result = enqueue(q, n);
	if (!result)
		sift(q, (q->_first+q->length - 1) % q->_length, compare);
	return result;
}

PNODE heap_dequeue(QUEUEP q, int (*compare)(const void *, const void *))
{
	if (q->length < 1)
		return NULL;
	PNODE root = peek(q);
	q->_queue[q->_first] = q->_queue[(q->_first+q->length - 1)%q->_length];
	q->length--;
	sift_down(q, q->_first, compare);
	return root;
}