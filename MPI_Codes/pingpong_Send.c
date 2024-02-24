#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>  

#define MAX_PING_PONG_COUNT (100)

int main()
{
    int rank, size, ping_pong_count = 0, dst = 0, status = 0;
    int a=3;
    char **argv = (char **)malloc(3 * sizeof(char *));
    
    // Allocate memory for each string in the array
    for (int i = 0; i < 3; i++) {
        argv[i] = (char *)malloc(10 * sizeof(char)); // Assuming a maximum length of 10 characters for each string
    }
    
    // Copy the strings into the array
    strcpy(argv[0], "_Send");
    strcpy(argv[1], "-n");
    strcpy(argv[2], "2");
    
    MPI_Init(&a,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    while (ping_pong_count < MAX_PING_PONG_COUNT) {
        dst = (rank + 1) % 2;

        if (dst) { 
            ping_pong_count++;
             MPI_Send(&ping_pong_count, 1, MPI_INT, dst, 0, MPI_COMM_WORLD); 
            printf("[%d] Sent message - count: %d, destination: %d\n", rank, ping_pong_count, dst);
            fflush(stdout); 
        } else { 
            sleep(3); // Artificial delay on receiver
            MPI_Status status; // Declare status variable
             MPI_Recv(&ping_pong_count, 1, MPI_INT, dst, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            printf("[%d] Received message (delayed) - count: %d, source: %d\n", rank, ping_pong_count, dst);
            fflush(stdout); 
        }
    } 

    MPI_Finalize(); 
    return 0;
}
