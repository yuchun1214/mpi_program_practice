#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "algorithm.h"
#include "ant.h"

bool read_table(const char *filename, table_t **table);

int main(int argc, const char *argv[]){
    if(argc < 2){
        printf("Please give the input txt file\n"); 
        exit(EXIT_FAILURE);
    }

    // FILE * file = fopen(argv[1], "r"); 
    // read the graph file
    table_t *graph_table;
    read_table(argv[1], &graph_table);
    for(int i = 0; i < graph_table->nrow; ++i){
        for(int j = 0; j < graph_table->ncol; ++j){
            printf("%03.0f ", graph_table->table[i][j]);
        }
        printf("\n");
    }

    return 0;

    table_t *pheromone;
    create_an_empty_table(graph_table->nrow, graph_table->ncol, &pheromone);

    // initialize pheromone
    for(int i = 0; i < pheromone->nrow; ++i){
        for(int j = 0; j < pheromone->ncol; ++j){
            pheromone->table[i][j] = 1;
        }
    }

    // create an ant
    ant_t ant;
    setup_an_ant(graph_table->nrow, &ant);

    // place ants on the graph
    setup_initial_city(&ant, 0); 
    
    determined_next_city(&ant, pheromone, graph_table);
   
    return 0;
}

