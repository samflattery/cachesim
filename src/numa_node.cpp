#include "numa_node.h"

NUMA::NUMA(int num_procs, int num_nodes, int node_id, int s, int E, int b, Protocol protocol,
           bool verbose)
    : node_id_(node_id),
      num_nodes_(num_nodes),
      num_procs_(num_procs),
      procs_per_node_(num_procs_ / num_nodes_),
      directory_(num_procs, b, protocol, node_id_) {
  for (int i = 0; i < procs_per_node_; ++i) {
    int cache_id = procs_per_node_ * node_id + i;
    caches_.push_back(Cache(cache_id, node_id, s, E, b, protocol));
  }

  if (node_id == 0) {
    std::cout << "Running simulation with cache settings:\n";
    caches_[0].printState();
  }

  interconnect_ = new Interconnect(node_id, num_nodes, num_procs, &caches_, &directory_, verbose);
}

NUMA::~NUMA() { delete interconnect_; }

void NUMA::connectInterconnect(std::pair<Interconnect *, int> pair) {
  interconnect_->connectInterconnect(pair.first, pair.second);
}

std::pair<Interconnect *, int> NUMA::getInterconnect() { return {interconnect_, node_id_}; }

void NUMA::cacheRead(int proc, unsigned long addr, int numa_node) {
  caches_[proc % procs_per_node_].cacheRead({addr, numa_node});
}

void NUMA::cacheWrite(int proc, unsigned long addr, int numa_node) {
  caches_[proc % procs_per_node_].cacheWrite({addr, numa_node});
}

Stats NUMA::getStats() const {
  Stats stats;
  for (const auto &cache : caches_) {
    stats.hits_ += cache.getHitCount();
    stats.misses_ += cache.getMissCount();
    stats.flushes_ += cache.getFlushCount();
    stats.evictions_ += cache.getEvictionCount();
    stats.invalidations_ += cache.getInvalidationCount();
    stats.dirty_evictions_ += cache.getDirtyEvictionCount();
  }
  stats.memory_reads_ = directory_.getMemoryReads();
  stats.local_interconnect_ = interconnect_->getLocalEvents();
  stats.global_interconnect_ = interconnect_->getGlobalEvents();
  return stats;
}

void NUMA::printStats() const {
  std::cout << "\tNUMA NODE: " << node_id_ << "\n"
            << "\t-----------";
  for (const auto &cache : caches_) {
    cache.printStats();
  }
  interconnect_->printStats();
  std::cout << std::endl;
  std::cout << "*** Memory Reads ***" << std::endl;
  std::cout << "Memory Reads: " << directory_.getMemoryReads() << std::endl;
  std::cout << std::endl;
}
