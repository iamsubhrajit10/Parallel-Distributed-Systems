compile with 
gcc -o <binary> <source code .c> -fopenmp
Works for length upto 8, couldn't make work greater than 8 due to memory constraints

For the iterative version: find the source code "DNA_iterative.c"
For the quadcore version: find the source code "DNA_4threads.c"
For the hexacore version: find the source code "DNA_16threads.c"
