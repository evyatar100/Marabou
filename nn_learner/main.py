import sys
from tree_size_estimator import TreeSizeEstimator


if __name__ == '__main__':
    csv_path = sys.argv[1]
    tree_estimator = TreeSizeEstimator()
    # tree_estimator.train_model(csv_path)

    tree_estimator.restore_model()

    data = pd.read_csv(csv_path)
    samples = data.drop(columns=['Unnamed: 0', ' sub-tree size']).values

    N_CONSTRAINS = 224
    network_state = samples[0, :N_CONSTRAINS * 2].reshape(1, N_CONSTRAINS * 2)
    tree_estimator.get_best_constraint(network_state)
    # train_model(csv_path)