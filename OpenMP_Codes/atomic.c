#include <stdio.h>
#include <omp.h>

#define N 10 // You can replace 10 with your desired array size

int main() {
    int sum = 0;
    int array[N] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}; // Example array

    #pragma omp parallel 
    {
        // do some parallel computation

        #pragma omp for
        for (int i = 0; i < N; i++) {
            // Perform atomic addition to the sum
            #pragma omp atomic
            sum += array[i];
        }
    }

    printf("Sum: %d\n", sum);

    return 0;
}
