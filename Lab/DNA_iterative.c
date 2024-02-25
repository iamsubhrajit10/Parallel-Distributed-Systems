#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROWS 11
#define COLS 262144
#define LENGTH 13

int main() {
    // Allocate memory for the DNA_String array
    char DNA[] = {'A', 'C', 'G', 'T'};
    int count_array[10]; // Array to store the count of DNA strings for each length
    char (*DNA_String)[COLS][LENGTH] = malloc(ROWS * sizeof(*DNA_String));
    
    if (DNA_String == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // Access and print elements of the array
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            printf("DNA_String[%d][%d]: ", i, j);
            for (int k = 0; k < LENGTH; k++) {
                printf("%c", DNA_String[i][j][k]);
            }
            printf("\n");
        }
    }
    

    // Generate DNA strings
    int count = 4;
    for (int length = 1; length <= 10; length++) {
        int index = 0;
        for (int j = 0; j < 4; j++) {
            for (int i = 0; i < count; i++) {
                strcpy(DNA_String[length][index], DNA_String[length - 1][i]);
                DNA_String[length][index][strlen(DNA_String[length][index])] = DNA[j];
                DNA_String[length][index][strlen(DNA_String[length][index])+1] = '\0';
                index++;
            }
        }
        count = index; // Update count for the next length
        count_array[length - 1] = count; // Store the count for this length
    }

    // Print the DNA strings
    for (int len = 1; len <= 10; len++) {
        printf("DNA strings of length %d:\n", len);
        for (int i = 0; i < count_array[len - 1]; i++) {
            printf("%s\n", DNA_String[len][i]);
        }
        printf("\n");
    }

    return 0;
}
