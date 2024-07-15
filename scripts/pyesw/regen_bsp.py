# Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
"""
This module re creates the bsp for a given domain and system device-tree.
"""

import utils
import argparse
import os
from create_bsp import Domain, create_domain
from library_utils import Library
from build_bsp import BSP
import inspect

class RegenBSP(BSP, Library):
    """
    This class contains attributes and functions to regenerate the bsp.
    It takes the domain path and sdt as inputs, reads the domain configuration
    file present in the path to get the required inputs.
    """

    def __init__(self, args):
        self.domain_path = utils.get_abs_path(args.get("domain_path"))
        BSP.__init__(self, args)
        domain_data = utils.fetch_yaml_data(self.domain_config_file, "domain")
        if "mode" in domain_data:
            self.proc_mode = domain_data["mode"]
        else:
            self.proc_mode = "64-bit"
        if args.get('sdt'):
            self.sdt = utils.get_abs_path(args["sdt"])
        if utils.is_file(os.path.join(self.domain_path, ".repo.yaml")):
            self.repo_info = os.path.join(self.domain_path, ".repo.yaml")
        else:
            self.repo_info = args['repo_info']

        Library.__init__(
            self,
            self.domain_path,
            self.proc,
            self.os,
            self.sdt,
            self.cmake_paths_append,
            self.libsrc_folder,
            self.repo_info
        )

    def modify_bsp(self, args):
        args.update({
            'ws_dir':self.domain_path,
            'proc':self.proc,
            'os':self.os,
            'template':self.template,
            'sdt':self.sdt,
            'repo_info':self.repo_info,
            'mode': self.proc_mode
        })

        # Remove existing folder structure
        utils.remove(self.libsrc_folder)
        utils.remove(self.include_folder)
        utils.remove(self.lib_folder)

        # Create BSP
        create_domain(args)

        # Get the library list in the new bsp
        domain_data = utils.fetch_yaml_data(self.domain_config_file, "domain")
        lib_list = list(domain_data["lib_config"].keys()) + [self.proc, self.os]
        drvlist = BSP.getdrv_list(self)
        lib_diff_old_to_new = [lib for lib in self.lib_list if lib not in lib_list]

        libs_to_add = []
        # Check library list against existing lib_list in the previous bsp.yaml
        # if lib is missing add the library
        cmake_file = os.path.join(self.domain_path, "CMakeLists.txt")
        build_folder = os.path.join(self.libsrc_folder, "build_configs")
        ignored_lib_list = []
        for lib in lib_diff_old_to_new:
            if not self.validate_drv_for_lib(lib, drvlist):
                ignored_lib_list.append(lib)
                continue
            self.gen_lib_metadata(lib)
            libs_to_add += [lib]
        if libs_to_add:
            self.config_lib(None, libs_to_add, "", is_app=False)
            domain_data = utils.fetch_yaml_data(self.domain_config_file, "domain")

        build_metadata = os.path.join(self.libsrc_folder, "build_configs", "gen_bsp")
        lib_config = domain_data["lib_config"]
        os_config = domain_data["os_config"]
        proc_config = domain_data["proc_config"]

        # Apply the Old software configuraiton
        cmake_cmd_append = ""
        # cmake syntax is using 'ON/OFF' option, 'True/False' is lagacy entry.
        bool_match = {"true": "ON", "false": "OFF"}
        for lib in self.lib_list:
            for key, value in self.bsp_lib_config[lib].items():
                val = value['value']
                if lib == self.proc:
                    proc_config[lib][key]['value'] = val
                elif lib == self.os:
                    os_config[lib][key]['value'] = val
                else:
                    lib_config[lib][key]['value'] = val
                if val in bool_match:
                    val = bool_match[val]
                cmake_cmd_append += f' -D{key}="{val}"'

        self.lib_list.remove(self.proc)
        if self.os == 'freertos':
            self.lib_list.remove("freertos")
            self.lib_list.append("freertos10_xilinx")
        cmake_subdir_list = ";".join(self.lib_list)
        utils.runcmd(f'cmake {self.domain_path} {self.cmake_paths_append} -DSUBDIR_LIST="{cmake_subdir_list}" {cmake_cmd_append}', cwd=build_metadata)

        utils.update_yaml(self.domain_config_file, "domain", "lib_config", lib_config)
        utils.update_yaml(self.domain_config_file, "domain", "proc_config", proc_config)
        utils.update_yaml(self.domain_config_file, "domain", "os_config", os_config)

        # In case of Re generate BSP with different SDT print differences
        add_drv_list = [drv for drv in drvlist if drv not in self.drvlist]
        del_drv_list = [drv for drv in self.drvlist if drv not in drvlist]
        if add_drv_list or del_drv_list or ignored_lib_list:
            print(f"During Regeneration of BSP")
            if add_drv_list:
                print(f"Drivers {*add_drv_list,} got added")
            if del_drv_list:
                print(f"Drivers {*del_drv_list,} got deleted")
            if ignored_lib_list:
                print(f"Libraries {*ignored_lib_list,} ignored due to incompatible with new system device-tree")


def regenerate_bsp(args):
    """
    Function to re generate the bsp for the user input domain path and system device-tree.
    """
    obj = RegenBSP(args)
    obj.modify_bsp(args)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Regenerate the BSP",
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
    parser.add_argument(
        "-s",
        "--sdt",
        action="store",
        help="Specify the System device-tree path (till system-top.dts file)"
    )
    parser.add_argument(
        "-r",
        "--repo_info",
        action="store",
        help="Specify the .repo.yaml absolute path to use the set repo info",
        default='.repo.yaml',
    )
    args = vars(parser.parse_args())
    regenerate_bsp(args)
