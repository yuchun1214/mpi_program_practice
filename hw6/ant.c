#include "ant.h"
#include "table.h"
#include "common.h"

extern double const * alpha, *beta, *evaporation_rate, *q;


bool setup_an_ant(int number_of_cities, ant_t *ant){
    if(!ant) {
#ifdef DEBUG
        printf("The variable ant is NULL, return false in the function setup_an_ant\n");
#endif
        return false;
    }
    
    *ant = INIT_ANT;

    ant->number_of_cities = number_of_cities;
    ant->cities = (city_entry_t **)malloc(sizeof(city_entry_t*)*number_of_cities);
    for(int i = 0; i <  number_of_cities; ++i) {
        ant->cities[i] = (city_entry_t*)malloc(sizeof(city_entry_t));
        *ant->cities[i] = (city_entry_t){ i, false, 0.0};
    }
    if(!ant->cities){
#ifdef DEBUG
        printf("Failed to allocate memory for ant->cities, return false in the function setup_an_ant\n");
        return false;
#endif
    }

    ant->route = (city_entry_t **)malloc(sizeof(city_entry_t*)*number_of_cities);
    
    if(!ant->route){
#ifdef DEBUG
        printf("Failed to allocate memory for ant->route, return false in the function setup_an_ant\n");
        return false;
#endif
    }

//     ant->best_route = (city_entry_t**)malloc(sizeof(city_entry_t *)*number_of_cities);
//     if(!ant->best_route){
//  #ifdef DEBUG
//         printf("Failed to allocate memory for ant->best_route, return false in the function setup_an_ant\n");
//         return false;
// #endif
//     }

    return true;
}

bool setup_initial_city(ant_t *ant, int city){
    bool set_flag = false;
    for(int i = 0; i < ant->number_of_cities; ++i){
        if(ant->cities[i]->id == city) {
            set_flag =  ant->cities[i]->step_on = true;
            ant->current_city = ant->cities[i];
            break;
        }
    }

    if(set_flag) {
        ant->route[0] = ant->current_city;
        ant->number_of_step_cities = 1;
        return true;
    } else{
#ifdef DEBUG
        printf("Cannot find city in the ant->cities, return false");
#endif
        return false;
    }
}

static int CMPcity(const void *c1, const void *c2){
    int c1_step_on = (*(city_entry_t**)c1)->step_on;
    int c2_step_on = (*(city_entry_t**)c2)->step_on;
    if(c1_step_on > c2_step_on) return 1;
    else if (c1_step_on < c2_step_on) return -1;
    else return 0;
}

city_entry_t *determined_next_city(ant_t *ant, table_t *pheromone, table_t *graph){
    // double beta = 2, alpha = 2.5;
    // sort the cities;    
    qsort(ant->cities, ant->number_of_cities, sizeof(city_entry_t*), CMPcity);
    for(int i = 0; i < ant->number_of_cities; ++i){
        // printf("Entry[%d] : %d\n", i, ant->cities[i]->step_on);
    }
    // calculate the probability for the unsteped city
    double normalization_factor = 0;
    for(int i = 0, number_of_unstep_cities = ant->number_of_cities - ant->number_of_step_cities; i < number_of_unstep_cities; ++i){
        normalization_factor += ant->cities[i]->probability = 
            pow(table_get_value(ant->current_city->id, ant->cities[i]->id, pheromone), *alpha) *
            pow(table_get_value(ant->current_city->id, ant->cities[i]->id, graph), -*beta);
    }
    
    double cum_probability = 0;
    for(int i = 0, number_of_unstep_cities = ant->number_of_cities - ant->number_of_step_cities; i < number_of_unstep_cities; ++i){
        cum_probability += ant->cities[i]->probability / normalization_factor;
        ant->cities[i]->probability = cum_probability;
        // printf("City[%d] : %.3f\n", i, ant->cities[i]->probability);
    }
    
    double rnd_val = rand_double_01();
    int city_idx = 0;
    for(int i = 0, number_of_unstep_cities = ant->number_of_cities - ant->number_of_step_cities; i < number_of_unstep_cities; ++i){
        if(rnd_val < ant->cities[i]->probability){
            city_idx = i;
            break;
        }
    }
    ant->prev_edge.i = ant->current_city->id;    
    // printf("rnd_val : %.3f, city_idx = %d\n", rnd_val, city_idx);
    ant->current_city = ant->cities[city_idx];
    ant->prev_edge.j = ant->current_city->id;
    ant->route[ant->number_of_step_cities] = ant->current_city;
    ++ant->number_of_step_cities;
    ant->current_city->step_on = true; 
    ant->tour_length += table_get_value(ant->prev_edge.i, ant->prev_edge.j, graph); 
    return ant->current_city;
}

double delta_Lk(ant_t *ant, int i, int j){
    if(i == ant->prev_edge.i && j == ant->prev_edge.j){
        return *q / ant->tour_length;
    }
    return 0;
}

void update_pheromone_table(ant_colony_t *colony, table_t *pheromone){
    for(int i = 0; i < pheromone->nrow; ++i){
        for(int j = 0; j < pheromone->ncol; ++j){
            double summation_of_delta_k = 0;
            for(int k = 0; k < colony->number_of_ants; ++k){
                summation_of_delta_k += delta_Lk(&colony->ants[i], i, j);
            } 
            table_set_value(i, j, pheromone, (1-*evaporation_rate) * table_get_value(i, j, pheromone) + summation_of_delta_k);
        }
    }
}

bool setup_an_colony(int population_size, int number_of_cities, ant_colony_t *colony){
    colony->ants = (ant_t *)malloc(sizeof(ant_t)*population_size); 
    if(!colony->ants){
#ifdef DEBUG
        printf("Failed to allocate memory for colony->ants in the function setup_an_colony\n");
#endif
        return false;
    }
    colony->number_of_ants = population_size;

    for(int i = 0; i < colony->number_of_ants; ++i){
        setup_an_ant(number_of_cities, &colony->ants[i]); 
    }
    return true;
}

void back_to_origin(ant_t *ant, table_t *graph){
    ant->prev_edge.i = ant->current_city->id;
    ant->prev_edge.j = ant->route[0]->id;
    ant->tour_length += table_get_value(ant->prev_edge.i, ant->prev_edge.j, graph);
    ant->current_city = ant->route[0];
}

void reset_an_ant(ant_t *ant){
    for(int i = 0; i < ant->number_of_cities; ++i){
        ant->cities[i]->step_on = false;
        ant->cities[i]->probability = 0.0;
        ant->route[i] = NULL;
    }
    ant->tour_length = 0;
    ant->number_of_step_cities = 1;
    ant->route[0] = ant->current_city;
    ant->current_city->step_on = true;
}

void store_tour(ant_t *ant, int *array){
    for(int i = 0; i < ant->number_of_cities; ++i){
        array[i] = ant->route[i]->id;
    }
}
