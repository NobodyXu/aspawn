#!/bin/bash

for each in $1; do
    echo -e '\nRunning' $each ...
    LD_LIBRARY_PATH=../ ./$each
    echo
done
