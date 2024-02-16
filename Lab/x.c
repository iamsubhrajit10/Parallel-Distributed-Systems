#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define MAX_CHAR_SET 62 // Total number of alphanumeric characters

// Function to generate alphanumeric strings
void generate_strings(int process_id, int num_processes, int max_length, int total_strings) {
    char char_set[MAX_CHAR_SET + 1] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int string_length;
    int i, j, idx;
    int local_count = 0;
    char** all_strings;

    // Calculate the total number of strings to be generated by each process
    int strings_per_process = total_strings / num_processes;
    if (process_id < total_strings % num_processes) {
        strings_per_process++;
    }

    // Allocate memory for storing all generated strings by this process
    all_strings = (char**)malloc(strings_per_process * sizeof(char*));
    
    // Calculate the starting and ending indices for this process
    int start_index = (process_id * total_strings * MAX_CHAR_SET) / num_processes;
    int end_index = ((process_id + 1) * total_strings * MAX_CHAR_SET) / num_processes;

    // Loop through each character in the character set for this process
    for (i = start_index; i < end_index; i++) {
        // Allocate memory for the current string
        all_strings[local_count] = (char*)malloc((max_length + 1) * sizeof(char));

        // Generate strings of different lengths
        for (string_length = 1; string_length <= max_length; string_length++) {
            // Initialize the current string with the first character
            all_strings[local_count][0] = char_set[i % MAX_CHAR_SET];

            // Generate strings of the current length recursively
            for (j = 1; j < string_length; j++) {
                // Calculate the next index for this process
                idx = ((i * num_processes) + process_id + j) % total_strings;
                all_strings[local_count][j] = char_set[idx % MAX_CHAR_SET];
            }

            // Null-terminate the string
            all_strings[local_count][string_length] = '\0';

            // Increment the local count
            local_count++;

            // Check if the required number of strings per process has been generated
            if (local_count >= strings_per_process) {
                printf("Voila! for process %d",process_id);
                break;
            }
        }

        // Check if the required number of strings per process has been generated
        if (local_count >= strings_per_process) {
            printf("Voila! for process %d",process_id);
            break;
        }
    }

    // Print all generated strings
    printf("\n");
    for (i = 0; i < local_count; i++) {
        printf("%s\n", all_strings[i]);
    }

    // Free the memory allocated for all_strings
    for (i = 0; i < local_count; i++) {
        free(all_strings[i]);
    }
    free(all_strings);
}

int main(int argc, char *argv[]) {
    int process_id, num_processes;
    int X = 1000; // Number of strings to generate
    int N = 4;  // Maximum length of strings
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    // Generate strings
    generate_strings(process_id, num_processes, N, X);

    MPI_Finalize();
    return 0;
}
