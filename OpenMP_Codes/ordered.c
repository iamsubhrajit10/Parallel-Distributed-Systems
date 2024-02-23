#include <stdio.h>
#include <omp.h>

#define N 10 // You can replace 10 with your desired value

int main() {
    #pragma omp parallel for ordered
    for (int i = 0; i < N; i++) {
        // do some parallel computation

        #pragma omp ordered
        {
            // print the result in order
            printf("Thread %d: Result for i = %d\n", omp_get_thread_num(), i);
        }
    }

    return 0;
}
