# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
"""
This module enables utilities for OpenAMP and Libmetal. This includes
enabling these libraries in BSPs, application configure and application
build steps.
"""

import os
import re
import sys
import lopper

import utils
from utils import is_file

APP_NAMES = {
    'openamp_echo_test': 'echo',
    'openamp_matrix_multiply': 'matrix_multiply',
    'openamp_rpc_demo': 'rpc_demo'
}

def open_amp_app_name(app_name):
    if app_name not in APP_NAMES.keys():
        print("ERROR: ", app_name, " not found in list of openamp apps")
        sys.exit(1)

    return APP_NAMES[app_name]

def open_amp_copy_lib_src(libdir, dstdir, lib):
    """
    Copies the src directory of the passed library from the respective path
    of OpenAMP or Libmetal to the libsrc folder of bsp.

    Args:
        libdir (str): location for library in embeddedsw for cmake info.
        dstdir (str): destionation to copy lib sources to

    Returns:
        None
    """
    # copy specific cmake files
    srcdir = os.path.join(libdir, "src", "sdt")
    top_srcdir = os.path.join(srcdir, 'top-CMakeLists.txt')

    top_dstdir = os.path.join(dstdir, 'CMakeLists.txt')
    utils.copy_file(top_srcdir, top_dstdir)

    # Add this file to specify cleanup properly of the library
    new_lib_cmake = os.path.join(libdir, "src", "sdt", "lib-CMakeLists.txt")
    utils.copy_file(new_lib_cmake, os.path.join(dstdir, "lib", "CMakeLists.txt"))

    if lib == 'openamp':
        new_depends_cmake = os.path.join(libdir, "src", "sdt", "depends.cmake")
        utils.copy_file(new_depends_cmake, os.path.join(dstdir, "cmake", "depends.cmake"))

    lib_cmakelist = os.path.join(dstdir, 'lib')
    lib_cmakelist = os.path.join(lib_cmakelist, 'CMakeLists.txt')
    with open(lib_cmakelist, 'r', encoding='utf-8') as file:
        content = file.read()
        # Tell CMake build to install headers in BSP include area
        content = content.replace("DESTINATION include RENAME ${PROJECT_NAME}/${f})",
                                  "DESTINATION ${CMAKE_INCLUDE_PATH}/ RENAME ${PROJECT_NAME}/${f})")
        # Tell CMake build to install library in BSP library area

        # libmetal specific logic. Only picked up in case of libmetal
        orig = "install (TARGETS ${_lib} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})"
        new = "install (TARGETS ${_lib} ARCHIVE DESTINATION ${CMAKE_LIBRARY_PATH})"
        content = content.replace(orig, new)
        # open-amp specific logic. Only picked up in case of open-amp
        orig = "install (DIRECTORY \"${CMAKE_CURRENT_SOURCE_DIR}/include/openamp\" "
        orig += "DESTINATION include)"
        new = "install (DIRECTORY \"${CMAKE_CURRENT_SOURCE_DIR}/include/openamp\" "
        new += "DESTINATION ${CMAKE_INCLUDE_PATH}/)"
        content = content.replace(orig, new)
        orig = "install (DIRECTORY \"${PROJECT_BINARY_DIR}/include/generated/openamp\" "
        orig += "DESTINATION include)"
        new = "install (DIRECTORY \"${PROJECT_BINARY_DIR}/include/generated/openamp\" "
        new += "DESTINATION ${CMAKE_INCLUDE_PATH}/)"
        content = content.replace(orig, new)

    lib_dstdir = os.path.join(dstdir, 'lib')
    lib_dstdir = os.path.join(lib_dstdir, 'CMakeLists.txt')
    with open(lib_dstdir, 'w', encoding='utf-8') as file:
        file.write(content)

def openamp_app_configure_common(obj, esw_app_dir, enable_generated_header=False):
    """
    Template that can be used to copy toolchain file and ensure it is present
    in CMake configure step
    Args:
        obj (App): App object that contains Takes all the user inputs in a dictionary.
        esw_app_dir (str): location to copy source in repo from
    Returns:
        None
    """
    if obj.template in ['openamp_echo_test', 'openamp_matrix_multiply', 'openamp_rpc_demo']:
        new_src = os.path.join(esw_app_dir, '..', 'openamp_sdt_common', 'src', 'sdt')
        obj.cmake_paths_append += ' -DOPENAMP_APP_NAME=' + APP_NAMES[obj.template]
    else:
        new_src = os.path.join(esw_app_dir, 'src', 'sdt')

    if enable_generated_header:
        obj.cmake_paths_append += " -D_AMD_GENERATED_=ON "

def create_app(mappings, obj, esw_app_dir):
    """
    Template that can be used to copy repo sources for apps.

    This can be used for OpenAMP and Libmetal apps.

    Args:
        mappings (dict): dictionary of source to destination files
        obj (App): App object that contains Takes all the user inputs in a dictionary.
        esw_app_dir (str): path to application files in embeddedsw repo
    Returns:
        None
    """
    if obj.template in ['openamp_echo_test', 'openamp_matrix_multiply', 'openamp_rpc_demo']:
        new_src = os.path.join(esw_app_dir, '..', 'openamp_sdt_common', 'src', 'sdt')
    else:
        new_src = os.path.join(esw_app_dir, 'src', 'sdt')

    for key in mappings.keys():
        src = os.path.join(new_src, key)
        dst = os.path.join(obj.app_src_dir, mappings[key])
        utils.copy_file(src, dst, silent_discard=True)

    openamp_app_configure_common(obj, esw_app_dir)

def create_openamp_app(obj, esw_app_dir):
    """
    Copies the src directory of the passed library from the respective path
    of OpenAMP or Libmetal to the libsrc folder of bsp.
    Args:
        obj (App): App object that contains Takes all the user inputs in a dictionary.
        esw_app_dir (str): path to application files in embeddedsw repo
    Returns:
        None
    """

    common_mappings = {
        'top-CMakeLists.txt'         : ['CMakeLists.txt'],
        'app-CMakeLists.txt'         : ['apps', 'CMakeLists.txt'],
        'app-example-CMakeLists.txt' : ['apps', 'examples', 'CMakeLists.txt'],
    }

    app_cmake_name = APP_NAMES[obj.template]
    common_mappings['demo-dir-CMakeLists.txt'] = ['apps', 'examples',
                    app_cmake_name, 'CMakeLists.txt']

    # Create new mappings based on app name. Each record is (embeddedsw-src : app-workspace
    # location)
    #
    # Paths must be generated to ensure that paths are system
    # agnostic using os.path.join(). This is because paths are different
    # on Windows vs. UNIX based systems.
    #
    new_file_mappings = {}
    for key in common_mappings.keys():
        path_array = common_mappings[key]
        esw_path_str  = path_array[0]

        # for each subsequent element after the first, append to path
        for i in path_array[1:]:
            esw_path_str = os.path.join(esw_path_str,  i)

        new_file_mappings[key] = esw_path_str

    create_app(new_file_mappings, obj, esw_app_dir)

def create_libmetal_app(obj, esw_app_dir):
    """
    Copies the src directory of the passed library from the respective path
    of OpenAMP or Libmetal to the libsrc folder of bsp.
    Args:
        obj (App): App object that contains Takes all the user inputs in a dictionary.
        esw_app_dir (str): path to application files in embeddedsw repo
    Returns:
        None
    """

    app_os = obj.os if obj.os == 'freertos' else 'generic'
    common_mappings = {
        'top-CMakeLists.txt'         : ['CMakeLists.txt'],
    }

    # Create new mappings based on app name. Each record is (embeddedsw-src : app-workspace
    # location)
    #
    # Paths must be generated to ensure that paths are system
    # agnostic using os.path.join(). This is because paths are different
    # on Windows vs. UNIX based systems.
    #
    new_file_mappings = {}
    for key in common_mappings.keys():
        path_array = common_mappings[key]
        esw_path_str  = path_array[0]

        # for each subsequent element after the first, append to path
        for i in path_array[1:]:
            esw_path_str = os.path.join(esw_path_str,  i)

        new_file_mappings[key] = esw_path_str

    create_app(new_file_mappings, obj, esw_app_dir)

def openamp_lopper_run(openamp_overlay, original_sdt, app_src_dir):
    """
    Run Lopper upon OpenAMP YAML information if present.
    Args:
        openamp_overlay (str): Path to OpenAMP Overlay information. This will
                               be present in domain.yaml
        original_sdt (str): Path to unpruned system device tree
        app_src_dir (str): Path to application workspace
    Returns:
        None
    """

    # here do lopper openamp stuff...
    if len(openamp_overlay) == 0:
        print('No overlay provided for OpenAMP app. Exiting Build.')
        sys.exit(1)

    lops_dir = os.path.join(utils.get_dir_path(lopper.__file__), "lops")
    lops = ["lop-load.dts", "lop-xlate-yaml.dts", "lop-r5-imux.dts"]
    lopper_cmd = "lopper -f -v  --enhanced  --permissive -i " + openamp_overlay
    for lop in lops:
        lopper_cmd += " -i " + lops_dir + "/" + lop
    # add remote role for this Lopper run after the lop-openamp-versal.dts lop
    lopper_cmd += " -O . " + original_sdt + " openamp_output.dts "
    lopper_cmd += " -- openamp remote "
    utils.runcmd(lopper_cmd)
    if is_file('amd_platform_info.h'):
        utils.copy_file('amd_platform_info.h', os.path.join(app_src_dir, 'apps',
                        'machine', 'zynqmp_r5', 'amd_platform_info.h'))
        utils.remove('amd_platform_info.h')
    else:
        print('OpenAMP Lopper run failed.')
        sys.exit(1)
    return 0
