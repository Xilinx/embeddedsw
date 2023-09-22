# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
import utils
import argparse
import os
from build_bsp import BSP

class ReconfigBSP(BSP):
    """
    This class contains attributes and functions to Reconfig the bsp.
    It takes the domain path as input validates it for the build
    configuration if it's changed will create new build folder and
    applies the cmake configuration.
    """

    def __init__(self, args):
        self.domain_path = utils.get_abs_path(args.get("domain_path"))
        BSP.__init__(self, args)

def reconfig_bsp(args):
    """
    Function to reconfig the bsp for the user input domain path.
    """
    obj = ReconfigBSP(args)

    build_metadata = os.path.join(obj.libsrc_folder, "build_configs", "gen_bsp")
    if utils.fetch_yaml_data(obj.domain_config_file, "path").get('path', {}):
        bsp_domain_path = utils.fetch_yaml_data(obj.domain_config_file, "path")["path"]
    else:
        bsp_domain_path = obj.domain_path
        utils.remove(build_metadata)
    """
    Recreate the build folder and run cmake configuration for the below use cases
    1. Shared platform use case.
    2. Platorm build folder was deleted by user explicitly.
    """
    if bsp_domain_path != obj.domain_path or not utils.is_dir(build_metadata):
        domain_data = utils.fetch_yaml_data(obj.domain_config_file, "domain")

        # Apply the Old software configuraiton
        cmake_cmd_append = ""
        # cmake syntax is using 'ON/OFF' option, 'True/False' is lagacy entry.
        bool_match = {"true": "ON", "false": "OFF"}
        lib_list = list(domain_data["lib_config"].keys()) + [obj.proc, obj.os]
        bsp_lib_config = domain_data["lib_config"]
        bsp_lib_config.update(domain_data["os_config"])
        bsp_lib_config.update(domain_data["proc_config"])
        for lib in lib_list:
            for key, value in bsp_lib_config[lib].items():
                val = value['value']
                if val in bool_match:
                    val = bool_match[val]
                cmake_cmd_append += f' -D{key}="{val}"'

        utils.remove(build_metadata)
        utils.mkdir(build_metadata)
        utils.runcmd(f'cmake -G "Unix Makefiles" {obj.domain_path} -DNON_YOCTO=ON -DSUBDIR_LIST="ALL" {obj.cmake_paths_append} {cmake_cmd_append}', cwd=build_metadata)

        utils.update_yaml(obj.domain_config_file, "path", "path", obj.domain_path, action="add")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Reconfig the BSP",
        usage='use "python %(prog)s --help" for more information',
        formatter_class=argparse.RawTextHelpFormatter,
    )
    required_argument = parser.add_argument_group("Required arguments")
    required_argument.add_argument(
        "-d",
        "--domain_path",
        action="store",
        help="Domain directory Path",
        required=True,
    )
    args = vars(parser.parse_args())
    reconfig_bsp(args)
