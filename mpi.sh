#!/bin/sh

mpic++ -std=c++11 -O3 mpi.cpp -o mpi_solve && mpiexec -np 16 ./mpi_solve
