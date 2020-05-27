#!/bin/bash
#SBATCH --job-name=train
#SBATCH --cpus-per-task=2
#SBATCH --output=train2_output.out
#SBATCH --partition=long
#SBATCH --time=24:00:00
#SBATCH --signal=B:SIGUSR1@300
#SBATCH --mem-per-cpu=4G


python main.py random1_data.csv

