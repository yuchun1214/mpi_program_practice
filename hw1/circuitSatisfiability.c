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

/**
 * CLEAR_BIT - a macro that sets the ith bit to 0
 *
 * @param num : a number
 * @param i : ith bit
 * @return the number whose ith bit is 0
 */
#define CLEAR_BIT(num, i) ((num) & (-1 ^ (1 << (i))))

/**
 * SET_BIT - a macro that sets the ith bit to 1
 *
 * @param num : a number
 * @param i : ith bit
 * @return the number whose ith bit is 0

 */
#define SET_BIT(num, i) ((num) | (1 << (i)))

/* EXTRACT_BIT is a macro that extracts the ith bit of number n.
 *
 * parameters: n, a number;
 *             i, the position of the bit we want to know.
 *
 * return: 1 if 'i'th bit of 'n' is 1; 0 otherwise
 */
#define EXTRACT_BIT(n, i) (((n) & (1 << (i))) ? 1 : 0)



int main(int argc, char *argv[])
{
    unsigned long long i; /* loop variable (32 bits) */
    int id = 0;           /* process id */
    int count = 0;        /* number of solutions */
    unsigned int MAX = UINT_MAX;
    int rank, comm_sz;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    double start_time = 0.0, total_time = 0.0;
    start_time = MPI_Wtime();

    // Each process takes about (MAX / comm_sz) numbers to pass into
    // checkCircuit function
    unsigned int start_idx, end_idx;
    unsigned int piece = MAX / comm_sz;
    start_idx = rank * piece;
    end_idx = (rank + 1) * piece;

    // The last process takes (MAX / comm_sz) + (MAX % comm_sz) numbers the
    // second terms is the remaining part.
    if (rank == comm_sz - 1) {
        count += checkCircuit(rank, MAX);
        end_idx = MAX;
    }


    // printf("#%d takes [%u, %u)\n", rank, start_idx, end_idx);
    for (i = start_idx; i < end_idx; ++i) {
        count += checkCircuit(rank, i);
    }

    // Send or receive the data
    // Use tree structure to collect the data for each process
    int _rank = rank;
    short bit_idx = 0;
    short head_bit = 32 - __builtin_clz(comm_sz);  // floor(log2(comm_sz)) + 1
    while (bit_idx < head_bit) {
        if (EXTRACT_BIT(_rank, bit_idx)) {
            MPI_Send(&count, 1, MPI_INT, CLEAR_BIT(_rank, bit_idx), 0,
                     MPI_COMM_WORLD);
            // printf("[%d] Send to [%d], bit_idx = %d\n", rank,
            //        CLEAR_BIT(rank, bit_idx), bit_idx);
            // fflush(stdout);
            break;
        } else {
            int other_count = 0;
            if (SET_BIT(_rank, bit_idx) < comm_sz) {
                MPI_Recv(&other_count, 1, MPI_INT, SET_BIT(_rank, bit_idx), 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                // printf("[%d] Recv from [%d], bit_idx = %d\n", rank,
                //        SET_BIT(_rank, bit_idx), bit_idx);
                // fflush(stdout);
                count += other_count;
            }
        }
        bit_idx += 1;
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

#define SIZE 32

/* checkCircuit() checks the circuit for a given input.
 * parameters: id, the id of the process checking;
 *             bits, the (long) rep. of the input being checked.
 *
 * output: the binary rep. of bits if the circuit outputs 1
 * return: 1 if the circuit outputs 1; 0 otherwise.
 */
int checkCircuit(int id, int bits)
{
    // int v[SIZE]; /* Each element is a bit of bits */
    // int i;

    // for (i = 0; i < SIZE; i++)
    //     v[i] = EXTRACT_BIT(bits, i);
    int v0, v1, v9, v10;
    v0 = EXTRACT_BIT(bits, 0);
    v1 = EXTRACT_BIT(bits, 1);
    v9 = EXTRACT_BIT(bits, 9);
    v10 = EXTRACT_BIT(bits, 10);

    unsigned int pattern = 0xFFFF9FF7;
    unsigned int _bits = bits | 0xFFFF0000;
    _bits |= 0x0603;
    // Originally, the if-clause is :
    // if (  (v[0] || v[1]) && (!v[1] || !v[3]) && (v[2] || v[3])
    //   && (!v[3] || !v[4]) && (v[4] || !v[5])
    //   && (v[5] || !v[6]) && (v[5] || v[6])
    //   && (v[6] || !v[15]) && (v[7] || !v[8])
    //   && (!v[7] || !v[13]) && (v[8] || v[9])
    //   && (v[8] || !v[9]) && (!v[9] || !v[10])
    //   && (v[9] || v[11]) && (v[10] || v[11])
    //   && (v[12] || v[13]) && (v[13] || !v[14])
    //   && (v[14] || v[15])  )
    //
    // I found the content in if-clause could be simplfied as
    // shown as below. The method I use to simplify is using Karnaugh map.
    // After doing that, the efficient got better.
    if ((v0 || v1) && (!v9 || !v10) && _bits == pattern) {
        printf("%d) 10011%d%d1111101%d%d\n", id, v10, v9, v1, v0);
        fflush(stdout);
        return 1;
    } else {
        return 0;
    }
}
