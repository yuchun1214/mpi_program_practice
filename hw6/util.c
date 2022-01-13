#include "util.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "table.h"

static int calculate_number_of_cols(char *text){
  	char * pch;
	int number_of_tokens = 0;
  	pch = strtok (text," ,.-");
  	while (pch != NULL)
  	{
	    ++number_of_tokens;
  	  pch = strtok (NULL, " ,.-");
  	}  
    return number_of_tokens;
}

static void form_a_row(char *text, double *row){
    char * pch;
	int number_of_tokens = 0;
  	pch = strtok (text," ,.-");
  	while (pch != NULL)
  	{
      row[number_of_tokens] = atoi(pch);
	  ++number_of_tokens;
  	  pch = strtok (NULL, " ,.-");
  	}  
}

bool read_table(const char *filename, table_t **table){
    FILE *file = fopen(filename, "r");
    const int line_length = 100000;
    char line[line_length];
    char *first_line;
    if(fgets(line, line_length, file) == NULL){
        printf("Failed to read text of the file : %s\n", filename); 
    }
    first_line = strdup(line);

    int number_of_cols = calculate_number_of_cols(first_line);
    free(first_line);
    create_an_empty_table(number_of_cols, number_of_cols, table);
    form_a_row(line, (*table)->table[0]);
    
    int line_idx = 1;
    while(fgets(line, line_length, file) != NULL){
        form_a_row(line, (*table)->table[line_idx]);
        ++line_idx;
    }
    
    fclose(file);
    return true;
}
