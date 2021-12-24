#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

void Count_sort(int a[], int n) { 
    int i, j, count;
    int* temp = (int*)malloc(n*sizeof(int));
#pragma omp parallel for private(j) private(count) num_threads(4)
    for (i = 0; i < n; i++) { 
        count = 0; 
        // printf("[%d] thread id = %d\n", i, omp_get_thread_num());
#pragma omp parallel for reduction(+:count) num_threads(2)
        for (j = 0; j < n; j++) 
            if (a[j] < a[i]) count++; 
            else if (a[j] == a[i] && j < i) count++;
        temp[count] = a[i]; 
    } 
    memcpy(a, temp, n*sizeof(int)); 
    free(temp); 
} 

void Count_sort2(int a[], int n) { 
    int i, j, count;
    int* temp = (int*)malloc(n*sizeof(int));
    for (i = 0; i < n; i++) { 
        count = 0; 
        for (j = 0; j < n; j++) 
            if (a[j] < a[i]) count++; 
            else if (a[j] == a[i] && j < i) count++;
        temp[count] = a[i]; 
    } 
    memcpy(a, temp, n*sizeof(int)); 
    free(temp); 
}

void simple_test(){
    int thread_id1, thread_id2;
    int sum = 0;
    int i, j;
#pragma omp parallel for 
    for(i = 0; i < 100; ++i){
        for(int j = 0; j < 10000;++j)
            for(int k = 0; k < 100000; ++k);

        int thread_id = omp_get_thread_num();
        printf("[%d]Thread_id = %d\n",i, thread_id);
    }
}

void testing(int *a, int n){
    for(int i = 1; i < n; ++i){
        if(a[i] < a[i - 1]){
            printf("Wrong !\n");
            exit(EXIT_FAILURE);
        }
    }
}

int CMPup(const void *a, const void *b){
    return *(int *)a > *(int *)b ? 1 : -1;
}

#define N 100000

int main(int argc, char *argv[]){
    int *a = (int*)malloc(sizeof(int)*N);
    for(int i = 0; i < N; ++i)
        a[i] = rand();

    int *aa = (int*)malloc(sizeof(int)*N);
    memcpy(aa, a, sizeof(int)*N);

    time_t now;

    now = time(NULL);
    // qsort(aa, N, sizeof(int), CMPup);
    Count_sort2(aa, N);
    printf("Time elapse of qsort: %lu s\n", time(NULL) - now);


    
    now = time(NULL);
    Count_sort(a, N);
    printf("Time elapse p_count_sort: %lu s\n", time(NULL) - now);


    testing(a, N);
    /*
    for(int i = 0; i < N; ++i)
        printf("%d, ", a[i]);
    printf("\n");
    */
    // simple_test();
}
