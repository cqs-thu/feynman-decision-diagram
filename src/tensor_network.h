#pragma once

#include <string>
#include <vector>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <cassert>
#include <map>
#include <unordered_set>

#include "global.h"

// #define FILE_TIME_TEST 

class tensor_network {

    friend class sum_power;
    friend class term;
    friend tensor_network from_qasm(std::string file, std::string config);

    template <typename adapter_t>
    friend class FeynmanCoreTest;

private:
    class edge {
    public:
        int neighbor;
        variable var;
        edge(int _nei, variable _var): neighbor(_nei), var(_var) {}
    };

    int num_qubit, num_gate, num_var, r;
    std::vector <std::vector<edge> > gate_edges;
    std::vector <variable> variable_start, variable_end;
    std::vector <int> last_gate;
    std::vector <int> exponent; // the seq. of coeff of all gates, share the same index with gate_edges

    std::vector <std::vector<variable>> qubit_variable;
    int factor_count;

    void add_edge(int target_gate_id, int qubit, variable new_var) {
        int qubit_last_gate_id = last_gate[qubit];
        variable w = variable_end[qubit];
        if (target_gate_id != INVALID_GATE) {
            if (target_gate_id >= num_gate) {
                // given an invalid gate
                // ERROR("Invalid gate id: %d\n", target_gate_id);
                return;
            }
            gate_edges[target_gate_id].push_back((edge){qubit_last_gate_id, w});
        }
        if (qubit_last_gate_id != INVALID_GATE) {
            gate_edges[qubit_last_gate_id].push_back((edge){target_gate_id, w});
        }
        if (new_var != INVALID_VARIABLE) {
            variable_end[qubit] = new_var;
        }
        last_gate[qubit] = target_gate_id;
    }

public:
    variable get_new_var() {
        return num_var++;
    }
    const std::vector<variable>& get_variable_start() const {
        return variable_start;
    }
    int get_new_gate() {
        gate_edges.push_back({});
        return num_gate++;
    }
    int get_num_qubit() const {
        return num_qubit;
    }
    int get_num_gate() const {
        return num_gate;
    }
    int get_num_var() const {
        return num_var;
    }
    int get_r() const {
        return r;
    }
    int get_factor_count() const {
        return factor_count;
    }

    std::vector<std::vector<variable>> get_qubit_variable()const
    {
        return qubit_variable;
    }

    tensor_network(int _q, int _r) {
        num_qubit = _q;
        num_gate = 0;
        num_var =_q;
        r = _r;
        factor_count = 0;
        variable_start.resize(_q);
        variable_end.resize(_q);
        last_gate.resize(_q);
        qubit_variable.resize(_q);
        for (int i = 0; i < _q; i ++) {
            variable_start[i] = variable_end[i] = i;
            last_gate[i] = -1;
        }        
    }

    std::vector<std::vector<variable>> get_variable_by_qubit_order() const {
        std::unordered_set<variable> exists_vars;
        std::vector<std::vector<variable>> ret(num_qubit);
        for (int i = 0; i < num_qubit; i ++) {
            ret[i].push_back(variable_start[i]);
            for (size_t j = 0; j < qubit_variable[i].size(); j ++) {
                if (qubit_variable[i][j] == INVALID_VARIABLE) {
                    continue;
                }
                if (exists_vars.find(qubit_variable[i][j]) == exists_vars.end()) {
                    ret[i].push_back(qubit_variable[i][j]);
                    exists_vars.insert(qubit_variable[i][j]);
                }
            }
        }
        return ret;
    }

    std::vector<variable> get_variable_by_gate_order() const {
        std::unordered_set<variable> exists_vars;
        std::vector<variable> ret;
        for (int i = 0; i < num_gate; i ++) {
            for (size_t j = 0; j < gate_edges[i].size(); j ++) {
                if (gate_edges[i][j].var == INVALID_VARIABLE) {
                    continue;
                }
                if (exists_vars.count(gate_edges[i][j].var) == 0) {
                    ret.push_back(gate_edges[i][j].var);
                    exists_vars.insert(gate_edges[i][j].var);
                }
            }
        }
        return ret;
    }

    void print() {
        printf("num_qubit: %d\n", num_qubit);
        printf("num_gate: %d\n", num_gate);
        printf("num_var: %d\n", num_var);
        printf("factor count: %d\n", factor_count);
        printf("r: %d\n\n", r);
        for (int i = 0; i < num_gate; i ++) {
            for (auto j : gate_edges[i]) {
                printf("%d -> %d (%d)\n", i, j.neighbor, j.var);
            }
        }
        printf("\nvar_st: ");
        for (int i = 0; i < num_qubit; i ++) {
            printf("%d, ", variable_start[i]);
        }
        printf("\nvar_ed: ");
        for (int i = 0; i < num_qubit; i ++) {
            printf("%d, ", variable_end[i]);
        }
        printf("\nqubit vars: ");
        for (int i = 0; i < num_qubit; ++ i) {
            for (int j = 0; j < qubit_variable[i].size(); ++ j) {
                printf("%d, ", qubit_variable[i][j]);
            }
            printf("\n");
        }
        puts("");
    }

    inline std::vector<variable> get_start_vars() const {
        return variable_start;
    }

    inline std::vector<variable> get_end_vars() const {
        return variable_end;
    }
};

struct gate {
    std::string name;
    std::vector <variable> new_var;
    int coef;
};

struct complex_gate {
    std::string name;
    std::vector <gate> seq;
};

const std::vector<complex_gate> gate_config(int r);
tensor_network from_qasm(std::string file, std::string config);
