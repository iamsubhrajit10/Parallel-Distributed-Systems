package mr

import (
	"bufio"
	"encoding/json"
	"fmt"
	"hash/fnv"
	"io/ioutil"
	"log"
	"net/rpc"
	"os"
	"path/filepath"
	"sort"
	"strconv"
	"strings"
	"time"
)

// KeyValue represents a key-value pair.
// It is used to store intermediate results during the map phase
// and to store the final output during the reduce phase.
type KeyValue struct {
	Key   string
	Value string
}

// ByKey implements the sort.Interface for sorting KeyValue by key.
type ByKey []KeyValue

// Len returns the length of the KeyValue slice.
func (a ByKey) Len() int {
	return len(a)
}

// Swap swaps the elements with indexes i and j in the KeyValue slice.
func (a ByKey) Swap(i, j int) {
	a[i], a[j] = a[j], a[i]
}

// Less returns true if the key at index i is less than the key at index j in the KeyValue slice.
func (a ByKey) Less(i, j int) bool {
	return a[i].Key < a[j].Key
}

// ihash calculates the hash value of a given key using the fnv hash function.
// It returns the hash value modulo NReduce to determine the reduce task number for each KeyValue emitted by Map.
func ihash(key string) int {
	h := fnv.New32a()
	h.Write([]byte(key))
	return int(h.Sum32() & 0x7fffffff)
}

// Worker retrieves tasks from the coordinator, performs the tasks, and then notifies the coordinator when the tasks are done.
// Worker is a function that performs the map-reduce tasks assigned by the coordinator.
// It takes two function parameters: mapf and reducef, which are responsible for performing the map and reduce operations, respectively.
// The Worker function continuously retrieves tasks from the coordinator and executes them until an "exit" task is received.
// It returns an error if any operation fails or if the coordinator crashes.
func Worker(mapf func(string, string) []KeyValue, reducef func(string, []string) string) error {
	// Initialize the exit flag to false
	exit := false

	// Continuously retrieve tasks from the coordinator until an "exit" task is received
	for !exit {
		// Retrieve the task, filename, number of reduce tasks, and the status of the retrieval operation
		task, filename, nReduce, ok := retrieveTask()

		// If the retrieval operation fails, return an error indicating that the coordinator has crashed
		if !ok {
			return fmt.Errorf("coordinator crashed worker exiting")
		}

		// Perform the appropriate action based on the task type
		switch task {
		case "map":
			// Calculate the task number using the ihash function
			nTask := ihash(filename)

			// Perform the map operation on the input file
			intermediate, err := doMap(mapf, filename)
			if err != nil {
				return fmt.Errorf("failed to do map: %w", err)
			}

			// Save the intermediate map results to files
			if !saveMapResults(intermediate, nReduce, nTask) {
				return fmt.Errorf("failed to save map results")
			}

			// Notify the coordinator that the map task is done and retrieve the status and exit flag
			ok, exit = callDone("map", filename)
			if err != nil {
				return fmt.Errorf("failed to call done: %w", err)
			}

		case "reduce":
			// Convert the filename to an integer index
			index, err := strconv.Atoi(filename)
			if err != nil {
				return fmt.Errorf("failed to convert filename to integer: %w", err)
			}

			// Get the list of files containing intermediate map results for the given reduce task
			files := getFiles(index)
			kvData := []KeyValue{}

			// Read the intermediate map results from each file and append them to kvData
			for _, filepath := range files {
				file, err := os.Open(filepath)
				if err != nil {
					return fmt.Errorf("failed to open file: %w", err)
				}
				defer file.Close()

				dec := json.NewDecoder(file)
				for {
					var kv KeyValue
					if err := dec.Decode(&kv); err != nil {
						break
					}
					kvData = append(kvData, kv)
				}
			}

			// Sort the key-value pairs by key
			sort.Sort(ByKey(kvData))

			// Perform the reduce operation on the sorted key-value pairs and write the output to a file
			doReduce(reducef, kvData, filename[:1])

			// Notify the coordinator that the reduce task is done and retrieve the status and exit flag
			ok, exit = callDone("reduce", filename)

		case "exit":
			// Set the exit flag to true to terminate the worker
			exit = true
		}

		// If the retrieval or completion operation fails, return an error indicating that the coordinator has crashed
		if !ok {
			return fmt.Errorf("coordinator crashed worker exiting")
		}

		// Sleep for a short duration before retrieving the next task
		time.Sleep(time.Millisecond * 100)
	}

	// Return nil to indicate successful execution of all tasks
	return nil
}

// getFiles returns a list of files with the given index.
// getFiles returns a slice of file paths that have the specified file index suffix.
// The file index is used to filter the files based on their names.
// The function first fetches the current working directory using os.Getwd().
// It then reads the directory using os.ReadDir() to get a list of files in the directory.
// For each file, it checks if the file name has the specified suffix using strings.HasSuffix().
// If a file has the suffix, its absolute path is added to the fileSet slice using filepath.Join().
// Finally, the function returns the fileSet slice containing the file paths.
func getFiles(fileIndex int) []string {
	suffix := strconv.Itoa(fileIndex) + ".json"
	dir, err := os.Getwd()
	if err != nil {
		log.Fatalf("Error while fetching directory: %v", err)
		return nil
	}

	files, err := os.ReadDir(dir)
	if err != nil {
		log.Fatalf("Error while reading directory: %v", err)
		return nil
	}

	fileSet := []string{}
	for _, file := range files {
		if strings.HasSuffix(file.Name(), suffix) {
			fileSet = append(fileSet, filepath.Join(dir, file.Name()))
		}
	}

	return fileSet
}

// doReduce does the reduce job.
// doReduce performs the reduce phase of the MapReduce job.
// It takes a reduce function, a slice of intermediate key-value pairs,
// and an index indicating the reduce task number.
// It writes the output of the reduce function to a file named "mr-out-{index}.txt".
// The reduce function is called for each unique key in the intermediate slice,
// with the corresponding values for that key.
// The output of the reduce function is written to the output file in the format:
//   key value
// where key is the unique key and value is the result of the reduce function.
// The function returns an error if there is any issue with file creation or writing.
func doReduce(reducef func(string, []string) string, intermediate []KeyValue, index string) error {
	outputFileName := "mr-out-" + index + ".txt"
	outputFile, err := os.Create(outputFileName)
	if err != nil {
		return fmt.Errorf("failed to create output file: %w", err)
	}
	defer outputFile.Close()

	values := make([]string, 0, len(intermediate))
	currentKeyIndex := 0
	for currentKeyIndex < len(intermediate) {
		sameKeyIndex := currentKeyIndex + 1
		for sameKeyIndex < len(intermediate) && intermediate[sameKeyIndex].Key == intermediate[currentKeyIndex].Key {
			sameKeyIndex++
		}
		values = values[:0]
		for tempKeyIndex := currentKeyIndex; tempKeyIndex < sameKeyIndex; tempKeyIndex++ {
			values = append(values, intermediate[tempKeyIndex].Value)
		}
		output := reducef(intermediate[currentKeyIndex].Key, values)

		if _, err := fmt.Fprintf(outputFile, "%v %v\n", intermediate[currentKeyIndex].Key, output); err != nil {
			return fmt.Errorf("failed to write to output file: %w", err)
		}
		currentKeyIndex = sameKeyIndex
	}

	return outputFile.Close()
}

// doMap does the map job.
// doMap applies the map function to the input file and returns the key-value pairs generated.
// It takes a map function, which is a function that takes a filename and its content as input
// and returns a slice of KeyValue pairs. It also takes the filename of the input file.
// The function opens the file, reads its content, and applies the map function to generate
// the key-value pairs. It returns the generated key-value pairs and any error encountered.
func doMap(mapf func(string, string) []KeyValue, filename string) ([]KeyValue, error) {
	file, err := os.Open(filename)
	if err != nil {
		return nil, fmt.Errorf("cannot open %v: %w", filename, err)
	}
	defer file.Close()

	content, err := ioutil.ReadAll(file)
	if err != nil {
		return nil, fmt.Errorf("cannot read %v: %w", filename, err)
	}

	kva := mapf(filename, string(content))

	return kva, nil
}

// saveMapResults saves the intermediate map results to files.
// It takes a slice of KeyValue pairs, the number of reduce tasks, and the task number.
// It saves the intermediate map results to files named "mr-{task}-{bucket}.json",
// where task is the task number and bucket is the reduce task number.
// The function returns true if the save operation is successful, false otherwise.
func saveMapResults(intermediate []KeyValue, nReduce int, nTask int) bool {
	fileMap := make(map[int]*os.File)
	var err error = nil
	baseFilename := "mr-" + strconv.Itoa(nTask) + "-" // Precomputed

	for _, kv := range intermediate {
		nBucket := ihash(kv.Key) % nReduce
		file, exist := fileMap[nBucket]
		if !exist {
			fname := baseFilename + strconv.Itoa(nBucket) + ".json"
			file, err = os.Create(fname)

			if err != nil {
				log.Fatalf("Error while creating map file for task %d bucket %d", nTask, nBucket)
				return false
			}
			fileMap[nBucket] = file
		}

		writer := bufio.NewWriter(file) // Buffering
		enc := json.NewEncoder(writer)
		if err := enc.Encode(&kv); err != nil {
			writer.Flush() // Ensure data is written on error
			return false
		}
		writer.Flush()
	}

	for _, f := range fileMap {
		f.Close()
	}

	return true
}

// handleCoordinatorCrash handles the case when the coordinator crashes.
// It prints a message indicating that the coordinator has crashed.
func handleCoordinatorCrash() {
	fmt.Println("coordinator crashed worker exiting")
	return
}

// CallExample is an example function to show how to make an RPC call to the coordinator.
// It sends an RPC request to the coordinator's Example method.
// It fills in the argument, sends the request, and waits for the reply.
// The reply should contain the value 100 in the Y field.
func CallExample() {
	// Declare an argument structure.
	args := ExampleArgs{}

	// Fill in the argument(s).
	args.X = 99

	// Declare a reply structure.
	reply := ExampleReply{}

	// Send the RPC request, wait for the reply.
	// The "Coordinator.Example" tells the receiving server that we'd like to call
	// the Example() method of struct Coordinator.
	ok := call("Coordinator.Example", &args, &reply)
	if ok {
		// reply.Y should be 100.
		fmt.Printf("reply.Y %v\n", reply.Y)
	} else {
		fmt.Printf("call failed!\n")
	}
}

// retrieveTask retrieves a task from the coordinator.
// It sends an RPC request to the coordinator's ProvideTask method to retrieve a task.
// It fills in the argument, sends the request, and waits for the reply.
// The reply should contain the task type, filename, number of reduce tasks, and a boolean indicating if the operation was successful.
func retrieveTask() (string, string, int, bool) {
	reply := WorkerTaskReply{}
	args := ExampleArgs{}

	ok := call("Coordinator.ProvideTask", &args, &reply)

	return reply.Task, reply.File, reply.Nreduce, ok
}

// callDone notifies the coordinator that a task is done.
// It sends an RPC request to the coordinator's TaskDone method to notify that a task is done.
// It fills in the argument, sends the request, and waits for the reply.
// The reply should contain a boolean indicating if the operation was successful and a boolean indicating if the worker should exit.
func callDone(task string, file string) (bool, bool) {
	args := WorkerTaskArgs{task, file}
	reply := WorkerDoneReply{false}

	ok := call("Coordinator.TaskDone", &args, &reply)

	return ok, reply.Exit
}

// send an RPC request to the coordinator, wait for the response.
// usually returns true.
// returns false if something goes wrong.
func call(rpcname string, args interface{}, reply interface{}) bool {
	sockname := coordinatorSock()
	c, err := rpc.DialHTTP("unix", sockname)
	if err != nil {
		log.Fatal("dialing:", err)
	}
	defer c.Close()

	err = c.Call(rpcname, args, reply)

	if err == nil {
		// log.Printf("call %s success", rpcname)
		return true
	}

	fmt.Println("Call Failed: ", err)
	return false
}
