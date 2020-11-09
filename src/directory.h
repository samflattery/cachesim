#pragma once

#include <unordered_map>
#include <vector>
#include "interconnect.h"

#define ADDR_LEN 64L

enum class DirectoryState {
  U = 0, // uncached - no caches have valid copy
  S = 1, // shared - at least one cache has data
  EM = 2 // exclusive / modified - one cache owns it
};

// models a line in the directory as a state and a list of presence bits
struct DirectoryLine {
  DirectoryLine(int procs) { presence_.resize(procs, false); }
  DirectoryState state_;
  std::vector<bool> presence_;
};

class Interconnect;

// models a single directory on a uniprocessor machine
// defines a mapping { block address -> ( state, presence bits ) } where the block address is the
// address with the lower b bits masked off, since they represent block offset bits
class Directory {
public:
  Directory(int procs, int b) : procs_(procs), block_offset_bits_(b) {}
  ~Directory() { for (auto &[addr, line] : directory_) delete line; }

  void connectToInterconnect(Interconnect *interconnect);

  void receiveBusRd(int cache_id, long address);
  void receiveBusRdX(int cache_id, long address);

  /* void sendInvalidate(int cache_id, long address); */
  /* void requestData(int cache_id, long address); */
  /* void sendData(int cache_id, long address); */

private:
  // translate a full address into a block address by zeroing lowest block_offset_bits_ bits
  long getAddr(long addr);

  // return the line of a given address, constructing a new line if none is in the directory
  DirectoryLine *getLine(long addr);

  int procs_;
  int block_offset_bits_;
  std::unordered_map<long, DirectoryLine *> directory_;
  Interconnect *interconnect_;
};
