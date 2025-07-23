#pragma once

#include <assert.h>
#include <algorithm>
#include <cmath>
#include <getopt.h>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <chrono>

// Global constants
constexpr size_t CACHE_RATIO = 64u;

// Initial size taken from CUDD defaults
constexpr size_t INIT_UNIQUE_SLOTS_PER_VAR = 256u;

// Timing
typedef std::chrono::steady_clock::time_point time_point;

inline time_point get_timestamp() {
    return std::chrono::steady_clock::now();
}

typedef unsigned long int time_duration;

inline unsigned long int duration_of(const time_point &before, const time_point &after) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count();
}

// Printing
#define FLUSH() { fflush(stdout); fflush(stderr); }
#define EXIT(e) { FLUSH(); exit(e); }

#define INFO(s, ...) { \
        fprintf(stdout, s, ##__VA_ARGS__); FLUSH() }
#define ERROR(s, ...) { \
        fprintf(stderr, s, ##__VA_ARGS__); FLUSH() }

void print_vector(const std::vector<int> &vec, std::string prompt = "") {
    std::cout << prompt << std::endl;
    for (auto &v : vec) {
        std::cout << v << ", ";
    }
    std::cout << std::endl << std::endl;
}

// Input parsing

enum TYPE {
    SINGLE, // 0, Compute the probability outputing 0^n
    FULL, // 1, Output the full state vector
    SAMPLE, // 2, Sample from the circuit output distribution
    PROBABILITY, // 3, Compute the probability of acceptance on the first qubit
    EQUAL // 4, Equivalence checking
};

int M = 128; /* MiB */
TYPE command_type = TYPE::SINGLE;
std::vector<std::string> input_files = {};
std::string gate_config_file = "gate_sets/default_2.json";
const std::string TERM_COMMAND = "term";
const std::string PATH_COMMAND = "path";
enum SORT_SETTING {
    NO_SORT, // 0, Qubit order, no simplification, best for linear network circuits
    SORT_BY_QUBIT_ORDER, // 1, Qubit order, simplification, best for BV, GHZ
    SORT_BY_GATE_ORDER, // 2, Gate sequence order induces a variable order, best for circuit of small number of qubits
    SORT_BY_QUBIT_SUM_TERM, // 3, Qubit order, induced term order, no simplification, best for google
    SORT_BY_COTENGRA_TERM, // 4, Run cotengra
    SORT_BY_COTENGRA_TREE, // 5: Only use for to_mtbdd_tree
};

SORT_SETTING sort_setting = SORT_SETTING::NO_SORT;
double cotengra_timeout_limit = -1.0;    // when this set to be negative, it means no timeout limit

bool parse_input(int &argc, char *argv[]) {
    bool exit = false;
    int c;

    opterr = 0;  // Squelch errors for non-common command-line arguments

    while ((c = getopt(argc, argv, "N:M:f:t:h:g:s:l:")) != -1) {
        try {
            switch (c) {
                case 'M':
                    M = std::stoi(optarg);
                    if (M == 0) {
                        ERROR("  Must specify positive amount of memory (-M)\n");
                        exit = true;
                    }
                    continue;

                case 'f': {
                    std::string file = optarg;
                    if (!file.empty()) {
                        input_files.push_back(file);
                    }
                    continue;
                }
                case 'g': {
                    std::string file = optarg;
                    if (!file.empty()) {
                        gate_config_file = file;
                    }
                    continue;
                }
                case 's': {
                    sort_setting = (enum SORT_SETTING) std::stoi(optarg);
                    continue;
                }
                case 'l': {
                    cotengra_timeout_limit = std::stod(optarg);
                    continue;
                }
                case 't':
                    command_type = (enum TYPE) std::stoi(optarg);
                    continue;

                case '?':  // All parameters not defined above will be overwritten to be the '?'
                           // character
                    ERROR("Undefined flag parameter used\n\n");
                    [[fallthrough]];  // Let the compiler know, that we intend to fall through to
                                      // 'h' case

                case 'h':
                    std::cout
                        << "Usage:  -flag       [default] Description" << std::endl
                        << std::endl
                        << "        -h                    Print this information" << std::endl
                        << "        -f FILENAME           Input file to run (use repeatedly for "
                           "multiple files)"
                        << std::endl
                        << "        -g FILENAME           JSON file containing gate set "
                        << std::endl
                        << "        -s SORT_SETTING   [" << (int)(sort_setting) << "]    Sorting setting"
                        << std::endl
                        << "        -l TIME     [" << cotengra_timeout_limit << "]    Timeout limit for cotengra"
                        << std::endl
                        << "        -M MiB      [128]     Amount of memory (MiB) to be dedicated "
                           "to the BDD package"
                        << std::endl
                        << "        -t TYPE     [0]       Type of task"
                        << std::endl;
                    return true;
            }
        } catch (std::invalid_argument const &ex) {
            ERROR("Invalid number: %s\n", ex.what());
            exit = true;
        } catch (std::out_of_range const &ex) {
            ERROR("Number out of range: %s", ex.what());
            exit = true;
        }
    }

    optind = 0;  // Reset getopt, such that it can be used again outside
    return exit;
}
