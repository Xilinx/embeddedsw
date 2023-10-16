# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
"""
This module builds archive files (.a) for the created bsp. These archive files
include os, driver and library related archives.
"""

import utils
import argparse
import os


class BSP:
    """
    This class contains attributes and functions to build the created bsp.
    It takes the domain path as input, reads the domain configuration file
    present in the path to get the required inputs, calls make command
    inside all the cmake build area and builds the archive (.a) files for
    baremetal.
    """

    def __init__(self, args):
        self.domain_path = utils.get_abs_path(args.get("domain_path"))
        self.domain_config_file = os.path.join(self.domain_path, "bsp.yaml")
        utils.validate_if_not_exist(
            self.domain_config_file, "domain", utils.get_base_name(self.domain_path)
        )
        domain_data = utils.fetch_yaml_data(self.domain_config_file, "domain")

        self.sdt = os.path.join(self.domain_path, domain_data["sdt"])
        self.proc = domain_data["proc"]
        self.os = domain_data["os"]
        self.include_folder = os.path.join(
            self.domain_path, domain_data["include_folder"]
        )
        self.lib_folder = os.path.join(self.domain_path, domain_data["lib_folder"])
        self.libsrc_folder = os.path.join(
            self.domain_path, domain_data["libsrc_folder"]
        )
        self.toolchain_file = os.path.join(
            self.domain_path, domain_data["toolchain_file"]
        )
        self.specs_file = os.path.join(
            self.domain_path, domain_data["specs_file"]
        )
        self.cmake_paths_append = f" -DCMAKE_LIBRARY_PATH={self.lib_folder} \
            -DCMAKE_INCLUDE_PATH={self.include_folder} \
            -DCMAKE_MODULE_PATH={self.domain_path} \
            -DCMAKE_TOOLCHAIN_FILE={self.toolchain_file} \
            -DCMAKE_SPECS_FILE={self.specs_file} \
            -DCMAKE_VERBOSE_MAKEFILE=ON"

        self.drvlist = self.getdrv_list()
        self.lib_config = domain_data["lib_config"]
        self.template = domain_data["template"]
        self.cmake_generator = utils.get_cmake_generator()

    def build_bsp(self):
        cmake_file = os.path.join(self.domain_path, "CMakeLists.txt")
        self.libsrc_folder = self.libsrc_folder.replace('\\','/')
        build_libxil = os.path.join(self.libsrc_folder, "build_configs/gen_bsp")
        utils.mkdir(build_libxil)
        self.domain_path = self.domain_path.replace('\\','/')
        self.cmake_paths_append = self.cmake_paths_append.replace('\\','/')
        build_libxil = build_libxil.replace('\\','/')
        utils.runcmd(f'cmake -G "{self.cmake_generator}" {self.domain_path} -DNON_YOCTO=ON -DSUBDIR_LIST="ALL" {self.cmake_paths_append}', cwd=build_libxil)
        utils.runcmd("cmake --build . --parallel 22 --verbose", cwd = build_libxil)
        utils.runcmd("cmake --install .", cwd=build_libxil)

    def getdrv_list(self):
        domain_data = utils.fetch_yaml_data(self.domain_config_file, "domain")
        drvlist = []
        for ip in domain_data["drv_info"].keys():
            if domain_data["drv_info"][ip] != "None":
                drvlist.append(domain_data["drv_info"][ip].get('driver',{}))

        if drvlist:
            # Remove duplicate references
            drvlist = list(dict.fromkeys(drvlist))
        return drvlist

def generate_bsp(args):
    """
    Function to compile the created bsp for the user input domain path.
    """
    obj = BSP(args)
    obj.build_bsp()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Build the created bsp",
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
    generate_bsp(args)
