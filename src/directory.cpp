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
  long num_bits = ADDR_LEN - block_offset_bits_ ;
  long block_mask = 0L;
  for (int i = 0; i < block_offset_bits_; i++) {
      block_mask |= (1L << i);
  }
  return addr | block_mask;
}

void Directory::receiveBusRd(int cache_id, long address) {
  long addr = getAddr(addr);
  DirectoryLine *line = getLine(addr);

  switch (line->state_) {
    case DirectoryState::U:
      // send the cache the data from memory, now in exclusive state
      line->state_ = DirectoryState::EM;
      break;
    case DirectoryState::S:
      // send the cache the data from memory, no transition needed, add proc to shared set
      line->presence_[cache_id] = true;
      break;
    case DirectoryState::EM:
      // downgrade the owner and get him to flush, now in shared state
      line->state_ = DirectoryState::S;
      break;
  }
}

void Directory::connectToInterconnect(Interconnect *interconnect) {
  interconnect_ = interconnect;
}

void Directory::receiveBusRdX(int cache_id, long address) {
  long addr = getAddr(addr);
  DirectoryLine *line = getLine(addr);

  switch (line->state_) {
    case DirectoryState::U:
      // send the cache the data from memory, now in exclusive state
      line->state_ = DirectoryState::EM;
      break;
    case DirectoryState::S:
      // invalidate all sharers, send the owner the memory block
      line->state_ = DirectoryState::EM;
      break;
    case DirectoryState::EM:
      // invalidate the old owner
      line->state_ = DirectoryState::EM;
      break;
  }

}
