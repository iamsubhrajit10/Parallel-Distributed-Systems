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

        // Buffer setup (similar to Scenario 3)
        int buffer_size = ...;
        char buffer[buffer_size]; 
        MPI_Buffer_attach(buffer, buffer_size);

        if (ping_pong_count % 2) { // Sender  
            ping_pong_count++; 
            printf("[%d] Before send, buffer[0]: %c\n", rank, buffer[0]);
            MPI_Request req;
            MPI_Ibsend(&ping_pong_count, 1, MPI_INT, dst, 0, MPI_COMM_WORLD, &req); 
            buffer[0] = 'X'; // Likely safe modification
            printf("[%d] After send (likely safe), buffer[0]: %c\n", rank, buffer[0]); 
            MPI_Wait(&req, MPI_STATUS_IGNORE); 
        } else { // Receiver
            MPI_Recv(&ping_pong_count, 1, MPI_INT, dst, 0, MPI_COMM_WORLD, &status);
            printf("[%d] Received message - count: %d, source: %d\n", rank, ping_pong_count, dst);
        } 

        MPI_Buffer_detach(&buffer, &buffer_size); 

    } 

    MPI_Finalize(); 
    return 0;
}
