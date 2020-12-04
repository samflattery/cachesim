#include "moesi_block.h"

MOESIBlock::MOESIBlock() : CacheBlock(), state_(MOESI::I) {}

InterconnectAction MOESIBlock::readBlock(int numa_node) {
  last_used_ = 0;
  numa_node_ = numa_node;
  return updateState(false);
}

InterconnectAction MOESIBlock::writeBlock(int numa_node) {
  last_used_ = 0;
  dirty_ = true;
  numa_node_ = numa_node;
  return updateState(true);
}

InterconnectAction MOESIBlock::updateState(bool is_write) {
  switch (state_) {
    case MOESI::M:
      // no interconnect events necessary, just stays in modified state
      hit_count_++;
      return InterconnectAction::NOACTION;
    case MOESI::O:
      // count this as a hit even in write case so the stats look slightly better
      hit_count_++;
      if (is_write) {
        // the new value must be broadcast to all sharing caches
        return InterconnectAction::BROADCAST;
      } else {
        return InterconnectAction::NOACTION;
      }
      break;
    case MOESI::E:
      if (is_write) {
        // no interconnect event, just silent upgrade
        state_ = MOESI::M;
      }
      // nothing to be done in read case, still have exclusive access
      hit_count_++;
      return InterconnectAction::NOACTION;
    case MOESI::S:
      if (is_write) {
        // count this as a write miss since the processor has to get the rights to write it
        miss_count_++;
        state_ = MOESI::M;
        return InterconnectAction::BUSRDX;
      } else {
        // nothing to be done in read case, stays in shared state
        hit_count_++;
        return InterconnectAction::NOACTION;
      }
    case MOESI::I:
      // invalid so it's always a miss regardless of read/write
      miss_count_++;
      if (is_write) {
        state_ = MOESI::M;
        return InterconnectAction::BUSRDX;
      } else {
        state_ = MOESI::E;
        return InterconnectAction::BUSRD;
      }
    default:
      return InterconnectAction::BUSRD;
  }
}

InterconnectAction MOESIBlock::evictAndReplace(bool is_write, long tag, int new_node) {
  if (state_ != MOESI::I) {
    // evicting a dirty block causes a flush to memory
    if (dirty_) {
      flushes_++;
      dirty_evictions_++;
    }
    evictions_++;
  }

  tag_ = tag;
  dirty_ = is_write;
  last_used_ = 0;
  numa_node_ = new_node;
  state_ = MOESI::I;

  return updateState(is_write);
}

std::ostream& operator<<(std::ostream& os, const MOESI& moesi) {
  switch (moesi) {
    case MOESI::M:
      os << "M";
      break;
    case MOESI::O:
      os << "O";
      break;
    case MOESI::E:
      os << "E";
      break;
    case MOESI::S:
      os << "S";
      break;
    case MOESI::I:
      os << "I";
      break;
  }
  return os;
}

bool MOESIBlock::isValid() { return state_ != MOESI::I; }

void MOESIBlock::invalidate() {
  invalidations_++;
  state_ = MOESI::I;
}

void MOESIBlock::fetch() {
  assert(state_ == MOESI::O || state_ == MOESI::E || state_ == MOESI::M);
  if (state_ == MOESI::M) {
    // we now own the data and service other cache's requests, so we don't have to flush
    state_ = MOESI::O;
  } else if (state_ == MOESI::E){
    // if it's just exclusive, just downgrade to shared for free
    state_ = MOESI::S;
  }
}

void MOESIBlock::receiveReadData(bool exclusive) { state_ = exclusive ? MOESI::E : MOESI::S; }

void MOESIBlock::receiveWriteData() { state_ = MOESI::M; }
