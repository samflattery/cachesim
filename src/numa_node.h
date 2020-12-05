#pragma once
#include <utility>
#include "cache.h"
#include "interconnect.h"
#include "protocols.h"

struct Stats {
  size_t hits_ = 0, misses_ = 0, flushes_ = 0, evictions_ = 0, dirty_evictions_ = 0,
         invalidations_ = 0, local_interconnect_ = 0, global_interconnect_ = 0, memory_reads_ = 0;
  Stats &operator+=(const Stats &other) {
    hits_ += other.hits_;
    misses_ += other.misses_;
    flushes_ += other.flushes_;
    evictions_ += other.evictions_;
    dirty_evictions_ += other.dirty_evictions_;
    local_interconnect_ += other.local_interconnect_;
    global_interconnect_ += other.global_interconnect_;
    memory_reads_ += other.memory_reads_;
    return *this;
  }
};

// bundles up a set of caches, a directory, and an interconnect into NUMA node
class NUMA {
 public:
  NUMA(int num_procs, int nodes, int node_id, int s, int E, int b, Protocol protocol,
       bool verbose = false);
  ~NUMA();

  void cacheRead(int proc, unsigned long addr, int numa_node);
  void cacheWrite(int proc, unsigned long addr, int numa_node);

  void printStats() const;
  Stats getStats() const;

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
