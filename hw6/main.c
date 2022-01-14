#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include <mpi/mpi.h>
#include <omp.h>

#include "algorithm.h"
#include "ant.h"
#include "table.h"
#include "util.h"
#include "def.h"

double _alpha = 1;
double _beta = 3.5;
double _evaporation_rate = 0.001;
double _q = 0.02;
const double maximum_generations = 1000;
const int population_size = 500;
// const int number_of_threads = population_size / ( population_size / 16);
const int number_of_threads = 2;

double const * alpha = &_alpha;
double const * beta = &_beta;
double const * evaporation_rate = &_evaporation_rate;
double const * q = &_q;

void setup(int rank, const char *filename, table_t **table);



int main(int argc, const char *argv[]){
    if(argc < 2){
        printf("Please give the input txt file\n"); 
        exit(EXIT_FAILURE);
    }
    int rank, comm_sz;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    srand(time(NULL));

    // read the graph file
    table_t *graph_table;
    setup(rank, argv[1], &graph_table);

    table_t *pheromone, *best_pheromone;
    create_an_empty_table(graph_table->nrow, graph_table->ncol, &pheromone);
    create_an_empty_table(graph_table->nrow, graph_table->ncol, &best_pheromone);
    int *best_tour = (int *)malloc(sizeof(int)*graph_table->ncol);
    double best_tour_length = INT_MAX, local_best_tour_length = INT_MAX;


    // initialize pheromone
    for(int i = 0; i < pheromone->nrow; ++i){
        for(int j = 0; j < pheromone->ncol; ++j){
            pheromone->table[i][j] = 1;
            best_pheromone->table[i][j] = 1;
        }
    }



    // create a colony
    ant_colony_t colony;
    setup_an_colony(population_size, graph_table->nrow, &colony);
    
    
    // place ants on the graph
    for(int i = 0;i < colony.number_of_ants; ++i){
        setup_initial_city(&colony.ants[i], initial_city(graph_table->nrow));
    }
    int times_of_invariance = 0; 
    bool has_varied;
    for(int g = 0; g < maximum_generations; ++g){
        // a generation : 
        has_varied = false;
        for(int i = 0 ; i < graph_table->nrow - 1; ++i){
#pragma omp parallel for num_threads(number_of_threads)
            for(int j = 0; j < colony.number_of_ants; ++j){
                determined_next_city(&colony.ants[j], pheromone, graph_table);
            }
            // update the pheromone table
            update_pheromone_table(&colony, pheromone); 
        }

#pragma omp parallel for num_threads(number_of_threads)
        for(int j = 0; j < colony.number_of_ants; ++j){
            back_to_origin(&colony.ants[j], graph_table);
        }

        update_pheromone_table(&colony, pheromone);
        // each ant has a solution
        // find out the best solution in this generation and update the 
        // global best solution
        // find lowest length and the index
        
        double lowest_length = INT_MAX;
        int lowest_entry_index;
        for(int i = 0; i < colony.number_of_ants; ++i){
            if(colony.ants[i].tour_length < lowest_length){
                lowest_length = colony.ants[i].tour_length;
                lowest_entry_index = i;
            }    
        }
        if(lowest_length < best_tour_length){
            local_best_tour_length = best_tour_length = lowest_length;
            store_tour(&colony.ants[lowest_entry_index], best_tour);
            times_of_invariance = 0;
            copy_table(best_pheromone, pheromone);
            has_varied = true;
            // printf("Found better solution : %.2f\n", best_tour_length);
        }

        if(!has_varied) ++times_of_invariance;

        if(times_of_invariance > 30){
            // printf(RED "Varied!\n" RESET);
		    times_of_invariance = 0;	
            _alpha += 0.2;
            _beta += 0.5;
            _q -= _q * 0.1;
            copy_table(pheromone, best_pheromone);
        }

        // reset all ants to start next generation
#pragma omp parallel for num_threads(number_of_threads)
        for(int j = 0; j < colony.number_of_ants; ++j){
            reset_an_ant(&colony.ants[j]);
        }
          
        // exchange part, 
        if(g % 100 == 0){
            MPI_Barrier(MPI_COMM_WORLD);
            double global_best;
            MPI_Allreduce(&best_tour_length, &global_best, 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);
            if(rank == 0){
                // printf("Global best is now : %2f\n", best_tour_length);
            } 
        }         
    }
    
    // collect best solution 
    // printf("rank[%d] Best solution = %.2f\n",rank,local_best_tour_length);

    struct{
        int rank, best_tour;
    } loc_data, global_data;

    loc_data.rank = rank;
    loc_data.best_tour = local_best_tour_length;
    
    MPI_Allreduce(&loc_data, &global_data, 1, MPI_2INT, MPI_MINLOC, MPI_COMM_WORLD);

    if(rank == global_data.rank){
        printf("Rank %d show : %d\n", rank, loc_data.best_tour);
        for(int i = 0; i < graph_table->nrow; ++i){
            printf("%d ", best_tour[i]);
        }
        printf("\n");

    }
      
    
    MPI_Finalize();
    return 0;
}


void setup(int rank, const char *filename, table_t **graph_table){
    int dim;
    if(rank == 0){
        // printf("Rank == %d -> read the table\n", rank);
        read_table(filename, graph_table);
        dim = (*graph_table)->nrow;
    }
    // scatter table dim
    MPI_Bcast(&dim, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if(rank != 0){
        create_an_empty_table(dim ,dim, graph_table);
    }

    MPI_Bcast((*graph_table)->table[0], dim*dim, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    // if(rank == 1){
    //     for(int i = 0; i < (*graph_table)->nrow; ++i){
    //         for(int j = 0; j < (*graph_table)->ncol; ++j){
    //             printf("%03.0f ", (*graph_table)->table[i][j]);
    //         }
    //         printf("\n");
    //     }

    // }
}
