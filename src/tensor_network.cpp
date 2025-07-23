#include "tensor_network.h"
#include "json.hpp"
using json = nlohmann::json;

#include <iostream>
#include <string>
#include <fstream>
#include <cassert>
#include <map>

namespace read_buf {
    std::map <std::string, int> gate_id;
    std::ifstream input;
    std::string buf;

    void load(std::string file, std::vector<::complex_gate> &gates) {
        input.open(file);
        for (auto i = 0; i < (int)gates.size(); i ++) {
            gate_id[gates[i].name] = i;
        }
        buf.clear();
    }

    void close() {
        input.close();
    }

    int gate() {
        buf.clear();
        for (; !input.eof(); ) {
            input >> buf;
            auto it = gate_id.find(buf);
            if (it != gate_id.end()) {
                return it -> second;
            }
            else {
                auto tmp = buf.find_first_of('[');
                if (buf.size() > 0 && tmp == buf.npos) {
                    std::cerr << "[WARNING] Unknown gate: " << buf << std::endl;
                }
            }
        }
        return -1;
    }

    int number() {
        for (; !input.eof() || !buf.empty(); ) {
            if (buf.empty()) {
                input >> buf;
            }
            auto tmp = buf.find_first_of('[');
            if (tmp == buf.npos) {
                buf.clear();
                continue;
            }
            buf.erase(0, tmp + 1);
            tmp = buf.find_first_of(']');
            if (tmp == buf.npos) {
                buf.clear();
                continue;
            }
            int ret = atoi(buf.substr(0, tmp).c_str());
            buf.erase(0, tmp + 1);
            return ret;
        }
        return -1;
    }
}

const std::vector<complex_gate> gate_config(std::string config, int &mod) {
    std::ifstream f(config);
    json data = json::parse(f);
    
    mod = data["mod"];

    std::map<std::string, gate> gates;
    std::vector<complex_gate> complex_gates;

    for (auto gate: data["gates"]) {
        std::string name = gate["name"];
        std::vector<variable> new_var;
        for (auto var: gate["new_var"]) {
            new_var.push_back(var);
        }
        gates[name] = {name, new_var, gate["coef"]};
    }

    for (auto complex_gate: data["complex_gates"]) {
        std::string name = complex_gate["name"];
        std::vector<gate> seq;
        for (auto gate: complex_gate["seq"]) {
            seq.push_back(gates[gate]);
        }
        complex_gates.push_back({name, seq});
    }

    return complex_gates;
}

tensor_network from_qasm(std::string file, std::string config) {
    int r;
    auto complex_gates = gate_config(config, r);
    read_buf::load(file, complex_gates);

    auto tn = tensor_network(read_buf::number(), r);
    for (int id = read_buf::gate(); id != -1; id = read_buf::gate()) {
        auto complex_gate = complex_gates[id];
        int cnt = complex_gate.seq[0].new_var.size();
        auto qubits = std::vector <int> (cnt);
        for (int i = 0; i < cnt; i ++) {
            qubits[i] = read_buf::number();
        }
        if (qubits.back() == -1) {
            break;
        }
        for (auto gate : complex_gate.seq) {
            int gate_id = tn.get_new_gate();
            auto new_vars = std::vector <int> (cnt);
            for (int i = 0; i < cnt; i ++) {
                if (gate.new_var[i] >= 0) {
                    new_vars[i] = tn.variable_end[qubits[gate.new_var[i]]];
                }
                else if (gate.new_var[i] == -1) {
                    new_vars[i] = tn.get_new_var();
                    tn.factor_count += 1;
                }
            }
            for (int i = 0; i < cnt; i ++) {
                if (gate.new_var[i] >= -1) {
                    tn.add_edge(gate_id, qubits[i], new_vars[i]);
                    tn.qubit_variable[qubits[i]].push_back(new_vars[i]);
                }
            }
            tn.exponent.push_back(gate.coef);
        }
    }
    for (int i = 0; i < tn.num_qubit; i ++) {
        tn.add_edge(-1, i, -1); 
        assert(tn.last_gate[i] == -1);
    }
    read_buf::close();
    return tn;
}

