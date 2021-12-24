#include "queue.h"

bool create_node(void *line, list_node_t **node){
    *node = (list_node_t*)malloc(sizeof(list_node_t));
    if(*node != NULL){
        *(*node) = INIT_NODE;    
        (*node)->data = line;
        return true;
    }else{
        return false;
    }
}

bool enqueue(queue_t *q, list_node_t *node){
    if(q){
        if(!q->head){
            q->head = node;
            q->tail = node;
            ++q->size;
        }else{
            q->tail->next = node; 
            node->prev = q->tail;

            q->tail = node;
            ++q->size;
        }
    }else return false;
   
    return true;
}

bool dequeue(queue_t *q, list_node_t **out){
    if(out && q && q->size >= 1){
        list_node_t *node = q->head;
        q->head = q->head->next;
        node->next = NULL;
        *out = node;
        --q->size;
        if(q->size == 0){
            q->tail = NULL;
            q->head = NULL;
        } else{
            q->head->prev = NULL;
        }    
    }else{
        if(out) *out = NULL;
        return false;
    }
    return true;
}

bool free_node(list_node_t **node){
    if(node && *node){
        if((*node)->data)
            free((*node)->data);

        free(*node);
        return true;
    }else{
        return false;
    }
}
