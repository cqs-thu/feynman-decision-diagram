#!/bin/bash

start=$(date +%s)

python3 test_zerotozero.py --subtest ghz --time 3600 &
python3 test_sample.py --subtest ghz --time 3600 &

wait 
end=$(date +%s)
runtime=$((end - start))
echo "table 4 finished, runtime ${runtime} s" | tee -a table4.log
