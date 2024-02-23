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

        if (dst) {  // Sender  
            ping_pong_count++; 
            MPI_Request req;
            MPI_Issend(&ping_pong_count, 1, MPI_INT, dst, 0, MPI_COMM_WORLD, &req); 
            printf("[%d] Sent message - count: %d, destination: %d\n", rank, ping_pong_count, dst);

            // ... potentially other work...

            int flag = 0;
            double start_time = MPI_Wtime();
            while (!flag && MPI_Wtime() - start_time < 3.0) { 
                MPI_Test(&req, &flag, MPI_STATUS_IGNORE); 
            }
            if (!flag) {
                printf("[%d] Timeout! Send to %d likely incomplete\n", rank, dst);
            } 
        } else { // Receiver
            sleep(5); // Delay before receiving
            MPI_Recv(&ping_pong_count, 1, MPI_INT, dst, 0, MPI_COMM_WORLD, &status);
            printf("[%d] Received message (delayed) - count: %d, source: %d\n", rank, ping_pong_count, dst);
        }

    } 

    MPI_Finalize(); 
    return 0;
}
