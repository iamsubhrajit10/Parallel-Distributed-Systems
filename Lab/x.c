#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define MAX_CHAR_SET 62 // Total number of alphanumeric characters

void swap(char *x, char *y) {
    char temp = *x;
    *x = *y;
    *y = temp;
}

void generate_permutations(char *char_set, int s, int e, int n, char *prefix, int prefix_length, char **permutations, int *counter, int x) {
    if (*counter >= x || prefix_length > n) {
        return;
    }

    strcpy(permutations[*counter], prefix);
    (*counter)++;

    for (int i = s; i <= e; i++) {
        char new_prefix[prefix_length + 2];
        strcpy(new_prefix, prefix);
        new_prefix[prefix_length] = char_set[i];
        new_prefix[prefix_length + 1] = '\0';

        generate_permutations(char_set, s, e, n, new_prefix, prefix_length + 1, permutations, counter, x);
    }
}

void generate_strings(int process_id, int num_processes, int max_length, int total_strings, char *global_buffer) {
    char char_set[MAX_CHAR_SET + 1] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

    int strings_per_process = total_strings / num_processes;
    if (process_id < total_strings % num_processes) {
        strings_per_process++;
    }

    int start_index = (process_id * strings_per_process * MAX_CHAR_SET) / total_strings;
    int end_index = ((process_id + 1) * strings_per_process * MAX_CHAR_SET) / total_strings;

    char **permutations = (char **)malloc((strings_per_process + 1) * sizeof(char *));
    for (int i = 0; i <= strings_per_process; i++) {
        permutations[i] = (char *)malloc((max_length + 1) * sizeof(char));
    }

    int counter = 0;

    generate_permutations(char_set, start_index, end_index, max_length, "", 0, permutations, &counter, strings_per_process + 1);

    // Gather strings from all processes to process 0
    for (int i = 0; i < num_processes; i++) {
        MPI_Barrier(MPI_COMM_WORLD);
        if (process_id == i) {
            for (int j = 1; j < num_processes; j++) {
                MPI_Recv(global_buffer, (max_length + 1) * strings_per_process, MPI_CHAR, j, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                printf("%s", global_buffer);
            }
            fflush(stdout);
        } else {
            MPI_Send(permutations[1], (max_length + 1) * strings_per_process, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    // Free allocated memory
    for (int i = 0; i <= strings_per_process; i++) {
        free(permutations[i]);
    }
    free(permutations);
}

int main(int argc, char *argv[]) {
    int process_id, num_processes;
    int X =100; // Number of unique strings to generate
    int N = 20;      // Maximum length of strings
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    char *global_buffer = (char *)malloc((N + 1) * X * sizeof(char));

    generate_strings(process_id, num_processes, N, X, global_buffer);

    free(global_buffer);

    MPI_Finalize();
    return 0;
}