#pragma once

#include <vector>
#include <iostream>
#include <algorithm>
#include <memory>
#include "interconnect.h"
#include "mesi_block.h"

#define ADDR_LEN 64L

// defines a cache set as a set of blocks
struct Set {
  Set(int associativity) {
    for (int i = 0; i < associativity; i++) {
      MESIBlock *block = new MESIBlock;
      blocks_.push_back(block);
    }
  }

  ~Set() {
    std::cout << "freeing block\n";
    for (auto block : blocks_) delete block;
  }
  std::vector<CacheBlock*> blocks_;
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

  // *** communication with interconnect ***
  void receiveInvalidate(long addr);
  void receiveFetch(long addr);
  void receiveReadMiss(long addr, bool exclusive);
  void receiveWriteMiss(long addr);

private:
  // perform a read / write to given address
  void performOperation(unsigned long addr, bool is_write);

  // get the tag and set of an address
  std::pair<long, std::shared_ptr<Set>> readAddr(unsigned long addr);

  // find the block that that matches the tag in a given set
  // return set->blocks_.end() if tag not found
  CacheBlock* findInSet(long tag, std::shared_ptr<Set> set);

  // find the block that the address maps to
  // asserts that the block is in the cache, since this method is used in the interconnect callbacks
  CacheBlock* findInCache(long addr);

  // evict the LRU block in the cache and set the state of the replacement block
  void evictAndReplace(long tag, std::shared_ptr<Set> set, bool is_write);

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

  // the interconnect through which messages to the directory are sent can be nullptr when running
  // with a single cache
  Interconnect *interconnect_;

  std::vector<std::shared_ptr<Set>> sets_;
};
