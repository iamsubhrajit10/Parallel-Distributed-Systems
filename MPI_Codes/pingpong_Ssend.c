#include <mpi.h>
#include <stdio.h>
#define MAX_PING_PONG_COUNT (100)

int main(int argc, char *argv[])
{
    int rank, size, ping_pong_count = 0, dst = 0, status = 0;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    while (ping_pong_count < MAX_PING_PONG_COUNT) {
        dst = (rank + 1) % 2;

        if (dst) {  // Sender  
            ping_pong_count++; 
            MPI_Ssend(&ping_pong_count, 1, MPI_INT, dst, 0, MPI_COMM_WORLD);  // Will block  
            printf("[%d] Sent message (Synchronous) - count: %d, destination: %d\n", rank, ping_pong_count, dst);
        } else { // Receiver
            sleep(5); // Introduce a significant delay before receiving
            MPI_Recv(&ping_pong_count, 1, MPI_INT, dst, 0, MPI_COMM_WORLD, &status);  
            printf("[%d] Received message (delayed) - count: %d, source: %d\n", rank, ping_pong_count, dst);
        }
    } 

    MPI_Finalize(); 
    return 0;
}
