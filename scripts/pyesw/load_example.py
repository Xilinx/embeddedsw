# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
"""
This module loads the driver example meta-data
"""
import utils
import argparse
import os
from build_bsp import BSP

def cmake_drv_custom_target(proc, libsrc_folder, sdt, cmake_drv_name_list, cmake_drv_path_list):
    cmake_cmd = f'''
set(DRIVER_TARGETS {cmake_drv_name_list})
set(DRIVER_LOCATIONS {cmake_drv_path_list})

list(LENGTH DRIVER_TARGETS no_of_drivers)
set(index 0)

while(${{index}} LESS ${{no_of_drivers}})
    list(GET DRIVER_TARGETS ${{index}} drv)
    list(GET DRIVER_LOCATIONS ${{index}} drv_dir)
    set(exp_dir "${{drv_dir}}/examples")
    add_custom_target(
        ${{drv}}_example ALL
        COMMAND lopper -f -O {libsrc_folder}/${{drv}} {sdt} -- bmcmake_metadata_xlnx {proc} ${{exp_dir}} drvcmake_metadata
        BYPRODUCTS x${{drv}}_exlist.yaml)
    MATH(EXPR index "${{index}}+1")
endwhile()
'''
    return cmake_cmd

class LoadExample(BSP):
    """
    This class contains attributes and functions to load the example meta-data in
    bsp.yaml file, It takes the domain as input, reads the domain configuration
    file present in the path to get the required inputs and updates the bsp.yaml.
    """
    def __init__(self, args):
        self.domain_path = utils.get_abs_path(args.get("domain_path"))
        BSP.__init__(self, args)

    def update_example(self):
        domain_data = utils.fetch_yaml_data(self.domain_config_file, "domain")
        build_metadata = os.path.join(self.libsrc_folder, "build_configs", "exmetadata")

        # If example metadata is already created return
        if utils.is_dir(build_metadata):
            return

        iplist = list(domain_data["drv_info"].keys())
        driver_name_list = []
        driver_path_list = []
        for ip in domain_data["drv_info"]:
            if domain_data["drv_info"][ip] != "None":
                driver_name_list.append(domain_data["drv_info"][ip].get('driver',{}))
                driver_path_list.append(domain_data["drv_info"][ip].get('path',{}))

        driver_name_list = list(dict.fromkeys(driver_name_list))
        cmake_drv_name_list = ';'.join(driver_name_list)
        driver_path_list = list(dict.fromkeys(driver_path_list))
        cmake_drv_path_list = ';'.join(driver_path_list)

        # Create top level CMakeLists.txt inside domain dir
        cmake_file = os.path.join(build_metadata, "CMakeLists.txt")
        utils.mkdir(build_metadata)

        cmake_header = """
cmake_minimum_required(VERSION 3.15)
project(bsp)
        """
        cmake_file_cmds = cmake_header
        cmake_file_cmds += cmake_drv_custom_target(self.proc, self.libsrc_folder, self.sdt, cmake_drv_name_list, cmake_drv_path_list)

        self.cmake_paths_append = self.cmake_paths_append.replace('\\','/')
        cmake_file_cmds = cmake_file_cmds.replace('\\', '/')
        utils.write_into_file(cmake_file, cmake_file_cmds)
        utils.runcmd(
            f'cmake -G "{self.cmake_generator}" {build_metadata} {self.cmake_paths_append}',
            cwd = build_metadata
        )

        utils.runcmd("cmake --build . --parallel 22 --verbose > nul", cwd = build_metadata)
        for ip,data in domain_data['drv_info'].items():
            if data != "None":
                driver = data['driver']
                drv_ex_list_yaml = os.path.join(self.libsrc_folder, driver, f"{driver}_exlist.yaml")
                if utils.is_file(drv_ex_list_yaml):
                    driver_ex = utils.load_yaml(drv_ex_list_yaml)
                    if len(driver_ex) != 0:
                        domain_data["drv_info"][ip].update({"examples":driver_ex[ip]})

        utils.update_yaml(self.domain_config_file, "domain", "drv_info", domain_data["drv_info"])
        for drv in driver_name_list:
            utils.remove(os.path.join(self.libsrc_folder, drv,  "*.cmake"), pattern=True)
            utils.remove(os.path.join(self.libsrc_folder, drv, "*.yaml"), pattern=True)

def load_bsp(args):
    """
    Function to load the example meta-data for a given domain.
    """
    obj = LoadExample(args)
    obj.update_example()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Load the example meta-data for a given domain",
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
    load_bsp(args)
