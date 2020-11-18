#pragma once
#include <iostream>
#include <vector>
#include "cache.h"
#include "directory.h"

// forward declarations because there is a circular dependency between the
// headers
class Cache;
class Directory;

// models an interconnect that the directory and cache will communicate through
class Interconnect {
 public:
  Interconnect(std::vector<Cache> *caches, Directory *directory, bool verbose = false);

  // cache -> directory messages
  void sendBusRd(int src, long addr);
  void sendBusRdX(int src, long addr);
  void sendEviction(int src, long addr);

  // directory -> cache messages
  void sendReadMiss(int dest, long addr, bool exclusive);
  void sendWriteMiss(int dest, long addr);
  void sendFetch(int dest, long addr);
  void sendInvalidate(int dest, long addr);

  void printStats();

 private:
  std::vector<Cache> *caches_;
  Directory *directory_;

  unsigned long cache_events_;
  unsigned long directory_events_;

  bool verbose_;
};
