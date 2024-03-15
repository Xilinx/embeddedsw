# Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
"""
This module updates the application meta-data app.yaml and CMakeLists.txt
for the new platform using the domain information provided.
"""

import utils
import argparse
import os
from build_bsp import BSP

class RetargetApp(BSP):
    def __init__(self, args):
        self.domain_path = utils.get_abs_path(args.get("domain_path"))
        BSP.__init__(self, args)
        self.app_src_dir = utils.get_abs_path(args["src_dir"])

def retarget_app(args):
    """
    Args:
        args (dict): Takes all the user inputs in a dictionary.
    """
    obj = RetargetApp(args)

    # Update the domain path as per new domain
    app_config_file = os.path.join(obj.app_src_dir, "app.yaml")
    template = utils.fetch_yaml_data(app_config_file, "template")["template"]
    utils.update_yaml(app_config_file, "domain_path", "domain_path", obj.domain_path, action="add")

    # Update link libraries as per new domain
    domain_data = utils.fetch_yaml_data(obj.domain_config_file, "domain")
    if domain_data['lib_info']:
        src_cmake = os.path.join(obj.app_src_dir, "CMakeLists.txt")
        lib_list = list(domain_data['lib_info'].keys())
        # Special handling for libmetal
        lib_list = [lib.replace('libmetal', 'metal') for lib in lib_list]
        cmake_lib_list = ';'.join(lib_list)
        utils.replace_line(
            src_cmake,
            f'PROJECT_LIB_DEPS xilstandalone',
            f'collect(PROJECT_LIB_DEPS xilstandalone;{cmake_lib_list})\n',
        )

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Use this script to change platform for a given application",
        usage='use "python %(prog)s --help" for more information',
        formatter_class=argparse.RawTextHelpFormatter,
    )
    required_argument = parser.add_argument_group("Required arguments")
    required_argument.add_argument(
        "-d",
        "--domain_path",
        action="store",
        help="Specify the built BSP Path",
        required=True,
    )
    required_argument.add_argument(
        "-s",
        "--src_dir",
        action="store",
        help="App source directory",
        required=True,
    )
    args = vars(parser.parse_args())
    retarget_app(args)
