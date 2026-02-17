# Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
"""
Create standalone library workspace from BSP.
"""

import argparse
import os
import utils
from build_bsp import BSP
from repo import Repo

logger = utils.get_logger(__name__)

class StaticLibrary(BSP, Repo):
    """
    This class creates a standalone static library workspace from BSP.
    It inherits from BSP to access BSP information and Repo to access embeddedsw paths.
    """

    def __init__(self, args):
        BSP.__init__(self, args)
        Repo.__init__(self, repo_yaml_path=args.get('repo_info', '.repo.yaml'))
        self.lib_name = args.get("name")
        self.repo_paths_list = self.repo_schema['paths']
        self._build_dir_struct(args)

    def _build_dir_struct(self, args):
        """Creates the directory structure for library."""
        self.lib_dir = utils.get_abs_path(args["ws_dir"])

        if args.get('src_dir'):
            self.lib_src_dir = utils.get_abs_path(args["src_dir"])
        else:
            self.lib_src_dir = os.path.join(self.lib_dir, "src")

        utils.mkdir(self.lib_src_dir)
        self.lib_config_file = os.path.join(self.lib_src_dir, "lib.yaml")

    def create(self):
        """Create the library workspace structure."""
        # Generate CMakeLists.txt
        cmake_content = f"""cmake_minimum_required(VERSION 3.15)

set(LIB_NAME {self.lib_name})
project(${{LIB_NAME}} C)


# Include any additional CMake files here
include(${{CMAKE_CURRENT_SOURCE_DIR}}/UserConfig.cmake)

# Collect all C source files
aux_source_directory(${{CMAKE_CURRENT_SOURCE_DIR}} LIB_SOURCES)
list(APPEND LIB_SOURCES ${{USER_COMPILE_SOURCES}})

# Separate USER_COMPILE_SOURCES into C sources and headers
foreach(src ${{USER_COMPILE_SOURCES}})
    if(src MATCHES "\\\\.h$")
        list(APPEND USER_HEADERS ${{src}})
    endif()
endforeach()

# Collect local header files
file(GLOB LIB_HEADERS "${{CMAKE_CURRENT_SOURCE_DIR}}/*.h")
list(APPEND LIB_HEADERS ${{USER_HEADERS}})

# Create static library
add_library(${{LIB_NAME}} STATIC ${{LIB_SOURCES}})

# Apply user-defined compiler options (warnings, optimization, debug flags, etc.)
string(APPEND CMAKE_C_FLAGS ${{USER_COMPILE_OPTIONS}})

# Set include directories - use CMAKE_INCLUDE_PATH from BSP
target_include_directories(${{LIB_NAME}}
    PUBLIC ${{CMAKE_CURRENT_SOURCE_DIR}}
    PUBLIC ${{USER_INCLUDE_DIRECTORIES}}
    PRIVATE ${{CMAKE_INCLUDE_PATH}}
)

# Apply user-defined compiler definitions (-D flags)
target_compile_definitions(${{LIB_NAME}} PUBLIC ${{USER_COMPILE_DEFINITIONS}})

# Install headers to build include directory
if(LIB_HEADERS)
    install(FILES ${{LIB_HEADERS}} DESTINATION ${{CMAKE_BINARY_DIR}}/include)
endif()
"""
        lib_cmake = os.path.join(self.lib_src_dir, "CMakeLists.txt")
        utils.write_into_file(lib_cmake, cmake_content)

        # Copy the User Configuration cmake file to the lib src dir
        user_config_cmake = utils.get_high_precedence_path(
            self.repo_paths_list, "UserConfig.cmake file", "cmake", "UserConfig.cmake"
        )
        utils.copy_file(user_config_cmake, self.lib_src_dir)

        # Add domain path entry in the library configuration file
        data = {"domain_path": self.domain_path}
        utils.write_yaml(self.lib_config_file, data)

        logger.info(f"Created library sources at: {self.lib_src_dir}")

def create_lib_template(args):
    """
    Function that uses the StaticLibrary class to create the template library.

    Args:
        args: Command line arguments
    """
    parser = argparse.ArgumentParser(description="Create standalone library from BSP")
    parser.add_argument('-d', '--domain_path', required=True, help='BSP directory path')
    parser.add_argument('-n', '--name', help='Library name (default: custom)', default='custom')
    parser.add_argument('-w', '--ws_dir', help='Workspace directory (Default: Current Work Directory)', default='.')
    parser.add_argument('--src_dir', help='Library source directory (Default: <ws_dir>/src)')
    parser.add_argument('-r', '--repo_info', default='.repo.yaml', help='Specify the .repo.yaml absolute path to use the set repo info')
    parser.add_argument('-v', '--verbose', action='count', default=0, help='Increase output verbosity')

    parsed = parser.parse_args(args)
    args_dict = {
        'domain_path': parsed.domain_path,
        'name': parsed.name,
        'ws_dir': parsed.ws_dir,
        'src_dir': parsed.src_dir,
        'repo_info': parsed.repo_info,
        'verbose': parsed.verbose
    }

    utils.setup_log(args_dict["verbose"])
    obj = StaticLibrary(args_dict)
    obj.create()
