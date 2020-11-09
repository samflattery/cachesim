#include "interconnect.h"

Interconnect::Interconnect(std::vector<Cache> *caches, Directory *directory)
  : caches_(caches), directory_(directory) {
  for (auto &cache : (*caches_)) {
    cache.connectToInterconnect(this);
  }
  directory_->connectToInterconnect(this);
}

void Interconnect::sendBusRd(int src, long addr) {
  directory_->receiveBusRd(src, addr);
  total_events_++;
}

void Interconnect::sendBusRdX(int src, long addr) {
  directory_->receiveBusRdX(src, addr);
  total_events_++;
}

void Interconnect::sendFetch(int dest, long addr) {
  (*caches_)[dest].receiveFetch(addr);
  total_events_++;
}

void Interconnect::sendReadMiss(int dest, long addr, bool exclusive) {
  (*caches_)[dest].receiveReadMiss(addr, exclusive);
  total_events_++;
}

void Interconnect::sendWriteMiss(int dest, long addr) {
  (*caches_)[dest].receiveWriteMiss(addr);
  total_events_++;
}

void Interconnect::sendInvalidate(int dest, long addr) {
  (*caches_)[dest].receiveInvalidate(addr);
  total_events_++;
}
