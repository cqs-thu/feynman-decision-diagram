OPENQASM 2.0;
include "qelib1.inc";
qreg q[4];
cz q[2], q[3];
h q[1];
ccz q[1], q[0], q[3];
h q[3];
cz q[3], q[2];
h q[0];