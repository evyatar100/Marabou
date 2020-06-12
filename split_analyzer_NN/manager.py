import os
import sys
from tree_size_estimator import TreeSizeEstimator
import json
import pandas as pd
from time import gmtime, strftime
import numpy as np

INIT = 'init'
TRAIN = 'train'
HELP = 'help'
SBATCH = 'sbatch'
OPTIONS = [INIT, TRAIN, HELP, SBATCH]


def folder2network_params(folder):
    return os.path.join(folder, 'network_params.json')


def folder2network_stats(folder):
    return os.path.join(folder, 'network_stats.json')


def folder2log(folder):
    return os.path.join(folder, 'log.txt')


def log(folder, txt):
    time_str = strftime("%Y-%m-%d_%H:%M:%S", gmtime())
    with open(folder2log(folder), 'a') as file:
        file.write(f'{time_str} {txt}\n')
    print(f'{time_str} logged into {folder}: {txt}')


def init_network(args):
    assert len(args) == 3
    folder = args[0]
    layers = int(args[1])
    layer_size = int(args[2])

    os.mkdir(folder)
    f = open(folder2log(folder), 'w')
    f.close()

    log(folder, f'init with inputs {args}')

    args = {'folder': folder,
            'layers': layers,
            'layer_size': layer_size}

    with open(folder2network_params(folder), 'w') as jfile:
        json.dump(args, jfile)

    tree_estimator = TreeSizeEstimator(layers, layer_size, folder)
    log(folder, 'finish initialisation.\n')

    create_sbatch(folder, 50)


def train(args):
    assert len(args) == 3

    folder = args[0]
    csv_path = args[1]
    epochs = int(args[2])
    log(folder, f'stating train with inputs {args}')

    with open(folder2network_params(folder)) as jfile:
        args = json.load(jfile)

    tree_estimator = TreeSizeEstimator(args['layers'], args['layer_size'], folder)
    tree_estimator.train_model(csv_path, epochs)

    # test
    data = pd.read_csv(csv_path)
    samples = data.drop(columns=['sub-tree_size']).values
    N_CONSTRAINS = 224
    network_state = samples[0, :N_CONSTRAINS * 9].reshape(1, -1)
    result = tree_estimator.get_best_constraint(network_state)
    print(result)

    log(folder,
        f'finish train. train_loss = {tree_estimator.get_train_loss()}, test_loss = {tree_estimator.get_test_loss()}.\n')

    args = {'train_loss': float(tree_estimator.get_train_loss()), 'test_loss': float(tree_estimator.get_test_loss())}
    with open(folder2network_stats(folder), 'w') as jfile:
        json.dump(args, jfile)


def create_sbatch(name_dir, epochs, data_path='random1_data.csv'):
    sbatch_file_path = os.path.join(name_dir, f'{name_dir}.sbatch')
    with open(sbatch_file_path, 'w') as file:
        file.write(f'#!/bin/bash\n')
        file.write(f'#SBATCH --job-name={name_dir}\n')
        file.write(f'#SBATCH --cpus-per-task=8\n')
        file.write(f'#SBATCH --output={name_dir}.out\n')
        file.write(f'#SBATCH --partition=long\n')
        file.write(f'#SBATCH --time=24:00:00\n')
        file.write(f'#SBATCH --signal=B:SIGUSR1@300\n')
        file.write(f'#SBATCH --mem-per-cpu=8G\n')
        file.write('\n')
        file.write(f'python manager.py train {name_dir} {data_path} {epochs}\n')


if __name__ == '__main__':
    assert len(sys.argv) > 1
    option = sys.argv[1]

    if option == INIT:
        init_network(sys.argv[2:])

    elif option == TRAIN:
        train(sys.argv[2:])

    elif option == SBATCH:
        name_dir = sys.argv[2]
        epochs = int(sys.argv[3])
        create_sbatch(name_dir, epochs)

    else:
        print('usage:')
        print('init name_dir layers layer_size')
        print('or')
        print('train name_dir csv_path epochs')
        print('or')
        print('sbatch name_dir epochs')
