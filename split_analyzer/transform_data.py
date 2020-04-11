import pandas as pd
import sys
from tqdm import tqdm

IS_CHOSEN_SUFFIX = '_is_chosen'
if __name__ == '__main__':
    #
    # data_file_src = 'data.csv'
    # data_file_dst = 'new_data.csv'
    data_file_src = sys.argv[1]
    data_file_dst = sys.argv[2]

    df = pd.read_csv(data_file_src)
    new_df = df.drop(columns=['current constraint', 'Unnamed: 0'])
    constraints_all_suffix = df.keys()[3:]
    constraints = sorted(list(set(constraint.split('_')[0] for constraint in constraints_all_suffix)))
    constraints_suffix = [constraint + IS_CHOSEN_SUFFIX for constraint in constraints]
    for constraint in tqdm(constraints):
        constraints_suffix = constraint + IS_CHOSEN_SUFFIX
        new_df[constraints_suffix] = 0
        new_df.loc[' ' + df['current constraint'] + IS_CHOSEN_SUFFIX == constraints_suffix, constraints_suffix] = 1

    print('saving to to_csv...')
    new_df.to_csv(data_file_dst)



