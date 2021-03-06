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
      global_events_(0L),
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
    global_events_++;
    cache_events_++;
    if (verbose_)
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
    global_events_++;
    cache_events_++;
    if (verbose_)
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

void Interconnect::sendData(int src, Address address, bool is_dirty) {
  if (address.numa_node != numa_node_) {
    global_events_++;
    cache_events_++;
    if (verbose_)
      std::cout << "sending over the main interconnect from " << numa_node_ << " to "
                << address.numa_node << "\n";
    interconnects_[address.numa_node]->sendData(src, address, is_dirty);
  } else {
    if (verbose_)
      std::cout << "cache " << src << " sending Data of " << std::hex << address.addr << std::dec
                << "\n";
    directory_->receiveData(src, address.addr, is_dirty);
    cache_events_++;
  }
}

void Interconnect::sendBroadcast(int src, Address address) {
  if (address.numa_node != numa_node_) {
    global_events_++;
    cache_events_++;
    if (verbose_)
      std::cout << "sending over the main interconnect from " << numa_node_ << " to "
                << address.numa_node << "\n";
    interconnects_[address.numa_node]->sendBroadcast(src, address);
  } else {
    if (verbose_)
      std::cout << "cache " << src << " sending Broadcast of " << std::hex << address.addr
                << std::dec << "\n";
    directory_->receiveBroadcast(src, address.addr);
    cache_events_++;
  }
}

void Interconnect::sendEviction(int src, Address address) {
  if (address.numa_node != numa_node_) {
    global_events_++;
    cache_events_++;
    if (verbose_)
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

int Interconnect::getNode(int dest) { return dest / (num_procs_ / num_numa_nodes_); }

void Interconnect::sendFetch(int dest, Address address) {
  int dest_node;
  // the message might need to be sent to a cache on a different NUMA node
  if ((dest_node = getNode(dest)) != numa_node_) {
    global_events_++;
    directory_events_++;
    if (verbose_)
      std::cout << "sending over the main interconnect from " << numa_node_ << " to " << dest_node
                << "\n";
    interconnects_[dest_node]->sendFetch(dest, address);
  } else {
    if (verbose_)
      std::cout << "sending Fetch of " << std::hex << address.addr << std::dec << " to cache "
                << dest << "\n";

    (*caches_)[dest % (num_procs_ / num_numa_nodes_)].receiveFetch(address);
    directory_events_++;
  }
}

void Interconnect::sendReadData(int dest, long addr, bool exclusive) {
  int dest_node;
  if ((dest_node = getNode(dest)) != numa_node_) {
    global_events_++;
    directory_events_++;
    if (verbose_)
      std::cout << "sending over the main interconnect from " << numa_node_ << " to " << dest_node
                << "\n";
    interconnects_[dest_node]->sendReadData(dest, addr, exclusive);
  } else {
    if (verbose_)
      std::cout << "sending ReadMiss on " << std::hex << addr << std::dec << " to cache " << dest
                << "\n";
    (*caches_)[dest % (num_procs_ / num_numa_nodes_)].receiveReadData(addr, exclusive);
    directory_events_++;
  }
}

void Interconnect::sendWriteData(int dest, long addr) {
  int dest_node;
  if ((dest_node = getNode(dest)) != numa_node_) {
    global_events_++;
    directory_events_++;
    if (verbose_)
      std::cout << "sending over the main interconnect from " << numa_node_ << " to " << dest_node
                << "\n";
    interconnects_[dest_node]->sendWriteData(dest, addr);
  } else {
    if (verbose_)
      std::cout << "sending WriteMiss on " << std::hex << addr << std::dec << " to cache " << dest
                << "\n";
    (*caches_)[dest % (num_procs_ / num_numa_nodes_)].receiveWriteData(addr);
    directory_events_++;
  }
}

void Interconnect::sendInvalidate(int dest, long addr) {
  int dest_node;
  if ((dest_node = getNode(dest)) != numa_node_) {
    global_events_++;
    directory_events_++;
    if (verbose_)
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
            << "Cache Events:\t\t\t" << cache_events_ << "\n"
            << "Directory Events:\t\t" << directory_events_ << "\n"
            << "Global Interconnect Events:\t" << global_events_ << "\n"
            << "Local Interconnect Latency:\t"
            << outputLatency((cache_events_ + directory_events_) * LOCAL_INTERCONNECT_LATENCY)
            << "\n"
            << "Global Interconnect Latency:\t"
            << outputLatency(global_events_ * GLOBAL_INTERCONNECT_LATENCY) << "\n";
}
