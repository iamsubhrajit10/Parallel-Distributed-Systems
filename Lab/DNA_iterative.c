#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char DNA[] = {'A', 'C', 'G', 'T'};
    char DNA_String[11][1024][12]; // 3D array for DNA strings with space for the null terminator
    int count_array[10]; // Array to store the count of DNA strings for each length

    // Initialize the first row with DNA characters
    for (int i = 0; i < 4; i++) {
        DNA_String[0][i][0] = DNA[i];
        DNA_String[0][i][1] = '\0'; // Null terminator
        printf("%s\n",DNA_String[0][i]);
    }
    

    // Generate DNA strings
    int count = 4;
    for (int length = 1; length <= 10; length++) {
        int index = 0;
        for (int j = 0; j < 4; j++) {
            for (int i = 0; i < count; i++) {
                // Inside the loop where you use strcpy
                printf("Copying: %s to %s\n", DNA_String[length - 1][i], DNA_String[length][index]);
                strcpy(DNA_String[length][index], DNA_String[length - 1][i]);
                strcat(DNA_String[length][index], &DNA[j]);
                printf("%s\n",DNA_String[length][index]);
                index++;
                
            }
        }
        if (length == 1){
                exit(0);
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
