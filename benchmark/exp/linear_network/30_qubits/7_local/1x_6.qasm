OPENQASM 2.0;
include "qelib1.inc";
qreg qubits[30];
h qubits[0];
h qubits[1];
h qubits[2];
h qubits[3];
h qubits[4];
h qubits[5];
h qubits[6];
h qubits[7];
h qubits[8];
h qubits[9];
h qubits[10];
h qubits[11];
h qubits[12];
h qubits[13];
h qubits[14];
h qubits[15];
h qubits[16];
h qubits[17];
h qubits[18];
h qubits[19];
h qubits[20];
h qubits[21];
h qubits[22];
h qubits[23];
h qubits[24];
h qubits[25];
h qubits[26];
h qubits[27];
h qubits[28];
h qubits[29];
cz qubits[0],qubits[1];
cz qubits[0],qubits[3];
cz qubits[0],qubits[4];
cz qubits[0],qubits[5];
cz qubits[0],qubits[6];
cz qubits[0],qubits[7];
cz qubits[0],qubits[8];
cz qubits[0],qubits[9];
cz qubits[0],qubits[13];
cz qubits[0],qubits[14];
cz qubits[0],qubits[15];
cz qubits[0],qubits[19];
cz qubits[0],qubits[23];
cz qubits[0],qubits[24];
cz qubits[0],qubits[26];
z qubits[1];
cz qubits[1],qubits[3];
cz qubits[1],qubits[4];
cz qubits[1],qubits[5];
cz qubits[1],qubits[6];
cz qubits[1],qubits[7];
cz qubits[1],qubits[8];
cz qubits[1],qubits[9];
cz qubits[1],qubits[13];
cz qubits[1],qubits[14];
cz qubits[1],qubits[15];
cz qubits[1],qubits[19];
cz qubits[1],qubits[23];
cz qubits[1],qubits[24];
cz qubits[1],qubits[26];
cz qubits[2],qubits[3];
cz qubits[2],qubits[4];
cz qubits[2],qubits[5];
cz qubits[2],qubits[6];
cz qubits[2],qubits[7];
cz qubits[2],qubits[8];
cz qubits[2],qubits[9];
cz qubits[2],qubits[13];
cz qubits[2],qubits[14];
cz qubits[2],qubits[15];
cz qubits[2],qubits[19];
cz qubits[2],qubits[23];
cz qubits[2],qubits[24];
cz qubits[2],qubits[26];
z qubits[3];
cz qubits[3],qubits[4];
cz qubits[3],qubits[5];
cz qubits[3],qubits[6];
cz qubits[3],qubits[7];
cz qubits[3],qubits[8];
cz qubits[3],qubits[9];
cz qubits[3],qubits[13];
cz qubits[3],qubits[14];
cz qubits[3],qubits[15];
cz qubits[3],qubits[19];
cz qubits[3],qubits[23];
cz qubits[3],qubits[24];
cz qubits[3],qubits[26];
z qubits[4];
cz qubits[4],qubits[5];
cz qubits[4],qubits[6];
cz qubits[4],qubits[7];
cz qubits[4],qubits[8];
cz qubits[4],qubits[9];
cz qubits[4],qubits[13];
cz qubits[4],qubits[14];
cz qubits[4],qubits[15];
cz qubits[4],qubits[19];
cz qubits[4],qubits[23];
cz qubits[4],qubits[24];
cz qubits[4],qubits[26];
z qubits[5];
cz qubits[5],qubits[6];
cz qubits[5],qubits[7];
cz qubits[5],qubits[8];
cz qubits[5],qubits[9];
cz qubits[5],qubits[13];
cz qubits[5],qubits[14];
cz qubits[5],qubits[15];
cz qubits[5],qubits[19];
cz qubits[5],qubits[23];
cz qubits[5],qubits[24];
cz qubits[5],qubits[26];
z qubits[6];
cz qubits[6],qubits[7];
cz qubits[6],qubits[8];
cz qubits[6],qubits[9];
cz qubits[6],qubits[13];
cz qubits[6],qubits[14];
cz qubits[6],qubits[15];
cz qubits[6],qubits[19];
cz qubits[6],qubits[23];
cz qubits[6],qubits[24];
cz qubits[6],qubits[26];
z qubits[7];
cz qubits[7],qubits[8];
cz qubits[7],qubits[9];
cz qubits[7],qubits[13];
cz qubits[7],qubits[14];
cz qubits[7],qubits[15];
cz qubits[7],qubits[19];
cz qubits[7],qubits[23];
cz qubits[7],qubits[24];
cz qubits[7],qubits[26];
z qubits[8];
cz qubits[8],qubits[9];
cz qubits[8],qubits[13];
cz qubits[8],qubits[14];
cz qubits[8],qubits[15];
cz qubits[8],qubits[19];
cz qubits[8],qubits[23];
cz qubits[8],qubits[24];
cz qubits[8],qubits[26];
z qubits[9];
cz qubits[9],qubits[13];
cz qubits[9],qubits[14];
cz qubits[9],qubits[15];
cz qubits[9],qubits[19];
cz qubits[9],qubits[23];
cz qubits[9],qubits[24];
cz qubits[9],qubits[26];
cz qubits[10],qubits[13];
cz qubits[10],qubits[14];
cz qubits[10],qubits[15];
cz qubits[10],qubits[19];
cz qubits[10],qubits[23];
cz qubits[10],qubits[24];
cz qubits[10],qubits[26];
cz qubits[11],qubits[13];
cz qubits[11],qubits[14];
cz qubits[11],qubits[15];
cz qubits[11],qubits[19];
cz qubits[11],qubits[23];
cz qubits[11],qubits[24];
cz qubits[11],qubits[26];
cz qubits[12],qubits[13];
cz qubits[12],qubits[14];
cz qubits[12],qubits[15];
cz qubits[12],qubits[19];
cz qubits[12],qubits[23];
cz qubits[12],qubits[24];
cz qubits[12],qubits[26];
z qubits[13];
cz qubits[13],qubits[14];
cz qubits[13],qubits[15];
cz qubits[13],qubits[19];
cz qubits[13],qubits[23];
cz qubits[13],qubits[24];
cz qubits[13],qubits[26];
z qubits[14];
cz qubits[14],qubits[15];
cz qubits[14],qubits[19];
cz qubits[14],qubits[23];
cz qubits[14],qubits[24];
cz qubits[14],qubits[26];
z qubits[15];
cz qubits[15],qubits[19];
cz qubits[15],qubits[23];
cz qubits[15],qubits[24];
cz qubits[15],qubits[26];
cz qubits[16],qubits[19];
cz qubits[16],qubits[23];
cz qubits[16],qubits[24];
cz qubits[16],qubits[26];
cz qubits[17],qubits[19];
cz qubits[17],qubits[23];
cz qubits[17],qubits[24];
cz qubits[17],qubits[26];
cz qubits[18],qubits[19];
cz qubits[18],qubits[23];
cz qubits[18],qubits[24];
cz qubits[18],qubits[26];
z qubits[19];
cz qubits[19],qubits[23];
cz qubits[19],qubits[24];
cz qubits[19],qubits[26];
cz qubits[20],qubits[23];
cz qubits[20],qubits[24];
cz qubits[20],qubits[26];
cz qubits[21],qubits[23];
cz qubits[21],qubits[24];
cz qubits[21],qubits[26];
cz qubits[22],qubits[23];
cz qubits[22],qubits[24];
cz qubits[22],qubits[26];
z qubits[23];
cz qubits[23],qubits[24];
cz qubits[23],qubits[26];
z qubits[24];
cz qubits[24],qubits[26];
cz qubits[25],qubits[26];
z qubits[26];
ccz qubits[18],qubits[21],qubits[24];
ccz qubits[11],qubits[12],qubits[15];
ccz qubits[3],qubits[7],qubits[8];
ccz qubits[4],qubits[5],qubits[6];
ccz qubits[19],qubits[20],qubits[23];
ccz qubits[18],qubits[20],qubits[22];
ccz qubits[6],qubits[7],qubits[8];
ccz qubits[25],qubits[26],qubits[27];
ccz qubits[19],qubits[21],qubits[23];
ccz qubits[8],qubits[9],qubits[10];
ccz qubits[14],qubits[15],qubits[16];
ccz qubits[20],qubits[21],qubits[23];
ccz qubits[9],qubits[11],qubits[14];
ccz qubits[7],qubits[9],qubits[11];
ccz qubits[8],qubits[10],qubits[13];
ccz qubits[2],qubits[3],qubits[5];
ccz qubits[23],qubits[24],qubits[25];
ccz qubits[16],qubits[20],qubits[22];
ccz qubits[15],qubits[17],qubits[19];
ccz qubits[2],qubits[4],qubits[5];
ccz qubits[15],qubits[16],qubits[17];
ccz qubits[10],qubits[14],qubits[15];
ccz qubits[22],qubits[26],qubits[28];
ccz qubits[19],qubits[20],qubits[22];
ccz qubits[13],qubits[14],qubits[15];
ccz qubits[8],qubits[9],qubits[11];
ccz qubits[17],qubits[18],qubits[19];
ccz qubits[19],qubits[22],qubits[24];
ccz qubits[13],qubits[16],qubits[18];
ccz qubits[8],qubits[11],qubits[12];
h qubits[0];
h qubits[1];
h qubits[2];
h qubits[3];
h qubits[4];
h qubits[5];
h qubits[6];
h qubits[7];
h qubits[8];
h qubits[9];
h qubits[10];
h qubits[11];
h qubits[12];
h qubits[13];
h qubits[14];
h qubits[15];
h qubits[16];
h qubits[17];
h qubits[18];
h qubits[19];
h qubits[20];
h qubits[21];
h qubits[22];
h qubits[23];
h qubits[24];
h qubits[25];
h qubits[26];
h qubits[27];
h qubits[28];
h qubits[29];
