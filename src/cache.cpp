#include "cache.h"

Cache::Cache(int id, int s, int E, int b) :
  cache_id_(id),
  s_(s), S_(1 << s), E_(E), b_(b), B_(1 << b),
  hit_count_(0),
  miss_count_(0),
  eviction_count_(0),
  dirty_blocks_evicted_(0),
  interconnect_(nullptr) {
  // initialize the set vector full of sets
  sets_.resize(S_, std::make_shared<Set>(E));
}

int Cache::getCacheId() const { return cache_id_; }

void Cache::connectToInterconnect(Interconnect *interconnect) { interconnect_ = interconnect; }

// return the tag and an iterator to the set that the line is in
std::pair<long, std::shared_ptr<Set>> Cache::readAddr(unsigned long addr) {
  long tag_size = ADDR_LEN - (s_ + b_); // the tag is whatever's left after s and b
  long set_mask = 0L;
  for (int i = 0; i < s_; i++) {
      set_mask |= (1L << i);
  }
  long tag = addr & ((1L << (ADDR_LEN - 1L)) >> (tag_size - 1));
  long set_index = (addr & (set_mask << b_)) >> b_;

  auto set_ptr = sets_[set_index];

  return {tag, set_ptr};
}

std::ostream& operator<<(std::ostream& os, const MESI &mesi) {
  switch (mesi) {
    case MESI::M:
      os << "M";
      break;
    case MESI::E:
      os << "E";
      break;
    case MESI::S:
      os << "S";
      break;
    case MESI::I:
      os << "I";
      break;
  }
  return os;
}

// find a tag in the set at the iterator
CacheBlock* Cache::findInSet(long tag, std::shared_ptr<Set> set) {
  CacheBlock* found = nullptr;

  // run the entire loop here so we can increment last_used_ for all blocks
  for (auto block : set->blocks_) {
    block->incrLastUsed();
    if (block->getTag() == tag) {
      found = block;
    }
  }

  return found;
}

void Cache::evictAndReplace(long tag, std::shared_ptr<Set> set, bool is_write) {
  // find the block with the largest last_used time stamp
  auto LRU_block = std::max_element(set->blocks_.begin(), set->blocks_.end(),
      [](auto const &lhs, auto const &rhs) { return lhs->getLastUsed() <= rhs->getLastUsed(); });

  (*LRU_block)->evictAndReplace(is_write, tag);
}

CacheBlock* Cache::findInCache(long addr) {
  auto pair = readAddr(addr);
  long tag = pair.first;
  auto set = pair.second;

  auto block = findInSet(tag, set);
  return block;
}

void Cache::performOperation(unsigned long addr, bool is_write) {
  auto pair = readAddr(addr);
  long tag = pair.first;
  auto set = pair.second;

  auto block = findInSet(tag, set);
  if (block != nullptr) {
    if (is_write) {
      block->writeBlock();
    }
    else {
      block->readBlock();
    }
    return;
  }

  // we missed in the cache and something needs to be evicted
  evictAndReplace(tag, set, is_write);
}

void Cache::cacheRead(unsigned long addr) {
  return performOperation(addr, /* is_write */ false);
}

void Cache::cacheWrite(unsigned long addr) {
  return performOperation(addr, /* is_write */ true);
}

void Cache::receiveInvalidate(long addr) {
  auto block = findInCache(addr);
  block->invalidate();
}

void Cache::receiveFetch(long addr) {
  auto block = findInCache(addr);
  block->flush();
}

void Cache::receiveReadMiss(long addr, bool exclusive) {
  auto block = findInCache(addr);
  block->readMiss(exclusive);
}

void Cache::receiveWriteMiss(long addr) {
  auto block = findInCache(addr);
  block->writeMiss();
}

void Cache::printState() const {
  std::cout << "set size:\t" << S_ << "\n"
            << "associativity:\t" << E_ << "\n"
            << "line size:\t" << B_ << "\n\n";
}

void Cache::printStats() const {
  std::cout << "\n*** Cache "            << cache_id_ << " ***\n"
            << "miss_count:\t\t"          << miss_count_ << "\n"
            << "hit_count:\t\t"           << hit_count_ << "\n"
            << "eviction_count:\t\t"      << eviction_count_ << "\n"
            << "dirty_blocks_evicted:\t"  << dirty_blocks_evicted_ << "\n\n";
}
