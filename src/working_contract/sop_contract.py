import cotengra as ctg 
import argparse
import sys
from multiprocessing import Process, Pipe

def read_tensor_data_from_file(filename):
    term_data = []
    io_variable = []
    with open(filename, 'r') as file:
        for line in file:
            line = line.strip()
            if line.startswith("IO:"):
                io_variable = list(map(int, line[3:].strip().split()))
            else:
                row = list(map(int, line.split()))
                term_data.append(row)
    term_variable = set(variable for term in term_data for variable in term)
    io_variable = [v for v in io_variable if v in term_variable]
    return term_data, io_variable, term_variable

class TreeNode:
    def __init__(self, init_gates: list = [], init_variables: list = []):
        self.gates = init_gates
        self.variables = init_variables
        
    def __repr__(self):
        return f"TreeNode({self.gates})"
    
    def merge(self, other):
        new_gates = self.gates + other.gates
        new_variables = list(set(self.variables) | set(other.variables)) 
        return TreeNode(new_gates, new_variables)
    
class TermTreeNode:
    def __init__(self, init_term_indexs: list = []):
        self.term_indexs = init_term_indexs
        
    def __repr__(self):
        return f"TermTreeNode({self.term_indexs})"
    
    def merge(self, other):
        new_term_indexs = self.term_indexs + other.term_indexs
        return TermTreeNode(new_term_indexs)

def get_hyperoptimizer() -> ctg.HyperOptimizer:
    # opt = ctg.ReusableHyperOptimizer(
    opt = ctg.HyperOptimizer(
        # directory=True,
        # methods=None,
        # minimize="flops",
        max_repeats=128,
        # max_time=30.0,
        reconf_opts={},
        parallel='auto',
        # simulated_annealing_opts=None,
        # slicing_opts=None,
        # slicing_reconf_opts=None,
        # reconf_opts=None,
        # optlib=None,
        # space=None,
        score_compression=0.75,
        # on_trial_error="warn",
        # max_training_steps=None,
        progbar=True,   
    )
    # opt = ctg.pathfinders.path_basic.OptimalOptimizer()
    return opt

def async_search_contraction_tree(input, output, size_dict, channel, time = 30.0): 
    opt = get_hyperoptimizer()
    opt.max_time = time
    tree = opt.search(input, output, size_dict)
    channel.send(tree)
    channel.close()

def search_contraction_tree(input, output, size_dict):
    opt = get_hyperoptimizer()
    tree = opt.search(input, output, size_dict)
    return tree

def get_ordered_variables(path, term_data, term_count, cotengra_output_list, term_variable):
    sum_variables = set()
    ordered_variables = []
    working_nodes = [TreeNode([i], term_data[i]) for i in range(term_count)]
    for merge_operation in path:
        a = working_nodes[merge_operation[0]]
        b = working_nodes[merge_operation[1]]
        new_node = a.merge(b)
        operation_sum_variables = set(a.variables) & set(b.variables)
        for v in operation_sum_variables:
            if v not in sum_variables:
                ordered_variables.append(v)
                sum_variables.add(v)
        # print(f"merge {a} and {b}")
        working_nodes.remove(a)
        working_nodes.remove(b)
        working_nodes.append(new_node)
    for v in cotengra_output_list:
        if v not in sum_variables:
            ordered_variables.append(v)
            sum_variables.add(v)
    for v in term_variable:
        if v not in sum_variables:
            ordered_variables.append(v)
            sum_variables.add(v)
    return ordered_variables

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-i','--input', type=str, required=True, help='input file path')
    parser.add_argument('-o','--output', type=str, required=False, help='output file path')
    parser.add_argument('-t', '--type', type=str, required=False, help='order type', default='variable')
    parser.add_argument('-l', '--limit', type=float, required=False, help='time limit for search', default=-1.0)
    args = parser.parse_args()
    input_file_path = args.input 
    time_limit = args.limit
    # print("time limit:", time_limit)
    
    term_data, io_variable, term_variable = read_tensor_data_from_file(input_file_path)
    # print(term_data)
    # print(io_variable)
    cotengra_input_list = term_data
    cotengra_output_list = io_variable
    size_dict = {}
    term_count = len(cotengra_input_list)
    order_type = args.type
    if term_count <= 1: # 退化情况不搜索
        if order_type == 'variable':
            with open(args.output, 'w') as file:
                for v in term_variable:
                    file.write(f"{v}\n")
        elif order_type == 'term':
            with open(args.output, 'w') as file:
                file.write("term:\n")
                for i in range(term_count):
                    file.write(f"{i}:")
                    for var in term_data[i]:
                        file.write(f" {var}")
                    file.write("\n")
                file.write("variable:\n")
                for v in term_variable:
                    file.write(f"{v}\n")
        
    variable_set = set()
    for term in cotengra_input_list:
        for variable in term:
            variable_set.add(variable)
    for variable in cotengra_output_list:
        variable_set.add(variable)
    
    for variable in variable_set:
        size_dict[variable] = 2
    
    tree: any = None
    if time_limit > 0.0:
        recv_channel, send_channel = Pipe()
        process = Process(target=async_search_contraction_tree, args=(cotengra_input_list, cotengra_output_list, size_dict, send_channel, time_limit))
        process.daemon = False
        process.start()
        process.join(timeout=time_limit)
        if process.is_alive():
            process.terminate()
            if args.output is not None:
                with open(args.output, 'w') as file:
                    file.write("Timeout")
            sys.exit()
        else:
            tree = recv_channel.recv()
    else:
        tree = search_contraction_tree(cotengra_input_list, cotengra_output_list, size_dict)

    path = tree.get_path()
    width = tree.contraction_width()
    cost = tree.contraction_cost()
    # print("tree width:", width, " tree cost:", cost)
    # print(path)

    ordered_variables = get_ordered_variables(
        path=path,
        term_data=term_data,
        term_count=term_count,
        cotengra_output_list=cotengra_output_list,
        term_variable=term_variable)
    
    if order_type == 'variable':  
        if args.output is not None:
            with open(args.output, 'w') as file:
                for v in ordered_variables:
                    file.write(f"{v}\n")
    elif order_type == 'term':
        contracted_terms = set()
        ordered_term_indexs = []
        working_nodes = [TermTreeNode([i]) for i in range(term_count)]
        for merge_operation in path:
            a = working_nodes[merge_operation[0]]
            b = working_nodes[merge_operation[1]]
            new_node = a.merge(b)
            if len(a.term_indexs) == 1 and (a not in contracted_terms):
                ordered_term_indexs.append(a.term_indexs[0])
                contracted_terms.add(a)
            if len(b.term_indexs) == 1 and (b not in contracted_terms):
                ordered_term_indexs.append(b.term_indexs[0])
                contracted_terms.add(b)
            working_nodes.remove(a)
            working_nodes.remove(b)
            working_nodes.append(new_node)
        if args.output is not None:
            with open(args.output, 'w') as file:
                file.write("term:\n")
                for i in ordered_term_indexs:
                    file.write(f"{i}:")
                    for var in term_data[i]:
                        file.write(f" {var}")
                    file.write("\n")
                file.write("variable:\n")
                for v in ordered_variables:
                    file.write(f"{v}\n")
    elif order_type == 'path':
        if args.output is not None:
            with open(args.output, 'w') as file:
                for operation in path:
                    file.write(f"{operation[0]} {operation[1]}\n")
        
    else:
        raise ValueError(f"Unsupported order type: {order_type}")