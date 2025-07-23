#include "sum_power.h"
#include "tensor_network.h"

int main()
{
    // std::vector<int> vars1 = {1, 2, 3};
    // std::vector<int> vars2 = {3, 4, 5, 6};
    // std::vector<term> terms;
    // terms.push_back(term(1, 2, vars1));
    // terms.push_back(term(1, 2, vars2));
    // sum_power s(2, terms);
    // sum_power::num_var = 6;
    // s.print();

    // std::vector<int> vars3 = {1, 4, 5, 6};
    // std::vector<int> vars4 = {5, 6};
    // std::vector<term> terms2;
    // terms2.push_back(term(1, 2, vars3));
    // terms2.push_back(term(1, 2, vars4));
    // sum_power s2(2, terms2);
    // s2.print();

    // sum_power s3 = s + s2;
    // s3.print();

    auto res = from_qasm("../circuits/sample.qasm", "./gate_sets/default_2.json");
    res.print();

    sum_power s1 = sum_power(res);
    s1.set_sum_vars(std::vector<variable>({4}));
    s1.print();
    sum_power s2 = sum_power(res);
    s2.set_sum_vars(std::vector<variable>({2}));
    s2.print();
    sum_power s3 = s1 + s2;
    s3.print();
    return 0;
}
