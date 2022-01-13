#include "ant.h"
#include "table.h"

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
        ant->route[ant->number_of_step_cities] = ant->current_city;
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
    double beta = 2, alpha = 2.5;
    // sort the cities;    
    qsort(ant->cities, ant->number_of_cities, sizeof(city_entry_t*), CMPcity);
    for(int i = 0; i < ant->number_of_cities; ++i){
        printf("Entry[%d] : %d\n", i, ant->cities[i]->step_on);
    }
    // calculate the probability for the unsteped city
    double normalization_factor = 0;
    for(int i = 0, number_of_unstep_cities = ant->number_of_cities - ant->number_of_cities; i < number_of_unstep_cities; ++i){
        normalization_factor += ant->cities[i]->probability = 
            pow(table_get_value(ant->current_city->id, ant->cities[i]->id, pheromone), alpha) *
            pow(table_get_value(ant->current_city->id, ant->cities[i]->id, graph), -beta);
    }
    for(int i = 0, number_of_unstep_cities = ant->number_of_cities - ant->number_of_cities; i < number_of_unstep_cities; ++i){
        ant->cities[i]->probability /= normalization_factor;
        printf("City[%d] : %.3f\n", ant->cities[i]->id, ant->cities[i]->probability);
    }
    return ant->current_city;
}


