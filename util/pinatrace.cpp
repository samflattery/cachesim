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

#include <cpuid.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "pin.H"

FILE *trace;
PIN_LOCK lock;


/* __cpuid_count gets information of the cpu and puts values using cpuid x86 instruction
 * ** should technically first check eflags that cpuid instruction exists
 * More info here: https://en.wikipedia.org/wiki/CPUID
 *
 * Each cpu has a core and APIC (advanced programmable interrupt controller)
 * CPUInfo[1] (EBX) bits 31-24 is the APIC ID, which we extract and return.
 * Since there is a one-to-one correspondance of APIC and cpu, we will use these numbers
 * More info here: https://wiki.osdev.org/APIC
 */
int getCPU() {
  int CPU;
  static uint32_t CPUInfo[4];
  __cpuid_count(1, 0, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);
  if ((CPUInfo[3] & (1 << 9)) == 0) {
    CPU = -1; /* no APIC on chip */
  } else {
    /* CPUInfo[1] is EBX, bits 24-31 are APIC ID */
    CPU = (unsigned)CPUInfo[1] >> 24;
  }
  if (CPU < 0) CPU = 0;
  return CPU;
}

// Print a memory read record
VOID RecordMemRead(VOID *ip, VOID *addr) {
  PIN_GetLock(&lock, 0);
  int cpu_id = getCPU();
  // TODO map memory lines to numa nodes
  int numa_node = 0;
  fprintf(trace, "%d W %p %d\n", cpu_id, addr, numa_node);
  PIN_ReleaseLock(&lock);
}

// Print a memory write record
VOID RecordMemWrite(VOID *ip, VOID *addr) {
  PIN_GetLock(&lock, 0);
  int cpu_id = getCPU();
  // TODO map memory lines to numa nodes
  int numa_node = 0;
  fprintf(trace, "%d W %p %d\n", cpu_id, addr, numa_node);
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

  trace = fopen("pinatrace.out", "w");

  INS_AddInstrumentFunction(Instruction, 0);
  PIN_AddFiniFunction(Fini, 0);

  // Never returns
  PIN_StartProgram();

  return 0;
}
