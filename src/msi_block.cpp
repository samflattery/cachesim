#include "msi_block.h"

MSIBlock::MSIBlock() : CacheBlock(), state_(MSI::I) {}

InterconnectAction MSIBlock::readBlock(int numa_node) {
  last_used_ = 0;
  numa_node_ = numa_node;
  return updateState(false);
}

InterconnectAction MSIBlock::writeBlock(int numa_node) {
  last_used_ = 0;
  dirty_ = true;
  numa_node_ = numa_node;
  return updateState(true);
}

InterconnectAction MSIBlock::updateState(bool is_write) {
  switch (state_) {
    case MSI::M:
      // no interconnect events necessary, just stays in modified state
      hit_count_++;
      return InterconnectAction::NOACTION;
    case MSI::S:
      if (is_write) {
        miss_count_++;
        state_ = MSI::M;
        return InterconnectAction::BUSRDX;
      } else {
        // nothing to be done in read case, stays in shared state
        hit_count_++;
        return InterconnectAction::NOACTION;
      }
    case MSI::I:
      // invalid so it's always a miss regardless of read/write
      miss_count_++;
      if (is_write) {
        state_ = MSI::M;
        return InterconnectAction::BUSRDX;
      } else {
        state_ = MSI::S;
        return InterconnectAction::BUSRD;
      }
    default:
      return InterconnectAction::BUSRD;
  }
}

InterconnectAction MSIBlock::evictAndReplace(bool is_write, long tag, int new_node) {
  if (state_ != MSI::I) {
    if (dirty_) {
      dirty_evictions_++;
    }
    evictions_++;
  }

  tag_ = tag;
  dirty_ = is_write;
  last_used_ = 0;
  numa_node_ = new_node;
  state_ = MSI::I;

  return updateState(is_write);
}

std::ostream& operator<<(std::ostream& os, const MSI& msi) {
  switch (msi) {
    case MSI::M:
      os << "M";
      break;
    case MSI::S:
      os << "S";
      break;
    case MSI::I:
      os << "I";
      break;
  }
  return os;
}

bool MSIBlock::isValid() { return state_ != MSI::I; }

void MSIBlock::invalidate() {
  invalidations_++;
  state_ = MSI::I;
}

void MSIBlock::flush() {
  assert(state_ == MSI::M);
  state_ = MSI::S;
}

void MSIBlock::receiveReadData([[maybe_unused]] bool exclusive) { state_ = MSI::S; }

void MSIBlock::receiveWriteData() { state_ = MSI::M; }
