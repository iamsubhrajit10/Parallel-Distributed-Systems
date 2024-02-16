/*Team 5
Subhrajit, Anshika
*/
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

// Function to generate alphanumeric strings
void generate_strings(int process_id, int num_processes, int max_length, int total_strings) {
    char char_set[MAX_CHAR_SET + 1] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int string_length;
    int i, j, idx;
    int local_count = 0;
    char str[max_length + 1];

    // Calculate the total number of strings to be generated by each process
    int strings_per_process = total_strings / num_processes;
    if (process_id < total_strings % num_processes) {
        strings_per_process++;
    }
    int start_index = (process_id * strings_per_process * MAX_CHAR_SET) / total_strings;
    int end_index = ((process_id + 1) * strings_per_process * MAX_CHAR_SET) / total_strings;
    char **permutations = (char **)malloc((strings_per_process+1) * sizeof(char *));
    for (int i = 0; i <= strings_per_process; i++) {
        permutations[i] = (char *)malloc((max_length + 1) * sizeof(char)); // +1 for null terminator
    }
    int counter = 0; // Counter to track the number of permutations generated

    // Generate permutations and store them in the 2D array
    generate_permutations(char_set, start_index, end_index , max_length, "", 0, permutations, &counter, strings_per_process+1);
    printf("\n");
    for (int i = 0; i < strings_per_process; i++) {
        // x_permutations[i-1]=permutations[i];
        printf("%s\n", permutations[i]);
    }
    //printf("No of strings printed: %d, by process %d\n",strings_per_process,process_id);
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
