# Copyright (C) 2023 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
"""
This module creates the driver or library example using the domain information
provided to it. It generates the directory structure and the metadata
required to build a particular driver or library example.
"""

import argparse
import fileinput
import os
import sys
import textwrap
import utils
from build_bsp import BSP
from repo import Repo
from validate_bsp import Validation

logger = utils.get_logger(__name__)

class App(BSP, Repo):
    """
    This class helps in creating a driver or library application.
    It contains attributes and functions that take domain path,
    and template name as an input, create the directory structure
    and the metadata(if needed) for the app.
    """

    def __init__(self, args):
        BSP.__init__(self, args)
        Repo.__init__(self, repo_yaml_path=args['repo_info'])
        self._build_dir_struct(args)
        self.app_name = args.get("example")
        self.name = args.get("name")
        self.repo_paths_list = self.repo_schema['paths']

    def _build_dir_struct(self, args):
        """
        Creates the directory structure for Apps.
        """
        self.app_dir = utils.get_abs_path(args["ws_dir"])
        if args.get('src_dir'):
            self.app_src_dir = utils.get_abs_path(args["src_dir"])
        else:
            self.app_src_dir = os.path.join(self.app_dir, "src")

        utils.mkdir(self.app_src_dir)
        # App directory needs to have its own yaml configuration
        # (for compiler flags, linker flags etc.)
        self.app_config_file = os.path.join(self.app_src_dir, "app.yaml")

def create_example(args):
    """
    Function that uses the above App class to create the template application.

    Args:
        args (dict): Takes all the user inputs in a dictionary.
    """
    obj = App(args)
    logger.info(f"Creating example {obj.app_name}")

    # Copy the application src directory from embeddedsw to app src folder.
    domain_data = utils.fetch_yaml_data(obj.domain_config_file, "domain")
    # Check whether it is library or driver
    if obj.name in domain_data['drv_info']:
        comp_path = domain_data['drv_info'][obj.name]['path']
    else:
        comp_path = domain_data['lib_info'][obj.name]['path']
    esw_app_dir = os.path.join(comp_path, "examples")
    subdir_dep_list = []
    additional_cmake_dep = []
    # Get the dependency list
    if obj.name in domain_data['drv_info']:
        dep_list = domain_data['drv_info'][obj.name]['examples'][obj.app_name]
    else:
        dep_list = domain_data['lib_info'][obj.name]['examples'][obj.app_name]
    if dep_list:
        for dep in dep_list:
            dep_path = os.path.join(esw_app_dir, dep)
            dest_path = os.path.join(obj.app_src_dir, dep)  # Preserve directory name
            if utils.is_dir(dep_path):
                # copy the directories
                utils.copy_directory(dep_path, dest_path)
                subdir_dep_list.append(dep)
            elif utils.is_file(dep_path):
                # copy the files
                if dep.endswith(".cmake"):
                    additional_cmake_dep.append(dep)
                utils.copy_file(dep_path, obj.app_src_dir)
            else:
                logger.warning(f" {dep_path} does not exist")

    app_file = os.path.join(esw_app_dir, obj.app_name)
    utils.copy_file(app_file, obj.app_src_dir)

    esw_app_dir = obj.get_comp_dir("empty_application")
    srcdir = os.path.join(esw_app_dir, "src")
    utils.copy_directory(srcdir, obj.app_src_dir)

    src_cmake = os.path.join(obj.app_src_dir, "CMakeLists.txt")
    app_name = os.path.splitext(utils.get_base_name(obj.app_name))[0]
    logger.debug("Updating CMakeLists.txt with application name.")
    utils.replace_line(
        src_cmake,
        f'APP_NAME empty_application',
        f'set(APP_NAME {app_name})',
    )
    add_subdir_string = ""
    if subdir_dep_list:
        for dir_dep in subdir_dep_list:
            add_subdir_string += f"add_subdirectory({dir_dep})\n"
        utils.replace_line(
            src_cmake,
            "# Add subdirectory dependency if available",
            f"# Add subdirectory dependency if available \n{add_subdir_string}")

    additonal_cmake_file_str = ""
    # Add any additional CMake dependencies
    if additional_cmake_dep:
        for dep in additional_cmake_dep:
            additonal_cmake_file_str += f"include(${{CMAKE_CURRENT_SOURCE_DIR}}/{dep})\n"
        utils.replace_line(
            src_cmake,
            "# Include any additional CMake files here",
            f"# Include any additional CMake files here \n{additonal_cmake_file_str}")

    # in case of library update link libraries
    if domain_data['lib_info']:
        lib_list = list(domain_data['lib_info'].keys())
        cmake_lib_list = ';'.join(lib_list)
        logger.debug(f"Updating library dependencies in CMakeLists.txt with {cmake_lib_list}")
        utils.replace_line(
            src_cmake,
            f'PROJECT_LIB_DEPS xilstandalone',
            f'collect(PROJECT_LIB_DEPS xilstandalone;{cmake_lib_list})',
        )

    # Generates the metadata for linker script
    linker_cmd = (
        f"lopper -O {obj.app_src_dir} {obj.sdt} -- baremetallinker_xlnx {obj.proc} {srcdir}"
    )

    utils.runcmd(linker_cmd,
        log_message = f"Generating Linker script for {obj.app_name}" ,
        error_message = f"Failed to generate Linker script for {obj.app_name}" ,
        verbose_level = 0
    )
    # Copy the static linker files from embeddedsw to the app src dir
    linker_dir = os.path.join(obj.app_src_dir, "linker_files")
    linker_src = utils.get_high_precedence_path(
            obj.repo_paths_list, "Linker file directory", "scripts", "linker_files"
        )
    utils.copy_directory(linker_src, linker_dir)
    # Copy the User Configuration cmake file to the app src dir
    user_config_cmake = utils.get_high_precedence_path(
            obj.repo_paths_list, "UserConfig.cmake file", "cmake", "UserConfig.cmake"
        )
    utils.copy_file(user_config_cmake, obj.app_src_dir)

    # Add domain path entry in the app configuration file.
    data = {"domain_path": obj.domain_path,
            "app_src_dir": esw_app_dir,
            "template": "empty_application"
        }
    utils.write_yaml(obj.app_config_file, data)

    # Create a dummy folder to get compile_commands.json
    compile_commands_dir = os.path.join(obj.app_src_dir, ".compile_commands")
    utils.mkdir(compile_commands_dir)
    obj.app_src_dir = obj.app_src_dir.replace('\\', '/')
    obj.cmake_paths_append = obj.cmake_paths_append.replace('\\', '/')
    dump = utils.discard_dump()
    utils.runcmd(
        f'cmake -G "{obj.cmake_generator}" {obj.app_src_dir} {obj.cmake_paths_append} -DUSER_LINK_LIBRARIES={obj.name} > {dump}',
        cwd = compile_commands_dir,
        log_message = "Running CMake Configuration",
        error_message = "Failed to run CMake Configuration",
        verbose_level = 0
    )
    '''
    compile_commands.json file needs to be kept inside src directory.
    Silent_discard needs to be true as for Empty Application, this file
    is not created.
    '''
    compile_commands_file = os.path.join(f"{compile_commands_dir}", "compile_commands.json")
    utils.copy_file(compile_commands_file, obj.app_src_dir, silent_discard=True)

    '''
    There are few GCC flags (e.g. -fno-tree-loop-distribute-patterns) that
    clang server doesnt recognise for Code Intellisense. To get over this
    "Unknown Argument" Error of clang, a .clangd file with below content is
    to be kept in parallel to compile_commands.json file.
    '''
    clangd_ignore_content = f'''
CompileFlags:
    Add: [-Wno-unknown-warning-option, -U__linux__, -U__clang__]
    Remove: [-m*, -f*]
'''
    clangd_ignore_file = os.path.join(obj.app_src_dir, ".clangd")
    utils.write_into_file(clangd_ignore_file, clangd_ignore_content)

    '''
    The generated compile_commands.json file has the directory path (where it
    was created originally) in it. That directory needs to be maintained to
    avoid clang error.
    '''
    utils.remove(os.path.join(compile_commands_dir, "*"), pattern=True)

    # Success prints if everything went well till this point.
    if utils.is_file(obj.app_config_file):
        logger.info(f"Successfully created example sources at {obj.app_src_dir}")

def main(arguments=None):
    parser = argparse.ArgumentParser(
        description="Use this script to create driver or library example using the BSP path",
        usage='use "empyro create_example --help" for more information',
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
        "-w", "--ws_dir", action="store", help="Workspace directory (Default: Current Work Directory)", default='.'
    )
    parser.add_argument(
        "-s", "--src_dir", action="store", help="App source directory (Default: <Current Work Directory>/src)"
    )
    required_argument.add_argument(
        "-e",
        "--example",
        action="store",
        help="Application name",
        required=True,
    )
    required_argument.add_argument(
        "-n",
        "--name",
        action="store",
        help="Specify the Instance name (OR) library name",
        required=True,
    )
    parser.add_argument(
        "-v",
        "--verbose",
        action="count",
        default=0,
        help='Increase output verbosity'
    )
    parser.add_argument(
        "-r",
        "--repo_info",
        action="store",
        help="Specify the .repo.yaml absolute path to use the set repo info",
        default='.repo.yaml',
    )
    args = vars(parser.parse_args(arguments))
    utils.setup_log(args["verbose"])
    create_example(args)

if __name__ == "__main__":
    main()