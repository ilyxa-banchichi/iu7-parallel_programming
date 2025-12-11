#!/bin/bash
#SBATCH --job-name="Ilya Task program"
#SBATCH --partition=debug
#SBATCH --nodes=10
#SBATCH --time=0-00:02:00
#SBATCH --ntasks-per-node=1
#SBATCH --mem=1992

mpirun -np 10 build/main_mpi_sync