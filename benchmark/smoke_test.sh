#!/bin/bash

echo "==== smoke test begin ===="
total_start=$(date +%s)

echo "running zero to zero smoke test...."
start=$(date +%s)
python3 test_zerotozero.py --subtest smoke_test --time 600
end=$(date +%s)
runtime=$((end - start))
echo "finish zero to zero smoke test in ${runtime} seconds"

echo "running sample smoke test...."
start=$(date +%s)
python3 test_sample.py --subtest smoke_test --time 600
end=$(date +%s)
runtime=$((end - start))
echo "finish sample smoke test in ${runtime} seconds"

echo "running equivalence check smoke test...."
start=$(date +%s)
python3 test_equivalence.py --subtest smoke_test --time 600
end=$(date +%s)
runtime=$((end - start))
echo "finish equivalence check smoke test in ${runtime} seconds"

total_end=$(date +%s)
total_time=$(($total_end - $total_start))
echo "==== smoke test end, total time: $total_time seconds ===="