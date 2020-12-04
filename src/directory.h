#pragma once

#include <cassert>
#include <unordered_map>
#include <vector>
#include "interconnect.h"
#include "protocols.h"

enum class DirectoryState {
  U = 0,  // uncached - no caches have valid copy
  S = 1,  // shared - at least one cache has data
  EM = 2  // exclusive / modified - one cache owns it
};

// models a line in the directory as a state and a list of presence bits
struct DirectoryLine {
  DirectoryLine(int procs) : state_(DirectoryState::U), owner_(-1) { presence_.resize(procs, false); }
  DirectoryState state_;
  std::vector<bool> presence_;
  int owner_; // in MOESI, the cache with the block in O state
};

class Interconnect;

// models a single directory on a uniprocessor machine
// defines a mapping { block address -> ( state, presence bits ) } where the block address is the
// address with the lower b bits masked off, since they represent block offset bits
class Directory {
 public:
  Directory(int procs, int b, Protocol protocol, int numa_node)
      : procs_(procs),
        block_offset_bits_(b),
        protocol_(protocol),
        numa_node_(numa_node),
        interconnect_(nullptr) {}
  ~Directory() {
    for (auto &[addr, line] : directory_) delete line;
  }

  void connectToInterconnect(Interconnect *interconnect);

  void receiveBusRd(int cache_id, unsigned long address);
  void receiveBusRdX(int cache_id, unsigned long address);
  void receiveEviction(int cache_id, unsigned long address);
  // receive data back from a cache after a fetch call
  void receiveData(int cache_id, unsigned long address);
  // receive a message to broadcast new data from an updated O block to all other sharers
  void receiveBroadcast(int cache_id, unsigned long address);

 private:
  // translate a full address into a block address by zeroing lowest block_offset_bits_ bits
  long getAddr(unsigned long addr);

  // return the line of a given address, constructing a new line if none is in the directory
  DirectoryLine *getLine(unsigned long addr);

  // find the owner of an EM line
  int findOwnerEM(DirectoryLine *line);

  // send invalidate messages to all sharers of a line except new_owner
  void invalidateSharers(DirectoryLine *line, int new_owner, unsigned long addr);

  int procs_;
  int block_offset_bits_;
  Protocol protocol_;
  int numa_node_;
  std::unordered_map<long, DirectoryLine *> directory_;

  // the interconnect through which messages are sent to the caches
  // should never be null after initialization of the interconnect since if we have a directory
  // we must have an interconnect
  Interconnect *interconnect_;
};
