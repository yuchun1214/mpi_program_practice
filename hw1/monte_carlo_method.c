#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <mpi/mpi.h>

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



static double _random_denominator = RAND_MAX >> 1;

/** _random () - get the random number in [-1, 1]
 *
 * @return : a random float number in [-1, 1]
 */
static inline double _random()
{
    return (rand() / _random_denominator) + -1;
}

/**
 * monte_carlo_method - Perform monte carlo method counting part
 *
 * The function counts the number of tosses in the circle.
 *
 * @param number_of_tosses : given the number_of_tosses to conduct the
 * experiment
 * @return : return the number of tosses which are in the circle
 */
static inline unsigned long long monte_carlo_method(
    unsigned long long number_of_tosses)
{
    double x, y;
    double distance_squared = 0;
    unsigned long long number_in_circle = 0;

    for (unsigned long long toss = 0; toss < number_of_tosses; ++toss) {
        x = _random();
        y = _random();
        distance_squared = x * x + y * y;
        if (distance_squared <= 1)
            ++number_in_circle;
    }
    return number_in_circle;
}

/**
 * get_input : get the user input
 *
 * Only the main process uses I/O to get the user input. After get the user
 * input, main process send the user inpug to other processes. Other processes
 * wait for receiving the input from the main process.
 *
 * @param rank : the process' rank
 * @param comm_sz : the communication size
 * @param number_of_tosses : the number of tosses should be performed on monte
 * carlo method
 */
static inline void get_input(int rank,
                             int comm_sz,
                             unsigned long long *number_of_tosses)
{
    if (rank) {
        MPI_Recv(number_of_tosses, 1, MPI_UNSIGNED_LONG_LONG, 0, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } else {
        scanf("%llu", number_of_tosses);
        for (int i = 1; i < comm_sz; ++i) {
            MPI_Send(number_of_tosses, 1, MPI_UNSIGNED_LONG_LONG, i, 0,
                     MPI_COMM_WORLD);
        }
    }
}


int main()
{
    srand(time(NULL));
    unsigned long long number_of_tosses;
    unsigned long long total_number_in_circle = 0;

    MPI_Init(NULL, NULL);
    int comm_sz, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    get_input(rank, comm_sz, &number_of_tosses);

    double start_time = 0.0, total_time = 0.0;
    start_time = MPI_Wtime();


    unsigned long long piece = number_of_tosses / comm_sz;
    unsigned long long start_idx, end_idx;
    start_idx = rank * piece;
    end_idx = (rank + 1) * piece;
    if (rank == comm_sz - 1) {
        end_idx = number_of_tosses;
    }
    total_number_in_circle = monte_carlo_method(end_idx - start_idx);

    // Send or receive the data
    // I use tree structure to send or receive the data.
    // After the process send the data, the process breaks the loop and which is
    // end
    int _rank = rank;
    short bit_idx = 0;
    short head_bit = 32 - __builtin_clz(comm_sz);
    while (bit_idx < head_bit) {
        if (EXTRACT_BIT(_rank, bit_idx)) {
            MPI_Send(&total_number_in_circle, 1, MPI_UNSIGNED_LONG_LONG,
                     CLEAR_BIT(_rank, bit_idx), 0, MPI_COMM_WORLD);
            // printf("[%d] Send to [%d], bit_idx = %d\n", rank,
            //        CLEAR_BIT(rank, bit_idx), bit_idx);
            // fflush(stdout);
            break;
        } else {
            unsigned long long other_count = 0;
            if (SET_BIT(_rank, bit_idx) < comm_sz) {
                MPI_Recv(&other_count, 1, MPI_UNSIGNED_LONG_LONG,
                         SET_BIT(_rank, bit_idx), 0, MPI_COMM_WORLD,
                         MPI_STATUS_IGNORE);
                // printf("[%d] Recv from [%d], bit_idx = %d\n", rank,
                //        SET_BIT(_rank, bit_idx), bit_idx);
                // fflush(stdout);
                total_number_in_circle += other_count;
            }
        }
        bit_idx += 1;
    }

    if (rank == 0) {
        double pi_estimate =
            4 * total_number_in_circle / (double) number_of_tosses;
        total_time = MPI_Wtime() - start_time;
        printf("Process %d finished in time %f secs..\n", rank, total_time);
        printf("pi_estimate = %f\n", pi_estimate);
        fflush(stdout);
    }

    MPI_Finalize();
    return 0;
}
