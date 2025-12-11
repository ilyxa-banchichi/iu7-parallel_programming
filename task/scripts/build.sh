#!/bin/bash
mpicc -DUSE_MULTITHREAD -fopenmp mpi_main.c -o build/main_mpi_parall
mpicc -fopenmp mpi_main.c -o build/main_mpi_sync

mpicc -DUSE_MULTITHREAD -fopenmp main.c -o build/main_parall
mpicc -fopenmp main.c -o build/main_sync