OPENQASM 2.0;
include "qelib1.inc";
qreg q[16];
h q[0];
h q[1];
cz q[0],q[1];
rz(-pi/2) q[0];
h q[0];
rz(pi/2) q[0];
h q[0];
rz(pi/4) q[0];
rz(-pi/2) q[1];
h q[1];
rz(pi/2) q[1];
h q[1];
h q[2];
rz(pi/4) q[2];
cz q[1],q[2];
h q[1];
rz(pi/2) q[1];
h q[1];
rz(3*pi/4) q[1];
h q[3];
rz(pi/4) q[3];
h q[4];
rz(-pi/4) q[4];
h q[5];
rz(pi/4) q[5];
h q[6];
h q[7];
cz q[6],q[7];
h q[7];
rz(pi/2) q[7];
h q[7];
rz(pi/4) q[7];
h q[8];
h q[9];
cz q[8],q[9];
cz q[4],q[8];
h q[4];
rz(pi/2) q[4];
h q[4];
rz(pi/2) q[4];
cz q[0],q[4];
h q[0];
rz(pi/2) q[0];
h q[0];
rz(pi/2) q[0];
cz q[4],q[5];
h q[8];
rz(pi/2) q[8];
h q[8];
rz(pi/4) q[8];
h q[9];
rz(pi/2) q[9];
h q[9];
h q[10];
rz(-pi/4) q[10];
cz q[6],q[10];
h q[6];
rz(pi/2) q[6];
h q[6];
cz q[2],q[6];
cz q[2],q[3];
h q[6];
rz(pi/2) q[6];
h q[6];
cz q[9],q[10];
h q[10];
rz(pi/2) q[10];
h q[10];
rz(pi/2) q[10];
h q[11];
rz(pi/4) q[11];
h q[12];
rz(pi/4) q[12];
h q[13];
rz(pi/4) q[13];
cz q[9],q[13];
rz(-pi/2) q[9];
h q[9];
rz(pi/2) q[9];
h q[9];
rz(pi/2) q[9];
cz q[12],q[13];
h q[14];
h q[15];
cz q[14],q[15];
rz(-pi/2) q[14];
h q[14];
rz(pi/2) q[14];
h q[14];
rz(3*pi/4) q[14];
h q[15];
rz(pi/2) q[15];
h q[15];
rz(-pi/4) q[15];
cz q[11],q[15];
cz q[10],q[11];
h q[15];
rz(pi/2) q[15];
h q[15];
rz(pi/2) q[15];