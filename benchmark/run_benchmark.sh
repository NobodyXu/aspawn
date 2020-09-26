#!/bin/bash

for each in $1; do
    echo -e '\nRunning' $each ...
    sudo cpupower frequency-set --governor performance
    LD_LIBRARY_PATH=../ ./$each
    sudo cpupower frequency-set --governor powersave
    echo
done
