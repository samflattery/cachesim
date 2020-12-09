#!/bin/bash

# assumes pin and cachesim in home directory
# run with -t to generate all traces
# run with -s to run all sims
# run with -p as well as one/both of these to prompt before doing anything

# make everything
cd ~/cachesim
make -j8
make -j8 programs

cd ~/pin/source/tools/ManualExamples
make

# make the results directory if it doesn't exist
[ ! -d ~/cachesim/results ] && mkdir ~/cachesim/results

progs=(ts_lock tts_lock ticketlock arraylock arraylock_aligned)
protocols=(MSI MESI MOESI)
prompt=false

threads=8

generate_traces () {
    cd ~/pin/source/tools/ManualExamples
    for prog in ${progs[@]}; do
        if [ $prompt = true ]
        then
            read -p "Generate trace for ${prog}?" -n 1 -r
            echo
        fi

        # this is hacky but oh well i don't know how to use bash if statents properly and
        # apparently if [[ $prompt = false || $REPLY ... ]] doesn't work
        if [ $prompt = false ]
        then REPLY="y"
        fi

        if [[ $REPLY =~ ^[Yy]$ ]]
        then
            echo "Generating trace for ${prog} with ${threads} threads"
            ../../../pin -t obj-intel64/pinatrace.so -o ~/cachesim/traces/${prog}${threads}.trace -- ~/cachesim/programs/${prog}.out ${threads}
        fi
    done
}

run_sim () {
    for prog in ${progs[@]}; do
        if [ $prompt = true ]
        then
            read -p "Run sims for ${prog}?" -n 1 -r
            echo
        fi

        if [ $prompt = false ]
        then REPLY="y"
        fi

        if [[ $REPLY =~ ^[Yy]$ ]]
        then
            for protocol in ${protocols[@]}; do
                echo "Running sim on ${prog} with protocol ${protocol} and ${threads} threads"
                ~/cachesim/cachesim -t ~/cachesim/traces/${prog}${threads}.trace -p ${threads} -n 2 -m ${protocol} -A -i > ~/cachesim/results/${prog}_${threads}_${protocol}.txt
            done
        fi
    done
}

sim=false
trace=false

while getopts ":stp" flag; do
    case "${flag}" in
        t) trace='true';;
        s) sim='true';;
        p) prompt='true';;
    esac
done

# apparently the = true is necessary here
if [ $trace = true ]
then generate_traces
fi

if [ $sim = true ]
then run_sim
fi
