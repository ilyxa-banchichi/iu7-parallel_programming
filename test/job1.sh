#!/bin/bash

#SBATCH --job-name="09.25"
#SBATCH --partition=debug
#SBATCH --nodes=10
#SBATCH --ntasks-per-node=1
#SBATCH --time=7-00:01:00
#SBATCH --mem=1992

echo "HOSTNAME = $HOSTNAME"
echo "SLURM_JOB_NODELIST = $SLURM_JOB_NODELIST"

mpirun -np 10 ./your_mpi_program