#include <mpi.h>
#include <stdio.h>
#define MAX_PING_PONG_COUNT (100)

int main(int argc, char *argv[]) {
    int rank, size, ping_pong_count = 0, dst = 0;
    MPI_Request request;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    while (ping_pong_count < MAX_PING_PONG_COUNT) {
        dst = (rank + 1) % 2;

        if (ping_pong_count % 2) {
            ping_pong_count++;
            MPI_Irsend(&ping_pong_count, 1, MPI_INT, dst, 0, MPI_COMM_WORLD, &request);
            printf("[%d] MPI_Irsend initiated\n", rank);

            // Potentially do other work while the message is being sent

            MPI_Wait(&request, &status); // Wait for send completion
            printf("[%d] Send completed\n", rank);
        } else {
            MPI_Recv(&ping_pong_count, 1, MPI_INT, dst, 0, MPI_COMM_WORLD, &status);
            printf("[%d] Received message - count: %d, source: %d\n", rank, ping_pong_count, status.MPI_SOURCE);
        }
    } 

    MPI_Finalize(); 
    return 0;
}
