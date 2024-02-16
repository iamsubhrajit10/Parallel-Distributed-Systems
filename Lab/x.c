#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define MAX_CHAR_SET 62 // Total number of alphanumeric characters

// Function to generate alphanumeric strings
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
        char new_prefix[prefix_length + 2]; // +2 for the null terminator and the character to be appended
        strcpy(new_prefix, prefix);
        new_prefix[prefix_length] = char_set[i];
        new_prefix[prefix_length + 1] = '\0';

        generate_permutations(char_set, s, e, n, new_prefix, prefix_length + 1, permutations, counter, x);
    }
}

void generate_strings(int process_id, int num_processes, int max_length, int total_strings) {
    char char_set[MAX_CHAR_SET + 1] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int string_length;
    int i, j, idx;
    int local_count = 0;
    char** all_strings;

    // Calculate the total number of strings to be generated by each process
    int strings_per_process = total_strings / num_processes;
    int remaining_strings = total_strings % num_processes;

    if (process_id < remaining_strings) {
        strings_per_process++;
    }

    // Allocate memory for storing all generated strings by this process
    all_strings = (char**)malloc(strings_per_process * sizeof(char*));

    // Calculate the starting and ending indices for this process
    int start_index = (process_id * total_strings * MAX_CHAR_SET) / num_processes;
    int end_index = ((process_id + 1) * total_strings * MAX_CHAR_SET) / num_processes;

    char **permutations = (char **)malloc(strings_per_process * sizeof(char *));
    for (int i = 0; i < x; i++) {
        permutations[i] = (char *)malloc((max_length + 1) * sizeof(char)); // +1 for null terminator
    }
    int counter = 0; // Counter to track the number of permutations generated

    // Generate permutations and store them in the 2D array
    generate_permutations(char_set, start_index, end_index , max_length, "", 0, permutations, &counter, strings_per_process);
    

    // Print all generated strings
    // Print the generated permutations
    printf("Generated Permutations for process %d:\n",process_id);
    for (int i = 0; i < counter; i++) {
        printf("%s\n", permutations[i]);
    }

    // Free memory allocated for permutations
    for (int i = 0; i < x; i++) {
        free(permutations[i]);
    }
    free(permutations);
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
