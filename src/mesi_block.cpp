#include "mesi_block.h"

MESIBlock::MESIBlock() : CacheBlock(), state_(MESI::I) {}

InterconnectAction MESIBlock::readBlock() {
  last_used_ = 0;
  return updateState(false);
}

InterconnectAction MESIBlock::writeBlock() {
  last_used_ = 0;
  dirty_ = true;
  return updateState(true);
}

InterconnectAction MESIBlock::updateState(bool is_write) {
  switch (state_) {
    case MESI::M:
      // no interconnect events necessary, just stays in modified state
      hit_count_++;
      break;
    case MESI::E:
      if (is_write) {
        // no interconnect event, just silent upgrade
        state_ = MESI::M;
      }
      hit_count_++;
      break;
    case MESI::S:
      if (is_write) {
        // TODO(samflattery) consider this a miss as the upgrade is required?
        miss_count_++;
        state_ = MESI::S;
        return InterconnectAction::BUSRDX;
      }
      hit_count_++;
      // nothing to be done in read case, stays in shared state
      break;
    case MESI::I:
      miss_count_++;
      if (is_write) {
        state_ = MESI::M;
        return InterconnectAction::BUSRDX;
      } else {
        state_ = MESI::E;
        return InterconnectAction::BUSRD;
      }
      break;
  }
  return InterconnectAction::NOACTION;
}

InterconnectAction MESIBlock::evictAndReplace(bool is_write, long tag) {
  if (state_ != MESI::I) {
    if (dirty_) {
      dirty_evictions_++;
    }
    evictions_++;
  }

  tag_ = tag;
  dirty_ = is_write;
  last_used_ = 0;
  state_ = MESI::I;

  return updateState(is_write);
}

bool MESIBlock::isValid() { return state_ != MESI::I; }

void MESIBlock::invalidate() { state_ = MESI::I; }

void MESIBlock::flush() { state_ = MESI::S; }

void MESIBlock::readMiss(bool exclusive) {
  state_ = exclusive ? MESI::E : MESI::S;
}

void MESIBlock::writeMiss() { state_ = MESI::M; }
