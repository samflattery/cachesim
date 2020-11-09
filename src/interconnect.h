#pragma once
#include <vector>
#include "cache.h"
#include "directory.h"

class Cache;
class Directory;

class Interconnect {
public:
  Interconnect(std::vector<Cache> *caches, Directory *directory);

  void sendBusRd(int src, long addr);
  void sendBusRdX(int src, long addr);

private:
  std::vector<Cache> *caches_;
  Directory *directory_;
};
