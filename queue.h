#include <stdio.h>
#include <stdlib.h>

/* For this particular project, we only need a queue containing integers */

/* Define a node in the queue */
struct queue_node_s {
	int data;
	struct queue_node_s * next_node;
};

struct queue_int_s {
	struct queue_node_s *head;
	struct queue_node_s *tail;
};

/* queue_int_s APIs */
struct queue_int_s * create_queue();
int enqueue(struct queue_int_s * queue, int data);
int dequeue(struct queue_int_s * queue, int * data);
void delete_queue(struct queue_int_s ** queuePointer);
int is_empty_queue(struct queue_int_s * queue);

