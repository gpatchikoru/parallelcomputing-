#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <mpi.h>

using namespace std;

//A)	 Archimedes rule
double archimedesPi(int sides) {
    return sides * sin(M_PI / sides);
}


//B)	 Leibniz series
double leibnizPi(int totalTerms, int rank, int size) {
    double localSum = 0.0;
    double globalSum = 0.0;

    for (int i = rank; i < totalTerms; i += size) {
        localSum += (i % 2 == 0 ? 1.0 : -1.0) / (2 * i + 1);
    }

    MPI_Reduce(&localSum, &globalSum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        return globalSum * 4;
    }
    return 0;
}

//C)	Walli's rule
double walli(int n, int rank, int size) {
    double stepSize = 1.0 / n;
    double localSum = 0.0;
    double globalSum = 0.0;

    for (int i = rank; i < n; i += size) {
        localSum += sqrt(1 - (i * stepSize) * (i * stepSize));
    }

    MPI_Reduce(&localSum, &globalSum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        return (4 * stepSize) * (0.5 + globalSum);
    }
    return 0;
}


//D)	Monte Carlo 
double monteCarloPi(int totalIterations, int rank, int size) {
    int localCount = 0;
    int globalCount = 0;
    srand(time(0) + rank);

    for (int i = rank; i < totalIterations; i += size) {
        double x = (double)rand() / RAND_MAX;
        double y = (double)rand() / RAND_MAX;
        if (x * x + y * y <= 1) {
            localCount++;
        }
    }

    MPI_Reduce(&localCount, &globalCount, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        return (4.0 * globalCount) / totalIterations;
    }
    return 0;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size, n;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        cout << "Enter the number of iterations/terms: ";
        cin >> n;
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    double piMC = monteCarloPi(n, rank, size);
    double piLeibniz = leibnizPi(n, rank, size);
    double piTrap = walli(n, rank, size);
    double piArch = archimedesPi(n);

    if (rank == 0) {
        cout << "Monte Carlo Pi Estimate: " << piMC << endl;
        cout << "Leibniz Series Pi Estimate: " << piLeibniz << endl;
        cout << "Trapezoidal Rule Pi Estimate: " << piTrap << endl;
        cout << "Archimedes' Pi Estimate: " << piArch << endl;
    }

    MPI_Finalize();
    return 0;
}