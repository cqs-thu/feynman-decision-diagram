#!/bin/bash

start=$(date +%s)

python3 test_zerotozero.py --subtest google_fast --time 3600 &
python3 test_sample.py --subtest google_fast --time 3600 &

wait 
end=$(date +%s)
runtime=$((end - start))
echo "table 2 small finished, runtime ${runtime} s" | tee -a table2_small.log

