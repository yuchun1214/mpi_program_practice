#include <assert.h>
#include <stdio.h>
#include "queue.h"

#define pass "✔"
#define failed "✗"

#define _str(x) #x
#define str(x) _str(x)

bool testing_init_queue_macro(int argc, const char *argv[]){
    queue_t queue;
    queue = INIT_QUEUE;
    bool result = true;
    result &= queue.head == NULL;
    result &= queue.tail == NULL;
    result &= queue.size == 0;
    return result;
}

bool testing_init_node_macro(int argc, const char *argv[]){
    list_node_t node;
    node = INIT_NODE;
    bool result = true;
    result &= node.data == NULL;
    result &= node.next == NULL;
    result &= node.prev == NULL;

    return result;
}

bool mock_create_node(){
    list_node_t * node;
    int a;
    bool result = true;
    if(create_node(&a, &node)){
        result &= node != NULL;
        result &= node->next == NULL;
        result &= node->prev == NULL;
        result &= node->data == &a;
        free(node);
    }else{
        result &= node == NULL;
    }
    
    return result;
}

bool test_create_node_true(int argc, const char *argv[]){
    bool result = true;
    result &= mock_create_node();
    return result;
}

bool test_create_node_false(int argc, const char *argv[]){
    bool result = true;
#define macro(num_byte) NULL
    result &= mock_create_node();
#undef macro
    return result;
}

bool test_enqueue(int argc, const char *argv[]){
    bool result = true;
    queue_t queue = INIT_QUEUE;
    list_node_t *node1, *node2;
    create_node(NULL, &node1);
    create_node(NULL, &node2);

    enqueue(&queue, node1);

    result &= queue.head == node1;
    result &= queue.tail == node1;
    result &= queue.size == 1; 

    enqueue(&queue, node2);
    result &= queue.head == node1;
    result &= queue.tail == node2;
    result &= queue.size == 2;
    result &= queue.tail->prev == node1;
    
    free(node1);
    free(node2);
    
    return result;
}

bool test_dequeue(int argc, const char *argv[]){
    bool result = true;
    queue_t queue = INIT_QUEUE;
    list_node_t *node1, *node2;
    create_node(NULL, &node1);
    create_node(NULL, &node2);

    enqueue(&queue, node1);
    enqueue(&queue, node2);
    
    list_node_t *n;
    result &= dequeue(&queue, &n);
    
    result &= n == node1;
    result &= n->prev == NULL;
    result &= n->next == NULL;
    
    result &= queue.head == node2;
    result &= queue.head->prev == NULL;
    result &= queue.size == 1;

    result &= dequeue(&queue, &n);
    result &= n == node2;
    result &= n->prev == NULL;
    result &= n->next == NULL;
     

    result &= queue.tail == queue.head;
    result &= queue.tail == NULL;
    result &= queue.head == NULL;
    result &= queue.size == 0;

    result &= !dequeue(&queue, &n);
    result &= n == NULL;

    free(node1);
    free(node2);
    
    return result;
}

#define REGISTER_TEST(func, argc, argv) \
    do{ \
        printf("%s ... %s\n", str(func), (func(argc, argv)) ? pass : failed);\
    }while(0);

int main(int argc, const char *argv[]){
    REGISTER_TEST(testing_init_queue_macro, argc, argv);
    REGISTER_TEST(testing_init_node_macro, argc, argv);
    REGISTER_TEST(test_create_node_true, argc, argv);
    REGISTER_TEST(test_create_node_false, argc, argv);
    REGISTER_TEST(test_enqueue, argc, argv);
    REGISTER_TEST(test_dequeue, argc, argv);
}
