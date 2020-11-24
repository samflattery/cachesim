#include "interconnect.h"

Interconnect::Interconnect(int numa_node, int num_numa_nodes, int num_procs,
                           std::vector<Cache> *caches, Directory *directory, bool verbose)
    : numa_node_(numa_node),
      num_numa_nodes_(num_numa_nodes),
      num_procs_(num_procs),
      caches_(caches),
      directory_(directory),
      cache_events_(0L),
      directory_events_(0L),
      verbose_(verbose) {
  interconnects_.resize(num_numa_nodes_);

  for (auto &cache : (*caches_)) {
    cache.connectToInterconnect(this);
  }

  directory_->connectToInterconnect(this);
}

int Interconnect::getNumaNode() { return numa_node_; }

void Interconnect::connectInterconnect(Interconnect *interconnect, int id) {
  interconnects_[id] = interconnect;
}

void Interconnect::sendBusRd(int src, Address address) {
  if (address.numa_node != numa_node_) {
    std::cout << "sending over the main interconnect from " << numa_node_ << " to "
              << address.numa_node << "\n";
    interconnects_[address.numa_node]->sendBusRd(src, address);
  } else {
    if (verbose_)
      std::cout << "cache " << src << " sending BusRd of " << std::hex << address.addr << std::dec
                << "\n";
    directory_->receiveBusRd(src, address.addr);
    cache_events_++;
  }
}

void Interconnect::sendBusRdX(int src, Address address) {
  if (address.numa_node != numa_node_) {
    std::cout << "sending over the main interconnect from " << numa_node_ << " to "
              << address.numa_node << "\n";
    interconnects_[address.numa_node]->sendBusRdX(src, address);
  } else {
    if (verbose_)
      std::cout << "cache " << src << " sending BusRdX of " << std::hex << address.addr << std::dec
                << "\n";
    directory_->receiveBusRdX(src, address.addr);
    cache_events_++;
  }
}

void Interconnect::sendEviction(int src, Address address) {
  if (address.numa_node != numa_node_) {
    std::cout << "sending over the main interconnect from " << numa_node_ << " to "
              << address.numa_node << "\n";
    interconnects_[address.numa_node]->sendEviction(src, address);
  } else {
    if (verbose_)
      std::cout << "cache " << src << " sending Eviction of " << std::hex << address.addr
                << std::dec << "\n";
    directory_->receiveEviction(src, address.addr);
    cache_events_++;
  }
}

int Interconnect::getNode(int dest) {
  return dest / (num_procs_ / num_numa_nodes_);
  /* return dest % num_numa_nodes_; */
}

void Interconnect::sendFetch(int dest, long addr) {
  int dest_node;
  // the message might need to be sent to a cache on a different NUMA node
  if ((dest_node = getNode(dest)) != numa_node_) {
    std::cout << "sending over the main interconnect from " << numa_node_ << " to " << dest_node
              << "\n";
    interconnects_[dest_node]->sendFetch(dest, addr);
  } else {
    if (verbose_)
      std::cout << "sending Fetch of " << std::hex << addr << std::dec << " to cache " << dest
                << "\n";
    (*caches_)[dest % (num_procs_ / num_numa_nodes_)].receiveFetch(addr);
    directory_events_++;
  }
}

void Interconnect::sendReadMiss(int dest, long addr, bool exclusive) {
  int dest_node;
  if ((dest_node = getNode(dest)) != numa_node_) {
    std::cout << "sending over the main interconnect from " << numa_node_ << " to " << dest_node
              << "\n";
    interconnects_[dest_node]->sendReadMiss(dest, addr, exclusive);
  } else {
    if (verbose_)
      std::cout << "sending ReadMiss on " << std::hex << addr << std::dec << " to cache " << dest
                << "\n";
    (*caches_)[dest % (num_procs_ / num_numa_nodes_)].receiveReadMiss(addr, exclusive);
    directory_events_++;
  }
}

void Interconnect::sendWriteMiss(int dest, long addr) {
  int dest_node;
  if ((dest_node = getNode(dest)) != numa_node_) {
    std::cout << "sending over the main interconnect from " << numa_node_ << " to " << dest_node
              << "\n";
    interconnects_[dest_node]->sendWriteMiss(dest, addr);
  } else {
    if (verbose_)
      std::cout << "sending WriteMiss on " << std::hex << addr << std::dec << " to cache " << dest
                << "\n";
    (*caches_)[dest % (num_procs_ / num_numa_nodes_)].receiveWriteMiss(addr);
    directory_events_++;
  }
}

void Interconnect::sendInvalidate(int dest, long addr) {
  int dest_node;
  if ((dest_node = getNode(dest)) != numa_node_) {
    std::cout << "sending over the main interconnect from " << numa_node_ << " to " << dest_node
              << "\n";
    interconnects_[dest_node]->sendInvalidate(dest, addr);
  } else {
    if (verbose_)
      std::cout << "sending Invalidate of " << std::hex << addr << std::dec << " to cache " << dest
                << "\n";
    (*caches_)[dest % (num_procs_ / num_numa_nodes_)].receiveInvalidate(addr);
    directory_events_++;
  }
}

void Interconnect::printStats() {
  std::cout << "*** Interconnect Events ***\n"
            << "Cache Events:\t" << cache_events_ << "\n"
            << "Directory Events:\t" << directory_events_ << "\n"
            << "Total events:\t" << cache_events_ + directory_events_ << "\n";
}
