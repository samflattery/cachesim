#include "cache.h"

Cache::Cache(int id, int s, int E, int b)
    : cache_id_(id),
      s_(s),
      S_(1 << s),
      E_(E),
      b_(b),
      B_(1 << b),
      interconnect_(nullptr) {
  // construct sets in a loop to make sure that they are not copied pointers
  sets_.reserve(S_);
  for (int i = 0; i < S_; i++) {
    sets_.push_back(std::make_shared<Set>(E));
  }
}

int Cache::getCacheId() const { return cache_id_; }

void Cache::connectToInterconnect(Interconnect* interconnect) {
  interconnect_ = interconnect;
}

// return the tag and an iterator to the set that the line is in
std::pair<long, std::shared_ptr<Set>> Cache::readAddr(unsigned long addr) {
  long tag_size =
      ADDR_LEN - (s_ + b_);  // the tag is whatever's left after s and b
  long set_mask = 0L;
  for (int i = 0; i < s_; i++) {
    set_mask |= (1L << i);
  }
  long tag = addr & ((1L << (ADDR_LEN - 1L)) >> (tag_size - 1));
  long set_index = (addr & (set_mask << b_)) >> b_;

  auto set_ptr = sets_[set_index];

  return {tag, set_ptr};
}

std::ostream& operator<<(std::ostream& os, const MESI& mesi) {
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

void Cache::evictAndReplace(long tag, std::shared_ptr<Set> set,
                            unsigned long addr, bool is_write) {
  // find the block with the largest last_used time stamp
  auto LRU_block =
      std::max_element(set->blocks_.begin(), set->blocks_.end(),
                       [](auto const& lhs, auto const& rhs) {
                         return lhs->getLastUsed() <= rhs->getLastUsed();
                       });

  if ((*LRU_block)->isValid()) {
    sendEviction((*LRU_block)->getTag(), addr);
  }

  InterconnectAction action = (*LRU_block)->evictAndReplace(is_write, tag);
  performInterconnectAction(action, addr);
}

void Cache::sendEviction(unsigned long tag, unsigned long addr) {
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
  interconnect_->sendEviction(cache_id_, old_addr);
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
    InterconnectAction action;
    if (is_write) {
      action = block->writeBlock();
    } else {
      action = block->readBlock();
    }
    performInterconnectAction(action, addr);
    return;
  }

  // we missed in the cache and something needs to be evicted
  evictAndReplace(tag, set, addr, is_write);
}

void Cache::cacheRead(unsigned long addr) {
  return performOperation(addr, /* is_write */ false);
}

void Cache::cacheWrite(unsigned long addr) {
  return performOperation(addr, /* is_write */ true);
}

void Cache::performInterconnectAction(InterconnectAction action,
                                      unsigned long addr) {
  if (interconnect_ == nullptr) return;

  switch (action) {
    case InterconnectAction::BUSRD:
      interconnect_->sendBusRd(cache_id_, addr);
      break;
    case InterconnectAction::BUSRDX:
      interconnect_->sendBusRdX(cache_id_, addr);
      break;
    case InterconnectAction::FLUSH:
      // TODO(samflattery) add flush case
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
};

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

size_t Cache::getHitCount() const {
  size_t count = 0;
  for (auto set : sets_) {
    for (auto block : set->blocks_) {
      count += block->getHitCount();
    }
  }
  return count;
}

size_t Cache::getMissCount() const {
  size_t count = 0;
  for (auto set : sets_) {
    for (auto block : set->blocks_) {
      count += block->getMissCount();
    }
  }
  return count;
}

size_t Cache::getEvictionCount() const {
  size_t count = 0;
  for (auto set : sets_) {
    for (auto block : set->blocks_) {
      count += block->getEvictionCount();
    }
  }
  return count;
}

size_t Cache::getDirtyEvictionCount() const {
  size_t count = 0;
  for (auto set : sets_) {
    for (auto block : set->blocks_) {
      count += block->getDirtyEvictionCount();
    }
  }
  return count;
}

void Cache::printState() const {
  std::cout << "set size:\t" << S_ << "\n"
            << "associativity:\t" << E_ << "\n"
            << "line size:\t" << B_ << "\n\n";
}

void Cache::printStats() const {
  std::cout << "\n*** Cache " << cache_id_ << " ***\n"
            << "miss_count:\t\t" << getMissCount() << "\n"
            << "hit_count:\t\t" << getHitCount() << "\n"
            << "eviction_count:\t\t" << getEvictionCount() << "\n"
            << "dirty_blocks_evicted:\t" << getDirtyEvictionCount() << "\n\n";
}
