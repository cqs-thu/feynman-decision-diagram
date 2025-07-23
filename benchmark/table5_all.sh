#!/bin/bash

start=$(date +%s)

python3 test_zerotozero.py --subtest linear_network_all --time 3600 
python3 test_sample.py --subtest linear_network_all --time 3600 

end=$(date +%s)
runtime=$((end-start))
echo "table 5 all finished, runtime ${runtime} s" | tee -a table5_all.log

