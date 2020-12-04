#include "directory.h"

DirectoryLine *Directory::getLine(unsigned long addr) {
  auto it = directory_.find(addr);
  if (it == directory_.end()) {
    DirectoryLine *line = new DirectoryLine(procs_);
    directory_.insert({addr, line});
    return line;
  }
  return it->second;
}

long Directory::getAddr(unsigned long addr) {
  long block_mask = 0L;
  for (int i = ADDR_LEN - 1; i >= block_offset_bits_; i--) {
    block_mask |= (1L << i);
  }
  return addr & block_mask;
}

void Directory::connectToInterconnect(Interconnect *interconnect) { interconnect_ = interconnect; }

void Directory::receiveData(int cache_id, unsigned long addr) {}

int Directory::findOwner(DirectoryLine *line) {
  assert(line->state_ == DirectoryState::EM);
  for (size_t i = 0; i < line->presence_.size(); ++i) {
    if (line->presence_[i]) return i;
  }
  // should never get here if EM
  assert(false);
  return -1;
}

void Directory::invalidateSharers(DirectoryLine *line, int new_owner, unsigned long addr) {
  assert(line->state_ == DirectoryState::S);
  for (size_t i = 0; i < line->presence_.size(); ++i) {
    if (i == (size_t)new_owner) continue;
    if (line->presence_[i]) interconnect_->sendInvalidate(i, addr);
  }
}

void Directory::receiveEviction(int cache_id, unsigned long address) {
  unsigned long addr = getAddr(address);
  DirectoryLine *line = getLine(addr);

  // unset the presence bit for that cache
  assert(line->presence_[cache_id]);
  line->presence_[cache_id] = 0;

  int num_sharers = 0;
  switch (line->state_) {
    case DirectoryState::U:
      // should not be possible to evict without any sharers
      assert(false);
      break;
    case DirectoryState::S:
      // count the number of sharers and demote if there are none left
      // don't promote to EM if there is only one sharer left since the last remaining cache
      // doesn't know it has exclusive access
      num_sharers = std::count(line->presence_.begin(), line->presence_.end(), true);
      if (num_sharers == 0) line->state_ = DirectoryState::U;

      break;
    case DirectoryState::EM:
      // there are now no caches sharing this line
      line->state_ = DirectoryState::U;
      break;
  }
}

void Directory::receiveBusRd(int cache_id, unsigned long address) {
  unsigned long addr = getAddr(address);
  DirectoryLine *line = getLine(addr);

  switch (line->state_) {
    case DirectoryState::U:
      // send the cache the data from memory, now in exclusive state
      interconnect_->sendReadMiss(cache_id, addr, /* exclusive */ true);
      line->presence_[cache_id] = true;

      // don't promote read requests to EM in MSI since caches don't know they have exclusive read
      // access
      if (protocol_ == Protocol::MESI) {
        line->state_ = DirectoryState::EM;
      } else if (protocol_ == Protocol::MSI) {
        line->state_ = DirectoryState::S;
      }
      break;
    case DirectoryState::S:
      // send the cache the data from memory, no transition needed, add proc to
      // shared set
      interconnect_->sendReadMiss(cache_id, addr, /* exclusive */ false);
      line->presence_[cache_id] = true;
      break;
    case DirectoryState::EM:
      // downgrade the owner and get him to flush, now in shared state
      {
        int owner_id = findOwner(line);
        interconnect_->sendFetch(owner_id, {addr, numa_node_});
        line->presence_[cache_id] = true;
        interconnect_->sendReadMiss(cache_id, addr, /* exclusive */ false);
      }

      line->state_ = DirectoryState::S;
      break;
    default:
      assert(false);
      break;
  }
}

void Directory::receiveBusRdX(int cache_id, unsigned long address) {
  unsigned long addr = getAddr(address);
  DirectoryLine *line = getLine(addr);

  switch (line->state_) {
    case DirectoryState::U:
      // send the cache the data from memory, now in exclusive state
      interconnect_->sendWriteMiss(cache_id, addr);
      line->presence_[cache_id] = true;
      line->state_ = DirectoryState::EM;
      break;
    case DirectoryState::S:
      // invalidate all sharers, except the cache sending the BusRdX, send the
      // owner the memory block
      invalidateSharers(line, cache_id, addr);
      interconnect_->sendWriteMiss(cache_id, addr);
      line->presence_[cache_id] = true;
      line->state_ = DirectoryState::EM;
      break;
    case DirectoryState::EM:
      // invalidate the old owner and send data to new owner
      int owner_id = findOwner(line);
      interconnect_->sendFetch(owner_id, {addr, numa_node_});
      interconnect_->sendInvalidate(owner_id, addr);
      line->presence_[owner_id] = false;

      line->presence_[cache_id] = true;
      interconnect_->sendWriteMiss(cache_id, addr);
      line->state_ = DirectoryState::EM;
      break;
  }
}
