#pragma once

#include <vector>
#include <iostream>
#include <algorithm>
#include "interconnect.h"

#define ADDR_LEN 64L

// explicitly set I = 0 to default initialize an enum to invalid state
enum class MESI {
  M = 1,
  E = 2,
  S = 3,
  I = 0
};

// defines a cache block / cache line metadata and its MESI state
struct Block {
  bool valid_;
  bool dirty_;
  long tag_;
  long last_used_; // used to track LRU block in a set
  MESI state_;
};

// defines a cache set as a set of blocks
struct Set {
  Set(int associativity) { blocks_.resize(associativity, Block()); }
  std::vector<Block> blocks_;
};

// forward declarations because there is a circular dependency between the headers
class Interconnect;

// a single LRU cache
class Cache {
public:
  // construct a new cache with given id with 2^s sets, 2^b bytes per block and
  // associativity E
  Cache(int id, int s, int E, int b);

  // perform a read / write to a given address
  void cacheWrite(unsigned long addr);
  void cacheRead(unsigned long addr);

  void printState() const;
  void printStats() const;

  void connectToInterconnect(Interconnect *interconnect);

private:
  // perform a read / write to given address
  void performOperation(unsigned long addr, bool is_write);

  // get the tag and set of an address
  std::pair<long, std::vector<Set>::iterator> readAddr(unsigned long addr);

  // find the block that that matches the tag in a given set
  // return set->blocks_.end() if tag not found
  std::vector<Block>::iterator findInCache(long tag, std::vector<Set>::iterator set);

  // update the MESI state of a block
  void updateBlockState(std::vector<Block>::iterator block, long addr, bool is_write);

  // evict the LRU block in the cache and set the state of the replacement block
  void evictAndReplace(long tag, std::vector<Set>::iterator set, long addr, bool is_write);

  int getCacheId() const;

  int cache_id_;

  // the settings of the cache such as line size
  int s_;
  int S_;
  int E_;
  int b_;
  int B_;

  // stats being collected about execution
  long hit_count_;
  long miss_count_;
  long eviction_count_;
  long dirty_blocks_evicted_;

  // the interconnect through which messages to the directory are sent
  Interconnect *interconnect_;

  std::vector<Set> sets_;
};
