# Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
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
    "openamp_echo_test": "echo",
    "openamp_matrix_multiply": "matrix_multiply",
    "openamp_rpc_demo": "rpc_demo",
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
        dstdir (str): destination to copy lib sources to

    Returns:
        None
    """
    # copy specific cmake files
    srcdir = os.path.join(libdir, "src", "sdt")
    top_srcdir = os.path.join(srcdir, "top-CMakeLists.txt")

    top_dstdir = os.path.join(dstdir, "CMakeLists.txt")
    utils.copy_file(top_srcdir, top_dstdir)

    # Add this file to specify cleanup properly of the library
    new_lib_cmake = os.path.join(libdir, "src", "sdt", "lib-CMakeLists.txt")
    utils.copy_file(new_lib_cmake, os.path.join(dstdir, "lib", "CMakeLists.txt"))

    if lib == "openamp":
        new_depends_cmake = os.path.join(libdir, "src", "sdt", "depends.cmake")
        utils.copy_file(
            new_depends_cmake, os.path.join(dstdir, "cmake", "depends.cmake")
        )

    lib_cmakelist = os.path.join(dstdir, "lib")
    lib_cmakelist = os.path.join(lib_cmakelist, "CMakeLists.txt")
    with open(lib_cmakelist, "r", encoding="utf-8") as file:
        content = file.read()
        # Tell CMake build to install headers in BSP include area
        content = content.replace(
            "DESTINATION include RENAME ${PROJECT_NAME}/${f})",
            "DESTINATION ${CMAKE_INCLUDE_PATH}/ RENAME ${PROJECT_NAME}/${f})",
        )
        # Tell CMake build to install library in BSP library area

        # libmetal specific logic. Only picked up in case of libmetal
        orig = "install (TARGETS ${_lib} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})"
        new = "install (TARGETS ${_lib} ARCHIVE DESTINATION ${CMAKE_LIBRARY_PATH})"
        content = content.replace(orig, new)
        # open-amp specific logic. Only picked up in case of open-amp
        orig = 'install (DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/openamp" '
        orig += "DESTINATION include)"
        new = 'install (DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/openamp" '
        new += "DESTINATION ${CMAKE_INCLUDE_PATH}/)"
        content = content.replace(orig, new)
        orig = 'install (DIRECTORY "${PROJECT_BINARY_DIR}/include/generated/openamp" '
        orig += "DESTINATION include)"
        new = 'install (DIRECTORY "${PROJECT_BINARY_DIR}/include/generated/openamp" '
        new += "DESTINATION ${CMAKE_INCLUDE_PATH}/)"
        content = content.replace(orig, new)

    lib_dstdir = os.path.join(dstdir, "lib")
    lib_dstdir = os.path.join(lib_dstdir, "CMakeLists.txt")
    with open(lib_dstdir, "w", encoding="utf-8") as file:
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
        obj.cmake_paths_append += (
            " -DOPENAMP_APP_NAME=" + openamp_app_names[obj.template]
        )
        obj.cmake_paths_append += " -D_AMD_GENERATED_=ON "
        obj.cmake_paths_append += " -DWITH_OPENAMP_FIND=OFF "
        obj.cmake_paths_append += " -DWITH_VENDOR_CMAKE_SCRIPT=ON "

    obj.cmake_paths_append += ' -DPROJECT_VENDOR="xlnx" '
    obj.cmake_paths_append += " -DWITH_DOC=OFF "
    obj.cmake_paths_append += " -DWITH_LIBMETAL_FIND=OFF "

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
    original_src = os.path.join(
        obj.app_src_dir,
        "openamp-system-reference",
        "examples",
        "legacy_apps",
        "CMakeLists.txt",
    )

    utils.replace_line(
        original_src,
        f"project (osr_legacy_apps C)",
        f'set (OPENAMP_APP_NAME "' + open_amp_app_name(obj.template) + '")\n',
    )

    # Specify app name if applicable
    if obj.app_name:
        utils.replace_line(
            os.path.join(obj.app_src_dir, "CMakeLists.txt"),
            f"project(openamp_sys_ref)",
            f"project ({obj.app_name} C)\n",
        )

    # by default these are on. set to OFF by removing them. unfortunately repeat runs will pull in
    # BSP toolchain file so just remove the option set here. There is different workflow
    # for Vitis tooling anyway.
    for i in ["WITH_TESTS", "WITH_EXAMPLES", "tests", "examples"]:
        utils.replace_line(original_src, i, "")


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
    utils.copy_file(
        os.path.join(esw_app_dir, "src", "sdt", "CMakeLists.txt"),
        os.path.join(obj.app_src_dir, "CMakeLists.txt"),
        silent_discard=True,
    )

    openamp_app_configure_common(obj, esw_app_dir)
    utils.replace_line(
        os.path.join(
            obj.app_src_dir,
            "openamp-system-reference",
            "examples",
            "libmetal",
            "CMakeLists.txt",
        ),
        "project (libmetal_apps C)",
        "",
    )

    # Specify app name if applicable
    if obj.app_name:
        utils.replace_line(
            os.path.join(obj.app_src_dir, "CMakeLists.txt"),
            f"project (libmetal_amp_demod C)",
            f"project ({obj.app_name} C)",
        )

        new_str = "include(${CMAKE_SOURCE_DIR}/UserConfig.cmake)\n"
        new_str += "set (_elf_name ${CMAKE_PROJECT_NAME})\n"
        utils.replace_line(
            os.path.join(
                obj.app_src_dir,
                "openamp-system-reference",
                "examples",
                "libmetal",
                "machine",
                "remote",
                "amd_rpu",
                "CMakeLists.txt",
            ),
            "set (_elf_name ${DEMO})",
            new_str,
        )


def openamp_lopper_run(bsp_sdt, linker_cmd, obj, esw_app_dir, soc):
    """
    Run Lopper upon OpenAMP YAML information if present.
    Args:
        bsp_sdt (str): Path to unpruned system device tree
        linker_cmd (str): Current lopper linker command
        obj (App): App object that contains Takes all the user inputs in a dictionary.
        esw_app_dir (str): Path to application source area
        soc (str): SOC Family string in lower case
    Returns:
        None
    """
    # YAML repository location has different naming convention
    # return original soc if key not found
    soc = {"versalnet": "versal-net", "versal_2ve_2vm": "versal-2ve-2vm"}.get(soc, soc)

    md_base_yaml = f"{soc}-multidomain-base.yaml"

    base_dir = os.path.join(obj.app_src_dir, "openamp-system-reference")
    examples_dir = os.path.join(base_dir, "examples")
    output_sdt = os.path.join(base_dir, "openamp_output.dts")

    if obj.template == "libmetal_echo_demo":
        domain_yaml = f"{soc}-libmetal-overlay.yaml"
        fname = "rpu.cmake"
        header_location = os.path.join(examples_dir, "libmetal", "machine", "remote", "amd_rpu")
        cmd2_substr = " --libmetal_output_file --compatible-string=libmetal,ipc-v1 "
        cmd2_substr += f" --processor={obj.proc} --openamp_output_filename={fname} --os=baremetal_dt "
    else:
        domain_yaml = f"{soc}-openamp-overlay.yaml"
        fname = "amd_platform_info.h"
        header_location = os.path.join(examples_dir, "legacy_apps", "machine", "xlnx", "zynqmp_r5")
        cmd2_substr = f" --openamp_header_only --openamp_output_filename={fname} --openamp_remote={obj.proc} "

    cmd1 = f"lopper -f --enhanced --auto -i {md_base_yaml} -i {domain_yaml} -O {obj.app_src_dir} {bsp_sdt} {output_sdt}"
    cmd2 = f"lopper -f --enhanced -O {obj.app_src_dir} {output_sdt} -- openamp {cmd2_substr}"

    hw_dir = os.path.join(obj.domain_path, "hw_artifacts")
    yaml_dir = os.path.join(os.environ.get("XILINX_VITIS"), "data", "openamp-metadata")

    for yaml_file in [ domain_yaml, md_base_yaml ]:
        dst = os.path.join(hw_dir, yaml_file)
        if os.path.exists(dst):
            logger.info(f"{yaml_file} already exists. Remove it to use the default copy.")
        else:
            utils.copy_file(os.path.join(yaml_dir, yaml_file), dst)

        # insert correct path for the YAML files to be picked up from
        cmd1 = cmd1.replace(yaml_file, dst)

    utils.runcmd(cmd1, log_message="OpenAMP Lopper Run 1 - create DT")
    utils.runcmd(cmd2, log_message="OpenAMP Lopper Run 2 - generate output")

    if not utils.is_file(fname):
        logger.error("OpenAMP Lopper run failed.")
        sys.exit(1)

    utils.copy_file(fname, os.path.join(header_location, fname))
    utils.remove(fname)


    # Change out SDT to use OpenAMP SDT here
    # Add flag at the end to denote this is for OpenAMP case
    linker_cmd = linker_cmd.replace(obj.sdt, output_sdt) + " openamp "
    return linker_cmd
