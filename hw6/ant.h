#ifndef __ANT_H__
#define __ANT_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include "table.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __city_entry{
    int id;
    bool step_on;
    double probability;
}city_entry_t;

typedef struct __ant{
    city_entry_t *current_city; // point on the current city in the array
    city_entry_t **cities; // an array
    city_entry_t **route; 
    int number_of_cities;
    int number_of_step_cities;
    struct {
        int i, j;
    } prev_edge;
    double tour_length;
}ant_t;

typedef struct __ant_colony{
    ant_t * ants;
    int number_of_ants;
}ant_colony_t;


#define INIT_ANT (ant_t){ .current_city = NULL, .cities = NULL, .route = NULL, .number_of_cities = 0, .number_of_step_cities = 0, .prev_edge = {.i = 0, .j = 0}, .tour_length = 0}

#define INIT_COLONY { .ants = NULL, .number_of_ants = 0}

bool setup_an_ant(int number_of_cities, ant_t *ant);
bool setup_initial_city(ant_t *ant, int city);
city_entry_t * determined_next_city(ant_t *ant, table_t *pheromone, table_t *graph);
double delta_Lk(ant_t *ant, int i, int j);
void update_pheromone_table(ant_colony_t *colony, table_t *pheromone);
bool setup_an_colony(int population_size, int number_of_cities, ant_colony_t *colony);
void back_to_origin(ant_t *ant, table_t *graph);
void reset_an_ant(ant_t *ant);
void store_tour(ant_t *ant, int*);
#ifdef __cplusplus
}
#endif

#endif
