#include "mesi_block.h"

MESIBlock::MESIBlock() : CacheBlock(), state_(MESI::I) {}

InterconnectAction MESIBlock::readBlock(int numa_node) {
  last_used_ = 0;
  numa_node_ = numa_node;
  return updateState(false);
}

InterconnectAction MESIBlock::writeBlock(int numa_node) {
  last_used_ = 0;
  dirty_ = true;
  numa_node_ = numa_node;
  return updateState(true);
}

InterconnectAction MESIBlock::updateState(bool is_write) {
  switch (state_) {
    case MESI::M:
      // no interconnect events necessary, just stays in modified state
      hit_count_++;
      return InterconnectAction::NOACTION;
    case MESI::E:
      if (is_write) {
        // no interconnect event, just silent upgrade
        state_ = MESI::M;
      }
      // nothing to be done in read case, still have exclusive access
      hit_count_++;
      return InterconnectAction::NOACTION;
    case MESI::S:
      if (is_write) {
        miss_count_++;
        state_ = MESI::M;
        return InterconnectAction::BUSRDX;
      } else {
        // nothing to be done in read case, stays in shared state
        hit_count_++;
        return InterconnectAction::NOACTION;
      }
    case MESI::I:
      // invalid so it's always a miss regardless of read/write
      miss_count_++;
      if (is_write) {
        state_ = MESI::M;
        return InterconnectAction::BUSRDX;
      } else {
        state_ = MESI::E;
        return InterconnectAction::BUSRD;
      }
    default:
      return InterconnectAction::BUSRD;
  }
}

InterconnectAction MESIBlock::evictAndReplace(bool is_write, long tag, int new_node) {
  if (state_ != MESI::I) {
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
  state_ = MESI::I;

  return updateState(is_write);
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

bool MESIBlock::isValid() { return state_ != MESI::I; }

void MESIBlock::invalidate() {
  invalidations_++;
  state_ = MESI::I;
}

void MESIBlock::flush() {
  assert(state_ == MESI::E || state_ == MESI::M);
  if (state_ == MESI::M) {
    flushes_++;
  }
  state_ = MESI::S;
}

void MESIBlock::receiveReadData(bool exclusive) { state_ = exclusive ? MESI::E : MESI::S; }

void MESIBlock::receiveWriteData() { state_ = MESI::M; }
