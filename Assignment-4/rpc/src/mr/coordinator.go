// The coordinator.go file contains the implementation of the Coordinator struct and its methods.
package mr

import (
	"fmt"
	"net"
	"net/http"
	"net/rpc"
	"os"
	"strconv"
	"sync"
	"time"
)

// Magic strings for task status and type
const (
	StatusLeft          = "left"
	StatusCompleted     = "completed"
	StatusRunningMap    = "running map"
	StatusRunningReduce = "running reduce"
	TaskMap             = "map"
	TaskReduce          = "reduce"
	TaskWait            = "wait"
	TaskExit            = "exit"
	WaitDuration        = time.Second * 10
)

// Task represents a task for the coordinator.
type Task struct {
	taskIdentifier string
	taskStatus     string
	timeStamp      time.Time
	mutex          sync.Mutex
}

// Coordinator represents the coordinator for the MapReduce job.
type Coordinator struct {
	mutex           sync.Mutex
	mapFiles        map[string]*Task
	reduceFiles     map[string]*Task
	mapTaskCount    int
	reduceTaskCount int
	nReduce         int
}

// ExampleArgs represents the arguments for an RPC example.
func (c *Coordinator) Example(args *ExampleArgs, reply *ExampleReply) error {
	reply.Y = args.X + 1
	return nil
}

// ProvideTask provides a task to a worker.
// ProvideTask is a method of the Coordinator struct that provides a task to a worker.
// It allocates either a map task or a reduce task to the worker, based on the availability of tasks.
// If there are map tasks available, it assigns a map task to the worker and updates the task status to StatusRunningMap.
// If there are no map tasks available but there are reduce tasks available, it assigns a reduce task to the worker
// and updates the task status to StatusRunningReduce.
// If there are no map or reduce tasks available, it assigns TaskExit to the worker, indicating that there are no more tasks.
// The function is thread-safe and uses a mutex to ensure exclusive access to the shared variables.
// It takes ExampleArgs as input arguments and WorkerTaskReply as output arguments.
// It returns an error if there is any issue allocating the task, otherwise it returns nil.
func (c *Coordinator) ProvideTask(args *ExampleArgs, reply *WorkerTaskReply) error {
	// Lock the mutex to ensure exclusive access to shared variables
	c.mutex.Lock()
	defer c.mutex.Unlock()

	// Check if there are map tasks available
	if c.mapTaskCount > 0 {
		// Allocate a map task to the worker and update the task status
		return c.allocateTask(c.mapFiles, StatusRunningMap, TaskMap, reply)
	}

	// Check if there are reduce tasks available
	if c.reduceTaskCount > 0 {
		// Allocate a reduce task to the worker and update the task status
		return c.allocateTask(c.reduceFiles, StatusRunningReduce, TaskReduce, reply)
	}

	// No more tasks available, assign TaskExit to the worker
	reply.Task = TaskExit
	return nil
}

// allocateTask allocates a task to a worker.
// allocateTask allocates a task to a worker based on the given taskFiles, runningStatus, and taskType.
// It searches for a task file that has a taskStatus of StatusLeft, marks it as runningStatus, updates the timeStamp,
// and returns the file, nreduce, and taskType in the reply. It also starts monitoring the task in a separate goroutine.
// If no task file is available, it sets the reply.Task to TaskWait.
// The function takes a map of taskFiles, where the key is the file name and the value is a pointer to a Task struct,
// the runningStatus string, the taskType string, and a pointer to a WorkerTaskReply struct.
// It returns an error if any.
func (c *Coordinator) allocateTask(taskFiles map[string]*Task, runningStatus, taskType string, reply *WorkerTaskReply) error {
	// Iterate over the taskFiles
	for file := range taskFiles {
		// Acquire the lock for the task file
		taskFiles[file].mutex.Lock()
		// Check if the taskStatus is StatusLeft
		if taskFiles[file].taskStatus == StatusLeft {
			// Mark the task as runningStatus
			taskFiles[file].taskStatus = runningStatus
			// Update the timeStamp
			taskFiles[file].timeStamp = time.Now()
			// Release the lock
			taskFiles[file].mutex.Unlock()

			// Set the reply fields
			reply.File = file
			reply.Nreduce = c.nReduce
			reply.Task = taskType

			// Start monitoring the task in a separate goroutine
			go monitorTask(taskFiles[file])

			// Return nil error
			return nil
		}
		// Release the lock
		taskFiles[file].mutex.Unlock()
	}

	// If no task file is available, set reply.Task to TaskWait
	reply.Task = TaskWait
	return nil
}

// monitorTask monitors a task.
// monitorTask is a function that monitors the status of a task and updates it if necessary.
// It waits for a specified duration and checks if the task has been completed.
// If the task is still not completed after the wait duration, it marks the task as left and updates the timestamp.
// This function is typically used by the coordinator to monitor the progress of tasks.
func monitorTask(task *Task) {
	time.Sleep(WaitDuration)

	task.mutex.Lock()
	defer task.mutex.Unlock()

	if task.taskStatus != StatusCompleted {
		task.taskStatus = StatusLeft
		task.timeStamp = time.Now()
	}
}

// TaskDone signals that a task is done.
// TaskDone marks a task as done and updates the task counts accordingly.
// It takes a WorkerTaskArgs argument containing the task details and a WorkerDoneReply argument to return the exit status.
// The method acquires a lock to ensure thread safety and releases it before returning.
// It checks the task type and updates the corresponding task count and file list accordingly.
// If the task type is invalid, it returns an error with a descriptive message.
// Finally, it sets the exit status based on whether all map and reduce tasks are completed.
func (c *Coordinator) TaskDone(args *WorkerTaskArgs, reply *WorkerDoneReply) error {
	c.mutex.Lock()
	defer c.mutex.Unlock()

	switch args.Task {
	case TaskMap:
		c.markTaskAsDone(c.mapFiles, args.File)
		c.mapTaskCount -= 1
	case TaskReduce:
		c.markTaskAsDone(c.reduceFiles, args.File)
		c.reduceTaskCount -= 1
	default:
		return fmt.Errorf("invalid task type: %s", args.Task)
	}

	reply.Exit = (c.mapTaskCount == 0 && c.reduceTaskCount == 0)
	return nil
}

// markTaskAsDone marks a task as done.
// markTaskAsDone marks a task as completed in the coordinator's taskFiles map.
// It takes a map of taskFiles and the name of the file to mark as done.
// The function acquires a lock on the task's mutex to ensure exclusive access to the task.
// After marking the task as completed, it updates the timestamp to the current time.
func (c *Coordinator) markTaskAsDone(taskFiles map[string]*Task, file string) {
	taskFiles[file].mutex.Lock()
	defer taskFiles[file].mutex.Unlock()

	taskFiles[file].taskStatus = StatusCompleted
	taskFiles[file].timeStamp = time.Now()
}

// server starts the coordinator server.
// server starts the RPC server for the Coordinator.
// It registers the Coordinator object for RPC and handles HTTP requests.
// It removes any existing socket file, listens on a Unix socket, and serves HTTP requests.
// The server runs in a separate goroutine.
// Returns an error if there was a problem registering the coordinator, removing the old socket,
// or listening on the socket. Otherwise, it returns nil.
func (c *Coordinator) server() error {
	if err := rpc.Register(c); err != nil {
		return fmt.Errorf("failed to register coordinator: %w", err)
	}

	rpc.HandleHTTP()

	sockname := coordinatorSock()

	if err := os.Remove(sockname); err != nil && !os.IsNotExist(err) {
		return fmt.Errorf("failed to remove old socket: %w", err)
	}

	listener, err := net.Listen("unix", sockname)
	if err != nil {
		return fmt.Errorf("failed to listen on socket: %w", err)
	}

	go http.Serve(listener, nil)

	return nil
}

// Done checks if the MapReduce job is done.
// Done checks if all map and reduce tasks have been completed.
// It returns true if all tasks have been completed, otherwise false.
func (c *Coordinator) Done() bool {
	c.mutex.Lock()
	defer c.mutex.Unlock()

	// Check if there are any remaining map tasks
	if c.mapTaskCount != 0 {
		return false
	}

	// Check if there are any remaining reduce tasks
	if c.reduceTaskCount != 0 {
		return false
	}

	// All tasks have been completed
	return true
}

// MakeCoordinator creates a new coordinator.
// MakeCoordinator creates a new Coordinator instance with the given files and number of reduce tasks.
// It initializes the mapFiles and reduceFiles maps with the corresponding tasks.
// The mapFiles map is populated with tasks for each file in the files slice, with an initial status of StatusLeft.
// The reduceFiles map is populated with tasks for each reduce task index, with an initial status of StatusLeft.
// The Coordinator struct also keeps track of the total number of map tasks (mapTaskCount), the total number of reduce tasks (reduceTaskCount),
// and the number of reduce tasks (nReduce).
// Finally, it starts the server for the coordinator and returns a pointer to the created Coordinator instance.
func MakeCoordinator(files []string, nReduce int) *Coordinator {
	coordinator := Coordinator{
		mutex:           sync.Mutex{},
		mapFiles:        make(map[string]*Task),
		reduceFiles:     make(map[string]*Task),
		mapTaskCount:    len(files),
		reduceTaskCount: nReduce,
		nReduce:         nReduce,
	}

	for _, file := range files {
		coordinator.mapFiles[file] = &Task{file, StatusLeft, time.Now(), sync.Mutex{}}
	}
	for i := 0; i < nReduce; i++ {
		idx := strconv.Itoa(i)
		coordinator.reduceFiles[idx] = &Task{idx, StatusLeft, time.Now(), sync.Mutex{}}
	}

	if err := coordinator.server(); err != nil {
		return nil
	}

	return &coordinator
}
