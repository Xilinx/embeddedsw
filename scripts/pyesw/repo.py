# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
"""
This module acts as a supporting module for all the other modules. It helps
in validating the embeddedsw repo set in the environment and sets up the
correct path for different components within embeddedsw. It doesnt have
any main() function and running this module independently is not intended.
"""

import os, sys
import glob
import re
import utils
import argparse


class Repo:
    """
    This is the base class to get the embeddedsw repo path. This checks if the
    embeddedsw path is set correctly. If set, this helps in retrieving the
    corresponding directory path of the component inside embeddedsw.
    """

    def __init__(self, repo_yaml_path=".repo.yaml"):
        repo_yaml_path = utils.get_abs_path(repo_yaml_path)
        self._validate_repo(repo_yaml_path, os.environ.get("ESW_REPO", ""))
        self.repo_yaml_path = repo_yaml_path
        self.repo_schema = utils.load_yaml(self.repo_yaml_path)

    def _validate_repo(self, repo_yaml_path, shell_esw_repo):
        """
        Returns the set absolute path of embeddedsw repo.

        Args:
            repo (str): The user input for the embeddedsw repo path
        Returns:
            repo (str):
                If user entry is correct, returns the absolute path
                of that entry
        """
        if not utils.is_file(repo_yaml_path):
            if shell_esw_repo:
                resolve_paths([shell_esw_repo])
            else:
                print(f"""\b
                    [ERROR]: Please set the Embeddedsw directory path.
                    Usage:
                        python3 repo.py -st <the ESW_REPO_PATH>
                    For multiple esw repo paths, use below with left one having higher precedence:
                        python3 repo.py -st <path_1> <path_2> ... <path_n>
                    """
                )
                sys.exit(1)

    def get_comp_dir(self, comp_name, sdt_path=""):
        path_found = False
        for entries in self.repo_schema.keys():
            if comp_name in self.repo_schema[entries].keys():
                path_found = True
                comp_dir = self.repo_schema[entries][comp_name]["path"][0]
                return self.validate_comp_path(comp_dir, comp_name)
        if not path_found:
            if sdt_path:
                has_drivers = os.path.join(sdt_path, "drivers")
                if utils.is_dir(has_drivers, silent_discard=False):
                    yaml_list = glob.glob(has_drivers + '/**/data/*.yaml', recursive=True)
                    yaml_file_abs = [yaml for yaml in yaml_list if f"{comp_name}.yaml" in yaml]
                    if yaml_file_abs:
                        comp_dir = utils.get_dir_path(utils.get_dir_path(yaml_file_abs[0]))
                        return self.validate_comp_path(comp_dir, comp_name)
            print(f"[ERROR]: Couldnt find the src directory for {comp_name}")
            sys.exit(1)

    def validate_comp_path(self, comp_dir, comp_name):
        assert utils.is_dir(
            comp_dir
        ), f"{comp_dir} doesnt exist. Not able to fetch the dir for {comp_name}"
        return comp_dir


def resolve_paths(repo_paths):
    path_dict = {
        'paths' : {},
        'os'    : {},
        'driver'  : {},
        'library' : {},
        'apps'    : {}
        }
    comp_list = []
    for path in repo_paths:
        abs_path = utils.get_abs_path(path)
        if not utils.is_dir(path):
            print(f"[ERROR]: Directory path {abs_path} doesn't exist")
            sys.exit(1)
        elif abs_path not in path_dict['paths'].keys():
            path_dict['paths'].update({abs_path : {}})
        else:
            continue
        files = glob.glob(abs_path + '/**/data/*.yaml', recursive=True)
        files.sort(reverse=True)
        for entries in files:
            dir_path = utils.get_dir_path(utils.get_dir_path(entries))
            comp_name = utils.get_base_name(dir_path)
            comp_name =  re.sub("_v(\d+)_(\d+)", "", comp_name)
            yaml_data = utils.load_yaml(entries)
            if yaml_data is None:
                continue

            if not path_dict[yaml_data['type']].get(comp_name):
                path_dict[yaml_data['type']][comp_name] = {"path" : [dir_path]}
            else:
                path_dict[yaml_data['type']][comp_name]["path"].append(dir_path)

    utils.write_yaml('.repo.yaml', path_dict)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Use this script to set ESW Repo Path",
        usage='use "python %(prog)s --help" for more information',
        formatter_class=argparse.RawTextHelpFormatter,
    )
    required_argument = parser.add_argument_group("Required arguments")
    required_argument.add_argument(
        "-st",
        "--set_repo_path",
        nargs='+',
        help="Embeddedsw directory Path",
        required=True,
    )
    args = vars(parser.parse_args())
    resolve_paths(args['set_repo_path'])