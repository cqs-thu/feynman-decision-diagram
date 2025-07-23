#!/bin/bash

start=$(date +%s)

python3 test_zerotozero.py --subtest google_all --time 3600 
python3 test_sample.py --subtest google_all --time 3600

end=$(date +%s)
runtime=$((end - start))
echo "table 2 all finished, runtime ${runtime} s" | tee -a table2_all.log

