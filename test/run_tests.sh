#!/bin/bash

for each in $1; do
    LD_LIBRARY_PATH=../ ./$each
done
