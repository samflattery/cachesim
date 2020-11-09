#include "directory.h"

DirectoryLine *Directory::getLine(long addr) {
  auto it = directory_.find(addr);
  if (it == directory_.end()) {
    DirectoryLine *line = new DirectoryLine(procs_);
    directory_.insert({addr, line});
    return line;
  }
  return it->second;
}

long Directory::getAddr(long addr) {
  long block_mask = 0L;
  for (int i = 0; i < block_offset_bits_; i++) {
      block_mask |= (1L << i);
  }
  return addr | block_mask;
}

void Directory::connectToInterconnect(Interconnect *interconnect) {
  interconnect_ = interconnect;
}

int Directory::findOwner(DirectoryLine *line) {
  assert(line->state_ == DirectoryState::EM);
  for (size_t i = 0; i < line->presence_.size(); ++i) {
    if (line->presence_[i]) return i;
  }
  // should never get here if EM
  assert(false);
  return -1;
}

void Directory::invalidateSharers(DirectoryLine *line, long addr) {
  assert(line->state_ == DirectoryState::S);
  for (size_t i = 0; i < line->presence_.size(); ++i) {
    if (line->presence_[i]) interconnect_->sendInvalidate(i, addr);
  }
}

void Directory::receiveBusRd(int cache_id, long address) {
  long addr = getAddr(address);
  DirectoryLine *line = getLine(addr);

  switch (line->state_) {
    case DirectoryState::U:
      // send the cache the data from memory, now in exclusive state
      interconnect_->sendReadMiss(cache_id, addr, /* exclusive */ true);
      line->presence_[cache_id] = true;
      line->state_ = DirectoryState::EM;
      break;
    case DirectoryState::S:
      // send the cache the data from memory, no transition needed, add proc to shared set
      interconnect_->sendReadMiss(cache_id, addr, /* exclusive */ false);
      line->presence_[cache_id] = true;
      break;
    case DirectoryState::EM:
      // downgrade the owner and get him to flush, now in shared state
      int owner_id = findOwner(line);
      interconnect_->sendFetch(owner_id, addr);
      line->presence_[cache_id] = true;
      interconnect_->sendReadMiss(cache_id, addr, /* exclusive */ false);

      line->state_ = DirectoryState::S;
      break;
  }
}

void Directory::receiveBusRdX(int cache_id, long address) {
  long addr = getAddr(address);
  DirectoryLine *line = getLine(addr);

  switch (line->state_) {
    case DirectoryState::U:
      // send the cache the data from memory, now in exclusive state
      interconnect_->sendWriteMiss(cache_id, addr);
      line->presence_[cache_id] = true;
      line->state_ = DirectoryState::EM;
      break;
    case DirectoryState::S:
      // invalidate all sharers, send the owner the memory block
      invalidateSharers(line, addr);
      interconnect_->sendWriteMiss(cache_id, addr);
      line->presence_[cache_id] = true;
      line->state_ = DirectoryState::EM;
      break;
    case DirectoryState::EM:
      // invalidate the old owner and send data to new owner
      int owner_id = findOwner(line);
      interconnect_->sendFetch(owner_id, addr);
      interconnect_->sendInvalidate(owner_id, addr);
      line->presence_[owner_id] = false;

      line->presence_[cache_id] = true;
      interconnect_->sendWriteMiss(cache_id, addr);
      line->state_ = DirectoryState::EM;
      break;
  }

}
