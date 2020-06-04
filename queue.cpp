#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "queue.h"

struct q_node {
	int key;
	void* data;
	struct q_node* next;
};

struct queue {
	struct q_node* first_item;
	struct q_node* last_item;
	int length;
};

//creates new node for the queue
struct q_node* new_node(void* data) {
	struct q_node* node = (struct q_node*)malloc(sizeof(struct q_node));
	if (node == NULL) {
		printf("malloc for q_node failed\n");
	}
	node->next = NULL;
	node->data = data;
	return node;
}

//creates new queue of q_nodes
queue_t queue_create(void) {
	struct queue* myqueue = (struct queue*)malloc(sizeof(struct queue));
	if (myqueue == NULL) {
		printf("malloc for queue failed\n");
	}
	myqueue->length = 0;
	myqueue->first_item = NULL;
	myqueue->last_item = NULL;
	return myqueue;
}

//return true if the queue is empty, false otherwise
static bool is_empty(queue_t queue) {
	if (queue->first_item == NULL && queue->last_item == NULL) {
		return true;
	}
	return false;
}

//first item in queue points to second item from front with next.  Last item points to NULL until another item enqueued and change last item pointer to that item
int queue_enqueue(queue_t queue, void *data) {
	if (queue == NULL || data == NULL) {
		return -1;
	}
	struct q_node* node = new_node(data);

	if (is_empty(queue)) {
		queue->first_item = node;
		queue->last_item = node;
		queue->length += 1;
		return 0;
	}
	queue->last_item->next = node;
	queue->last_item = node;
	queue->length += 1;
	return 0;
}

int queue_dequeue(queue_t queue, void **data) {
	if (queue == NULL || data == NULL || is_empty(queue)) {
        printf("error\n");
		return -1;
	}
	struct q_node* temp = queue->first_item;
	//*data = queue->first_item->data;
    //printf("queue length = %d\n", queue->length);
    if(queue->length > 1){
        queue->first_item = queue->first_item->next;
    }else{
        //printf("make first item NULL\n");
        queue->first_item = NULL;
    }
	free(temp);
	queue->length -= 1;
	return 0;
}

int queue_length(queue_t queue) {
	return queue->length;
}

