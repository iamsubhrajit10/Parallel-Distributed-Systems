#include <stdio.h>
#include <omp.h>

float x = 0.0;

// Example function for consuming values
void consume(float x_value, int a_value) {
    // Modify the value of 'x'
    x_value += a_value;
    // Do something with the consumed values
    printf("Consumed: x = %.2f, a = %d\n", x_value, a_value);
}

int main() {
    #pragma omp parallel
    {
        int a;

        // Simulating the assignment of a value to 'a'
        a = omp_get_thread_num() + 1;

        // Critical section to safely consume and modify values
        #pragma omp critical
        consume(x, a);
    }

    return 0;
}
