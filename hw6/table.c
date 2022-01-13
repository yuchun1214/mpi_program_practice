#include "table.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

bool create_an_empty_table(int _nrow, int _ncol, table_t **_table){
    double *table_content = (double *)malloc(sizeof(double)*_nrow*_ncol);
    if(!table_content){
        perror("Failed to malloc table_content in create_a_empty_table function");
        return false;
    }

    double **row_pointers = (double **)malloc(sizeof(double*)*_nrow);
    if(!row_pointers){
        perror("Failed to malloc row_pointers in create_a_empty_table function");
        return false;
    }
   
    for(int i = 0; i < _nrow; ++i){
        row_pointers[i] = table_content + i * _ncol;
    }
    memset(table_content, 0, sizeof(int)*_ncol*_nrow); 

    table_t *table = (table_t *)malloc(sizeof(table_t));
    *_table = table;
    table->table = row_pointers;
    table->ncol = _ncol;
    table->nrow = _nrow;

    return true;
}

bool table_set_value(int row, int col, table_t *table, double val){
    if(!table || row > table->nrow || col > table->ncol || !table->table){
        return false;
    }

    table->table[row][col] = val;
    return true;
}


double table_get_value(int row, int col, table_t *table){
    return table->table[row][col]; 
}
