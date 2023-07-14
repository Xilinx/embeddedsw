# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
"""
This module fetches all the template app related data from esw yamls and puts
it into a yaml file.
"""

import utils
from repo import Repo
import os, argparse

def fetch_template_data(esw_schema, dest_dir):
    """
    Collects all the template related data into a yaml file.

    Args:
        esw_app_dir_path : sw_apps directory path of embeddedsw
    """
    overall_data = {}
    apps_schema = esw_schema.get('apps',{})
    for app in apps_schema.keys():
        app_data_file = os.path.join(apps_schema[app]['path'][0], 'data', f'{app}.yaml')
        if utils.is_file(app_data_file):
            overall_data[app] = utils.load_yaml(app_data_file)

    if not utils.is_dir(dest_dir):
        utils.mkdir(dest_dir)
    utils.write_yaml(os.path.join(dest_dir,'template_data.yaml'), overall_data)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Fetches all the template app related data from esw yamls\
        and puts it into a yaml file",
        usage='use "python %(prog)s --help" for more information',
    )
    parser.add_argument(
        "-d",
        "--dir",
        action="store",
        help="Specify the directory path where yaml will be created (Default: Current working directory)",
        default="./"
    )
    parser.add_argument(
        "-r",
        "--repo_info",
        action="store",
        help="Specify the .repo.yaml absolute path to use the set repo info",
        default='.repo.yaml',
    )

    args = vars(parser.parse_args())
    repo_obj = Repo(repo_yaml_path = args['repo_info'])
    fetch_template_data(repo_obj.repo_schema, args['dir'])
