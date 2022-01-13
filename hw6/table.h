#ifndef __TABLE_H__
#define __TABLE_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __table_t{
    double **table;
    int ncol;
    int nrow;
}table_t;

#define INIT_TABLE (table_t){ .table = NULL, .ncol = NULL, .nrow = NULL}

bool create_an_empty_table(int nrow, int ncol, table_t **table);
bool table_set_value(int row, int col, table_t *table, double val);
double table_get_value(int row, int col, table_t * table);

#ifdef __cplusplus
}
#endif

#endif
