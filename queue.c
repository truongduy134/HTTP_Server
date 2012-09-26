#include "queue.h"

/* create_queue()
 *
 *	Creates an empty queue with head = tail = null
 * 
 *	Returns a pointer to the queue structure created. 
 */
struct queue_int_s * create_queue()
{
	struct queue_int_s *newQueue = malloc(sizeof(struct queue_int_s));

	if(newQueue != NULL)
	{	
		newQueue->head = NULL;
		newQueue->tail = NULL;
	}

	return newQueue; 
}

/* enqueue(...)
 *
 *	Adds the new integer to the end of the queue.
 *	
 *	Returns 1 if enqueuing is successful. Returns 0 otherwise. 
 */
int enqueue(struct queue_int_s * queue, int data)
{
	struct queue_node_s * newNode;
	if(queue->head == NULL)
	{
		/* If the queue is empty */
		queue->head = malloc(sizeof(struct queue_node_s));
		if(queue->head == NULL)
			return 0;
		queue->head->data = data;
		queue->head->next_node = NULL;
		queue->tail = queue->head;
	}
	else
	{
		/* Create a new node */
		newNode = malloc(sizeof(struct queue_node_s));
		newNode->data = data;
		newNode->next_node = NULL;

		/* Attach new node to the end of the queue */
		queue->tail->next_node = newNode;

		/* Update the new tail */
		queue->tail = newNode;
	}
	return 1;
}

/* dequeue(...)
 *	Removes the head of the queue. The value of the head is stored
 *		in the second parameter of the function.
 *
 *	Returns 0 if user attempts to dequeue from an empty queue. 
 *	Returns 1 otherwise.
 */
int dequeue(struct queue_int_s * queue, int * data)
{
	if(is_empty_queue(queue))
		return 0;

	struct queue_node_s *nextHead = queue->head->next_node;

	*data = queue->head->data;

	/* Deallocate memeory of the head */
	struct queue_node_s *oldHead = queue->head;
	queue->head = nextHead;
	free(oldHead);

	if(queue->head == NULL)
		/* The queue now is empty */
		queue->tail = NULL;
	return 1;
}

/* delete_queue(...)
 *
 *	Deallocate all the memory of the queue structure.
 */ 
void delete_queue(struct queue_int_s ** queuePointer)
{
	if(queuePointer == NULL)
		return;
	struct queue_node_s * curNode = (*queuePointer)->head;
	struct queue_node_s * temp;

	while(curNode != NULL)
	{
		temp = curNode->next_node;
		free(curNode);
		curNode = temp;		
	}
	free(*queuePointer);
	*queuePointer = NULL;
}

/* is_empty_queue(...)
 *
 *	Returns 1 if the queue is empty. Returns 0 otherwise.
 */
int is_empty_queue(struct queue_int_s * queue)
{
	return (queue->head == NULL) ? 1 : 0;
}
