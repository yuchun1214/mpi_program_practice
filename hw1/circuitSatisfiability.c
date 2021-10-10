/* circuitSatifiability.c solves the Circuit Satisfiability
 *  Problem using a brute-force sequential solution.
 *
 *   The particular circuit being tested is "wired" into the
 *   logic of function 'checkCircuit'. All combinations of
 *   inputs that satisfy the circuit are printed.
 *
 *   16-bit version by Michael J. Quinn, Sept 2002.
 *   Extended to 32 bits by Joel C. Adams, Sept 2013.
 */

#include <limits.h>  // UINT_MAX
#include <stdio.h>   // printf()

#include <mpi/mpi.h>

int checkCircuit(int, int);

int main(int argc, char *argv[])
{
    unsigned int i;         /* loop variable (32 bits) */
    int id = 0;    /* process id */
    int count = 0; /* number of solutions */
    unsigned int MAX = USHRT_MAX; 
    int rank, comm_sz;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    double start_time = 0.0, total_time = 0.0;
    start_time = MPI_Wtime();

    unsigned int start_idx, end_idx;
    unsigned int piece = MAX / comm_sz;
    start_idx = rank * piece;
    end_idx = (rank + 1) * piece;
    // printf("#%d take [%u, %u)\n", rank, start_idx, end_idx);
    for(i = start_idx; i < end_idx; ++i){
        count += checkCircuit(rank, i);
    }

    if(rank){
        MPI_Send(&count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }else{
        unsigned int remaining_start_idx = (comm_sz) * piece;
        // printf("#%d take [%d, %d]\n", rank, remaining_start_idx, USHRT_MAX);
        for(i = remaining_start_idx; i <= MAX; ++i){
            count += checkCircuit(rank, i);
        }
        int source_count;
        for(unsigned int source = 1; source < (unsigned int)comm_sz; ++source){
            MPI_Recv(&source_count, 1, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            count += source_count;
        }
    }

    if(rank == 0){
        total_time = MPI_Wtime() - start_time;
        printf("Process %d finished in time %f secs..\n", id, total_time);
        fflush(stdout);
        printf("\nA total of %d solutions were found.\n\n", count);
    }
    MPI_Finalize();
    return 0;
}

/* EXTRACT_BIT is a macro that extracts the ith bit of number n.
 *
 * parameters: n, a number;
 *             i, the position of the bit we want to know.
 *
 * return: 1 if 'i'th bit of 'n' is 1; 0 otherwise
 */

#define EXTRACT_BIT(n, i) ((n & (1 << i)) ? 1 : 0)


/* checkCircuit() checks the circuit for a given input.
 * parameters: id, the id of the process checking;
 *             bits, the (long) rep. of the input being checked.
 *
 * output: the binary rep. of bits if the circuit outputs 1
 * return: 1 if the circuit outputs 1; 0 otherwise.
 */

#define SIZE 16

int checkCircuit(int id, int bits)
{
    int v[SIZE]; /* Each element is a bit of bits */
    int i;

    for (i = 0; i < SIZE; i++)
        v[i] = EXTRACT_BIT(bits, i);

    int pattern = 0b1001111111110111; 
    int _bits = bits;
    _bits |= 0x63;

    // if ((v[0] || v[1]) && (!v[9] || !v[10]) && !(pattern ^ _bits))
   	if (  (v[0] || v[1]) && (!v[1] || !v[3]) && (v[2] || v[3])
       && (!v[3] || !v[4]) && (v[4] || !v[5])
       && (v[5] || !v[6]) && (v[5] || v[6])
       && (v[6] || !v[15]) && (v[7] || !v[8])
       && (!v[7] || !v[13]) && (v[8] || v[9])
       && (v[8] || !v[9]) && (!v[9] || !v[10])
       && (v[9] || v[11]) && (v[10] || v[11])
       && (v[12] || v[13]) && (v[13] || !v[14])
       && (v[14] || v[15]) ) 
    {
       printf ("%d) %d => %d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d \n", id, bits, 
          v[15],v[14],v[13],v[12],
          v[11],v[10],v[9],v[8],v[7],v[6],v[5],v[4],v[3],v[2],v[1],v[0]);
       fflush (stdout);
       return 1;
    } else {
       return 0;
    }

}
