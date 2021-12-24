#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "queue.h"

#include <vector>
#include <map>
#include <set>

#include <omp.h>

using namespace std;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct __thread_data_t{
    char *filename;
    queue_t *queue;
}thread_data_t;

int shutdown = 0;
int number_of_producers = 0;
int no_line = 0;

map<string, int> counter;

void *producer_function(void *_data){
    thread_data_t *data = (thread_data_t*)_data;
    queue_t *queue = data->queue;

    FILE *file = fopen(data->filename, "r");
    char *line = (char*)malloc(sizeof(char)*10000);
    size_t sizen;
    list_node_t *node;
    // actually use fprintf(file, "%s", line) can automatically seperate the line by space
    while(getline(&line, &sizen, file) != EOF){
        if(create_node(strdup(line), &node)){
            pthread_mutex_lock(&lock);
            enqueue(queue, node);
            pthread_mutex_unlock(&lock);
        }else{
            perror("There is no more memory!\n");
            exit(EXIT_FAILURE);
        }
    }

    fclose(file);
    free(line);
    ++shutdown;
    return NULL;
}

void tokenize(char *text){
    char *prev_ptr, *current_ptr;
    prev_ptr = current_ptr = text;
    vector<string> tokens;
    while(*current_ptr != '\0'){
        if(*current_ptr == ' '){
            *current_ptr = '\0';
            ++current_ptr;
            tokens.push_back(string(prev_ptr));
            prev_ptr = current_ptr;
        } else ++ current_ptr;
    }
    tokens.push_back(prev_ptr);

    for(int i = 0, size = tokens.size(); i < size; ++i){
        if(counter.count(tokens[i]) != 0){
            pthread_mutex_lock(&counter_lock);
            counter[tokens[i]]++;
            pthread_mutex_unlock(&counter_lock);
        }
    }
}

void* consumer_function(void *_data){
    thread_data_t *data = (thread_data_t *)_data;
    queue_t *queue = data->queue;
    list_node_t *node;
    while(shutdown != number_of_producers || (shutdown == number_of_producers && queue->size != 0)){
        pthread_mutex_lock(&lock);
        dequeue(queue, &node);
        pthread_mutex_unlock(&lock);
        if(node) {
            tokenize((char*)node->data);
            free_node(&node);
        }
    }
    return NULL;
}

int main(int argc, const char *argv[]){
    if(argc < 2){
        printf("You need to pass the argument like this : ");
        printf("$> ./find [number_of_consumers] [keyword.txt] [text_file2] ... \n");
        exit(EXIT_FAILURE);
    }
    
    char *text = (char*)malloc(sizeof(char)*100);
    FILE *file = fopen(argv[2], "r");
    while(fscanf(file, "%s", text) != EOF) counter[text] = 0;

    queue_t queue = INIT_QUEUE;
    
    thread_data_t *data = (thread_data_t*)malloc(sizeof(thread_data_t)*argc - 3);
    number_of_producers = argc - 3;
    int number_of_consumers = atoi(argv[1]);
    for(int i = 3; i < argc; ++i){
        printf("read file : %s\n", argv[i]);
        data[i-3] = (thread_data_t){ .filename = strdup(argv[i]), .queue = &queue};
    }

//     #pragma omp parallel num_threads(number_of_producers + number_of_consumers)
//     {
//         int id = omp_get_thread_num();
//         if(id < number_of_producers){
//             producer_function(data + id);
//         }else{
//             consumer_function(data);
//         }
//     }
    // for(int i = 0, total_number_of_threads = number_of_consumers + number_of_producers; i < total_number_of_threads; ++i){
    //         }
    
    pthread_t *producers = (pthread_t*)malloc(sizeof(pthread_t)*number_of_producers);
    pthread_t *consumers = (pthread_t*)malloc(sizeof(pthread_t)*number_of_consumers);
    for(int i = 0; i < number_of_producers; ++i){
        pthread_create(&producers[i], NULL, producer_function, data + i);
    }
     
    for(int i = 0; i < number_of_consumers; ++i){
        pthread_create(consumers + i, NULL, consumer_function, &data[0]); 
    }

    for(int i = 0; i < number_of_consumers; ++i){
        pthread_join(consumers[i], NULL);
    }

    for(auto it = counter.begin(); it != counter.end(); it++){
        printf("%s -> %d\n", it->first.c_str(), it->second); 
    } 
    printf("Total number of lines : %d\n", no_line);
}
