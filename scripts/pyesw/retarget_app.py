# Copyright (C) 2023 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
"""
This module updates the application meta-data app.yaml and CMakeLists.txt
for the new platform using the domain information provided.
"""

import argparse
import os
import utils
from build_bsp import BSP

logger = utils.get_logger(__name__)

class RetargetApp(BSP):
    def __init__(self, args):
        self.domain_path = utils.get_abs_path(args.get("domain_path"))
        BSP.__init__(self, args)
        self.app_src_dir = utils.get_abs_path(args["src_dir"])

def retarget_app(args):
    """
    Args:
        args (dict): Takes all the user inputs in a dictionary.
    """
    obj = RetargetApp(args)

    # Update the domain path as per new domain
    app_config_file = os.path.join(obj.app_src_dir, "app.yaml")
    template = utils.fetch_yaml_data(app_config_file, "template")["template"]

    # Read the current domain_path BEFORE updating it
    current_domain_path = utils.fetch_yaml_data(app_config_file, "domain_path")['domain_path']
    utils.update_yaml(app_config_file, "domain_path", "domain_path", obj.domain_path, action="add")

    # Compare and delete the build directory if domain_path has changed
    if current_domain_path != obj.domain_path:
        if args.get('build_dir'):
            app_build_dir = utils.get_abs_path(args["build_dir"])
            if utils.is_dir(app_build_dir):
                logger.info(f"Deleting existing build directory: {app_build_dir} (domain path mismatch detected)")
                utils.remove(app_build_dir)

    # Update link libraries as per new domain
    domain_data = utils.fetch_yaml_data(obj.domain_config_file, "domain")
    if domain_data['lib_info']:
        logger.info("Updating link libraries based on new domain")
        src_cmake = os.path.join(obj.app_src_dir, "CMakeLists.txt")
        lib_list = list(domain_data['lib_info'].keys())
        # Special handling for libmetal
        lib_list = [lib.replace('libmetal', 'metal') for lib in lib_list]
        # Special handling for openamp
        lib_list = [lib.replace('openamp', 'open_amp') for lib in lib_list]
        cmake_lib_list = ';'.join(lib_list)
        utils.replace_line(
            src_cmake,
            f'PROJECT_LIB_DEPS xilstandalone',
            f'collect(PROJECT_LIB_DEPS xilstandalone;{cmake_lib_list})',
        )

    """
    Delete the build folder in case of shared workspace
    """
    if args.get('build_dir'):
        app_build_dir = utils.get_abs_path(args["build_dir"])
        if utils.is_dir(app_build_dir):
            compile_commands_file = os.path.join(app_build_dir, "compile_commands.json")
            cmake_cache = os.path.join(app_build_dir, "CMakeCache.txt")
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
                        utils.remove(app_build_dir)
            elif utils.is_file(compile_commands_file):
                build_data = utils.load_json(compile_commands_file)
                if isinstance(build_data, list):
                    if build_data[0].get('directory', {}):
                        build_dir = build_data[0]["directory"]
                        if os.name == "nt":
                            build_dir = os.path.normpath(build_dir)
                            build_dir = os.path.normcase(build_dir.replace('\\', '/'))
                            app_build_dir = os.path.normcase(app_build_dir)
                        if build_dir != app_build_dir:
                            utils.remove(app_build_dir)
            else:
                """
                In case build folder doesn't have compile_commands.json delete the
                build folder for safer side.
                """
                logger.debug("No compile_commands.json found. Removing build directory for safety.")
                utils.remove(app_build_dir)

            """
            From 2024.2 release on wards cmake default generator moved to ninja as a part
            of performance enhancment in case of old release workspace update app CMakeLists.txt
            to inline with ninja generator.
            """
            is_ninja_build = os.path.join(app_build_dir, "build.ninja")
            if not utils.is_file(is_ninja_build):
                logger.debug("Ninja build file not found. Clearing build directory and updating CMakeLists.txt.")
                utils.remove(app_build_dir)
                src_cmake = os.path.join(obj.app_src_dir, "CMakeLists.txt")
                replace_header = f'''
if (${{NON_YOCTO}})
file(GLOB bsp_archives "${{CMAKE_LIBRARY_PATH}}/*.a")
set_source_files_properties(${{_sources}} OBJECT_DEPENDS "${{bsp_archives}}")
endif()
'''
                utils.replace_line(
                    src_cmake,
                    f'set_source_files_properties(${{_sources}} OBJECT_DEPENDS "${{CMAKE_LIBRARY_PATH}}/*.a")',
                    replace_header
                )

def main(arguments=None):
    parser = argparse.ArgumentParser(
        description="Use this script to change platform for a given application",
        usage='use "empyro retarget_app --help" for more information',
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
    required_argument.add_argument(
        "-s",
        "--src_dir",
        action="store",
        help="App source directory",
        required=True,
    )
    parser.add_argument(
        "-b",
        "--build_dir",
        action="store",
        help="Specify the App Build Directory",
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
    logger.info("Retargeting app")
    retarget_app(args)

if __name__ == "__main__":
    main()
