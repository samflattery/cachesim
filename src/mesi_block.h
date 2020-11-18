#pragma once
#include <cassert>
#include <iostream>

#include "cache_block.h"

// explicitly set I to default initialize an enum to invalid state
enum class MESI { M = 1, E = 2, S = 3, I = 0 };

// defines a cache block / cache line metadata and its MESI state
class MESIBlock : public CacheBlock {
 public:
  MESIBlock();
  virtual ~MESIBlock() {}

  virtual bool isValid() override;

  virtual InterconnectAction writeBlock() override;
  virtual InterconnectAction readBlock() override;

  virtual InterconnectAction evictAndReplace(bool is_write, long tag) override;

  virtual void invalidate() override;
  virtual void flush() override;
  virtual void receiveReadData(bool exclusive) override;
  virtual void receiveWriteData() override;

 private:
  InterconnectAction updateState(bool is_write) override;

  MESI state_;
};
