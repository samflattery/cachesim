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

  void sendBusRd(int src, long addr);
  void sendBusRdX(int src, long addr);

private:
  std::vector<Cache> *caches_;
  Directory *directory_;
};
