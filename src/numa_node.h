#include <utility>
#include "cache.h"
#include "interconnect.h"

// bundles up a set of caches, a directory, and an interconnect into NUMA node
class NUMA {
 public:
  NUMA(int num_procs, int nodes, int node_id, int s, int E, int b, bool verbose = false);
  ~NUMA();

  void cacheRead(int proc, unsigned long addr, int numa_node);
  void cacheWrite(int proc, unsigned long addr, int numa_node);

  void printStats() const;

  void connectInterconnect(std::pair<Interconnect *, int> pair);
  std::pair<Interconnect *, int> getInterconnect();

 private:
  int node_id_;
  int num_nodes_;
  int num_procs_;
  int procs_per_node_;

  std::vector<Cache> caches_;
  Directory directory_;
  Interconnect *interconnect_;
};