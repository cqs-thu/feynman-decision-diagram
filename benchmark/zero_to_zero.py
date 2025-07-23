import os
import subprocess
import re
import json
import argparse

path_to_FDD = "../"
path_to_SliQSim = "../../SliQSim"
path_to_DDSIM = "../../mqt-ddsim"

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Test FDD, SliQSim, and DDSIM")
    parser.add_argument("-c", type=str, required=True, help="Path to the circuit")
    parser.add_argument("-r", type=str, required=True, help="Path to the result")
    parser.add_argument("-t", type=int, default=3600, help="Time Limit")
    parser.add_argument("-g", type=str, required=True, help="Path to gate config")
    parser.add_argument("-s", type=str, default="1", help="Ordering strategy")
    parser.add_argument("--FDD", action="store_true", help="Run FDD")
    parser.add_argument("--SliQSim", action="store_true", help="Run SliQSim")
    parser.add_argument("--DDSIM", action="store_true", help="Run DDSIM")

    args = parser.parse_args()
    circuit = args.c
    result = args.r
    time_limit = args.t
    gate_config = args.g
    s = args.s
    FDD = args.FDD
    SliQSim = args.SliQSim
    DDSIM = args.DDSIM

    FDD_cmd = [os.path.join(path_to_FDD, "build/src/cudd_circuit_bdd"), "-s", s, "-g", gate_config, "-f", circuit]
    SliQSim_cmd = [os.path.join(path_to_SliQSim, "SliQSim"), "--print_info", "--type", "2", "--sim_qasm", circuit]
    DDSIM_cmd = [os.path.join(path_to_DDSIM, "build/apps/mqt-ddsim-simple"), "--pv", "--ps", "--simulate_file", circuit]

    output = {}

    # FDD
    if FDD:
        try:
            out = subprocess.run(FDD_cmd, capture_output=True, timeout=time_limit)
            info = out.stdout.decode()
            amp = re.findall(r'zero to zero is \[(.*)\]', info)[0]
            total_time = re.findall(r'Total Runtime: (.*)s', info)[0]
            main_time = re.findall(r'Main Algo: (.*)s', info)[0]
            order_time = re.findall(r'Ordering: (.*)s', info)[0]
            mem_main = re.findall(r'main algo: (.*) bytes', info)[0]
            mem_order = re.findall(r'ordering: (.*) bytes', info)[0]
            res = {
                "amplitude": amp,
                "total_time": total_time,
                "main_time": main_time,
                "order_time": order_time,
                "memory_main": mem_main,
                "memory_order": mem_order
            }
        except subprocess.TimeoutExpired as e:
            res = {
                "amplitude": "",
                "total_time": f">{time_limit}s",
                "main_time": "",
                "order_time": "",
                "memory_main": "",
                "memory_order": ""
            }
        except Exception as e:
            res = {
                "amplitude": "",
                "total_time": "Error",
                "main_time": "",
                "order_time": "",
                "memory_main": "",
                "memory_order": "",
                "Error": str(e)
            }


        output["FDD"] = res

    # SliQSim
    if SliQSim:
        try:
            out = subprocess.run(SliQSim_cmd, capture_output=True, timeout=time_limit)
            info = out.stdout.decode()
            amp = re.findall(r'"zero2zero": "(.*)"', info)[0]
            time = re.findall(r"Runtime: (.*) seconds", info)[0]
            mem = re.findall(r"Peak memory usage: (.*) bytes", info)[0]
            res = {
                "amplitude": amp,
                "time": time,
                "memory": mem
            }
        except subprocess.TimeoutExpired as e:
            res = {
                "amplitude": "",
                "time": f">{time_limit}s",
                "memory": ""
            }
        except Exception as e:
            res = {
                "amplitude": "",
                "time": "Error",
                "memory": "",
                "Error": str(e)
            }

        output["SliQSim"] = res

    # DDSIM
    if DDSIM:
        try:
            out = subprocess.run(DDSIM_cmd, capture_output=True, timeout=time_limit)
            # print(out.stdout)
            info = out.stdout.decode()
            amps = re.findall(r'\[\n      (.*),\n      (.*)\n    \]', info)[0]
            amp = f"({amps[0]},{amps[1]})"
            time = re.findall(r'.*"simulation_time": (.*),', info)[0]
            mem = re.findall(r'"peak_memory": (.*),', info)[0]
            res = {
                "amplitude": amp,
                "time": time,
                "memory": mem
            }
        except subprocess.TimeoutExpired as e:
            res = {
                "time": f">{time_limit}s",
                "memory": ""
            }
        except Exception as e:
            res = {
                "time": "Error",
                "memory": "",
                "Error": str(e)
            }

        output["DDSIM"] = res

    with open(result, "w") as f:
        json.dump(output, f, indent=4)