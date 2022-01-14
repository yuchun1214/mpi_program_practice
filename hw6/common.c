#include "common.h"
#include <stdlib.h>

double rand_double_01(){
    return (double) rand() / (double) (RAND_MAX + 1.0);
}
