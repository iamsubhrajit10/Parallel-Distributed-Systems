#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define MAX_CHAR_SET 62 // Total number of alphanumeric characters

// Function to generate alphanumeric strings
void generate_strings(int process_id, int num_processes, int max_length, int total_strings, char* buffer) {
    char char_set[MAX_CHAR_SET + 1] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int string_length;
    int i, j, idx;
    int local_count = 0;
    char str[max_length + 1];

    // Loop through each length of the string
    for (string_length = 1; string_length <= max_length; string_length++) {
        // Calculate the starting index and ending index for this process
        int start_index = (process_id * MAX_CHAR_SET) / num_processes;
        int end_index = ((process_id + 1) * MAX_CHAR_SET) / num_processes;

        // Loop through each character in the character set for this process
        for (i = start_index; i < end_index; i++) {
            str[0] = char_set[i];
            // Generate strings of the current length recursively
            for (j = 1; j < string_length; j++) {
                // Calculate the next index for this process
                idx = (process_id * MAX_CHAR_SET * (string_length - 1)) / num_processes;
                str[j] = char_set[(idx + i) % MAX_CHAR_SET];
            }
            // Null-terminate the string
            str[string_length] = '\0';
            // Copy the generated string to the buffer
            strcpy(&buffer[(local_count * (max_length + 1)) + local_count], str);
            local_count++;
            if (local_count >= total_strings) {
                break;
            }
        }
        if (local_count >= total_strings) {
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    int process_id, num_processes;
    int X = 100; // Number of strings to generate
    int N = 4;   // Maximum length of strings
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    // Calculate the total number of strings
    int total_strings = X;

    // Allocate memory for the buffer to store the generated strings
    char *send_buffer = (char *)malloc(total_strings * (N + 1) * sizeof(char));
    char *recv_buffer = NULL;

    // Process 0 allocates memory for receiving buffer
    if (process_id == 0) {
        recv_buffer = (char *)malloc(total_strings * num_processes * (N + 1) * sizeof(char));
    }

    // Generate strings
    generate_strings(process_id, num_processes, N, total_strings, send_buffer);

    // Gather all the generated strings to process 0
    MPI_Gather(send_buffer, total_strings * (N + 1), MPI_CHAR, recv_buffer, total_strings * (N + 1), MPI_CHAR, 0, MPI_COMM_WORLD);

    // Process 0 outputs the generated strings
    if (process_id == 0) {
        for (int i = 0; i < total_strings * num_processes; i++) {
            printf("%s\n", &recv_buffer[i * (N + 1)]);
        }
    }

    // Free allocated memory
    free(send_buffer);
    if (process_id == 0) {
        free(recv_buffer);
    }

    MPI_Finalize();
    return 0;
}
