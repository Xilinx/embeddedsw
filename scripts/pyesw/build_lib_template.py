# Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
"""
Build standalone library from workspace.
"""

import argparse
import os
import utils
from build_bsp import BSP

logger = utils.get_logger(__name__)

class Build_StaticLibrary(BSP):
    """
    This class helps in building a standalone static library.
    """

    def __init__(self, args):
        self._build_dir_struct(args)
        BSP.__init__(self, args)

    def _build_dir_struct(self, args):
        """Creates the directory structure for library build."""
        self.lib_dir = utils.get_abs_path(args["ws_dir"])
        if args.get('src_dir'):
            self.lib_src_dir = utils.get_abs_path(args["src_dir"])
        else:
            self.lib_src_dir = os.path.join(self.lib_dir, "src")
        if args.get('build_dir'):
            self.lib_build_dir = utils.get_abs_path(args["build_dir"])
        else:
            self.lib_build_dir = os.path.join(self.lib_dir, "build")
        utils.mkdir(self.lib_build_dir)

        # Get BSP reference from YAML config
        self.lib_config_file = os.path.join(self.lib_src_dir, "lib.yaml")
        config_data = utils.fetch_yaml_data(self.lib_config_file, "domain_path")
        self.domain_path = config_data["domain_path"]
        args["domain_path"] = self.domain_path

def build_lib_template(args):
    """Build a standalone library."""
    parser = argparse.ArgumentParser(description="Build standalone library from workspace")
    parser.add_argument('-w', '--ws_dir', help='Specify the Library Workspace Directory)', default='.')
    parser.add_argument('--src_dir', help='Specify the Library source Directory')
    parser.add_argument('--build_dir', help='Specify the Library Build Directory')
    parser.add_argument('-v', '--verbose', action='count', default=0, help='Increase output verbosity')

    parsed = parser.parse_args(args)
    args_dict = {
        'ws_dir': parsed.ws_dir,
        'src_dir': parsed.src_dir,
        'build_dir': parsed.build_dir,
        'verbose': parsed.verbose
    }

    utils.setup_log(args_dict["verbose"])

    obj = Build_StaticLibrary(args_dict)

    # Run CMake configuration
    obj.lib_src_dir = obj.lib_src_dir.replace('\\', '/')
    obj.cmake_paths_append = obj.cmake_paths_append.replace(f" -DCMAKE_LIBRARY_PATH={obj.lib_folder}","")
    obj.cmake_paths_append = obj.cmake_paths_append.replace('\\', '/')
    obj.lib_build_dir = obj.lib_build_dir.replace('\\', '/')

    utils.runcmd(
        f'cmake -G "{obj.cmake_generator}" {obj.lib_src_dir} {obj.cmake_paths_append} > {utils.discard_dump()}',
        cwd=obj.lib_build_dir,
        log_message="Configuring CMake for the Library",
        error_message="CMake Configuration for the Library Failed",
        verbose_level = 0
    )
    # Build
    verbosity = utils.get_cmake_verbosity(obj.verbose)
    capture_output = True if verbosity == "" else False
    utils.runcmd(
        f"cmake --build . --parallel 22 {verbosity}",
        cwd=obj.lib_build_dir,
        log_message="Building Library",
        error_message="Library Building Failed",
        capture_output=capture_output,
        verbose_level=0
    )
    utils.runcmd(
        f"cmake --install {obj.lib_build_dir}",
        cwd=obj.lib_build_dir,
        log_message="Copying headers and built archives",
        error_message="Failed to copy headers and built archive, cmake install failed",
        verbose_level = 0
    )

    logger.info(f"Successfully built library at: {obj.lib_build_dir}")
