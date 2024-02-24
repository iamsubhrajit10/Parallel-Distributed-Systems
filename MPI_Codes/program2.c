#include <stdio.h>
#include <mpi.h>

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 3) {
        printf("This program must be run with 3 processes.\n");
        MPI_Finalize();
        return 1;
    }

    int received_data;
    MPI_Status status;
    MPI_Recv(&received_data, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    printf("Process %d received data: %d\n", rank, received_data);

    MPI_Finalize();
    return 0;
}
