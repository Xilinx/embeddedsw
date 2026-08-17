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

        new_str = (
            "# DO NOT EDIT this file for any customized settings.\n"
            "# User-configurable settings are in UserConfig.cmake.\n"
            "include(${CMAKE_SOURCE_DIR}/UserConfig.cmake)\n"
            "set (_elf_name ${CMAKE_PROJECT_NAME})\n"
        )
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
    # Only SoC names that differ from the YAML filename prefix need remapping.
    # Versal and ZynqMP already match and fall through unchanged.
    soc = {"versalnet": "versal-net", "versal_2ve_2vm": "versal-2ve-2vm"}.get(soc, soc)

    # Build the OpenAMP example paths used by the generated outputs.
    base_dir = os.path.join(obj.app_src_dir, "openamp-system-reference")
    examples_dir = os.path.join(base_dir, "examples")
    output_sdt = os.path.join(base_dir, "openamp_output.dts")

    # Pick the second lopper stage output and destination based on the app type.
    if obj.template == "libmetal_echo_demo":
        fname = "rpu.cmake"
        header_location = os.path.join(examples_dir, "libmetal", "machine", "remote", "amd_rpu")
        cmd2_substr = " --libmetal_output_file --compatible-string=libmetal,ipc-v1 "
        cmd2_substr += f" --processor={obj.proc} --openamp_output_filename={fname} --os=baremetal_dt "
    else:
        fname = "amd_platform_info.h"
        header_location = os.path.join(examples_dir, "legacy_apps", "machine", "xlnx", "zynqmp_r5")
        cmd2_substr = f" --openamp_header_only --openamp_output_filename={fname} --openamp_remote={obj.proc} "

    hw_dir = os.path.join(obj.domain_path, "hw_artifacts")
    # Pick up default domain YAMLs from the same ESW repo copy that supplied the template app.
    yaml_dir = os.path.join(os.path.dirname(esw_app_dir), "openamp_sdt")

    # Collect all YAMLs from the BSP hw_artifacts directory and use them as run-1 lopper inputs.
    if os.path.isdir(hw_dir):
        yaml_files = sorted(
            os.path.join(hw_dir, entry)
            for entry in os.listdir(hw_dir)
            if os.path.isfile(os.path.join(hw_dir, entry))
            and entry.lower().endswith((".yaml", ".yml"))
        )
    else:
        yaml_files = []

    # The normalized SoC name matches the default-domain YAML filename prefix.
    default_yaml = f"{soc}-default-domain.yaml"
    default_yaml_dst = os.path.join(hw_dir, default_yaml)

    # Prefer user-provided BSP YAMLs and only add the repo default when none are present.
    if yaml_files:
        present_yamls = ", ".join(os.path.basename(yaml_file) for yaml_file in yaml_files)
        logger.info(
            f"Using existing Domain YAML input(s) from {hw_dir}: {present_yamls}. "
            "Default domain YAML will not be added."
        )
    elif utils.is_file(os.path.join(yaml_dir, default_yaml)):
        utils.copy_file(os.path.join(yaml_dir, default_yaml), default_yaml_dst)
        yaml_files = [default_yaml_dst]
        logger.info(f"No OpenAMP YAMLs found in {hw_dir}. Added default YAML: {default_yaml}.")
    else:
        logger.error(
            f"No default OpenAMP domain YAML was found for SoC '{soc}' in {yaml_dir}."
        )
        sys.exit(1)

    # Run lopper stage 1 with the YAML inputs from BSP hw_artifacts, then generate the final output artifact.
    yaml_args = " ".join(f"-i {yaml_file}" for yaml_file in yaml_files)
    cmd1 = f"lopper -f --enhanced --auto {yaml_args} -O {obj.app_src_dir} {bsp_sdt} {output_sdt}"
    cmd2 = f"lopper -f --enhanced -O {obj.app_src_dir} {output_sdt} -- openamp {cmd2_substr}"

    utils.runcmd(cmd1, log_message="OpenAMP Lopper Run 1 - create DT")

    if not utils.is_file(output_sdt):
        logger.error(f"OpenAMP Lopper run 1 did not generate output SDT: {output_sdt}")
        sys.exit(1)

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
