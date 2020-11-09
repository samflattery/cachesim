#pragma once
#include <vector>
#include "cache.h"
#include "directory.h"

// forward declarations because there is a circular dependency between the headers
class Cache;
class Directory;

// models an interconnect that the directory and cache will communicate through
class Interconnect {
public:
  Interconnect(std::vector<Cache> *caches, Directory *directory);

  // cache -> directory messages
  void sendBusRd(int src, long addr);
  void sendBusRdX(int src, long addr);

  // directory -> cache messages
  void sendReadMiss(int dest, long addr, bool exclusive);
  void sendWriteMiss(int dest, long addr);
  void sendFetch(int dest, long addr);
  void sendInvalidate(int dest, long addr);

private:
  std::vector<Cache> *caches_;
  Directory *directory_;

  unsigned long total_events_;
};
