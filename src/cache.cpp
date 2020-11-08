#include "cache.h"

Cache::Cache(int s, int E, int b) :
  s_(s), S_(1 << s), E_(E), b_(b), B_(1 << b),
  hit_count_(0),
  miss_count_(0),
  eviction_count_(0),
  dirty_blocks_evicted_(0) {
  // initialize the set vector full of sets
  sets_.resize(S_, Set(E));
}

// return the tag and an iterator to the set that the line is in
std::pair<long, std::vector<Set>::iterator> Cache::readAddr(unsigned long addr) {
  long tag_size = ADDR_LEN - (s_ + b_); // the tag is whatever's left after s and b
  long set_mask = 0L;
  for (int i = 0; i < s_; i++) {
      set_mask |= (1L << i);
  }
  long tag = addr & ((1L << (ADDR_LEN - 1L)) >> (tag_size - 1));
  long set_index = (addr & (set_mask << b_)) >> b_;

  auto set_iter = sets_.begin() + set_index;

  return {tag, set_iter};
}

// find a tag in the set at the iterator
// return true if the block was found in the cache, i.e. no eviction necessary
bool Cache::findInCache(long tag, std::vector<Set>::iterator set, bool is_write) {
  bool found = false;

  // run the entire loop here so we can increment last_used_ for all blocks
  for (auto block_iter = set->blocks_.begin(); block_iter != set->blocks_.end(); ++block_iter) {
    block_iter->last_used_++;

    if (block_iter->tag_ == tag) {
      found = true;

      // the block was accessed, reset its LRU counter
      block_iter->last_used_ = 0;

      if (is_write) {
        block_iter->dirty_ = true;
      }

      // The block will now be brought into the cache if it was initially invalid
      bool valid = block_iter->valid_;
      block_iter->valid_ = true;

      if (valid) hit_count_++;
      else miss_count_++;
    }

  }
  return found;
}

void Cache::performOperation(unsigned long addr, bool is_write) {
  auto pair = readAddr(addr);
  long tag = pair.first;
  auto set_iter = pair.second;

  if (findInCache(tag, set_iter, is_write)) {
    return;
  }

  // not found in the cache, must have been a non-cold miss
  miss_count_++;

  // the data was not found in the cache, something needs to be evicted
  // find the block with the largest last_used time stamp
  auto LRU_iter = std::max_element(set_iter->blocks_.begin(), set_iter->blocks_.end(),
      [](auto const &lhs, auto const &rhs) { return lhs.last_used_ <= rhs.last_used_; });

  // update counters
  if (LRU_iter->valid_) {
    if (LRU_iter->dirty_) {
      dirty_blocks_evicted_++;
    }
    eviction_count_++;
  }

  // update the block to refer to the new block that was brought into the cache
  LRU_iter->tag_ = tag;
  LRU_iter->valid_ = true;
  LRU_iter->dirty_ = is_write;
  LRU_iter->last_used_ = 0;
}

void Cache::cacheRead(unsigned long addr) {
  return performOperation(addr, /* is_write */ false);
}

void Cache::cacheWrite(unsigned long addr) {
  return performOperation(addr, /* is_write */ true);
}

void Cache::printState() {
  std::cout << "s:\t" << s_ << "\n"
            << "S:\t" << S_ << "\n"
            << "E:\t" << E_ << "\n"
            << "b:\t" << b_ << "\n"
            << "B:\t" << B_ << "\n";
}

void Cache::printStats() {
  std::cout << "miss_count:\t\t"          << miss_count_ << "\n"
            << "hit_count:\t\t"           << hit_count_ << "\n"
            << "eviction_count:\t\t"      << eviction_count_ << "\n"
            << "dirty_blocks_evicted:\t"  << dirty_blocks_evicted_ << "\n";
}
