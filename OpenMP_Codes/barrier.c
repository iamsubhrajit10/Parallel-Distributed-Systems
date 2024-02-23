#include <stdio.h>
#include <omp.h>

int main() {
    #pragma omp parallel
    {
        // do some parallel computation
        int thread_id = omp_get_thread_num();
        printf("Thread %d: Performing parallel computation\n", thread_id);

        // Barrier synchronization point
        #pragma omp barrier

        // do some more parallel computation
        printf("Thread %d: Performing more parallel computation\n", thread_id);
    }

    return 0;
}
