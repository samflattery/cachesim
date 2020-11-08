#include <fstream>
#include <iostream>
#include <getopt.h>

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

  while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
      switch (opt) {
      case 'h':
        std::cout << usage;
        return 0;
      case 'v':
        printf("verbose\n");
        break;
      case 's':
        /* s = atol(optarg); */
        /* S = 1 << s; // S = 2 ^ s */
        break;
      case 'E':
        /* E = atol(optarg); */
        break;
      case 'b':
        /* b = atol(optarg); */
        /* B = 1 << b; // B = 2 ^ b; */
        break;
      case 't':
        filepath = std::string(optarg);
        break;
      default:
        std::cerr << usage;
        return 1;
    }
  }

  if (filepath == "") {
    std::cerr << "No trace file given\n";
    return 1;
  }

  std::ifstream trace(filepath);
  if (!trace.is_open()) {
    std::cerr << "Invalid trace file\n";
    return 1;
  }

  std::string processor;
  std::string rw;
  std::string address;

  while (trace >> processor >> rw >> address) {
    std::cout << processor << " " << rw << " " << address << "\n";
  }

  trace.close();

  return 0;
}
