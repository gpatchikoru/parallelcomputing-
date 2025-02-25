## Project 2
This project consists of two MPI-based parallel programs designed for prime number computations:

1. prog2a.c - checks the number of prime numbers in a dataset of 100,000 randomly generated integers.
2. prog2b.c - generates a specified number of prime numbers and saves them to a file (`prime.dat`).

both programs leverage MPI (Message Passing Interface)
for distributed computing to improve efficiency and performance.

## Check MPI Installation
to verify if MPI is installed run:
which mpicc
which mpirun

## Load the MPI if not loaded 
module load MPI

## compile all programs
make

## cat - View all prime numbers
cat prime.dat  


