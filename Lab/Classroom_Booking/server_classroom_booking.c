#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

/* Macros for various return status codes*/
#define OK                      0
#define SLOT_ALREADY_BOOKED    -1
#define COOL_DOWN_PERIOD       -2
#define INVALID_REQUEST        -3
#define COOL_DOWN_DELAY        20
#define AVAILABLE               0
#define NOT_AVAILABLE           1
#define VALID_SLOT_TIME         0
#define INVALID_SLOT_TIME      -1
#define PORT 8080
#define NO_OF_ROOMS             5
#define NO_OF_SLOTS             8
#define VALID_ROOM              0
#define INVALID_ROOM           -1

/* Thread Array*/
pthread_t *tid;

/* Mutex variables to handle respective critical sections*/
pthread_mutex_t booking_lock;  
pthread_mutex_t cancel_lock;  
pthread_mutex_t write_lock;  
pthread_mutex_t client_lock;

/* Stores the count of clients connected to server*/
int clientCount = 0;  
// Structure to represent a client request
struct Request {
    int socket;
    struct Request* next;
};

// Linked list to manage the request queue
struct RequestQueue {
    struct Request* front;
    struct Request* rear;
};

// Initialize the request queue
void initRequestQueue(struct RequestQueue* queue) {
    queue->front = NULL;
    queue->rear = NULL;
}

// Enqueue a new request
void enqueueRequest(struct RequestQueue* queue, int socket) {
    struct Request* newRequest = (struct Request*)malloc(sizeof(struct Request));
    newRequest->socket = socket;
    newRequest->next = NULL;

    if (queue->rear == NULL) {
        queue->front = newRequest;
        queue->rear = newRequest;
    } else {
        queue->rear->next = newRequest;
        queue->rear = newRequest;
    }
}

// Dequeue a request
int dequeueRequest(struct RequestQueue* queue) {
    if (queue->front == NULL) {
        return -1; // Queue is empty
    }

    int socket = queue->front->socket;
    struct Request* temp = queue->front;

    if (queue->front == queue->rear) {
        queue->front = NULL;
        queue->rear = NULL;
    } else {
        queue->front = queue->front->next;
    }

    free(temp);
    return socket;
}

// Global request queue
struct RequestQueue requestQueue;

/* Structure for maintaing status of Rooms*/
struct rooms{
    int** room;
    int** room_booking_timestamp;
} rooms_status;

/* Function to initialize the room_status data structure*/
void init_rooms(){
    rooms_status.room = calloc(NO_OF_ROOMS,sizeof(int *));
    if (rooms_status.room == NULL){
        printf("\nMemory allocation for rooms failed!\n");
        exit(-1);
    }
    for (int i=0;i<NO_OF_ROOMS;i++){
        rooms_status.room[i] = (int *)calloc(NO_OF_SLOTS, sizeof(int));
    }
    rooms_status.room_booking_timestamp = calloc(NO_OF_ROOMS,sizeof(int *));
    if (rooms_status.room_booking_timestamp == NULL){
        printf("\nMemory allocation for room_booking_timestamp failed!\n");
        exit(-1);
    }
    for (int i=0;i<NO_OF_ROOMS;i++){
        rooms_status.room_booking_timestamp[i] = (int *)calloc(NO_OF_SLOTS, sizeof(int));
    }
}


/* Returns validity of the time slot passed by the client*/
int isValidTimeSlot(const char *timeSlot) {
    const char *validTimeSlots[] = {"08:00-09:30", "09:30-11:00", "11:00-12:30", "12:30-14:00", "14:00-15:30", "15:30-17:00", "17:00-18:30", "18:30-20:00"};

    for (int i = 0; i < NO_OF_SLOTS; i++) {
        if (strcmp(timeSlot, validTimeSlots[i]) == 0) {
            return VALID_SLOT_TIME;  // The time slot is valid
        }
    }

    return INVALID_SLOT_TIME;  // The time slot is not valid
}

/* Returns the slot number from 0 to 7 for valid time-slots accordingly, and -1 incase of invalid time slot*/
int getSlotNumber(const char *timeSlot){
    const char *validTimeSlots[8] = {"08:00-09:30", "09:30-11:00", "11:00-12:30", "12:30-14:00", "14:00-15:30", "15:30-17:00", "17:00-18:30", "18:30-20:00"};

    for (int i = 0; i < NO_OF_SLOTS; i++) {
        if (strcmp(timeSlot, validTimeSlots[i]) == 0) {
            return i;
        }
    }
    return -1;
}

/* Returns the Slot-Time taking the slot number (0-7)*/
char* getSlotTime(int slot_no){
    switch(slot_no){
        case 0:
            return("08:00-09:30");
            break;
        case 1:
            return("09:30-11:00");
            break;
        case 2: 
            return("11:00-12:30");
            break;
        case 3: 
            return("12:30-14:00");
            break;
        case 4: 
            return("14:00-15:30");
            break;
        case 5:
            return("15:30-17:00");
            break;
        case 6:
            return("17:00-18:30");
            break;
        case 7:
            return("18:30-20:00");
            break;
        default:
            printf("Error: getSlotTime()\n");
            return(NULL);
    }
}

/* Returns if the Room no is valid or not*/
int isValidRoomNo(int room_no){
    if (room_no<0 || room_no >5){
        return INVALID_ROOM;
    }
    return VALID_ROOM;
}

/* Returns Status code according to the status of booking request made*/
int bookRoom(int room_no, char* slot_time, char* req_timestamp){
    pthread_mutex_lock(&booking_lock);  // Acquire the booking lock
    int STATUS_CODE=INVALID_REQUEST;
    // Checking either the requested time slot is valid or room number is valid
    if (isValidTimeSlot(slot_time)==INVALID_SLOT_TIME || isValidRoomNo(room_no)==INVALID_ROOM){
        pthread_mutex_unlock(&booking_lock);  // Release the lock
        return STATUS_CODE;
    }
    int slot_no=getSlotNumber(slot_time);
  
    room_no--;
    if (rooms_status.room[room_no][slot_no] == NOT_AVAILABLE){
        STATUS_CODE = SLOT_ALREADY_BOOKED;
        pthread_mutex_unlock(&booking_lock);  // Release the lock
        return STATUS_CODE;
    }
    STATUS_CODE = OK;
    rooms_status.room[room_no][slot_no] = NOT_AVAILABLE;
    rooms_status.room_booking_timestamp[room_no][slot_no] = atoi(req_timestamp);
    pthread_mutex_unlock(&booking_lock);  // Release the lock
    return STATUS_CODE;
}

/* Returns Status code according to the status of cancellation request made*/
int cancelRoom(int room_no, char* slot_time, char* req_timestamp){
    pthread_mutex_lock(&cancel_lock);  // Acquire the cancel lock
    int STATUS_CODE=INVALID_REQUEST;
    // Checking either the requested time slot is valid or room number is valid
    if (isValidTimeSlot(slot_time)==INVALID_SLOT_TIME || isValidRoomNo(room_no)==INVALID_ROOM){

        pthread_mutex_unlock(&cancel_lock);  // Release the lock
        return STATUS_CODE;
    }
    
    int slot_no = getSlotNumber(slot_time);
    room_no--;
    // Checking if at all the room is booked or not
    if (rooms_status.room[room_no][slot_no] == AVAILABLE){
        pthread_mutex_unlock(&cancel_lock);  // Release the lock
        return STATUS_CODE;
    }
    
    //Checking the cooldown period
    if (atoi(req_timestamp)-1<=(rooms_status.room_booking_timestamp[room_no][slot_no]+COOL_DOWN_DELAY)){
        STATUS_CODE = COOL_DOWN_PERIOD;
        pthread_mutex_unlock(&cancel_lock);  // Release the lock
        return STATUS_CODE;
    }

    rooms_status.room[room_no][slot_no] = AVAILABLE;
    STATUS_CODE = OK;
    pthread_mutex_unlock(&cancel_lock);  // Release the lock
    return STATUS_CODE;
}

/* Returns Status of all rooms, and while writing to the csv file, it only shows the booked rooms*/
struct rooms getRoomsStatus(){
    return rooms_status;
}


/* Writes to the "output.csv" file*/
void write_csv(int room_no, char req_type, char* slot_time, int status, char *get_msg){
    pthread_mutex_lock(&write_lock);
    char* filename="output.csv";
    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        printf("Error opening file %s for writing.\n", filename);
        return;
    }
    switch(req_type){
        case 'B':
            fprintf(file,"%s,%d,%s,%d,\n","BOOK",room_no,slot_time,status);
            break;
        case 'C':
            fprintf(file,"%s,%d,%s,%d,\n","CANCEL",room_no,slot_time,status);
            break;
        case 'G':
            fprintf(file,"%s,,,%d,%s\n","GET",status,get_msg);
            break;
        default:
            printf("Error\n");
    }
    fclose(file); 
    pthread_mutex_unlock(&write_lock);
}

/* Handles client threads */
void *handleClient(void *arg) {
    pthread_mutex_lock(&client_lock);
    int newSocket = *(int *)arg;
    int req_count=0;

    while (1) {
        // pthread_mutex_lock(&recv_lock);
        int newSocket = dequeueRequest(&requestQueue);
        if (newSocket == -1) {
            // Queue is empty, wait for a while
            usleep(10000); // Sleep for 10 milliseconds
            continue;
        }
        char buffer[1024];
        bzero(buffer, sizeof(buffer));
        ssize_t recvStatus = recv(newSocket, buffer, sizeof(buffer), 0);
        if (recvStatus <= 0) {
        // Either an error or the client disconnected
            if (recvStatus == 0) {
                printf("Client disconnected.\n");
            } else {
                perror("Error in receiving data");
            }
            break;
        }
        // pthread_mutex_unlock(&recv_lock);

        if (strcmp(buffer, ":exit") == 0) {
            printf("Client disconnected.\n");
            break;
        } else {

            clientCount++;
            // pthread_mutex_unlock(&client_lock);


            // Parse the received data
            char req_timestamp[50], req_type[50], slot_time[50];
            int room_no;
            sscanf(buffer, "%s %s %d %s", req_timestamp, req_type, &room_no, slot_time);
            // Handle the request based on req_type
            if (strcmp(req_type, "BOOK") == 0) {
                int status = bookRoom(room_no, slot_time, req_timestamp);
                write_csv(room_no,'B',slot_time,status,NULL);
            } else if (strcmp(req_type, "CANCEL") == 0) {
                int status = cancelRoom(room_no, slot_time, req_timestamp);
                write_csv(room_no,'C',slot_time,status,NULL);
            } else if (strcmp(req_type, "GET") == 0) { 
                struct rooms st = getRoomsStatus();
                 // Initial buffer size, you can adjust it based on your data
                size_t bufferSize = 1024;
                char* msg = (char*)malloc(bufferSize * sizeof(char));
                
                // Initialize the string
                msg[0] = '\0';
                
                for (int i = 0; i < NO_OF_ROOMS; i++) {
                    for (int j = 0; j < NO_OF_SLOTS; j++) {
                        if(i==0 && j==0){
                            snprintf(msg + strlen(msg), bufferSize - strlen(msg), "\"{");
                        } 
                        if (rooms_status.room[i][j] == NOT_AVAILABLE) {
                            if(strcmp(msg,"\"{")==0){
                                snprintf(msg + strlen(msg), bufferSize - strlen(msg), "('%d', '%s')", i + 1, getSlotTime(j));
                            }else{
                                snprintf(msg + strlen(msg), bufferSize - strlen(msg), ", ('%d', '%s')", i + 1, getSlotTime(j));
                            }
                        }
                        if(i==NO_OF_ROOMS-1 && j==NO_OF_SLOTS-1){
                            snprintf(msg + strlen(msg), bufferSize - strlen(msg), "}\"");
                        }
                    }
                }
                write_csv(0,'G',NULL,OK,msg);
            }
            else {
                printf("Unknown request type: %s\n", req_type);
            }
            // pthread_mutex_lock(&client_lock);
            clientCount--;
            close(newSocket);
            bzero(buffer, sizeof(buffer));
        }
    }
    // Close the socket and remove the thread
    //close(newSocket);
    pthread_mutex_unlock(&client_lock);
    pthread_exit(NULL);
}


int main() {
    char* filename="output.csv";
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error opening file %s for writing.\n", filename);
        return 0;
    }
    fprintf(file,"Type,Room,Timeslot,Status,\n");
    fclose(file);
    pthread_mutex_init(&booking_lock, NULL);  // Initialize the mutex
    pthread_mutex_init(&cancel_lock, NULL);  // Initialize the mutex
    pthread_mutex_init(&client_lock, NULL);
    pthread_mutex_init(&write_lock, NULL);
    initRequestQueue(&requestQueue);

    int num_threads = 10000;
    tid = malloc(num_threads*sizeof(pthread_t));
    init_rooms();
    int sockfd, ret;
    struct sockaddr_in serverAddr;
    int newSocket;
    struct sockaddr_in newAddr;
    socklen_t addr_size;

    // Allocate initial memory for pthreads
    clientCount = 0;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("[-]Error in connection.\n");
        exit(1);
    }
    printf("[+]Server Socket is created.\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    ret = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (ret < 0) {
        printf("[-]Error in binding.\n");
        exit(1);
    }
    printf("[+]Bind to port %d\n", PORT);

    if (listen(sockfd, 30000) == 0) {
        printf("[+]Listening....\n");
    } else {
        printf("[-]Error in binding.\n");
    }

    while (1) {
        newSocket = accept(sockfd, (struct sockaddr *)&newAddr, &addr_size);
        
        if (newSocket < 0) {
            perror("Accept Failed");
            continue;
        }
        printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
        enqueueRequest(&requestQueue, newSocket);
        // Reallocate memory for the new client
        clientCount++;


        // Create a new socket for the client
        int *socketPtr = malloc(sizeof(int));
        *socketPtr = newSocket;

        // Create a new thread to handle the client with its own socket and counter value
       // printf("Client count: %d\n",clientCount);
        if (pthread_create(&tid[clientCount - 1], NULL, handleClient, socketPtr) != 0) {
            printf("Failed to create thread.\n");
        }
        if (clientCount>num_threads){
            pthread_t *temp_tid = realloc(tid, sizeof(pthread_t) * clientCount);
            if (temp_tid == NULL) {
                fprintf(stderr, "Failed to reallocate memory for pthreads.\n");
                // Handle error and possibly free(tid) here
                free(tid);
            } else {
                tid = temp_tid;
            }
        }
    }
    for (int i = 0; i < num_threads; i++) {
        pthread_join(tid[i], NULL);
    }
    for (int i = 0; i < sizeof(tid)/sizeof(tid[i]); i++) {
        free((void*)tid[i]);
    }

    free(tid);
    // Free memory for rooms and room_booking_timestamp
    for (int i = 0; i < NO_OF_ROOMS; i++) {
        free(rooms_status.room[i]);
        free(rooms_status.room_booking_timestamp[i]);
    }
    free(rooms_status.room);
    free(rooms_status.room_booking_timestamp);

    close(sockfd);

    return 0;
}
