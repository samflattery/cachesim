# cachesim

15-418 Parallel Computer Architecture and Programming Final Project

## Deliverables

### Proposal
The proposal can be found [here](./docs/proposal.pdf)

### Checkpoint
The checkpoint report can be found [here](./docs/checkpoint.pdf)

### Final
The final report can be found [here](./docs/final.pdf)

## Usage

### Running the simulation on an existing trace

Quick start:
```
./cachesim -p 8 -n 2 -m MESI -Ai -t traces/tts_nosleep8.trace
```
which runs the sim with an Intel L1 cache configuration on a small sample trace.  The full traces for the programs we used to generate our data are too large to be stored on Github.

For more information, run `./cachesim -h`

### Generating your own traces
Download the `pin` program [here](https://software.intel.com/content/www/us/en/develop/articles/pin-a-binary-instrumentation-tool-downloads.html) and put unzip it to `~/pin`.

Run the following:
```
cd ~/pin/source/tools/ManualExamples
make
../../../pin -t obj-intel64/pinatrace.{so, dylib} -o <path/to/output/trace.trace> -- path/to/your/executable
```
(`pinatrace.so` on Linux, `pintrace.dylib` on macOS)

**Note:** the traces must be generated on a Linux machine (e.g. CMU's `andrew` machines), since they rely on the `/proc` filesystem.

### Generating the lock traces
Ensure `pin` and `cachesim` are in the home directory.

Run `./util/run-traces.sh -t [-p]`

The `-p` argument is optional if you want it to prompt you to let you just run certain traces.

This will run the pintool on all the programs in the `programs/` directory and output the trace files into the `traces/` directory.  The `num_numa` variable in `run-traces.sh` can be modified if the system does not have 2 (the number on `andrew`) NUMA nodes.

### Running the sim on the lock traces

Run `./util/run-traces.sh -s [-p]` after generating the traces, or `./util/run-traces.sh -s -t [-p]` otherwise.

### Generating plots of the data

Run `./util/plot.py` to generate all line charts and output them into `plots/`.  See `plot.py` for information about how to generate specific plots, or bar charts.

