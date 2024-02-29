/*
Subhrajit (23210106)    Tennis Game Requests using Multi-threaded Concurrent Programming
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <limits.h>
#include <omp.h>
#include <fcntl.h>
#include <errno.h>
#include <stdatomic.h>


int NUM_CLIENTS;
#define PORT 8080
#define NO_OF_COURTS 4
#define OCCUPIED 1
#define UNOCCUPIED 0
#define INVALID_REQUEST -4
#define MEN_TIME_DURATION_DOUBLES 15
#define WOMEN_TIME_DURATION_DOUBLES 10
#define MEN_TIME_DURATION_SINGLES 10
#define WOMEN_TIME_DURATION_SINGLES 5
#define TIME_OUT_TIME 30

/* Thread Array */
int clientCount = 0;
int server_time_stamp = 1;
int global_server_time_stamp=1;
int sockfd;
// Declare an atomic variable to indicate if all matches are done
atomic_int ALL_MATCHES_DONE = 0;

size_t waitingSinglesPlayerCount = 0, waitingDoublesPlayerCount = 0;
size_t WAITING_SINGLES_POOL_SIZE = 1000;
size_t WAITING_DOUBLES_POOL_SIZE = 1000;

struct courts
{
    int *court;
    int *last_game_start_time;
    int *last_game_end_time;
    int *game_running;
    int **player_ids;
    int **player_sockets;
} courts_status;
/* Function to initialize the room_status data structure */
void init_courts()
{
    courts_status.court = calloc(NO_OF_COURTS, sizeof(int *));
    if (courts_status.court == NULL)
    {
        printf("\nMemory allocation for courts failed!\n");
        exit(-1);
    }

    courts_status.last_game_start_time = calloc(NO_OF_COURTS, sizeof(int *));
    if (courts_status.last_game_start_time == NULL)
    {
        printf("\nMemory allocation for last_game_start_time failed!\n");
        exit(-1);
    }
    courts_status.last_game_end_time = calloc(NO_OF_COURTS, sizeof(int *));
    if (courts_status.last_game_end_time == NULL)
    {
        printf("\nMemory allocation for last_game_start_time failed!\n");
        exit(-1);
    }
    courts_status.game_running = calloc(NO_OF_COURTS, sizeof(int *));
    courts_status.player_ids = calloc(NO_OF_COURTS, sizeof(int *));
    for (int i=0;i<NO_OF_COURTS;i++){
        courts_status.player_ids[i] = calloc (4, sizeof(int));
    }
    courts_status.player_sockets = calloc(NO_OF_COURTS, sizeof(int *));
    for (int i=0;i<NO_OF_COURTS;i++){
        courts_status.player_sockets[i] = calloc (4, sizeof(int));
    }

}


// Player structure
struct Player
{
    int player_id;
    int arrival_time;
    char gender;
    char preference;
    int socketNumber;
    int max_permissible_time;
};
struct Response
{
    int court_no;
    int game_type;
    struct Player *matchedPlayer;
};
struct Player *singlesPlayerQueue;
struct Player *doublesPlayerQueue;


// Structure to represent a client request
struct Request
{
    int socket;
    struct Request *next;
};

// Linked list to manage the request queue
struct RequestQueue
{
    struct Request *front;
    struct Request *rear;
};
// Global request queue
struct RequestQueue requestQueue;

void init_queues()
{
    // Initialize singlesPlayerQueue with dynamically allocated memory
    singlesPlayerQueue = (struct Player *)malloc(WAITING_SINGLES_POOL_SIZE * sizeof(struct Player));
    if (singlesPlayerQueue == NULL)
    {
        perror("Error allocating memory for singlesPlayerQueue");
        // Handle memory allocation failure
    }
    // Initialize doublesPlayerQueue with dynamically allocated memory
    doublesPlayerQueue = (struct Player *)malloc(WAITING_DOUBLES_POOL_SIZE * sizeof(struct Player));
    if (doublesPlayerQueue == NULL)
    {
        perror("Error allocating memory for doublesPlayerQueue");
        // Handle memory allocation failure
    }
}
int recieveNoOfClients(){
    int sockfd, newSocket;
    struct sockaddr_in serverAddr;
    struct sockaddr_in newAddr;
    socklen_t addr_size;
    int num_clients;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("[-]Error in connection.\n");
        exit(1);
    }
    printf("[+]Server Socket is created.\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT-1);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("[-]Error in binding.\n");
        exit(1);
    }

    if (listen(sockfd, 10) == 0) {
        printf("[+]Listening....\n");
    } else {
        printf("[-]Error in binding.\n");
    }

    addr_size = sizeof(newAddr);
    newSocket = accept(sockfd, (struct sockaddr *)&newAddr, &addr_size);
    if (newSocket < 0) {
        perror("Accept Failed");
        exit(1);
    }
    printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

    // Receive buffer data
    char buffer[1024] = {0};
    int valread = read(newSocket, buffer, 1024);
    printf("Received: %s\n", buffer);
    
    sscanf(buffer, "%d",&num_clients);
    close(newSocket);
    close(sockfd);
    return num_clients;
}
// Initialize the request queue
void initRequestQueue(struct RequestQueue *queue)
{
    queue->front = NULL;
    queue->rear = NULL;
}

// Enqueue a new request
void enqueueRequest(struct RequestQueue *queue, int socket)
{
    struct Request *newRequest = (struct Request *)malloc(sizeof(struct Request));
    newRequest->socket = socket;
    newRequest->next = NULL;

    if (queue->rear == NULL)
    {
        queue->front = newRequest;
        queue->rear = newRequest;
    }
    else
    {
        queue->rear->next = newRequest;
        queue->rear = newRequest;
    }
}

// Dequeue a request
int dequeueRequest(struct RequestQueue *queue)
{
    if (queue->front == NULL)
    {
        return -1; // Queue is empty
    }

    int socket = queue->front->socket;
    struct Request *temp = queue->front;

    if (queue->front == queue->rear)
    {
        queue->front = NULL;
        queue->rear = NULL;
    }
    else
    {
        queue->front = queue->front->next;
    }

    free(temp);
    return socket;
}
void checkTimeOutPlayers(int time_stamp){
    for(int i=0;i<waitingSinglesPlayerCount;i++){
        
        if(singlesPlayerQueue[i].max_permissible_time<time_stamp){
            char failureMsg[100];
            snprintf(failureMsg, sizeof(failureMsg),"%d %d %d %d %d %d %d %d",-1,time_stamp,time_stamp,-1,singlesPlayerQueue[i].player_id,-1,-1,-1);
            printf("Player-ID %d got time-out, arrival-time %d, max_perm_time %d\n",singlesPlayerQueue[i].player_id,singlesPlayerQueue[i].arrival_time,singlesPlayerQueue[i].max_permissible_time);
            send(singlesPlayerQueue[i].socketNumber, failureMsg, strlen(failureMsg) + 1, 0);
            for (int j = i; i < waitingSinglesPlayerCount - 1; i++){
                singlesPlayerQueue[i] = singlesPlayerQueue[i + 1];
            }
            waitingSinglesPlayerCount--; 
            if(singlesPlayerQueue[i].preference=='b' || singlesPlayerQueue[i].preference=='B'){
                 if (waitingDoublesPlayerCount > 0)
                    {
                        printf("\nID: %d\n",singlesPlayerQueue[i].player_id);
                        int find_i = -1;
                        int j;
                        for (; j < waitingDoublesPlayerCount; j++)
                        {
                            if (doublesPlayerQueue[j].player_id == doublesPlayerQueue[j].player_id)
                            {
                                find_i = i;
                                break;
                            }
                        }
                        // singlesPlayerQueue[find_i]=NULL;
                        if (find_i != -1)
                        {
                            for (j = find_i; j < waitingDoublesPlayerCount - 1; j++)
                            {
                                doublesPlayerQueue[j] = doublesPlayerQueue[j + 1];
                            }
                            waitingDoublesPlayerCount--;
                        }
                    }
            }        
        }
    }
    for(int i=0;i<waitingDoublesPlayerCount;i++){
        if(doublesPlayerQueue[i].max_permissible_time<time_stamp){
            char failureMsg[100];
            snprintf(failureMsg, sizeof(failureMsg),"%d %d %d %d %d %d %d %d",-1,time_stamp,time_stamp,-1,doublesPlayerQueue[i].player_id,-1,-1,-1);
            send(doublesPlayerQueue[i].socketNumber, failureMsg, strlen(failureMsg) + 1, 0);
            printf("Player-ID %d got time-out, arrival-time %d, max_perm_time %d\n",doublesPlayerQueue[i].player_id,doublesPlayerQueue[i].arrival_time,doublesPlayerQueue[i].max_permissible_time);
            for (int j = i; i < waitingDoublesPlayerCount - 1; i++){
                doublesPlayerQueue[i] = doublesPlayerQueue[i + 1];
            }
            waitingDoublesPlayerCount--; 
            
        }
    }
}



void checkMatchCompletions(int time_stamp){
    checkTimeOutPlayers(time_stamp);
    // Create an array to store indices
    int court_indices[NO_OF_COURTS];
    
    // Initialize the indices array
    for (int i = 0; i < NO_OF_COURTS; i++) {
        court_indices[i] = i;
    }

    // Sort the indices array based on ending time (assuming courts_status is an array of CourtStatus)
    for (int i = 0; i < NO_OF_COURTS - 1; i++) {
        for (int j = 0; j < NO_OF_COURTS - i - 1; j++) {
            if (courts_status.last_game_end_time[court_indices[j]] > courts_status.last_game_end_time[court_indices[j + 1]]) {
                // Swap indices if ending time is greater
                int temp = court_indices[j];
                court_indices[j] = court_indices[j + 1];
                court_indices[j + 1] = temp;
            }
        }
    }
    
     for (int i = 0; i < NO_OF_COURTS; i++)
    {
        if ((time_stamp + 1 > (courts_status.last_game_end_time[court_indices[i]])) && (courts_status.court[court_indices[i]] == OCCUPIED))
        {
            
            int g_type, pl1, pl2, pl3, pl4;

            if (courts_status.game_running[court_indices[i]] == 0){
                    // assigning winner for singles match
                    srand(time(NULL));
                    int winner = rand() % 2;
                    char msg[100];
                    snprintf(msg, sizeof(msg),"%d %d %d %d %d %d %d %d %d",0,courts_status.last_game_start_time[court_indices[i]],courts_status.last_game_end_time[court_indices[i]],court_indices[i]+1,courts_status.player_ids[court_indices[i]][0],courts_status.player_ids[court_indices[i]][1],-1,-1,winner+1);
                    send(courts_status.player_sockets[court_indices[i]][0], msg, strlen(msg) + 1, 0);
                    send(courts_status.player_sockets[court_indices[i]][1], msg, strlen(msg) + 1, 0);
            } else if (courts_status.game_running[court_indices[i]] == 1){
                    // assigning winner for doubles match
                    srand(time(NULL));
                    int winner = rand() % 4;
                    char msg[100];
                    snprintf(msg, sizeof(msg),"%d %d %d %d %d %d %d %d %d",1,courts_status.last_game_start_time[court_indices[i]],courts_status.last_game_end_time[court_indices[i]],court_indices[i]+1,courts_status.player_ids[court_indices[i]][0],courts_status.player_ids[court_indices[i]][1],courts_status.player_ids[court_indices[i]][2],courts_status.player_ids[court_indices[i]][3],winner);
                    send(courts_status.player_sockets[court_indices[i]][0], msg, strlen(msg) + 1, 0);
                    send(courts_status.player_sockets[court_indices[i]][1], msg, strlen(msg) + 1, 0);
                    send(courts_status.player_sockets[court_indices[i]][2], msg, strlen(msg) + 1, 0);
                    send(courts_status.player_sockets[court_indices[i]][3], msg, strlen(msg) + 1, 0);
            }
            courts_status.court[court_indices[i]] = UNOCCUPIED;
        }
        usleep(10000);
    }
}

struct Response *canFormSinglesMatch(struct Player *nPlayer)
{
    struct Player newPlayer = *nPlayer;
    int availableCourtNumber = -1;
    struct Response *response = (struct Response *)malloc(sizeof(struct Response));
    response->court_no = -1;
    response->game_type = -1;
    int chance = 0;
    response->matchedPlayer = (struct Player *)malloc(sizeof(struct Player));
    if (waitingSinglesPlayerCount > 0)
    {
        for (int court_no = 0; court_no < NO_OF_COURTS; court_no++)
        {
            if (courts_status.court[court_no] == UNOCCUPIED)
            {
                // Sort the existing players based on the FCFS policy
                for (int i = 0; i < waitingSinglesPlayerCount; i++)
                {
                    for (int j = i + 1; j < waitingSinglesPlayerCount; j++)
                    {
                        if (singlesPlayerQueue[i].arrival_time > singlesPlayerQueue[j].arrival_time)
                        {
                            // Swap players to maintain FCFS order
                            struct Player temp = singlesPlayerQueue[i];
                            singlesPlayerQueue[i] = singlesPlayerQueue[j];
                            singlesPlayerQueue[j] = temp;
                        }
                    }
                }

                struct Player matchedPlayer = singlesPlayerQueue[0]; // matched player

                size_t i = 0;
                if (matchedPlayer.preference == 'B' || matchedPlayer.preference == 'b')
                {
                    if (waitingDoublesPlayerCount > 0)
                    {
                        int find_i = -1;
                        for (; i < waitingDoublesPlayerCount; i++)
                        {
                            if (matchedPlayer.player_id == doublesPlayerQueue[i].player_id)
                            {
                                find_i = i;
                                break;
                            }
                        }
                        // singlesPlayerQueue[find_i]=NULL;
                        if (find_i != -1)
                        {
                            for (i = find_i; i < waitingDoublesPlayerCount - 1; i++)
                            {
                                doublesPlayerQueue[i] = doublesPlayerQueue[i + 1];
                            }
                            waitingDoublesPlayerCount--;
                        }
                        // singlesPlayerQueue[i]=NULL;
                    }
                }
                // Remove the matched player from the queue

                for (i = 0; i < waitingSinglesPlayerCount - 1; i++)
                {
                    singlesPlayerQueue[i] = singlesPlayerQueue[i + 1];
                }
                // singlesPlayerQueue[i]=NULL;
                waitingSinglesPlayerCount--;
                courts_status.last_game_start_time[court_no] = newPlayer.arrival_time;
                if (newPlayer.gender == 'M')
                {
                    courts_status.last_game_end_time[court_no] = newPlayer.arrival_time + MEN_TIME_DURATION_SINGLES;
                }
                else if (matchedPlayer.gender == 'M')
                {
                    courts_status.last_game_end_time[court_no] = newPlayer.arrival_time + MEN_TIME_DURATION_SINGLES;
                }
                else
                {
                    courts_status.last_game_end_time[court_no] = newPlayer.arrival_time + WOMEN_TIME_DURATION_SINGLES;
                }
                courts_status.court[court_no] = OCCUPIED;
                response->game_type = 0;
                response->matchedPlayer[0] = matchedPlayer;
                availableCourtNumber = court_no;
                chance = 1;
                response->court_no = availableCourtNumber;
                courts_status.game_running[court_no] = 0;
                courts_status.player_sockets[court_no][0] = matchedPlayer.socketNumber;
                courts_status.player_sockets[court_no][1] = newPlayer.socketNumber;
                courts_status.player_ids[court_no][0]=matchedPlayer.player_id;
                courts_status.player_ids[court_no][1]=newPlayer.player_id;
                courts_status.player_ids[court_no][2]=-1;
                courts_status.player_ids[court_no][3]=-1;
                break;
            }
        }
        if (chance == 0)
        {
            waitingSinglesPlayerCount++;
            if (waitingSinglesPlayerCount > WAITING_SINGLES_POOL_SIZE)
            {
                WAITING_SINGLES_POOL_SIZE *= 2;
                struct Player *temp = (struct Player *)realloc(singlesPlayerQueue, WAITING_SINGLES_POOL_SIZE * sizeof(struct Player));
                if (temp == NULL)
                {
                    perror("Singles Player Memory Reallocation Failure.\n");
                }
                else
                {
                    // realloc successful, newArr may point to the same or a new memory block
                    singlesPlayerQueue = temp; // update the array pointer if needed
                }
            }
            singlesPlayerQueue[waitingSinglesPlayerCount - 1] = newPlayer;
            if (newPlayer.preference == 'B' || newPlayer.preference == 'b')
            {
                waitingDoublesPlayerCount++;
                if (waitingDoublesPlayerCount > WAITING_DOUBLES_POOL_SIZE)
                {
                    WAITING_DOUBLES_POOL_SIZE *= 2;
                    struct Player *temp = (struct Player *)realloc(doublesPlayerQueue, WAITING_DOUBLES_POOL_SIZE * sizeof(struct Player));
                    if (temp == NULL)
                    {
                        perror("Singles Player Memory Reallocation Failure.\n");
                    }
                    else
                    {
                        // realloc successful, newArr may point to the same or a new memory block
                        doublesPlayerQueue = temp; // update the array pointer if needed
                    }
                }
                doublesPlayerQueue[waitingDoublesPlayerCount - 1] = newPlayer;
            }

            // response->matchedPlayer=NULL;
        }
    }
    else
    {
        waitingSinglesPlayerCount++;
        if (waitingSinglesPlayerCount > WAITING_SINGLES_POOL_SIZE)
        {
            WAITING_SINGLES_POOL_SIZE *= 2;
            struct Player *temp = (struct Player *)realloc(singlesPlayerQueue, WAITING_SINGLES_POOL_SIZE * sizeof(struct Player));
            if (temp == NULL)
            {
                perror("Singles Player Memory Reallocation Failure.\n");
            }
            else
            {
                // realloc successful, newArr may point to the same or a new memory block
                singlesPlayerQueue = temp; // update the array pointer if needed
            }
        }
        singlesPlayerQueue[waitingSinglesPlayerCount - 1] = newPlayer;
        // response->matchedPlayer=NULL;
    }
    response->court_no = availableCourtNumber;
    return response;
}

struct Response *canFormDoublesMatch(struct Player *nPlayer)
{
    struct Player newPlayer = *nPlayer;
    int availableCourtNumber = -1;
    struct Response *response = (struct Response *)malloc(sizeof(struct Response));
    response->court_no = -1;
    response->game_type = -1;
    int chance = 0;
    response->matchedPlayer = (struct Player *)malloc(3 * sizeof(struct Player));
    if (waitingDoublesPlayerCount > 2)
    {
        for (int court_no = 0; court_no < NO_OF_COURTS; court_no++)
        {
            if (courts_status.court[court_no] == UNOCCUPIED)
            {
                chance = 1;
                // Sort the existing players based on the FCFS policy
                for (int i = 0; i < waitingDoublesPlayerCount; i++)
                {
                    for (int j = i + 1; j < waitingDoublesPlayerCount; j++)
                    {
                        if (doublesPlayerQueue[i].arrival_time > doublesPlayerQueue[j].arrival_time)
                        {
                            // Swap players to maintain FCFS order
                            struct Player temp = doublesPlayerQueue[i];
                            doublesPlayerQueue[i] = doublesPlayerQueue[j];
                            doublesPlayerQueue[j] = temp;
                        }
                    }
                }

                struct Player matchedPlayer_0 = doublesPlayerQueue[0]; // matched player
                struct Player matchedPlayer_1 = doublesPlayerQueue[1];
                struct Player matchedPlayer_2 = doublesPlayerQueue[2];
                // Remove the matched player from the queue
                size_t i = 0;

                if (matchedPlayer_0.preference == 'B' || matchedPlayer_0.preference == 'b')
                {
                    if (waitingSinglesPlayerCount > 0)
                    {
                        int find_i = -1;
                        for (; i < waitingSinglesPlayerCount; i++)
                        {
                            if (matchedPlayer_0.player_id == singlesPlayerQueue[i].player_id)
                            {
                                find_i = i;
                                break;
                            }
                        }
                        // singlesPlayerQueue[find_i]=NULL;
                        if (find_i != -1)
                        {
                            for (i = find_i; i < waitingSinglesPlayerCount - 1; i++)
                            {
                                singlesPlayerQueue[i] = singlesPlayerQueue[i + 1];
                            }
                            waitingSinglesPlayerCount--;
                        }
                    }
                }
                i = 0;
                if (matchedPlayer_1.preference == 'B' || matchedPlayer_1.preference == 'b')
                {
                    if (waitingSinglesPlayerCount > 0)
                    {
                        int find_i = -1;
                        for (; i < waitingSinglesPlayerCount; i++)
                        {
                            if (matchedPlayer_1.player_id == singlesPlayerQueue[i].player_id)
                            {
                                find_i = i;
                                break;
                            }
                        }
                        // singlesPlayerQueue[find_i]=NULL;
                        if (find_i != -1)
                        {
                            for (i = find_i; i < waitingSinglesPlayerCount - 1; i++)
                            {
                                singlesPlayerQueue[i] = singlesPlayerQueue[i + 1];
                            }
                            waitingSinglesPlayerCount--;
                        }
                        // singlesPlayerQueue[i]=NULL;
                    }
                }
                i = 0;
                if (matchedPlayer_2.preference == 'B' || matchedPlayer_2.preference == 'b')
                {
                    if (waitingSinglesPlayerCount > 0)
                    {
                        int find_i = -1;
                        for (; i < waitingSinglesPlayerCount; i++)
                        {
                            if (matchedPlayer_2.player_id == singlesPlayerQueue[i].player_id)
                            {
                                find_i = i;
                                break;
                            }
                        }
                        // singlesPlayerQueue[find_i]=NULL;
                        if (find_i != -1)
                        {
                            for (i = find_i; i < waitingSinglesPlayerCount - 1; i++)
                            {
                                singlesPlayerQueue[i] = singlesPlayerQueue[i + 1];
                            }
                            waitingSinglesPlayerCount--;
                        }
                    }
                }

                for (int k = 0; k < 3; k++)
                {
                    for (i = 0; i < waitingDoublesPlayerCount - 1; i++)
                    {
                        doublesPlayerQueue[i] = doublesPlayerQueue[i + 1];
                    }
                    waitingDoublesPlayerCount--;
                }
                courts_status.last_game_start_time[court_no] = newPlayer.arrival_time;
                if (newPlayer.gender == 'M')
                {
                    courts_status.last_game_end_time[court_no] = newPlayer.arrival_time + MEN_TIME_DURATION_DOUBLES;
                }
                else if (matchedPlayer_0.gender == 'M' || matchedPlayer_1.gender == 'M' || matchedPlayer_2.gender == 'M')
                {
                    courts_status.last_game_end_time[court_no] = newPlayer.arrival_time + MEN_TIME_DURATION_DOUBLES;
                }
                else
                {
                    courts_status.last_game_end_time[court_no] = newPlayer.arrival_time + WOMEN_TIME_DURATION_DOUBLES;
                }
                courts_status.court[court_no] = OCCUPIED;
                response->game_type = 1;
                response->matchedPlayer[0] = matchedPlayer_0;
                response->matchedPlayer[1] = matchedPlayer_1;
                response->matchedPlayer[2] = matchedPlayer_2;
                courts_status.game_running[court_no] = 1;
                courts_status.player_sockets[court_no][0] = matchedPlayer_0.socketNumber;
                courts_status.player_sockets[court_no][1] = matchedPlayer_1.socketNumber;
                courts_status.player_sockets[court_no][2] = matchedPlayer_2.socketNumber;
                courts_status.player_sockets[court_no][3] = newPlayer.socketNumber;
   
                courts_status.player_ids[court_no][0]=matchedPlayer_0.player_id;
                courts_status.player_ids[court_no][1]=matchedPlayer_1.player_id;
                courts_status.player_ids[court_no][2]=matchedPlayer_2.player_id;
                courts_status.player_ids[court_no][3]=newPlayer.player_id;
                
                availableCourtNumber = court_no;
                response->court_no = availableCourtNumber;

                break;
            }
        }
        if (chance == 0)
        {
            waitingDoublesPlayerCount++;
            if (waitingDoublesPlayerCount > WAITING_DOUBLES_POOL_SIZE)
            {
                WAITING_DOUBLES_POOL_SIZE *= 2;
                struct Player *temp = (struct Player *)realloc(doublesPlayerQueue, WAITING_DOUBLES_POOL_SIZE * sizeof(struct Player));
                if (temp == NULL)
                {
                    perror("Doubles Player Memory Reallocation Failure.\n");
                }
                else
                {
                    // realloc successful, newArr may point to the same or a new memory block
                    doublesPlayerQueue = temp; // update the array pointer if needed
                }
            }
            doublesPlayerQueue[waitingDoublesPlayerCount - 1] = newPlayer;
            if (newPlayer.preference == 'B' || newPlayer.preference == 'b')
            {
                waitingSinglesPlayerCount++;
                if (waitingSinglesPlayerCount > WAITING_SINGLES_POOL_SIZE)
                {
                    WAITING_SINGLES_POOL_SIZE *= 2;
                    struct Player *temp = (struct Player *)realloc(singlesPlayerQueue, WAITING_SINGLES_POOL_SIZE * sizeof(struct Player));
                    if (temp == NULL)
                    {
                        perror("Singles Player Memory Reallocation Failure.\n");
                    }
                    else
                    {
                        // realloc successful, newArr may point to the same or a new memory block
                        singlesPlayerQueue = temp; // update the array pointer if needed
                    }
                }
                singlesPlayerQueue[waitingSinglesPlayerCount - 1] = newPlayer;
            }
        }
    }
    else
    {
        waitingDoublesPlayerCount++;
        if (waitingDoublesPlayerCount > WAITING_DOUBLES_POOL_SIZE)
        {
            WAITING_DOUBLES_POOL_SIZE *= 2;
            struct Player *temp = (struct Player *)realloc(doublesPlayerQueue, WAITING_DOUBLES_POOL_SIZE * sizeof(struct Player));
            if (temp == NULL)
            {
                perror("Doubles Player Memory Reallocation Failure.\n");
            }
            else
            {
                // realloc successful, newArr may point to the same or a new memory block
                doublesPlayerQueue = temp; // update the array pointer if needed
            }
        }
        doublesPlayerQueue[waitingDoublesPlayerCount - 1] = newPlayer;
        // response->matchedPlayer=NULL;
    }
    response->court_no = availableCourtNumber;
    return response;
}
struct Response *checkSinglesMatchScheduler(int time_stamp)
{
    int availableCourtNumber = -1;
    struct Response *response = (struct Response *)malloc(sizeof(struct Response));
    response->court_no = -1;
    response->game_type = -1;
    response->matchedPlayer = (struct Player *)malloc(2 * sizeof(struct Player));
    struct Player newPlayer;
    if (waitingSinglesPlayerCount > 1)
    {
        for (int court_no = 0; court_no < NO_OF_COURTS; court_no++)
        {
            if (courts_status.court[court_no] == UNOCCUPIED)
            {
                // Sort the existing players based on the FCFS policy
                for (int i = 0; i < waitingSinglesPlayerCount; i++)
                {
                    for (int j = i + 1; j < waitingSinglesPlayerCount; j++)
                    {
                        if (singlesPlayerQueue[i].arrival_time > singlesPlayerQueue[j].arrival_time)
                        {
                            // Swap players to maintain FCFS order
                            struct Player temp = singlesPlayerQueue[i];
                            singlesPlayerQueue[i] = singlesPlayerQueue[j];
                            singlesPlayerQueue[j] = temp;
                        }
                    }
                }
                newPlayer = singlesPlayerQueue[0];
                size_t i = 0;

                // Remove the matched player from the queue
                if (newPlayer.preference == 'B' || newPlayer.preference == 'b')
                {
                    if (waitingDoublesPlayerCount > 0)
                    {
                        int find_i = -1;
                        for (; i < waitingDoublesPlayerCount; i++)
                        {
                            if (newPlayer.player_id == doublesPlayerQueue[i].player_id)
                            {
                                find_i = i;
                                break;
                            }
                        }
                        // singlesPlayerQueue[find_i]=NULL;
                        if (find_i != -1)
                        {
                            for (i = find_i; i < waitingDoublesPlayerCount - 1; i++)
                            {
                                doublesPlayerQueue[i] = doublesPlayerQueue[i + 1];
                            }
                            waitingDoublesPlayerCount--;
                        }
                    }
                }

                for (i = 0; i < waitingSinglesPlayerCount - 1; i++)
                {
                    singlesPlayerQueue[i] = singlesPlayerQueue[i + 1];
                }
                waitingSinglesPlayerCount--;
                newPlayer.arrival_time = time_stamp;
                response->matchedPlayer[0] = newPlayer;
                break;
            }
        }
    }
    if (waitingSinglesPlayerCount > 0)
    {
        struct Response *temp_res = canFormSinglesMatch(&newPlayer);
        if (temp_res->court_no >= 0)
        {
            response->matchedPlayer[1] = temp_res->matchedPlayer[0];
            availableCourtNumber = temp_res->court_no;
        }
    }
    response->court_no = availableCourtNumber;
    return response;
}
struct Response *checkDoublesMatchScheduler(int time_stamp)
{
    int availableCourtNumber = -1;
    struct Response *response = (struct Response *)malloc(sizeof(struct Response));
    response->court_no = -1;
    response->game_type = -1;
    response->matchedPlayer = (struct Player *)malloc(4 * sizeof(struct Player));
    struct Player newPlayer;
    if (waitingDoublesPlayerCount > 3)
    {
        for (int court_no = 0; court_no < NO_OF_COURTS; court_no++)
        {
            if (courts_status.court[court_no] == UNOCCUPIED)
            {
                // Sort the existing players based on the FCFS policy
                for (int i = 0; i < waitingDoublesPlayerCount; i++)
                {
                    for (int j = i + 1; j < waitingDoublesPlayerCount; j++)
                    {
                        if (doublesPlayerQueue[i].arrival_time > doublesPlayerQueue[j].arrival_time)
                        {
                            // Swap players to maintain FCFS order
                            struct Player temp = doublesPlayerQueue[i];
                            doublesPlayerQueue[i] = doublesPlayerQueue[j];
                            doublesPlayerQueue[j] = temp;
                        }
                    }
                }
                newPlayer = doublesPlayerQueue[0];
                size_t i = 0;
                if (newPlayer.preference == 'B' || newPlayer.preference == 'b')
                {
                    if (waitingSinglesPlayerCount > 0)
                    {
                        int find_i = -1;
                        for (; i < waitingSinglesPlayerCount; i++)
                        {
                            if (newPlayer.player_id == singlesPlayerQueue[i].player_id)
                            {
                                find_i = i;
                                break;
                            }
                        }
                        // singlesPlayerQueue[find_i]=NULL;
                        if (find_i != -1)
                        {
                            for (i = find_i; i < waitingSinglesPlayerCount - 1; i++)
                            {
                                singlesPlayerQueue[i] = singlesPlayerQueue[i + 1];
                            }
                            waitingSinglesPlayerCount--;
                        }
                    }
                }
                // Remove the matched player from the queue

                for (i = 0; i < waitingDoublesPlayerCount - 1; i++)
                {
                    doublesPlayerQueue[i] = doublesPlayerQueue[i + 1];
                }
                waitingDoublesPlayerCount--;
                newPlayer.arrival_time = time_stamp;
                response->matchedPlayer[0] = newPlayer;
                break;
            }
        }
        if (waitingDoublesPlayerCount > 2)
        {
            struct Response *temp_res = canFormDoublesMatch(&newPlayer);
            if (temp_res->court_no >= 0)
            {
                response->matchedPlayer[1] = temp_res->matchedPlayer[0];
                response->matchedPlayer[2] = temp_res->matchedPlayer[1];
                response->matchedPlayer[3] = temp_res->matchedPlayer[2];
                availableCourtNumber = temp_res->court_no;
            }
        }
    }

    response->court_no = availableCourtNumber;
    return response;
}



void checkMatchScheduler(int time_stamp)
{
    printf("\nTime:   %d\t", time_stamp);
    printf("Singles Queue Size:  %ld\n", waitingSinglesPlayerCount);
    printf("Singles Queue: ");
    for (int i = 0; i < waitingSinglesPlayerCount; i++)
    {
        printf("%d ", singlesPlayerQueue[i].player_id);
    }
    printf("\n");
    printf("Doubles Queue Size:  %ld\n", waitingDoublesPlayerCount);
    printf("Doubles Queue: ");
    for (int i = 0; i < waitingDoublesPlayerCount; i++)
    {
        printf("%d ", doublesPlayerQueue[i].player_id);
    }
    printf("\n");
    struct Response *response = (struct Response *)malloc(sizeof(struct Response));
    response->court_no = -1;
    response->game_type = -1;
    char successMsg[1024];
    for (int i = 0; i < NO_OF_COURTS; i++)
    {
        if ((time_stamp + 1 > (courts_status.last_game_end_time[i])) && (courts_status.court[i] == OCCUPIED))
        {
            courts_status.court[i] = UNOCCUPIED;
        }
    }
    printf("Free Courts:\t");
    for (int i = 0; i < NO_OF_COURTS; i++)
    {
        if (courts_status.court[i] == UNOCCUPIED)
            printf("%d\t", i + 1);
    }
    printf("\n");

    // checking if matches can be scheduled
    for (int k = 0; k < NO_OF_COURTS; ++k)
    {
        if (courts_status.court[k] == UNOCCUPIED)
        {
            if (waitingDoublesPlayerCount > 3)
            {
                response = checkDoublesMatchScheduler(time_stamp);
            }
            else if (waitingSinglesPlayerCount > 1)
            {
                response = checkSinglesMatchScheduler(time_stamp);
            }
            bzero(successMsg,sizeof(successMsg));
        }
    }
}
struct Response *canFormMatch(struct Player *nPlayer)
{

    struct Player newPlayer = *nPlayer;
    printf("\nTime:   %d\t", newPlayer.arrival_time);
    printf("\nPlayer ID:  %d, Preference: %c, Gender:    %c\n", newPlayer.player_id, newPlayer.preference, newPlayer.gender);

    int availableCourtNumber = -1;
    // Handle court free event
    for (int i = 0; i < NO_OF_COURTS; i++)
    {
        if ((newPlayer.arrival_time + 1 > (courts_status.last_game_end_time[i])) && (courts_status.court[i] == OCCUPIED))
        {
            courts_status.court[i] = UNOCCUPIED;
        }
    }
    printf("Free Courts:\t");
    for (int i = 0; i < NO_OF_COURTS; i++)
    {
        if (courts_status.court[i] == UNOCCUPIED)
            printf("%d\t", i + 1);
    }
    struct Response *response = (struct Response *)malloc(sizeof(struct Response));
    response->court_no = -1;
    response->game_type = -1;
    // printf("ID: %d, Pref: %c\n",newPlayer.player_id,newPlayer.preference);
    switch (newPlayer.preference)
    {
    case 'S':
        response = canFormSinglesMatch(nPlayer);
        break;
    case 'D':
        response = canFormDoublesMatch(nPlayer);
        break;
    case 'B': // D ->S
        if (waitingDoublesPlayerCount > 2)
        {
            response = canFormDoublesMatch(nPlayer);
        }
        else if (waitingSinglesPlayerCount > 0)
        {
            response = canFormSinglesMatch(nPlayer);
        }
        else
        {
            waitingSinglesPlayerCount++;

            if (waitingSinglesPlayerCount > WAITING_SINGLES_POOL_SIZE)
            {
                WAITING_SINGLES_POOL_SIZE *= 2;
                struct Player *temp = (struct Player *)realloc(singlesPlayerQueue, WAITING_SINGLES_POOL_SIZE * sizeof(struct Player));
                if (temp == NULL)
                {
                    perror("Singles Player Memory Reallocation Failure.\n");
                }
                else
                {
                    // realloc successful, newArr may point to the same or a new memory block
                    singlesPlayerQueue = temp; // update the array pointer if needed
                }
            }
            singlesPlayerQueue[waitingSinglesPlayerCount - 1] = newPlayer;
            waitingDoublesPlayerCount++;
            if (waitingDoublesPlayerCount > WAITING_DOUBLES_POOL_SIZE)
            {
                WAITING_DOUBLES_POOL_SIZE *= 2;
                struct Player *temp = (struct Player *)realloc(doublesPlayerQueue, WAITING_DOUBLES_POOL_SIZE * sizeof(struct Player));
                if (temp == NULL)
                {
                    perror("Doubles Player Memory Reallocation Failure.\n");
                }
                else
                {
                    // realloc successful, newArr may point to the same or a new memory block
                    doublesPlayerQueue = temp; // update the array pointer if needed
                }
            }
            doublesPlayerQueue[waitingDoublesPlayerCount - 1] = newPlayer;
            // response->matchedPlayer=NULL;
            response->court_no = -1;
        }
        // response->court_no=availableCourtNumber;
        break;
    case 'b': // S ->D
        if (waitingSinglesPlayerCount > 0)
        {
            response = canFormSinglesMatch(nPlayer);
        }
        else if (waitingDoublesPlayerCount > 2)
        {
            response = canFormDoublesMatch(nPlayer);
        }
        else
        {
            waitingSinglesPlayerCount++;
            if (waitingSinglesPlayerCount > WAITING_SINGLES_POOL_SIZE)
            {
                WAITING_SINGLES_POOL_SIZE *= 2;
                struct Player *temp = (struct Player *)realloc(singlesPlayerQueue, WAITING_SINGLES_POOL_SIZE * sizeof(struct Player));
                if (temp == NULL)
                {
                    perror("Singles Player Memory Reallocation Failure.\n");
                }
                else
                {
                    // realloc successful, newArr may point to the same or a new memory block
                    singlesPlayerQueue = temp; // update the array pointer if needed
                }
            }
            singlesPlayerQueue[waitingSinglesPlayerCount - 1] = newPlayer;
            waitingDoublesPlayerCount++;
            if (waitingDoublesPlayerCount > WAITING_DOUBLES_POOL_SIZE)
            {
                WAITING_DOUBLES_POOL_SIZE *= 2;
                struct Player *temp = (struct Player *)realloc(doublesPlayerQueue, WAITING_DOUBLES_POOL_SIZE * sizeof(struct Player));
                if (temp == NULL)
                {
                    perror("Doubles Player Memory Reallocation Failure.\n");
                }
                else
                {
                    // realloc successful, newArr may point to the same or a new memory block
                    doublesPlayerQueue = temp; // update the array pointer if needed
                }
            }
            doublesPlayerQueue[waitingDoublesPlayerCount - 1] = newPlayer;
            // response->matchedPlayer=NULL;
            response->court_no = -1;
        }
        break;
    default:
        printf("Singles Queue Size:  %ld\n", waitingSinglesPlayerCount);
        printf("Singles Queue: ");
        for (int i = 0; i < waitingSinglesPlayerCount; i++)
        {
            printf("%d ", singlesPlayerQueue[i].player_id);
        }
        printf("\n");
        printf("Doubles Queue Size:  %ld\n", waitingDoublesPlayerCount);
        printf("Doubles Queue: ");
        for (int i = 0; i < waitingDoublesPlayerCount; i++)
        {
            printf("%d ", doublesPlayerQueue[i].player_id);
        }
        printf("\n");
        return NULL;
    }
    printf("\nSingles Queue Size:  %ld\n", waitingSinglesPlayerCount);
    printf("Singles Queue: ");
    for (int i = 0; i < waitingSinglesPlayerCount; i++)
    {
        printf("%d ", singlesPlayerQueue[i].player_id);
    }
    printf("\n");
    printf("Doubles Queue Size:  %ld\n", waitingDoublesPlayerCount);
    printf("Doubles Queue: ");
    for (int i = 0; i < waitingDoublesPlayerCount; i++)
    {
        printf("%d ", doublesPlayerQueue[i].player_id);
    }
    printf("\n");
    return response;
}


/* Handles client threads */
void handleClient(int newSocket)
{
    while (!atomic_load(&ALL_MATCHES_DONE))
    {
        int newSocket = dequeueRequest(&requestQueue);
        if (newSocket == -1)
        {
            // Queue is empty, wait for a while
            usleep(10000); // Sleep for 10 milliseconds
            continue;
        }
        char buffer[1024];
        bzero(buffer, sizeof(buffer));
        ssize_t recvStatus = recv(newSocket, buffer, sizeof(buffer), 0);
        if (recvStatus <= 0)
        {
            // Either an error or the client disconnected
            if (recvStatus == 0)
            {
                printf("Client disconnected.\n");
            }
            else
            {
                perror("Error in receiving data");
            }
            break;
        }

        if (strcmp(buffer, ":exit") == 0)
        {
            printf("Client disconnected.\n");
            break;
        }
        else
        {
            // Parse the received data
            char gender, preference;
            int player_id, arrival_time;
            sscanf(buffer, "%d %d %c %c", &player_id, &arrival_time, &gender, &preference);
            struct Player *newPlayer = (struct Player *)malloc(sizeof(struct Player));
            newPlayer->player_id = player_id;
            newPlayer->arrival_time = arrival_time;
            newPlayer->gender = gender;
            newPlayer->preference = preference;
            newPlayer->socketNumber = newSocket;
            newPlayer->max_permissible_time = arrival_time+TIME_OUT_TIME;
            
            for (int i = server_time_stamp; i <= arrival_time; i++)
            {   
                    #pragma omp critical
                    checkMatchCompletions(i);
                    checkMatchScheduler(i);
                    usleep(1000);
            }
            server_time_stamp = arrival_time;

            if (player_id == 0 && gender == 'X' && preference == 'X')
            {
                char successMsg[1024];
                sprintf(successMsg, "All client requests received by server.");
                send(newSocket,successMsg, strlen(successMsg) + 1, 0);
                printf("Exiting...\n");
                close(sockfd);
                atomic_store(&ALL_MATCHES_DONE, 1); // Set the flag to true
                exit(0);
            }
            
            struct Response *response = canFormMatch(newPlayer);
            bzero(buffer, sizeof(buffer));
            break;
        }
    }
}


int main()
{
    initRequestQueue(&requestQueue);
    init_courts();
    init_queues();
    
    int ret;
    struct sockaddr_in serverAddr;
    struct sockaddr_in newAddr;
    socklen_t addr_size;

    clientCount = 0;

    ret = 0;

    printf("Current Count of Clients: %d\n", clientCount);
    NUM_CLIENTS=recieveNoOfClients();
    int clientsLeft=NUM_CLIENTS;
    int newSocket;
    struct sockaddr_in localServerAddr;
    newSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (newSocket < 0)
    {
        printf("[-]Error in connection.\n");
        exit(1);
    }
    printf("[+]Server Socket is created for thread.\n");
    memset(&localServerAddr, '\0', sizeof(localServerAddr));
    localServerAddr.sin_family = AF_INET;
    localServerAddr.sin_port = htons(PORT);
    localServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    ret = bind(newSocket, (struct sockaddr *)&localServerAddr, sizeof(localServerAddr));
    if (ret < 0)
    {
        printf("[-]Error in binding. Maybe earlier socket is still running. Wait for some time or use different PORT\n");
        exit(1);
    }
    printf("[+]Bind to port\n");
    if (ret < 0)
    {
        printf("[-]Error in binding.\n");
        exit(1);
    }
    printf("[+]Bind to port %d\n", PORT);

    if (listen(newSocket, NUM_CLIENTS*2) == 0)
    {
        printf("[+]Listening....\n");
    }
    else
    {
        printf("[-]Error in binding.\n");
    }

    #pragma omp parallel num_threads(NUM_CLIENTS) shared(server_time_stamp, singlesPlayerQueue,doublesPlayerQueue,ALL_MATCHES_DONE)
    { 
        int tid = omp_get_thread_num();
        while (!atomic_load(&ALL_MATCHES_DONE)) {
            #pragma omp parallel for ordered
            for(int i=0;i<=NUM_CLIENTS;i++)
            {
                while(1){
                    int clientSocket = accept(newSocket, (struct sockaddr *)&newAddr, &addr_size);
                    if (clientSocket < 0)
                    {
                        perror("Accept Failed");
                        continue;
                    }
                    printf("Connection accepted from %s:%d by thread %d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port), tid);
                    #pragma omp critical
                    enqueueRequest(&requestQueue, clientSocket);
                    printf("Thread %d handling now.\n",tid);
                    handleClient(clientSocket);
                    break;
                }  
            }          
        }

    }

    close(sockfd);
    // Free memory for rooms and room_booking_timestamp
    printf("Exited\n");
    return 0;
}