#pragma once
#include <cassert>
#include <iostream>

#include "cache_block.h"

// explicitly set I to default initialize an enum to invalid state
enum class MSI { M, S, I };

// defines a cache block / cache line metadata and its MSI state
class MSIBlock : public CacheBlock {
 public:
  MSIBlock();
  virtual ~MSIBlock() {}

  virtual bool isValid() override;

  virtual InterconnectAction writeBlock(int numa_node) override;
  virtual InterconnectAction readBlock(int numa_node) override;

  virtual InterconnectAction evictAndReplace(bool is_write, long tag, int new_node) override;

  virtual void invalidate() override;
  virtual void flush() override;
  virtual void receiveReadData([[maybe_unused]] bool exclusive) override;
  virtual void receiveWriteData() override;

 private:
  InterconnectAction updateState(bool is_write) override;

  MSI state_;
};
