#!/bin/bash
mpicc -DUSE_MULTITHREAD -fopenmp mpi_main.c -o build/main_mpi_parall