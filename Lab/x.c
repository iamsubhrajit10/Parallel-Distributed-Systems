#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define MAX_CHAR_SET 62 // Total number of alphanumeric characters

// Function to generate unique alphanumeric strings
void generate_unique_strings(int process_id, int num_processes, int max_length, int total_strings, char *local_buffer, int *counts) {
    char char_set[MAX_CHAR_SET + 1] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int string_length;
    int i, j;
    int local_count = 0;
    char str[max_length + 1];

    // Calculate the total number of strings to be generated by each process
    int strings_per_process = (total_strings + num_processes - 1) / num_processes;

    // Loop through each length of the string
    for (string_length = 1; string_length <= max_length; string_length++) {
        // Loop through each character in the character set for this process
        for (i = 0; i < strings_per_process; i++) {
            // Generate strings of the current length based on process rank and unique identifier
            snprintf(str, sizeof(str), "%d_%d_", process_id, i);
            int id_length = strlen(str);
            for (j = id_length; j < string_length + id_length; j++) {
                str[j] = char_set[j % MAX_CHAR_SET];
            }
            str[string_length + id_length] = '\0';
            printf("%s\n",str);
            // Copy the generated string to the local buffer
            strcpy(&local_buffer[local_count * (max_length + 1)], str);
            local_count++;
            if (local_count >= strings_per_process) {
                break;
            }
        }
        if (local_count >= strings_per_process) {
            break;
        }
    }

    // Update the count for each process
    counts[process_id] = local_count;
}

int main(int argc, char *argv[]) {
    int process_id, num_processes;
    int X = 1000; // Number of unique strings to generate
    int N = 10;   // Maximum length of strings
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    // Calculate the total number of strings to be generated by each process
    int strings_per_process = (X + num_processes - 1) / num_processes;

    // Calculate the local buffer size for each process
    int local_buffer_size = strings_per_process * (N + 1);

    // Allocate local buffer for each process
    char *local_buffer = (char *)malloc(local_buffer_size * sizeof(char));

    // Calculate the counts array for MPI_Gatherv
    int *counts = (int *)malloc(num_processes * sizeof(int));

    // Generate unique strings
    generate_unique_strings(process_id, num_processes, N, X, local_buffer, counts);

    // Print the number of strings generated by each process
    for (int i = 0; i < num_processes; i++) {
        MPI_Barrier(MPI_COMM_WORLD);
        if (process_id == i) {
            printf("Process %d generated %d strings.\n", process_id, counts[process_id]);
            fflush(stdout);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    // Free local buffer and counts array
    free(local_buffer);
    free(counts);

    MPI_Finalize();
    return 0;
}
