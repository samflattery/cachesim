#include "interconnect.h"

Interconnect::Interconnect(std::vector<Cache> *caches, Directory *directory,
                           bool verbose)
    : caches_(caches),
      directory_(directory),
      total_events_(0L),
      verbose_(verbose) {
  for (auto &cache : (*caches_)) {
    cache.connectToInterconnect(this);
  }
  directory_->connectToInterconnect(this);
}

void Interconnect::sendBusRd(int src, long addr) {
  if (verbose_)
    std::cout << "cache " << src << " sending BusRd of " << std::hex << addr
              << std::dec << "\n";
  directory_->receiveBusRd(src, addr);
  total_events_++;
}

void Interconnect::sendBusRdX(int src, long addr) {
  if (verbose_)
    std::cout << "cache " << src << " sending BusRdX of " << std::hex << addr
              << std::dec << "\n";
  directory_->receiveBusRdX(src, addr);
  total_events_++;
}

void Interconnect::sendEviction(int src, long addr) {
  if (verbose_)
    std::cout << "cache " << src << " sending Eviction of " << std::hex << addr
              << std::dec << "\n";
  directory_->receiveEviction(src, addr);
  total_events_++;
}

void Interconnect::sendFetch(int dest, long addr) {
  if (verbose_)
    std::cout << "sending Fetch of " << std::hex << addr << std::dec
              << " to cache " << dest << "\n";
  (*caches_)[dest].receiveFetch(addr);
  total_events_++;
}

void Interconnect::sendReadMiss(int dest, long addr, bool exclusive) {
  if (verbose_)
    std::cout << "sending ReadMiss on " << std::hex << addr << std::dec
              << " to cache " << dest << "\n";
  (*caches_)[dest].receiveReadMiss(addr, exclusive);
  total_events_++;
}

void Interconnect::sendWriteMiss(int dest, long addr) {
  if (verbose_)
    std::cout << "sending WriteMiss on " << std::hex << addr << std::dec
              << " to cache " << dest << "\n";
  (*caches_)[dest].receiveWriteMiss(addr);
  total_events_++;
}

void Interconnect::sendInvalidate(int dest, long addr) {
  if (verbose_)
    std::cout << "sending Invalidate of " << std::hex << addr << std::dec
              << " to cache " << dest << "\n";
  (*caches_)[dest].receiveInvalidate(addr);
  total_events_++;
}
