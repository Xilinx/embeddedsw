# Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
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


"""
Rules that decide the priority order:
1. Versionless component path will always have the higher priority than the
    versioned path no matter what the entered path order is.
2. For multiple versionless paths, priority shifts from left to right of the
    entered paths, left one having the higher priority.
3. For versioned paths, higher versions will have higher priority no matter
    what the entered path order is.
4. For multiple versioned paths having the same versions, priority shifts
    from left to right of the entered paths, left one having the higher priority.
"""
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

        files = sorted(glob.glob(abs_path + '/**/data/*.yaml', recursive=True))
        for entries in files:
            dir_path = utils.get_dir_path(utils.get_dir_path(entries))
            comp_name = utils.get_base_name(dir_path)
            comp_version_info = re.split("_v(\d+)_(\d+)", comp_name)
            comp_name = comp_version_info[0]
            comp_major_version = 9999
            comp_minor_version = 0
            if len(comp_version_info) > 1:
                comp_major_version = comp_version_info[1]
                comp_minor_version = comp_version_info[2]
            comp_version = float(f"{comp_major_version}.{int(comp_minor_version):04d}")

            yaml_data = utils.load_yaml(entries)
            if yaml_data is None or not yaml_data.get('type'):
                continue

            if not path_dict[yaml_data['type']].get(comp_name):
                path_dict[yaml_data['type']][comp_name] = [(comp_version, dir_path)]
            else:
                path_dict[yaml_data['type']][comp_name] += [(comp_version, dir_path)]

    for comp_type in ['os','driver','library','apps']:
        for comp_name, version_path_tuple in path_dict[comp_type].items():
            comp_path_list = []
            sorted_version_path_tuple = sorted(version_path_tuple, key=lambda x: x[0], reverse=True)
            for version_path_comb in sorted_version_path_tuple:
                comp_path_list += [version_path_comb[1]]
            path_dict[comp_type][comp_name] = {'path': comp_path_list}

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