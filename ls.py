# Implementation of the ls command
# Thales Mello

import argparse
import os
import datetime


def get_parser():
    parser = argparse.ArgumentParser()
    parser.add_argument("-m", "--modified", action="store_true",
                        help="show last modified date/time [default: off]")
    choices = tuple("name n modified m size s".split())
    parser.add_argument("-o", "--order", metavar="ORDER",
                        choices=choices,
                        default=choices[0],
                        help="order by {} [default: {}]".
                             format(choices, choices[0]))
    parser.add_argument("-r", "--recursive", action="store_true",
                        help="recurse into subdirectories [default: off]")
    parser.add_argument("-s", "--size", action="store_true",
                        help="show sizes [default: off]")
    parser.add_argument("path", nargs="*", default=["."],
                        help="The paths are optional; if not given . is used.")
    return parser


def ls(path, size, order, recursive, modified):
    if not recursive:
        file_dict_list = []
        for p in path:
            file_dict_list += get_file_dict_list(p)
        print_file_dict_list(file_dict_list, modified, size, order)
    else:
        cur_path = os.getcwd()
        for p in path:
            cur_path = os.getcwd()
            for parent, dir_list, file_list in os.walk(p):
                print(parent + ":")
                file_dict_list = get_file_dict_list(parent, file_list)
                print_file_dict_list(file_dict_list, modified, size, order,
                                     shortname=True)
                print()


def print_file_dict_list(file_dict_list, modified, size, order,
                         shortname=False):
    sort_key = get_sort_key(order)
    file_dict_list = sorted(file_dict_list, key=lambda x: x[sort_key])
    for file in file_dict_list:
        if modified:
            print(file["modified"].strftime("%x %X"), end=" ")
        if size:
            print("{:>6,}".format(file["size"]), end=" ")
        if not shortname:
            print(file["name"])
        else:
            print(file["shortname"])


def get_sort_key(order):
    if order in ["name", "n"]:
        sort_key = "name"
    elif order in ["modified", "m"]:
        sort_key = "modified"
    elif order in ["size", "s"]:
        sort_key = "size"
    return sort_key


def get_file_dict_list(p=".", file_list=None):
    if not file_list:
        file_list = os.listdir(p)
    file_dict_list = []
    for file in file_list:
        filename = (p != ".") and os.path.join(p, file) or file
        stat = os.stat(filename)
        file_dict_list.append({
            "name": filename,
            "shortname": file,
            "modified": datetime.datetime.fromtimestamp(stat.st_mtime),
            "size": stat.st_size
        })
    return file_dict_list


def main():
    parser = get_parser()
    args = parser.parse_args()
    ls(**vars(args))

if __name__ == '__main__':
    main()
