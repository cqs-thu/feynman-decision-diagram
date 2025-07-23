qubit_num = 50
layer_num = 6

if __name__ == '__main__':
    with open('brick_layer.qasm', 'w') as f:
        print('OPENQASM 2.0;', file=f)
        print('include "qelib1.inc";', file=f)
        print('qreg q[{}];'.format(qubit_num), file=f)
        for i in range(qubit_num):
            print('h qubits[{}];'.format(i), file=f)
        for i in range(layer_num):
            begin_index = i % 2
            for j in range(begin_index, qubit_num-1, 2):
                print('cz qubits[{}],qubits[{}];'.format(j, j+1), file=f)
            for j in range(qubit_num):
                print('h qubits[{}];'.format(j), file=f)