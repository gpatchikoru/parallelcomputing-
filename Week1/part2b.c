#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

//function to determine if a number is prime
int is_prime(int num) {
    if (num < 2) return 0;
    for (int i = 2; i * i <= num; i++) {
        if (num % i == 0) return 0;
    }
    return 1;
}
//function to generate a specified count of prime numbers
void find_primes(int *primes, int count) {
    int candidate = rand() % 10000 + 2;  //start at a random number > 1
    int found = 0;
    while (found < count) {
        if (is_prime(candidate)) {
            primes[found++] = candidate;
        }
        candidate++;
    }
}
int main(int argc, char **argv) {
    int world_size, rank, num_primes; 
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    srand(time(NULL) + rank);  //ensure different seeds for each process

    if (rank == 0) {
        //process 0 collects input from the user
        printf("Enter the total number of prime numbers to generate: ");
        fflush(stdout);
        scanf("%d", &num_primes);
        int base_count = num_primes / world_size;
        int extra = num_primes % world_size;  

        //distribute prime count to each process
        for (int i = 1; i < world_size; i++) {
            int send_count = base_count + (i < extra ? 1 : 0);
            MPI_Send(&send_count, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
        //generate primes for Process 0
        int self_count = base_count + (0 < extra ? 1 : 0);
        int *self_primes = (int *)malloc(self_count * sizeof(int));
        find_primes(self_primes, self_count);
        //collect results from all processes
        int *all_primes = (int *)malloc(num_primes * sizeof(int));
        int total_found = self_count;
        for (int i = 0; i < self_count; i++) {
            all_primes[i] = self_primes[i];
        }
        //receive prime numbers from other processes
        for (int i = 1; i < world_size; i++) {
            int received_count;
            MPI_Recv(&received_count, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&all_primes[total_found], received_count, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            total_found += received_count;
        }
        //write results to file
        FILE *file = fopen("prime.dat", "w");
        if (file) {
            for (int i = 0; i < total_found; i++) {
                fprintf(file, "%d\n", all_primes[i]);
            }
            fclose(file);
            printf("Prime numbers saved to 'prime.dat'.\n");
        } else {
            printf("Error opening file 'prime.dat' for writing.\n");
        }
        //cleanup memory
        free(self_primes);
        free(all_primes);
    } else {
        //other processes receive their assigned workload
        int task_count;
        MPI_Recv(&task_count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // generate primes for assigned count
        int *primes = (int *)malloc(task_count * sizeof(int));
        find_primes(primes, task_count);
        //send results back to Process 0
        MPI_Send(&task_count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(primes, task_count, MPI_INT, 0, 0, MPI_COMM_WORLD);

        //cleanup memory
        free(primes);
    }
    MPI_Finalize();
    return 0;
}
