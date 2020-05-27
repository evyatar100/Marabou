#!/bin/bash
#SBATCH --job-name=train4
#SBATCH --cpus-per-task=16
#SBATCH --output=train4_output.out
#SBATCH --partition=long
#SBATCH --time=24:00:00
#SBATCH --signal=B:SIGUSR1@300
#SBATCH --mem-per-cpu=4G


python main.py random1_data.csv 6 300 50

