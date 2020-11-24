#include "interconnect.h"

Interconnect::Interconnect(std::vector<Cache> *caches, Directory *directory, bool verbose)
    : caches_(caches),
      directory_(directory),
      cache_events_(0L),
      directory_events_(0L),
      verbose_(verbose) {
  for (auto &cache : (*caches_)) {
    cache.connectToInterconnect(this);
  }
  directory_->connectToInterconnect(this);
}

void Interconnect::sendBusRd(int src, long addr) {
  if (verbose_)
    std::cout << "cache " << src << " sending BusRd of " << std::hex << addr << std::dec << "\n";
  directory_->receiveBusRd(src, addr);
  cache_events_++;
}

void Interconnect::sendBusRdX(int src, long addr) {
  if (verbose_)
    std::cout << "cache " << src << " sending BusRdX of " << std::hex << addr << std::dec << "\n";
  directory_->receiveBusRdX(src, addr);
  cache_events_++;
}

void Interconnect::sendEviction(int src, long addr) {
  if (verbose_)
    std::cout << "cache " << src << " sending Eviction of " << std::hex << addr << std::dec << "\n";
  directory_->receiveEviction(src, addr);
  cache_events_++;
}

void Interconnect::sendFetch(int dest, long addr) {
  if (verbose_)
    std::cout << "sending Fetch of " << std::hex << addr << std::dec << " to cache " << dest
              << "\n";
  (*caches_)[dest].receiveFetch(addr);
  directory_events_++;
}

void Interconnect::sendReadMiss(int dest, long addr, bool exclusive) {
  if (verbose_)
    std::cout << "sending ReadMiss on " << std::hex << addr << std::dec << " to cache " << dest
              << "\n";
  (*caches_)[dest].receiveReadMiss(addr, exclusive);
  directory_events_++;
}

void Interconnect::sendWriteMiss(int dest, long addr) {
  if (verbose_)
    std::cout << "sending WriteMiss on " << std::hex << addr << std::dec << " to cache " << dest
              << "\n";
  (*caches_)[dest].receiveWriteMiss(addr);
  directory_events_++;
}

void Interconnect::sendInvalidate(int dest, long addr) {
  if (verbose_)
    std::cout << "sending Invalidate of " << std::hex << addr << std::dec << " to cache " << dest
              << "\n";
  (*caches_)[dest].receiveInvalidate(addr);
  directory_events_++;
}

void Interconnect::printStats() {
  std::cout << "*** Interconnect Events ***\n"
            << "Cache Events:\t" << cache_events_ << "\n"
            << "Directory Events:\t" << directory_events_ << "\n"
            << "Total events:\t" << cache_events_ + directory_events_ << "\n";
}
