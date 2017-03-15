/**
 * Luscious Locks Lab 
 * CS 241 - Fall 2016
 */
//#include "queue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Struct representing a node in a queue_t
 */
typedef struct queue_node_t {

  struct queue_node_t *next;
  void *data;
} queue_node_t;

/**
 * Struct representing a queue
 */
typedef struct queue_t 
{
  queue_node_t *head, *tail;
  int size;
  int maxSize;
  pthread_cond_t cv;
  pthread_mutex_t m;
} queue_t;

/**
 *  Given data, place it on the queue.  Can be called by multiple threads.
 *  Blocks if the queue is full.
 */
void queue_push(queue_t *queue, void *data)
{
	pthread_mutex_lock(&queue->m);

        while(queue->size == queue->maxSize && queue->maxSize >= 0)
        {
                pthread_cond_wait(&queue->cv, &queue->m);
        }


	queue->size++;
	queue_node_t * node = malloc(sizeof(queue_node_t));
	node->next = NULL;
	if(queue->size == 1)
	{
		queue->head = node;
	}
	else
	{
		queue->tail->next = node;
	}
	queue->tail = node;
	node->data = data;

	pthread_cond_broadcast(&queue->cv);
	
	pthread_mutex_unlock(&queue->m);

//	pthread_cond_broadcast(&queue->cv);
}

/**
 *  Retrieve the data from the front of the queue.  Can be called by multiple
 * threads.
 *  Blocks if the queue is empty.
 */
void *queue_pull(queue_t *queue) 
{
	pthread_mutex_lock(&queue->m);

        while(queue->size == 0)
        {
                pthread_cond_wait(&queue->cv, &queue->m);
        }


	queue_node_t * oldHead = queue->head;
	queue->head = queue->head->next;
	queue->size--;
	void * data = oldHead->data;
	free(oldHead);

	pthread_cond_broadcast(&queue->cv);
	pthread_mutex_unlock(&queue->m);
	return data;
}

/**
 *  Allocates heap memory for a queue_t and initializes it.
 *  Returns a pointer to this allocated space.
 */
queue_t *queue_create(int maxSize) 
{
	queue_t * output = malloc(sizeof(queue_t));
	output->head = NULL;
	output->tail = NULL;
	output->size = 0;
	output->maxSize = maxSize;
	pthread_mutex_init(&output->m, NULL);
	pthread_cond_init(&output->cv, NULL);
	return output;
  /* Your code here */
  return NULL;
}

/**
 *  Destroys the queue, freeing any remaining nodes in it.
 */
void queue_destroy(queue_t *queue) 
{
	while(queue->head)
	{
		queue_node_t * h = queue->head->next;
		free(queue->head);
		queue->head = h;
	}

	pthread_mutex_destroy(&queue->m);
	pthread_cond_destroy(&queue->cv);
	free(queue);
}

int size(queue_t * queue)
{
	return queue->size;
}
