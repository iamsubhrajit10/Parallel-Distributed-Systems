#!/bin/bash

# # Naive Version
# echo "Compiling the Naive Version..."
# gcc -pg -o naiveOutput naiveCompute.c

# echo "Executing the Naive Version..."
# # Record the start time
# start_time=$(date +%s.%N)
# ./naiveOutput
# # Record the end time
# wait
# end_time=$(date +%s.%N)
# # Calculate the execution time
# execution_time=$(echo "$end_time - $start_time" | bc)
# echo "Total execution time for Naive Version: $execution_time seconds"
# gprof naiveOutput gmon.out > analysisNaive.txt

#Optimized Version

echo "Compiling the Optimized Version"
gcc -fopenmp -pg -Ofast -o optimizedOutput optimizedCompute.c

# Record the start time
echo "Executing the Optimized Version"
start_time=$(date +%s.%N)
./optimizedOutput
# Record the end time
wait
end_time=$(date +%s.%N)
# Calculate the execution time
execution_time=$(echo "$end_time - $start_time" | bc)
echo "Total execution time for Optimized Version: $execution_time seconds"
gprof optimizedOutput gmon.out > analysisOptimized.txt

