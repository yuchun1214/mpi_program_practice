#include <mpi/mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void read_input(int rank, int comm_sz, int *n)
{
    if (rank == 0) {
        scanf("%d", n);
    }
    MPI_Bcast(n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    *n /= comm_sz;


    // printf("Rank[%d] got %d\n",rank, *n);
}

int cmp(const void *a, const void *b)
{
    if (*(int *) a > *(int *) b)
        return 1;
    else
        return -1;
}

int compute_partner(int phase, int rank, int comm_sz)
{
    int partner;
    if (phase % 2 == 0) {
        if (rank % 2 != 0)
            partner = rank - 1;
        else
            partner = rank + 1;
    } else {
        if (rank % 2 != 0)
            partner = rank + 1;
        else
            partner = rank - 1;
    }

    if (partner == -1 || partner == comm_sz)
        partner = MPI_PROC_NULL;

    return partner;
}

int main(int argc, char *argv[])
{
    FILE *fout = freopen("result.txt", "w", stdout);

    int comm_sz, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
        srand(time(NULL));

    int n;
    read_input(rank, comm_sz, &n);
    int *arr = (int *) malloc(sizeof(int) * n);
    int *arr2 = (int *) malloc(sizeof(int) * (n));
    int *temp = (int *) malloc(sizeof(int) * n);
    for (int i = 0; i < n; ++i) {
        arr[i] = rand();
    }

    qsort(arr, n, sizeof(int), cmp);

    // if(rank == 0){
    //     printf("Rank 0 : \n");
    //     for(int i = 0; i < n; ++i){
    //         printf("\t%d\n", arr[i]);
    //     }
    //     fflush(stdout);
    // }
    // MPI_Barrier(MPI_COMM_WORLD);

    // if(rank == 1){
    //     printf("Rank 1 : \n");
    //     for(int i = 0; i < n; ++i){
    //         printf("\t%d\n", arr[i]);
    //     }
    //     fflush(stdout);
    // }
    // MPI_Barrier(MPI_COMM_WORLD);
    for (int phase = 0; phase < comm_sz; ++phase) {
        int partner = compute_partner(phase, rank, comm_sz);
        MPI_Sendrecv(arr, n, MPI_INT, partner, 0, arr2, n, MPI_INT, partner, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        memcpy(temp, arr, sizeof(int) * n);
        int i, j, k;
        if (partner == MPI_PROC_NULL)
            continue;

        if (rank < partner) {  // keep the smaller
            i = j = k = 0;
            while (i < n && j < n && k < n) {
                if (temp[i] < arr2[j]) {
                    arr[k++] = temp[i++];
                } else {
                    arr[k++] = arr2[j++];
                }
            }
        } else {
            i = j = k = n - 1;
            while (i >= 0 && j >= 0 && k >= 0) {
                if (temp[i] > arr2[j]) {
                    arr[k--] = temp[i--];
                } else {
                    arr[k--] = arr2[j--];
                }
            }
        }

        // if(rank == 1){
        //     printf("Phase %d : \n", phase);
        //     printf("\ttemp : \n");
        //     for(int i = 0; i < n; ++i){
        //         printf("\t%d\n", temp[i]);
        //     }
        //     printf("\tarr2 : \n");
        //     for(int i = 0; i < n; ++i){
        //         printf("\t%d\n", arr2[i]);
        //     }
        //     printf("\tarr : \n");
        //     for(int i = 0; i < n; ++i){
        //         printf("\t%d\n", arr[i]);
        //     }
        // }
    }



    int *main_container = (int *) malloc(sizeof(int) * n * comm_sz);
    MPI_Gather(arr, n, MPI_INT, main_container, n, MPI_INT, 0, MPI_COMM_WORLD);
    if (rank == 0) {
        for (int i = 0, size = n * comm_sz; i < size; ++i) {
            printf("%d\n", main_container[i]);
        }
    }


    MPI_Finalize();
    fclose(fout);
    return 0;
}
