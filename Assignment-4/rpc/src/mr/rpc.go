package mr

import (
	"os"
	"strconv"
)

// ExampleArgs represents the arguments for an RPC example.
type ExampleArgs struct {
	X int
}

// ExampleReply represents the reply for an RPC example.
type ExampleReply struct {
	Y int
}

// WorkerTaskReply represents the reply for a worker task.
type WorkerTaskReply struct {
	Task    string
	File    string
	Nreduce int
}

// WorkerTaskArgs represents the arguments for a worker task.
type WorkerTaskArgs struct {
	Task string
	File string
}

// WorkerDoneReply represents the reply for a worker done signal.
type WorkerDoneReply struct {
	Exit bool
}

const (
	MsgForJob          = "MsgForJob"
	MsgForInterFileLoc = "MsgForInterFileLoc"
	MsgForFinishMap    = "MsgForFinishMap"
	MsgForFinishReduce = "MsgForFinishReduce"
)

// Args represents the arguments for an RPC message.
type Args struct {
	MsgType string // Indicates which type of message: job request, file location, or signaling that map or reduce job is finished
	MsgCnt  string // Message count
}

// Reply represents the reply for an RPC message.
type Reply struct {
	Filename    string   // Name of the file
	MapNum      int      // Number of map tasks
	RedNum      int      // Number of reduce tasks
	Reducers    int      // Number of reducers
	TaskType    string   // Type of the task (map or reduce)
	RedFileList []string // List of intermediate files for reduce tasks
}

// InterFile represents the intermediate file message.
type InterFile struct {
	MsgType int // Type of the message
	MsgCnt  int // Message count
	RedType int // Type of reduce task
}

// coordinatorSock returns the coordinator socket path.
func coordinatorSock() string {
	s := "/var/tmp/cs612-mr-"
	s += strconv.Itoa(os.Getuid())
	return s
}
