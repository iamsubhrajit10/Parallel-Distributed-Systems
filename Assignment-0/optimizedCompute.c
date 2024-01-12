//#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <omp.h>

#define SIZE 2048
#define NUM_THREADS 4

uint64_t A[SIZE][SIZE], B[SIZE][SIZE], C[SIZE][SIZE], D[SIZE][SIZE];

// Structure to pass data to each thread
struct ThreadData {
    int start_row;
    int end_row;
};

// Function to initialize a matrix with unique random values
void initializeMatrix() {
    for (size_t i = 0; i < SIZE; ++i) {
        for (size_t j = 0; j < SIZE; ++j) {
            B[i][j] = rand() % 1000;
            C[i][j] = rand() % 1000;
            D[i][j] = rand() % 1000;
        }
    }
}


// Function to transpose matrix Y
void transposeMatrix(uint64_t (*target)[SIZE], uint64_t (*source)[SIZE]) {
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            target[j][i] = source[i][j];
        }
    }
}


//Function to print a matrix (for debugging purposes)
void printMatrix(uint64_t (*matrix)[SIZE]) {
    printf("\n");
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            printf("%ld ", matrix[i][j]);
        }
        printf("\n");
    }
}

void add() {
    for(int i=0; i<SIZE;i++){
        for (int j=0;j<SIZE;j++){
            C[i][j]+=D[i][j];
        }
    }
}


int main() {
    srand(time(NULL));

    //pthread_t threads[NUM_THREADS];
    

    // Initialize matrices B, C, D
    initializeMatrix();

    // printf("Matrix B: ");
    // printMatrix(B);
    // printf("Matrix C: ");
    // printMatrix(C);
    // printf("Matrix D: ");
    // printMatrix(D);

     // Record start time
    double start_time = omp_get_wtime();

    // C <- C+D
    add();

    // D<- (C)T
    transposeMatrix(D,C);

    // Divide the rows among threads
    struct ThreadData threadData[NUM_THREADS];
    int rowsPerThread = SIZE / NUM_THREADS;
    int extraRows = SIZE % NUM_THREADS;
    int startRow = 0;

    // openmp parallel threads
    #pragma omp parallel num_threads(NUM_THREADS)
    {
        int thread_id = omp_get_thread_num();
        int start_row = thread_id * rowsPerThread;
        int end_row = start_row + rowsPerThread + (thread_id < extraRows ? 1 : 0);

        for (int i = start_row; i < end_row; ++i) {
            for (int j = 0; j < SIZE; ++j) {
                int sum = 0;
                // Unroll the innermost loop
                for (int k = 0; k < SIZE; k += 2) {
                    sum += B[i][k] * (D[j][k]) + B[i][k + 1] * D[j][k + 1];
                }
                A[i][j] = sum;
            }
        }
    }
    // Record end time
    double end_time = omp_get_wtime();

    printf("\nDone! \n");
    // Calculate and print the elapsed time
    printf("Time taken to compute A=BC+BD: %f seconds\n", end_time - start_time);
    // printf("Matrix A: ");
    // printMatrix(A);
    return 0;
}