#!/bin/bash

THREADS=$(seq -s " " 2 $(nproc))
PROBLEMS=$(find testbed -name "*" -type f)
SERIAL="./knapsackBB_serial"
PARALLEL="./knapsackBB_mpi"
THREADS_FILE="threads.csv"

if [ ! -d "output" ]; then
    mkdir output
fi

echo $THREADS > $THREADS_FILE

for PROBLEM in $PROBLEMS; do
    THREAD=1
    PROBLEM_NAME=$(head -n 1 $PROBLEM | awk '{print $1 "_" $2}')
    echo -e "Running $PROBLEM_NAME.$THREAD..."
    $SERIAL $PROBLEM | tee "output/P.$PROBLEM_NAME.$THREAD.o"
    for THREAD in $THREADS; do
        echo -e "Running $PROBLEM_NAME.$THREAD..."
        mpiexec -n $THREAD $PARALLEL $PROBLEM | tee "output/P.$PROBLEM_NAME.$THREAD.o"
    done
done
