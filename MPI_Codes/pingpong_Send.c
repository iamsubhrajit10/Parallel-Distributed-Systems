#include <mpi.h>
#include <stdio.h>
#include <unistd.h>  

#define MAX_PING_PONG_COUNT (100)

int main(int argc, char *argv[])
{
    int rank, size, ping_pong_count = 0, dst = 0, status = 0;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    while (ping_pong_count < MAX_PING_PONG_COUNT) {
        dst = (rank + 1) % 2;

        if (ping_pong_count % 2) { 
            ping_pong_count++;
             MPI_Send(&ping_pong_count, 1, MPI_INT, dst, MPI_ANY_TAG, MPI_COMM_WORLD); 
            printf("[%d] Sent message - count: %d, destination: %d\n", rank, ping_pong_count, dst);
            fflush(stdout); 
        } else { 
            // sleep(3); // Artificial delay on receiver
            MPI_Status status; // Declare status variable
             MPI_Recv(&ping_pong_count, 1, MPI_INT, dst, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            printf("[%d] Received message (delayed) - count: %d, source: %d\n", rank, ping_pong_count, dst);
            fflush(stdout); 
        }
    } 

    MPI_Finalize(); 
    return 0;
}
