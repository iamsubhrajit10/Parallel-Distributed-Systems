#include <stdio.h>
#include <omp.h>

int main() {
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    // do some computation
                    printf("Task 1: Thread %d is performing computation\n", omp_get_thread_num());
                }

                #pragma omp task
                {
                    // do some computation
                    printf("Task 2: Thread %d is performing computation\n", omp_get_thread_num());
                }
            }

            // use the results of the tasks
            printf("Single Thread: Using results of the tasks\n");
        }
    }

    return 0;
}
