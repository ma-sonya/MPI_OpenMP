#!/bin/sh

g++ -std=c++11 -O3 -o openmp_solve -fopenmp open-mp.cpp && ./openmp_solve
