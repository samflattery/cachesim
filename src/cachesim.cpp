#include <fstream>
#include <iostream>
#include <getopt.h>
#include "cache.h"

void runTrace(std::ifstream &trace, Cache &cache) {
  cache.printState();

  int proc;
  char rw;
  unsigned long addr;

  while (trace >> proc >> rw >> std::hex >> addr) {
    std::cout << proc << " " << rw << " " << std::hex << addr << std::dec << "\n";
    if (rw == 'R') {
      cache.cacheRead(addr);
    } else {
      cache.cacheWrite(addr);
    }
  }

  cache.printStats();
}

int main(int argc, char **argv) {
  std::string usage;
  usage += "-h: help\n";
  usage += "-v: verbose output that displays trace info\n";
  usage += "-s <s>: number of set index bits (S = 2^s)\n";
  usage += "-E <E>: associativity (number of lines per set)\n";
  usage += "-b <b>: number of block bits (B = 2^b)\n";
  usage += "-t <tracefile>: name of memory trace to replay\n";

  char opt;
  std::string filepath;

  int s;
  int E;
  int b;

  // parse command line options
  while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
      switch (opt) {
      case 'h':
        std::cout << usage;
        return 0;
      case 'v':
        printf("verbose\n");
        break;
      case 's':
        s = atol(optarg);
        break;
      case 'E':
        E = atol(optarg);
        break;
      case 'b':
        b = atol(optarg);
        break;
      case 't':
        filepath = std::string(optarg);
        break;
      default:
        std::cerr << usage;
        return 1;
    }
  }

  // -t option is required
  if (filepath == "") {
    std::cerr << "No trace file given\n";
    return 1;
  }

  std::ifstream trace(filepath);
  if (!trace.is_open()) {
    std::cerr << "Invalid trace file\n";
    return 1;
  }

  // run the input trace on the cache
  Cache cache(s, E, b);
  runTrace(trace, cache);

  trace.close();

  return 0;
}
