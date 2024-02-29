#!/bin/bash

# Read the CSV file
csv_file="input.csv" 
rows=$(tail -n +2 "$csv_file" | wc -l)

# Calculate the number of processes
N=$((rows + 2))

# Compile the server program
gcc -o server server.c -fopenmp
# Run the server program 
sudo ./server &

# Compile the client.c file
mpicc -o client client.c

# Run the client program using mpirun
mpirun -np "$N" ./client