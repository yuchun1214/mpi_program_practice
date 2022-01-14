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

bool copy_table(table_t *dest, table_t *src){
    if(!dest || !src){
#ifdef DEBUG
        printf("One of parameters is null, return false in the function %s\n", __func__);
#endif
        return false;
    }

    // check if two tables are in the same size
    if(src->nrow == dest->nrow && src->ncol == dest->ncol){
        memcpy(dest->table[0], src->table[0], sizeof(double)*src->nrow*src->ncol);
        return true;
    }else{
#ifdef DEBUG
        printf("Two tables are not in the same size, return false in the function %s\n", __func__);
#endif
        return false;
    }
}
