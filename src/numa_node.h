#include <utility>
#include "cache.h"
#include "interconnect.h"

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
  int procs_per_node_;
  int node_id_;
  int num_nodes_;
  int num_procs_;

  std::vector<Cache> caches_;
  Directory directory_;
  Interconnect *interconnect_;
};
