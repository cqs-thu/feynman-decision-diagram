#pragma once

#include "common.h"
#include "sum_power.h"
#include "tensor_network.h"
#include <random>
#include <sys/time.h>
#include <limits>
#include <unistd.h>
#include <sys/resource.h>
// It is expected that the adapter has the following member functions
//
//   1. leaf(int)
//   2. add_const(int)
//   3. ithvar(int)
//   4. mod_plus(dd_t, dd_t)
//   5. size()
//   6. export_to_dot(dd_t, filename)
//   7. print_stats()

int omega;  // global variable for the module

double order_time, bdd_time , mod_plus_time, eval_time;
int init_node_cnt, peak_node_cnt;

double elapsedTime(const struct timeval &t1, const struct timeval &t2) {
    return (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1000000.0;
}

size_t getPeakRSS(int who = RUSAGE_SELF) {
    struct rusage rusage;
    getrusage(who, &rusage);
    return (size_t)(rusage.ru_maxrss * 1024L);
}

// The zero_amplitude functions compute the amplitude of all-zeros at the output
// of a quantum circuit, given that the circuit is initialized with the all-zero
// state \ket{0^n} as input.
template <typename adapter_t>
std::complex<long double> zero_amplitude(std::string& filename) {
    tensor_network t = from_qasm(filename, gate_config_file); // TODO: omega;
    return zero_amplitude<adapter_t>(t);
}

template <typename adapter_t>
std::complex<long double> zero_amplitude(adapter_t &adapter, std::string& filename) {
    tensor_network t = from_qasm(filename, gate_config_file); // TODO: omega;
    return zero_amplitude(adapter, t);
}

template <typename adapter_t>
std::complex<long double> zero_amplitude(tensor_network& t) {
    adapter_t adapter;
    return zero_amplitude(adapter, t);
}

template <typename adapter_t>
std::complex<long double> zero_amplitude(adapter_t &adapter, tensor_network& t) {
    sum_power sop(t);
    sop.set_zeros(t.get_start_vars());
    sop.set_zeros(t.get_end_vars());
    return sop.evaluate(adapter);
}

// The amplitude functions compute the amplitude of a specific basis at the
// output of a quantum circuit, given that the circuit is initialized with the
// all-zero state \ket{0^n} as input.
template <typename adapter_t>
std::complex<long double> amplitude(tensor_network& t, std::vector<int>& basis) {
    adapter_t adapter;
    return amplitude(adapter, t, basis);
}

template <typename adapter_t>
std::complex<long double> amplitude(adapter_t &adapter, tensor_network& t, std::vector<int>& basis) {
    // Make sure the qubit order is from the first to the last
    std::vector<int> start_vars = t.get_start_vars();
    std::vector<int> end_vars = t.get_end_vars();

    std::map<int, int> vals;
    std::transform(end_vars.begin(), end_vars.end(), basis.begin(),
                   std::inserter(vals, vals.begin()),
                   std::make_pair<int const&, int const&>);

    std::vector<int> common_vars;
    std::set_intersection(start_vars.begin(), start_vars.end(),
                          end_vars.begin(), end_vars.end(),
                          std::back_inserter(common_vars));

    for (auto v: common_vars) {
        // If any common variables is not set to 0
        if (vals[v]) { return 0.0; }
    }

    sum_power sop(t);
    sop.set_zeros(start_vars);
    sop.set_values(vals);

    return sop.evaluate(adapter);
}

// Only works when the output has no common label with the input
template <typename adapter_t>
long double circuit_accept_probability(const tensor_network& t) {
    sum_power sop(t);
    int nv = sop.num_vars();

    std::vector<variable> in = t.get_start_vars();
    std::vector<variable> out = t.get_end_vars();
    std::map<variable, int> res;
    sop.set_zeros(in);
    adapter_t adapter;

    sum_power sopc(sop);
    sopc.negate();
    sopc.shift_sum_vars(nv);
    sop = sop + sopc;
    sop.all_sum_vars();
    sop.set_orders(adapter);
    auto mtbdd = sop.to_mtbdd_bin(adapter);
    auto factor_count = sop.get_factor_count();

    auto z = term(omega/2, {out[0]});
    sum_power delta(omega, {z}, 0);
    auto d = delta.to_mtbdd_bin(adapter);
    mtbdd = adapter.mod_plus(mtbdd, d);

    return adapter.evaluate(mtbdd, factor_count).real() * 0.5 + 0.5;
}

template <typename adapter_t>
std::string sample_circuit_output_new(const tensor_network& t) {

    sum_power sop(t);
    auto nv = sop.num_vars();

    std::vector<variable> in = t.get_start_vars();
    std::vector<variable> out = t.get_end_vars();
    std::map<variable, int> res;
    sop.set_zeros(in);
    adapter_t adapter;

    sum_power sopdag(sop);
    sopdag.negate();
    sopdag.shift_sum_vars(nv);
    sop = sop + sopdag;
    sop.all_sum_vars();

    struct timeval bdd_start, bdd_end;
    gettimeofday(&bdd_start, NULL);

    if (sort_setting == SORT_BY_QUBIT_ORDER) {
        out = sop.counting_identity_simplify(out);
    }

    sop.set_orders(adapter);
    auto mtbdd = sop.to_mtbdd_bin(adapter);
    gettimeofday(&bdd_end, NULL);
    bdd_time += elapsedTime(bdd_start, bdd_end);

    init_node_cnt = adapter.get_node_count(mtbdd);
    peak_node_cnt = adapter.get_peak_count();

    auto factor_count = sop.get_factor_count();

    long double total_prob = 1.0;

    for (auto q : out) {

        if (res.count(q) != 0) { // Link measured
            continue;
        }
        auto z = term(omega/2, {q});

        sum_power sop_delta(omega, {z}, 0);

        gettimeofday(&bdd_start, NULL);
        auto delta = sop_delta.to_mtbdd_bin(adapter);
        gettimeofday(&bdd_end, NULL);
        bdd_time += elapsedTime(bdd_start, bdd_end);

        gettimeofday(&bdd_start, NULL);
        auto ddz = adapter.mod_plus(mtbdd, delta);
        gettimeofday(&bdd_end, NULL);
        mod_plus_time += elapsedTime(bdd_start, bdd_end);

        gettimeofday(&bdd_start, NULL);
        auto prob = adapter.evaluate(ddz, factor_count).real() * 0.5 + 0.5 * total_prob;
        gettimeofday(&bdd_end, NULL);
        eval_time += elapsedTime(bdd_start, bdd_end);

        std::random_device rd;
        std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
        std::uniform_real_distribution<double> dis(0.0, 1.0);

        int val = dis(gen) >= (prob / total_prob) ? 1 : 0;
        res.insert({q, val});

        // Update mtbdd based on the sampled outcome
        if (val) {
            mtbdd = adapter.set_one(mtbdd, q);
            total_prob = total_prob - prob;
        } else {
            mtbdd = adapter.set_zero(mtbdd, q);
            total_prob = prob;
        }
        // remove addvars
        adapter.remove_var(q);
    }

    std::string res_str;
    for (int i=0; i<out.size(); i++) {
        res_str += std::to_string(res[out[i]]);
    }

    return res_str;
}

template <typename adapter_t>
std::string sample_circuit_output(const tensor_network& t) {

    sum_power sop(t);
    std::vector<variable> in = t.get_start_vars();
    std::vector<variable> out = t.get_end_vars();
    std::map<variable, int> res;
    sop.set_zeros(in);
    adapter_t adapter;

    for (auto q : out) {
        sum_power sop_copy(sop);
        sop_copy.set_values(res);
        auto prob = sop_copy.norm_squared(adapter);

        std::random_device rd;
        std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
        std::uniform_real_distribution<double> dis(0.0, 1.0);

        int val = dis(gen) >= prob ? 1 : 0;
        res.insert({q, val});
    }

    std::string res_str;
    for (int i=0; i<out.size(); i++) {
        res_str += std::to_string(res[out[i]]);
    }

    return res_str;
}

template <typename adapter_t>
std::string sample_circuit_output(const std::string& filename) {
    tensor_network t = from_qasm(filename, gate_config_file); // TODO: omega;
    return sample_circuit_output_new<adapter_t>(t);
}

template <typename adapter_t>
std::string sample_circuit_output(const std::string& filename, int repeats) {

    tensor_network t = from_qasm(filename, gate_config_file); // TODO: omega;

    std::map<std::string, int> res;

    for (int i = 0; i < repeats; ++i) {
        res[sample_circuit_output_new<adapter_t>(t)]++;
    }

    std::string res_str;
    for(auto&& [key, val] : res) {
        res_str += key + ": " + std::to_string(val) + "\n";
    }

    return res_str;
}

template <typename adapter_t>
double trace(const tensor_network& t) {
    adapter_t adapter;
    return trace(adapter, t);
}

template <typename adapter_t>
bool equivalence_checking(std::string filename_a, std::string filename_b) {
    tensor_network ta = from_qasm(filename_a, gate_config_file);
    tensor_network tb = from_qasm(filename_b, gate_config_file);
    auto qa = ta.get_num_qubit();
    auto qb = tb.get_num_qubit();

    if (qa != qb) { return false; }

    auto ina = ta.get_start_vars();
    auto inb = tb.get_start_vars();
    auto outa = ta.get_end_vars();
    auto outb = tb.get_end_vars();
    std::vector<int> same_inout_b;
    for (int i = 0; i < inb.size(); ++i) {
        if (inb[i] == outb[i]) {
            same_inout_b.push_back(i);
        }
    }    
    sum_power sopa(ta);
    sum_power sopb(tb);
    adapter_t adapter;
    int na = sopa.num_vars();

    sopb.negate();
    sopb.shift_vars(na);
    shift_all_vars(inb, na);
    shift_all_vars(outb, na);
    sopb.subst_vars(outb, outa);
    sopb.subst_vars(inb, ina);
    sopa = sopa + sopb;
    for (auto link_qubit_index: same_inout_b) {
        sopa.subst_var_only(outa[link_qubit_index], ina[link_qubit_index]);
    }
    sopa.all_sum_vars();
    sopa.combine_terms(); // TODO: combine_terms may have a bug so that sum_vars are a superset of vars.
                          // when GHZ/4.qasm mismatched in equiv checking

    struct timeval bdd_start, bdd_end;
    gettimeofday(&bdd_start, NULL);

    sopa.counting_identity_simplify();
    auto factor_count = sopa.get_factor_count();
    auto nvars = sopa.get_nvars();

    sopa.set_orders(adapter);
    auto mtbdd = sopa.to_mtbdd_bin(adapter);
    gettimeofday(&bdd_end, NULL);
    bdd_time += elapsedTime(bdd_start, bdd_end);
    init_node_cnt = adapter.get_node_count(mtbdd);
    peak_node_cnt = adapter.get_peak_count();

    gettimeofday(&bdd_start, NULL);
    bool ret;
    if (omega == 2) {
        ret = adapter.check_norm_2(mtbdd, factor_count + 2 * qa);
    } else if (omega == 8) {
        ret = adapter.check_norm_8(mtbdd, factor_count + 2 * qa);
    } else {
        ERROR("Modulus %d not supported\n", omega);
        EXIT(-1);
    }
    gettimeofday(&bdd_end, NULL);
    eval_time += elapsedTime(bdd_start, bdd_end);

    return ret;

}

// The current implementation requires that no swap of variables are used.
template <typename adapter_t>
double trace(adapter_t &adapter, const tensor_network& t) {
    sum_power sop(t);
    sop.subst_vars(t.get_end_vars(), t.get_start_vars());
    return sop.evaluate<adapter_t>(adapter).real();
}

template <typename adapter_t>
void simulate_circuit(int argc, char **argv) {

    bool should_exit = parse_input(argc, argv);
    if (should_exit) {
        exit(-1);
    }

    std::cout << "FeynmanDD Simulation using " << adapter_t::NAME << " and " << M << "M memory:";
    std::cout << std::endl << std::endl;

    struct timeval start_t, end_t;

    gettimeofday(&start_t, NULL);
    order_time = 0.0;
    bdd_time = 0.0;
    mod_plus_time = 0.0;
    eval_time = 0.0;

    switch (command_type) {
        case SINGLE:
            for(auto filename : input_files) {
                std::cout << "    Working on file '" << filename << "'." << std::endl;

                auto amp = zero_amplitude<adapter_t>(filename);

                std::cout << "    The amplitude from zero to zero is [";

                if (fabs(amp.imag()) < std::numeric_limits<long double>::epsilon() && fabs(amp.real()) < std::numeric_limits<long double>::epsilon())
                {
                    std::cout << "0";
                }

                if (fabs(amp.real()) > std::numeric_limits<long double>::epsilon())
                {
                    std::cout << amp.real();
                }

                if (fabs(amp.imag()) > std::numeric_limits<long double>::epsilon())
                {
                    std::cout << (amp.imag() > 0 && fabs(amp.real()) > std::numeric_limits<long double>::epsilon() ? "+" : "") << amp.imag() << "i";
                }
                std::cout << "]." << std::endl << std::endl;
            }
            break;
        case FULL: break;
        case SAMPLE: 
            for(auto filename : input_files) {
                std::cout << "Working on file '" << filename << "'." << std::endl;
                std::cout << "Sampled output\n" << sample_circuit_output<adapter_t>(filename) << std::endl;
            }
            break;
        case EQUAL:
        {
            if (input_files.size() != 2) {
                std::cerr << "You need two input circuit files to decide the equivalence." << std::endl;
                exit(-1);
            }

            std::cout << "Working on files '" << input_files[0] << "' and '" <<
                input_files[1] << "'." << std::endl;
            bool eq = equivalence_checking<adapter_t>(input_files[0], input_files[1]);
            std::cout << "The circuits are " << (eq ? "" : "NOT ") << "equivalent." << std::endl;
            break;
        }
        default:
            ERROR("  Invalide type!\n");
            exit(-1);
    }
    gettimeofday(&end_t, NULL);
    double total_time = elapsedTime(start_t, end_t);

    // std::cout<<"Init Node Count: "<<init_node_cnt<<"\n";
    // std::cout<<"Init Peak Count: "<<peak_node_cnt<<"\n";
    std::cout<<"Total Runtime: "<<total_time<<"s\n";
    std::cout<<"    Main Algo: "<<total_time - order_time<<"s\n";
    std::cout<<"    Build MTBDD: "<<bdd_time<<"s\n";
    std::cout<<"    Mod Plus: "<<mod_plus_time<<"s\n";
    std::cout<<"    Evaluate: "<<eval_time<<"s\n";
    std::cout<<"    Ordering: "<<order_time<<"s\n";

    std::cout << "Peak memory usage by main algo: " << getPeakRSS() << " bytes" << std::endl;

    std::cout << "Peak memory usage by ordering: " << getPeakRSS(RUSAGE_CHILDREN) << " bytes" << std::endl;


}
