#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include "algorithm.h"
#include "ant.h"
#include "table.h"
#include "util.h"
#include "def.h"

double _alpha = 1;
double _beta = 3.5;
double _evaporation_rate = 0.001;
double _q = 0.02;
const double maximum_generations = 500;

double const * alpha = &_alpha;
double const * beta = &_beta;
double const * evaporation_rate = &_evaporation_rate;
double const * q = &_q;

bool read_table(const char *filename, table_t **table);

int main(int argc, const char *argv[]){
    if(argc < 2){
        printf("Please give the input txt file\n"); 
        exit(EXIT_FAILURE);
    }

    // srand(time(NULL));

    // read the graph file
    table_t *graph_table;
    read_table(argv[1], &graph_table);
    // for(int i = 0; i < graph_table->nrow; ++i){
    //     for(int j = 0; j < graph_table->ncol; ++j){
    //         printf("%03.0f ", graph_table->table[i][j]);
    //     }
    //     printf("\n");
    // }

    // return 0;

    table_t *pheromone, *best_pheromone;
    create_an_empty_table(graph_table->nrow, graph_table->ncol, &pheromone);
    create_an_empty_table(graph_table->nrow, graph_table->ncol, &best_pheromone);
    int *best_tour = (int *)malloc(sizeof(int)*graph_table->ncol);
    double best_tour_length = 100000000;


    // initialize pheromone
    for(int i = 0; i < pheromone->nrow; ++i){
        for(int j = 0; j < pheromone->ncol; ++j){
            pheromone->table[i][j] = 1;
            best_pheromone->table[i][j] = 1;
        }
    }


    // create a colony
    ant_colony_t colony;
    setup_an_colony(500, graph_table->nrow, &colony);

    
    // place ants on the graph
    for(int i = 0;i < colony.number_of_ants; ++i){
        setup_initial_city(&colony.ants[i], initial_city(graph_table->nrow));
    }
    int times_of_invariance = 0; 
    bool has_varied;
    for(int g = 0; g < maximum_generations; ++g){
        // a generation : 
        // printf("Generation[%d] starts\n", g);
        has_varied = false;
        for(int i = 0 ; i < graph_table->nrow - 1; ++i){
            for(int j = 0; j < colony.number_of_ants; ++j){
                // printf("g = %d, i = %d, j = %d\n", g, i , j);
                determined_next_city(&colony.ants[j], pheromone, graph_table);
            }
            // update the pheromone table
            update_pheromone_table(&colony, pheromone); 
        }
        for(int j = 0; j < colony.number_of_ants; ++j){
            // printf("g == %d, j == %d\n", g, j);
            back_to_origin(&colony.ants[j], graph_table);
        }
        update_pheromone_table(&colony, pheromone);
        // each ant has a solution
        // find out the best solution in this generation and update the 
        // global best solution
         
        for(int i = 0; i < colony.number_of_ants; ++i){
            if(colony.ants[i].tour_length < best_tour_length){
                best_tour_length = colony.ants[i].tour_length;
                // printf("g = %d, i = %d\n", g, i);
                store_tour(&colony.ants[i], best_tour);
                times_of_invariance = 0;
                copy_table(best_pheromone, pheromone);
                has_varied = true;
                printf("Found better solution : %.2f\n", best_tour_length);
            }    
        }
        
        if(!has_varied) ++times_of_invariance;

        if(times_of_invariance > 30){
            printf(RED "Varied!\n" RESET);
		    times_of_invariance = 0;	
            _alpha += 0.2;
            _beta += 0.5;
            _q -= _q * 0.1;
            copy_table(pheromone, best_pheromone);
        }

        // reset all ants to start next generation
        for(int j = 0; j < colony.number_of_ants; ++j){
            reset_an_ant(&colony.ants[j]);
        }
        printf("Generation[%d] is finished\n", g);
    }
        
    printf("Best solution = %.2f\n", best_tour_length);
    for(int i = 0; i < graph_table->nrow; ++i){
        printf("%d ", best_tour[i]);
    }
    printf("\n");

    return 0;
}

