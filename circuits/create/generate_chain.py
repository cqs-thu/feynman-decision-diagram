import random
hadamard_gate_number = 10

if __name__ == '__main__':
    with open('chain.qasm', 'w') as f:
        print('OPENQASM 2.0;', file=f)
        print('include "qelib1.inc";', file=f)
        print('qreg q[1];', file=f)
        print('h q[0];', file=f)
        for i in range(hadamard_gate_number - 1):
            t_count = random.randint(1,7)
            for j in range(t_count):
                print('t q[0];', file=f)
            print('h q[0];', file=f)
