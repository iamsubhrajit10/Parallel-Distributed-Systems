"A = BC+BD",
Done by Subhrajit (23210106)
# Language Used
C

# Optimized Approach
## Results:
Minimum Time Observed to compute A=BC+BD: 2.560208 seconds

### Output Snapshots
```
root@fedora:/media/sf_E_DRIVE/IIT Gandhinagar/2ndSem/Parallel & Distributed Systems/Assignment-0# ./optimizedOutput

Done! 
Time taken to compute A=BC+BD: 2.819531 seconds
root@fedora:/media/sf_E_DRIVE/IIT Gandhinagar/2ndSem/Parallel & Distributed Systems/Assignment-0# ./optimizedOutput

Done! 
Time taken to compute A=BC+BD: 2.592956 seconds
root@fedora:/media/sf_E_DRIVE/IIT Gandhinagar/2ndSem/Parallel & Distributed Systems/Assignment-0# ./optimizedOutput

Done! 
Time taken to compute A=BC+BD: 2.721839 seconds
root@fedora:/media/sf_E_DRIVE/IIT Gandhinagar/2ndSem/Parallel & Distributed Systems/Assignment-0# ./optimizedOutput

Done! 
Time taken to compute A=BC+BD: 2.669866 seconds
root@fedora:/media/sf_E_DRIVE/IIT Gandhinagar/2ndSem/Parallel & Distributed Systems/Assignment-0# ./optimizedOutput

Done! 
Time taken to compute A=BC+BD: 2.560208 seconds
```



## Distributive Property
To solve A=BC+BD, in the optimized approach, using the distributive property of matrices, we transformed BC+BD into B(C+D). This helps in reducing one matrix multiplication, which in turn optimizes the performance.
i.e. A=BC+BD=B(C+D)

## Transposing matrix
Also, in the conventional matrix multiplication, usually we access the first matrix row-major way, the second matrix in column-major way. To optimize according to the caches, which generally accumulate data row-wise, we've transposed the second matrix for the multiplication so it can be accessed row-major wise.
So, we added the two matrices C and D with the add() storing the result in C, then we've transposed the C matrix and stored it in D.
i.e.
```
C = C+D
D = (C)T
```

## Multi-threading using OpenMP
Now we've used multi-threading on multiplication to optimize the performance further.
We've created 4 threads using openmp, which divides the matrices into 4 blocks (row-wise).

## Overall execution order
```
C = C+D
D = (C)T
// Created 4 threads
A = B*D     // 4 threads performed execution on 4 separate consecutive chunks of rows.
```

## gcc flag
To optimize during compilation, we've used -Ofast gcc flag, and also we've used -fopenmp flag for 'omp.'. To use the gprof profiler, we've included -pg flag too.

## Compilation
To compile and run the optimized approach with gprof profiler:
```
$ gcc -fopenmp -pg -Ofast -o optimizedOutput optimizedCompute.c
$ ./optimizedOutput
$ gprof optimizedOutput gmon.out > analysisOptimized.txt
```

## Profiling
We find it difficult to profile the openmp parallel code using gprof, as in the profiling data it shows frame_dummy. Instead, to measure the time spent on computing A = BC+BD, we used omp_get_wtime() to measure the time spent. Before adding C and D we captured the starting time, and after the computation (add, transpose, multiply) is done, we captured the ending time.
We've shown the time spent on computing the A = BC+BD, as well as the overall time spent to execute the whole program.

## Initializing the Matrices
We've initialized the matrices B, C, and D randomly using the srand(time(NULL)) seeding with rand()%1000.



# Naive Approach
To solve A=BC+BD, in the naive approach we just multiplied BC and BD then added. And also the matrix multiplication involves conventional multiplication approach. For e.g. multiplying B and C, B is accessed row-wise in row-major order, while the matrix C is accessed column wise in column-major order.

## Compilation


To compile and run the naive approach source code with profiler manually:
```
$ gcc -pg -o naiveOutput naiveCompute.c
$ ./naiveOutput
$ gprof naiveOutput gmon.out > analysisNaive.txt
```
analysisNaive.txt will contain the profiling details for the naive approach to solve.
