#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#define MAX_PING_PONG_COUNT (100)

int main(int argc, char *argv[])
{
    int rank, size, ping_pong_count = 0, dst = 0;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    while (ping_pong_count < MAX_PING_PONG_COUNT) {
        dst = (rank + 1) % 2;

        int buffer_size = 1024;
        char buffer[buffer_size]; 
        MPI_Buffer_attach(buffer, buffer_size); 

        if (dst) { // Sender
            ping_pong_count++;
            buffer[0]='Y';
            printf("[%d] Before send, buffer[0]: %c\n", rank, buffer[0]);
            MPI_Bsend(&ping_pong_count, 1, MPI_INT, dst, 0, MPI_COMM_WORLD);  
            buffer[0] = 'X'; // Potentially unsafe
            printf("[%d] After send (may be unsafe), buffer[0]: %c\n", rank, buffer[0]);
        } else { // Receiver
            sleep(3);
            MPI_Recv(&ping_pong_count, 1, MPI_INT, dst, 0, MPI_COMM_WORLD, &status);
            printf("[%d] Received message - count: %d, source: %d\n", rank, ping_pong_count, dst);
        } 
        MPI_Buffer_detach(&buffer, &buffer_size); 
    } 

    MPI_Finalize(); 
    return 0;
}
