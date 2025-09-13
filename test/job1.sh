!/bin/bash

SVATCH --job-name="09.25"

SBATCH --partition=debug

SBATCH --nodes=10

SBATCH --time=7-00:01:00
SBATCH --ntasks-per-node=1

SBATCH --mem=1992

echo "HOSTNAME = $HOSTNAME"

mpirun -np 10 m