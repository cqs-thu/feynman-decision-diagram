import os
import subprocess
import re
import json
import argparse
from pathlib import Path
from concurrent.futures import ProcessPoolExecutor, as_completed
from tqdm import tqdm

exp_results_dir = "./results/exp_zero_to_zero_result"
smoke_results_dir = "./results/smoke_result/zero_to_zero"

def run_script_without_SliQSim(circuit_file, result_file, gate_config, time_limit, sort_setting):
    result = subprocess.run(f"python3 zero_to_zero.py --FDD --DDSIM -c {circuit_file} -r {result_file} -g {gate_config} -t {time_limit} -s {sort_setting}", shell=True)
    return result.returncode

def run_script(circuit_file, result_file, gate_config, time_limit, sort_setting):
    result = subprocess.run(f"python3 zero_to_zero.py --FDD --DDSIM --SliQSim -c {circuit_file} -r {result_file} -g {gate_config} -t {time_limit} -s {sort_setting}", shell=True)
    return result.returncode

def test_BV(timeout: int):
    gate_config = "../gate_sets/default_2.json"
    n_list = [100, 500, 1000, 5000, 10000]
    result_dir = os.path.join(exp_results_dir, "bv")
    os.makedirs(result_dir, exist_ok=True)
    sort_setting = "1"
    max_workers = 5
    with ProcessPoolExecutor(max_workers=max_workers) as executor:
        futures = [] 
        for n in n_list:
            circuit_file = f"./exp/BV/bv{n}.qasm"
            result_file = f"{result_dir}/bv{n}.json"
            future = executor.submit(run_script, circuit_file, result_file, gate_config, timeout, sort_setting)
            futures.append(future)
        for future in tqdm(as_completed(futures), total=len(futures), desc="BV zero to zero test"):
            try: 
                future.result()
            except Exception as e:
                print(f"Error: {e}")

def test_GHZ(timeout: int):
    gate_config = "../gate_sets/default_2.json"
    n_list = [100, 500, 1000, 5000, 10000]
    result_dir = os.path.join(exp_results_dir, "ghz")
    os.makedirs(result_dir, exist_ok=True) 
    sort_setting = "1"
    max_workers = 5
    with ProcessPoolExecutor(max_workers=max_workers) as executor:
        futures = []
        for n in n_list:
            circuit_file = f"./exp/GHZ/{n}.qasm"
            result_file = f"{result_dir}/ghz{n}.json"
            future = executor.submit(run_script, circuit_file, result_file, gate_config, timeout, sort_setting)
            futures.append(future)
        for future in tqdm(as_completed(futures), total=len(futures), desc="GHZ zero to zero test"):
            try: 
                future.result()
            except Exception as e:
                print(f"Error: {e}")


def test_google_fast(timeout: int):
    gate_config = "../gate_sets/google.json"
    result_dir = os.path.join(exp_results_dir, "google_fast")
    os.makedirs(result_dir, exist_ok=True)
    sort_setting = "1"
    max_workers = 4 
    with ProcessPoolExecutor(max_workers=max_workers) as executor:
        futures = [] 
        for (n, k) in [(4,4), (4,5)]:
            cz_circuit_file = f"./exp/google/cz_v2_d10/inst_{n}x{k}_10_0.qasm"
            cz_result_file = f"{result_dir}/cz_v2_d10_{n}x{k}_10_0.json"
            future = executor.submit(run_script, cz_circuit_file, cz_result_file, gate_config, timeout, sort_setting)
            futures.append(future)
             
            is_circuit_file = f"./exp/google/is_v1_d10/inst_{n}x{k}_10_0.qasm"
            is_result_file = f"{result_dir}/is_v1_d10_{n}x{k}_10_0.json"
            # TODO: check replace iSWAP   
            future = executor.submit(run_script_without_SliQSim, is_circuit_file, is_result_file, gate_config, timeout, sort_setting)
            futures.append(future)
        for future in tqdm(as_completed(futures), total=len(futures), desc="Google fast zero to zero test"):
            try: 
                future.result()
            except Exception as e:
                print(f"Error: {e}")
    

def test_google_all(timeout: int):
    gate_config = "../gate_sets/google.json"
    result_dir = os.path.join(exp_results_dir, "google_all")
    os.makedirs(result_dir, exist_ok=True)
    sort_setting = "1"
    max_workers = 32 
    with ProcessPoolExecutor(max_workers=max_workers) as executor:
        futures = [] 
        for (n, k) in [(4,4), (4,5), (5,5)]:
            for i in range(10):
                cz_circuit_file = f"./exp/google/cz_v2_d10/inst_{n}x{k}_10_{i}.qasm"
                cz_result_file = f"{result_dir}/cz_v2_d10_{n}x{k}_10_{i}.json"
                future = executor.submit(run_script, cz_circuit_file, cz_result_file, gate_config, timeout, sort_setting)
                futures.append(future)
             
                is_circuit_file = f"./exp/google/is_v1_d10/inst_{n}x{k}_10_{i}.qasm"
                is_result_file = f"{result_dir}/is_v1_d10_{n}x{k}_10_{i}.json"
                # TODO: check replace iSWAP   
                future = executor.submit(run_script_without_SliQSim, is_circuit_file, is_result_file, gate_config, timeout, sort_setting)
                futures.append(future)
        for future in tqdm(as_completed(futures), total=len(futures), desc="Google all zero to zero test"):
            try: 
                future.result()
            except Exception as e:
                print(f"Error: {e}")

def test_linear_network_fast(timeout: int):
    gate_config = "../gate_sets/default_2.json"
    result_dir = os.path.join(exp_results_dir, "linear_network_fast")
    os.makedirs(result_dir, exist_ok=True)
    sort_setting = "1"
    max_workers = 4 
    with ProcessPoolExecutor(max_workers=max_workers) as executor:
        futures = [] 
        for n in [20, 30]:
            for k in [5, 7]:
                circuit_file = f"./exp/linear_network/{n}_qubits/{k}_local/1x_0.qasm"
                result_file = f"{result_dir}/linear_network_{n}_qubits_{k}_local_1x_0.json"
                future = executor.submit(run_script_without_SliQSim, circuit_file, result_file, gate_config, timeout, sort_setting)
                futures.append(future)
        for future in tqdm(as_completed(futures), total=len(futures), desc="Linear network fast zero to zero test"):
            try: 
                future.result()
            except Exception as e:
                print(f"Error: {e}")

def test_linear_network_all(timeout:int):
    gate_config = "../gate_sets/default_2.json"
    result_dir = os.path.join(exp_results_dir, "linear_network_all")
    os.makedirs(result_dir, exist_ok=True)
    sort_setting = "1"
    max_workers = 32
    with ProcessPoolExecutor(max_workers=max_workers) as executor:
        futures = [] 
        for n in [20, 30, 40]:
            for k in [5, 7]:
                for i in range(10):
                    circuit_file = f"./exp/linear_network/{n}_qubits/{k}_local/1x_{i}.qasm"
                    result_file = f"{result_dir}/linear_network_{n}_qubits_{k}_local_1x_{i}.json"
                    future = executor.submit(run_script_without_SliQSim, circuit_file, result_file, gate_config, timeout, sort_setting)
                    futures.append(future)
        for future in tqdm(as_completed(futures), total=len(futures), desc="Linear network all zero to zero test"):
            try: 
                future.result()
            except Exception as e:
                print(f"Error: {e}")

def smoke_test(timeout: int):
    result_dir = smoke_results_dir
    os.makedirs(result_dir, exist_ok=True)
    gate_config = "../gate_sets/default_2.json"
    sort_setting = "1" 
    max_workers = 4 
    with ProcessPoolExecutor(max_workers=max_workers) as executor:
        futures = []
        
        # smallest BV circuit
        circuit_file = f"./exp/BV/bv80.qasm"
        result_file = f"{result_dir}/bv80.json"
        future = executor.submit(run_script, circuit_file, result_file, gate_config, timeout, sort_setting)
        futures.append(future)
        
        # smallest GHZ circuit
        circuit_file = f"./exp/GHZ/100.qasm"
        result_file = f"{result_dir}/ghz100.json"
        future = executor.submit(run_script, circuit_file, result_file, gate_config, timeout, sort_setting)
        futures.append(future)
        
        # smallest linear network circuit 
        circuit_file = f"./exp/linear_network/20_qubits/5_local/1x_0.qasm"
        result_file = f"{result_dir}/linear_network_20_qubits_5_local_1x_0.json"
        future = executor.submit(run_script_without_SliQSim, circuit_file, result_file, gate_config, timeout, sort_setting)
        futures.append(future)
        
        # smallest Google circuit 
        google_gate_config = "../gate_sets/google.json"
        circuit_file = f"./exp/google/cz_v2_d10/inst_4x4_10_0.qasm"
        result_file = f"{result_dir}/cz_v2_d10_4x4_10_0.json"
        future = executor.submit(run_script, circuit_file, result_file, google_gate_config, timeout, sort_setting)
        futures.append(future)  
        
        for future in tqdm(as_completed(futures), total=len(futures), desc="Smoke test"):
            try: 
                future.result()
            except Exception as e:
                print(f"Error: {e}")
        

if __name__ == '__main__':
    parser = argparse.ArgumentParser() 
    parser.add_argument('-t', '--time', type=int, default=3600, help="Time Limit")
    parser.add_argument('-s', '--subtest', type=str, default='bv', help="Subtest")
    
    args = parser.parse_args()
    time_limit = args.time
    subtest = args.subtest
    
    if subtest == 'bv':
        test_BV(time_limit)
    elif subtest == 'ghz':
        test_GHZ(time_limit)
    elif subtest == 'google_fast':
        test_google_fast(time_limit)
    elif subtest == 'google_all':
        test_google_all(time_limit)
    elif subtest == 'linear_network_fast':
        test_linear_network_fast(time_limit)
    elif subtest == 'linear_network_all':
        test_linear_network_all(time_limit)
    elif subtest == 'smoke_test':
        smoke_test(time_limit)
    else:
        print("Invalid subtest")
    