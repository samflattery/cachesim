#pragma once
#include <cassert>
#include <iostream>

#include "cache_block.h"

enum class MESI { M, E, S, I };

// defines a cache block / cache line metadata and its MESI state
class MESIBlock : public CacheBlock {
 public:
  MESIBlock();
  virtual ~MESIBlock() {}

  virtual bool isValid() override;

  virtual InterconnectAction writeBlock(int numa_node) override;
  virtual InterconnectAction readBlock(int numa_node) override;

  virtual InterconnectAction evictAndReplace(bool is_write, long tag, int new_node) override;

  virtual void invalidate() override;
  virtual void flush() override;
  virtual void receiveReadData(bool exclusive) override;
  virtual void receiveWriteData() override;

 private:
  InterconnectAction updateState(bool is_write) override;

  MESI state_;
};
