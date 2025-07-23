#!/bin/bash

start=$(date +%s)

python3 test_zerotozero.py --subtest linear_network_fast --time 3600 &
python3 test_sample.py --subtest linear_network_fast --time 3600 &

wait 
end=$(date +%s)
runtime=$((end-start))
echo "table 5 small finished, runtime ${runtime} s" | tee -a table5_small.log


