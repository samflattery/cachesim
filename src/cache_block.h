#pragma once

enum class InterconnectAction { NOACTION, BUSRD, BUSRDX, FLUSH };

// Abstract class for a cache block
class CacheBlock {
 public:
  CacheBlock()
      : dirty_(false),
        tag_(0L),
        last_used_(0L),
        flushes_(0L),
        hit_count_(0L),
        evictions_(0L),
        miss_count_(0L),
        invalidations_(0L),
        dirty_evictions_(0L) {}

  virtual ~CacheBlock() {}

  // Get metadata about the block
  bool isDirty() const { return dirty_; }
  long getLastUsed() const { return last_used_; }
  long getTag() const { return tag_; }
  void incrLastUsed() { last_used_++; }
  int getNumaNode() { return numa_node_; }

  // Get stats on the block
  size_t getHitCount() { return hit_count_; }
  size_t getMissCount() { return miss_count_; }
  size_t getFlushCount() { return flushes_; }
  size_t getEvictionCount() { return evictions_; }
  size_t getInvalidationCount() { return invalidations_; }
  size_t getDirtyEvictionCount() { return dirty_evictions_; }

  // Is this block valid
  virtual bool isValid() = 0;

  // Read and write the block
  virtual InterconnectAction writeBlock(int numa_node) = 0;
  virtual InterconnectAction readBlock(int numa_node) = 0;

  // Evict a block and replace with new tag
  virtual InterconnectAction evictAndReplace(bool is_write, long tag, int new_node) = 0;

  // interconnect invalidated my line
  virtual void invalidate() = 0;

  // flush to main memory
  virtual void flush() = 0;

  // response from interconnect after read miss
  virtual void receiveReadData([[maybe_unused]] bool exclusive) = 0;

  // response from interconnect after write miss
  virtual void receiveWriteData() = 0;

 protected:
  virtual InterconnectAction updateState(bool is_write) = 0;

  bool dirty_;
  long tag_;
  long last_used_;  // used to track LRU block in a set

  // the NUMA node that the memory stored in this block belongs to so when we evict we know where
  // to send the eviction message to
  int numa_node_;

  // number of times the block has been flushed to memory
  size_t flushes_;

  size_t hit_count_;
  size_t evictions_;
  size_t miss_count_;
  size_t invalidations_;
  size_t dirty_evictions_;

};
