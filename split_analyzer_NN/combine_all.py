import os
import sys
import pandas as pd
import glob

default_name = "combined.csv"

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(f'USE: .py csv_files_dir new_csv(optional, default = {default_name}')
        exit(1)

    dfs = []
    os.chdir(sys.argv[1])

    if len(sys.argv) <= 2:
        name = default_name
    else:
        name = sys.argv[2]

    if os.path.exists(name):
        os.remove(name)

    n_cols = None
    for file in os.listdir('.'):
        if file.endswith(".csv"):
            df = pd.read_csv(file)
            print(file, f'shape={df.shape}')

            if n_cols is None:
                n_cols = df.shape[1]
            else:
                if n_cols != df.shape[1]:
                    choice = input(f'the first df has {n_cols} columns, while this one has {df.shape[1]} what to do?'
                                   f'1. skip.'
                                   f'2. stop.')
                    if choice == '1':
                        continue
                    else:
                        print('exiting without creation of output file..')
                        exit(1)

            dfs.append(df)

    print('concatinating')
    big_frame = pd.concat(dfs)

    print(f'saving to {name}')
    big_frame.to_csv(name, index=False)

    print("done..")
    print(f'new file was created: {name}')
