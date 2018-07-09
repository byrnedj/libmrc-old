#!/bin/sh

make clean
make

./gen_mrc ycsb 100 100 

#diff ycsb.mrc ycsb.correct


