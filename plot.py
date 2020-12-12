import matplotlib.pyplot as plt
from matplotlib import style
import os
from parse import parse
from pprint import pprint


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

        self.cache_time = self.parse_time('Cache Access Latency:\t{}', lines[29])
        self.mem_read_time = self.parse_time('Memory Read Latency:\t{}', lines[30])
        self.mem_write_time = self.parse_time('Memory Write Latency:\t{}', lines[31])
        self.mem_access_time = self.parse_time('Memory Access Latency:\t{}', lines[32])
        self.inter_loc_time = self.parse_time('Local Interconnect Latency:\t{}', lines[33])
        self.inter_global_time = self.parse_time('Global Interconnect Latency:\t{}', lines[34])

    def parse_time(self, form, line):
        try_ms = parse(form + 'ms\n', line)
        try_ns = parse(form + 'ns\n', line)
        if try_ms != None:
            return int(try_ms[0]) * 1000
        elif try_ns != None:
            return int(try_ns[0]) / 1000
        else:
            return int(parse(form + 'us\n', line)[0])

    def get(self, field):
        return self.__dict__[field]

def unzip(l):
    return [i for i, j in l], [j for i,j in l]

all_traces = []
def load_results(directory):
    lock_dirs = os.listdir(directory)
    for lock_dir in lock_dirs:
        res_files = os.listdir(f"./{directory}/{lock_dir}")
        for f in res_files:
            curr_trace = TraceData(lock_dir, f)
            all_traces.append(curr_trace)


def compare_protocols_and_nprocs(metric):
    for lock in lock_types:
        msi_x = []
        msi_y = []
        mesi_x = []
        mesi_y = []
        moesi_x = []
        moesi_y = []
        for trace in all_traces:
            if trace.lock_type == lock:
                if trace.protocol == 'MSI':
                    msi_y.append(trace.get(metric))
                    msi_x.append(trace.nprocs)
                elif trace.protocol == 'MESI':
                    mesi_y.append(trace.get(metric))
                    mesi_x.append(trace.nprocs)
                else: # trace.protocol == 'MOESI'
                    moesi_y.append(trace.get(metric))
                    moesi_x.append(trace.nprocs)
        msi_x, msi_y = unzip(sorted(list(zip(msi_x, msi_y)), key=lambda v:v[0]))
        mesi_x, mesi_y = unzip(sorted(list(zip(mesi_x, mesi_y)), key=lambda v:v[0]))
        moesi_x, moesi_y = unzip(sorted(list(zip(moesi_x, moesi_y)), key=lambda v:v[0]))
        plt.plot(msi_x, msi_y, label='msi', color='blue', marker='o')
        plt.plot(mesi_x, mesi_y, label='mesi', color='red', marker='o')
        plt.plot(moesi_x, moesi_y, label='moesi', color='green', marker='o')
        plt.legend()
        plt.title(f'{metric} vs Number of Procs, {lock}')
        plt.xlabel('Number of Procs')
        if metric.endswith('time'):
            plt.ylabel(f'{metric} (us)')
        else:
            plt.ylabel(metric)
        plt.savefig(f'plots/{metric} vs Number of Procs, {lock}')
        plt.clf()


def compare_locks(metric, normalize=True):
    for protocol in protocols:
        for n in nprocs:
            x = []
            y = []
            for trace in all_traces:
                if trace.nprocs == n and trace.protocol == protocol:
                    x.append(trace.lock_type)
                    if normalize:
                        y.append(trace.get(metric)/trace.total_ops)
                    else:
                        y.append(trace.get(metric))
            plt.bar(x, y, align='center')
            plt.title(f'{metric} per lock type, {protocol}, p={n}')
            if normalize:
                plt.ylabel(f'{metric} Proportion')
            else:
                plt.ylabel(f'{metric}')
            plt.xlabel('Lock type')
            plt.savefig(f'plots/{metric} per lock type, {protocol}, p={n}')
            plt.clf()

def compare_protocols(metric, normalize=True):
    for lock in lock_types:
        for n in nprocs:
            x = []
            y = []
            for trace in all_traces:
                if trace.nprocs == n and trace.lock_type == lock:
                    x.append(trace.protocol)
                    if normalize:
                        y.append(trace.get(metric)/trace.total_ops)
                    else:
                        y.append(trace.get(metric))
            plt.bar(x, y, align='center')
            plt.title(f'{metric} per Protocol, {lock}, p={n}')
            if normalize:
                plt.ylabel(f'{metric} Proportion')
            else:
                plt.ylabel(f'{metric}')
            plt.xlabel('Protocol')
            plt.savefig(f'plots/{metric} per Protocol, {lock}, p={n}')
            plt.clf()


protocols = ['MSI', 'MESI', 'MOESI']
lock_types = ['arraylock', 'arraylockaligned', 'ticketlock', 'ts', 'tts']
nprocs = [2, 4, 8, 16, 32]
# Uncomment if using bst_results
# nprocs = [8]

# First parse the result files, use 'results' or 'bst_results'
load_results('results')

# To plot times as a function of nprocs, use compare_protocols_and_nprocs
# compare_protocols_and_nprocs('cache_time')

# To compare locks or protocol, use compare_locks or compare_protocol
# compare_protocols('mem_access_time', normalize=False)

