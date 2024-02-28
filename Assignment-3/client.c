#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <mpi.h>

#define MAX_LINE_SIZE 100
#define PORT 8080
#define TIMEOUT_SECONDS 300

int num_records;
char *input_file_name = "input.csv";
char *output_file_name = "output.csv";


typedef struct {
    int player_id;
    int arrival_time;
    char gender;
    char preference;
} Record;
Record *records;

Record* read_csv(const char *filename, int *num_records) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Could not open file %s\n", filename);
        return NULL;
    }

    // Count the number of lines in the file
    int count = 0;
    char ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') {
            count++;
        }
    }
    count--;
    Record *records;
    // Allocate memory for the records
    if(count>0){
        records = malloc(sizeof(Record) * count);
        if (records == NULL) {
            printf("Memory allocation failed.\n");
            fclose(file);
            return NULL;
        }
    }

    // Go back to the start of the file
    rewind(file);

    char line[MAX_LINE_SIZE];

    if (fgets(line, sizeof(line), file) == NULL) {
        // Handle error or empty file
        fclose(file);
        free(records);
        return NULL;
    }

    int i = 0;
    while (fgets(line, sizeof(line), file)) {
        Record record;
        char *token;

        // Parse req_timestamp
        token = strtok(line, ",");

        record.player_id = atoi(token);

        // Parse req_type
        token = strtok(NULL, ",");

        record.arrival_time=atoi(token);

        // Parse room_no
        token = strtok(NULL, ",");
        record.gender = token[0];

        // Parse slot_start_time
        token = strtok(NULL, ",");
        record.preference=token[0];

        // Store the record
        records[i] = record;
        i++;
    }

    fclose(file);

    // Return the number of records
    *num_records = count;

    return records;
}

MPI_Win file_lock; // MPI window for file locking

void write_csv(int player_id, int g_type, int start_time, int end_time, int court_no, int pl1, int pl2, int pl3, int pl4)
{
    // Lock the window for exclusive access to the file
    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, 0, file_lock);

    FILE *file = fopen(output_file_name, "a");
    if (file == NULL)
    {
        printf("Error opening file %s for writing.\n", output_file_name);
        MPI_Win_unlock(0, file_lock); // Release the lock on error
        return;
    }

    if (g_type == -1){
        fprintf(file, "%d,%d,%d,\n",pl1,-1,end_time);
    } 
    else if (g_type == 0)
        fprintf(file, "%d,%d,%d,%d,%d,%d,\n", player_id,start_time, end_time, court_no, pl1, pl2);
    else if(g_type==1)
        fprintf(file, "%d,%d,%d,%d,%d,%d,%d,%d,\n",player_id, start_time, end_time, court_no, pl1, pl2, pl3, pl4);

    fclose(file);

    // Release the lock on the file
    MPI_Win_unlock(0, file_lock);
}


int sendRequest(int player_id, int arrival_time, char gender, char preference) {
    int clientSocket;
    struct sockaddr_in serverAddr;

    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    
    if (clientSocket < 0) {
        printf("Error creating socket.\n");
        exit(1);
    }
    // fcntl(clientSocket, F_SETFL, O_NONBLOCK);

    // Set up server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        if (errno != EINPROGRESS) {
            printf("Error connecting to server.\n");
            close(clientSocket);
            exit(1);
        }
    }

    // Wait for connection to complete or timeout
    fd_set write_fds;
    FD_ZERO(&write_fds);
    FD_SET(clientSocket, &write_fds);

    struct timeval timeout;
    timeout.tv_sec = TIMEOUT_SECONDS;
    timeout.tv_usec = 0;

    int selectResult = select(clientSocket + 1, NULL, &write_fds, NULL, &timeout);
    if (selectResult < 0) {
        printf("Error in select.\n");
        close(clientSocket);
        exit(1);
    } else if (selectResult == 0) {
        printf("Connection timeout.\n");
        close(clientSocket);
        exit(1);
    }
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "%d %d %c %c", player_id, arrival_time, gender, preference);
    int sendStatus = send(clientSocket, buffer, strlen(buffer) + 1, 0);
    if (sendStatus == -1) {
        perror("Error sending data");
        // Handle the error or exit the client
    }
    return clientSocket;
}



void receiveResponse(int clientSocket, int player_id){
    char buffer[1024];
    bzero(buffer, sizeof(buffer));
    ssize_t recvStatus = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (recvStatus <= 0) {
        // Either an error or the client disconnected
        if (recvStatus == 0) {
            printf("Player-ID %d got disconnected.\n",player_id);
        } else {
            perror("Error in receiving data");
        }
    } else {
        printf("Response from player-ID %d: %s\n", player_id, buffer);
        if(strcmp(buffer, "All client requests received by server.") == 0){
            close(clientSocket);
            return;
        }
        int g_type, game_start_time, game_end_time,court_no,pl1,pl2,pl3,pl4;
        sscanf(buffer, "%d %d %d %d %d %d %d %d", &g_type,&game_start_time,&game_end_time,&court_no,&pl1,&pl2,&pl3,&pl4);
        
        if (g_type == 0 || g_type == 1 || g_type == -1){
            write_csv(player_id,g_type,game_start_time,game_end_time,court_no,pl1,pl2,pl3,pl4);
        } 
    }
    // Close the socket
    close(clientSocket);
}
void sendNumberOfClients(){
    int clientSocket;
    struct sockaddr_in serverAddr;

    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        printf("Error creating socket.\n");
        exit(1);
    }

    // Set up server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT-1);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) != 0) {
        printf("Error connecting to server.\n");
        close(clientSocket);
        exit(1);
    }

    // Prepare and send request
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "%d",num_records);

    int sendStatus = send(clientSocket, buffer, strlen(buffer) + 1, 0);  // Use strlen(buffer) + 1 instead of sizeof(buffer)
    if (sendStatus == -1) {
        perror("Error sending data");
    // Handle the error or exit the client
    }
    // Close the socket
    close(clientSocket);
}


int main(int argc, char** argv) {
    MPI_Init(NULL, NULL);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    Record *records;
    // Read the input file
    records = read_csv("input.csv", &num_records);
    if (records == NULL) {
        printf("Error reading input file.\n");
        MPI_Finalize();
        return 1;
    }

    

    // Create a window for file locking
    MPI_Win_create(NULL, 0, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &file_lock);
    

    // Initialize records here
    
    int current_arrival_time = records[0].arrival_time;
    if (world_rank == 0) { // Master process
        // Send the number of clients to the server
        sendNumberOfClients();
        for (int i = 0; i < num_records; i++) {
            // printf("Current Time: %d, Pid: %d, Pid.arrival time: %d\n", current_arrival_time, records[i].player_id, records[i].arrival_time);
            // if (records[i].arrival_time > current_arrival_time) {
            //     sleep(records[i].arrival_time - current_arrival_time);
            // }
            //Broadcast the current arrival time to all processes
            current_arrival_time = records[i].arrival_time;
            MPI_Bcast(&current_arrival_time, 1, MPI_INT, 0, MPI_COMM_WORLD);
            sleep(1); // Add a brief pause for synchronization
        }
    } else { // Worker processes
        sleep(1); // Add a brief pause for synchronization
        int received_arrival_time;
        // Receive the current arrival time from the master process
        MPI_Bcast(&received_arrival_time, 1, MPI_INT, 0, MPI_COMM_WORLD);
        int record_no = world_rank - 1;
        // printf("World Rank: %d, Record No: %d, Arrival Time: %d, Initial Arrival Time: %d\n", world_rank, record_no, records[record_no].arrival_time, received_arrival_time);
        if (records[record_no].arrival_time > received_arrival_time) {
            // Wait until the arrival time matches
            // sleep(records[record_no].arrival_time - received_arrival_time);
            // MPI_Bcast(&received_arrival_time, 1, MPI_INT, 0, MPI_COMM_WORLD);
            // printf("World Rank: %d, Record No: %d, Arrival Time: %d, Received Arrival Time: %d\n", world_rank, record_no, records[record_no].arrival_time, received_arrival_time);
            sleep(records[record_no].arrival_time - received_arrival_time);
        }
        printf("Player-ID:%d with arrival time %d is ready to send its data.\n", records[record_no].player_id, records[record_no].arrival_time);
        int newSock = sendRequest(records[record_no].player_id, records[record_no].arrival_time, records[record_no].gender, records[record_no].preference);
        printf("Player-ID:%d sent its data.\n", records[record_no].player_id);
        receiveResponse(newSock, records[record_no].player_id);
        MPI_Barrier(MPI_COMM_WORLD);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Win_free(&file_lock);
    MPI_Finalize();
    free(records);
    return 0;
}
