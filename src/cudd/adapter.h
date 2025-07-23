#include <gmp.h>
#include <gmpxx.h>
#include <math.h>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <string>

#include "cudd.h"
#include "cuddInt.h"
#include "cuddObj.hh"

extern int omega;  // Global variable for the modulus

// Copied here for compile from cudd
#define FREE(obj) (free(obj), (obj) = 0)

#define MP_FREE(pm) (mpz_clear(pm), FREE(pm))

DdNode *Cudd_addModPlus(DdManager *dd, DdNode **f, DdNode **g) {
    DdNode *res;
    DdNode *F, *G;
    CUDD_VALUE_TYPE value;

    F = *f;
    G = *g;
    if (F == DD_ZERO(dd))
        return (G);
    if (G == DD_ZERO(dd))
        return (F);
    if (cuddIsConstant(F) && cuddIsConstant(G)) {
        value = cuddV(F) + cuddV(G);
        if (value >= (CUDD_VALUE_TYPE)omega - 0.1) {
            value -= (CUDD_VALUE_TYPE)omega;
        }
        res = cuddUniqueConst(dd, value);
        return (res);
    }
    if (F > G) { /* swap f and g */
        *f = G;
        *g = F;
    }
    return (NULL);
}

// Copied here for compile from cudd, switched to GMP style
static enum st_retval cuddGmpStCountfree(void *key, void *value, void *arg) {
    mpz_ptr d = (mpz_ptr)value;
    (void)key; /* avoid warning */
    (void)arg; /* avoid warning */
    MP_FREE(d);
    return (ST_CONTINUE);
}

// Adapted from cuddApaCountMintermAux, switched to GMP style
static mpz_ptr cuddGmpCountByValueAux(DdManager const *manager,
                                      DdNode *node,
                                      mpz_t mmax,
                                      mpz_t mmin,
                                      st_table *table,
                                      CUDD_VALUE_TYPE target) {
    DdNode *Nt, *Ne;
    mpz_ptr mint, mint1, mint2;

    if (cuddIsConstant(node)) {
        int singleRef = Cudd_Regular(node)->ref == 1;
        if (fabs(cuddV(node) - target) > 0.1) {  // Need to delete the background part
            if (singleRef) {
                mint = (mpz_ptr)malloc(sizeof(mpz_t));
                if (mint == NULL) {
                    return NULL;
                }
                mpz_init(mint);
                mpz_set(mint, mmin);
                return mint;
            } else {
                return mmin;
            }
        } else {
            if (singleRef) {
                mint = (mpz_ptr)malloc(sizeof(mpz_t));
                if (mint == NULL) {
                    return NULL;
                }
                mpz_init(mint);
                mpz_set(mint, mmax);
                return mint;
            } else {
                return mmax;
            }
        }
    }

    if (node->ref > 1 && st_lookup(table, node, (void **)&mint)) {
        return mint;
    }

    Nt = cuddT(node);
    Ne = cuddE(node);

    mint1 = cuddGmpCountByValueAux(manager, Nt, mmax, mmin, table, target);
    if (mint1 == NULL) {
        return (NULL);
    }

    mint2 = cuddGmpCountByValueAux(manager, Cudd_Regular(Ne), mmax, mmin, table, target);
    if (mint2 == NULL) {
        if (Nt->ref == 1)
            MP_FREE(mint1);
        return (NULL);
    }

    mint = (mpz_ptr)malloc(sizeof(mpz_t));
    if (mint == NULL) {
        if (Nt->ref == 1)
            MP_FREE(mint1);
        if (Cudd_Regular(Ne)->ref == 1)
            MP_FREE(mint2);
        return (NULL);
    }

    mpz_init(mint);
    if (Cudd_IsComplement(Ne)) {
        mpz_sub(mint, mmax, mint2);
        mpz_add(mint, mint1, mint);
    } else {
        mpz_add(mint, mint1, mint2);
    }
    mpz_tdiv_q_2exp(mint, mint, 1);

    /* If the refernce count of a child is 1, its minterm count
    ** hasn't been stored in table.  Therefore, it must be explicitly
    ** freed here. */
    if (Nt->ref == 1)
        MP_FREE(mint1);
    if (Cudd_Regular(Ne)->ref == 1)
        MP_FREE(mint2);

    if (node->ref > 1) {
        if (st_insert(table, node, mint) == ST_OUT_OF_MEM) {
            MP_FREE(mint);
            return NULL;
        }
    }
    return mint;
}

// Adapted from cuddApaCountMinterm, switched to GMP style
void Cudd_GmpCountByValue(DdManager const *manager,
                          DdNode *node,
                          int nvars,
                          CUDD_VALUE_TYPE target,
                          mpz_t count) {
    mpz_t mmax, mmin;
    st_table *table;
    mpz_ptr i;

    mpz_init(mmax);
    mpz_ui_pow_ui(mmax, 2, nvars);
    mpz_init(mmin);
    mpz_set_ui(mmin, 0);
    table = st_init_table(st_ptrcmp, st_ptrhash);
    if (table == NULL) {
        mpz_clear(mmax);
        mpz_clear(mmin);
        return;
    }

    i = cuddGmpCountByValueAux(manager, Cudd_Regular(node), mmax, mmin, table, target);

    if (i == NULL) {
        mpz_clear(mmax);
        mpz_clear(mmin);
        st_foreach(table, cuddGmpStCountfree, NULL);
        st_free_table(table);
        return;
    }
    if (Cudd_IsComplement(node)) {
        mpz_sub(count, mmax, i);
    } else {
        mpz_set(count, i);
    }

    mpz_clear(mmax);
    mpz_clear(mmin);
    st_foreach(table, cuddGmpStCountfree, NULL);
    st_free_table(table);
    if (Cudd_Regular(node)->ref == 1)
        MP_FREE(i);
}

unsigned int cudd_cachesize(int varcount) {
    const size_t number_of_buckets = CUDD_UNIQUE_SLOTS * varcount;

    constexpr size_t sizeof_DdSubtable = 8 + 9 * 4 + 8;
    const size_t buckets_bytes = number_of_buckets * sizeof_DdSubtable;

    const size_t bytes_remaining = static_cast<size_t>(M) * 1024u * 1024u - buckets_bytes;

    // CUDD may increase the number of buckets, thereby decreasing the space
    // left for nodes. This will skew the ratio in favour of the cache, but this
    // is the best we can do.
    constexpr size_t sizeof_DdNode = 2 * 4 + 3 * 8;
    constexpr size_t sizeof_DdCache = 4 * 8;

    // We need to maximise x and y in the following system of inequalities:
    //              24x + 32y <= M , x = y * CACHE_RATIO
    const size_t x =
        bytes_remaining / ((sizeof_DdNode * CACHE_RATIO + sizeof_DdCache) / CACHE_RATIO);
    const size_t y = x / CACHE_RATIO;

    return y;
}

unsigned long cudd_memorysize() {
    constexpr size_t CUDD_MAX = std::numeric_limits<unsigned long>::max();
    return std::min(static_cast<size_t>(M), CUDD_MAX / (1024 * 1024)) * 1024 * 1024;
}

class cudd_adapter {
  protected:
    Cudd __mgr;

  protected:
    cudd_adapter(const int bdd_varcount, const int zdd_varcount)
        : __mgr(bdd_varcount,
                zdd_varcount,
                CUDD_UNIQUE_SLOTS,
                cudd_cachesize(bdd_varcount + zdd_varcount),
                cudd_memorysize()) {}

    ~cudd_adapter() {} /* Do nothing */

  public:
    inline size_t allocated_nodes() { return __mgr.ReadKeys(); }

    void print_stats() {
        INFO("\nCUDD Statistics:\n");

        INFO("   Table:\n");
        INFO("   | peak node count:     %zu\n", __mgr.ReadPeakNodeCount());
        INFO("   | node count (bdd):    %zu\n", __mgr.ReadNodeCount());
        INFO("   | keys:                %u\n", __mgr.ReadKeys());
        INFO("   | dead:                %u\n", __mgr.ReadDead());

        INFO("   Garbage Collections:\n");
        INFO("   | runs:                %u\n", __mgr.ReadGarbageCollections());
        INFO("   | time (ms):           %zu\n", __mgr.ReadGarbageCollectionTime());
    }
};

class cudd_add_adapter : public cudd_adapter {
  public:
    inline static const std::string NAME = "CUDD [ADD]";
    typedef ADD dd_t;
    typedef ADD build_node_t;
    std::map<int, dd_t> add_vars;
    std::map<int, int> add_vars_index;
    int index_counter;
    std::vector<int> order;   // Is this used?

  public:
    cudd_add_adapter() : cudd_adapter(0, 0) {
        __mgr.AutodynDisable(); // Disable dynamic ordering
        __mgr.AutodynEnable(CUDD_REORDER_SIFT);
        index_counter = 0;
    }

  public:
    inline int get_nvars() { return add_vars.size(); }

    inline void info() const { __mgr.info(); }

    inline dd_t add_const(int val) { return __mgr.constant(val); }

    inline dd_t leaf(int val) { return __mgr.constant(val); }

    void intro_vars(const std::vector<variable>& v) {
        for (auto var : v)
            intro_var(var);
    }

    void intro_var(int label) {
        if (add_vars.find(label) == add_vars.end()) {
            add_vars.insert({label, __mgr.addVar(index_counter)});
            order.push_back(label);
            add_vars_index.insert({label, index_counter});
            index_counter++;
        }
    }

    void remove_var(int label) {
        add_vars.erase(label);
    }

    inline dd_t ithvar(int label) {
        return add_vars[label];
    }

    inline int get_node_count(const dd_t &dd) { return dd.nodeCount(); }

    inline int get_peak_count() { return Cudd_ReadPeakLiveNodeCount(__mgr.getManager()); }

    inline dd_t ite(const dd_t &i, const dd_t &t, const dd_t &e) { return i.Ite(t, e); }

    inline dd_t set_zero(const dd_t &f, int x) {
        return f.Compose(add_const(0), add_vars_index[x]);
    }

    inline dd_t set_one(const dd_t &f, int x) {
        return f.Compose(add_const(1), add_vars_index[x]);
    }

    inline int size() { return __mgr.ReadSize(); }

    inline dd_t mod_plus(const dd_t &a, const dd_t &b) {
        DdManager *mgr = __mgr.getManager();
        DdNode *result = Cudd_addApply(mgr, Cudd_addModPlus, a.getNode(), b.getNode());
        return dd_t(__mgr, result);
    }

    inline void count(const dd_t &b, CUDD_VALUE_TYPE target, int nvars, mpz_t counter) {
        DdManager *mgr = __mgr.getManager();
        Cudd_GmpCountByValue(mgr, b.getNode(), nvars, target, counter);
    }

    inline std::string count_as_string(const dd_t &b, CUDD_VALUE_TYPE target, int nvars) {
        mpz_t counter;
        mpz_init(counter);
        count(b, target, nvars, counter);
        std::string ret = mpz_get_str(NULL, 10, counter);
        mpz_clear(counter);
        return ret;
    }

    inline void export_to_dot(const dd_t &b,const char *filename) {
        FILE *fp = fopen(filename, "w");
        if (fp == NULL) {
            std::cerr << "Error: Cannot open file " << filename << std::endl;
            return;
        }
        try {
            const std::vector<dd_t>& nodes{b};
            __mgr.DumpDot(nodes, NULL, NULL, fp);
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
        fclose(fp);
    }

    // Exact version for mod 2, return true if the dd has norm 1
    bool check_norm_2(dd_t mtbdd, int factor_count) {
        auto r = omega;

        assert(r == 2);

        mpz_t counters[r];
        mpz_t sum;

        for (int target = 0; target < r; ++target) {
            mpz_init(counters[target]);
            count(mtbdd, target, get_nvars(), counters[target]);
        }
        mpz_init(sum);
        mpz_sub(sum, counters[0], counters[1]);
        mpz_mul(sum, sum, sum);
        mpz_ui_pow_ui(counters[0], 2, factor_count);
        bool ret = mpz_cmp(sum, counters[0]) == 0;

        for (int target = 0; target < r; ++target) {
            mpz_clear(counters[target]);
        }
        mpz_clear(sum);
        return ret;
    }

    // Exact version for mod 8
    bool check_norm_8(dd_t mtbdd, int factor_count) {
        auto r = omega;

        assert(r == 8);

        mpz_t counters[r];
        mpz_t sum[r/2];

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

        mpz_t cross;
        mpz_t tmp;
        mpz_init(cross);
        mpz_init(tmp);
        mpz_sub(tmp, sum[1], sum[3]);
        mpz_mul(tmp, tmp, sum[2]);
        mpz_add(cross, sum[1], sum[3]);
        mpz_mul(cross, cross, sum[0]);
        mpz_add(cross, cross, tmp);
        bool ret;
        if (mpz_cmp_ui(cross, 0) != 0) {
            ret = false; // cross term is not correct
        } else {
            mpz_set_ui(cross, 0);
            for (int target = 0; target < r / 2; ++target) {
                mpz_mul(tmp, sum[target], sum[target]);
                mpz_add(cross, cross, tmp);
            }
            mpz_ui_pow_ui(tmp, 2, factor_count);
            ret = mpz_cmp(cross, tmp) == 0;
        }

        for (int target = 0; target < r / 2; ++target) {
            mpz_clear(sum[target]);
        }
        mpz_clear(cross);
        mpz_clear(tmp);
        return ret;
    }

    std::complex<long double> evaluate(dd_t mtbdd, int factor_count) {
        auto r = omega;

        mpz_t counters[r];
        mpz_t sum[r/2];

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
        #ifdef BENCHMARK_TIME
         std::cout << "[evaluate], end" << std::endl;
        #endif
        return res_complex;

    } 
};
