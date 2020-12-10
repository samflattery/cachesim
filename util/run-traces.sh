#!/bin/bash

# assumes pin and cachesim in home directory
# run with -t to generate all traces
# run with -s to run all sims
# run with -p as well as one/both of these to prompt before doing anything

# make everything
cd ~/cachesim
make -j8
make -j8 programs

# cp ~/cachesim/util/pinatrace.cpp ~/pin/source/tools/ManualExamples
cd ~/pin/source/tools/ManualExamples
make

# make the results directory if it doesn't exist
[ ! -d ~/cachesim/results ] && mkdir ~/cachesim/results

progs=(ts_lock tts_lock ticketlock arraylock arraylock_aligned)
protocols=(MSI MESI MOESI)

# cmd line flags
prompt=false
sim=false
trace=false

# num threads to run the programs with
threads=8

# run msi, mesi and moesi sims on given prog name
run_one_sim () {
    prog=$1
    for protocol in ${protocols[@]}; do
        echo "Running sim on ${prog} with protocol ${protocol} and ${threads} threads"
        ~/cachesim/cachesim -t ~/cachesim/traces/${prog}${threads}.trace -p ${threads} -n 2 -m ${protocol} -A -i > ~/cachesim/results/10incrsleep/${prog}_${threads}_${protocol}.txt
    done
}

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
            outfile=~/cachesim/traces/${prog}${threads}.trace
            ../../../pin -t obj-intel64/pinatrace.so -o $outfile -- ~/cachesim/programs/${prog}.out ${threads}

            # check that the file ends in the eof str
            eof_str=$(tail -n 1 $outfile)
            if [[ $eof_str != "#eof" ]]
            then echo "Failed to generate trace - out of memory"
                 exit 1
            fi

            # run the sim on it
            if [ $sim = true ]
            then
                run_one_sim $prog
                echo "Removing ${outfile}"
                rm $outfile
            fi
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
            run_one_sim $prog
        fi
    done
}

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

# sims will be run after traces are collected if trace and sim are true
if [ $sim = true -a $trace = false ]
then run_sim
fi
