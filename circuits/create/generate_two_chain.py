layer_num = 600

if __name__ == '__main__':
    with open('two_chain.qasm', 'w') as f:
        print('OPENQASM 2.0;', file=f)
        print('include "qelib1.inc";', file=f)
        print('qreg q[2];', file=f)
        for i in range(layer_num):
            print('h qubits[0];', file=f)
            print('h qubits[1];', file=f)
            print('cz qubits[0], qubits[1];', file=f)