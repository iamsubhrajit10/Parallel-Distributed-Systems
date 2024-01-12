# Compile & Execute using automated bash script
JUST RUN THE 'scriptCompileRun.sh' bash script file in linux to compile and execute the Optimized Code at once!
```
$ ./scriptCompileRun.sh
```

# Optimized Approach

## Distributive Property
To solve A=BC+BD, in the optimized approach, using the distributive property of matrices, we transformed BC+BD into B(C+D). This helps in reducing one matrix multiplication, which in turn optimizes the performance.
i.e. A=BC+BD=B(C+D)

## Transposing matrix
Also, in the conventional matrix multiplication, usually we access the first matrix row-major way, the second matrix in column-major way. To optimize according to the caches, which generally accumulate data row-wise, we've transposed the second matrix for the multiplication so it can be accessed row-major wise.

## Multi-threading using OpenMP
Now we've used multi-threading to optimize the performance further.
We've created 4 threads using openmp, which divides the matrices into 4 blocks (row-wise).

## gcc flag
To optimize during compilation, we've used -Ofast gcc flag.

## Compilation
To compile and run the optimized approach with profiler:
```
$ gcc -fopenmp -pg -Ofast -o optimizedOutput optimizedCompute.c
$ ./optimizedOutput
$ gprof optimizedOutput gmon.out > analysisOptimized.txt
```


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




