


#	Concurrent Programming on Tennis Game Request Management
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
| 8         | 4            | M      | b          |
| 9         | 4            | F      | D          |
| 10        | 5            | M      | D          |
| 11        | 5            | F      | D          |
| 12        | 5            | F      | D          |
| 13        | 5            | M      | S          |
| 14        | 10           | M      | B          |
| 15        | 10           | F      | D          |
| 16        | 11           | M      | b          |
| 17        | 12           | M      | S          |
| 18        | 12           | F      | B          |
| 19        | 14           | F      | b          |
| 20        | 14           | F      | S          |
| 41        | 15           | M      | B          |
| 42        | 15           | F      | S          |
| 43        | 15           | M      | S          |
| 44        | 16           | F      | D          |
| 45        | 16           | M      | D          |
| 46        | 17           | F      | S          |
| 47        | 17           | M      | B          |
| 48        | 18           | F      | D          |
| 49        | 18           | M      | S          |
| 50        | 19           | F      | B          |
| 51        | 19           | F      | S          |
| 52        | 20           | M      | B          |
| 53        | 20           | M      | D          |
| 54        | 21           | F      | D          |
| 55        | 21           | M      | D          |
| 56        | 22           | F      | B          |
| 57        | 22           | F      | S          |
| 58        | 23           | M      | S          |
| 59        | 24           | M      | D          |
| 60        | 24           | F      | S          |
| 0         | 69           | X      | X          |

  **Please Note**: If there's only a single .csv file to be executed, please insert a dummy row at the end of all input rows with the following pattern: <Player-ID=0, Arrival-time=Last Request Time+45, Gender=X, Preference=X>
  For e.g.: 0,69,X,X
  And if there are multiple .csv files to be executed, then insert the dummy row only to the last .csv file to be executed.

**Sample Output:**

Each line corresponds to a game played.  

| Game-start-time | Game-end-time | Court-Number | List-of-player-ids                  |
|------------------|---------------|--------------|-------------------------------------|
| 2                | 12            | 1            | 4, 5                                |
| 3                | 18            | 2            | 1, 2, 3, 6                          |
| 4                | 14            | 3            | 7, 8                                |
| 5                | 20            | 4            | 9, 10, 11, 12                       |
| 12               | 22            | 1            | 13, 14                              |
| 14               | 24            | 3            | 16, 17                              |
| 18               | 33            | 2            | 15, 18, 19, 41                      |
| 20               | 35            | 4            | 44, 45, 47, 48                      |
| 22               | 37            | 1            | 50, 52, 53, 54                      |
| 24               | 29            | 3            | 20, 42                              |
| 29               | 39            | 3            | 43, 46                              |
| 33               | 43            | 2            | 49, 51                              |
| 35               | 40            | 4            | 56, 57                              |
| 37               | 47            | 1            | 58, 60                              |



  
# Used Language

C
# Compiling & Executing the Code
The code to manage the requests is divided into a client-server concurrent programming model.
Each request is sent by the client-side to the server, where the server processes the game-requests accordingly,
We should start the server first before starting the clients.
## Server Code
### Compilation
```bash
$ gcc -o server Server-Tennis.c -pthread
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
$ ./client
```
# Explanation
The server relies on the thread-pool concept; upon receipt of a request, the server assigns a thread from the thread pool to process its request. It runs continuously until it gets the dummy input row. If you don't provide the dummy row as input, the server will not look into the pending waiting queues that may be processed after the last request arrival-time. 
It used Pthreads C library to manage each request using concurrent multi-threading. More on Pthreads: [man-pthreads](https://www.man7.org/linux/man-pages/man7/pthreads.7.html).

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
Incase, if the court becomes free, ties between between Singles and Doubles are given over to Doubles.
