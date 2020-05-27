#!/bin/bash
#SBATCH --job-name=train3
#SBATCH --cpus-per-task=2
#SBATCH --output=train3_output.out
#SBATCH --partition=long
#SBATCH --time=24:00:00
#SBATCH --signal=B:SIGUSR1@300
#SBATCH --mem-per-cpu=8G


python main.py random1_data.csv 10 100 50

