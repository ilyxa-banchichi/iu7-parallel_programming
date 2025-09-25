#!/bin/bash
#SBATCH --job-name="Ilya Lab2 MPI programm"
#
#SBATCH --partition=debug
#
#SBATCH --nodes=10
#
#SBATCH --time=0-00:05:00
#
###SBATCH --time=7-00:00:00
#
#SBATCH --ntasks-per-node=1
#
#SBATCH --mem=1992

### export I_MPI_PMI_LIBRARY=/usr/lib64/libpmi.so

mpirun -np 10 main