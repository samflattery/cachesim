#pragma once

#include <vector>
#include <iostream>
#include <algorithm>
#include <memory>
#include "interconnect.h"

#include "mesi_block.h"
#include "cache_block.h"

#define ADDR_LEN 64L

// defines a cache set as a set of blocks
struct Set {
  Set(int associativity) {
    blocks_.reserve(associativity);
    for (int i = 0; i < associativity; i++) {
      blocks_.push_back(new MESIBlock);
    }
  }
  ~Set() { for (auto block : blocks_) delete block; }
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
  void evictAndReplace(long tag, std::shared_ptr<Set> set, unsigned long addr, bool is_write);

  // send a given message over the interconnect
  void performInterconnectAction(InterconnectAction action, unsigned long addr);

  // send a message over the interconnect saying that this memory address has been evicted so this
  // cache no longer has ownership of it
  // tag is the tag of the evicted bit
  // addr is the address of the new block, which is used to add the set bits back to tag
  void sendEviction(unsigned long tag, unsigned long addr);

  int getCacheId() const;

  size_t getHitCount() const;
  size_t getMissCount() const;
  size_t getEvictionCount() const;
  size_t getDirtyEvictionCount() const;

  int cache_id_;

  // the settings of the cache such as line size
  int s_;
  int S_;
  int E_;
  int b_;
  int B_;

  // the interconnect through which messages to the directory are sent can be nullptr when running
  // with a single cache
  Interconnect *interconnect_;

  std::vector<std::shared_ptr<Set>> sets_;
};
