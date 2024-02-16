#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define MAX_CHAR_SET 62 // Total number of alphanumeric characters

// Function to generate alphanumeric strings
char* generate_strings(int process_id, int num_processes, int max_length, int total_strings) {
    char char_set[MAX_CHAR_SET + 1] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int string_length;
    int i, j, idx;
    int local_count = 0;
    char* all_strings = NULL;

    // Calculate the total number of strings to be generated by each process
    int strings_per_process = total_strings / num_processes;
    if (process_id < total_strings % num_processes) {
        strings_per_process++;
    }

    // Allocate memory for storing all generated strings by this process
    all_strings = (char*)malloc(strings_per_process * (max_length + 1) * sizeof(char));

    // Loop through each length of the string
    for (string_length = 1; string_length <= max_length; string_length++) {
        // Calculate the starting index and ending index for this process
        int start_index = (process_id * strings_per_process * MAX_CHAR_SET) / total_strings;
        int end_index = ((process_id + 1) * strings_per_process * MAX_CHAR_SET) / total_strings;

        // Loop through each character in the character set for this process
        for (i = start_index; i < end_index; i++) {
            all_strings[local_count * (max_length + 1)] = char_set[i % MAX_CHAR_SET];
            // Generate strings of the current length recursively
            for (j = 1; j < string_length; j++) {
                // Calculate the next index for this process
                idx = (process_id * strings_per_process * MAX_CHAR_SET * (string_length - 1)) / total_strings;
                all_strings[local_count * (max_length + 1) + j] = char_set[(idx + i) % MAX_CHAR_SET];
            }
            local_count++;
            if (local_count >= strings_per_process) {
                break;
            }
        }
        if (local_count >= strings_per_process) {
            break;
        }
    }
    return all_strings;
}

int main(int argc, char* argv[]) {
    int process_id, num_processes;
    int X = 100; // Number of strings to generate
    int N = 4;   // Maximum length of strings
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    char* all_strings = generate_strings(process_id, num_processes, N, X);

    // Gather all generated strings from all processes at process 0
    char* all_generated_strings = NULL;
    int* recvcounts = NULL;
    int* displs = NULL;
    if (process_id == 0) {
        all_generated_strings = (char*)malloc(X * num_processes * (N + 1) * sizeof(char));
        recvcounts = (int*)malloc(num_processes * sizeof(int));
        displs = (int*)malloc(num_processes * sizeof(int));
    }
    MPI_Gather(&X, 1, MPI_INT, recvcounts, 1, MPI_INT, 0, MPI_COMM_WORLD);
    int total_strings = 0;
    if (process_id == 0) {
        displs[0] = 0;
        total_strings = recvcounts[0];
        for (int i = 1; i < num_processes; i++) {
            total_strings += recvcounts[i];
            displs[i] = displs[i - 1] + recvcounts[i - 1];
        }
    }
    MPI_Gatherv(all_strings, X * (N + 1), MPI_CHAR, all_generated_strings, recvcounts, displs, MPI_CHAR, 0, MPI_COMM_WORLD);

    // Print all generated strings at process 0
    if (process_id == 0) {
        for (int i = 0; i < total_strings; i++) {
            printf("%s\n", all_generated_strings + i * (N + 1));
        }
        free(all_generated_strings);
        free(recvcounts);
        free(displs);
    }

    // Free memory allocated for all_strings
    free(all_strings);

    MPI_Finalize();
    return 0;
}
