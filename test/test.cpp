#include "../src/common.h"
#include "../src/sum_power.h"
#include "../src/tensor_network.h"
#include "../src/simulator.h"
#include "../src/cudd/adapter.h"
//#include "../src/sylvan/adapter.h"
#include <random>
#include <gtest/gtest.h>

using testing::Types;

template <typename adapter_t>
class FeynmanCoreTest : public ::testing::Test {
  public:
    void SetUp() override{};
    void TearDown() override{};
};

TYPED_TEST_SUITE_P(FeynmanCoreTest);

TYPED_TEST_P(FeynmanCoreTest, CheckQubitOrder) {
    tensor_network t = from_qasm("circuits/ghz.qasm", "gate_sets/default_2.json");
    sum_power sop(t);
    int nv = sop.num_vars();
    EXPECT_EQ(sop.get_sum_vars(), std::vector<variable>({5, 6, 7}));

    EXPECT_EQ(t.get_start_vars(), std::vector<int>({0, 1, 2, 3}));
    EXPECT_EQ(t.get_end_vars(), std::vector<int>({4, 8, 9, 10}));
    EXPECT_EQ(sop.get_var_by_qubit_order(), std::vector<std::vector<variable>>({{0, 4}, {1, 5, 8}, {2, 6, 9}, {3, 7, 10}}));


    EXPECT_EQ(sop.compute_qubit_order(), std::vector<variable>({0, 4, 1, 5, 8, 2, 6, 9, 3, 7, 10}));

    sop.set_zeros(t.get_start_vars());
    EXPECT_EQ(sop.compute_qubit_order(), std::vector<variable>({4, 5, 8, 6, 9, 7, 10}));

    EXPECT_EQ(sop.to_string(), "x4 * x5 + x4 * x6 + x4 * x7 + x5 * x8 + x6 * x9 + x7 * x10");
    sum_power sopc(sop);
    sopc.negate();
    EXPECT_EQ(sopc.to_string(), "x7 * x10 + x6 * x9 + x5 * x8 + x4 * x7 + x4 * x6 + x4 * x5");
    sopc.shift_sum_vars(nv);
    EXPECT_EQ(sopc.get_vars(), std::vector<int>({4, 8, 9, 10, 16, 17, 18}));
    EXPECT_EQ(sopc.to_string(), "x10 * x18 + x9 * x17 + x8 * x16 + x4 * x18 + x4 * x17 + x4 * x16");
    EXPECT_EQ(sopc.get_var_by_qubit_order(), std::vector<std::vector<variable>>({{4}, {8, 16}, {9, 17}, {10, 18}}));

    sop = sop + sopc;
    EXPECT_EQ(sop.to_string(), "x4 * x5 + x4 * x6 + x4 * x7 + x5 * x8 + x6 * x9 + x7 * x10 + x10 * x18 + x9 * x17 + x8 * x16 + x4 * x18 + x4 * x17 + x4 * x16");
    EXPECT_EQ(sop.get_var_by_qubit_order(), std::vector<std::vector<variable>>({{4}, {5, 8, 16}, {6, 9, 17}, {7, 10, 18}}));

    EXPECT_EQ(sop.compute_qubit_order(), std::vector<variable>({4, 5, 8, 16, 6, 9, 17, 7, 10, 18}));
}

TYPED_TEST_P(FeynmanCoreTest, NumVarAndNumTerm) {
    tensor_network t = from_qasm("benchmark/exp/google/is_v1_d10/inst_5x5_10_0.qasm", "./gate_sets/google.json");
    sum_power sop(t);

    EXPECT_EQ(sop.num_vars(), 123);
    EXPECT_EQ(sop.num_terms(), 342);
}

TYPED_TEST_P(FeynmanCoreTest, CheckChangeVarOrder) {
    tensor_network t = from_qasm("circuits/create/iswap.qasm", "./gate_sets/google.json");
    sum_power sop(t);

    EXPECT_EQ(sop.to_string(), "2 * x1 + 4 * x0 * x1 + 2 * x0 + 2 * x2 + 4 * x0 * x2 + 2 * x0");
    EXPECT_EQ(t.get_start_vars(), std::vector<int>({0, 1, 2}));
    EXPECT_EQ(t.get_end_vars(), std::vector<int>({1, 2, 0}));
}

TYPED_TEST_P(FeynmanCoreTest, CheckMod8) {
    tensor_network t = from_qasm("circuits/random_10qubit_0.qasm", "./gate_sets/T.json");
    
    // sort_setting = SORT_SETTING::SORT_BY_COTENGRA_TERM;
    auto z2z = zero_amplitude<TypeParam>(t);

    ASSERT_DOUBLE_EQ(z2z.real(), 0.0);
    ASSERT_DOUBLE_EQ(z2z.imag(), 0.0);

    t = from_qasm("circuits/htk_10.qasm", "./gate_sets/T.json");
    z2z = zero_amplitude<TypeParam>(t);
    ASSERT_DOUBLE_EQ(z2z.real(), -0.0024378128355074637);
    ASSERT_DOUBLE_EQ(z2z.imag(), -0.00808266643500933);
    sort_setting = SORT_SETTING::NO_SORT;
}


TYPED_TEST_P(FeynmanCoreTest, CheckQASMLoader) {
    tensor_network t = from_qasm("./circuits/h2.qasm", "./gate_sets/default_2.json");
    EXPECT_EQ(t.get_num_qubit(), 1);
    EXPECT_EQ(t.get_num_var(), 3);
}

TYPED_TEST_P(FeynmanCoreTest, CheckSumOfPowers) {
    tensor_network t = from_qasm("./circuits/h2.qasm", "./gate_sets/default_2.json");
    sum_power sop(t);
    EXPECT_EQ(sop.get_r(), 2);
    EXPECT_EQ(sop.num_vars(), 3);
    EXPECT_EQ(sop.num_terms(), 2);

    EXPECT_EQ(sop.to_string(), "x0 * x1 + x1 * x2");

    std::vector<int> vz = {0};

    std::map<variable, int> degree = sop.compute_degrees();
    EXPECT_EQ(degree[1], 2);
    EXPECT_EQ(degree[2], 1);

    sop.set_zeros(vz);
    EXPECT_EQ(sop.num_terms(), 1);
    EXPECT_EQ(sop.to_string(), "x1 * x2");

    sum_power sopa(t);
    sopa.shift_sum_vars(2); // Shift internal var indices by 2

    EXPECT_EQ(sopa.to_string(), "x0 * x3 + x2 * x3");

    std::vector<int> v = {3};
    sopa.set_ones(v);

    EXPECT_EQ(sopa.to_string(), "x0 + x2");

    {
        TypeParam adapter;

        auto dd = sopa.to_mtbdd_bin(adapter);
        EXPECT_EQ(adapter.count_as_string(dd, 0, sopa.get_nvars()), "2");
    }
    std::vector<int> v2 = {0, 2};
    sopa.set_ones(v2);

    EXPECT_EQ(sopa.to_string(), "0");

    sum_power sopb(t);

    TypeParam adapter2;
    sopb.set_orders(adapter2);
    auto dd2 = sopb.to_mtbdd_bin(adapter2); // Looks like we cannot assign it to dd
    EXPECT_EQ(adapter2.count_as_string(dd2, 0, sopb.get_nvars()), "6");
    EXPECT_EQ(adapter2.count_as_string(dd2, 1, sopb.get_nvars()), "2");
    EXPECT_EQ(adapter2.count_as_string(dd2, 0, sopb.get_nvars()), "6");


    tensor_network tsimp = from_qasm("./circuits/hhi.qasm", "./gate_sets/default_2.json");
    sum_power sopsimp(tsimp);
    sopsimp.counting_identity_simplify();
    EXPECT_EQ(sopsimp.to_string(), "0");
}

TYPED_TEST_P(FeynmanCoreTest, CheckSimulator) {


    tensor_network t = from_qasm("./circuits/ccx.qasm", "./gate_sets/default_2.json");
    EXPECT_EQ(t.get_num_qubit(), 3);
    EXPECT_EQ(t.get_num_var(), 5);

    sum_power sop(t);

    EXPECT_EQ(sop.to_string(), "x2 * x3 + x0 * x1 * x3 + x3 * x4");


    //sort_setting = SORT_SETTING::SORT_BY_COTENGRA_TERM;

    std::vector<int> b100({1,0,0});
    std::vector<int> b000({0,0,0});
    auto a100 = amplitude<TypeParam>(t, b100).real();
    auto a000 = amplitude<TypeParam>(t, b000).real();
    auto tr = trace<TypeParam>(t);
    ASSERT_DOUBLE_EQ(a100, 0.0);
    ASSERT_DOUBLE_EQ(a000, 1.0);
    ASSERT_DOUBLE_EQ(tr, 6.0); // tr CCX = 6

    EXPECT_EQ(sop.to_string(), "x2 * x3 + x0 * x1 * x3 + x3 * x4");
    {
        TypeParam adapter;
        sop.set_orders(adapter);
        auto dd = sop.to_mtbdd_bin(adapter);

        sort_setting = SORT_SETTING::NO_SORT;

        EXPECT_EQ(adapter.count_as_string(dd, 1, sop.get_nvars()), "8");
    }
    sop.set_zeros(t.get_variable_start());
    EXPECT_EQ(sop.to_string(), "x3 * x4");
    EXPECT_EQ(sop.num_vars(), 2);

    EXPECT_EQ((int)std::round(trace<TypeParam>(t)), 6);

    EXPECT_EQ((int)std::round(zero_amplitude<TypeParam>(t).real()), 1);

    tensor_network th1 = from_qasm("./circuits/ghz.qasm", "./gate_sets/default_2.json");
    ASSERT_DOUBLE_EQ(circuit_accept_probability<TypeParam>(th1), 0.5);
    std::string samp = sample_circuit_output_new<TypeParam>(th1);
    EXPECT_PRED1([](auto str) {return str == "0000" || str == "1111";}, samp);

    tensor_network iqp = from_qasm("./circuits/random_iqp-20-200.qasm", "./gate_sets/default_2.json");
    EXPECT_EQ(iqp.get_num_qubit(), 20);
    EXPECT_EQ(iqp.get_num_var(), 20);
    sum_power sop_iqp(iqp);
    TypeParam adapter;
    sop_iqp.set_orders(adapter);
    auto dd_iqp = sop_iqp.to_mtbdd_bin(adapter);
    EXPECT_LE(adapter.get_node_count(dd_iqp), 53381);

    dd_iqp = sop_iqp.to_mtbdd_plus(adapter);
    EXPECT_LE(adapter.get_node_count(dd_iqp), 53381);

    dd_iqp = sop_iqp.to_mtbdd_bin(adapter);
    EXPECT_LE(adapter.get_node_count(dd_iqp), 53381);
}

TYPED_TEST_P(FeynmanCoreTest, CheckSetTerms) {

    const int test_r = 24;

    std::vector<term> test_terms = {
        term(1, {1, 2}),
        term(1, {1, 3}),
        term(2, {1, 4}),
        term(3, {2, 3}),
        term(21, {1, 2}),
        term(21, {1, 2, 3, 4}),
        term(1, {1, 2}),
        term(5, {2}),
        term(21, {3}),
        term(1, {1}),
        term(4, {2, 3}),
        term(5, {4, 3, 2}),
        term(6, {2, 1, 4, 3}),
        term(2, {3, 2, 1}),
        term(23, {}),
        term(3, {3})
    };

    sum_power sop(test_r, test_terms);
    sop.set_ones({4});
    sop.combine_terms();
    EXPECT_EQ(sop.to_string(), "23 + 3 * x1 + 5 * x2 + 23 * x1 * x2 + x1 * x3 + 12 * x2 * x3 + 5 * x1 * x2 * x3");
    sop.set_zeros({2});
    EXPECT_EQ(sop.to_string(), "23 + 3 * x1 + x1 * x3");
}

TYPED_TEST_P(FeynmanCoreTest, CheckSimulator2) {
    tensor_network t = from_qasm("./circuits/create/is_v1_d5_inst_4x4_5_0.qasm", "./gate_sets/google.json");
    auto z2z = zero_amplitude<TypeParam>(t);
    ASSERT_NEAR(z2z.real(), -0.003334192932, 1e-8);
    ASSERT_NEAR(z2z.imag(), -0.008049453796, 1e-8);
}

TYPED_TEST_P(FeynmanCoreTest, CheckEquivalence) {
    const std::string origin_file = "./circuits/compare/mod5d2_70.qasm";
    const std::string opt_file = "./circuits/compare/mod5d2_70opt1.qasm";
    const std::string miss_file = "./circuits/compare/mod5d2_70_miss.qasm";
    const std::string rev_file = "./circuits/compare/mod5d2_70_rev.qasm";

    EXPECT_TRUE(equivalence_checking<TypeParam>(origin_file, opt_file));
    EXPECT_FALSE(equivalence_checking<TypeParam>(origin_file, miss_file));
    EXPECT_FALSE(equivalence_checking<TypeParam>(origin_file, rev_file));
}

REGISTER_TYPED_TEST_SUITE_P(FeynmanCoreTest,
                            CheckQubitOrder,
                            NumVarAndNumTerm,
                            CheckChangeVarOrder,
                            CheckMod8,
                            CheckQASMLoader,
                            CheckSumOfPowers,
                            CheckSimulator,
                            CheckSimulator2,
                            CheckSetTerms,
                            CheckEquivalence);

typedef Types<cudd_add_adapter> Adapters;
// typedef Types<sylvan_mtbdd_adapter> Adapters;

INSTANTIATE_TYPED_TEST_SUITE_P(All, FeynmanCoreTest, Adapters);
