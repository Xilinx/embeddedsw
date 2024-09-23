# Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
"""
This module builds an already created app. It doesn't contain any members
other than main().
"""

import utils
import os
import argparse
from build_bsp import generate_bsp
from build_bsp import BSP
from create_app import App
from validate_bsp import Validation
from open_amp import openamp_app_configure_common
from open_amp import openamp_lopper_run
from utils import is_file

class Build_App(BSP):
    """
    This class helps in building a template application.
    """

    def __init__(self, args):
        self._build_dir_struct(args)
        BSP.__init__(self, args)

    def _build_dir_struct(self, args):
        if args["ws_dir"] == '.' and not args.get('build_dir'):
            print("[WARNING]: The app Workspace is taken as current working directory. To avoid this, please use -w option")
        self.app_dir = utils.get_abs_path(args.get("ws_dir"))
        if args.get('src_dir'):
            self.app_src_dir = utils.get_abs_path(args["src_dir"])
        else:
            self.app_src_dir = os.path.join(self.app_dir, "src")
        if args.get('build_dir'):
            self.app_build_dir = utils.get_abs_path(args["build_dir"])
        else:
            self.app_build_dir = os.path.join(self.app_dir, "build")
        utils.mkdir(self.app_build_dir)
        self.app_config_file = os.path.join(self.app_src_dir, "app.yaml")
        self.domain_path = utils.fetch_yaml_data(self.app_config_file, "domain_path")["domain_path"]
        args["domain_path"] = self.domain_path

def build_app(args):
    obj = Build_App(args)

    # Build the bsp first before building application
    cmake_cache = os.path.join(obj.libsrc_folder, "build_configs", "gen_bsp", "CMakeCache.txt")
    if utils.is_file(cmake_cache):
        generate_bsp(args)

    # Run make inside cmake configured build area
    obj.app_src_dir = obj.app_src_dir.replace('\\', '/')
    obj.cmake_paths_append = obj.cmake_paths_append.replace('\\', '/')
    obj.app_build_dir = obj.app_build_dir.replace('\\', '/')

    app_name = utils.fetch_yaml_data(obj.app_config_file, "template")["template"]

    domain_data = utils.fetch_yaml_data(obj.domain_config_file, "domain")
    # in case of library update link libraries
    if domain_data['lib_info']:
        src_cmake = os.path.join(obj.app_src_dir, "CMakeLists.txt")
        lib_list = list(domain_data['lib_info'].keys())
        # Special handling for libmetal
        lib_list = [lib.replace('libmetal', 'metal') for lib in lib_list]
        if obj.os == "freertos":
            lib_list.append(obj.os)
        # FixME: Link the math library by default for libmetal dependent drivers
        if 'metal' in lib_list:
            lib_list.append('m')
        cpu_list_file = os.path.join(obj.domain_path, "cpulist.yaml")
        avail_cpu_data = utils.fetch_yaml_data(cpu_list_file, "cpulist")
        proc_ip_name = avail_cpu_data[obj.proc]
        if "microblaze_riscv" in proc_ip_name or "microblaze" in proc_ip_name:
            lib_list.append("gloss")
        cmake_lib_list = ';'.join(lib_list)
        utils.replace_line(
            src_cmake,
            f'PROJECT_LIB_DEPS xilstandalone',
            f'collect(PROJECT_LIB_DEPS xilstandalone;{cmake_lib_list})\n',
        )
    utils.runcmd(f'cmake -G "{obj.cmake_generator}" {obj.app_src_dir} {obj.cmake_paths_append}', cwd=obj.app_build_dir)
    utils.copy_file(f"{obj.app_build_dir}/compile_commands.json", obj.app_src_dir, silent_discard=True)
    utils.runcmd("cmake --build . --parallel 22 --verbose", cwd=obj.app_build_dir)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description=f"""\b
            Use this script to build the created app.
            It expects either -w <app ws path> or
            -s <app src dir path> and -b <app build dir path>
            passed during create_app""",
        usage='use "python %(prog)s --help" for more information',
        formatter_class=argparse.RawTextHelpFormatter,
    )

    # Get the app_path created by the user
    parser.add_argument(
        "-w",
        "--ws_dir",
        action="store",
        help="Specify the App Workspace Directory",
        default='.',
    )

    parser.add_argument(
        "-b",
        "--build_dir",
        action="store",
        help="Specify the App Build Directory",
    )

    parser.add_argument(
        "-s",
        "--src_dir",
        action="store",
        help="Specify the App source directory "
    )
    parser.add_argument(
        "-r",
        "--repo_info",
        action="store",
        help="Specify the .repo.yaml absolute path to use the set repo info",
        default='.repo.yaml',
    )

    args = vars(parser.parse_args())
    build_app(args)
