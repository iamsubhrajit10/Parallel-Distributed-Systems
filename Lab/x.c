#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define MAX_CHAR_SET 62 // Total number of alphanumeric characters

char* flatten(char **strings, int num_strings, int max_length) {
    // Calculate total length (with delimiter and null terminators)
    int total_length = num_strings * max_length + num_strings + 1; 

    // Allocate memory
    char *flattened_array = (char *)malloc(total_length * sizeof(char));
    if (flattened_array == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    // Flatten
    int index = 0;
    for (int i = 0; i < num_strings; i++) {
        int length = strlen(strings[i]);
        // Copy individual string
        for (int j = 0; j < length; j++) {
            flattened_array[index++] = strings[i][j];
        }
        // Add delimiter (unless it's the last string)
        if (i < num_strings - 1) {
            flattened_array[index++] = ' '; 
       }
    }

    // Null-terminate
    flattened_array[index] = '\0';
    return flattened_array;
}

void generate_permutations(char *char_set, int s, int e, int n, char *prefix, int prefix_length, char **permutations, int *counter, int x) {
    if (*counter >= x || prefix_length > n) {
        return;
    }

    strcpy(permutations[*counter], prefix);
    (*counter)++;

    for (int i = s; i <= e; i++) {
        char new_prefix[prefix_length + 2]; // +2 for the null terminator and the character to be appended
        strcpy(new_prefix, prefix);
        new_prefix[prefix_length] = char_set[i];
        new_prefix[prefix_length + 1] = '\0';
        
        generate_permutations(char_set, s, e, n, new_prefix, prefix_length + 1, permutations, counter, x);
    }
}

void generate_strings(int process_id, int num_processes, int max_length, int total_strings) {
    char char_set[MAX_CHAR_SET + 1] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int strings_per_process = total_strings / num_processes;
    if (process_id < total_strings % num_processes) {
        strings_per_process++;
    }
    int start_index = (process_id * strings_per_process * MAX_CHAR_SET) / total_strings;
    int end_index = ((process_id + 1) * strings_per_process * MAX_CHAR_SET) / total_strings;
    char **permutations = (char **)malloc((strings_per_process + 1) * sizeof(char *));
    for (int i = 0; i <= strings_per_process; i++) {
        permutations[i] = (char *)malloc((max_length + 1) * sizeof(char)); // +1 for null terminator
    }
    int counter = 0; // Counter to track the number of permutations generated

    // Generate permutations and store them in the 2D array
    generate_permutations(char_set, start_index, end_index , max_length+1, "", 0, permutations, &counter, strings_per_process + 1);

    // Flatten the permutations into a single string
    char *flattened = flatten(permutations, strings_per_process + 1, max_length+1);

    for (int i = 0; i <= strings_per_process; i++) {
        free(permutations[i]);
    }
    free(permutations);
    //free(flattened); // Free the flattened array

    MPI_Send(flattened, strlen(flattened) + 1, MPI_CHAR, num_processes, 0, MPI_COMM_WORLD);
    free(flattened); // Release memory 
}

void receive_permutations(int process_id, int num_processes, int max_length, int total_strings, char **all_permutations) {
    int strings_per_process = total_strings / num_processes;
    if (process_id < total_strings % num_processes) {
        strings_per_process++;
    }
    // int size =  * ;
    int size = (strings_per_process + 1) * (max_length + 1) + (strings_per_process + 1) + 1; 
    // Receive from each sender process
    all_permutations = (char **)malloc((num_processes)*sizeof(char *));
    
    for (int src = 0; src < num_processes ; src++) { 
        char *received_data = (char *)malloc(size * sizeof(char));
        if (received_data == NULL) {
            fprintf(stderr, "Process %d: Memory allocation failed\n", process_id);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // Receive synchronized with the sender
        MPI_Recv(received_data, size, MPI_CHAR, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Store the received data into the all_permutations array
        all_permutations[src] = received_data; 

        // Free the temporary buffer
        free(received_data); 
        printf("%s\n",all_permutations[src]);
    }
    
   
}


int main(int argc, char *argv[]) {
    int process_id, num_processes;
    int X = 100; // Number of strings to generate
    int N = 4;    // Maximum length of strings
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    // Allocate memory for storing all received permutations in the last process
    char **all_permutations = NULL;
    if (process_id == num_processes - 1) {
        all_permutations = (char **)malloc(num_processes * sizeof(char *));
        if (all_permutations == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
    }
    
    
    if (process_id == num_processes - 1) {
        receive_permutations(process_id, num_processes - 1, N, X, all_permutations);
    } else {
        generate_strings(process_id, num_processes - 1, N, X);
    }

    MPI_Finalize();
    return 0;
}
