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


#define MAX_LINE_SIZE 100
#define PORT 8080
#define TIMEOUT_SECONDS 300

int num_records;
char *input_file_name;
char *output_file_name;
sem_t *sending_sem; // Semaphore for synchronization
sem_t *writing_sem;
int *turn; // Shared variable for indicating the turn


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
    if(count>0)
        records = malloc(sizeof(Record) * count);
    if (records == NULL) {
        printf("Memory allocation failed.\n");
        fclose(file);
        return NULL;
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
void write_csv(int player_id, int g_type, int start_time, int end_time, int court_no, int pl1, int pl2, int pl3, int pl4)
{
    sem_wait(writing_sem); // Wait until it's the process's turn to send
    FILE *file = fopen(output_file_name, "a");
    if (file == NULL)
    {
        printf("Error opening file %s for writing.\n", output_file_name);
        return;
    }
    if (g_type == -1){
        fprintf(file, "%d,%d,%d,\n",pl1,-1,end_time);
    } else if (g_type == 0)
        fprintf(file, "%d,%d,%d,%d,%d,%d,\n", player_id,start_time, end_time, court_no, pl1, pl2);
    else if(g_type==1)
        fprintf(file, "%d,%d,%d,%d,%d,%d,%d,%d,\n",player_id, start_time, end_time, court_no, pl1, pl2, pl3, pl4);

    fclose(file);
    sem_post(writing_sem); // Release semaphore for the next process
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
            printf("Player-ID %d got disconnected.\n",player_id,clientSocket);
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


int main(int argc, char *argv[]) {
    if(argc<3||argc>3){
        printf("Error! Execute as $ ./client <input file name> <output file name");
        exit(-1);
    }
    input_file_name = argv[1];
    output_file_name = argv[2];
    records = read_csv(input_file_name, &num_records);
    printf("Number of Max Clients: %d\n",num_records);
    if (records == NULL) {
        // Handle error
        return 1;
    }
    
    FILE *file = fopen(output_file_name, "w");
    if (file == NULL)
    {
        printf("Error opening file %s for writing.\n", output_file_name);
        return 0;
    }
    fprintf(file, "Player-ID,Game-start-time,Game-end-time,Court-Number,List-of-player-ids,\n");
    fclose(file);
    
    // communicate the number of clients to the server
    sendNumberOfClients();
    // Create shared memory for semaphore and turn
    key_t key_sem = ftok("/tmp", 'S');
    key_t key_turn = ftok("/tmp", 'T');
    int shmid_sem = shmget(key_sem, sizeof(sem_t), IPC_CREAT | 0666);
    int shmid_wrt = shmget(key_sem, sizeof(sem_t), IPC_CREAT | 0666);
    int shmid_turn = shmget(key_turn, sizeof(int), IPC_CREAT | 0666);
    sending_sem = shmat(shmid_sem, NULL, 0);
    writing_sem = shmat(shmid_wrt, NULL, 0);
    turn = shmat(shmid_turn, NULL, 0);
    sem_init(sending_sem, 1, 1); // Initialize semaphore for sending synchronization
    sem_init(writing_sem, 1, 1);
    *turn = 1; // Initialize turn
    sleep(1);
    for (int i = 0; i < num_records; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) { // Child process
            // Calculate sleep duration based on arrival time difference
            int sleep_duration = 0;
            if (i > 0) {
                sleep_duration = records[i].arrival_time - records[i - 1].arrival_time;
            }

            while (*turn != records[i].player_id){
                usleep(10000);
            } // Wait until it's the process's turn to send
            sem_wait(sending_sem); // Wait until it's the process's turn to send

            
            if (sleep_duration > 0 && records[i].player_id>0) {
                printf("Player-ID:%d with arrival time %d sleeps for %d minute(s).\n", records[i].player_id,records[i].arrival_time, sleep_duration);
                sleep(sleep_duration);
            }
            
            printf("Player-ID:%d with arrival time %d is ready to send its data.\n", records[i].player_id,records[i].arrival_time);
            int newSock = sendRequest(records[i].player_id, records[i].arrival_time, records[i].gender, records[i].preference);
            sem_post(sending_sem); // Release semaphore for the next process
            *turn = (records[i].player_id + 1) % num_records; // Update turn
            printf("Player-ID:%d sent its data.\n", records[i].player_id);
            receiveResponse(newSock, records[i].player_id);
        
            exit(EXIT_SUCCESS); // Exit child process after processing the record
        }
    }

    // Wait for all child processes to complete
    for (int i = 0; i < num_records; i++) {
        int status;
        wait(&status);
    }
    free(records);
    sem_destroy(sending_sem); // Destroy semaphore
    shmdt(sending_sem); // Detach shared memory
    shmdt(turn); // Detach shared memory
    return 0;
}