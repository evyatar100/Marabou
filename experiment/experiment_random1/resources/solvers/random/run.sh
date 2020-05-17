#!/bin/bash

# define variables
input_net=$1
input_property=$2
summary=$3
flags=$4

function post {
      	python3 $scriptdir/scripts/post.py $results_file $summary
	exit
}
trap 'kill -TERM $pid & wait $pid ; sleep 30  ; post' SIGTERM

# the working directory is where 'resources' directory is
# useful paths:
scriptdir="$(dirname "$0")"
tmpbasedir="resources/tmp"
resultdir="resources/results/solver_result"

# $1 = nnet file
# $2 = property file
# $3 = summary file
# $4 = additional flags

tmpdir=$(mktemp -d -p $tmpbasedir)
property_file=$tmpdir/property
nnet_file=$tmpdir/nnet
results_file="$tmpdir/results_$(basename "$summary")"

# give permissions to scripts
chmod 777 $scriptdir/scripts/pre
chmod 777 $scriptdir/marabou.elf

# convert
$scriptdir/scripts/pre $input_net $input_property $nnet_file $property_file
# run solver
$scriptdir/marabou.elf $nnet_file $property_file --summary-file=$results_file $flags & pid=$!
# convert results
wait $pid
post
