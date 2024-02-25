#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define ROWS 11
#define COLS 65536
#define Z 11
#define LENGTH 7

int main() {
    // Allocate memory for the DNA_String array
    char DNA[] = {'A', 'C', 'G', 'T'};
    int count_array[LENGTH]; // Array to store the count of DNA strings for each length
    char (*DNA_String)[COLS][Z] = malloc(ROWS * sizeof(*DNA_String));
    
    if (DNA_String == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the first row with DNA characters
    for (int i = 0; i < 4; i++) {
        DNA_String[0][i][0] = DNA[i];
        DNA_String[0][i][1] = '\0'; // Null terminator
        printf("%s\n", DNA_String[0][i]);
    }

    // Generate DNA strings
    int count = 4;
    int index = 0; // Initialize index
    int num_threads = omp_get_max_threads(); // Get the number of threads
    for (int length = 1; length <= LENGTH; length++) {
        #pragma omp parallel num_threads(num_threads) shared(index, count, DNA_String, DNA) 
        {
            int tid = omp_get_thread_num(); // Get thread ID
            int chunk_size = count / num_threads; // Calculate chunk size
            int start = tid * chunk_size; // Calculate starting index for this thread
            int end = (tid == num_threads - 1) ? count : start + chunk_size; // Calculate ending index for this thread

            for (int j = 0; j < 4; j++) {
                for (int i = start; i < end; i++) {
                    strcpy(DNA_String[length][index], DNA_String[length - 1][i]);
                    int curr_len = strlen(DNA_String[length][index]);
                    DNA_String[length][index][curr_len] = DNA[j];
                    DNA_String[length][index][curr_len + 1] = '\0';
                    #pragma omp atomic // Ensure atomic increment of index
                    index++;
                }
            }
        }
        count = index; // Update count for the next length
        count_array[length - 1] = count; // Store the count for this length
    }

    // Print the DNA strings
    for (int len = 1; len <= LENGTH; len++) {
        printf("DNA strings of length %d:\n", len);
        for (int i = 0; i < count_array[len - 1]; i++) {
            printf("%s\n", DNA_String[len][i]);
        }
        printf("\n");
    }

    // Free allocated memory
    free(DNA_String);

    return 0;
}
