#!/bin/bash


make clean
make


for run in {1..10}; do ./test-lru; done | sort | uniq -c
#for run in {1..1000}; do ./run; done | sort | uniq -c

