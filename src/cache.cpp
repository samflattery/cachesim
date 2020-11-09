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
  sets_.resize(S_, Set(E));
}

int Cache::getCacheId() const { return cache_id_; }

void Cache::connectToInterconnect(Interconnect *interconnect) {
  interconnect_ = interconnect;
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
std::vector<Block>::iterator Cache::findInSet(long tag, std::vector<Set>::iterator set) {
  std::vector<Block>::iterator found = set->blocks_.end();

  // run the entire loop here so we can increment last_used_ for all blocks
  for (auto block_iter = set->blocks_.begin(); block_iter != set->blocks_.end(); ++block_iter) {
    block_iter->last_used_++;

    if (block_iter->tag_ == tag) {
      found = block_iter;
    }

  }
  return found;
}

void Cache::updateBlockState(std::vector<Block>::iterator block, long addr, bool is_write) {
  switch (block->state_) {
    case MESI::M:
      // no interconnect events necessary, just stays in modified state
      break;
    case MESI::E:
      if (is_write) {
        // no interconnect event, just silent upgrade
        block->state_ = MESI::M;
      }
      break;
    case MESI::S:
      if (is_write) {
        if (interconnect_ != nullptr) {
          interconnect_->sendBusRdX(cache_id_, addr);
        } else {
          block->state_ = MESI::S;
        }
      }
      // nothing to be done in read case, stays in shared state
      break;
    case MESI::I:
      if (is_write) {
        if (interconnect_ != nullptr) {
          interconnect_->sendBusRdX(cache_id_, addr);
        } else {
          block->state_ = MESI::M;
        }
      } else {
        if (interconnect_ != nullptr) {
          interconnect_->sendBusRd(cache_id_, addr);
        } else {
          block->state_ = MESI::E;
        }
      }
      break;
  }
}

void Cache::evictAndReplace(long tag, std::vector<Set>::iterator set, long addr, bool is_write) {
  // find the block with the largest last_used time stamp
  auto LRU_iter = std::max_element(set->blocks_.begin(), set->blocks_.end(),
      [](auto const &lhs, auto const &rhs) { return lhs.last_used_ <= rhs.last_used_; });

  // update eviction counters
  if (LRU_iter->state_ != MESI::I) {
    if (LRU_iter->dirty_) {
      dirty_blocks_evicted_++;
    }
    eviction_count_++;
  }

  // update the block to refer to the new block that was brought into the cache
  LRU_iter->tag_ = tag;
  LRU_iter->dirty_ = is_write;
  LRU_iter->last_used_ = 0;
  LRU_iter->state_ = MESI::I; // TODO(this should be different)

  updateBlockState(LRU_iter, addr, is_write);
}

std::vector<Block>::iterator Cache::findInCache(long addr) {
  auto pair = readAddr(addr);
  long tag = pair.first;
  auto set_iter = pair.second;

  auto block = findInSet(tag, set_iter);
  assert(block != set_iter->blocks_.end());
  return block;
}

void Cache::performOperation(unsigned long addr, bool is_write) {
  auto pair = readAddr(addr);
  long tag = pair.first;
  auto set_iter = pair.second;

  auto block_end = set_iter->blocks_.end();

  auto block = findInSet(tag, set_iter);
  if (block != block_end) {
    // found the block in the cache
    // the block was accessed, reset its LRU counter
    block->last_used_ = 0;

    if (is_write) {
      block->dirty_ = true;
    }

    // The block will now be brought into the cache if it was initially invalid
    if (block->state_ != MESI::I) hit_count_++;
    else miss_count_++;

    // TODO(samflattery) update stats when updating block state since write to
    // non-exclusive data is considered a write-miss, etc.
    updateBlockState(block, addr, is_write);

    return;
  }

  // we missed in the cache and something needs to be evicted
  miss_count_++;
  evictAndReplace(tag, set_iter, addr, is_write);

}

void Cache::cacheRead(unsigned long addr) {
  return performOperation(addr, /* is_write */ false);
}

void Cache::cacheWrite(unsigned long addr) {
  return performOperation(addr, /* is_write */ true);
}

void Cache::receiveInvalidate(long addr) {
  auto block = findInCache(addr);
  block->state_ = MESI::I;
}

void Cache::receiveFetch(long addr) {
  auto block = findInCache(addr);
  block->state_ = MESI::S;
  // flush data back to directory
}

void Cache::receiveReadMiss(long addr, bool exclusive) {
  auto block = findInCache(addr);
  if (exclusive) block->state_ = MESI::E;
  else block->state_ = MESI::S;
}

void Cache::receiveWriteMiss(long addr) {
  auto block = findInCache(addr);
  block->state_ = MESI::M;
}

void Cache::printState() const {
  std::cout << "id:\t" << cache_id_ << "\n"
            << "s:\t" << s_ << "\n"
            << "S:\t" << S_ << "\n"
            << "E:\t" << E_ << "\n"
            << "b:\t" << b_ << "\n"
            << "B:\t" << B_ << "\n";
}

void Cache::printStats() const {
  std::cout << "cache_id:\t\t"            << cache_id_ << "\n"
            << "miss_count:\t\t"          << miss_count_ << "\n"
            << "hit_count:\t\t"           << hit_count_ << "\n"
            << "eviction_count:\t\t"      << eviction_count_ << "\n"
            << "dirty_blocks_evicted:\t"  << dirty_blocks_evicted_ << "\n";
}
