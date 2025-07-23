#!/bin/bash

start=$(date +%s)

python3 test_equivalence.py --subtest revlib --time 600 
python3 test_equivalence.py --subtest bv_and_ghz --time 600
python3 test_equivalence.py --subtest linear_network --time 600  
python3 test_equivalence.py --subtest google --time 600 

end=$(date +%s)
runtime=$((end - start))
echo "table 7 finished, runtime ${runtime} s" | tee -a table7.log

