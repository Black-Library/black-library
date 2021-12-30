#!/bin/python3
# python3 process_all_files.py --input_fields input_fields.json
# sample json file
# {
#     "store_directory": "/mnt/black-library/store"
# }
# 
# Trying to use
# https://google.github.io/styleguide/pyguide.html

import argparse
import json
import os

parser = argparse.ArgumentParser(description="Script that touches every section file")
parser.add_argument("-i", "--input_fields", required=True, help="json file with inputs")
args = parser.parse_args()

def create_file_list(dir):
    file_list = []
    for f in os.listdir(dir):
        if os.path.isfile(os.path.join(dir, f)):
            file_list.append(os.path.join(dir, f))

    return file_list

def create_uuid_list(directory):
    uuid_list = []
    for dir in os.listdir(directory):
        if os.path.isdir(os.path.join(directory, dir)):
            uuid_list.append(os.path.join(directory, dir))

    return uuid_list

def read_input_json(file):
    fp = open(file)
    input_fields = json.load(fp)
    store_directory = input_fields["store_directory"]

    return store_directory

def main():
    #read input file
    (store_directory) = read_input_json(args.input_fields)

    uuid_list = create_uuid_list(store_directory)

    print("UUID list length: {0}".format(len(uuid_list)))

    for dir in uuid_list:
        print("Processing {0}".format(dir))
        file_list = create_file_list(dir)
        print("Found {} files".format(len(file_list)))

    return

if __name__== "__main__":
    main()