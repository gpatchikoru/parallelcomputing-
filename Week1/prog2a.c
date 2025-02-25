#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 100000
// Optimized function to check if a number is prime
int check_prime(int num) {
    if (num < 2) return 0;
    if (num == 2 || num == 3) return 1;
    if (num % 2 == 0) return 0; // exclude even numbers >2
    for (int i = 3; i * i <= num; i += 2) {  
        if (num % i == 0) return 0;
    }
    return 1;
}
int main(int argc, char *argv[]) {
    int rank, size;
    int *numbers = NULL, *sub_array = NULL;
    int local_prime_count = 0, total_prime_count = 0;
    int *send_counts = NULL, *displacements = NULL;
    int recv_count;
    double start_time, end_time;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (rank == 0) {
        numbers = (int *)malloc(ARRAY_SIZE * sizeof(int));
        if (numbers == NULL) {
            fprintf(stderr, "memory allocation failed for numbers array\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        srand(time(NULL));
        for (int i = 0; i < ARRAY_SIZE; i++) {
            numbers[i] = rand() % 100000 + 1;
        }
        //calculate send_counts and displacements
        send_counts = (int *)malloc(size * sizeof(int));
        displacements = (int *)malloc(size * sizeof(int));
        if (send_counts == NULL || displacements == NULL) {
            fprintf(stderr, "memory allocation failed for send_counts/displacements\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        int remainder = ARRAY_SIZE % size;
        displacements[0] = 0;
        for (int i = 0; i < size; i++) {
            send_counts[i] = ARRAY_SIZE / size;
            if (i < remainder) send_counts[i]++;
            if (i > 0) displacements[i] = displacements[i - 1] + send_counts[i - 1];
        }
    }
    //each process determines its receive count
    MPI_Scatter(send_counts, 1, MPI_INT, &recv_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
    sub_array = (int *)malloc(recv_count * sizeof(int));
    if (sub_array == NULL) {
        fprintf(stderr, "memory allocation failed for sub_array\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    MPI_Barrier(MPI_COMM_WORLD);  //synchronize before measuring execution time
    start_time = MPI_Wtime();
    //scatter the array using MPI_Scatterv
    MPI_Scatterv(numbers, send_counts, displacements, MPI_INT, sub_array, recv_count, MPI_INT, 0, MPI_COMM_WORLD);
    //check primes in local sub_array
    for (int i = 0; i < recv_count; i++) {
        if (check_prime(sub_array[i])) {
            local_prime_count++;
        }
    }
    //gather all local counts to process 0
    int *local_counts = NULL;
    if (rank == 0) {
        local_counts = (int *)malloc(size * sizeof(int));
        if (local_counts == NULL) {
            fprintf(stderr, "memory allocation failed for local_counts\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
    MPI_Gather(&local_prime_count, 1, MPI_INT, local_counts, 1, MPI_INT, 0, MPI_COMM_WORLD);
    end_time = MPI_Wtime();
    //process 0 prints results
    if (rank == 0) {
        total_prime_count = 0;
        for (int i = 0; i < size; i++) {
            printf("process %d found %d primes.\n", i, local_counts[i]);
            total_prime_count += local_counts[i];
        }
        printf("total prime numbers found: %d\n", total_prime_count);
        printf("execution time: %f seconds\n", end_time - start_time);
        free(local_counts);
        free(send_counts);
        free(displacements);
        free(numbers);
    }
    free(sub_array);
    MPI_Finalize();
    return 0;
}
