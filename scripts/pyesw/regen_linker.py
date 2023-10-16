# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
"""
This module regenerates the linker script for a given template application
using the domain information provided to it.
"""

import utils
import argparse
import os
from build_bsp import BSP
from repo import Repo

class RegenLinker(BSP, Repo):
    """
    This class helps in regenerating linker script for the template application.
    It contains attributes and functions that take domain path, and app source
    directory as an input.
    """
    def __init__(self, args):
        BSP.__init__(self, args)
        Repo.__init__(self, repo_yaml_path=args['repo_info'])
        self.repo_paths_list = self.repo_schema['paths']
        self.app_src_dir = utils.get_abs_path(args["src_dir"])

def regen_linker(args):
    """
    Function that uses the above RegenLinker class to regenerate
    linker script for the template application.

    Args:
        args (dict): Takes all the user inputs in a dictionary.
    """
    obj = RegenLinker(args)

    app_config_file = os.path.join(obj.app_src_dir, "app.yaml")
    template = utils.fetch_yaml_data(app_config_file, "template")["template"]

    esw_app_dir = obj.get_comp_dir(template)
    srcdir = os.path.join(esw_app_dir, "src")
    app_linker_build = os.path.join(obj.app_src_dir, "linker_build")
    utils.mkdir(app_linker_build)

    # Copy the static linker files from embeddedsw to the app src dir
    linker_dir = os.path.join(obj.app_src_dir, "linker_files")
    linker_src = utils.get_high_precedence_path(
            obj.repo_paths_list, "Linker file directory", "scripts", "linker_files"
        )
    utils.copy_directory(linker_src, linker_dir)

    # Generates the metadata for linker script
    linker_cmd = (
        f"lopper -O {app_linker_build} {obj.sdt} -- baremetallinker_xlnx {obj.proc} {srcdir}"
    )
    if template == "memory_tests":
        utils.runcmd(f"{linker_cmd} memtest")
    else:
        utils.runcmd(linker_cmd)

    cmake_file = os.path.join(app_linker_build, "CMakeLists.txt")
    cmake_file_cmds = f"""
cmake_minimum_required(VERSION 3.15)
project(bsp)
find_package(common)
include({template.capitalize()}Example.cmake)
linker_gen({linker_dir})
    """
    cmake_file_cmds = cmake_file_cmds.replace('\\', '/')
    obj.cmake_paths_append = obj.cmake_paths_append.replace('\\', '/')
    utils.write_into_file(cmake_file, cmake_file_cmds)
    utils.runcmd(f'cmake -G "{obj.cmake_generator}" {app_linker_build} {obj.cmake_paths_append}', cwd=app_linker_build)
    utils.copy_file(os.path.join(app_linker_build, "lscript.ld"), obj.app_src_dir)
    utils.remove(app_linker_build)

    # Success prints if everything went well till this point.
    print(f"Successfully Regenerated linker script at {obj.app_src_dir}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Use this script to create a template App using the BSP path",
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
    parser.add_argument(
        "-s", "--src_dir", action="store", help="App source directory (Default: <Current Work Directory>/src)"
    )
    parser.add_argument(
        "-r",
        "--repo_info",
        action="store",
        help="Specify the .repo.yaml absolute path to use the set repo info",
        default='.repo.yaml',
    )
    args = vars(parser.parse_args())
    regen_linker(args)
