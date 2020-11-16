#include "mesi_block.h"

MESIBlock::MESIBlock() :
  CacheBlock(), state_(MESI::I) {}

void MESIBlock::readBlock() {
  last_used_ = 0;
  updateState(false);
}

void MESIBlock::writeBlock() {
  last_used_ = 0;
  updateState(true);
}

InterconnectAction MESIBlock::updateState(bool is_write) {
  switch (state_) {
    case MESI::M:
      // no interconnect events necessary, just stays in modified state
      break;
    case MESI::E:
      if (is_write) {
        // no interconnect event, just silent upgrade
        state_ = MESI::M;
      }
      break;
    case MESI::S:
      if (is_write) {
        state_ = MESI::S;
        return InterconnectAction::BUSRDX;
      }
      // nothing to be done in read case, stays in shared state
      break;
    case MESI::I:
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

void MESIBlock::invalidate() {
  state_ = MESI::I;
}


void MESIBlock::flush() {
}

void MESIBlock::readMiss(bool exclusive) {
  state_ = exclusive ? MESI::E : MESI::S;
}

void MESIBlock::writeMiss() {
  state_ = MESI::M;
}



