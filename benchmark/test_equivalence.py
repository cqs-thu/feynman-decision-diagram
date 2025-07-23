import os 
import re
import subprocess
import argparse
import json
from tqdm import tqdm
from concurrent.futures import ProcessPoolExecutor, as_completed

path_to_FDD = "../"
path_to_MQT = "./"

equivalence_results_dir = "./results/equivalence_result"
smoke_results_dir = "./results/smoke_result/equivalence"

def run_fdd_check(file1, file2, gate_config, time_limit, sort_config):
    FDD_cmd = [os.path.join(path_to_FDD, "build/src/cudd_circuit_bdd"),
               "-s", sort_config, 
               "-t", "4",
               "-g", gate_config,
               "-f", file1,
               "-f", file2]
    try:
        out = subprocess.run(FDD_cmd, capture_output=True, timeout=time_limit)
        info = out.stdout.decode()
        answer = re.findall(r'The circuits are (.*)\.', info)[0]
        total_time = re.findall(r'Total Runtime: (.*)s', info)[0]
        bdd_time = re.findall(r'Build MTBDD: (.*)s', info)[0]
        mod_plus_time = re.findall(r'Mod Plus: (.*)s', info)[0]
        eval_time = re.findall(r'Evaluate: (.*)s', info)[0]
        order_time = re.findall(r'Ordering: (.*)s', info)[0]
        mem_main = re.findall(r'main algo: (.*) bytes', info)[0]
        mem_order = re.findall(r'ordering: (.*) bytes', info)[0]
        res = {
           "ans": answer,
           "total_time": total_time,
           "bdd_time": bdd_time,
           "mod_plus_time": mod_plus_time,
           "eval_time": eval_time,
           "order_time": order_time,
           "memory_main": mem_main,
           "memory_order": mem_order
        }
    except subprocess.TimeoutExpired as e: 
        res = {
           "ans": "Don't know",
           "total_time": f">{time_limit}s",
           "memory_main": "",
           "memory_order": "",
           "error describe": str(e)
        }
    except Exception as e: 
        res = {
           "ans": "Don't know",
           "total_time": "Error",
           "memory_main": "",
           "memory_order": "",
           "error describe": str(e)
        }
    return res 

def run_mqt_check(file1, file2, time_limit):
    QCEC_cmd = ["python3", os.path.join(path_to_MQT, "mqt_check.py"),
                "-f1", file1,
                "-f2", file2]
    try:
        out = subprocess.run(QCEC_cmd, capture_output=True, timeout=time_limit)
        info = out.stdout.decode()
        answer = re.findall(r'The circuits are (.*)\.', info)[0]
        time = re.findall(r'time: (.*)s', info)[0]
        peak_memory = re.findall(r'peak memory usage: (.*)MB', info)[0]
        current_memory = re.findall(r'current memory usage: (.*)MB', info)[0]
        res = {
            "ans": answer,
            "time": time,
            "peak_memory": peak_memory,
            "current_memory": current_memory
        }
    except subprocess.TimeoutExpired as e: 
        res = {
            "ans": "Don't know",
            "time": f">{time_limit}s",
            "error describe": str(e)
        }
    except Exception as e: 
        res = {
            "ans": "Don't know",
            "time": "Error",
            "error describe": str(e)
        }
    return res 

def check_equivalence(file1, file2, result_file_path, gate_config, time_limit, sort_config):
    # print(f"Checking equivalence of {file1} and {file2}")
    res1 = run_fdd_check(file1, file2, gate_config, time_limit, sort_config)
    res2 = run_mqt_check(file1, file2, time_limit)
    # write to file 
    output = {
        "FDD": res1,
        "MQT": res2
    }
    with open(result_file_path, "w") as f:
        json.dump(output, f, indent=4)

def check_equivalence_group(gate_config: str, time_limit:int, 
                            sort_setting: str, test_name: str,
                            circuit_base_dir: str,
                            origin_files: list[str],
                            description: str,
                            max_workers: int = 32):
    result_dir = os.path.join(equivalence_results_dir, test_name)
    result_opt_dir = os.path.join(result_dir, "opt3")
    result_missing_dir = os.path.join(result_dir, "missing")
    result_reverse_dir = os.path.join(result_dir, "reverse")
    os.makedirs(result_dir, exist_ok=True)
    os.makedirs(result_opt_dir, exist_ok=True)
    os.makedirs(result_missing_dir, exist_ok=True)
    os.makedirs(result_reverse_dir, exist_ok=True)
    
    origin_path = os.path.join(circuit_base_dir, "origin")
    opt_path = os.path.join(circuit_base_dir, "opt3")
    missing_path = os.path.join(circuit_base_dir, "missing")
    reverse_path = os.path.join(circuit_base_dir, "reverse")
    
    with ProcessPoolExecutor(max_workers=max_workers) as executor:
        futures = []
        for filename in origin_files:
            origin_file_path = os.path.join(origin_path, filename)
            # generate equivalence task
            opt_file_path = os.path.join(opt_path, filename.replace('.qasm', 'opt3.qasm'))
            if os.path.isfile(opt_file_path):
                result_file_path = os.path.join(result_opt_dir, filename.replace('.qasm', '_opt3.json'))
                future = executor.submit(check_equivalence,
                                         origin_file_path,
                                         opt_file_path,
                                         result_file_path,
                                         gate_config,
                                         time_limit,
                                         sort_setting)
                futures.append(future)
            # generate fail task: missing gate 
            missing_file_path = os.path.join(missing_path, filename.replace('.qasm', '_miss.qasm'))
            if os.path.isfile(missing_file_path):
                result_file_path = os.path.join(result_missing_dir, filename.replace('.qasm', '_miss.json'))
                future = executor.submit(check_equivalence,
                                         origin_file_path,
                                         missing_file_path,
                                         result_file_path,
                                         gate_config,
                                         time_limit,
                                         sort_setting)
                futures.append(future)
            # generate fail task: reverse gate
            reverse_file_path = os.path.join(reverse_path, filename.replace('.qasm', '_rev.qasm'))
            if os.path.isfile(reverse_file_path):
                result_file_path = os.path.join(result_reverse_dir, filename.replace('.qasm', '_rev.json'))
                future = executor.submit(check_equivalence,
                                         origin_file_path,
                                         reverse_file_path,
                                         result_file_path,
                                         gate_config,
                                         time_limit,
                                         sort_setting)
                futures.append(future)
        
        for future in tqdm(as_completed(futures), total=len(futures), desc=description):
            try:
                future.result()
            except Exception as e:
                print(f"Error: {e}")


def test_revlib(time_limit):
    gate_config = "../gate_sets/default_2.json"
    sort_setting = "1"
    max_workers = 32
    test_name = "revlib"
    circuit_base_dir = "./equivalence/RevLib/default"
    
    origin_path = os.path.join(circuit_base_dir, "origin")
    origin_files = os.listdir(origin_path)
    origin_files = [os.path.basename(f) for f in origin_files]
    
    description = "revlib equivalence test"
    check_equivalence_group(
        gate_config=gate_config,
        time_limit=time_limit,
        sort_setting=sort_setting,
        test_name=test_name,
        circuit_base_dir=circuit_base_dir,
        origin_files=origin_files,
        description=description,
        max_workers=max_workers
    )

def test_bv_and_ghz(time_limit):
    gate_config = "../gate_sets/default_2.json"
    sort_setting = "1"
    max_workers = 32 
    test_name = "bv_and_ghz"
    circuit_base_dir = "./equivalence/simulate_circuits/default"
    
    origin_files = []
    for n in [100, 500]:
        origin_files.append(f'bv{n}.qasm')
    for n in [100, 500, 1000, 5000, 10000]:
        origin_files.append(f'GHZ_{n}.qasm')
    
    description = "bv and ghz equivalence test"
    check_equivalence_group(
        gate_config=gate_config,
        time_limit=time_limit,
        sort_setting=sort_setting,
        test_name=test_name,
        circuit_base_dir=circuit_base_dir,
        origin_files=origin_files,
        description=description,
        max_workers=max_workers
    )
    

def test_linear_network(time_limit):
    gate_config = "../gate_sets/default_2.json"
    sort_setting = "1"
    max_workers = 32 
    test_name = "linear_network"
    circuit_base_dir = "./equivalence/simulate_circuits/default"
    
    origin_files = []
    for n in [20, 30, 40]:
        for k in [5, 7]:
            origin_files.append(f'linear_{n}_{k}_1_0.qasm')
    
    description = "linear network equivalence test"
    check_equivalence_group(
        gate_config=gate_config,
        time_limit=time_limit,
        sort_setting=sort_setting,
        test_name=test_name,
        circuit_base_dir=circuit_base_dir,
        origin_files=origin_files,
        description=description,
        max_workers=max_workers
    )

def test_google(time_limit):
    gate_config = "../gate_sets/google.json"
    sort_setting = "1"
    max_workers = 32
    test_name = "google"
    circuit_base_dir = "./equivalence/simulate_circuits/google"
    
    origin_files = []
    for (n, k) in [(4,4), (4,5), (5,5)]:
        origin_files.append(f'cz_v2_d10_inst_{n}x{k}_10_0.qasm')
    for (n, k) in [(4,4), (4,5), (5,5), (5,6), (6,6)]:
        origin_files.append(f'cz_v2_d5_inst_{n}x{k}_5_0.qasm')
        
    description = "google equivalence test"
    check_equivalence_group(
        gate_config=gate_config,
        time_limit=time_limit,
        sort_setting=sort_setting,
        test_name=test_name,
        circuit_base_dir=circuit_base_dir,
        origin_files=origin_files,
        description=description,
        max_workers=max_workers
    )
    
def smoke_test(time_limit):
    result_dir = smoke_results_dir
    result_opt_dir = os.path.join(result_dir, "opt3")
    result_missing_dir = os.path.join(result_dir, "missing")
    result_reverse_dir = os.path.join(result_dir, "reverse")
    
    os.makedirs(result_dir, exist_ok=True)
    os.makedirs(result_opt_dir, exist_ok=True)
    os.makedirs(result_missing_dir, exist_ok=True)
    os.makedirs(result_reverse_dir, exist_ok=True)
    
    sort_setting = "1"
    max_workers = 10 
    # gate_config = "../gate_sets/default_2.json"
    with ProcessPoolExecutor(max_workers=max_workers) as executor:
        futures = []

        for (subdir, circuit_base_name, gate_config) in [
          ("RevLib/default", "4gt11-v1_85", "../gate_sets/default_2.json"),
          ("simulate_circuits/default", "GHZ_100", "../gate_sets/default_2.json"),
          ("simulate_circuits/default", "linear_20_5_1_0", "../gate_sets/default_2.json"),
          ("simulate_circuits/google", "cz_v2_d5_inst_4x4_5_0", "../gate_sets/google.json")  
        ]:
            origin_circuit_file = f"./equivalence/{subdir}/origin/{circuit_base_name}.qasm"
            opt_circuit_file = f"./equivalence/{subdir}/opt3/{circuit_base_name}opt3.qasm"
            if os.path.isfile(opt_circuit_file):
                result_file = f"{result_opt_dir}/{circuit_base_name}_opt3.json"
                future = executor.submit(check_equivalence,
                                        origin_circuit_file,
                                        opt_circuit_file,
                                        result_file,
                                        gate_config,
                                        time_limit,
                                        sort_setting)
                futures.append(future)
            
            missing_circuit_file = f"./equivalence/{subdir}/missing/{circuit_base_name}_miss.qasm"
            if os.path.isfile(missing_circuit_file):
                result_file = f"{result_missing_dir}/{circuit_base_name}_miss.json"
                future = executor.submit(check_equivalence,
                                        origin_circuit_file,
                                        missing_circuit_file,
                                        result_file,
                                        gate_config,
                                        time_limit,
                                        sort_setting)
                futures.append(future)
            
            reverse_circuit_file = f"./equivalence/{subdir}/reverse/{circuit_base_name}_rev.qasm"
            if os.path.isfile(reverse_circuit_file):
                result_file = f"{result_reverse_dir}/{circuit_base_name}_rev.json"
                future = executor.submit(check_equivalence,
                                        origin_circuit_file,
                                        reverse_circuit_file,
                                        result_file,
                                        gate_config,
                                        time_limit,
                                        sort_setting)
                futures.append(future)
        for future in tqdm(as_completed(futures), total=len(futures), desc="Smoke test"):
            try:
                future.result()
            except Exception as e:
                print(f"Error: {e}")
        
        

if __name__ == '__main__':
    parser = argparse.ArgumentParser() 
    parser.add_argument('-t', '--time', type=int, default=600, help="Time Limit")
    parser.add_argument('-s', '--subtest', type=str, default='bv', help="Subtest")
    
    args = parser.parse_args()
    time_limit = args.time
    subtest = args.subtest
    
    if subtest == 'revlib':
        test_revlib(time_limit)
    elif subtest == 'bv_and_ghz':
        test_bv_and_ghz(time_limit)
    elif subtest == 'linear_network':
        test_linear_network(time_limit) 
    elif subtest == 'google':
        test_google(time_limit)
    elif subtest == 'smoke_test':
        smoke_test(time_limit)
    else:
        print("Invalid subtest")