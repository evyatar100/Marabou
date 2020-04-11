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

    os.remove(name)

    for file in os.listdir('.'):
        if file.endswith(".csv"):
            print(file)
            dfs.append(pd.read_csv(file))

    big_frame = pd.concat(dfs)

    big_frame.to_csv(name, index=False)

    print("done..")
    print(f'new file was created: {name}')
