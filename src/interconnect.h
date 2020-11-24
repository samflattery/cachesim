#pragma once
#include <iostream>
#include <vector>
#include "cache.h"
#include "directory.h"

// forward declarations because there is a circular dependency between the
// headers
class Cache;
class Directory;

// defined in cache.h
struct Address;

// models an interconnect that the directory and cache will communicate through
class Interconnect {
 public:
  Interconnect(int numa_node, int num_numa_nodes, int num_procs, std::vector<Cache> *caches, Directory *directory,
               bool verbose = false);
  void connectInterconnect(Interconnect *interconnect, int id);

  // cache -> directory messages
  void sendBusRd(int src, Address address);
  void sendBusRdX(int src, Address address);
  void sendEviction(int src, Address address);

  // directory -> cache messages
  void sendReadMiss(int dest, long addr, bool exclusive);
  void sendWriteMiss(int dest, long addr);
  void sendFetch(int dest, long addr);
  void sendInvalidate(int dest, long addr);

  void printStats();

  int getNumaNode();

 private:
  int getNode(int dest);

  int numa_node_;
  int num_numa_nodes_;
  int num_procs_;

  std::vector<Cache> *caches_;
  Directory *directory_;

  std::vector<Interconnect *> interconnects_;

  unsigned long cache_events_;
  unsigned long directory_events_;

  bool verbose_;
};
