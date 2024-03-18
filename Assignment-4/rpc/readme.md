# Word Count with MapReduce and RPC

MapReduce is a programming model that allows for distributed processing of large data sets across clusters of computers. In this case, we've implemented how a word count task can be performed using MapReduce and Remote Procedure Calls (RPC).

## Map Phase

1. **Task Distribution**: The Coordinator, implemented in `$(pwd)/src/mr/coordinator.go`, distributes the map tasks to available Workers. Each map task corresponds to a file that needs to be processed. This distribution is done using RPC, a method that allows for function calls between different processes.

2. **Word Counting**: Each Worker, implemented in `$(pwd)/src/mr/worker.go`, reads its assigned file and applies the map function. In the context of a word count task, the map function takes a document as input and produces a list of key-value pairs as output, where each key is a word from the document and the value is 1.

3. **Intermediate Results**: The output of the map function is written to intermediate files. Each Worker writes its output to a separate file for each reduce task. The file to write to is determined by applying a hash function to the key.

## Reduce Phase

1. **Task Distribution**: Once all map tasks are complete, the Coordinator begins to distribute the reduce tasks to the Workers. Each reduce task corresponds to a group of intermediate files that share the same hash value.

2. **Aggregation**: Each Worker reads its assigned intermediate files and applies the reduce function. In the context of a word count task, the reduce function takes a key and a list of values as input and combines the values for each key. In this case, it sums up the values (all 1s) for each word, effectively counting the occurrences of each word.

3. **Final Results**: The output of the reduce function is written to output files, one for each reduce task. The final result of the word count task is the union of all output files.

## Fault Tolerance

The Coordinator monitors the progress of all tasks. If a Worker fails or takes too long to complete a task, the Coordinator can reassign the task to a different Worker. This ensures that all tasks are eventually completed, even in the presence of Worker failures or slow Workers.

In summary, the word count task is performed using MapReduce and RPC by distributing the task across multiple Workers, each of which performs a part of the task and produces intermediate results. These results are then aggregated to produce the final result. The Coordinator manages the distribution and monitoring of tasks, ensuring that all tasks are completed.

# Execution
To execute the module, we may run the test cases script directly: 'test-mr-many.sh', which build all the necessary modules and run the program 
```bash
sudo ./test-mr-many.sh <num-of-trials>
```
## Note
The 'test-mr-many.sh' script was written in an windows environment, to make it compatible to run in linux environment please use dos2unix
```bash
dos2unix test-mr-many.sh
```
Disable Go Modules and set your GOPATH by using the following commands
```bash
export GO111MODULE=off
export GOPATH=$(pwd)
```
# coordinator.go

`coordinator.go` implements a Coordinator struct that manages the distribution and monitoring of tasks for a MapReduce job. The MapReduce job could be any task that follows the MapReduce model, such as a word count problem. The communication between the Coordinator and the workers is done using Remote Procedure Calls (RPC).

## Workflow

1. **Initialization**: The `MakeCoordinator` function initializes a new Coordinator with a list of files to process and a number of reduce tasks. It creates a map task for each file and a reduce task for each reduce task index, all with an initial status of "left". It then starts the RPC server for the Coordinator.

2. **Task Request**: The `ProvideTask` method is called by a worker to request a task. The Coordinator checks if there are any map tasks available. If there are, it assigns a map task to the worker and updates the task status to "running map". If there are no map tasks but there are reduce tasks available, it assigns a reduce task to the worker and updates the task status to "running reduce". If there are no tasks available, it assigns "exit" to the worker, indicating that there are no more tasks.

3. **Task Allocation**: The `allocateTask` method is used by `ProvideTask` to allocate a task to a worker. It searches for a task with a status of "left", marks it as "running", and returns the task details to the worker. It also starts monitoring the task in a separate goroutine.

4. **Task Monitoring**: The `monitorTask` function monitors a task. If the task is not completed after a certain duration, it marks the task as "left" again, making it available for another worker to pick up.

5. **Task Completion**: The `TaskDone` method is called by a worker to signal that a task is done. The Coordinator marks the task as "completed" and updates the task counts accordingly.

6. **Job Completion**: The `Done` method checks if all map and reduce tasks have been completed. It returns true if all tasks are done, otherwise false.

This process continues until all map and reduce tasks are completed. The Coordinator ensures that all tasks are eventually completed, even if some workers fail or are slow.

# rpc.go

`rpc.go` defines the data structures and constants used for Remote Procedure Call (RPC) communication between the Coordinator and the Workers in the MapReduce implementation.

## Contents

1. **ExampleArgs and ExampleReply**: These are example structures for RPC arguments and replies.

2. **WorkerTaskReply and WorkerTaskArgs**: These structures define the reply and arguments for a worker task. They include fields for the task type, file to process, and the number of reduce tasks.

3. **WorkerDoneReply**: This structure defines the reply for a worker done signal. It includes a boolean field to indicate whether the worker should exit.

4. **Constants**: These constants represent different message types used in the communication.

5. **Args and Reply**: These structures define the arguments and reply for an RPC message. They include fields for the message type, message count, file name, number of map tasks, number of reduce tasks, type of task, and a list of intermediate files for reduce tasks.

6. **InterFile**: This structure represents the intermediate file message. It includes fields for the message type, message count, and type of reduce task.

7. **coordinatorSock**: This function returns the coordinator socket path. It's used for establishing the RPC connection between the Coordinator and the Workers.

In summary, `rpc.go` defines the data structures and constants used for RPC communication in the MapReduce implementation.

# worker.go

## Initialization
The `worker.go` file is part of the worker package, which contains functions and structures necessary for performing MapReduce tasks. It imports required packages such as `fmt`, `hash/fnv`, `os`, `strconv`, and `time`.

## KeyValue Struct
The `KeyValue` struct represents a key-value pair used during the map and reduce phases of the MapReduce process.

## ByKey Interface
The `ByKey` interface implements sorting for `KeyValue` pairs by key.

## ihash Function
The `ihash` function calculates the hash value of a given key using the FNV hash function. It returns the hash value modulo `NReduce` to determine the reduce task number for each `KeyValue` emitted by Map.

## Worker Function
The `Worker` function retrieves tasks from the coordinator, performs the tasks, and notifies the coordinator when the tasks are done. It continuously retrieves tasks until an "exit" task is received.

It retrieves the task type, filename, number of reduce tasks, and the status of the retrieval operation using the `retrieveTask` function.

Based on the task type, it performs either map or reduce operation.

For map tasks, it calculates the task number using the `ihash` function, performs the map operation on the input file, saves the intermediate map results to files, and notifies the coordinator that the map task is done.

For reduce tasks, it gets the list of files containing intermediate map results for the given reduce task, reads the intermediate map results from each file, performs the reduce operation, writes the output to a file, and notifies the coordinator that the reduce task is done.

If an "exit" task is received, it sets the exit flag to true to terminate the worker.

## Helper Functions
- `getFiles`: Returns a list of files with the given index.
- `doReduce`: Performs the reduce phase of the MapReduce job.
- `doMap`: Applies the map function to the input file and generates key-value pairs.
- `saveMapResults`: Saves intermediate map results to files.
- `handleCoordinatorCrash`: Handles the case when the coordinator crashes.
- `CallExample`, `retrieveTask`, `callDone`, `call`: Example and helper functions for making RPC calls to the coordinator.

## Notes
The code includes comments explaining each function's purpose and usage. Error handling is implemented to deal with coordinator crashes or task failures.

Overall, the `worker.go` provides the functionality to perform map and reduce tasks in a distributed MapReduce system, interacting with a coordinator to retrieve tasks and report task completion.
