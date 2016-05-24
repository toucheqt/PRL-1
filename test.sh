#!/bin/bash
# Project: Minimum extraction sort using MPI
# Author: Ondøej Krpec, xkrpec01@stud.fit.vutbr.cz
# Date: 29.3.2016
# Description: Test script for implementation of minimum extraction sort for subject PRL lectured at FIT VUT in Brno. 

if [ $# != 2 ]; then
    echo "Chyba: Spatny pocet parametru."
    exit 1
fi

mpic++ --prefix /sr/local/share/OpenMPI -o mes mes.cpp
dd if=/dev/random bs=1 count=$1 of=numbers 2>>/dev/null
mpirun --prefix /usr/local/share/OpenMPI -np $2 mes numbers $2
rm mes numbers