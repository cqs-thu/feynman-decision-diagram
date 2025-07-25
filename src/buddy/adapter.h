#include <bdd.h>

////////////////////////////////////////////////////////////////////////////////
/// Initialisation of BuDDy. The size of each node in the unique table is 6*4 =
/// 24 bytes (BddNode in kernel.h) while each cache entry takes up 4*4 = 16
/// bytes (BddCacheData in cache.h).
///
/// So, the memory in bytes occupied when given NODE_SLOTS and CACHE_SLOTS is
///
///                       24 * NODE_SLOTS + 16 * CACHE_SLOTS
///
/// - bdd_init:
///     We initialise BuDDy with a unique table of some number of nodes and a
///     cache with a set number of entries. The nodetable may grow, if need be
///     (except if something else is specified).
///
///     The initial size of the nodetable is in fact not the given table size,
///     but rather the smallest prime number larger than the given value.
///
/// - bdd_setmaxincrease:
///     The amount the original unique table is allowed to be increased during
///     garbage collection. If it is set to 0, then you fix the current size.
///
/// - bdd_setmaxnodesize
///     Sets the maximum number of nodes in the nodetable.
///
/// - bdd_setcacheratio:
///     Allows the cache to grow in size together with the nodetable. This
///     specifies the ratio between the node table and the cache. If it is not
///     called, then the cache is of a fixed size.
///
/// - bdd_setvarnum:
///     Declare the number of variables to expect to be used.
////////////////////////////////////////////////////////////////////////////////

// memory ceiling for BuDDy is MAX_INT.
constexpr size_t MAX_INT = 2147483647;

struct buddy_init_size
{
  int node_size;
  int cache_size;
};

buddy_init_size compute_init_size()
{
  // We need to maximise x and y in the following system of inequalities:
  //              24x + 16y <= M , x = y * CACHE_RATIO
  const size_t memory_bytes = static_cast<size_t>(M) * 1024 * 1024;
  const size_t x = memory_bytes / ((24u * CACHE_RATIO + 16u) / CACHE_RATIO);
  const size_t y = x / CACHE_RATIO;

  return { std::min(x, MAX_INT), std::min(y, MAX_INT / CACHE_RATIO) };
}

class buddy_bdd_adapter
{
public:
  inline static const std::string NAME = "BuDDy [BDD]";
  typedef bdd dd_t;
  typedef bdd build_node_t;

private:
  dd_t _latest_build;

  // Init and Deinit
public:
  buddy_bdd_adapter(int varcount)
  {
#ifndef BDD_BENCHMARK_GRENDEL
    const buddy_init_size init_size = compute_init_size();
    bdd_init(init_size.node_size, init_size.cache_size);

    // Set cache ratio if table changes in size. This is disabled, since the
    // table size is fixed below.
    // bdd_setcacheratio(CACHE_RATIO);

    // Fix table to current initial size. BuDDy chooses a nodetable size the
    // closest prime BIGGER than the given number. This means, we cannot fix the
    // size with 'bdd_setmaxnodenum'. So, we must instead set it to never
    // increase.
    //
    // TODO: Find the largest prime smaller than the computed number of nodes?
    bdd_setmaxincrease(0);
#else
    bdd_init(MAX_INT, MAX_INT / CACHE_RATIO);
#endif

    bdd_setvarnum(varcount);

    // Disable default gbc_handler
    bdd_gbc_hook(NULL);

    // Disable dynamic variable reordering
    bdd_disable_reorder();

    _latest_build = leaf_false();
  }

  ~buddy_bdd_adapter()
  {
    bdd_done();
  }

private:
  template<typename IT>
  inline bdd make_cube(IT rbegin, IT rend)
  {
    bdd res = leaf_true();
    while (rbegin != rend) {
      res = bdd_ite(bdd_ithvar(*(rbegin++)), res, leaf_false());
    }
    return res;
  }

  // BDD Operations
public:
  inline bdd leaf(bool val)
  { return val ? leaf_true() : leaf_false(); }

  inline bdd leaf_true()
  { return bddtrue; }

  inline bdd leaf_false()
  { return bddfalse; }

  inline bdd ithvar(int label)
  { return bdd_ithvar(label); }

  inline bdd nithvar(int label)
  { return bdd_nithvar(label); }

  inline bdd negate(const bdd &b)
  { return bdd_not(b); }

  inline bdd ite(const bdd &i, const bdd &t, const bdd &e)
  { return bdd_ite(i,t,e); }

  inline bdd exists(const bdd &b, int label)
  { return bdd_exist(b, bdd_ithvar(label)); }

  template<typename IT>
  inline bdd exists(const bdd &b, IT rbegin, IT rend)
  { return bdd_exist(b, make_cube(rbegin, rend)); }

  inline bdd forall(const bdd &b, int label)
  { return bdd_forall(b, bdd_ithvar(label)); }

  template<typename IT>
  inline bdd forall(const bdd &b, IT rbegin, IT rend)
  { return bdd_forall(b, make_cube(rbegin, rend)); }

  inline uint64_t nodecount(const bdd &b)
  { return bdd_nodecount(b); }

  inline uint64_t satcount(const bdd &b)
  { return bdd_satcount(b); }

  inline std::vector<std::pair<int, char>>
  pickcube(const bdd &b)
  {
    std::vector<std::pair<int, char>> res;
    bdd sat = bdd_satone(b);

    while (sat != bddfalse && sat != bddtrue) {
      const int var = bdd_var(sat);
      const bdd sat_low = bdd_low(sat);
      const bdd sat_high = bdd_high(sat);

      const bool go_high = sat_high != bddfalse;
      if (sat_low == sat_high) {
        res.push_back({ var, '2' });
      } else if (go_high) {
        res.push_back({ var, '1' });
      } else { // !go_high
        res.push_back({ var, '0' });
      }
      sat = go_high ? sat_high : sat_low;
    }
    return res;
  }

  // BDD Build Operations
public:
  inline bdd build_node(const bool value)
  {
    const bdd res = value ? leaf_true() : leaf_false();
    if (_latest_build == leaf_false()) { _latest_build = res; }
    return res;
  }

  inline bdd build_node(const int label, const bdd &low, const bdd &high)
  {
    _latest_build = ite(bdd_ithvar(label), high, low);
    return _latest_build;
  }

  inline bdd build()
  {
    const bdd res = _latest_build;
    _latest_build = leaf_false(); // <-- Reset and free builder reference
    return res;
  }

  // Statistics
public:
  inline size_t allocated_nodes()
  { return bdd_getnodenum(); }

  void print_stats()
  {
    INFO("\nBuDDy statistics:\n");

    bddStat stats;
    bdd_stats(&stats);

    INFO("   Table:\n");
    INFO("   | total produced:      %zu\n", stats.produced);

    // Commented lines are only available if 'CACHESTATS' flag is set
    // bddCacheStat cache_stats;
    // bdd_cachestats(&cache_stats);

    // INFO(" | | access:              %zu\n", cache_stats.uniqueAccess);
    // INFO(" | | hits:                %zu\n", cache_stats.uniqueHit);
    // INFO(" | | miss:                %zu\n", cache_stats.uniqueMiss);
    // INFO(" | Cache:\n");
    // INFO(" | | hits:                %zu\n", cache_stats.opHit);
    // INFO(" | | miss:                %zu\n", cache_stats.opMiss);

    INFO("   Garbage Collections:   %u\n",  stats.gbcnum);
  }
};
