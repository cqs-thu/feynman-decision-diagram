#!/bin/bash

echo "==== begin ===="
total_start=$(date +%s)

tables=("table2_all.sh" "table3.sh" "table4.sh" "table5_all.sh" "table7.sh")

for script in ${tables[@]}; do 
    echo "running $script"
    bash $script
    echo "done $script"
done 

total_end=$(date +%s)
total_time=$(($total_end - $total_start))
echo "==== end, total time: $total_time seconds ===="