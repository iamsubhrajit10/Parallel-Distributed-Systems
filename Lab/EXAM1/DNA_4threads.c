#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define ROWS 11
#define COLS 65536
#define Z 11
#define LENGTH 2

int main() {
    // Allocate memory for the DNA_String array
    char DNA[] = {'A', 'C', 'G', 'T'};
    int count_array[LENGTH]; // Array to store the count of DNA strings for each length
    char (*DNA_String)[COLS][Z] = malloc(ROWS * COLS * Z * sizeof(char));
    
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
    for (int length = 1; length <= LENGTH; length++) {
        int index = 0;
        for (int j = 0; j < 4; j++) {
            #pragma omp parallel for num_threads(4) shared(DNA_String, count, index, length)
            for (int i = 0; i < count; i++) {
                int local_index; // Declare local_index inside the loop
                #pragma omp critical // Use critical section to avoid race condition on index
                {
                    local_index = index++; // Increment index in a critical section
                }
                strcpy(DNA_String[length][local_index], DNA_String[length - 1][i]);
                int curr_len = strlen(DNA_String[length][local_index]);
                DNA_String[length][local_index][curr_len] = DNA[j];
                DNA_String[length][local_index][curr_len + 1] = '\0';
            }
        }
        count = index; // Update count for the next length
        count_array[length - 1] = count; // Store the count for this length
    }

    // Print the DNA strings
    for (int len = 1; len <= LENGTH; len++) {
        for (int i = 0; i < count_array[len - 1]; i++) {
            printf("%s\n", DNA_String[len][i]);
        }
        printf("\n");
    }

    // Free allocated memory
    free(DNA_String);

    return 0;
}
