/*
 * Copyright 2002-2020 Intel Corporation.
 *
 * This software is provided to you as Sample Source Code as defined in the accompanying
 * End User License Agreement for the Intel(R) Software Development Products ("Agreement")
 * section 1.L.
 *
 * This software and the related documents are provided as is, with no express or implied
 * warranties, other than those that are expressly stated in the License.
 */

/*
 *  This file contains an ISA-portable PIN tool for tracing memory accesses.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include <fstream>
#include <map>
#include <iostream>
#include <sstream>
#include <vector>

#include "pin.H"




FILE *trace;
PIN_LOCK lock;

typedef std::map<unsigned long, int> TULongIntMap;
typedef std::pair<unsigned long, int> TULongIntPair;


int pagesize; // kernel pagesize;
TULongIntMap addressToNumaMap;

int getNumaNode(VOID *addr) {
    // round address to nearest page
    unsigned long address = (unsigned long)addr / pagesize * pagesize;
    std::map<unsigned long,int>::iterator it;
    if ((it = addressToNumaMap.find(address)) != addressToNumaMap.end()) {
        // if we already have this address, then report its numa node
        return it->second;
    } else {
        // open the map and numa_maps files and assign a numa node to the addess
        // in a weighted random manner
        std::ifstream maps, numa_maps;
        char buf[1024];
        sprintf(buf, "/proc/%d/maps", getpid());
        maps.open(buf, std::ios::in);

        sprintf(buf, "/proc/%d/numa_maps", getpid());
        numa_maps.open(buf, std::ios::in);
        if (!maps || !numa_maps) {
            exit(1);
        }
        int currentNode = 0;
        while(maps.getline(buf, 1024)) {
            unsigned long minAddr, maxAddr;
            sscanf(buf, "%lx-%lx ", &minAddr, &maxAddr);
            if (minAddr <= address && address <= maxAddr) {
                while(numa_maps.getline(buf, 1024)) {
                    const char delim[2] = " ";
                    char *tok = strtok(buf, delim);
                    if ((unsigned long)strtol(tok, NULL, 16) == minAddr) {
                        int totalCount = 0;
                        std::vector<TULongIntPair> v;
                        while(tok != NULL) {
                            int node; long count;
                            if (sscanf(tok, "N%d=%ld", &node, &count) == 2) {
                                v.push_back(TULongIntPair(count, node));
                                totalCount += count;
                            }
                            tok = strtok(NULL, delim);
                        }
                        int r = totalCount == 0 ? 0 : rand() % totalCount;
                        for (unsigned int i = 0; i < v.size(); i++) {
                            r -= v[i].first;
                            if (r < 0) { currentNode = v[i].second; break; }
                        };
                        addressToNumaMap.insert(TULongIntPair(address, currentNode));
                        maps.close();
                        numa_maps.close();
                        return currentNode;
                    }
                }
            }
        }

        // Occasionally, there's a concurrency issue with the proc file in which we do not
        // find the address.  In limited experiments, this occurred rarely, (< .1% of operations)
        // In this case we assign it to node 0.  We must add it to the lookup map since the
        // same address reached later must have the same node.
        addressToNumaMap.insert(TULongIntPair(address, currentNode));
        maps.close();
        numa_maps.close();
        return currentNode;
    }
}

// Print a memory read record
VOID RecordMemRead(VOID *ip, VOID *addr) {
  PIN_GetLock(&lock, 0);
  fprintf(trace, "%d R %p %d\n", PIN_ThreadId(), addr, getNumaNode(addr));
  PIN_ReleaseLock(&lock);
}

// Print a memory write record
VOID RecordMemWrite(VOID *ip, VOID *addr) {
  PIN_GetLock(&lock, 0);
  fprintf(trace, "%d W %p %d\n", PIN_ThreadId(), addr, getNumaNode(addr));
  PIN_ReleaseLock(&lock);
}

// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID *v) {
  // Instruments memory accesses using a predicated call, i.e.
  // the instrumentation is called iff the instruction will actually be executed.
  //
  // On the IA-32 and Intel(R) 64 architectures conditional moves and REP
  // prefixed instructions appear as predicated instructions in Pin.
  UINT32 memOperands = INS_MemoryOperandCount(ins);

  // Iterate over each memory operand of the instruction.
  for (UINT32 memOp = 0; memOp < memOperands; memOp++) {
    if (INS_MemoryOperandIsRead(ins, memOp)) {
      INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead, IARG_INST_PTR,
                               IARG_MEMORYOP_EA, memOp, IARG_END);
    }
    // Note that in some architectures a single memory operand can be
    // both read and written (for instance incl (%eax) on IA-32)
    // In that case we instrument it once for read and once for write.
    if (INS_MemoryOperandIsWritten(ins, memOp)) {
      INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite, IARG_INST_PTR,
                               IARG_MEMORYOP_EA, memOp, IARG_END);
    }
  }
}

VOID Fini(INT32 code, VOID *v) {
  fclose(trace);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage() {
  PIN_ERROR("This Pintool prints a trace of memory addresses\n" + KNOB_BASE::StringKnobSummary() +
            "\n");
  return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[]) {
  if (PIN_Init(argc, argv)) return Usage();

  pagesize = getpagesize();
  srand(time(NULL));

  trace = fopen("pinatrace.out", "w");

  INS_AddInstrumentFunction(Instruction, 0);
  PIN_AddFiniFunction(Fini, 0);

  // Never returns
  PIN_StartProgram();

  return 0;
}
