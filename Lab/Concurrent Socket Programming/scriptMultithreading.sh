#!/bin/bash


# Record the start time
start_time=$(date +%s.%N)

# Compile 10 instances of the same file
for i in {1..100}; do
    gcc -o "client$i" clientCode.c
done

# Run the compiled executables
for i in {1..100}; do
    "./client$i" "$i" &
done

# Optionally, wait for all processes to finish
wait
# Record the end time
end_time=$(date +%s.%N)

# Calculate the execution time
execution_time=$(echo "$end_time - $start_time" | bc)

# Print the execution time
echo "Total execution time (Multi-threading): $execution_time seconds"