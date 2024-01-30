#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define NO_OF_COURTS 4
#define OCCUPIED 1
#define UNOCCUPIED 0
#define INVALID_REQUEST -4
#define MEN_TIME_DURATION 15
#define WOMEN_TIME_DURATION 10

/* Thread Array */
pthread_t *tid;
int clientCount=0;
int server_time_stamp=1;

pthread_mutex_t request_lock;
pthread_mutex_t client_lock;
pthread_mutex_t write_lock;
pthread_mutex_t scheduler_lock;

size_t waitingSinglesPlayerCount=0, waitingDoublesPlayerCount=0;
size_t WAITING_SINGLES_POOL_SIZE = 1000;
size_t WAITING_DOUBLES_POOL_SIZE = 1000;

struct courts
{
    int *court;
    int *last_game_start_time;
    int *last_game_end_time;
} courts_status;

// Player structure
struct Player
{
    int player_id;
    int arrival_time;
    char gender;
    char preference;
};

struct Player* singlesPlayerQueue;
struct Player* doublesPlayerQueue;

void init_queues(){
    // Initialize singlesPlayerQueue with dynamically allocated memory
    singlesPlayerQueue = (struct Player*)malloc(WAITING_SINGLES_POOL_SIZE * sizeof(struct Player));
    if (singlesPlayerQueue == NULL) {
        perror("Error allocating memory for singlesPlayerQueue");
        // Handle memory allocation failure
    }
    // Initialize doublesPlayerQueue with dynamically allocated memory
    doublesPlayerQueue = (struct Player*)malloc(WAITING_DOUBLES_POOL_SIZE * sizeof(struct Player));
    if (doublesPlayerQueue == NULL) {
        perror("Error allocating memory for doublesPlayerQueue");
        // Handle memory allocation failure
    }

}


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
}
struct Response{
    int court_no;
    int game_type;
    struct Player* matchedPlayer;
};

/* Writes to the "output.csv" file*/
void write_csv(int g_type,int start_time,int end_time, int court_no, int pl1, int pl2, int pl3, int pl4){
    pthread_mutex_lock(&write_lock);
    char* filename="output.csv";
    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        printf("Error opening file %s for writing.\n", filename);
        return;
    }
    if (g_type==0)
        fprintf(file,"%d,%d,%d,%d,%d,\n",start_time,end_time,court_no,pl1,pl2);
    else
        fprintf(file,"%d,%d,%d,%d,%d,%d,%d,\n",start_time,end_time,court_no,pl1,pl2,pl3,pl4);


    fclose(file); 
    pthread_mutex_unlock(&write_lock);
}

struct Response* canFormSinglesMatch(struct Player *nPlayer){
    struct Player newPlayer = *nPlayer;
    int availableCourtNumber=-1;
    struct Response *response = (struct Response*)malloc(sizeof(struct Response)); 
    response->court_no=-1;
    response->game_type=-1;
    int chance=0;
    response->matchedPlayer=(struct Player*)malloc(sizeof(struct Player));
    if (waitingSinglesPlayerCount>0){
        for(int court_no=0;court_no<NO_OF_COURTS;court_no++){
            if (courts_status.court[court_no] == UNOCCUPIED){
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

                struct Player matchedPlayer=singlesPlayerQueue[0]; //matched player
                
                size_t i=0;
                if (matchedPlayer.preference == 'B' || matchedPlayer.preference == 'b'){
                    if(waitingDoublesPlayerCount>0){
                        int find_i=-1;
                        for (;i<waitingDoublesPlayerCount;i++){
                            if (matchedPlayer.player_id == doublesPlayerQueue[i].player_id){
                                find_i=i;
                                break; 
                            }
                        }
                    // singlesPlayerQueue[find_i]=NULL;
                        if (find_i != -1){
                                for (i = find_i; i < waitingDoublesPlayerCount - 1; i++){
                                    doublesPlayerQueue[i] = doublesPlayerQueue[i + 1];
                                }
                                waitingDoublesPlayerCount--;
                        }
                        //singlesPlayerQueue[i]=NULL;
                    }
                }
                //Remove the matched player from the queue
                
                for (i=0;i<waitingSinglesPlayerCount-1;i++){
                    singlesPlayerQueue[i]=singlesPlayerQueue[i+1];
                }
                //singlesPlayerQueue[i]=NULL;
                waitingSinglesPlayerCount--;
                courts_status.last_game_start_time[court_no]=newPlayer.arrival_time;
                if (newPlayer.gender == 'M'){
                    courts_status.last_game_end_time[court_no]=newPlayer.arrival_time+MEN_TIME_DURATION;
                } else if(matchedPlayer.gender == 'M'){
                    courts_status.last_game_end_time[court_no]=newPlayer.arrival_time+MEN_TIME_DURATION;
                } else{
                    courts_status.last_game_end_time[court_no]=newPlayer.arrival_time+WOMEN_TIME_DURATION;
                }
                courts_status.court[court_no]=OCCUPIED;
                response->game_type=0;
                response->matchedPlayer[0]=matchedPlayer;
                availableCourtNumber=court_no;
                chance=1;
                response->court_no=availableCourtNumber;
                break;
            } 
        }
        if(chance==0){
            waitingSinglesPlayerCount++;
            if (waitingSinglesPlayerCount > WAITING_SINGLES_POOL_SIZE){
                WAITING_SINGLES_POOL_SIZE*=2;
                struct Player *temp = (struct Player*)realloc(singlesPlayerQueue, WAITING_SINGLES_POOL_SIZE * sizeof(struct Player));
                if (temp == NULL) {
                    perror("Singles Player Memory Reallocation Failure.\n");
                } else {
                    // realloc successful, newArr may point to the same or a new memory block
                    singlesPlayerQueue = temp; // update the array pointer if needed
                }
            }
            singlesPlayerQueue[waitingSinglesPlayerCount-1] = newPlayer;
            if (newPlayer.preference=='B' || newPlayer.preference=='b'){
                waitingDoublesPlayerCount++;
                if (waitingDoublesPlayerCount > WAITING_DOUBLES_POOL_SIZE){
                    WAITING_DOUBLES_POOL_SIZE*=2;
                    struct Player *temp = (struct Player*)realloc(doublesPlayerQueue, WAITING_DOUBLES_POOL_SIZE * sizeof(struct Player));
                    if (temp == NULL) {
                    perror("Singles Player Memory Reallocation Failure.\n");
                    } else {
                    // realloc successful, newArr may point to the same or a new memory block
                    doublesPlayerQueue = temp; // update the array pointer if needed
                    }
                }
                doublesPlayerQueue[waitingDoublesPlayerCount-1] =newPlayer;
            }
            
        // response->matchedPlayer=NULL;
        }
    }else{
        waitingSinglesPlayerCount++;
        if (waitingSinglesPlayerCount > WAITING_SINGLES_POOL_SIZE){
            WAITING_SINGLES_POOL_SIZE*=2;
            struct Player *temp = (struct Player*)realloc(singlesPlayerQueue, WAITING_SINGLES_POOL_SIZE * sizeof(struct Player));
            if (temp == NULL) {
                perror("Singles Player Memory Reallocation Failure.\n");
            } else {
                // realloc successful, newArr may point to the same or a new memory block
                singlesPlayerQueue = temp; // update the array pointer if needed
            }
        }
        singlesPlayerQueue[waitingSinglesPlayerCount-1] = newPlayer;
        // response->matchedPlayer=NULL;
    }
    response->court_no=availableCourtNumber;
    return response;
}


struct Response* canFormDoublesMatch(struct Player *nPlayer){
    struct Player newPlayer = *nPlayer;
    int availableCourtNumber=-1;
    struct Response *response = (struct Response*)malloc(sizeof(struct Response)); 
    response->court_no=-1;
    response->game_type=-1;
    int chance=0;
    response->matchedPlayer = (struct Player*)malloc(3*sizeof(struct Player));
    if (waitingDoublesPlayerCount>2) {
        for(int court_no=0;court_no<NO_OF_COURTS;court_no++){
            if (courts_status.court[court_no] == UNOCCUPIED){
                chance=1;
                // Sort the existing players based on the FCFS policy
                for (int i = 0; i < waitingDoublesPlayerCount; i++){
                    for (int j = i + 1; j < waitingDoublesPlayerCount; j++){
                        if (doublesPlayerQueue[i].arrival_time > doublesPlayerQueue[j].arrival_time){
                            // Swap players to maintain FCFS order
                            struct Player temp = doublesPlayerQueue[i];
                            doublesPlayerQueue[i] = doublesPlayerQueue[j];
                            doublesPlayerQueue[j] = temp;
                        }
                    }
                }

                struct Player matchedPlayer_0 = doublesPlayerQueue[0]; //matched player
                struct Player matchedPlayer_1 = doublesPlayerQueue[1];
                struct Player matchedPlayer_2 = doublesPlayerQueue[2];
                //Remove the matched player from the queue
                size_t i=0;

                    if (matchedPlayer_0.preference == 'B' || matchedPlayer_0.preference == 'b'){
                        if(waitingSinglesPlayerCount>0){
                            int find_i=-1;
                            for (;i<waitingSinglesPlayerCount;i++){
                                if (matchedPlayer_0.player_id == singlesPlayerQueue[i].player_id){
                                    find_i=i;
                                    break; 
                                }
                            }
                        // singlesPlayerQueue[find_i]=NULL;
                            if (find_i != -1){
                                for (i = find_i; i < waitingSinglesPlayerCount - 1; i++){
                                    singlesPlayerQueue[i] = singlesPlayerQueue[i + 1];
                                }
                                waitingSinglesPlayerCount--;
                            }
                        }
                    }
                    i=0;
                    if (matchedPlayer_1.preference == 'B' || matchedPlayer_1.preference == 'b'){
                        if(waitingSinglesPlayerCount>0){
                            int find_i=-1;
                            for (;i<waitingSinglesPlayerCount;i++){
                                if (matchedPlayer_1.player_id == singlesPlayerQueue[i].player_id){
                                    find_i=i;
                                    break; 
                                }
                            }
                        // singlesPlayerQueue[find_i]=NULL;
                            if (find_i != -1){
                                for (i = find_i; i < waitingSinglesPlayerCount - 1; i++){
                                    singlesPlayerQueue[i] = singlesPlayerQueue[i + 1];
                                }
                                waitingSinglesPlayerCount--;
                            }
                            //singlesPlayerQueue[i]=NULL;
                        }
                    }
                    i=0;
                    if (matchedPlayer_2.preference == 'B' || matchedPlayer_2.preference == 'b'){
                        if(waitingSinglesPlayerCount>0){
                            int find_i=-1;
                            for (;i<waitingSinglesPlayerCount;i++){
                                if (matchedPlayer_2.player_id == singlesPlayerQueue[i].player_id){
                                    find_i=i;
                                    break; 
                                }
                            }
                        // singlesPlayerQueue[find_i]=NULL;
                            if (find_i != -1){
                                for (i = find_i; i < waitingSinglesPlayerCount - 1; i++){
                                    singlesPlayerQueue[i] = singlesPlayerQueue[i + 1];
                                }
                                waitingSinglesPlayerCount--;
                            }
                        }
                    }

                for (int k=0;k<3;k++){
                    for (i=0;i<waitingDoublesPlayerCount-1;i++){
                        doublesPlayerQueue[i]=doublesPlayerQueue[i+1];
                    }
                    waitingDoublesPlayerCount--;
                }
                courts_status.last_game_start_time[court_no]=newPlayer.arrival_time;
                if (newPlayer.gender == 'M'){
                    courts_status.last_game_end_time[court_no]=newPlayer.arrival_time+MEN_TIME_DURATION;
                } else if( response->matchedPlayer[0].gender == 'M' || response->matchedPlayer[1].gender == 'M' || response->matchedPlayer[2].gender == 'M'){
                    courts_status.last_game_end_time[court_no]=newPlayer.arrival_time+MEN_TIME_DURATION;
                } else{
                    courts_status.last_game_end_time[court_no]=newPlayer.arrival_time+WOMEN_TIME_DURATION;
                }
                courts_status.court[court_no]=OCCUPIED;
                response->game_type=1;
                response->matchedPlayer[0]=matchedPlayer_0;
                response->matchedPlayer[1]=matchedPlayer_1;
                response->matchedPlayer[2]=matchedPlayer_2;
                availableCourtNumber=court_no;
                response->court_no=availableCourtNumber;
                
                break;
            }
        }
        if (chance==0){
            waitingDoublesPlayerCount++;
            if (waitingDoublesPlayerCount > WAITING_DOUBLES_POOL_SIZE){
                WAITING_DOUBLES_POOL_SIZE*=2;
                struct Player *temp = (struct Player*)realloc(doublesPlayerQueue, WAITING_DOUBLES_POOL_SIZE * sizeof(struct Player));
                if (temp == NULL) {
                    perror("Doubles Player Memory Reallocation Failure.\n");
                } else {
                    // realloc successful, newArr may point to the same or a new memory block
                    doublesPlayerQueue = temp; // update the array pointer if needed
                }
            }
            doublesPlayerQueue[waitingDoublesPlayerCount-1] = newPlayer;
            if (newPlayer.preference=='B' || newPlayer.preference=='b'){
                waitingSinglesPlayerCount++;
                if (waitingSinglesPlayerCount > WAITING_SINGLES_POOL_SIZE){
                    WAITING_SINGLES_POOL_SIZE*=2;
                    struct Player *temp = (struct Player*)realloc(singlesPlayerQueue, WAITING_SINGLES_POOL_SIZE * sizeof(struct Player));
                    if (temp == NULL) {
                        perror("Singles Player Memory Reallocation Failure.\n");
                    } else {
                        // realloc successful, newArr may point to the same or a new memory block
                        singlesPlayerQueue = temp; // update the array pointer if needed
                    }
                }
                singlesPlayerQueue[waitingSinglesPlayerCount-1] = newPlayer;
            }
        }
    } else{
        waitingDoublesPlayerCount++;
        if (waitingDoublesPlayerCount > WAITING_DOUBLES_POOL_SIZE){
            WAITING_DOUBLES_POOL_SIZE*=2;
            struct Player *temp = (struct Player*)realloc(doublesPlayerQueue, WAITING_DOUBLES_POOL_SIZE * sizeof(struct Player));
            if (temp == NULL) {
                perror("Doubles Player Memory Reallocation Failure.\n");
            } else {
                // realloc successful, newArr may point to the same or a new memory block
                doublesPlayerQueue = temp; // update the array pointer if needed
            }
        }
        doublesPlayerQueue[waitingDoublesPlayerCount-1] = newPlayer;
        // response->matchedPlayer=NULL;
    }
    response->court_no=availableCourtNumber;
    return response;
}
struct Response* checkSinglesMatchScheduler(int time_stamp){
    int availableCourtNumber=-1;
    struct Response *response = (struct Response*)malloc(sizeof(struct Response)); 
    response->court_no=-1;
    response->game_type=-1;
    response->matchedPlayer=(struct Player*)malloc(2*sizeof(struct Player));
    struct Player newPlayer;
    if(waitingSinglesPlayerCount>1){
        for(int court_no=0;court_no<NO_OF_COURTS;court_no++){
            if (courts_status.court[court_no] == UNOCCUPIED){
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
                size_t i=0;
                
                //Remove the matched player from the queue
                if (newPlayer.preference == 'B' || newPlayer.preference == 'b'){
                    if(waitingDoublesPlayerCount>0){
                        int find_i=-1;
                        for (;i<waitingDoublesPlayerCount;i++){
                            if (newPlayer.player_id == doublesPlayerQueue[i].player_id){
                            find_i=i;
                            break; 
                            }
                        }
                    // singlesPlayerQueue[find_i]=NULL;
                        if (find_i != -1){
                                for (i = find_i; i < waitingDoublesPlayerCount - 1; i++){
                                    doublesPlayerQueue[i] = doublesPlayerQueue[i + 1];
                                }
                                waitingDoublesPlayerCount--;
                        }
                    }
                }
                
                for (i=0;i<waitingSinglesPlayerCount-1;i++){
                    singlesPlayerQueue[i]=singlesPlayerQueue[i+1];
                }
                waitingSinglesPlayerCount--;
                newPlayer.arrival_time=time_stamp;
                response->matchedPlayer[0]=newPlayer;
                break;
            }
        }
    }
    if (waitingSinglesPlayerCount>0){
        struct Response* temp_res = canFormSinglesMatch(&newPlayer);
        if(temp_res->court_no>=0){
            response->matchedPlayer[1]=temp_res->matchedPlayer[0];
            availableCourtNumber=temp_res->court_no;
        }
    }
    response->court_no=availableCourtNumber;
    return response;
}
struct Response* checkDoublesMatchScheduler(int time_stamp){
    int availableCourtNumber=-1;
    struct Response *response = (struct Response*)malloc(sizeof(struct Response)); 
    response->court_no=-1;
    response->game_type=-1;
    response->matchedPlayer=(struct Player*)malloc(4*sizeof(struct Player));
    struct Player newPlayer;
    if(waitingDoublesPlayerCount>3){
        for(int court_no=0;court_no<NO_OF_COURTS;court_no++){
            if (courts_status.court[court_no] == UNOCCUPIED){
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
                size_t i=0;
                if (newPlayer.preference == 'B' || newPlayer.preference == 'b'){
                    if(waitingSinglesPlayerCount>0){
                        int find_i=-1;
                        for (;i<waitingSinglesPlayerCount;i++){
                            if (newPlayer.player_id == singlesPlayerQueue[i].player_id){
                                find_i=i;
                            break; 
                            }
                        }
                    // singlesPlayerQueue[find_i]=NULL;
                        if (find_i != -1){
                                for (i = find_i; i < waitingSinglesPlayerCount - 1; i++){
                                    singlesPlayerQueue[i] = singlesPlayerQueue[i + 1];
                                }
                                waitingSinglesPlayerCount--;
                        }
                    }
                }
                //Remove the matched player from the queue
                
                for (i=0;i<waitingDoublesPlayerCount-1;i++){
                    doublesPlayerQueue[i]=doublesPlayerQueue[i+1];
                }
                waitingDoublesPlayerCount--;
                newPlayer.arrival_time=time_stamp;
                response->matchedPlayer[0]=newPlayer;
                break;
            }
        }
        if (waitingDoublesPlayerCount>2){
            struct Response* temp_res = canFormDoublesMatch(&newPlayer);
            if(temp_res->court_no>=0){
                response->matchedPlayer[1]=temp_res->matchedPlayer[0];
                response->matchedPlayer[2]=temp_res->matchedPlayer[1];
                response->matchedPlayer[3]=temp_res->matchedPlayer[2];
                availableCourtNumber=temp_res->court_no;
            }
        }
    }

    response->court_no=availableCourtNumber;
    return response;
}

void checkMatchScheduler(int time_stamp){
    printf("\nTime:   %d\t",time_stamp);
    
    pthread_mutex_lock(&scheduler_lock);
    struct Response *response = (struct Response*)malloc(sizeof(struct Response)); 
    response->court_no=-1;
    response->game_type=-1;
    for (int i=0; i<NO_OF_COURTS; i++){
        if ((time_stamp+1 > (courts_status.last_game_end_time[i])) && (courts_status.court[i] == OCCUPIED)){
            courts_status.court[i] = UNOCCUPIED;
        } 
    }
    printf("Free Courts:\t");
    for (int i=0;i<NO_OF_COURTS;i++){
        if(courts_status.court[i]==UNOCCUPIED)
            printf("%d\t",i+1);
    }
    printf("\n");
    for (int k=0; k<NO_OF_COURTS;++k){
        if (courts_status.court[k]==UNOCCUPIED){
            if(waitingDoublesPlayerCount>3){
                response = checkDoublesMatchScheduler(time_stamp);
                if (response){
                    if (response->court_no>=0){
                        write_csv(1,time_stamp,courts_status.last_game_end_time[response->court_no], response->court_no+1,response->matchedPlayer[0].player_id,response->matchedPlayer[1].player_id,response->matchedPlayer[2].player_id, response->matchedPlayer[3].player_id);
                    }
                }
            } else if(waitingSinglesPlayerCount>1){
                response = checkSinglesMatchScheduler(time_stamp);
                if(response){
                    if (response->court_no>=0){
                        write_csv(0,time_stamp,courts_status.last_game_end_time[response->court_no], response->court_no+1,response->matchedPlayer[0].player_id,response->matchedPlayer[1].player_id,0,0);
                    }
                }
            }
        }
    }
    printf("Singles Queue Size:  %d\n",waitingSinglesPlayerCount);
    printf("Singles Queue: ");
    for(int i=0;i<waitingSinglesPlayerCount;i++){
        printf("%d ",singlesPlayerQueue[i].player_id);
    }
    printf("\n");
    printf("Doubles Queue Size:  %d\n",waitingDoublesPlayerCount);
    printf("Doubles Queue: ");
    for(int i=0;i<waitingDoublesPlayerCount;i++){
        printf("%d ",doublesPlayerQueue[i].player_id);
    }
    printf("\n");
    pthread_mutex_unlock(&scheduler_lock);
}
struct Response* canFormMatch(struct Player *nPlayer)
{
    pthread_mutex_lock(&request_lock);
    
    struct Player newPlayer = *nPlayer;
    printf("\nTime:   %d\t",newPlayer.arrival_time);
    printf("\nPlayer ID:  %d, Preference: %c, Gender:    %c\n",newPlayer.player_id,newPlayer.preference,newPlayer.gender);
    
    

    int availableCourtNumber=-1;
    // Handle court free event
    for (int i=0; i<NO_OF_COURTS; i++){
        if ((newPlayer.arrival_time+1 > (courts_status.last_game_end_time[i])) && (courts_status.court[i] == OCCUPIED)){
            courts_status.court[i] = UNOCCUPIED;
        }
    }
    printf("Free Courts:\t");
    for (int i=0;i<NO_OF_COURTS;i++){
        if(courts_status.court[i]==UNOCCUPIED)
            printf("%d\t",i+1);
    }
    struct Response *response = (struct Response*)malloc(sizeof(struct Response)); 
    response->court_no=-1;
    response->game_type=-1;
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
            if (waitingDoublesPlayerCount>2) {
                response = canFormDoublesMatch(nPlayer);
            } else if (waitingSinglesPlayerCount>0){
                response = canFormSinglesMatch(nPlayer);
            }else{
                waitingSinglesPlayerCount++;
                
                if (waitingSinglesPlayerCount > WAITING_SINGLES_POOL_SIZE){
                    WAITING_SINGLES_POOL_SIZE*=2;
                    struct Player *temp = (struct Player*)realloc(singlesPlayerQueue, WAITING_SINGLES_POOL_SIZE * sizeof(struct Player));
                    if (temp == NULL) {
                        perror("Singles Player Memory Reallocation Failure.\n");
                    } else {
                        // realloc successful, newArr may point to the same or a new memory block
                        singlesPlayerQueue = temp; // update the array pointer if needed
                    }
                }
                singlesPlayerQueue[waitingSinglesPlayerCount-1] = newPlayer;
                waitingDoublesPlayerCount++;
                if (waitingDoublesPlayerCount > WAITING_DOUBLES_POOL_SIZE){
                    WAITING_DOUBLES_POOL_SIZE*=2;
                    struct Player *temp = (struct Player *)realloc(doublesPlayerQueue, WAITING_DOUBLES_POOL_SIZE * sizeof(struct Player));
                    if (temp == NULL) {
                        perror("Doubles Player Memory Reallocation Failure.\n");
                    } else {
                        // realloc successful, newArr may point to the same or a new memory block
                        doublesPlayerQueue = temp; // update the array pointer if needed
                    }
                }
                doublesPlayerQueue[waitingDoublesPlayerCount-1] = newPlayer;
                // response->matchedPlayer=NULL;
                response->court_no=-1;
            }
            //response->court_no=availableCourtNumber;
            break;
        case 'b': //S ->D
            if (waitingSinglesPlayerCount>0){
                response = canFormSinglesMatch(nPlayer);
            } else if (waitingDoublesPlayerCount>2) {
                response = canFormDoublesMatch(nPlayer);
            }else{
                waitingSinglesPlayerCount++;
                if (waitingSinglesPlayerCount > WAITING_SINGLES_POOL_SIZE){
                    WAITING_SINGLES_POOL_SIZE*=2;
                    struct Player *temp = (struct Player*)realloc(singlesPlayerQueue, WAITING_SINGLES_POOL_SIZE * sizeof(struct Player));
                    if (temp == NULL) {
                        perror("Singles Player Memory Reallocation Failure.\n");
                    } else {
                        // realloc successful, newArr may point to the same or a new memory block
                        singlesPlayerQueue = temp; // update the array pointer if needed
                    }
                }
                singlesPlayerQueue[waitingSinglesPlayerCount-1] = newPlayer;
                waitingDoublesPlayerCount++;
                if (waitingDoublesPlayerCount > WAITING_DOUBLES_POOL_SIZE){
                    WAITING_DOUBLES_POOL_SIZE*=2;
                    struct Player *temp = (struct Player*)realloc(doublesPlayerQueue, WAITING_DOUBLES_POOL_SIZE * sizeof(struct Player));
                    if (temp == NULL) {
                        perror("Doubles Player Memory Reallocation Failure.\n");
                    } else {
                        // realloc successful, newArr may point to the same or a new memory block
                        doublesPlayerQueue = temp; // update the array pointer if needed
                    }
                }
                doublesPlayerQueue[waitingDoublesPlayerCount-1] = newPlayer;
                // response->matchedPlayer=NULL;
                response->court_no=-1;
            }
            break;
        default:
            printf("Singles Queue Size:  %d\n",waitingSinglesPlayerCount);
                printf("Singles Queue: ");
                for(int i=0;i<waitingSinglesPlayerCount;i++){
                    printf("%d ",singlesPlayerQueue[i].player_id);
                }
                printf("\n");
                printf("Doubles Queue Size:  %d\n",waitingDoublesPlayerCount);
                printf("Doubles Queue: ");
                for(int i=0;i<waitingDoublesPlayerCount;i++){
                    printf("%d ",doublesPlayerQueue[i].player_id);
                }
                printf("\n");
                pthread_mutex_unlock(&request_lock);
                return NULL;
    }
    printf("\nSingles Queue Size:  %d\n",waitingSinglesPlayerCount);
    printf("Singles Queue: ");
    for(int i=0;i<waitingSinglesPlayerCount;i++){
        printf("%d ",singlesPlayerQueue[i].player_id);
    }
    printf("\n");
    printf("Doubles Queue Size:  %d\n",waitingDoublesPlayerCount);
    printf("Doubles Queue: ");
    for(int i=0;i<waitingDoublesPlayerCount;i++){
        printf("%d ",doublesPlayerQueue[i].player_id);
    }
    printf("\n");
    pthread_mutex_unlock(&request_lock);
    return response;
}
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



/* Handles client threads */
void *handleClient(void *arg) {
    pthread_mutex_lock(&client_lock);
    int newSocket = *(int *)arg;
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

        if (strcmp(buffer, ":exit") == 0) {
            printf("Client disconnected.\n");
            break;
        } else {
            // Parse the received data
            char gender, preference;
            int player_id,arrival_time;
            sscanf(buffer, "%d %d %c %c", &player_id, &arrival_time, &gender, &preference);
            if(server_time_stamp<=arrival_time){
                for(int i=server_time_stamp;i<=arrival_time;i++){
                    checkMatchScheduler(i);
                }
                server_time_stamp=arrival_time+1;
            }
            if(player_id==0 && gender=='X' && preference=='X'){
                printf("Exiting...\n");
                exit(0);
            }
            // server_time_stamp++;
            //printf("Server Time: %d, Arrival Time: %d\n",server_time_stamp,arrival_time);
            // Handle the request based on req_type
            // printf("%d %d %c %c\n", player_id, arrival_time, gender, preference);
            struct Player *newPlayer = (struct Player *)malloc(sizeof(struct Player));
            newPlayer->player_id = player_id;
            newPlayer->arrival_time = arrival_time;
            newPlayer->gender = gender;
            newPlayer->preference = preference;
            struct Response *response = canFormMatch(newPlayer);
            if (response!=NULL){
                if(response->court_no>=0){
                    if(response->game_type == 0){
                        //printf("O/P: %d, %d, %d, %d\n",newPlayer->arrival_time, response->court_no+1, newPlayer->player_id,response->matchedPlayer[0].player_id);
                        write_csv(0,newPlayer->arrival_time,courts_status.last_game_end_time[response->court_no], response->court_no+1,response->matchedPlayer[0].player_id,newPlayer->player_id,0,0);
                    } else{
                        //printf("O/P: %d, %d, %d, %d, %d, %d\n",newPlayer->arrival_time, response->court_no+1, newPlayer->player_id,response->matchedPlayer[0].player_id,response->matchedPlayer[1].player_id,response->matchedPlayer[2].player_id);
                        write_csv(1,newPlayer->arrival_time,courts_status.last_game_end_time[response->court_no], response->court_no+1,response->matchedPlayer[0].player_id,response->matchedPlayer[1].player_id,response->matchedPlayer[2].player_id, newPlayer->player_id);

                    }
                }
            }
            close(newSocket);
            clientCount--;
            printf("Current Count of Connected Clients: %d\n",clientCount);
            bzero(buffer, sizeof(buffer));
        }
    }
    
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
    fprintf(file,"Game-start-time,Game-end-time,Court-Number,List-of-player-ids,\n");
    fclose(file);
    pthread_mutex_init(&client_lock, NULL);
    pthread_mutex_init(&write_lock, NULL);
    pthread_mutex_init(&request_lock, NULL);
    initRequestQueue(&requestQueue);
    init_courts();
    init_queues();

    int num_threads = 10000;
    tid = malloc(num_threads*sizeof(pthread_t));
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
    printf("Current Count of Clients: %d\n",clientCount);
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

    close(sockfd);
    printf("Exited\n");
    return 0;
}