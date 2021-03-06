#pragma once
#include <chrono>
#include <string>
// times are given in nanoseconds

// taken from /sys/devices/system/node/node0/distance on andrew
// 10 20 means node 0 is distance 10 from itself and 20 from node 1 (i.e. twice as big a latency)
static const int NUMA_DISTANCE = 2;

// L1 cache hit time
// https://stackoverflow.com/questions/3142779/how-much-time-does-it-take-to-fetch-one-word-from-memory
static const int CACHE_LATENCY = 1;

// main memory read/write time
// assumes memory writes take the same amount of time as reads
static const int MEMORY_LATENCY = 100;

// directory -> cache and cache->directory latency
// Intel's interconnect has operates at ~5GT/s (gigatransfers per second), so each transfer takes
// about 1/(10^9)s = 1ns
// https://en.wikipedia.org/wiki/Intel_QuickPath_Interconnect#Frequency_specifications
// this is probably not super accurate but we can change it if we get better info
static const int LOCAL_INTERCONNECT_LATENCY = 1;

// NUMA node -> NUMA node latency
static constexpr int GLOBAL_INTERCONNECT_LATENCY = LOCAL_INTERCONNECT_LATENCY * NUMA_DISTANCE;

std::string outputLatency(size_t x);
