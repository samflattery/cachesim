#include <getopt.h>
#include <fstream>
#include <iostream>
#include "cache.h"
#include "directory.h"
#include "interconnect.h"

void runSimulation(int s, int E, int b, std::ifstream &trace, int procs, bool verbose) {
  std::vector<Cache> caches;
  for (int i = 0; i < procs; ++i) {
    caches.push_back(Cache(i, s, E, b));
  }

  std::cout << "Running simulation with cache settings:\n";
  caches[0].printState();
  Directory directory(procs, b);

  Interconnect *interconnect;
  if (procs > 1) {
    interconnect = new Interconnect(&caches, &directory, verbose);
  } else {
    interconnect = nullptr;
  }

  int proc;
  char rw;
  unsigned long addr;

  while (trace >> proc >> rw >> std::hex >> addr >> std::dec) {
    if (verbose) {
      std::cout << "\n" << proc << " " << rw << " " << std::hex << addr << std::dec << "\n";
    }

    if (rw == 'R') {
      caches[proc].cacheRead(addr);
    } else {
      caches[proc].cacheWrite(addr);
    }
  }

  for (const auto &cache : caches) {
    cache.printStats();
  }
  if (interconnect != nullptr) {
    interconnect->printStats();
    delete interconnect;
  }
}

int main(int argc, char **argv) {
  std::string usage;
  usage += "-h: help\n";
  usage += "-v: verbose output that displays trace info\n";
  usage += "-s <s>: number of set index bits (S = 2^s)\n";
  usage += "-E <E>: associativity (number of lines per set)\n";
  usage += "-b <b>: number of block bits (B = 2^b)\n";
  usage += "-t <tracefile>: name of memory trace to replay\n";
  usage += "-p <processors>: number of processors\n";

  char opt;
  std::string filepath;

  int s;
  int E;
  int b;
  int procs;
  bool verbose = false;

  // parse command line options
  while ((opt = getopt(argc, argv, "hvs:E:b:t:p:")) != -1) {
    switch (opt) {
      case 'h':
        std::cout << usage;
        return 0;
      case 'v':
        verbose = true;
        break;
      case 's':
        s = atoi(optarg);
        break;
      case 'E':
        E = atoi(optarg);
        break;
      case 'b':
        b = atoi(optarg);
        break;
      case 't':
        filepath = std::string(optarg);
        break;
      case 'p':
        procs = atoi(optarg);
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
  runSimulation(s, E, b, trace, procs, verbose);

  trace.close();

  return 0;
}
