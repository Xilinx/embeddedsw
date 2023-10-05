# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
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
    libxil_a_path = os.path.join(obj.domain_path, 'lib', 'libxil.a')
    libxilstandalone_a_path = os.path.join(obj.domain_path, 'lib', 'libxilstandalone.a')

    if not utils.is_file(libxil_a_path) or not utils.is_file(libxilstandalone_a_path):
        generate_bsp(args)

    # Run make inside cmake configured build area
    obj.app_src_dir = obj.app_src_dir.replace('\\', '/')
    obj.cmake_paths_append = obj.cmake_paths_append.replace('\\', '/')
    obj.app_build_dir = obj.app_build_dir.replace('\\', '/')

    app_name = utils.fetch_yaml_data(obj.app_config_file, "template")["template"]
    bsp_obj = BSP(args)
    overlay_path = os.path.join(bsp_obj.domain_path, 'hw_artifacts', 'domain.yaml')
    if app_name in ['openamp_echo_test', 'openamp_matrix_multiply', 'openamp_rpc_demo'] and is_file(overlay_path):
        bsp_obj = BSP(args)

        original_sdt = os.path.join(bsp_obj.domain_path, 'hw_artifacts', 'sdt.dts')
        print('Domain YAML is found. Passing this in to a OpenAMP Lopper run to generate platform info header.')
        enable_lopper = True
        openamp_lopper_run(overlay_path, original_sdt, obj.app_src_dir)

        args['src_dir'] = bsp_obj.domain_path
        args['template'] = app_name
        args['ws_dir'] = obj.app_dir
        app_obj = App(args)
        esw_app_dir = app_obj.get_comp_dir(app_name)
        obj.template = app_name
        openamp_app_configure_common(obj, esw_app_dir, enable_lopper)

    domain_data = utils.fetch_yaml_data(obj.domain_config_file, "domain")
    # in case of library update link libraries
    if domain_data['lib_info']:
        src_cmake = os.path.join(obj.app_src_dir, "CMakeLists.txt")
        lib_list = list(domain_data['lib_info'].keys())
        # Special handling for libmetal
        lib_list = [lib.replace('libmetal', 'metal') for lib in lib_list]
        cmake_lib_list = ';'.join(lib_list)
        utils.replace_line(
            src_cmake,
            'xiltimer',
            f'collect(PROJECT_LIB_DEPS {cmake_lib_list})\n',
        )
    utils.runcmd(f'cmake -G "Unix Makefiles" {obj.app_src_dir} {obj.cmake_paths_append} -DNON_YOCTO=ON', cwd=obj.app_build_dir)
    utils.copy_file(f"{obj.app_build_dir}/compile_commands.json", obj.app_src_dir, silent_discard=True)
    utils.runcmd("make -j22", cwd=obj.app_build_dir)

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
