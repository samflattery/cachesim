#pragma once
#include <vector>
#include <iostream>
#include <algorithm>

#define ADDR_LEN 64L

struct Block {
  Block(bool valid, bool dirty, long tag) : valid_(valid), dirty_(dirty), tag_(tag) {}
  Block() = default;
  bool valid_;
  bool dirty_;
  long tag_;
  long last_used_;
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
  bool findInCache(long tag, std::vector<Set>::iterator set, bool is_write);

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
