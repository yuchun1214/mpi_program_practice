#ifndef __QUEUE_H__
#define __QUEUE_H__
#include <stdlib.h>
#include <stdbool.h>

typedef struct list_node_t list_node_t;
struct list_node_t{
   void *data;
   list_node_t *next;
   list_node_t *prev;
};

typedef struct{
    int size;
    list_node_t *head;
    list_node_t *tail;
}queue_t;

#define INIT_QUEUE (queue_t){ .size = 0, .head = NULL, .tail = NULL}

#define INIT_NODE (list_node_t){ .data  = NULL, .next = NULL, .prev = NULL}

bool create_node(void *line, list_node_t **node);
bool enqueue(queue_t *q, list_node_t *newnode);
bool dequeue(queue_t *q, list_node_t **out);
bool free_node(list_node_t **node);

#endif
