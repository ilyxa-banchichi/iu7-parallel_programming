#!/bin/bash
clang -DUSE_MULTITHREAD -fopenmp main.c -o build/main_parall
clang -fopenmp main.c -o build/main_sync