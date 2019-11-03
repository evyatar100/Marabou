import os
import sys
import pandas as pd
import glob

default_name = "combined.csv"

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("enter path to dir")
        exit(1)

    dfs = []
    os.chdir(sys.argv[1])
    for file in os.listdir('.'):
        if file.endswith(".csv"):
            dfs.append(pd.read_csv(file, index_col=None, header=0))

    big_frame = pd.concat(dfs, ignore_index=True)

    if len(sys.argv) <= 2:
        name = default_name
    else:
        name = sys.argv[2]

    big_frame.to_csv(name)