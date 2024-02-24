#include <stdio.h>
#include <mpi.h>

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    printf("Program1 size: %d\n",size);
    if (size != 5) {
        printf("This program must be run with 2 processes.\n");
        MPI_Finalize();
        return 1;
    }

    int data = rank + 1;
    MPI_Send(&data, 1, MPI_INT, (rank + 1) % size, 0, MPI_COMM_WORLD);

    MPI_Finalize();
    return 0;
}
