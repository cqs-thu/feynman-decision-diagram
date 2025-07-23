#pragma once

#include <gmp.h>
#include <gmpxx.h>
#include <algorithm>
#include <complex>
#include <iostream>
#include <iterator>
#include <numeric>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <cmath>
#include <sys/time.h>

#include "global.h"
#include "common.h"
#include "tensor_network.h"
// #define TENSOR_DEBUG
// #define BENCHMARK_TIME
// #define TERM_ORDER_DEBUG
// #define CONTRACT_TREE_DEBUG
// The term class stores a product term of the form
// 'coeff * x1 * x2 * ... * xn'
//
// The term will corresponds to a power gate or tensor

extern int omega; // modulus
extern double elapsedTime(const struct timeval &t1, const struct timeval &t2);
static constexpr long double PI = static_cast<long double>(
    3.141592653589793238462643383279502884197169399375105820974L);

extern double order_time, bdd_time, mod_plus_time, eval_time;
extern int init_node_cnt, peak_node_cnt;

template< typename ContainerT, typename PredicateT >
void erase_if( ContainerT& items, const PredicateT& predicate ) {
    for( auto it = items.begin(); it != items.end(); ) {
        if( predicate(*it) ) it = items.erase(it);
        else ++it;
    }
}

template <typename item>
void make_unique(std::vector<item> &vec) {
    std::sort(vec.begin(), vec.end());
    vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
}

template <typename item>
void ignore_appeared_ele(std::vector<item> &vec) {
    std::unordered_set<item> appeared;
    vec.erase(std::remove_if(vec.begin(), vec.end(),
                             [&](auto x) { return appeared.find(x) != appeared.end() ? true : (appeared.insert(x), false); }),
              vec.end());
}

// Concat the second into the first, make unique
template <typename item>
void concat_unique(std::vector<item> &a, const std::vector<item> &b) {
    a.insert(a.end(), b.begin(), b.end());
    make_unique(a);
}

// Remove items in a that appear in b
template <typename item>
void difference(std::vector<item> &a, const std::vector<item> &b) {
    a.erase(std::remove_if(begin(a), end(a),
                      [&](auto x) { return std::find(begin(b), end(b), x) != end(b); }), end(a));
}

template <typename item>
void subst_only(std::vector<item> &target, item from, item to) {

    for (auto it = target.begin(); it != target.end(); ++it) {
        if (*it == from) {
            *it = to;
        }
    }
}

// Shift all vars in v
template <typename item>
void shift_all_vars(std::vector<item> &v, item shift) {
    std::for_each(v.begin(), v.end(), [&shift](item &n) {
        n += shift;
    });
}

// Substitute variables in target
template <typename item>
void subst_unique(std::vector<item> &target,
                  const std::vector<item> from,
                  const std::vector<item> to) {

    assert(from.size() == to.size());

    for (auto it = target.begin(); it != target.end(); ++it) {
        for (size_t i = 0; i < from.size(); ++i) {
            if (*it == from[i])
                *it = to[i];
        }
    }

    make_unique(target);
}

class term {
    friend class sum_power;

  private:
    int coeff;
    std::vector<variable> vars; // This should be a *sorted* list of variables

  public:
    term() : coeff(0) {}

    term(int coeff, const std::vector<variable> &vars) : coeff(coeff), vars(vars) {}

    term(std::vector<tensor_network::edge> &gate, int coeff) : coeff(coeff) {
        // Use edges connected to a gate to construct a term
        for (auto arrow : gate)
            vars.push_back(arrow.var);
        make_unique(vars);
    }

  public:
    inline std::string to_string() const {

        if (coeff == 0) {
            return "0";
        }

        if (vars.size() == 0) {
            return std::to_string(coeff);
        }

        std::string pre = coeff == 1 ? "x" : std::to_string(coeff) + " * x";
        pre += std::to_string(*(vars.begin()));
        return std::accumulate(std::next(vars.begin()), vars.end(), pre,
                               [](std::string x, int y) { return x + " * x" + std::to_string(y); });
    }

    inline void print() { std::cout << to_string() << std::endl; }

    inline bool has_var(variable x) { return std::find(vars.begin(), vars.end(), x) != vars.end(); }

    inline variable another_var(variable x) { return (vars[0] == x) ? vars[1] : vars[0]; } // return a variable connected to x
};

// The sum_power class stores the data for the sum of powers
class sum_power {

  private:
    int r;
    std::vector<term> terms;
    std::vector<variable> sum_vars; // Internal vars
    std::vector<variable> vars; // All variables, sorted
    int factor_count;

    std::vector<std::vector<variable>> var_by_qubit_order;
    std::vector<variable> var_by_gate_order;
  public:

    sum_power() { r = 0; factor_count = 0; }
    sum_power(int r, const std::vector<term> &terms, const std::vector<variable> &vars, int factor_count = 0)
        : r(r), terms(terms), vars(vars), factor_count(factor_count) {}
    sum_power(int r, const std::vector<term> &terms, int factor_count = 0) : r(r), terms(terms), factor_count(factor_count) {
        vars = vars_in_terms(terms);
    }
    sum_power(const sum_power &sop) {
        r = sop.r;
        terms = sop.terms;
        sum_vars = sop.sum_vars;
        vars = sop.vars;
        factor_count = sop.factor_count;
        var_by_qubit_order = sop.var_by_qubit_order;
        var_by_gate_order = sop.var_by_gate_order;
    }

    sum_power(const tensor_network &t) {
        r = t.r;
        for (unsigned i = 0; i < t.gate_edges.size(); ++i) {
            auto edges_of_gate = t.gate_edges[i];
            auto coeff = t.exponent[i];
            if (edges_of_gate.size() == 0)
                continue;
            add_term(term(edges_of_gate, coeff));

            
            // // Add internal vars to sum_vars
            // for (auto edge : edges_of_gate) { // TODO: 非常可疑的操作
            //     if (edge.neighbor != INVALID_GATE) {
            //         sum_vars.push_back(edge.var);
            //     }
            // }
            // make_unique(sum_vars);
        }

        vars = vars_in_terms(terms);

        std::vector<variable> start_v = t.get_start_vars();
        std::vector<variable> end_v = t.get_end_vars();
        std::unordered_set<variable> start_vh(start_v.begin(), start_v.end());
        std::unordered_set<variable> end_vh(end_v.begin(), end_v.end());

        for(auto&& v: vars) {
            if (start_vh.find(v) == start_vh.end() && end_vh.find(v) == end_vh.end()) {
                sum_vars.push_back(v);
            }
        }

        factor_count = t.get_factor_count();
        var_by_qubit_order = t.get_variable_by_qubit_order();
        var_by_gate_order = t.get_variable_by_gate_order();
    }

  public:
    inline int get_nvars() { return vars.size(); }

    int get_r() const { return r; }

    int get_factor_count() const { return factor_count; }

    std::vector<std::vector<variable>> get_var_by_qubit_order() const {
        return var_by_qubit_order;
    }

    std::vector<variable> get_vars() const { return vars; }
    std::vector<variable> get_sum_vars() const { return sum_vars; }

    void all_sum_vars() { sum_vars = vars; }

    void remove_var(variable v) {

        terms.erase(std::remove_if(terms.begin(), terms.end(), [this, &v](auto &elem){
            return elem.has_var(v);
        }), terms.end());

        vars.erase(std::remove_if(vars.begin(), vars.end(), [this, &v](auto &elem){
            return v == elem;
        }), vars.end());

        sum_vars.erase(std::remove_if(sum_vars.begin(), sum_vars.end(), [this, &v](auto &elem){
            return v == elem;
        }), sum_vars.end());
    }

    // Shift all vars in sum_vars
    void shift_sum_vars(std::vector<int> &v, int shift) {
        std::for_each(v.begin(), v.end(), [this, &shift](int &n) {
            if (std::find(sum_vars.begin(), sum_vars.end(), n) != sum_vars.end()) {
                n += shift;
            }
        });
    }

    void shift_sum_vars(int shift) {
        shift_sum_vars(vars, shift);
        make_unique(vars); // Make sure vars is sorted
        for(auto&& v: var_by_qubit_order) {
            shift_sum_vars(v, shift);
        }
        for (auto &term : terms) {
            shift_sum_vars(term.vars, shift);
            make_unique(term.vars);
        }

        shift_sum_vars(sum_vars, shift); // Do this at last
    }

    void shift_vars(int shift) {

        for (auto & v:var_by_qubit_order) {
            shift_all_vars(v, shift);
        }

        for (auto &term : terms) {
            shift_all_vars(term.vars, shift);
        }

        shift_all_vars(vars, shift);
        shift_all_vars(sum_vars, shift);
    }

    // Works for sop of all vars summed
    // Variables in frozen stays (or linked)
    std::vector<variable> counting_identity_simplify(const std::vector<variable> &frozen = {}) {

        std::unordered_set<variable> frozen_set(frozen.begin(), frozen.end());
        std::vector<variable> link = frozen;

        auto deg = compute_degrees();

        erase_if(deg, [](const auto& item) {
            auto const& [key, value] = item;
            return (value > 2);
        });

        for(auto const& pair: deg) {
            counting_identity_simplify_var(pair.first, frozen_set, link);
        }

        combine_terms();
        return link;

    }

    // If v is is in exactly two degree two or one, coeff r/2 terms,
    // simplify using the cancellation rule.
    void counting_identity_simplify_var(
        const variable v, const std::unordered_set<variable> &frozen,
        std::vector<variable> &link) {

        std::vector<term> vt;

        // Find the terms containing v
        std::for_each(terms.begin(), terms.end(), [&vt, v](term &t) {
            if (t.has_var(v)) {
                vt.push_back(t);
            }
        });

        // vt must have size 1 or 2, v cannot be frozen
        if (vt.size() > 2 || frozen.find(v) != frozen.end()) {
            return;
        }

        // abort if coeff is not r/2 or the term has more than 2 vars
        for (auto t: vt) {
            if (t.coeff != r/2 || t.vars.size() > 2) {
                return;
            }
        }

        if (vt.size() == 1) {
            // v x case
            if (vt[0].vars.size() == 2) {
                auto x = vt[0].another_var(v);

                if (frozen.find(x) == frozen.end()) {
                   // x is not frozen, x -> 0
                   remove_var(v);
                   set_zeros({x});
                   factor_count -= 2;
                }
            }
            return;
        }

        if (vt.size() == 2) {

            // v x + v y
            if (vt[0].vars.size() == 2 && vt[1].vars.size() == 2) {
               auto x = vt[0].another_var(v);
               auto y = vt[1].another_var(v);

               remove_var(v);
               factor_count -= 2;
               if (frozen.find(x) == frozen.end()) {
                   // x is not frozen, x -> y
                   subst_var_only(x, y);
                   return;
               }

               if (frozen.find(y) == frozen.end()) {
                   // y is not frozen, y -> x
                   subst_var_only(y, x);
                   return;
               }

               // x y are both frozen, link them
               subst_var_only(x, y);
               subst_only(link, x, y);
               return;
            }

            // v x + v
            auto x = vt[0].vars.size() == 1 ? vt[1].another_var(v) : vt[0].another_var(v);
            if (frozen.find(x) == frozen.end()) {
                // x is not frozen, x -> 1
                remove_var(v);
                set_ones({x});
                factor_count -= 2;
            }
        }
    }

    std::map<variable, int> compute_degrees() {
        std::map<variable, int> deg;

        std::for_each(terms.begin(), terms.end(), [&deg](term &t) {

            for (auto v : t.vars) {
                deg[v]++;
            }
        });

        return deg;
    }

    void set_sum_vars(const std::vector<variable> &v) {
        sum_vars = v;
        make_unique(sum_vars);
    }


    std::vector<variable> compute_variable_order_from_term_order(
        std::vector<term> &ordered_terms) {

        std::vector<variable> ordered_vars;

        std::unordered_set<variable> var_index;
        for (auto &t : ordered_terms) {
            for(auto &v: t.vars) {
                if (var_index.count(v) == 0) {
                    var_index.emplace(v);
                    ordered_vars.push_back(v);
                }
            }
        }
        return ordered_vars;
    }

    void negate() { // 模r意义下取负 & 翻转qubit order
        for (auto &term : terms) {
            term.coeff = r - term.coeff;
        }
        for (auto&& v: var_by_qubit_order) {
            std::reverse(v.begin(), v.end());
        }
        std::reverse(terms.begin(), terms.end());
    }

    void add_term(const term &t) { terms.push_back(t); }

    auto num_terms() const { return terms.size(); }

    auto num_vars() const { return vars.size(); }

    // void sort_variables_by_contract_order() {
    //     static int count = 0;
    //     const std::string working_path = "./src/working_contract";
    //     const std::string tmp_file_path = working_path + "/tmp"+std::to_string(count)+".txt";
    //     count += 1;
    //     const std::string order_file_path = working_path + "/order.txt";
    //     const std::string contract_py_path = working_path + "/sop_contract.py";
    //     this->print_as_tensor(tmp_file_path);
    //     const std::string command = "python3 " + contract_py_path + " -i " + tmp_file_path + " -o " + order_file_path + " -l " + std::to_string(cotengra_timeout_limit);
    //     int ret_value = system(command.c_str());
    //     if (ret_value != 0) {
    //         std::cerr << "Error: cannot run command " << command << std::endl;
    //         return;
    //     } else {
    //         std::ifstream order_file(order_file_path);
    //         std::string line;
    //         std::vector<variable> ordered_vars;
    //         bool order_is_valid = true;
    //         while (std::getline(order_file, line)) {
    //             try {
    //                 variable v = std::stoi(line);
    //                 ordered_vars.push_back(v);
    //             } catch (const std::exception& e) {
    //                 order_is_valid = false;
    //                 break;
    //             }
    //         }
    //         order_file.close();
    //         if (order_is_valid) {
    //             this->vars = ordered_vars;
    //         }
    //         #ifndef TENSOR_DEBUG
    //         try {
    //             std::filesystem::remove(tmp_file_path);
    //             std::filesystem::remove(order_file_path);
    //         } catch (const std::filesystem::filesystem_error& e) {
    //             std::cerr << "Error: " << e.what() << std::endl;
    //         }
    //         #endif
    //     }
    // }

    std::vector<variable> compute_tensor_order() {

        std::vector<term> ordered_terms;
        static int count = 0;
        const std::string working_path = "./src/working_contract";
        const std::string tmp_file_path = working_path + "/tmp" + \
            std::to_string(count++) + ".txt";
        const std::string order_file_path = working_path + "/term_order.txt";
        const std::string contract_py_path = working_path + "/sop_contract.py";
        this->print_as_tensor(tmp_file_path);

        const std::string command = "python3 " + contract_py_path + \
            " -i " + tmp_file_path + " -o " + order_file_path + " -l " + \
            std::to_string(cotengra_timeout_limit) + " -t " + TERM_COMMAND;

        int ret_value = system(command.c_str());

        if (ret_value != 0) {
            std::cerr << "Error: cannot run command " << command << std::endl;
            return vars;
        }

        std::ifstream order_file(order_file_path);
        std::string line;
        ordered_terms.clear();
        bool order_is_valid = true;
        enum ReadType {
            Invalid,
            Term,
            Variable
        };
        std::vector<variable> ordered_vars;
        ReadType read_type = Invalid;
        while (std::getline(order_file, line)) {
            try {
                if (line == "term:") {
                    read_type = Term;
                    continue;
                } else if (line == "variable:") {
                    read_type = Variable;
                    continue;
                }

                if (read_type == Invalid) {
                    order_is_valid = false;
                    break;
                } else if (read_type == Term) {
                    std::istringstream stream(line);
                    int term_index;
                    if (!(stream >> term_index)) {
                        throw std::invalid_argument("Invalid term index");
                    }
                    if (term_index < 0 || term_index >= terms.size()) {
                        throw std::invalid_argument("Out of range term index");
                    }
                    #ifdef TERM_ORDER_DEBUG
                        std::cout << term_index << " ";
                    #endif
                    ordered_terms.push_back(terms[term_index]);
                } else if (read_type == Variable) {
                    variable v = std::stoi(line);
                    ordered_vars.push_back(v);
                }
            } catch (const std::exception& e) {
                order_is_valid = false;
                break;
            }
        }
        order_file.close();

        if (!order_is_valid) {
            ordered_terms = terms;
            #ifdef TERM_ORDER_DEBUG
                std::cout << "get term order fail!" << std::endl;
            #endif
        } else {
            this->vars = ordered_vars;
            #ifdef TERM_ORDER_DEBUG
                std::cout << "term order is:" << std::endl;
                for (auto term: ordered_terms) {
                    term.print();
                }
                std::cout << std::endl;
                std::cout << "variable order is :";
                for (auto v: this->vars) {
                    std::cout << v << " ";
                }
                std::cout << std::endl;
            #endif
        }
        #ifndef TERM_ORDER_DEBUG
        try {
            std::filesystem::remove(tmp_file_path);
            std::filesystem::remove(order_file_path);
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
        #endif

        assert(terms.size() == ordered_terms.size());
        terms = ordered_terms;

        return compute_variable_order_from_term_order(ordered_terms);
    }

    std::vector<std::pair<int, int>> get_contract_path() const {
        std::vector<std::pair<int, int>> contract_path;
        contract_path.reserve(terms.size());
        static int count = 0;
        const std::string working_path = "./src/working_contract";
        const std::string tmp_file_path = working_path + "/tmp"+std::to_string(count)+".txt";
        count += 1;
        const std::string contract_file_path = working_path + "/contract_path.txt";
        const std::string contract_py_path = working_path + "/sop_contract.py";
        this->print_as_tensor(tmp_file_path);
        const std::string command = "python3 " + contract_py_path + " -i " +
            tmp_file_path + " -o " + contract_file_path + " -l " +
            std::to_string(cotengra_timeout_limit) + " -t " + PATH_COMMAND;
        
        auto generate_default_sequence = [&]() {
            for (int i = 0; i < this->terms.size() - 1; ++i) {
                contract_path.push_back(std::make_pair(i, i+1));
            }
        };

        int ret_value = system(command.c_str());
        if (ret_value != 0) {
            std::cerr << "Error: cannot run command " << command << std::endl;
            // generate default sequence 
            generate_default_sequence();
            return contract_path;
        } else {
            std::ifstream contract_file(contract_file_path);
            std::string line;
            bool contract_path_is_valid = true;
            while (std::getline(contract_file, line)) {
                std::istringstream stream(line);
                int term_a;
                int term_b;
                if (!(stream >> term_a >> term_b)) {
                    contract_path_is_valid = false;
                    break;
                }
                if (term_a < 0 || term_a >= terms.size() || term_b < 0 ||
                    term_b >= terms.size()) {
                    contract_path_is_valid = false;
                    break;
                }
                contract_path.push_back(std::make_pair(term_a, term_b));
            }
            contract_file.close();
            if (!contract_path_is_valid) {
                contract_path.clear();
                generate_default_sequence();
            } else {
                #ifdef CONTRACT_TREE_DEBUG
                    std::cout << "contract path is:" << std::endl;
                    for (auto pair: contract_path) {
                        std::cout << "(" << pair.first << ", " << pair.second << ")" << std::endl;
                    }
                #endif
            }
            return contract_path;
        }
    }

    // All order functions should return the ordered variables and order the terms
    std::vector<variable> compute_default_order() {
        return vars;
    }

    std::vector<variable> compute_qubit_order() {

        std::vector<variable> ordered_vars(vars);
        std::unordered_map<variable, int> var_index;

        int idx_cnt = 0;
        for (int i = 0; i < this->var_by_qubit_order.size(); ++i) {
            for (int j = 0; j< this->var_by_qubit_order[i].size(); ++j) {
                var_index[this->var_by_qubit_order[i][j]] = idx_cnt++;
            }
        }

        std::sort(ordered_vars.begin(), ordered_vars.end(),
                  [&var_index](const variable &a, const variable &b) {
            return var_index[a] < var_index[b];
        });

        if (sort_setting == SORT_BY_QUBIT_SUM_TERM) {
            set_term_order_from_variable_order(ordered_vars);
        }

        return ordered_vars;
    }

    std::vector<variable> compute_gate_order() {

        std::vector<variable> ordered_vars(vars);

        std::unordered_map<variable, int> var_index;

        for (int i = 0; i < this->var_by_gate_order.size(); ++i) {
            var_index[this->var_by_gate_order[i]] = i;
        }

        std::sort(ordered_vars.begin(), ordered_vars.end(),
                  [&var_index](const variable &a, const variable &b) {
            return var_index[a] < var_index[b];
        });

        return ordered_vars;
    }

    struct term_hasher
    {
        size_t operator()(const term& t) const
        {
            return std::hash<std::string>{}(t.to_string());
        }
    };

    struct term_equals
    {
        bool operator()(const term& a, const term& b) const
        {
            return a.to_string() == b.to_string();
        }
    };

    void set_term_order_from_variable_order(std::vector<variable> &ordered_vars) {

        std::unordered_map<variable, int> var_value;
        std::unordered_map<term, int, term_hasher, term_equals> term_index;
        int value = 0;

        for (auto &v: ordered_vars) {
            var_value[v] = value++;
        }

        for (auto &t: terms) {
            int term_value = 0;
            for (auto &vars: t.vars) {
                term_value += var_value[vars];
            }
            term_index[t] = term_value;
        }

        std::sort(terms.begin(), terms.end(),
                  [&term_index](const term &a, const term &b) {
            return term_index[a] < term_index[b];
        });
    }

    void set_term_order_from_variable_order_min(std::vector<variable> &ordered_vars) {

        std::unordered_map<variable, int> var_value;
        std::unordered_map<term, int, term_hasher, term_equals> term_index;
        int value = 0;

        for (auto &v: ordered_vars) {
            var_value[v] = value++;
        }

        for (auto &t: terms) {
            int term_value = var_value[t.vars[0]];
            for (auto &vars: t.vars) {
                if (term_value > var_value[vars]) {
                    term_value = var_value[vars];
                }
            }
            term_index[t] = term_value;
        }

        std::sort(terms.begin(), terms.end(),
                  [&term_index](const term &a, const term &b) {
            return term_index[a] < term_index[b];
        });
    }

    template <typename adapter_t>
    void set_orders(adapter_t &adapter) {
        switch (sort_setting) {
            case NO_SORT:
            case SORT_BY_QUBIT_ORDER:
            case SORT_BY_QUBIT_SUM_TERM:
                adapter.intro_vars(this->compute_qubit_order());
                break;
            case SORT_BY_GATE_ORDER:
                adapter.intro_vars(this->compute_gate_order());
                break;
            case SORT_BY_COTENGRA_TERM:
                adapter.intro_vars(this->compute_tensor_order());
                break;
            default:
                adapter.intro_vars(this->compute_default_order());
                break;
        }
    }

    std::vector<variable> vars_in_terms(const std::vector<term> &terms) const {
        std::vector<variable> vars;

        for (auto term : terms)
            concat_unique(vars, term.vars);

        return vars;
    }

    sum_power operator+(sum_power &sop) {
        sum_power res(*this);

        res.terms.insert(res.terms.end(), sop.terms.begin(), sop.terms.end());
        
        concat_unique(res.vars, sop.vars);
        concat_unique(res.sum_vars, sop.sum_vars);
        res.factor_count += sop.factor_count;
        res.add_qubit_order(sop); // Merge the var_by_qubit_order
        return res;
    }

    void add_qubit_order(sum_power &sop) {
        for (int i = 0; i < var_by_qubit_order.size(); ++i) {
            concat_unique(var_by_qubit_order[i], sop.var_by_qubit_order[i]);
        }
    }

    std::string to_string() {
        if (terms.size() == 0) { return "0"; }
        std::vector<std::string> terms_string;
        std::string pre = terms[0].to_string();
        return std::accumulate(std::next(terms.begin()), terms.end(), pre,
                               [](std::string x, term &y) { return x + " + " + y.to_string(); });
    }

    void print() const {
        std::cout << "r: " << r << std::endl;
        std::cout << "Number of terms: " << num_terms() << std::endl;
        std::cout << "Number of variables: " << num_vars() << std::endl;
        std::cout << "Factor count: " << factor_count << std::endl;
        std::cout << "Terms: " << std::endl;
        for (auto term : terms)
            term.print();
    }

    void print_as_tensor(const std::string& filename) const {
        std::ofstream output_file(filename, std::ios::trunc);
        if (!output_file) {
            std::cerr << "Error: cannot open file " << filename << std::endl;
            return;
        }
        output_file << "IO: ";
        std::unordered_set<variable> sum_vars_set(sum_vars.begin(), sum_vars.end());
        for (auto var: vars) {
            if (sum_vars_set.find(var) == sum_vars_set.end()) {
                output_file << var << " ";
            }
        }
        output_file << std::endl;
        for (auto term: terms) {
            for (auto var : term.vars) {
                output_file << var << " ";
            }
            output_file << std::endl;
        }
        output_file.close();
    }

    void full_dump() const {

        std::cout << std::endl;
        std::cout << "A full dump of the sum of power data structure";
        std::cout << std::endl << std::endl;

        std::cout << "Modulus: " << r;
        std::cout << std::endl << std::endl;

        std::cout << "Factor count: " << factor_count << std::endl << std::endl;

        std::cout << "Terms: " << std::endl;
        for (auto &term : terms) {
            std::cout << "  Coeff: " << term.coeff << std::endl;
            std::cout << "  Vars: ";
            for (auto &var : term.vars) {
                std::cout << var << " ";
            }
            std::cout << std::endl << std::endl;
        }

        std::cout << "Variables: ";
        for (auto &var : vars) {
            std::cout << var << " ";
        }
        std::cout << std::endl << std::endl;

        std::cout << "Summed Variables: ";
        for (auto &var : sum_vars) {
            std::cout << var << " ";
        }
        std::cout << std::endl << std::endl;

    }

    template <typename adapter_t>
    typename adapter_t::dd_t to_mtbdd_plus(adapter_t &adapter) {

        omega = r;

        auto result = adapter.leaf(0);

        for (auto term : terms) {
            auto term_result = adapter.leaf(term.coeff);
            for (auto var : term.vars) {
                auto var_result = adapter.ithvar(var);
                term_result = term_result * var_result;
            }
            result = adapter.mod_plus(result, term_result);
        }

        return result;
    }

    // Use ordered_terms to compute the mtbdd
    template <typename adapter_t>
    typename adapter_t::dd_t to_mtbdd_bin(adapter_t &adapter, int left, int right) {
        if(left>right)
        {
            return adapter.leaf(0);
        }
        if(left==right)
        {
            auto term = terms[left];
            auto term_result = adapter.leaf(term.coeff);
            for (auto var : term.vars) {
                adapter.intro_var(var);
                auto var_result = adapter.ithvar(var);
                term_result = term_result * var_result;
            }
            return term_result;
        }

        int m = left + (right - left) / 2;

        return adapter.mod_plus(to_mtbdd_bin(adapter, left, m), to_mtbdd_bin(adapter, m+1, right));
    }

    template <typename adapter_t>
    typename adapter_t::dd_t to_mtbdd_bin(adapter_t &adapter) {
        omega = r;
        return to_mtbdd_bin<adapter_t>(adapter, 0, terms.size() - 1);
    }

    template <typename adapter_t>
    typename adapter_t::dd_t to_mtbdd_tree(adapter_t &adapter) {
        omega = r;
        if (terms.size() <= 2) {    // 处理退化情况
            auto result = adapter.leaf(0);
            for (auto term: terms) {
                auto term_result = adapter.leaf(term.coeff);
                for (auto var: term.vars) {
                    auto var_result = adapter.ithvar(var);
                    term_result = term_result * var_result;
                }
                result = adapter.mod_plus(result, term_result);
            }
            return result;
        }
        std::vector<std::pair<int, int>> contract_path = this->get_contract_path();
        struct TreeNode {
            bool is_leaf;
            int leaf_term_index;
            TreeNode* left_child;
            TreeNode* right_child;
            TreeNode(bool is_leaf_ = false, int leaf_term_index_ = -1, TreeNode* left_child_ = nullptr, TreeNode* right_child_ = nullptr):
                is_leaf(is_leaf_), leaf_term_index(leaf_term_index_), left_child(left_child_), right_child(right_child_ ){}
            
            
            typename adapter_t::dd_t get_dd(const std::vector<term>& origin_terms, adapter_t &adapter) const {
                if (is_leaf) {
                    auto term = origin_terms[leaf_term_index]; 
                    auto term_result = adapter.leaf(term.coeff);
                    for (auto var: term.vars) {
                        adapter.intro_var(var);
                        auto var_result = adapter.ithvar(var);
                        term_result = term_result * var_result;
                    }
                    return term_result; 
                } else {
                    auto left_dd= left_child->get_dd(origin_terms, adapter);
                    auto right_dd = right_child->get_dd(origin_terms, adapter);
                    return adapter.mod_plus(left_dd, right_dd);
                }               
            }
        };
        std::vector<TreeNode*> tree_nodes;
        tree_nodes.reserve(terms.size());
        for (int i = 0; i < terms.size(); ++i) {
            tree_nodes.push_back(new TreeNode(true, i, nullptr, nullptr));
        }
        for (auto pair : contract_path) {   // O(n^2)的路径解析，可能是重要的优化点
            int a = pair.first;
            int b = pair.second;
            TreeNode* new_node = new TreeNode(false, -1, tree_nodes[a], tree_nodes[b]);
            if (a < b) {
                tree_nodes.erase(tree_nodes.begin() + b);
                tree_nodes.erase(tree_nodes.begin() + a);
            } else {
                tree_nodes.erase(tree_nodes.begin() + a);
                tree_nodes.erase(tree_nodes.begin() + b);
            }
            tree_nodes.push_back(new_node);
        }
        #ifdef BENCHMARK_TIME
            std::cout <<"get path and build DD begin.." << std::endl;
        #endif
        return tree_nodes[0]->get_dd(terms, adapter);
    }

    void set_zeros(const std::vector<variable> &vs) {

        std::unordered_set<variable> vs_set(vs.begin(), vs.end());

        int p = 0;

        for (int i = 0; i < terms.size(); ++i) {
            bool should_delete = false;
            for (int j = 0; j < terms[i].vars.size(); ++j) {
                if (vs_set.count(terms[i].vars[j])) {
                    should_delete = true;
                    break;
                }
            }
            if (should_delete) {
                continue;
            } else {
                terms[p++] = terms[i];
            }
        }
        terms.resize(p);

        difference(vars, vs);

        // Remove vs from var_by_qubit_order
        for (auto &vec: var_by_qubit_order) {
            difference(vec, vs);
        }
    }

    void combine_terms() {
        if (terms.empty()) {
            return;
        }
        for (int i = 0; i < terms.size(); ++i) {   // 首先确保term内部的var是有序的
            sort(terms[i].vars.begin(), terms[i].vars.end());
        }
        auto term_compare = [](const term& x, const term& y) {  // 比较term之间的顺序
            if (x.vars.size() != y.vars.size()) {
                return x.vars.size() < y.vars.size();
            }
            for (int i = 0; i < x.vars.size(); ++i) {
                if (x.vars[i] == y.vars[i]) {
                    continue;
                }    
                return x.vars[i] < y.vars[i];
            }
            return false;
        };
        sort(terms.begin(), terms.end(), term_compare);
        auto term_equal_var = [](const term& x, const term& y) {    // 判断同类项
            if (x.vars.size() != y.vars.size()) {
                return false;
            }
            for (int i = 0; i < x.vars.size(); ++i) {
                if (x.vars[i] != y.vars[i]) {
                    return false;
                }
            }
            return true;
        };
        int p = 0;
        int q_begin = 0;
        int q_end = 1;
        while (q_end < terms.size()) {
            if (term_equal_var(terms[q_begin], terms[q_end])) {
                terms[q_begin].coeff += terms[q_end].coeff;
                terms[q_begin].coeff %= r;
            } else {
                if (terms[q_begin].coeff != 0) {
                    terms[p++] = terms[q_begin];
                }
                q_begin = q_end;
            }
            q_end += 1;
        }
        if (q_begin < terms.size()) {
            if (terms[q_begin].coeff != 0) {
                terms[p++] = terms[q_begin];
            }
        }
        terms.resize(p);
        int old_var_size = vars.size();
        auto new_term_vars = vars_in_terms(terms);
        int new_var_size = new_term_vars.size();
        assert(old_var_size >= new_var_size);
        if (new_var_size < old_var_size) {  
            factor_count -= 2 * (old_var_size - new_var_size);
            // try to avoid modifying vars...
            vars = new_term_vars;
            std::unordered_set<variable> new_vars(vars.begin(), vars.end());
            erase_if(sum_vars, [&new_vars](const variable& v) { return new_vars.count(v) == 0; });
        }
    }

    void set_ones(const std::vector<int> &vs) {

        for (auto &term : terms) {
            std::vector<int> vars_new;
            std::copy_if(term.vars.begin(), term.vars.end(),
                         std::inserter(vars_new, vars_new.end()),
                         [&vs](int j) { return std::find(vs.begin(), vs.end(), j) == vs.end(); });

            term.vars = vars_new;
        }

        for (auto i = terms.begin(); i != terms.end(); ++i) {
            for (auto j = i+1; j != terms.end(); ) {
                if (j == terms.end()) {
                    break;
                }
                if (j->vars == i->vars && j->coeff != 0) {
                    i->coeff += j->coeff;
                    if (i->coeff >= r) {
                        i->coeff -= r;
                    }
                    j = terms.erase(j);
                } else {
                    ++j;
                }
            }
        }

        terms.erase(std::remove_if(terms.begin(), terms.end(),  
                                   [&](auto &elem){ return elem.coeff == 0; }), terms.end());

        // Do not recompute vars here!
        // Even if the terms are cancelled, the sum_vars should remain.
        difference(vars, vs);

        // Remove vs from var_by_qubit_order
        for (auto &vec: var_by_qubit_order) {
            difference(vec, vs);
        }

        // TODO gate order?
    }

    // For an sop x * a + b where b does not contain x, return the sop a
    void set_one_cofactor(variable x) {
        terms.erase(std::remove_if(terms.begin(), terms.end(),
                                   [&](auto &elem) { return !elem.has_var(x); }), terms.end());
        set_ones({x});
    }


    void set_values(const std::map<int, int> &vals) {
        std::vector<int> zeros, ones;
        for (auto it = vals.begin(); it != vals.end(); it++) {
            if (it->second) {
                ones.push_back(it->first);
            } else {
                zeros.push_back(it->first);
            }
        }
        set_zeros(zeros);
        set_ones(ones);
    }

    void subst_vars(const std::vector<int> from, const std::vector<int> to) {
        for (auto &term : terms) {
            subst_unique(term.vars, from, to);
        }

        vars = vars_in_terms(terms);
        concat_unique(sum_vars, to); // TODO: Why to? So it requires that to will be summed and not work if we simply rename a var
    }

    void subst_var_only(const variable from, const variable to) {

        // This can happen!
        if (from == to) {
            return;
        }

        for (auto &term : terms) {
            subst_only(term.vars, from, to);
        }

        difference(vars, {from});
        difference(sum_vars, {from});

        // Remove vs from var_by_qubit_order
        for (auto &vec: var_by_qubit_order) {
            subst_only(vec, from, to);
        }
    }

    template <typename adapter_t>
    double norm_squared(adapter_t &adapter) {
        sum_power sop(*this);
        sop.negate(); // Compute the complex conjudate of the sop
        sop.shift_sum_vars(num_vars());
        auto sop_sum = sop + *this;
        sop_sum.sum_vars = sop_sum.vars;
        //sop_sum.hi_simplify();
        return sop_sum.evaluate(adapter).real();
    }

    template <typename adapter_t>
    std::complex<long double> evaluate(adapter_t &adapter) {
        #ifdef BENCHMARK_TIME
            std::cout << "[evaluate], begin" << std::endl;
        #endif
        this->combine_terms();
        this->counting_identity_simplify();
        this->set_orders(adapter);

        struct timeval bdd_start, bdd_end;
        gettimeofday(&bdd_start, NULL);

        auto mtbdd = to_mtbdd_bin(adapter);

        gettimeofday(&bdd_end, NULL);
        bdd_time += elapsedTime(bdd_start, bdd_end);
        init_node_cnt = adapter.get_node_count(mtbdd);
        peak_node_cnt = adapter.get_peak_count();
        #ifdef BENCHMARK_TIME
            std::cout << "[evaluate], to_mtbdd done" << std::endl;
        #endif

        return adapter.evaluate(mtbdd, this->factor_count);
    }
};
