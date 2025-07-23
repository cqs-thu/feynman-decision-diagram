from mqt import qcec 
import argparse
import tracemalloc
import time

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-f1', '--file1', type=str, required=True, help='path to circuit 1')
    parser.add_argument('-f2', '--file2', type=str, required=True, help='path to circuit 2')
    args = parser.parse_args()
    circuit_file_1 = args.file1
    circuit_file_2 = args.file2
    start_time = time.time()
    tracemalloc.start()
    result = qcec.verify(circuit_file_1, circuit_file_2)
    current, peak = tracemalloc.get_traced_memory()
    tracemalloc.stop()
    end_time = time.time()
    
    # print(result.json())
    print("The circuits are ",result.equivalence,".")
    print(f"time: {end_time - start_time:.4f}s")
    print(f"current memory usage: {current / (1024 * 1024):.6f}MB")
    print(f"peak memory usage: {peak / (1024 * 1024):.6f}MB")
    