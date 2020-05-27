#!/bin/bash
#SBATCH --job-name=train
#SBATCH --cpus-per-task=16
#SBATCH --output=merge_output.out
#SBATCH --partition=long
#SBATCH --time=24:00:00
#SBATCH --signal=B:SIGUSR1@300
#SBATCH --mem-per-cpu=8G


python combile_all.py ../splitSelector_statistics

