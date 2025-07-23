import cotengra as ctg 
import re
import argparse

def generate_strings(n):
    result = []
    for i in range(n):
        s = ""
        num = i
        while num >= 0:
            s = chr(num % 26 + ord('a')) + s
            num = num // 26 - 1
        result.append(s)
    return result

def read_tensor_data_from_file(filename):
    max_variable_number = 0
    max_gate_number = 0
    pattern = r'(-?\d+)\s*->\s*(-?\d+)\s*\((\d+)\)'
    data_list = []
    with open(filename, 'r') as file:
        for line in file:
            match = re.match(pattern, line.strip())
            if match:
                gate_id = int(match.group(1))
                neighbor_gate_id = int(match.group(2))
                max_gate_number = max(max_gate_number, gate_id, neighbor_gate_id)
                variable = int(match.group(3))
                max_variable_number = max(max_variable_number, variable)
                data_list.append((gate_id, neighbor_gate_id, variable))
    return data_list, max_variable_number, max_gate_number

class TreeNode:
    def __init__(self, init_gates: list = []):
        self.gates = init_gates
        
    def __repr__(self):
        return f"TreeNode({self.gates})"
    
    def merge(self, other):
        new_gates = self.gates + other.gates
        return TreeNode(new_gates)

class GateVertex:
    def __init__(self, gate_id: int):
        self.gate_id = gate_id
        self.neighbors = {}
        
    def add_neighbor(self, neighbor_id: int, variables: list = None):
        if self.neighbors.get(neighbor_id) is None:
            self.neighbors[neighbor_id] = variables
        else:
            self.neighbors[neighbor_id] += variables
    
    def __repr__(self):
        return f"GateVertex({self.gate_id}, {self.neighbors})"

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-i','--input', type=str, required=True, help='input file path')
    parser.add_argument('-o','--output', type=str, required=True, help='output file path')
    args = parser.parse_args()
    input_file_path = args.input 
    data, max_variable_number, max_gate_number = read_tensor_data_from_file(input_file_path)
    io_index = 0
    variable_list = generate_strings(max_variable_number + 1)
    cotengra_input_list = [[] for _ in range(max_gate_number + 1)]
    cotengra_output_list = []
    size_dict = {}
    
    graph = [GateVertex(i) for i in range(max_gate_number + 1)]
    
    def get_io_variable(index):
        return "io" + str(index)
    
    for gate_id, neighbor_gate_id, variable in data:    
        if neighbor_gate_id == -1:
            io_variable = get_io_variable(io_index)
            cotengra_output_list.append(io_variable)
            io_index += 1
            cotengra_input_list[gate_id].append(io_variable)
        else:
            cotengra_input_list[gate_id].append(variable_list[variable])
            cotengra_input_list[neighbor_gate_id].append(variable_list[variable])
            graph[gate_id].add_neighbor(neighbor_gate_id, [variable])
            graph[neighbor_gate_id].add_neighbor(gate_id, [variable])
    for i in range(io_index):
        size_dict[get_io_variable(i)] = 2
    for v in variable_list:
        size_dict[v] = 2
    # print(cotengra_input_list)
    # print(cotengra_output_list)
    # print(size_dict)
    opt = ctg.HyperOptimizer()
    tree = opt.search(cotengra_input_list, cotengra_output_list, size_dict)
    path = tree.get_path()
    # print(path)
    
    sum_variables = set()
    ordered_variables = []
    working_nodes = [TreeNode([i]) for i in range(max_gate_number + 1)]
    for merge_operation in path:
        a = working_nodes[merge_operation[0]]
        b = working_nodes[merge_operation[1]]
        new_node = a.merge(b)
        for u in a.gates:
            for v in b.gates:
                if v not in graph[u].neighbors:
                    continue
                for w in graph[u].neighbors[v]:
                    if w not in sum_variables:
                        sum_variables.add(w)
                        ordered_variables.append(w)
        # print(f"merge {a} and {b}")
        working_nodes.remove(a)
        working_nodes.remove(b)
        working_nodes.append(new_node)
    
    with open(args.output, 'w') as file:
        for v in ordered_variables:
            file.write(f"{v}\n")