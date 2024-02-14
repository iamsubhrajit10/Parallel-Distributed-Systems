


#	Concurrent Programming on Tennis Game Request Management using OpenMP
There are  **4 Tennis courts** and players arrive randomly to play.

  

**Characteristics of the player:**

1.  Gender: M or F
2.  Game Preference: S, D, b, B  
    
    S -> Singles (2 players),  
    
    D -> Doubles (4 players),  
    
    b -> first preference S and second preference D  
    
    B -> first preference D and second preference S
    

Each game lasts for 'x' minutes where x is:

-   Singles Game

-   Male: 10 min
-   Female: 5 min

-   Doubles Game

-   Male: 15 min
-   Female: 10 min

Write a program that implements this scenario.


A game can consist of both M and F players. In such a case, take game time to be that of a M player.
- The server should return a response to the client only after the match is over.
- The maximum waiting time for the client to get allotted a match is 30 minutes, after which the client should receive a response indicating the timeout.
- Each line of the input is an independent client instance.
  

The input for the program must be taken from a .csv file.   

**Sample input:**

**Each line corresponds to one player and their attributes.**
Player-ID & Arrival-time starts from 1 for valid inputs. Player-ID is unique across all the requests. Arrival-time should be in non-decreasing order (inside an .csv file as well as across the multiple .csv files). For e.g. if you're running input1.csv, input2.csv and input3.csv one by one, then first input's Arrival-time of input2.csv file should be >= the last input's Arrival-time of input1.csv.
| Player-ID | Arrival-time | Gender | Preference |
|-----------|--------------|--------|------------|
| 1         | 1            | M      | D          |
| 2         | 1            | M      | D          |
| 3         | 1            | F      | D          |
| 4         | 1            | F      | S          |
| 5         | 2            | M      | S          |
| 6         | 3            | F      | D          |
| 7         | 3            | F      | B          |
| 8         | 4            | M      | B          |
| 9         | 4            | F      | D          |
| 10        | 5            | M      | D          |
| 11        | 5            | F      | D          |
| 12        | 5            | F      | D          |
| 13        | 5            | M      | S          |
| 14        | 10           | M      | B          |
| 15        | 10           | F      | D          |
| 16        | 11           | M      | B          |
| 17        | 12           | M      | S          |
| 18        | 12           | F      | B          |
| 19        | 14           | F      | B          |
| 20        | 14           | F      | S          |
| 21        | 15           | M      | B          |
| 22        | 15           | F      | S          |
| 23        | 15           | M      | S          |
| 24        | 16           | F      | D          |
| 25        | 16           | M      | D          |
| 26        | 17           | F      | S          |
| 27        | 17           | M      | B          |
| 28        | 18           | F      | D          |
| 29        | 18           | M      | S          |
| 30        | 19           | F      | B          |
| 31        | 19           | F      | S          |
| 32        | 20           | M      | B          |
| 33        | 20           | M      | D          |
| 34        | 21           | F      | D          |
| 35        | 21           | M      | D          |
| 36        | 22           | F      | B          |
| 37        | 22           | F      | S          |
| 38        | 23           | M      | S          |
| 39        | 24           | M      | D          |
| 40        | 24           | F      | S          |
| 0         | 69           | X      | X          |


  **Please Note**: 
   - Please insert a dummy row at the end of all input rows with the following pattern: <Player-ID=0, Arrival-time=Last Request Time+45, Gender=X, Preference=X>
  For e.g.: 0,69,X,X.
   - Arrival times should start from 1, Player-ID should start from 1 (except dummy row).
   - Also, there shouldn't be any gap in the Player-IDs, i.e. Player-IDs should be sequential (1,2,3,...OK) (1,2,4,.. NOT OK)
  

**Sample Output:**

Each line corresponds to a game played.  

| Player-ID | Game-start-time | Game-end-time | Court-Number | List-of-player-ids |
|-----------|-----------------|---------------|--------------|--------------------|
| 5         | 2               | 12            | 1            | 4 5                |
| 4         | 2               | 12            | 1            | 4 5                |
| 7         | 4               | 14            | 3            | 7 8                |
| 8         | 4               | 14            | 3            | 7 8                |
| 1         | 3               | 18            | 2            | 1 2 3 6            |
| 3         | 3               | 18            | 2            | 1 2 3 6            |
| 6         | 3               | 18            | 2            | 1 2 3 6            |
| 2         | 3               | 18            | 2            | 1 2 3 6            |
| 11        | 5               | 20            | 4            | 9 10 11 12         |
| 12        | 5               | 20            | 4            | 9 10 11 12         |
| 10        | 5               | 20            | 4            | 9 10 11 12         |
| 9         | 5               | 20            | 4            | 9 10 11 12         |
| 14        | 12              | 22            | 1            | 14 13              |
| 13        | 12              | 22            | 1            | 14 13              |
| 17        | 14              | 24            | 3            | 17 16              |
| 16        | 14              | 24            | 3            | 17 16              |
| 20        | 24              | 29            | 3            | 22 20              |
| 22        | 24              | 29            | 3            | 22 20              |
| 15        | 18              | 33            | 2            | 18 19 21 15        |
| 18        | 18              | 33            | 2            | 18 19 21 15        |
| 19        | 18              | 33            | 2            | 18 19 21 15        |
| 21        | 18              | 33            | 2            | 18 19 21 15        |
| 25        | 20              | 35            | 4            | 25 27 28 24        |
| 27        | 20              | 35            | 4            | 25 27 28 24        |
| 28        | 20              | 35            | 4            | 25 27 28 24        |
| 24        | 20              | 35            | 4            | 25 27 28 24        |
| 33        | 22              | 37            | 1            | 32 33 34 30        |
| 32        | 22              | 37            | 1            | 32 33 34 30        |
| 30        | 22              | 37            | 1            | 32 33 34 30        |
| 34        | 22              | 37            | 1            | 32 33 34 30        |
| 26        | 29              | 39            | 3            | 26 23              |
| 23        | 29              | 39            | 3            | 26 23              |
| 37        | 35              | 40            | 4            | 37 36              |
| 36        | 35              | 40            | 4            | 37 36              |
| 31        | 33              | 43            | 2            | 31 29              |
| 29        | 33              | 43            | 2            | 31 29              |
| 40        | 37              | 47            | 1            | 40 38              |
| 38        | 37              | 47            | 1            | 40 38              |
| 35        | -1              | 52            |              |                    |
| 39        | -1              | 55            |              |                    |



  
# Used Language

C
# Compiling & Executing the Code
The code to manage the requests is divided into a client-server concurrent programming model.
Each request is sent by the client-side to the server, where the server processes the game-requests accordingly,
We should start the server first before starting the clients.
## Server Code
### Compilation
```bash
$ gcc -o server Server-Tennis.c -fopenmp
```
### Execution 
```bash
$ ./server
```
### Client Code
### Compilation
```bash
$ gcc -o client Client-Tennis.c
```
### Execution
```bash
$ ./client <input csv file name> <output csv file name?
```
# Explanation
## Server-side
The server relies on OpenMP threads to manage the clients concurrently; it creates as many threads as the number of clients (no. of rows in the csv file). The server runs continuously until it gets the dummy input row. If you don't provide the dummy row as input, the server will not look into the pending waiting queues that may be processed after the last request arrival-time. 
More on OpenMP: [OpenMP-org](openmp.org).

It majorly follows FCFS approach, i.e. upon arrival of a request, it instantly checks if any games can be arranged, if not it inserts the request onto the queue(s) based on preference.

The server maintains two queues: Singles Game Requests Queue and Doubles Game Requests Queue.
Based upon the preference of the request it does the following:
 

 - S
	 - It first checks if any of the tennis court is unoccupied or not
		 - Then, it checks the Singles Game Requests Queue if there's atleast one pending game request on the queue.
		 - If there's atleast one player available, it assigns the match with this player, and remove the player from the Queue(s) (based on preference)
		 - Otherwise, it inserts the request in the Single Game Requests Queue.
	 - Otherwise, it inserts it in the Single Game Requests Queue.
 - D
	 - It first checks if any of the tennis court is unoccupied or not
	    -	Then, it checks the Double Game Requests Queue if there’s atleast three pending game request on the queue.
	    - If there’s atleast three player available, it assigns the match with these players, and remove the player from the Queue(s) (based on preference)
	    - Otherwise, it inserts the request in the Double Game Requests Queue.
	-   Otherwise, it inserts it in the Double Game Requests Queue.
- B
	- It first checks if any of the tennis court is unoccupied or not
		- Then, it first checks for D
		- Then, it checks for S
		- If both are not the case, it inserts the requests on both the Queue
	- Otherwise, it inserts the requests on both the Queue
- b
	- It first checks if any of the tennis court is unoccupied or not
		- Then, it first checks for S
		- Then, it checks for D
		- If both are not the case, it inserts the requests on both the Queue
	- Otherwise, it inserts the requests on both the Queue

The server, in between the arrival-time gaps, checks if there's any game(s) possible.
Incase, if the court becomes free, ties between Singles and Doubles are given over to Doubles.
Also, the Queues are maintained according to arrival time. 

## Server-side code explanation:
### Accepting Connections:
The server waits for incoming connections using the accept function within a loop.
Upon accepting a connection, it enqueues the client's socket into a request queue for further processing.
### Handling Clients in Parallel:
The server handles each client in a separate thread using OpenMP parallelism.
It creates a thread pool with the number of threads equal to the number of clients.
Each thread continuously dequeues a client's socket from the request queue and processes it.
### Client Handling:

The handleClient function is responsible for managing the communication with the client.
It performs tasks such as receiving data from the client, processing requests, and sending responses.
### Parallel Execution:

OpenMP directives are used to parallelize the processing of client connections, allowing multiple clients to be handled concurrently.


## Client-side
The client-code treats each row of the input-csv file independently. It relies on forking child process for each client request, while managing synchronization among the processes using shared-memory IPC, semaphores.
Client-side Code Explanation:

### Initializing Shared Memory and Semaphores:
To facilitate synchronization between child processes, the program creates and initializes shared memory segments and semaphores. Keys for shared memory segments are generated, and semaphores are initialized to manage access to shared resources. The turn variable is also initialized to regulate the order of execution among child processes.

### Forking Child Processes:
The program forks child processes to handle individual records from the CSV file concurrently. Each child process calculates the sleep duration based on the difference in arrival times between records. Child processes wait until it's their turn to send data, ensuring orderly execution. Once their turn arrives, child processes acquire a semaphore to send data, update the turn variable for the next process, and receive a response upon completion of the match from the server before exiting.

### Waiting for Child Processes:
The parent process waits for all child processes to complete their tasks before proceeding. It does this to ensure that all client requests are processed before terminating the program.

### Cleanup:
After completing its tasks, the program frees memory allocated for storing CSV records, destroys semaphores, and detaches shared memory segments to release system resources.


