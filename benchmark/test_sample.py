import os
import subprocess
import re
import json
import argparse
from pathlib import Path
from concurrent.futures import ProcessPoolExecutor, as_completed
from tqdm import tqdm

path_to_FDD = "../"
path_to_SliQSim = "../../SliQSim"
path_to_DDSIM = "../../mqt-ddsim"

sample_results_dir = "./results/exp_sample_result"

experiment_dir = "./exp"
add_measure_dir = "./tmp_addmeasure"

smoke_results_dir = "./results/smoke_result/sample"

def replace_path_and_extension(file_path, old_base, new_base, new_extension = ".json"):
    path = Path(file_path)
    new_path = Path(new_base) / path.relative_to(old_base)
    new_path = new_path.with_suffix(new_extension)
    return new_path

def simulate_circuit(circuit,result, gate_config, time_limit, s,
                     FDD: bool, SliQSim: bool, DDSIM: bool):
    os.makedirs(os.path.dirname(result), exist_ok=True)
    output = {}
    if FDD:
        FDD_cmd = [os.path.join(path_to_FDD, "build/src/cudd_circuit_bdd"), "-s", s, "-t", "2", "-g", gate_config, "-f", circuit]
        try:
            out = subprocess.run(FDD_cmd, capture_output=True, timeout=time_limit)
            info = out.stdout.decode()
            total_time = re.findall(r'Total Runtime: (.*)s', info)[0]
            main_time = re.findall(r'Main Algo: (.*)s', info)[0]
            bdd_time = re.findall(r'Build MTBDD: (.*)s', info)[0]
            mod_plus_time = re.findall(r'Mod Plus: (.*)s', info)[0]
            eval_time = re.findall(r'Evaluate: (.*)s', info)[0]
            order_time = re.findall(r'Ordering: (.*)s', info)[0]
            mem_main = re.findall(r'main algo: (.*) bytes', info)[0]
            mem_order = re.findall(r'ordering: (.*) bytes', info)[0]
            res = {
                "total_time": total_time,
                "main_time": main_time,
                "bdd_time": bdd_time,
                "mod_plus_time": mod_plus_time,
                "eval_time": eval_time,
                "order_time": order_time,
                "memory_main": mem_main,
                "memory_order": mem_order
            }
        except subprocess.TimeoutExpired as e:
            res = {
                "total_time": f">{time_limit}s",
                "main_time": "",
                "bdd_time": "",
                "mod_plus_time": "",
                "eval_time": "",
                "order_time": "",
                "memory_main": "",
                "memory_order": "",
                "error describe": str(e)
            }
        except Exception as e:
            res = {
                "total_time": "Error",
                "main_time": "",
                "bdd_time": "",
                "mod_plus_time": "",
                "eval_time": "",
                "order_time": "",
                "memory_main": "",
                "memory_order": "",
                "error describe": str(e)
            }
        output["FDD"] = res
    if SliQSim:
        add_measure_file = replace_path_and_extension(circuit, experiment_dir, add_measure_dir, ".qasm")
        add_measure_file = add_measure_file.with_stem(f"{add_measure_file.stem}_add_measurements")
        add_measure_file.parent.mkdir(parents=True, exist_ok=True)
        if not add_measure_file.exists():
            add_measure_file.touch()
        os.system(f"python3 add_measurement.py -i {circuit} -o {add_measure_file}")
        SliQSim_cmd = [os.path.join(path_to_SliQSim, "SliQSim"), "--print_info", "--type", "0", "--sim_qasm", add_measure_file]
        try:
            out = subprocess.run(SliQSim_cmd, capture_output=True, timeout=time_limit)
            info = out.stdout.decode()
            time = re.findall(r"Runtime: (.*) seconds", info)[0]
            mem = re.findall(r"Peak memory usage: (.*) bytes", info)[0]
            res = {
                "time": time,
                "memory": mem
            }
        except subprocess.TimeoutExpired as e:
            res = {
                "time": f">{time_limit}s",
                "memory": "",
                "error describe": str(e)
            }
        except Exception as e:
            res = {
                "time": "Error",
                "memory": "",
                "error describe": str(e)
            }
        output["SliQSim"] = res
        os.remove(add_measure_file)
    if DDSIM:
        DDSIM_cmd = [os.path.join(path_to_DDSIM, "build/apps/mqt-ddsim-simple"), "--shots", "1", "--pm", "--ps", "--simulate_file", circuit]
        try:
            out = subprocess.run(DDSIM_cmd, capture_output=True, timeout=time_limit)
            info = out.stdout.decode()
            time = re.findall(r'.*"simulation_time": (.*),', info)[0]
            mem = re.findall(r'"peak_memory": (.*),', info)[0]
            res = {
                "time": time,
                "memory": mem
            }
        except subprocess.TimeoutExpired as e:
            res = {
                "time": f">{time_limit}s",
                "memory": "",
                "error describe": str(e)
            }
        except Exception as e:
            res = {
                "time": "Error",
                "memory": "",
                "error describe": str(e)
            }

        output["DDSIM"] = res
    with open(result, 'w') as f:
        json.dump(output, f, indent=4)

def test_BV(timeout: int):
    gate_config = "../gate_sets/default_2.json"
    n_list = [100, 500, 1000, 5000, 10000]
    result_dir = os.path.join(sample_results_dir, "bv")
    os.makedirs(result_dir, exist_ok=True)
    sort_setting = "1"
    max_workers = 5
    (FDD_option, SliQSim_option, DDSIM_option) = (True, True, True) 
    with ProcessPoolExecutor(max_workers=max_workers) as executor:
        futures = []
        for n in n_list:
            circuit_file = f"./exp/BV/bv{n}.qasm"
            result_file = f"{result_dir}/bv{n}.json"
            future = executor.submit(simulate_circuit, circuit_file, result_file, gate_config, timeout, sort_setting,
                                     FDD_option, SliQSim_option, DDSIM_option)
            futures.append(future)
        for future in tqdm(as_completed(futures), total=len(futures), desc="BV single sample test"):
            try:
                future.result()
            except Exception as e:
                print(f"Error: {e}")

def test_GHZ(timeout: int):
    gate_config = "../gate_sets/default_2.json"
    n_list = [100, 500, 1000, 5000, 10000]
    result_dir = os.path.join(sample_results_dir, "ghz")
    os.makedirs(result_dir, exist_ok=True)
    sort_setting = "1"
    max_workers = 5
    (FDD_option, SliQSim_option, DDSIM_option) = (True, True, True) 
    with ProcessPoolExecutor(max_workers=max_workers) as executor:
        futures = []
        for n in n_list:
            circuit_file = f"./exp/GHZ/{n}.qasm"
            result_file = f"{result_dir}/ghz{n}.json"
            future = executor.submit(simulate_circuit, circuit_file, result_file, gate_config, timeout, sort_setting,
                                     FDD_option, SliQSim_option, DDSIM_option)
            futures.append(future)
        for future in tqdm(as_completed(futures), total=len(futures), desc="GHZ single sample test"):
            try:
                future.result()
            except Exception as e:
                print(f"Error: {e}")    

def test_google_fast(timeout: int):
    gate_config = "../gate_sets/google.json"
    result_dir = os.path.join(sample_results_dir, "google_fast")
    os.makedirs(result_dir, exist_ok=True)
    sort_setting = "1"
    max_workers = 4 
    # TODO: check replace iSWAP
    (cz_FDD_option, cz_SliQSim_option, cz_DDSIM_option) = (True, True, True)
    (is_FDD_option, is_SliQSim_option, is_DDSIM_option) = (True, False, True)
    with ProcessPoolExecutor(max_workers=max_workers) as executor:
        futures = []
        for (n,k) in [(4,4), (4,5)]:
            cz_circuit_file = f"./exp/google/cz_v2_d10/inst_{n}x{k}_10_0.qasm"
            cz_result_file = f"{result_dir}/cz_v2_d10_{n}x{k}_10_0.json"
            future = executor.submit(simulate_circuit, cz_circuit_file, cz_result_file, gate_config, timeout, sort_setting,
                                     cz_FDD_option, cz_SliQSim_option, cz_DDSIM_option)
            futures.append(future)
            
            is_circuit_file = f"./exp/google/is_v1_d10/inst_{n}x{k}_10_0.qasm"
            is_result_file = f"{result_dir}/is_v1_d10_{n}x{k}_10_0.json"
            future = executor.submit(simulate_circuit, is_circuit_file, is_result_file, gate_config, timeout, sort_setting,
                                     is_FDD_option, is_SliQSim_option, is_DDSIM_option)
            futures.append(future)
        for future in tqdm(as_completed(futures), total=len(futures), desc="Google fast sample test"):
            try:
                future.result()
            except Exception as e:
                print(f"Error: {e}")

def test_google_all(timeout: int):
    gate_config = "../gate_sets/google.json"
    result_dir = os.path.join(sample_results_dir, "google_all")
    os.makedirs(result_dir, exist_ok=True)
    sort_setting = "1"
    max_workers = 32
    # TODO: check replace iSWAP
    (cz_FDD_option, cz_SliQSim_option, cz_DDSIM_option) = (True, True, True)
    (is_FDD_option, is_SliQSim_option, is_DDSIM_option) = (True, False, True)
    with ProcessPoolExecutor(max_workers=max_workers) as executor:
        futures = []
        for (n, k) in [(4,4), (4,5), (5,5)]:
            for i in range(10):
                cz_circuit_file = f"./exp/google/cz_v2_d10/inst_{n}x{k}_10_{i}.qasm"
                cz_result_file = f"{result_dir}/cz_v2_d10_{n}x{k}_10_{i}.json"
                future = executor.submit(simulate_circuit, cz_circuit_file, cz_result_file, gate_config, timeout, sort_setting,
                                         cz_FDD_option, cz_SliQSim_option, cz_DDSIM_option)
                futures.append(future)
                
                is_circuit_file = f"./exp/google/is_v1_d10/inst_{n}x{k}_10_{i}.qasm"
                is_result_file = f"{result_dir}/is_v1_d10_{n}x{k}_10_{i}.json"
                future = executor.submit(simulate_circuit, is_circuit_file, is_result_file, gate_config, timeout, sort_setting,
                                         is_FDD_option, is_SliQSim_option, is_DDSIM_option)
                futures.append(future)
        for future in tqdm(as_completed(futures), total=len(futures), desc="Google all sample test"):
            try:
                future.result()
            except Exception as e:
                print(f"Error: {e}")


def test_linear_network_fast(timeout: int):
    gate_config = "../gate_sets/default_2.json"
    result_dir = os.path.join(sample_results_dir, "linear_network_fast")
    os.makedirs(result_dir, exist_ok=True)
    sort_setting = "1"
    max_workers = 4
    (FDD_option, SliQSim_option, DDSIM_option) = (True, False, True)
    with ProcessPoolExecutor(max_workers=max_workers) as executor:
        futures = []
        for n in [20, 30]:
            for k in [5, 7]:
                circuit_file = f"./exp/linear_network/{n}_qubits/{k}_local/1x_0.qasm"
                result_file = f"{result_dir}/linear_network_{n}_qubits_{k}_local_1x_0.json"
                future = executor.submit(simulate_circuit, circuit_file, result_file, gate_config, timeout, sort_setting,
                                         FDD_option, SliQSim_option, DDSIM_option)
                futures.append(future)
        for future in tqdm(as_completed(futures), total=len(futures), desc="Linear network fast sample test"):
            try:
                future.result()
            except Exception as e:
                print(f"Error: {e}")
                

def test_linear_network_all(timeout: int):
    gate_config = "../gate_sets/default_2.json"
    result_dir = os.path.join(sample_results_dir, "linear_network_all")
    os.makedirs(result_dir, exist_ok=True)
    sort_setting = "1"
    max_workers = 32
    (FDD_option, SliQSim_option, DDSIM_option) = (True, False, True)
    with ProcessPoolExecutor(max_workers=max_workers) as executor:
        futures = []
        for n in [20, 30]:
            for k in [5, 7]:
                for i in range(10):
                    circuit_file = f"./exp/linear_network/{n}_qubits/{k}_local/1x_{i}.qasm"
                    result_file = f"{result_dir}/linear_network_{n}_qubits_{k}_local_1x_{i}.json"
                    future = executor.submit(simulate_circuit, circuit_file, result_file, gate_config, timeout, sort_setting,
                                             FDD_option, SliQSim_option, DDSIM_option)
                    futures.append(future)
        for future in tqdm(as_completed(futures), total=len(futures), desc="Linear network all sample test"):
            try:
                future.result()
            except Exception as e:
                print(f"Error: {e}")

def smoke_test(timeout: int):
    result_dir = smoke_results_dir
    os.makedirs(result_dir, exist_ok=True)
    gate_config = "../gate_sets/default_2.json"
    sort_setting = "1"
    (FDD_option, SliQSim_option, DDSIM_option) = (True, True, True) 
    max_workers = 4
    with ProcessPoolExecutor(max_workers=max_workers) as executor:
        futures = []
        
        # smallest BV circuit
        circuit_file = f"./exp/BV/bv80.qasm"
        result_file = f"{result_dir}/bv80.json"
        future = executor.submit(simulate_circuit, circuit_file, result_file, gate_config, timeout, sort_setting,
                                 FDD_option, SliQSim_option, DDSIM_option)
        futures.append(future)
        
        # smallest GHZ circuit
        circuit_file = f"./exp/GHZ/100.qasm"
        result_file = f"{result_dir}/ghz100.json"
        future = executor.submit(simulate_circuit, circuit_file, result_file, gate_config, timeout, sort_setting,
                                 FDD_option, SliQSim_option, DDSIM_option)
        futures.append(future)
        
        # smallest linear network circuit 
        circuit_file = f"./exp/linear_network/20_qubits/5_local/1x_0.qasm"
        result_file = f"{result_dir}/linear_network_20_qubits_5_local_1x_0.json"
        linear_SliQSim_option = False
        future = executor.submit(simulate_circuit, circuit_file, result_file, gate_config, timeout, sort_setting,
                                 FDD_option, linear_SliQSim_option, DDSIM_option)
        futures.append(future)
        
        # smallest Google circuit 
        google_gate_config = "../gate_sets/google.json"
        circuit_file = f"./exp/google/cz_v2_d10/inst_4x4_10_0.qasm"
        result_file = f"{result_dir}/cz_v2_d10_4x4_10_0.json"
        future = executor.submit(simulate_circuit, circuit_file, result_file, google_gate_config, timeout, sort_setting,
                                 FDD_option, SliQSim_option, DDSIM_option)
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