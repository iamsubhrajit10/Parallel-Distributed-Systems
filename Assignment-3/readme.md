# 	TennisServe: A Parallel Game Matching Server with OpenMP & MPI Tennis Game Request Server using OpenMP and MPI

There are  **4 Tennis courts** and players arrive randomly to play.


The input for the program must be taken from a .csv file.   
## Input format
The client requests must be in a csv file called "input.csv"
**Sample input:**

**Each line corresponds to one player and their attributes.**
Player-ID & Arrival-time starts from 1 for valid inputs. Player-ID is unique across all the requests. Arrival-time should be in non-decreasing order. 
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

  **Please Note**: 
   - Arrival times should start from 1, Player-ID should start from 1 (except dummy row).
   - Also, there shouldn't be any gap in the Player-IDs, i.e. Player-IDs should be sequential (1,2,3,...OK) (1,2,4,.. NOT OK)
After a game is over, one player/ team is randomly chosen as a winner.
Using MPI, the loser(s) should send a congratulatory message to the winner(s), and the winner(s) should send a thank you message back.
**Sample Output:**

Each line corresponds to a game played.  
| Player-ID | Game-start-time | Game-end-time | Court-Number | List-of-player-ids |         |           |           |
|---------|---------|---------|---------|---------|---------|---------|---------| 
| 4       | 2       | 12      | 1       | 4       | 5       |         |         | 
| 5       | 5 To 4  | 4: Congratulations on your win! |         |         |         |         |         | 
| 4       | 4 To 5  | 5: Thank you for the game!       |         |         |         |         |         | 
| 5       | 2       | 12      | 1       | 4       | 5       |         |         | 
| 5       | 5 To 4  | 4: Congratulations on your win! |         |         |         |         |         | 
| 4       | 4 To 5  | 5: Thank you for the game!       |         |         |         |         |         | 
| 7       | 4       | 14      | 3       | 7       | 8       |         |         | 
| 8       | 8 To 7  | 7: Congratulations on your win! |         |         |         |         |         | 
| 7       | 7 To 8  | 8: Thank you for the game!       |         |         |         |         |         | 
| 8       | 4       | 14      | 3       | 7       | 8       |         |         | 
| 8       | 8 To 7  | 7: Congratulations on your win! |         |         |         |         |         | 
| 7       | 7 To 8  | 8: Thank you for the game!       |         |         |         |         |         | 
| 1       | 3       | 18      | 2       | 2       | 3       | 1       | 6       | 
| 2       | 2 To 1  | 1: Congratulations on your win! |         |         |         |         |         | 
| 1       | 1 To 2  | 2: Thank you for the game!       |         |         |         |         |         | 
| 3       | 3 To 1  | 1: Congratulations on your win! |         |         |         |         |         | 
| 1       | 1 To 3  | 3: Thank you for the game!       |         |         |         |         |         | 
| 2       | 3       | 18      | 2       | 2       | 3       | 1       | 6       | 
| 2       | 2 To 1  | 1: Congratulations on your win! |         |         |         |         |         | 
| 1       | 1 To 2  | 2: Thank you for the game!       |         |         |         |         |         | 
| 2       | 2 To 6  | 6: Congratulations on your win! |         |         |         |         |         | 
| 6       | 6 To 2  | 2: Thank you for the game!       |         |         |         |         |         | 
| 3       | 3       | 18      | 2       | 2       | 3       | 1       | 6       | 
| 3       | 3 To 1  | 1: Congratulations on your win! |         |         |         |         |         | 
| 1       | 1 To 3  | 3: Thank you for the game!       |         |         |         |         |         | 
| 3       | 3 To 6  | 6: Congratulations on your win! |         |         |         |         |         | 
| 6       | 6 To 3  | 3: Thank you for the game!       |         |         |         |         |         | 
| 6       | 3       | 18      | 2       | 2       | 3       | 1       | 6       | 
| 2       | 2 To 6  | 6: Congratulations on your win! |         |         |         |         |         | 
| 6       | 6 To 2  | 2: Thank you for the game!       |         |         |         |         |         | 
| 3       | 3 To 6  | 6: Congratulations on your win! |         |         |         |         |         | 
| 6       | 6 To 3  | 3: Thank you for the game!       |         |         |         |         |         | 
| 9       | 5       | 20      | 4       | 9       | 12      | 10      | 11      | 
| 10      | 5       | 20      | 4       | 9       | 12      | 10      | 11      | 
| 12      | 5       | 20      | 4       | 9       | 12      | 10      | 11      | 
| 11      | 5       | 20      | 4       | 9       | 12      | 10      | 11      | 
| 14      | 12      | 22      | 1       | 14      | 13      |         |         | 
| 14      | 14 To 13 | 13: Congratulations on your win! |         |         |         |         |         | 
| 13      | 13 To 14 | 14: Thank you for the game!       |         |         |         |         |         | 
| 13      | 12      | 22      | 1       | 14      | 13      |         |         | 
| 14      | 14 To 13 | 13: Congratulations on your win! |         |         |         |         |         | 
| 13      | 13 To 14 | 14: Thank you for the game!       |         |         |         |         |         | 
| 17      | 14      | 24      | 3       | 17      | 16      |         |         | 
| 16      | 16 To 17 | 17: Congratulations on your win! |         |         |         |         |         | 
| 17      | 17 To 16 | 16: Thank you for the game!       |         |         |         |         |         | 
| 16      | 14      | 24      | 3       | 17      | 16      |         |         | 
| 16      | 16 To 17 | 17: Congratulations on your win! |         |         |         |         |         | 
| 17      | 17 To 16 | 16: Thank you for the game!       |         |         |         |         |         | 
| 22      | 24      | 29      | 3       | 22      | 20      |         |         | 
| 20      | 20 To 22 | 22: Congratulations on your win! |         |         |         |         |         | 
| 22      | 22 To 20 | 20: Thank you for the game!       |         |         |         |         |         | 
| 20      | 24      | 29      | 3       | 22      | 20      |         |         | 
| 20      | 20 To 22 | 22: Congratulations on your win! |         |         |         |         |         | 
| 22      | 22 To 20 | 20: Thank you for the game!       |         |         |         |         |         | 
| 21      | 18      | 33      | 2       | 18      | 19      | 21      | 15      | 
| 18      | 18 To 21 | 21: Congratulations on your win! |         |         |         |         |         | 
| 21      | 21 To 18 | 18: Thank you for the game!       |         |         |         |         |         | 
| 19      | 19 To 21 | 21: Congratulations on your win! |         |         |         |         |         | 
| 21      | 21 To 19 | 19: Thank you for the game!       |         |         |         |         |         | 
| 18      | 18 To 21 | 21: Congratulations on your win! |         |         |         |         |         | 
| 21      | 21 To 18 | 18: Thank you for the game!       |         |         |         |         |         | 
| 18      | 18 To 15 | 15: Congratulations on your win! |         |         |         |         |         | 
| 15      | 15 To 18 | 18: Thank you for the game!       |         |         |         |         |         | 
| 15      | 18      | 33      | 2       | 18      | 19      | 21      | 15      | 
| 18      | 18 To 15 | 15: Congratulations on your win! |         |         |         |         |         | 
| 15      | 15 To 18 | 18: Thank you for the game!       |         |         |         |         |         | 
| 19      | 19 To 15 | 15: Congratulations on your win! |         |         |         |         |         | 
| 15      | 15 To 19 | 19: Thank you for the game!       |         |         |         |         |         | 
| 19      | 18      | 33      | 2       | 18      | 19      | 21      | 15      | 
| 19      | 19 To 21 | 21: Congratulations on your win! |         |         |         |         |         | 
| 21      | 21 To 19 | 19: Thank you for the game!       |         |         |         |         |         | 
| 19      | 19 To 15 | 15: Congratulations on your win! |         |         |         |         |         | 
| 15      | 15 To 19 | 19: Thank you for the game!       |         |         |         |         |         | 
| 28      | 20      | 35      | 4       | 24      | 27      | 28      | 25      | 
| 24      | 24 To 28 | 28: Congratulations on your win! |         |         |         |         |         | 
| 28      | 28 To 24 | 24: Thank you for the game!       |         |         |         |         |         | 
| 27      | 27 To 28 | 28: Congratulations on your win! |         |         |         |         |         | 
| 28      | 28 To 27 | 27: Thank you for the game!       |         |         |         |         |         | 
| 25      | 20      | 35      | 4       | 24      | 27      | 28      | 25      | 
| 24      | 24 To 25 | 25: Congratulations on your win! |         |         |         |         |         | 
| 25      | 25 To 24 | 24: Thank you for the game!       |         |         |         |         |         | 
| 27      | 27 To 25 | 25: Congratulations on your win! |         |         |         |         |         | 
| 25      | 25 To 27 | 27: Thank you for the game!       |         |         |         |         |         | 
| 27      | 20      | 35      | 4       | 24      | 27      | 28      | 25      | 
| 27      | 27 To 28 | 28: Congratulations on your win! |         |         |         |         |         | 
| 28      | 28 To 27 | 27: Thank you for the game!       |         |         |         |         |         | 
| 27      | 27 To 25 | 25: Congratulations on your win! |         |         |         |         |         | 
| 25      | 25 To 27 | 27: Thank you for the game!       |         |         |         |         |         | 
| 24      | 20      | 35      | 4       | 24      | 27      | 28      | 25      | 
| 24      | 24 To 28 | 28: Congratulations on your win! |         |         |         |         |         | 
| 28      | 28 To 24 | 24: Thank you for the game!       |         |         |         |         |         | 
| 24      | 24 To 25 | 25: Congratulations on your win! |         |         |         |         |         | 
| 25      | 25 To 24 | 24: Thank you for the game!       |         |         |         |         |         | 
| 35      | 22      | 37      | 1       | 33      | 32      | 35      | 30      | 
| 33      | 33 To 35 | 35: Congratulations on your win! |         |         |         |         |         | 
| 35      | 35 To 33 | 33: Thank you for the game!       |         |         |         |         |         | 
| 32      | 32 To 35 | 35: Congratulations on your win! |         |         |         |         |         | 
| 35      | 35 To 32 | 32: Thank you for the game!       |         |         |         |         |         | 
| 30      | 22      | 37      | 1       | 33      | 32      | 35      | 30      | 
| 33      | 33 To 30 | 30: Congratulations on your win! |         |         |         |         |         | 
| 30      | 30 To 33 | 33: Thank you for the game!       |         |         |         |         |         | 
| 32      | 32 To 30 | 30: Congratulations on your win! |         |         |         |         |         | 
| 30      | 30 To 32 | 32: Thank you for the game!       |         |         |         |         |         | 
| 32      | 22      | 37      | 1       | 33      | 32      | 35      | 30      | 
| 32      | 32 To 35 | 35: Congratulations on your win! |         |         |         |         |         | 
| 35      | 35 To 32 | 32: Thank you for the game!       |         |         |         |         |         | 
| 32      | 32 To 30 | 30: Congratulations on your win! |         |         |         |         |         | 
| 30      | 30 To 32 | 32: Thank you for the game!       |         |         |         |         |         | 
| 33      | 22      | 37      | 1       | 33      | 32      | 35      | 30      | 
| 33      | 33 To 35 | 35: Congratulations on your win! |         |         |         |         |         | 
| 35      | 35 To 33 | 33: Thank you for the game!       |         |         |         |         |         | 
| 33      | 33 To 30 | 30: Congratulations on your win! |         |         |         |         |         | 
| 30      | 30 To 33 | 33: Thank you for the game!       |         |         |         |         |         | 
| 26      | 29      | 39      | 3       | 26      | 23      |         |         | 
| 23      | 23 To 26 | 26: Congratulations on your win! |         |         |         |         |         | 
| 26      | 26 To 23 | 23: Thank you for the game!       |         |         |         |         |         | 
| 23      | 29      | 39      | 3       | 26      | 23      |         |         | 
| 23      | 23 To 26 | 26: Congratulations on your win! |         |         |         |         |         | 
| 26      | 26 To 23 | 23: Thank you for the game!       |         |         |         |         |         | 
| 36      | 35      | 40      | 4       | 36      | 37      |         |         | 
| 37      | 37 To 36 | 36: Congratulations on your win! |         |         |         |         |         | 
| 36      | 36 To 37 | 37: Thank you for the game!       |         |         |         |         |         | 
| 37      | 35      | 40      | 4       | 36      | 37      |         |         | 
| 37      | 37 To 36 | 36: Congratulations on your win! |         |         |         |         |         | 
| 36      | 36 To 37 | 37: Thank you for the game!       |         |         |         |         |         | 
| 31      | 33      | 43      | 2       | 31      | 29      |         |         | 
| 29      | 29 To 31 | 31: Congratulations on your win! |         |         |         |         |         | 
| 31      | 31 To 29 | 29: Thank you for the game!       |         |         |         |         |         | 
| 29      | 33      | 43      | 2       | 31      | 29      |         |         | 
| 29      | 29 To 31 | 31: Congratulations on your win! |         |         |         |         |         | 
| 31      | 31 To 29 | 29: Thank you for the game!       |         |         |         |         |         | 
| 38      | 37      | 47      | 1       | 40      | 38      |         |         | 
| 38      | 38 To 40 | 40: Congratulations on your win! |         |         |         |         |         | 
| 40      | 40 To 38 | 38: Thank you for the game!       |         |         |         |         |         | 
| 40      | 37      | 47      | 1       | 40      | 38      |         |         | 
| 38      | 38 To 40 | 40: Congratulations on your win! |         |         |         |         |         | 
| 40      | 40 To 38 | 38: Thank you for the game!       |         |         |         |         |         | 
| 34      | -1      | 52      |         |         |         |         |         | 
| 39      | -1      | 55      |         |         |         |         |         | 




  
# Used Language

C
# Compiling & Executing the Code
The code to manage the requests is divided into a client-server concurrent programming model.
Each request is sent by the client-side to the server, where the server processes the game-requests accordingly,
We should start the server first before starting the clients.
## Server Code
### Compilation
```bash
$ gcc -o server server.c -fopenmp
```
### Execution 
```bash
$ ./server
```
### Client Code
### Compile & Execute
```bash
$ ./client_script.sh
```
# Explanation
## Server-side
The server relies on OpenMP threads to manage the clients concurrently; it creates as many threads as the number of clients (no. of rows in the csv file). The server runs continuously until it gets the dummy input row. If you don't provide the dummy row as input, the server will not look into the pending waiting queues that may be processed after the last request arrival-time. 
More on OpenMP: [OpenMP-org](https://openmp.org).

It majorly follows FCFS approach, i.e. upon arrival of a request, it instantly checks if any games can be arranged, if not it inserts the request onto the queue(s) based on preference.

The server maintains two queues: Singles Game Requests Queue and Doubles Game Requests Queue.
Logic of the server is as same as assingment-2.

After the completion of each game, the server randomly decides one winner team from the game, passes the winner to the involved players.

## Client-side
The code is divided into two parts: one for the master process (when the process rank is 0) and one for the worker processes.

1. **Master Process**: The master process, process 0, iterates over a set of records. For each record, it broadcasts the `arrival_time` to all other processes in the MPI world (all processes that are part of the MPI program). After broadcasting the `arrival_time`, it sleeps for a second to allow for synchronization.

2. **Worker Processes**: Each worker process first sleeps for a second to allow for synchronization. Then, it receives the `arrival_time` broadcasted by the master process. It calculates its record number based on its rank in the MPI world. If the `arrival_time` in its record is greater than the `arrival_time` received from the master, it waits until the two match. This is done by sleeping for the difference between the two times. If it's the last record, it sleeps for the difference between the `arrival_time` of the last and the first record.

This code ensures that all processes start working on their records at the same time, based on the `arrival_time` broadcasted by the master process.

And, with the winner id communicated by the server, the respective processes communicate with each other with appropriate messages through MPI Send and MPI Receive calls.


