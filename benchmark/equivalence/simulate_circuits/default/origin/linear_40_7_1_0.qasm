OPENQASM 2.0;
include "qelib1.inc";
qreg qubits[40];
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
h qubits[30];
h qubits[31];
h qubits[32];
h qubits[33];
h qubits[34];
h qubits[35];
h qubits[36];
h qubits[37];
h qubits[38];
h qubits[39];
z qubits[0];
cz qubits[0],qubits[4];
cz qubits[0],qubits[5];
cz qubits[0],qubits[6];
cz qubits[0],qubits[8];
cz qubits[0],qubits[9];
cz qubits[0],qubits[10];
cz qubits[0],qubits[15];
cz qubits[0],qubits[16];
cz qubits[0],qubits[17];
cz qubits[0],qubits[18];
cz qubits[0],qubits[23];
cz qubits[0],qubits[24];
cz qubits[0],qubits[25];
cz qubits[0],qubits[29];
cz qubits[0],qubits[30];
cz qubits[0],qubits[32];
cz qubits[0],qubits[33];
cz qubits[0],qubits[35];
cz qubits[0],qubits[37];
cz qubits[1],qubits[4];
cz qubits[1],qubits[5];
cz qubits[1],qubits[6];
cz qubits[1],qubits[8];
cz qubits[1],qubits[9];
cz qubits[1],qubits[10];
cz qubits[1],qubits[15];
cz qubits[1],qubits[16];
cz qubits[1],qubits[17];
cz qubits[1],qubits[18];
cz qubits[1],qubits[23];
cz qubits[1],qubits[24];
cz qubits[1],qubits[25];
cz qubits[1],qubits[29];
cz qubits[1],qubits[30];
cz qubits[1],qubits[32];
cz qubits[1],qubits[33];
cz qubits[1],qubits[35];
cz qubits[1],qubits[37];
cz qubits[2],qubits[4];
cz qubits[2],qubits[5];
cz qubits[2],qubits[6];
cz qubits[2],qubits[8];
cz qubits[2],qubits[9];
cz qubits[2],qubits[10];
cz qubits[2],qubits[15];
cz qubits[2],qubits[16];
cz qubits[2],qubits[17];
cz qubits[2],qubits[18];
cz qubits[2],qubits[23];
cz qubits[2],qubits[24];
cz qubits[2],qubits[25];
cz qubits[2],qubits[29];
cz qubits[2],qubits[30];
cz qubits[2],qubits[32];
cz qubits[2],qubits[33];
cz qubits[2],qubits[35];
cz qubits[2],qubits[37];
cz qubits[3],qubits[4];
cz qubits[3],qubits[5];
cz qubits[3],qubits[6];
cz qubits[3],qubits[8];
cz qubits[3],qubits[9];
cz qubits[3],qubits[10];
cz qubits[3],qubits[15];
cz qubits[3],qubits[16];
cz qubits[3],qubits[17];
cz qubits[3],qubits[18];
cz qubits[3],qubits[23];
cz qubits[3],qubits[24];
cz qubits[3],qubits[25];
cz qubits[3],qubits[29];
cz qubits[3],qubits[30];
cz qubits[3],qubits[32];
cz qubits[3],qubits[33];
cz qubits[3],qubits[35];
cz qubits[3],qubits[37];
z qubits[4];
cz qubits[4],qubits[5];
cz qubits[4],qubits[6];
cz qubits[4],qubits[8];
cz qubits[4],qubits[9];
cz qubits[4],qubits[10];
cz qubits[4],qubits[15];
cz qubits[4],qubits[16];
cz qubits[4],qubits[17];
cz qubits[4],qubits[18];
cz qubits[4],qubits[23];
cz qubits[4],qubits[24];
cz qubits[4],qubits[25];
cz qubits[4],qubits[29];
cz qubits[4],qubits[30];
cz qubits[4],qubits[32];
cz qubits[4],qubits[33];
cz qubits[4],qubits[35];
cz qubits[4],qubits[37];
z qubits[5];
cz qubits[5],qubits[6];
cz qubits[5],qubits[8];
cz qubits[5],qubits[9];
cz qubits[5],qubits[10];
cz qubits[5],qubits[15];
cz qubits[5],qubits[16];
cz qubits[5],qubits[17];
cz qubits[5],qubits[18];
cz qubits[5],qubits[23];
cz qubits[5],qubits[24];
cz qubits[5],qubits[25];
cz qubits[5],qubits[29];
cz qubits[5],qubits[30];
cz qubits[5],qubits[32];
cz qubits[5],qubits[33];
cz qubits[5],qubits[35];
cz qubits[5],qubits[37];
z qubits[6];
cz qubits[6],qubits[8];
cz qubits[6],qubits[9];
cz qubits[6],qubits[10];
cz qubits[6],qubits[15];
cz qubits[6],qubits[16];
cz qubits[6],qubits[17];
cz qubits[6],qubits[18];
cz qubits[6],qubits[23];
cz qubits[6],qubits[24];
cz qubits[6],qubits[25];
cz qubits[6],qubits[29];
cz qubits[6],qubits[30];
cz qubits[6],qubits[32];
cz qubits[6],qubits[33];
cz qubits[6],qubits[35];
cz qubits[6],qubits[37];
cz qubits[7],qubits[8];
cz qubits[7],qubits[9];
cz qubits[7],qubits[10];
cz qubits[7],qubits[15];
cz qubits[7],qubits[16];
cz qubits[7],qubits[17];
cz qubits[7],qubits[18];
cz qubits[7],qubits[23];
cz qubits[7],qubits[24];
cz qubits[7],qubits[25];
cz qubits[7],qubits[29];
cz qubits[7],qubits[30];
cz qubits[7],qubits[32];
cz qubits[7],qubits[33];
cz qubits[7],qubits[35];
cz qubits[7],qubits[37];
z qubits[8];
cz qubits[8],qubits[9];
cz qubits[8],qubits[10];
cz qubits[8],qubits[15];
cz qubits[8],qubits[16];
cz qubits[8],qubits[17];
cz qubits[8],qubits[18];
cz qubits[8],qubits[23];
cz qubits[8],qubits[24];
cz qubits[8],qubits[25];
cz qubits[8],qubits[29];
cz qubits[8],qubits[30];
cz qubits[8],qubits[32];
cz qubits[8],qubits[33];
cz qubits[8],qubits[35];
cz qubits[8],qubits[37];
z qubits[9];
cz qubits[9],qubits[10];
cz qubits[9],qubits[15];
cz qubits[9],qubits[16];
cz qubits[9],qubits[17];
cz qubits[9],qubits[18];
cz qubits[9],qubits[23];
cz qubits[9],qubits[24];
cz qubits[9],qubits[25];
cz qubits[9],qubits[29];
cz qubits[9],qubits[30];
cz qubits[9],qubits[32];
cz qubits[9],qubits[33];
cz qubits[9],qubits[35];
cz qubits[9],qubits[37];
z qubits[10];
cz qubits[10],qubits[15];
cz qubits[10],qubits[16];
cz qubits[10],qubits[17];
cz qubits[10],qubits[18];
cz qubits[10],qubits[23];
cz qubits[10],qubits[24];
cz qubits[10],qubits[25];
cz qubits[10],qubits[29];
cz qubits[10],qubits[30];
cz qubits[10],qubits[32];
cz qubits[10],qubits[33];
cz qubits[10],qubits[35];
cz qubits[10],qubits[37];
cz qubits[11],qubits[15];
cz qubits[11],qubits[16];
cz qubits[11],qubits[17];
cz qubits[11],qubits[18];
cz qubits[11],qubits[23];
cz qubits[11],qubits[24];
cz qubits[11],qubits[25];
cz qubits[11],qubits[29];
cz qubits[11],qubits[30];
cz qubits[11],qubits[32];
cz qubits[11],qubits[33];
cz qubits[11],qubits[35];
cz qubits[11],qubits[37];
cz qubits[12],qubits[15];
cz qubits[12],qubits[16];
cz qubits[12],qubits[17];
cz qubits[12],qubits[18];
cz qubits[12],qubits[23];
cz qubits[12],qubits[24];
cz qubits[12],qubits[25];
cz qubits[12],qubits[29];
cz qubits[12],qubits[30];
cz qubits[12],qubits[32];
cz qubits[12],qubits[33];
cz qubits[12],qubits[35];
cz qubits[12],qubits[37];
cz qubits[13],qubits[15];
cz qubits[13],qubits[16];
cz qubits[13],qubits[17];
cz qubits[13],qubits[18];
cz qubits[13],qubits[23];
cz qubits[13],qubits[24];
cz qubits[13],qubits[25];
cz qubits[13],qubits[29];
cz qubits[13],qubits[30];
cz qubits[13],qubits[32];
cz qubits[13],qubits[33];
cz qubits[13],qubits[35];
cz qubits[13],qubits[37];
cz qubits[14],qubits[15];
cz qubits[14],qubits[16];
cz qubits[14],qubits[17];
cz qubits[14],qubits[18];
cz qubits[14],qubits[23];
cz qubits[14],qubits[24];
cz qubits[14],qubits[25];
cz qubits[14],qubits[29];
cz qubits[14],qubits[30];
cz qubits[14],qubits[32];
cz qubits[14],qubits[33];
cz qubits[14],qubits[35];
cz qubits[14],qubits[37];
z qubits[15];
cz qubits[15],qubits[16];
cz qubits[15],qubits[17];
cz qubits[15],qubits[18];
cz qubits[15],qubits[23];
cz qubits[15],qubits[24];
cz qubits[15],qubits[25];
cz qubits[15],qubits[29];
cz qubits[15],qubits[30];
cz qubits[15],qubits[32];
cz qubits[15],qubits[33];
cz qubits[15],qubits[35];
cz qubits[15],qubits[37];
z qubits[16];
cz qubits[16],qubits[17];
cz qubits[16],qubits[18];
cz qubits[16],qubits[23];
cz qubits[16],qubits[24];
cz qubits[16],qubits[25];
cz qubits[16],qubits[29];
cz qubits[16],qubits[30];
cz qubits[16],qubits[32];
cz qubits[16],qubits[33];
cz qubits[16],qubits[35];
cz qubits[16],qubits[37];
z qubits[17];
cz qubits[17],qubits[18];
cz qubits[17],qubits[23];
cz qubits[17],qubits[24];
cz qubits[17],qubits[25];
cz qubits[17],qubits[29];
cz qubits[17],qubits[30];
cz qubits[17],qubits[32];
cz qubits[17],qubits[33];
cz qubits[17],qubits[35];
cz qubits[17],qubits[37];
z qubits[18];
cz qubits[18],qubits[23];
cz qubits[18],qubits[24];
cz qubits[18],qubits[25];
cz qubits[18],qubits[29];
cz qubits[18],qubits[30];
cz qubits[18],qubits[32];
cz qubits[18],qubits[33];
cz qubits[18],qubits[35];
cz qubits[18],qubits[37];
cz qubits[19],qubits[23];
cz qubits[19],qubits[24];
cz qubits[19],qubits[25];
cz qubits[19],qubits[29];
cz qubits[19],qubits[30];
cz qubits[19],qubits[32];
cz qubits[19],qubits[33];
cz qubits[19],qubits[35];
cz qubits[19],qubits[37];
cz qubits[20],qubits[23];
cz qubits[20],qubits[24];
cz qubits[20],qubits[25];
cz qubits[20],qubits[29];
cz qubits[20],qubits[30];
cz qubits[20],qubits[32];
cz qubits[20],qubits[33];
cz qubits[20],qubits[35];
cz qubits[20],qubits[37];
cz qubits[21],qubits[23];
cz qubits[21],qubits[24];
cz qubits[21],qubits[25];
cz qubits[21],qubits[29];
cz qubits[21],qubits[30];
cz qubits[21],qubits[32];
cz qubits[21],qubits[33];
cz qubits[21],qubits[35];
cz qubits[21],qubits[37];
cz qubits[22],qubits[23];
cz qubits[22],qubits[24];
cz qubits[22],qubits[25];
cz qubits[22],qubits[29];
cz qubits[22],qubits[30];
cz qubits[22],qubits[32];
cz qubits[22],qubits[33];
cz qubits[22],qubits[35];
cz qubits[22],qubits[37];
z qubits[23];
cz qubits[23],qubits[24];
cz qubits[23],qubits[25];
cz qubits[23],qubits[29];
cz qubits[23],qubits[30];
cz qubits[23],qubits[32];
cz qubits[23],qubits[33];
cz qubits[23],qubits[35];
cz qubits[23],qubits[37];
z qubits[24];
cz qubits[24],qubits[25];
cz qubits[24],qubits[29];
cz qubits[24],qubits[30];
cz qubits[24],qubits[32];
cz qubits[24],qubits[33];
cz qubits[24],qubits[35];
cz qubits[24],qubits[37];
z qubits[25];
cz qubits[25],qubits[29];
cz qubits[25],qubits[30];
cz qubits[25],qubits[32];
cz qubits[25],qubits[33];
cz qubits[25],qubits[35];
cz qubits[25],qubits[37];
cz qubits[26],qubits[29];
cz qubits[26],qubits[30];
cz qubits[26],qubits[32];
cz qubits[26],qubits[33];
cz qubits[26],qubits[35];
cz qubits[26],qubits[37];
cz qubits[27],qubits[29];
cz qubits[27],qubits[30];
cz qubits[27],qubits[32];
cz qubits[27],qubits[33];
cz qubits[27],qubits[35];
cz qubits[27],qubits[37];
cz qubits[28],qubits[29];
cz qubits[28],qubits[30];
cz qubits[28],qubits[32];
cz qubits[28],qubits[33];
cz qubits[28],qubits[35];
cz qubits[28],qubits[37];
z qubits[29];
cz qubits[29],qubits[30];
cz qubits[29],qubits[32];
cz qubits[29],qubits[33];
cz qubits[29],qubits[35];
cz qubits[29],qubits[37];
z qubits[30];
cz qubits[30],qubits[32];
cz qubits[30],qubits[33];
cz qubits[30],qubits[35];
cz qubits[30],qubits[37];
cz qubits[31],qubits[32];
cz qubits[31],qubits[33];
cz qubits[31],qubits[35];
cz qubits[31],qubits[37];
z qubits[32];
cz qubits[32],qubits[33];
cz qubits[32],qubits[35];
cz qubits[32],qubits[37];
z qubits[33];
cz qubits[33],qubits[35];
cz qubits[33],qubits[37];
cz qubits[34],qubits[35];
cz qubits[34],qubits[37];
z qubits[35];
cz qubits[35],qubits[37];
cz qubits[36],qubits[37];
z qubits[37];
h qubits[27];
ccx qubits[24], qubits[25], qubits[27];
h qubits[27];
h qubits[6];
ccx qubits[0], qubits[1], qubits[6];
h qubits[6];
h qubits[4];
ccx qubits[0], qubits[3], qubits[4];
h qubits[4];
h qubits[17];
ccx qubits[12], qubits[16], qubits[17];
h qubits[17];
h qubits[30];
ccx qubits[27], qubits[29], qubits[30];
h qubits[30];
h qubits[27];
ccx qubits[24], qubits[25], qubits[27];
h qubits[27];
h qubits[4];
ccx qubits[1], qubits[3], qubits[4];
h qubits[4];
h qubits[30];
ccx qubits[27], qubits[29], qubits[30];
h qubits[30];
h qubits[10];
ccx qubits[7], qubits[8], qubits[10];
h qubits[10];
h qubits[26];
ccx qubits[22], qubits[25], qubits[26];
h qubits[26];
h qubits[5];
ccx qubits[2], qubits[3], qubits[5];
h qubits[5];
h qubits[5];
ccx qubits[0], qubits[4], qubits[5];
h qubits[5];
h qubits[15];
ccx qubits[9], qubits[14], qubits[15];
h qubits[15];
h qubits[12];
ccx qubits[8], qubits[11], qubits[12];
h qubits[12];
h qubits[19];
ccx qubits[14], qubits[16], qubits[19];
h qubits[19];
h qubits[26];
ccx qubits[22], qubits[25], qubits[26];
h qubits[26];
h qubits[28];
ccx qubits[23], qubits[25], qubits[28];
h qubits[28];
h qubits[20];
ccx qubits[15], qubits[18], qubits[20];
h qubits[20];
h qubits[28];
ccx qubits[22], qubits[26], qubits[28];
h qubits[28];
h qubits[17];
ccx qubits[12], qubits[14], qubits[17];
h qubits[17];
h qubits[4];
ccx qubits[2], qubits[3], qubits[4];
h qubits[4];
h qubits[31];
ccx qubits[28], qubits[30], qubits[31];
h qubits[31];
h qubits[27];
ccx qubits[21], qubits[23], qubits[27];
h qubits[27];
h qubits[38];
ccx qubits[33], qubits[34], qubits[38];
h qubits[38];
h qubits[14];
ccx qubits[11], qubits[12], qubits[14];
h qubits[14];
h qubits[34];
ccx qubits[30], qubits[32], qubits[34];
h qubits[34];
h qubits[6];
ccx qubits[1], qubits[3], qubits[6];
h qubits[6];
h qubits[14];
ccx qubits[10], qubits[13], qubits[14];
h qubits[14];
h qubits[17];
ccx qubits[11], qubits[13], qubits[17];
h qubits[17];
h qubits[20];
ccx qubits[16], qubits[18], qubits[20];
h qubits[20];
h qubits[8];
ccx qubits[5], qubits[7], qubits[8];
h qubits[8];
h qubits[39];
ccx qubits[34], qubits[38], qubits[39];
h qubits[39];
h qubits[27];
ccx qubits[25], qubits[26], qubits[27];
h qubits[27];
h qubits[22];
ccx qubits[16], qubits[20], qubits[22];
h qubits[22];
h qubits[21];
ccx qubits[19], qubits[20], qubits[21];
h qubits[21];
h qubits[31];
ccx qubits[29], qubits[30], qubits[31];
h qubits[31];
h qubits[14];
ccx qubits[9], qubits[10], qubits[14];
h qubits[14];
h qubits[2];
ccx qubits[0], qubits[1], qubits[2];
h qubits[2];
h qubits[39];
ccx qubits[37], qubits[38], qubits[39];
h qubits[39];
h qubits[35];
ccx qubits[32], qubits[34], qubits[35];
h qubits[35];
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
h qubits[30];
h qubits[31];
h qubits[32];
h qubits[33];
h qubits[34];
h qubits[35];
h qubits[36];
h qubits[37];
h qubits[38];
h qubits[39];