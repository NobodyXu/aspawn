#!/bin/bash -ex

echo Env variable PATH = ${PATH}

for each in $1; do
    echo -e '\nRunning' $each ...
    #LD_LIBRARY_PATH=../ ./$each
    ./$each
    echo
done
