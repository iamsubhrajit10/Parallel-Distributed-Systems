#include <stdio.h>
#include <omp.h>

int main() {
    int x = 0;

    #pragma omp parallel default(none) private(x)
    {
        // Simulating some parallel computation
        int thread_id = omp_get_thread_num();
        printf("Thread %d: Performing parallel computation\n", thread_id);

        // Ensuring visibility of the updated value of x to all threads
        #pragma omp flush(x)

        // Simulating the use of the updated value of x
        x += thread_id;

        // Printing the value of x from each thread
        printf("Thread %d: Value of x = %d\n", thread_id, x);
    }

    return 0;
}
