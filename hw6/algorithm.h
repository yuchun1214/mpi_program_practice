#ifndef __ALGORITHM_H__
#define __ALGORITHM_H__

#include <stdbool.h>
#include "table.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __algorithm_t{
    table_t *graph, *pheromone;
}algorithm_t;

#ifdef __cplusplus
}
#endif

#endif
