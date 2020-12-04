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

// models an interconnect that the directories, caches, and NUMA nodes can communicate through
// acts as both the interconnect beween components in one UMA address space, as well as a larger
// global interconnect between NUMA addres spaces by maintaining a list of all interconnects in
// other NUMA nodes and updating stats differently when messages are being sent between nodes
class Interconnect {
 public:
  Interconnect(int numa_node, int num_numa_nodes, int num_procs, std::vector<Cache> *caches,
               Directory *directory, bool verbose = false);
  void connectInterconnect(Interconnect *interconnect, int id);

  // cache -> directory messages
  void sendBusRd(int src, Address address);
  void sendBusRdX(int src, Address address);
  void sendData(int src, Address address);
  void sendEviction(int src, Address address);
  void sendBroadcast(int src, Address address);

  // directory -> cache messages
  void sendReadMiss(int dest, long addr, bool exclusive);
  void sendWriteMiss(int dest, long addr);
  // the address contains the addr of the memory as well as the NUMA node of the directory that
  // requested the data
  void sendFetch(int dest, Address address);
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

  // all of the other interconnects on the NUMA system
  // interconnects_[i] is the interconnect of NUMA node i, so that we can pass messages between NUMA
  // nodes
  std::vector<Interconnect *> interconnects_;

  unsigned long cache_events_;
  unsigned long directory_events_;
  // messages being sent between nodes
  unsigned long global_events_;

  bool verbose_;
};
