# Copyright (C) 2023 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

import argparse
import os
import utils
from build_bsp import BSP

logger = utils.get_logger(__name__)

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
    From 2024.2 release on wards cmake default generator moved to ninja as a part
    of performance enhancment in case of old release workspace delete the build folder.
    """
    is_ninja_build = os.path.join(obj.libsrc_folder, "build_configs", "gen_bsp", "build.ninja")
    if utils.is_file(is_ninja_build):
        cmake_cache = os.path.join(build_metadata, "CMakeCache.txt")
        if utils.is_file(cmake_cache):
            current_compiler_ar_path = utils.find_line_in_file(cmake_cache, "CMAKE_ASM_COMPILER_AR:FILEPATH")
            if current_compiler_ar_path:
                # Extract the path and name
                current_compiler_ar_path = current_compiler_ar_path.split("=")[-1].strip()
                current_compiler_ar_path = os.path.normpath(current_compiler_ar_path)
                compiler_ar_name = os.path.basename(current_compiler_ar_path)
                # Find the updated path
                updated_compiler_ar_path = utils.find_compiler_path(compiler_ar_name)
                if current_compiler_ar_path != updated_compiler_ar_path:
                    logger.info(
                        f"Compiler path changed from {current_compiler_ar_path} to {updated_compiler_ar_path}. Clearing build directory."
                    )
                    utils.remove(build_metadata)
    else:
        logger.info("Detected old GNU Make based build, Removing Build Metadata")
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
        obj.cmake_paths_append = obj.cmake_paths_append.replace('\\','/')
        obj.domain_path = obj.domain_path.replace('\\','/')
        utils.runcmd(
            f'cmake -G "{obj.cmake_generator}" {obj.domain_path} -DSUBDIR_LIST="ALL" {obj.cmake_paths_append} {cmake_cmd_append}',
            cwd = build_metadata,
            log_message = "Configuring CMake with updated BSP configurations",
            error_message = "Failed to configure CMake with updated BSP configurations",
            verbose_level = 0
        )
        utils.update_yaml(obj.domain_config_file, "path", "path", obj.domain_path, action="add")

def main(arguments=None):
    parser = argparse.ArgumentParser(
        description="Reconfig the BSP",
        usage='use "empyro reconfig_bsp --help" for more information',
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
    parser.add_argument(
        "-v",
        "--verbose",
        action="count",
        default=0,
        help='Increase output verbosity'
    )
    args = vars(parser.parse_args(arguments))
    utils.setup_log(args["verbose"])
    reconfig_bsp(args)

if __name__ == "__main__":
    main()