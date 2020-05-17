#!/bin/bash
#SBATCH --job-name=random1
#SBATCH --cpus-per-task=8
#SBATCH --output=resources/results/job_result/task_random1_%j_solver_0_nnet_1.out
#SBATCH --partition=long
#SBATCH --time=34:00:00
#SBATCH --signal=B:SIGUSR1@300
#SBATCH --mem-per-cpu=8G


function extract_result {
	python3 resources/utils/result_extractor.py
}

pgid=$(($(ps -o pgid= -p $$)))
trap 'kill -TERM $pid & wait $pid ; extract_result ; exit' SIGUSR1


function extract_result {
	python3 resources/utils/result_extractor.py
}

pgid=$(($(ps -o pgid= -p $$)))
trap 'kill -TERM -$pgid & extract_result' SIGTERM

pwd; hostname; date

cwd=$(pwd)

resources/solvers/random/run.sh resources/nnet/net2_7_26 resources/property.txt resources/results/solver_result/solver_0_nnet_1.sum '' & pid=$!
wait $pid

cd $cwd

extract_result

date
