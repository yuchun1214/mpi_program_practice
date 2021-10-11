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
    unsigned long long i; /* loop variable (32 bits) */
    int id = 0;           /* process id */
    int count = 0;        /* number of solutions */
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

    if (rank == comm_sz - 1) {
        count += checkCircuit(rank, MAX);
        end_idx = MAX;
    }


    // printf("#%d take [%u, %u)\n", rank, start_idx, end_idx);
    for (i = start_idx; i < end_idx; ++i) {
        count += checkCircuit(rank, i);
    }

    if (rank) {
        MPI_Send(&count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    } else {
        unsigned long long remaining_start_idx = (comm_sz) *piece;
        for (i = remaining_start_idx; i <= MAX; ++i) {
            count += checkCircuit(rank, i);
        }
        int source_count;
        for (unsigned int source = 1; source < (unsigned int) comm_sz;
             ++source) {
            MPI_Recv(&source_count, 1, MPI_INT, source, 0, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);
            count += source_count;
        }
    }

    if (rank == 0) {
        total_time = MPI_Wtime() - start_time;
        printf("Process %d finished in time %f secs..\n", id, total_time);
        printf("\nA total of %d solutions were found.\n\n", count);
        fflush(stdout);
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

#define SIZE 32

int checkCircuit(int id, int bits)
{
    // int v[SIZE]; /* Each element is a bit of bits */
    // int i;

    // for (i = 0; i < SIZE; i++)
    //     v[i] = EXTRACT_BIT(bits, i);
    int v0, v1, v9, v10;
    v0 = EXTRACT_BIT(bits, 1);
    v1 = EXTRACT_BIT(bits, 1);
    v9 = EXTRACT_BIT(bits, 9);
    v10 = EXTRACT_BIT(bits, 10);

    unsigned int pattern = 0xFFFF9FF7;
    unsigned int _bits = bits | 0xFFFF0000;
    _bits |= 0x0603;

    if ((EXTRACT_BIT(bits, 0) || EXTRACT_BIT(bits, 1)) &&
        (!EXTRACT_BIT(bits, 9) || !EXTRACT_BIT(bits, 10)) && _bits == pattern) {
        // printf ("%d) 10011%d%d1111101%d%d\n", id, bits,
        //    v10, v9, v1, v0);
        // fflush (stdout);
        return 1;
    } else {
        return 0;
    }
}
