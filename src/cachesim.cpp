#include <getopt.h>
#include <fstream>
#include <iostream>

#include "cache.h"
#include "directory.h"
#include "interconnect.h"
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

void runSimulation(int s, int E, int b, std::ifstream &trace, int procs, int numa_nodes,
                   Protocol protocol, bool verbose) {
  std::vector<NUMA *> nodes;
  for (int i = 0; i < numa_nodes; ++i) {
    NUMA *node = new NUMA(procs, numa_nodes, i, s, E, b, protocol, verbose);
    nodes.push_back(node);
  }

  setupInterconnects(nodes);

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
  }

  for (auto node : nodes) {
    node->printStats();
    delete node;
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
  usage += "-n <numa nodes>: number of NUMA nodes\n";
  usage += "-m <MSI | MESI | MOESI>: the cache protocol to use\n";

  char opt;
  std::string filepath;
  std::string protocol;

  int s;
  int E;
  int b;
  int procs;
  int numa_nodes = 1;
  bool verbose = false;

  // parse command line options
  while ((opt = getopt(argc, argv, "hvs:E:b:t:p:n:m:")) != -1) {
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
  }else {
    std::cerr << "Invalid protocol, must be <MSI|MESI>\n";
    return 1;
  }

  std::ifstream trace(filepath);
  if (!trace.is_open()) {
    std::cerr << "Invalid trace file\n";
    return 1;
  }

  // run the input trace on the cache
  runSimulation(s, E, b, trace, procs, numa_nodes, prot, verbose);

  trace.close();

  return 0;
}
