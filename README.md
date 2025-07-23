# FeynmanDD: Classical Data Structure for Quantum Circuit Analysis

## Build

```commandline
git clone url-of-this-repo
git submodule update --init --recursive
make build test
```

## Execution

Compute amplitude
```commandline
./build/src/cudd_circuit_bdd -h 
./build/src/cudd_circuit_bdd -f benchmark/exp/google/cz_v2_d10/inst_4x4_10_0.qasm -g gate_sets/google.json -s 1 -t 0
```

Sample one output string
```commandline
./build/src/cudd_circuit_bdd -f benchmark/exp/google/cz_v2_d10/inst_4x4_10_0.qasm -g gate_sets/google.json -s 1 -t 2
```

Check equivalence of two circuits
```commandline
./build/src/cudd_circuit_bdd -f benchmark/exp/google/cz_v2_d10/inst_4x4_10_0.qasm -f benchmark/exp/google/cz_v2_d10/inst_4x4_10_1.qasm -g gate_sets/google.json -s 1 -t 4
```
