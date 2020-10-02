#!/bin/bash

sudo cpupower frequency-set --governor performance
echo

for each in $1; do
    echo -e '\nRunning' $each ...
    LD_LIBRARY_PATH=../ ./$each
    echo
done

sudo cpupower frequency-set --governor powersave
