#!/bin/python3
# python3 process_all_files.py --input_fields input_fields.json
# sample json file
# {
#     "store_directory": "/mnt/black-library/store/"
# }
# 
# Trying to use
# https://google.github.io/styleguide/pyguide.html

import argparse
import os

parser = argparse.ArgumentParser(description="Script that checks export file for duplicate md5s")
parser.add_argument("-f", "--file", required=True, help="json file with inputs")
args = parser.parse_args()


def main():
    #read input file

    input_file = args.file

    md5_dict = {}
    repeated_list = set()

    with open(input_file) as fp:
        for line in fp:
            line_split = line.split(',')
            uuid = line_split[0]
            md5 = line_split[2]
            if uuid in md5_dict:
                if md5 in md5_dict.get(uuid):
                    repeated_list.add(md5)
                md5_dict.get(uuid).append(md5)
            else:
                md5_dict[uuid] = [md5]

    for md5 in md5_dict:
        print(f"md5: {md5} len: {len(md5_dict.get(md5))}")

    print(repeated_list)

    return

if __name__== "__main__":
    main()