import matplotlib.pyplot as plt
from matplotlib import style
import os
from parse import parse
from pprint import pprint

all_traces = []

class TraceData:
    def do_stuff(self):
        print(self.x)
    def __init__(self, direc, f):
        parse_res = parse('{}_{}_{}.txt', f)
        self.lock_type = parse_res[0]
        self.nprocs = int(parse_res[1])
        self.protocol = parse_res[2]

        file = open(f"./results/{direc}/{f}", mode='r')
        lines = file.readlines()
        file.close()
        self.total_ops = int(parse('Total Reads/Writes:\t\t{}\n', lines[7])[0])

        self.total_hits = int(parse('Total Hits:\t\t{}\n', lines[11])[0])
        self.total_misses = int(parse('Total Misses:\t\t{}\n', lines[12])[0])
        self.total_flushes = int(parse('Total Flushes:\t\t{}\n', lines[13])[0])
        self.total_evict = int(parse('Total Evictions: \t{}\n', lines[14])[0])
        self.total_devict = int(parse('Total Dirty Evictions: \t{}\n', lines[15])[0])
        self.total_invalid = int(parse('Total Invalidations: \t{}\n', lines[16])[0])

        self.total_local = int(parse('Total Local Interconnect Events: \t{}\n', lines[20])[0])
        self.total_global = int(parse('Total Global Interconnect Events: \t{}\n', lines[21])[0])

        self.total_mem = int(parse('Total Memory Reads: \t{}\n', lines[25])[0])


lock_dirs = os.listdir('./results/')
for lock_dir in lock_dirs:
    res_files = os.listdir(f"./results/{lock_dir}")
    for f in res_files:
        curr_trace = TraceData(lock_dir, f)
        all_traces.append(curr_trace)

protocols = ['MSI', 'MESI', 'MOESI']
nprocs = [2, 4, 8, 16, 32]
lock_types = ['arraylock', 'arraylockaligned', 'ticketlock', 'ts', 'tts']

# MEMORY
# COMPARE LOCK TYPES, HOLD PROTOCOL AND NPROCS
for protocol in protocols:
    for n in nprocs:
        x = []
        y = []
        for trace in all_traces:
            if trace.nprocs == n and trace.protocol == protocol:
                x.append(trace.lock_type)
                y.append(trace.total_mem/trace.total_ops)
        plt.bar(x, y, align='center')
        plt.title(f'Memory reads per lock type, {protocol}, p={n}')
        plt.ylabel('Memory Read Proportion')
        plt.xlabel('Lock type')
        plt.savefig(f'plots/Memory read per lock type, {protocol}, p={n}')
        plt.clf()

# COMPARE PROTOCOL, HOLD LOCK AND NPROCS
for lock in lock_types:
    for n in nprocs:
        x = []
        y = []
        for trace in all_traces:
            if trace.nprocs == n and trace.lock_type == lock:
                x.append(trace.protocol)
                y.append(trace.total_mem/trace.total_ops)
        plt.bar(x, y, align='center')
        plt.title(f'Memory reads per protocol, {lock}, p={n}')
        plt.ylabel('Memory Read Proportion')
        plt.xlabel('Protocol')
        plt.savefig(f'plots/Memory read per protocol, {lock}, p={n}')
        plt.clf()

# INVALIDATIONS
# COMPARE LOCK TYPES, HOLD PROTOCOL AND NPROCS
for protocol in protocols:
    for n in nprocs:
        x = []
        y = []
        for trace in all_traces:
            if trace.nprocs == n and trace.protocol == protocol:
                x.append(trace.lock_type)
                y.append(trace.total_invalid/trace.total_ops)
        plt.bar(x, y, align='center')
        plt.title(f'Invalidation per lock type, {protocol}, p={n}')
        plt.ylabel('Invalidation Proportion')
        plt.xlabel('Lock type')
        plt.savefig(f'plots/Invalidation per lock type, {protocol}, p={n}')
        plt.clf()

# COMPARE PROTOCOL, HOLD LOCK AND NPROCS
for lock in lock_types:
    for n in nprocs:
        x = []
        y = []
        for trace in all_traces:
            if trace.nprocs == n and trace.lock_type == lock:
                x.append(trace.protocol)
                y.append(trace.total_invalid/trace.total_ops)
        plt.bar(x, y, align='center')
        plt.title(f'Invalidation per protocol, {lock}, p={n}')
        plt.ylabel('Invalidation Proportion')
        plt.xlabel('Protocol')
        plt.savefig(f'plots/Invalidation per protocol, {lock}, p={n}')
        plt.clf()
