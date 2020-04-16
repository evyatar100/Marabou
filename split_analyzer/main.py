import sys
from tree_size_estimator import TreeSizeEstimator
import pandas as pd
import numpy as np


if __name__ == '__main__':
    csv_path = sys.argv[1]
    tree_estimator = TreeSizeEstimator()
    tree_estimator.train_model(csv_path)

    tree_estimator.restore_model()
    #
    data = pd.read_csv(csv_path)
    samples = data.drop(columns=[' sub-tree size']).values

    N_CONSTRAINS = 224
    network_state = samples[0, :N_CONSTRAINS * 2].reshape(1, N_CONSTRAINS * 2)
    #
    # network_state1 = network_state[0]
    # network_state_str = ''
    # for i in network_state1[:-1]:
    #     network_state_str += str(i) + ','
    # network_state_str += str(network_state1[-1])
    # print(network_state_str)


    result = tree_estimator.get_best_constraint(network_state)
    print(result)

