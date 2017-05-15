#!/bin/bash

# Copyright 2017 Meritxell Jordana Gavieiro
# Copyright 2017 Albert Merino Pulido
# Copyright 2017 Marc Sanchez Fauste

echo "Width Items Result ProblemSolvingTime TotalTime Nthreads"

for res in output/*.o*;  do
    result=$(cat $res | awk -F ':' '{print $1 " " $2 " " $3 " " $4 " " $5}')
    threads=$(echo "$res" | awk -F '.' '{print $3}')
    echo "$result $threads"
done
