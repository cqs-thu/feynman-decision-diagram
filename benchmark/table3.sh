#!/bin/bash

start=$(date +%s)

python3 test_zerotozero.py --subtest bv --time 3600 &
python3 test_sample.py --subtest bv --time 3600 &

wait 
end=$(date +%s)
runtime=$((end - start))
echo "table 3 finished, runtime ${runtime} s" | tee -a table3.log


