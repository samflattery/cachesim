#include "cache.h"

Cache::Cache(int id, int numa_node, int s, int E, int b, Protocol protocol)
    : cache_id_(id),
      numa_node_(numa_node),
      s_(s),
      S_(1 << s),
      E_(E),
      b_(b),
      B_(1 << b),
      protocol_(protocol),
      interconnect_(nullptr) {
  // construct sets in a loop to make sure that they are not copied pointers
  sets_.reserve(S_);
  for (int i = 0; i < S_; i++) {
    sets_.push_back(std::make_shared<Set>(E, protocol_));
  }
}

int Cache::getCacheId() const { return cache_id_; }

void Cache::connectToInterconnect(Interconnect* interconnect) { interconnect_ = interconnect; }

// return the tag and an iterator to the set that the line is in
std::pair<long, std::shared_ptr<Set>> Cache::readAddr(unsigned long addr) {
  long tag_size = ADDR_LEN - (s_ + b_);  // the tag is whatever's left after s and b
  long set_mask = 0L;
  for (int i = 0; i < s_; i++) {
    set_mask |= (1L << i);
  }
  long tag = addr & ((1L << (ADDR_LEN - 1L)) >> (tag_size - 1));
  long set_index = (addr & (set_mask << b_)) >> b_;

  auto set_ptr = sets_[set_index];

  return {tag, set_ptr};
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

void Cache::evictAndReplace(long tag, std::shared_ptr<Set> set, Address address, bool is_write) {
  // find the block with the largest last_used time stamp
  auto LRU_block = std::max_element(
      set->blocks_.begin(), set->blocks_.end(),
      [](auto const& lhs, auto const& rhs) { return lhs->getLastUsed() <= rhs->getLastUsed(); });

  if ((*LRU_block)->isValid()) {
    sendEviction((*LRU_block)->getTag(), address.addr, (*LRU_block)->getNumaNode());
  }

  InterconnectAction action = (*LRU_block)->evictAndReplace(is_write, tag, address.numa_node);
  performInterconnectAction(action, address);
}

void Cache::sendEviction(unsigned long tag, unsigned long addr, int numa_node) {
  if (interconnect_ == nullptr) return;

  // reconstruct the address using the tag and set bits of addr
  unsigned long old_addr;

  long set_mask = 0L;
  for (int i = 0; i < s_; i++) {
    set_mask |= (1L << i);
  }
  long set_bits = (addr & (set_mask << b_));

  old_addr = tag | set_bits;

  // send message over interconnect to directory
  interconnect_->sendEviction(cache_id_, {old_addr, numa_node});
}

CacheBlock* Cache::findInCache(long addr) {
  auto pair = readAddr(addr);
  long tag = pair.first;
  auto set = pair.second;

  auto block = findInSet(tag, set);
  return block;
}

void Cache::performOperation(Address address, bool is_write) {
  auto pair = readAddr(address.addr);
  long tag = pair.first;
  auto set = pair.second;

  auto block = findInSet(tag, set);

  if (block != nullptr) {
    InterconnectAction action;
    if (is_write) {
      action = block->writeBlock(address.numa_node);
    } else {
      action = block->readBlock(address.numa_node);
    }
    performInterconnectAction(action, address);
    return;
  }

  // we missed in the cache and something needs to be evicted
  evictAndReplace(tag, set, address, is_write);
}

void Cache::cacheRead(Address address) { return performOperation(address, /* is_write */ false); }

void Cache::cacheWrite(Address address) { return performOperation(address, /* is_write */ true); }

void Cache::performInterconnectAction(InterconnectAction action, Address address) {
  if (interconnect_ == nullptr) return;

  switch (action) {
    case InterconnectAction::BUSRD:
      interconnect_->sendBusRd(cache_id_, address);
      break;
    case InterconnectAction::BUSRDX:
      interconnect_->sendBusRdX(cache_id_, address);
      break;
    case InterconnectAction::BROADCAST:
      interconnect_->sendBroadcast(cache_id_, address);
      break;
    case InterconnectAction::NOACTION:
      break;
    default:
      break;
  }
}

void Cache::receiveInvalidate(long addr) {
  auto block = findInCache(addr);
  block->invalidate();
}

void Cache::receiveFetch(Address address) {
  auto block = findInCache(address.addr);
  block->fetch();
  interconnect_->sendData(cache_id_, address, block->isDirty());
}

void Cache::receiveReadData(long addr, bool exclusive) {
  auto block = findInCache(addr);
  block->receiveReadData(exclusive);
}

void Cache::receiveWriteData(long addr) {
  auto block = findInCache(addr);
  block->receiveWriteData();
}

CacheStats Cache::getStats() const {
  CacheStats stats;
  for (auto set : sets_) {
    for (auto block : set->blocks_) {
      stats.hits_ += block->getHitCount();
      stats.misses_ += block->getMissCount();
      stats.flushes_ += block->getFlushCount();
      stats.invalidations_ += block->getInvalidationCount();
      stats.evictions_ += block->getEvictionCount();
      stats.dirty_evictions_ += block->getDirtyEvictionCount();
    }
  }
  stats.memory_writes_ = stats.dirty_evictions_ + stats.flushes_;
  return stats;
}

void Cache::printState() const {
  std::cout << "set size:\t" << S_ << "\n"
            << "associativity:\t" << E_ << "\n"
            << "line size:\t" << B_ << "\n\n";
}

void Cache::printStats() const {
  auto stats = getStats();
  std::cout << "\n*** Cache " << cache_id_ << " ***\n"
            << "Misses:\t\t\t" << stats.misses_ << "\n"
            << "Hits:\t\t\t" << stats.hits_ << "\n"
            << "Flushes:\t\t" << stats.flushes_ << "\n"
            << "Evictions:\t\t" << stats.evictions_ << "\n"
            << "Dirty Evictions:\t" << stats.dirty_evictions_ << "\n"
            << "Invalidations:\t\t" << stats.invalidations_ << "\n"
            << "Cache Access Latency:\t" << outputLatency(stats.hits_ * CACHE_LATENCY) << "\n"
            << "Memory Write Latency:\t" << outputLatency(stats.memory_writes_ * MEMORY_LATENCY)
            << "\n\n";
}

int Cache::getID() const { return cache_id_; }
