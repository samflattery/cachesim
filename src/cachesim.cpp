#include <getopt.h>
#include <fstream>
#include <iostream>

#include "cache.h"
#include "directory.h"
#include "interconnect.h"
#include "latencies.h"
#include "numa_node.h"
#include "protocols.h"

// returns the NUMA node proc is on
int procToNode(int proc, int num_procs, int numa_nodes) { return proc / (num_procs / numa_nodes); }

// connect all of the NUMA regions interconnects, so node1->interconnect_[i] ==
// node->interconnect_[i]
void setupInterconnects(std::vector<NUMA *> &nodes) {
  for (auto node : nodes) {
    for (auto node1 : nodes) {
      node1->connectInterconnect(node->getInterconnect());
    }
  }
}

void printAggregateStats(std::vector<NUMA *> &nodes, int total_events, bool skip0) {
  Stats stats;
  for (const auto &node : nodes) {
    stats += node->getStats(skip0);
  }
  if (skip0) {
    std::cout << "\t** Aggregate Stats Without Processor 0 ***" << std::endl;
  } else {
    std::cout << "\t** Aggregate Stats ***" << std::endl;
  }

  std::cout << std::endl << "Total Reads/Writes:\t\t" << total_events << std::endl << std::endl;

  std::cout << "Caches" << std::endl
            << "------" << std::endl
            << "Total Hits:\t\t" << stats.hits_ << std::endl
            << "Total Misses:\t\t" << stats.misses_ << std::endl
            << "Total Flushes:\t\t" << stats.flushes_ << std::endl
            << "Total Evictions: \t" << stats.evictions_ << std::endl
            << "Total Dirty Evictions: \t" << stats.dirty_evictions_ << std::endl
            << "Total Invalidations: \t" << stats.invalidations_ << std::endl
            << std::endl;

  std::cout << "Interconnects" << std::endl
            << "-------------" << std::endl
            << "Total Local Interconnect Events: \t" << stats.local_interconnect_ << std::endl
            << "Total Global Interconnect Events: \t" << stats.global_interconnect_ << std::endl
            << std::endl;

  std::cout << "Memory" << std::endl
            << "------" << std::endl
            << "Total Memory Reads: \t" << stats.memory_reads_ << std::endl
            << std::endl;

  std::cout << "Latencies" << std::endl
            << "---------" << std::endl
            << "Cache Access Latency:\t\t" << outputLatency(stats.hits_ * CACHE_LATENCY) << "\n"
            << "Memory Read Latency:\t\t" << outputLatency(stats.memory_reads_ * MEMORY_LATENCY)
            << "\n"
            << "Memory Write Latency:\t\t" << outputLatency(stats.memory_writes_ * MEMORY_LATENCY)
            << "\n"
            << "Memory Access Latency:\t\t"
            << outputLatency((stats.memory_reads_ + stats.memory_writes_) * MEMORY_LATENCY) << "\n"
            << "Local Interconnect Latency:\t"
            << outputLatency(stats.local_interconnect_ * LOCAL_INTERCONNECT_LATENCY) << "\n"
            << "Global Interconnect Latency:\t"
            << outputLatency(stats.global_interconnect_ * GLOBAL_INTERCONNECT_LATENCY) << "\n"
            << std::endl
            << std::endl;
}

void runSimulation(int s, int E, int b, std::ifstream &trace, int procs, int numa_nodes,
                   Protocol protocol, bool individual, bool aggregate, bool aggregate_skip0,
                   bool verbose) {
  std::vector<NUMA *> nodes;
  for (int i = 0; i < numa_nodes; ++i) {
    NUMA *node = new NUMA(procs, numa_nodes, i, s, E, b, protocol, verbose);
    nodes.push_back(node);
  }

  setupInterconnects(nodes);
  int total_events = 0;
  int total_events_wo_0 = 0;

  int proc;     // the requesting proc
  char rw;      // whether it is a read or write
  int node_id;  // the node where addr resides
  unsigned long addr;

  while (trace >> proc >> rw >> std::hex >> addr >> std::dec >> node_id) {
    if (verbose) {
      std::cout << "\n"
                << proc << " " << rw << " " << std::hex << addr << std::dec << " " << node_id
                << "\n";
    }

    if (node_id >= numa_nodes or proc >= procs) {
      std::cout << "Invalid value of p or n for given trace\n";
      exit(1);
    }

    // get the NUMA node that the requesting proc belongs to
    int proc_node = procToNode(proc, procs, numa_nodes);
    if (rw == 'R') {
      nodes[proc_node]->cacheRead(proc, addr, node_id);
    } else {
      nodes[proc_node]->cacheWrite(proc, addr, node_id);
    }
    if (proc != 0) {
      total_events_wo_0++;
    }
    total_events++;
  }

  if (aggregate) {
    printAggregateStats(nodes, total_events, false);
  }

  if (aggregate_skip0) {
    printAggregateStats(nodes, total_events_wo_0, true);
  }

  for (auto node : nodes) {
    if (individual) {
      node->printStats();
    }
    delete node;
  }
}

int main(int argc, char **argv) {
  std::string usage;
  usage += "-t <tracefile>: name of memory trace to replay\n";
  usage += "-p <processors>: number of processors used to generate trace\n";
  usage += "-n <numa nodes>: number of NUMA nodes used to generate trace\n";
  usage += "-m <MSI | MESI | MOESI>: the cache protocol to use, default is MESI\n";
  usage += "-s <s>: number of set index bits (S = 2^s)\n";
  usage += "-E <E>: associativity (number of lines per set)\n";
  usage += "-b <b>: number of block bits (B = 2^b)\n";
  usage += "-v: verbose output that displays trace info\n";
  usage += "-a: display aggregate stats\n";
  usage += "-A: display aggregate stats with those of proc 0 omitted\n";
  usage += "-i: display individual stats (i.e.per cache, per NUMA node)\n";
  usage += "-h: help\n";

  char opt;
  std::string filepath;
  std::string protocol;

  // default to Intel L1 cache
  int s = 6;
  int E = 8;
  int b = 6;
  int procs = 1;
  int numa_nodes = 1;
  bool verbose = false;
  bool aggregate = false;
  bool aggregate_skip0 = false;
  bool individual = false;

  // parse command line options
  while ((opt = getopt(argc, argv, "hvaAis:E:b:t:p:n:m:")) != -1) {
    switch (opt) {
      case 'h':
        std::cout << usage;
        return 0;
      case 'v':
        verbose = true;
        break;
      case 'a':
        aggregate = true;
        break;
      case 'A':
        aggregate_skip0 = true;
        break;
      case 'i':
        individual = true;
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
      case 'n':
        numa_nodes = atoi(optarg);
        break;
      case 'm':
        protocol = std::string(optarg);
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

  Protocol prot;
  if (protocol == "" || protocol == "MESI") {
    // default to MESI
    prot = Protocol::MESI;
  } else if (protocol == "MSI") {
    prot = Protocol::MSI;
  } else if (protocol == "MOESI") {
    prot = Protocol::MOESI;
  } else {
    std::cerr << "Invalid protocol, must be <MSI|MESI|MOESI>\n";
    return 1;
  }

  std::ifstream trace(filepath);
  if (!trace.is_open()) {
    std::cerr << "Invalid trace file\n";
    return 1;
  }

  // run the input trace on the cache
  runSimulation(s, E, b, trace, procs, numa_nodes, prot, individual, aggregate, aggregate_skip0,
                verbose);

  trace.close();

  return 0;
}
