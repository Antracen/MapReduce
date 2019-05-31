#!/bin/bash

# This script will extract the results from our bash script benchmarks and compile them into a CSV file

echo "Weak scaling:"
echo "Weak? (0/1),File size (GB),Procs (number),Time (indulgigng print)"

# of=/cfs/nobackup/m/mawass/MapReduce/Results/size_${file_size}GB_procs_${procs}_strong.txt


for filename in ../../Results/*
do
    if [[ $filename =~ "strong" ]]; then
        echo -n "1,"
    else
        echo -n "0,"
    fi
    echo $filename | grep -Poi "size_\K\d+" | tr -d '\n'
    echo -n ","
    echo $filename | grep -Poi "procs_\K\d+" | tr -d '\n'
    echo -n ","
    tail -n 2 $filename | grep -Poi "Time including printing = \K\d+.\d+"
done
