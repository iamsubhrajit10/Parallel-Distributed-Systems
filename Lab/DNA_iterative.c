#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char DNA[] = {'A', 'C', 'G', 'T'};
    char String[11][1024][11]; // 3D array for DNA strings

    // Initialize the first row with DNA characters
    for (int i = 0; i < 4; i++) {
        String[0][i][0] = DNA[i];
        String[0][i][1] = '\0';
    }
    // Generate DNA strings
    int count_array[10];
    // Generate DNA strings
int count = 4;
for (int length = 1; length <= 10; length++) {
    int index = 0;
    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < count; i++) {
            strcpy(String[length][index], String[length - 1][i]);
            strncat(String[length][index], &DNA[j], 1);
            index++;
        }
    }
    count = index; // Update count for the next length
    count_array[length-1]=count;
}



    // Print the DNA strings
    for (int len = 1; len <= 10; len++) {
        printf("DNA strings of length %d:\n", len);
        for (int i = 0; i < count_array[len-1]; i++) {
            printf("%s\n", String[len][i]);
        }
        printf("\n");
    }

    return 0;
}
