#pragma once

#include <vector>
#include <iostream>
#include <algorithm>

#define ADDR_LEN 64L

// explicitly set I = 0 to default initialize an enum to invalid state
enum class MESI {
  M = 1,
  E = 2,
  S = 3,
  I = 0
};

struct Block {
  bool valid_;
  bool dirty_;
  long tag_;
  long last_used_; // used to track LRU block in a set
  MESI state_;
};

struct Set {
  Set(int associativity) { blocks_.resize(associativity, Block()); }
  std::vector<Block> blocks_;
};

class Cache {
public:
  Cache(int s, int E, int b);

  void cacheWrite(unsigned long addr);
  void cacheRead(unsigned long addr);

  void printState();
  void printStats();

private:
  void performOperation(unsigned long addr, bool is_write);
  std::pair<long, std::vector<Set>::iterator> readAddr(unsigned long addr);
  std::vector<Block>::iterator findInCache(long tag, std::vector<Set>::iterator set);

  void updateBlockState(std::vector<Block>::iterator block, bool is_write);
  void evictAndReplace(long tag, std::vector<Set>::iterator set, bool is_write);

  void issueBusRd() {}
  void issueBusRdX() {}
  void issueFlush() {}

  void receiveBusRd() {}
  void receiveBusRdX() {}

  int s_;
  int S_;
  int E_;
  int b_;
  int B_;

  long hit_count_;
  long miss_count_;
  long eviction_count_;
  long dirty_blocks_evicted_;

  std::vector<Set> sets_;
};
