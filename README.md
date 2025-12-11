# iu7-parallel_programming

## Commands

mpicc -fopenmp -o main file_name - компиляция
sbatch job.sh - запуск
cat slurm-.out - результат
squeue - очередь
scancel - отмена
sinfo - хз

scontrol show partition=debug - показывает количество доступных нод

clang -fopenmp main.c -o main
