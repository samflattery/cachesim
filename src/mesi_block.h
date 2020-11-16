#pragma once
#include <iostream>

#include "cache_block.h"

// explicitly set I to default initialize an enum to invalid state
enum class MESI {
  M = 1,
  E = 2,
  S = 3,
  I
};

// defines a cache block / cache line metadata and its MESI state
class MESIBlock : public CacheBlock {
public:
  MESIBlock();
  virtual ~MESIBlock() {}

  virtual void writeBlock() override;
  virtual void readBlock() override;

  virtual InterconnectAction evictAndReplace(bool is_write, long tag) override;

  virtual void invalidate() override;
  virtual void flush() override;
  virtual void readMiss(bool exclusive) override;
  virtual void writeMiss() override;

private:
  InterconnectAction updateState(bool is_write) override;

  MESI state_;
};
