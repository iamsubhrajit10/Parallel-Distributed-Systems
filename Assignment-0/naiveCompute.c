#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define SIZE 2048

uint64_t A[SIZE][SIZE], B[SIZE][SIZE], C[SIZE][SIZE], D[SIZE][SIZE];

// Function to initialize a matrix with unique random values
void initializeMatrix(uint64_t matrix[SIZE][SIZE], int matrixIndex) {
    srand(time(NULL) + matrixIndex);
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            matrix[i][j] = rand() % 100;
        }
    }
}


void *matrixMultiply() {
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            A[i][j] = 0;
            for (int k = 0; k < SIZE; ++k) {
                A[i][j] += B[i][k] *C[k][j]+ B[i][k]* D[k][j];
            }
        }
    }
}

int main() {
    // Initialize matrices B, C, D
    initializeMatrix(B,0);
    initializeMatrix(C,1);
    initializeMatrix(D,2);

    matrixMultiply();

    printf("\nDone! \n");
    return 0;
}