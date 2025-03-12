# Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
"""
This module enables utilities for OpenAMP and Libmetal. This includes
enabling these libraries in BSPs, application configure and application
build steps.
"""

import os
import re
import sys

import utils

logger = utils.get_logger(__name__)

openamp_app_names = {
    'openamp_echo_test': 'echo',
    'openamp_matrix_multiply': 'matrix_multiply',
    'openamp_rpc_demo': 'rpc_demo'
}

def open_amp_app_name(app_name):
    if app_name not in openamp_app_names.keys():
        logger.error(f"{app_name} not found in list of openamp apps")
        sys.exit(1)

    return openamp_app_names[app_name]

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

def openamp_app_configure_common(obj, esw_app_dir):
    """
    Template that can be used to copy toolchain file and ensure it is present
    in CMake configure step
    Args:
        obj (App): App object that contains Takes all the user inputs in a dictionary.
        esw_app_dir (str): location to copy source in repo from
    Returns:
        None
    """
    if obj.template in openamp_app_names.keys():
        obj.cmake_paths_append += ' -DOPENAMP_APP_NAME=' + openamp_app_names[obj.template]
        obj.cmake_paths_append += " -D_AMD_GENERATED_=ON "

    obj.cmake_paths_append += " -DPROJECT_VENDOR=\"xlnx\" "
    obj.cmake_paths_append += " -DWITH_DOC=OFF "

    if obj.os == "freertos":
        obj.cmake_paths_append += " -DUSE_FREERTOS=ON"

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

    openamp_app_configure_common(obj, esw_app_dir)

    # Point to correct demo to build
    utils.replace_line(os.path.join(obj.app_src_dir, "examples", "legacy_apps", "CMakeLists.txt"),
                       f'project (osr_legacy_apps C)',
                       f'set (OPENAMP_APP_NAME \"' + open_amp_app_name(obj.template) + '\")\n')

    # Specify app name if applicable
    if obj.app_name:
        utils.replace_line(os.path.join(obj.app_src_dir, "CMakeLists.txt"),
                           f'project(openamp_sys_ref)', f'project ({obj.app_name} C)\n')

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
    utils.copy_file(os.path.join(esw_app_dir, 'src', 'sdt', 'top-CMakeLists.txt'),
                    os.path.join(obj.app_src_dir, 'CMakeLists.txt'),
                    silent_discard=True)

    openamp_app_configure_common(obj, esw_app_dir)

    # Specify app name if applicable
    if obj.app_name:
        utils.replace_line(os.path.join(obj.app_src_dir, "CMakeLists.txt"),
                           f'project (libmetal_amp_demod C)', f'project ({obj.app_name} C)')

def openamp_lopper_run(bsp_sdt, linker_cmd, obj, esw_app_dir):
    """
    Run Lopper upon OpenAMP YAML information if present.
    Args:
        bsp_sdt (str): Path to unpruned system device tree
        linker_cmd (str): Current lopper linker command
        obj (App): App object that contains Takes all the user inputs in a dictionary.
        esw_app_dir (str): Path to application source area
    Returns:
        None
    """
    openamp_lopper_host_remote_tuples = {
        'cortexr52_0' : ('a78_0', 'r52_0', 'VersalGen2'),
        'cortexr52_1' : ('a78_0', 'r52_1', 'VersalGen2'),
        'cortexr52_2' : ('a78_0', 'r52_2', 'VersalGen2'),
        'cortexr52_3' : ('a78_0', 'r52_3', 'VersalGen2'),
        'cortexr52_4' : ('a78_0', 'r52_4', 'VersalGen2'),
        'cortexr52_5' : ('a78_0', 'r52_5', 'VersalGen2'),
        'cortexr52_6' : ('a78_0', 'r52_6', 'VersalGen2'),
        'cortexr52_7' : ('a78_0', 'r52_7', 'VersalGen2'),
        'cortexr52_8' : ('a78_0', 'r52_8', 'VersalGen2'),
        'cortexr52_9' : ('a78_0', 'r52_9', 'VersalGen2'),
        'psx_cortexr52_0' : ('a78_0', 'r52_0', 'versal-net'),
        'psx_cortexr52_1' : ('a78_0', 'r52_1', 'versal-net'),
        'psx_cortexr52_2' : ('a78_0', 'r52_2', 'versal-net'),
        'psx_cortexr52_3' : ('a78_0', 'r52_3', 'versal-net'),
        'psv_cortexr5_0'  : ('a72_0', 'r5_0', 'versal'),
        'psv_cortexr5_1'  : ('a72_0', 'r5_1', 'versal'),
        'psu_cortexr5_0'  : ('a53_0', 'r5_0', 'zynqmp'),
        'psu_cortexr5_1'  : ('a53_0', 'r5_1', 'zynqmp'),
    }

    (host, remote, soc) = openamp_lopper_host_remote_tuples[obj.proc]
    overlay_dst = os.path.join(obj.domain_path, "hw_artifacts", "domain.yaml")
    output_sdt = os.path.join(obj.app_src_dir, "openamp_output.dts")

    utils.copy_file(os.path.join(os.environ.get('XILINX_VITIS'), 'data', 'openamp-metadata', f"openamp-overlay-{soc}.yaml"),
                    overlay_dst)

    cmd =  f"lopper -f -v --enhanced --permissive -i lop-xlate-yaml.dts "
    cmd += f"-i {overlay_dst} -O {obj.app_src_dir} {bsp_sdt} {output_sdt} "
    cmd += f"-- openamp --openamp_role=remote --openamp_host={host} --openamp_remote={remote}"
    utils.runcmd(cmd, log_message="OpenAMP Lopper Run")

    if utils.is_file('amd_platform_info.h'):
        utils.copy_file('amd_platform_info.h', os.path.join(obj.app_src_dir,
                        'examples', 'legacy_apps', 'machine', 'zynqmp_r5', 'amd_platform_info.h'))
        utils.remove('amd_platform_info.h')

        # Change out SDT to use OpenAMP SDT here
        linker_cmd = linker_cmd.replace(obj.sdt, output_sdt)

        # Add flag at the end to denote this is for OpenAMP case
        linker_cmd += " openamp "

        return linker_cmd
    else:
        logger.error('OpenAMP Lopper run failed.')
        sys.exit(1)
    return 0


