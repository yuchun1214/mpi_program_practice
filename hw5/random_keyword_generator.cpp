#include <cstdio>
#include <limits>
#include <string>
#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace std;


double random_range(int lower, int upper){
    double rnd;
    rnd = (double)rand() / RAND_MAX;
    rnd = lower + rnd * (upper - lower); 
    return rnd;
}

string generate_string(int number_of_character, char **text, int number_of_keywords, double probability){
    string s;
    if(probability > 0.5){
        for(int i = 0; i < number_of_character; ++i){
            s += (char)random_range(33, 126);     
        }
    }else{
        int idx = random_range(0, number_of_keywords);
        s = text[idx];
    }
    return s;
}

void create_a_file(int suffix, char **text, int number_of_keywords){
    string file_name = to_string(suffix) + ".txt";
    FILE *file = fopen(file_name.c_str(), "w");
    string s1, s2, s3;
    int number_of_lines = random_range(500000, 1000000);
    for(int i = 0; i < number_of_lines; ++i){
        s1 = generate_string(random_range(2, 10), text, number_of_keywords, random_range(0, 1));
        s2 = generate_string(random_range(2, 10), text, number_of_keywords, random_range(0, 1));
        s3 = generate_string(random_range(2, 10), text, number_of_keywords, random_range(0, 1));
        fprintf(file, "%s %s %s\n", s1.c_str(), s2.c_str(), s3.c_str());
    }
    fclose(file);
}

int main(int argc, const char *argv[]){ 
    int number_of_files;
    if(argc < 2){
        printf("arguments : [number of files]\n");
        exit(EXIT_FAILURE);
    }else{
        number_of_files = atoi(argv[1]);
    }

    int number_of_keywords = 0;
    char ** keyword_list = (char **)malloc(sizeof(char *)*10000);
    char *text = (char*)malloc(sizeof(char)*100);
    FILE *file = fopen("keyword.txt", "r");
    while(fscanf(file, "%s", text) != EOF){
        keyword_list[number_of_keywords] = strdup(text);
        ++number_of_keywords;
    }

    for(int i = 0; i < number_of_files; ++i){
        create_a_file(i, keyword_list, number_of_keywords);
    }

    for(int i = 0; i < number_of_keywords; ++i){
        free(keyword_list[i]);
    }

    free(text);
    free(keyword_list);
}
