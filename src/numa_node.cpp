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

void NUMA::printStats() const {
  std::cout << "\tNUMA NODE: " << node_id_ << "\n"
            << "\t-----------";
  for (const auto &cache : caches_) {
    cache.printStats();
  }
  interconnect_->printStats();
  std::cout << std::endl;
}
