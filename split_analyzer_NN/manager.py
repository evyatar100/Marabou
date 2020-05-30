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
OPTIONS = ['init', 'train', 'help']


def folder2json(folder):
    return os.path.join(folder, 'network_params.json')


def folder2log(folder):
    return os.path.join(folder, 'log.txt')


def log(folder, txt):
    time_str = strftime("%Y-%m-%d_%H:%M:%S", gmtime())
    with open(folder2log(folder), 'a') as file:
        file.write(f'{time_str} {txt}')
    print(f'{time_str} logged into {folder}: {txt}')


def init_network(args):
    assert len(args) == 3
    folder = args[0]
    layers = int(args[1])
    layer_size = int(args[1])

    os.mkdir(folder)
    f = open(folder2log(folder), 'w')
    f.close()

    log(folder, f'init with inputs {args}')

    args = {'folder': folder,
            'layers': layers,
            'layer_size': layer_size}

    with open(folder2json(folder), 'w') as jfile:
        json.dump(args, jfile)
    tree_estimator = TreeSizeEstimator(layers, layer_size, folder)
    log(folder, 'finish initialisation.\n')


def train(args):
    assert len(args) == 3

    folder = args[0]
    csv_path = args[1]
    epochs = int(args[2])
    log(folder, f'stating train with inputs {args}')

    with open(folder2json(folder)) as jfile:
        args = json.load(jfile)
    tree_estimator = TreeSizeEstimator(args['layers'], args['layer_size'], folder)
    tree_estimator.train_model(csv_path, epochs)
    log(folder, 'finish training.\n')

    # test
    data = pd.read_csv(csv_path)
    samples = data.drop(columns=['sub-tree_size']).values
    N_CONSTRAINS = 224
    network_state = samples[0, :N_CONSTRAINS * 9].reshape(1, -1)
    result = tree_estimator.get_best_constraint(network_state)
    print(result)

    log(folder, 'finish test.\n')


if __name__ == '__main__':
    assert len(sys.argv) > 1
    option = sys.argv[1]

    if option == INIT:
        init_network(sys.argv[2:])

    elif option == TRAIN:
        train(sys.argv[2:])

    else:
        print('usage:')
        print('init name_dir layers layer_size')
        print('or')
        print('train name_dir csv_path epochs')




