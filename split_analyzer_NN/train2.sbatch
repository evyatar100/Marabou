#!/bin/bash
#SBATCH --job-name=train2
#SBATCH --cpus-per-task=8
#SBATCH --output=train2_output.out
#SBATCH --partition=long
#SBATCH --time=12:00:00
#SBATCH --signal=B:SIGUSR1@300
#SBATCH --mem-per-cpu=8G


python manager.py train model2 random1_data.csv 50
 

