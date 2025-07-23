#include <sylvan_obj.hpp>
#include <sylvan.h>
#include <sylvan_int.h>
#include <avl.h>
#include <sylvan_mtbdd.h>
#include <sylvan_mtbdd_int.h>
#include <gmp.h>
#include <gmpxx.h>
#include <iostream>

extern int omega; // The global variable for the module

using namespace sylvan;

////////////////////////////////////////////////////////////////////////////////
/// Initialisation of Sylvan.
///
/// From 'sylvan_commons.h' we know that every node takes up 24 bytes of memory
/// and every operation cache entry takes up 36 bytes.
///
/// Lace initialisation
/// - lace_start:             Initializes LACE given the number of threads and
///                           the size of the task queue.
///
/// Sylvan initialisation:
///   Nodes table size: 24 bytes * nodes
///   Cache table size: 36 bytes * cache entries
///
/// - sylvan_set_limit:       Set the memory limit, the (exponent of the) ratio
///                           between node table and cache, and lastly make the
///                           table sizes be as big as possible.
///
/// - sylvan_set_granularity: 1 for "use cache for every operation".
////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

TASK_DECL_2(MTBDD, mtbdd_mod_plus, MTBDD*, MTBDD*);

TASK_IMPL_2(MTBDD, mtbdd_mod_plus, MTBDD*, pa, MTBDD*, pb) {

    MTBDD a = *pa, b = *pb;
    if (a == mtbdd_false) return b;
    if (b == mtbdd_false) return a;

    mtbddnode_t na = MTBDD_GETNODE(a);
    mtbddnode_t nb = MTBDD_GETNODE(b);

    if (mtbddnode_isleaf(na) && mtbddnode_isleaf(nb)) {
        uint64_t val_a = mtbddnode_getvalue(na);
        uint64_t val_b = mtbddnode_getvalue(nb);
        if (mtbddnode_gettype(na) == 0 && mtbddnode_gettype(nb) == 0) {
            // both integer
            int64_t c = *(int64_t*)(&val_a) + *(int64_t*)(&val_b);
            if (c >= omega) { c -= omega; }
            return mtbdd_int64(c);
        } else {
            assert(0);
        }
    }
    // if (mtbdd_isleaf(a) && mtbdd_isleaf(b)) {
    //     assert(mtbdd_gettype(a) == 0 && mtbdd_gettype(b) == 0);
    //     uint64_t val_a = mtbdd_getvalue(a);
    //     uint64_t val_b = mtbdd_getvalue(b);
    //     int64_t c = (val_a + val_b) % omega;
    //     return mtbdd_int64(c);
    // }

    if (a < b) {
        *pa = b;
        *pb = a;
    }

    return mtbdd_invalid;
}

TASK_DECL_3(mpz_t*, mtbdd_count_by_value_gmp, MTBDD, size_t, int);

TASK_IMPL_3(mpz_t*, mtbdd_count_by_value_gmp, MTBDD, dd, size_t, nvars, int, target)
{
    if (mtbdd_isleaf(dd)) {
        // test if 0
        mpz_t* ret = (mpz_t*)malloc(sizeof(mpz_t));
        mpz_init(*ret);
        
        mtbddnode_t dd_node = MTBDD_GETNODE(dd);
        uint64_t dd_val = mtbddnode_getvalue(dd_node);
        if (dd_val == (uint64_t) target) {
            mpz_ui_pow_ui(*ret, 2, nvars);
        } else {

            mpz_set_ui(*ret, 0);
        }
        return ret;
    }

    /* Perhaps execute garbage collection */
    sylvan_gc_test();
    union {
        mpz_t* ret;
        uint64_t s;
    } hack;
    hack.ret = (mpz_t*)malloc(sizeof(mpz_t));
    mpz_init(*(hack.ret));
    // /* Consult cache */
    if (cache_get3(CACHE_BDD_SATCOUNT, dd, target, nvars, &hack.s)) {    // 借用SATCOUNT的id维护缓存，程序其他地方不能再使用SATCOUNT
        sylvan_stats_count(BDD_SATCOUNT_CACHED);
        return hack.ret;
    }
    SPAWN(mtbdd_count_by_value_gmp, mtbdd_gethigh(dd), nvars-1, target);
    mpz_t* low = CALL(mtbdd_count_by_value_gmp, mtbdd_getlow(dd), nvars-1, target);
    mpz_t* high = SYNC(mtbdd_count_by_value_gmp);
    mpz_add(*(hack.ret), *low, *high);
    if (cache_put3(CACHE_BDD_SATCOUNT, dd, target, nvars, hack.s)) {
        sylvan_stats_count(BDD_SATCOUNT_CACHEDPUT);
    }

    return hack.ret;
}

// TASK_DECL_3(double, mtbdd_count_by_value, MTBDD, size_t, int);

// TASK_IMPL_3(double, mtbdd_count_by_value, MTBDD, dd, size_t, nvars, int, target)
// {
//     if (mtbdd_isleaf(dd)) {
//         // test if 0
//         mtbddnode_t dd_node = MTBDD_GETNODE(dd);
//         uint64_t dd_val = mtbddnode_getvalue(dd_node);
//         if (dd_val == (uint64_t) target) {
//             return powl(2.0L, nvars);
//         } else {
//             return 0.0;
//         }
//     }

//     /* Perhaps execute garbage collection */
//     sylvan_gc_test();

//     union {
//         double d;
//         uint64_t s;
//     } hack;

//     /* Consult cache */
//     if (cache_get4(CACHE_BDD_PATHCOUNT, dd, 0, nvars, target,&hack.s)) {    // 借用PATHCOUNT的id维护缓存
//         sylvan_stats_count(BDD_PATHCOUNT_CACHED);
//         return hack.d;
//     }

//     SPAWN(mtbdd_count_by_value, mtbdd_gethigh(dd), nvars-1, target);
//     double low = CALL(mtbdd_count_by_value, mtbdd_getlow(dd), nvars-1, target);
//     hack.d = low + SYNC(mtbdd_count_by_value);

//     if (cache_put4(CACHE_BDD_PATHCOUNT, dd, 0, nvars, target,hack.s)) {
//         sylvan_stats_count(BDD_PATHCOUNT_CACHEDPUT);
//     }

//     return hack.d;
// }

// #define mtbdd_count_by_value(dd, nvars, target) RUN(mtbdd_count_by_value, dd, nvars, target)

#define mtbdd_count_by_value_gmp(dd, nvars, target) RUN(mtbdd_count_by_value_gmp, dd, nvars, target)

#ifdef __cplusplus
}
#endif /* __cplusplus */

size_t log2(size_t n) {
    size_t exp = 1u;
    size_t val = 2u;  // 2^1
    while (val < n) {
        val <<= 1u;
        exp++;
    }
    return exp;
}

class sylvan_mtbdd_adapter {
  public:
    inline static const std::string NAME = "Sylvan-MTBDD";
    typedef Mtbdd dd_t;
    int nvars;

    std::map<variable, dd_t> mtbdd_vars;
    std::vector<variable> order;    
    MtbddMap var_value_map;

  public:

    sylvan_mtbdd_adapter(int nvars) : nvars(nvars) {
        // std::cout <<"adapter construction begin..." << std::endl;
        int n_workers = 0; //auto-detect
        lace_start(n_workers, 0);

        const size_t memory_bytes = static_cast<size_t>(M) * 1024u * 1024u;

        // Init Sylvan
        sylvan_set_limits(
            memory_bytes,       // Set memory limit
            log2(CACHE_RATIO),  // Set (exponent) of cache ratio
            0);                 // Initialise unique node table to full size
        sylvan_set_granularity(1);
        sylvan_init_package();
        sylvan_init_mtbdd();
        // std::cout <<"adapter construction!" << std::endl;
    }

    sylvan_mtbdd_adapter() : sylvan_mtbdd_adapter(0) {
        
    }

    ~sylvan_mtbdd_adapter() {
        // std::cout <<"adapter deconstruction begin..." << std::endl;
        sylvan_quit();
        lace_stop();
        // std::cout <<"adapter deconstruction!" << std::endl;
    }

    inline void set_nvars(int n) { nvars = n; }

    inline int get_nvars() { 
        assert(nvars == mtbdd_var.size());
        return nvars; 
    }

    inline dd_t mod_plus(const dd_t &a, const dd_t &b) {
        return a.Apply(b, TASK(mtbdd_mod_plus));
    }

    inline dd_t leaf(int val) {return add_const(val); }

    inline dd_t ithvar(int label) { 
        return mtbdd_vars[label];
    }

    inline dd_t nithvar(int label) {return ~ithvar(label); }

    inline dd_t ite(const dd_t &i, const dd_t &t, const dd_t &e) {
        return i.Ite(t, e);
    }

    inline int size() {return nvars; }

    inline dd_t add_const(int val) { return Mtbdd::int64Terminal(val); }

    inline uint64_t nodecount(const dd_t &b) {
        return b.NodeCount() - 1;
    }

    inline int get_node_count(const dd_t &dd) { return dd.NodeCount() ; }

    inline int get_peak_count() {
        // TODO: how to get peak count in sylvan?
        return -1;
    }

    inline dd_t set_zero(dd_t &f, variable x) {
        var_value_map.put(x, mtbdd_false);
        return f.Compose(var_value_map);
    }

    inline dd_t set_one(dd_t &f, variable x) {
        var_value_map.put(x, mtbdd_true);
        return f.Compose(var_value_map);
    }

    inline void print_stats() {
        INFO("\n");
        sylvan_stats_report(stdout);
    }

    void ordered_vars(const std::vector<variable>& v) {
        for (auto var: v) {
            intro_var(var);
        }
    }

    void intro_var(variable label) {
        if (mtbdd_vars.find(label) == mtbdd_vars.end()) {
            mtbdd_vars[label] = Mtbdd::mtbddVar(label);
            order.push_back(label);
            nvars += 1;
        }
    }

    void remove_var(int label) {
        mtbdd_vars.erase(label);
        nvars -= 1;
    }

    inline void count(const dd_t &b, int target, int nvars, mpz_t counter) {
        mpz_t *cnt_gmp = mtbdd_count_by_value_gmp(b.GetMTBDD(), nvars, target);
        mpz_set(counter, *cnt_gmp);
        mpz_clear(*cnt_gmp);
        free(cnt_gmp);

        cache_clear();
    }

    // inline int count(const dd_t &b, int target) {

    //     // double cnt = mtbdd_count_by_value(b.GetMTBDD(), size(), target);
    //     // std::cout << std::endl <<  "sylnan-mtbdd count by orgin method for target" << target << ": " << cnt << std::endl;
    //     // cache_clear();

    //     mpz_t *cnt_gmp = mtbdd_count_by_value_gmp(b.GetMTBDD(), size(), target);
    //     //std::cout << std::endl <<  "sylnan-mtbdd count by gmp method for target" << target << ": " << mpz_get_d(*cnt_gmp) << std::endl;
    //     int cnt = mpz_get_d(*cnt_gmp);
    //     mpz_clear(*cnt_gmp);
    //     cache_clear();
    //     return cnt;
    // }

    inline std::string count_as_string(dd_t & b, int target, int _nvars) {
        mpz_t counter;
        mpz_init(counter);
        count(b, target, get_nvars(), counter);
        std::string ret = mpz_get_str(nullptr, 10, counter);
        mpz_clear(counter);
        return ret;
    }

    inline void export_to_dot(const dd_t &b, const char* filename) {
        // TODO: check visualize
    }

    std::complex<long double> evaluate(dd_t mtbdd, int factor_count) {
        auto r = omega;

        mpz_t counters[r];
        mpz_t sum[r/2];
        // std::cout << "adapter evaluate" << std::endl;
        for (int target = 0; target < r; ++target) {
            mpz_init(counters[target]);
            count(mtbdd, target, get_nvars(), counters[target]);
        }
        for (int target = 0; target < r / 2; ++target) {
            mpz_init(sum[target]);
            mpz_sub(sum[target], counters[target], counters[r / 2 + target]);
        }
        for (int target = 0; target < r; ++target) {
            mpz_clear(counters[target]);
        }
        mpf_t result_f[r/2];
        mpf_t hadamard_f;
        mpf_t factor_div_2_f;
        mpf_t one;

        for (int target = 0; target < r / 2; ++target) {
            mpf_init(result_f[target]);
        }
        mpf_init(hadamard_f);
        mpf_init(factor_div_2_f);
        mpf_init(one);

        mpf_t re, im;
        mpf_init(re);
        mpf_init(im);
        mpf_set_d(re, 0.0);
        mpf_set_d(im, 0.0);

        mpf_t cosine, sine;
        mpf_init(cosine);
        mpf_init(sine);

            // mpf_set_z(result_f, sum);
        for (int target = 0; target < r / 2; ++target) {
            mpf_set_z(result_f[target], sum[target]);
            mpf_set_d(cosine, cos(2 * PI * target / (long double)(r)));
            mpf_set_d(sine, sin(2 * PI * target / (long double)(r)));
            mpf_mul(cosine, result_f[target], cosine);
            mpf_mul(sine, result_f[target], sine);
            mpf_add(re, re, cosine);
            mpf_add(im, im, sine);
        }


        mpf_set_d(one, 1.0);
        int factor_div_2 = factor_count >> 1;
        mpf_set_ui(factor_div_2_f, factor_div_2);
        mpf_mul_2exp(hadamard_f, one, factor_div_2);
        // mpf_div(result_f, result_f, hadamard_f);
        mpf_div(re, re, hadamard_f);
        mpf_div(im, im, hadamard_f);
        if (factor_count % 2 == 1) {
            mpf_t sqrt_2, two;
            mpf_init(sqrt_2);
            mpf_init(two);
            mpf_set_d(two, 2.0);
            mpf_sqrt(sqrt_2, two);
            // mpf_div(result_f, result_f, sqrt_2);
            mpf_div(re, re, sqrt_2);
            mpf_div(im, im, sqrt_2);
            mpf_clear(sqrt_2);
            mpf_clear(two);
        }

        std::complex<long double> res_complex(mpf_get_d(re), mpf_get_d(im));

        for (int target = 0; target < r / 2; ++target) {
            mpz_clear(sum[target]);
        }
        // mpf_clear(result_f);
        for (int target = 0; target < r / 2; ++target) {
            mpf_clear(result_f[target]);
        }
        mpf_clear(hadamard_f);
        mpf_clear(factor_div_2_f);
        mpf_clear(one);
        mpf_clear(re);
        mpf_clear(im);
        mpf_clear(cosine);
        mpf_clear(sine);
        // std::cout <<"adapter evaluate done!" << std::endl;
        return res_complex;
    }
};

