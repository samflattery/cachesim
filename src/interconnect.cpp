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
}

void Interconnect::sendBusRdX(int src, long addr) {
  directory_->receiveBusRdX(src, addr);
}
