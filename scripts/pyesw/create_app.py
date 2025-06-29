# Copyright (C) 2023 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
"""
This module creates the template application using the domain information
provided to it. It generates the directory structure and the metadata
required to build a particular template application.
"""

import argparse
import os
import sys
import textwrap

import utils
from build_bsp import BSP
from open_amp import (create_libmetal_app, create_openamp_app,
                      openamp_lopper_run, openamp_app_names)
from repo import Repo
from utils import log_time
from validate_bsp import Validation
from validate_hw import ValidateHW

logger = utils.get_logger(__name__)

class App(BSP, Repo):
    """
    This class helps in creating a template application. It contains attributes
    and functions that take domain path, and template name as an input, create
    the directory structure and the metadata(if needed) for the app.
    """

    def __init__(self, args):
        BSP.__init__(self, args)
        Repo.__init__(self, repo_yaml_path=args['repo_info'])
        self._build_dir_struct(args)
        self.app_name = args.get("name")
        self.template = args.get("template")
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

        # Validate the passed template w.r.t the passed domain path
        if os.environ.get("VALIDATE_ARGS") == "True":
            obj = Validation(args)
            obj.validate_template_for_bsp()
        utils.mkdir(self.app_src_dir)
        # App directory needs to have its own yaml configuration
        # (for compiler flags, linker flags etc.)
        self.app_config_file = os.path.join(self.app_src_dir, "app.yaml")

@log_time
def create_app(args):
    """
    Function that uses the above App class to create the template application.

    Args:
        args (dict): Takes all the user inputs in a dictionary.
    """
    obj = App(args)

    if os.environ.get("VALIDATE_ARGS") == "True":
        logger.info("Validating inputs")
        validate_obj = ValidateHW(
            obj.domain_path, obj.proc, obj.os, obj.sdt,
            obj.template, obj.repo_yaml_path
        )
        validate_obj.validate_hw()

    domain_data = utils.fetch_yaml_data(obj.domain_config_file, "domain")
    esw_app_dir = obj.get_comp_dir(obj.template)
    srcdir = os.path.join(esw_app_dir, "src")
    if obj.template in openamp_app_names.keys():
        srcdir = os.path.join(os.environ.get('XILINX_VITIS'), 'data')
        srcdir = os.path.join(srcdir, 'openamp-system-reference')
    elif obj.template == 'libmetal_echo_demo':
        srcdir = os.path.join(os.environ.get('XILINX_VITIS'), 'data')
        srcdir = os.path.join(srcdir, 'libmetal')

    utils.copy_directory(srcdir, obj.app_src_dir)

    if obj.app_name:
        src_cmake = os.path.join(obj.app_src_dir, "CMakeLists.txt")
        utils.replace_line(
            src_cmake,
            f'APP_NAME {obj.template}',
            f'set(APP_NAME {obj.app_name})',
        )

    # in case of library update link libraries
    if domain_data['lib_info']:
        src_cmake = os.path.join(obj.app_src_dir, "CMakeLists.txt")
        lib_list = list(domain_data['lib_info'].keys())
        # Special handling for libmetal
        lib_list = [lib.replace('libmetal', 'metal') for lib in lib_list]
        # Special handling for openamp
        lib_list = [lib.replace('openamp', 'open_amp') for lib in lib_list]
        # FixME: Link the math library by default for libmetal dependent drivers
        if 'metal' in lib_list:
            lib_list.append('m')
        if obj.os == "freertos":
            lib_list.append(obj.os)
        cpu_list_file = os.path.join(obj.domain_path, "cpulist.yaml")
        avail_cpu_data = utils.fetch_yaml_data(cpu_list_file, "cpulist")
        proc_ip_name = avail_cpu_data[obj.proc]
        if "microblaze_riscv" in proc_ip_name or "microblaze" in proc_ip_name:
            lib_list.append("gloss")
        cmake_lib_list = ';'.join(lib_list)
        utils.replace_line(
            src_cmake,
            f'PROJECT_LIB_DEPS xilstandalone',
            f'collect(PROJECT_LIB_DEPS xilstandalone;{cmake_lib_list})',
        )

    # Checks if the app depends on any driver, if yest, generates the corresponding metadata
    app_yaml_file = os.path.join(esw_app_dir, "data", f"{obj.template}.yaml")
    if utils.is_file(app_yaml_file):
        app_schema = utils.load_yaml(app_yaml_file)
        if app_schema.get("depends"):
            utils.runcmd(
                f"lopper -O {obj.app_src_dir} {obj.sdt} -- bmcmake_metadata_xlnx {obj.proc} {srcdir} hwcmake_metadata {obj.repo_yaml_path}",
                log_message = f"Generating Hardware Metadata for {obj.template}",
                error_message = f"Failed to generate Hardware Meta-data for {obj.template}",
                verbose_level = 0
            )

    # Generates the metadata for linker script
    linker_cmd = (
        f"lopper -O {obj.app_src_dir} {obj.sdt} -- baremetallinker_xlnx {obj.proc} {srcdir}"
    )
    bsp_obj = BSP(args)
    overlay_path = os.path.join(bsp_obj.domain_path, 'hw_artifacts', 'domain.yaml')

    if obj.template in openamp_app_names.keys():
        bsp_obj = BSP(args)
        original_sdt = os.path.join(bsp_obj.domain_path, 'hw_artifacts', 'sdt.dts')

        # Note that lopper command to generate linker script will be updated
        # here to use OpenAMP SDT. To generate OpenAMP SDT, need original SDT
        linker_cmd = openamp_lopper_run(original_sdt, linker_cmd, obj, esw_app_dir)

    if obj.template == "memory_tests":
        utils.runcmd(
            f"{linker_cmd} memtest",
            log_message = "Generating Linker Script for memory_tests",
            error_message = "Failed to generate Linker Script for memory_tests",
            verbose_level = 0
        )

    elif obj.template != "versal_plm":
        utils.runcmd(
            linker_cmd,
            log_message = f"Generating Linker script for {obj.template}",
            error_message = f"Failed to generate Linker script for {obj.template}",
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

    # Generate the CMake file specifically for peripheral app
    if obj.template == "peripheral_tests":
        if domain_data['os_config'][obj.os]:
            stdin = domain_data['os_config'][obj.os][f'{obj.os}_stdin']['value']
        utils.runcmd(
            f"lopper -O {obj.app_src_dir} {obj.sdt} -- baremetal_gentestapp_xlnx {obj.proc} {obj.repo_yaml_path} {stdin}",
            log_message = "Generating meta-data for peripheral tests",
            error_message = "Peripheral test meta-data generation failed.",
            verbose_level = 0
        )

    # Copy psu_init* files for zynq and zynqmp fsbl app
    if "fsbl" in obj.template:
        if obj.template == "zynqmp_fsbl":
            init_file = "psu_init"
        elif obj.template == "zynq_fsbl":
            init_file = "ps7_init"
        else:
            logger.error(
                "Not a valid FSBL app entry. Expecting either zynqmp_fsbl or zynq_fsbl as template"
            )
            sys.exit(1)
        init_c = os.path.join(obj.domain_path, "hw_artifacts", f"{init_file}.c")
        init_h = os.path.join(obj.domain_path, "hw_artifacts", f"{init_file}.h")
        utils.copy_file(init_c, obj.app_src_dir, silent_discard=True)
        utils.copy_file(init_h, obj.app_src_dir, silent_discard=True)

    if obj.template in openamp_app_names.keys():
        create_openamp_app(obj, esw_app_dir)
    elif obj.template == 'libmetal_echo_demo':
        create_libmetal_app(obj, esw_app_dir)

    # Add domain path entry in the app configuration file.
    data = {"domain_path": obj.domain_path,
            "app_src_dir": esw_app_dir,
            "template": obj.template,
            "lang": args["lang"]
        }
    utils.write_yaml(obj.app_config_file, data)

    if not args["no_clangd"]:
        # Create a dummy folder to get compile_commands.json
        compile_commands_dir = os.path.join(obj.app_src_dir, ".compile_commands")
        utils.mkdir(compile_commands_dir)
        obj.app_src_dir = obj.app_src_dir.replace('\\', '/')
        obj.cmake_paths_append = obj.cmake_paths_append.replace('\\', '/')
        dump = utils.discard_dump()
        utils.runcmd(
            f'cmake -G "{obj.cmake_generator}" {obj.app_src_dir} {obj.cmake_paths_append} > {dump}',
            cwd = compile_commands_dir,
            log_message = "Dummy cmake call for compile_commands.json",
            error_message = "Failed to generate compile_commands.json"
        )

        '''
        compile_commands.json file needs to be kept inside src directory.
        Silent_discard needs to be true as for Empty Application, this file
        is not created.
        '''
        utils.copy_file(os.path.join(compile_commands_dir, "compile_commands.json"), obj.app_src_dir, silent_discard=True)

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
        logger.info(f"Successfully Created Application sources at {obj.app_src_dir}")

def main(arguments=None):
    parser = argparse.ArgumentParser(
        description="Use this script to create a template App using the BSP path",
        usage='use "empyro create_app --help" for more information',
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
    parser.add_argument(
        "-n", "--name", action="store", help="App name"
    )
    required_argument.add_argument(
        "-t",
        "--template",
        action="store",
        required=True,
        help=textwrap.dedent(
            """\
        Specify template app name. Available names are:
            - empty_application
            - hello_world
            - memory_tests
            - peripheral_tests
            - zynqmp_fsbl
            - zynqmp_pmufw
            - lwip_echo_server
            - freertos_hello_world
            - versal_plm
            - spartanup_plm
            - versal_psmfw
            - freertos_lwip_echo_server
            - freertos_lwip_tcp_perf_client
            - freertos_lwip_tcp_perf_server
            - freertos_lwip_udp_perf_client
            - freertos_lwip_udp_perf_server
            - lwip_tcp_perf_client
            - lwip_tcp_perf_server
            - lwip_udp_perf_client
            - lwip_udp_perf_server
            - dhrystone
            - zynqmp_dram_test
        """
        ),
    )
    parser.add_argument(
        "-r",
        "--repo_info",
        action="store",
        help="Specify the .repo.yaml absolute path to use the set repo info",
        default='.repo.yaml',
    )
    parser.add_argument(
        "-v",
        "--verbose",
        action="count",
        default=0,
        help='Increase output verbosity'
    )
    parser.add_argument(
        "--no_clangd",
        action="store",
        default=False,
        help="Don't generate clangd meta-data useful for Vivado PLM kind of use cases (Default: False)",
        choices=["False", "True"],
    )
    parser.add_argument(
        "-l",
        "--lang",
        action="store",
        default="c",
        help="Specify the language (Default: c) this option is applicable only for \
              hello_world and empty_application templates",
        choices=["c", "c++"],
    )
    args = vars(parser.parse_args(arguments))
    utils.setup_log(args["verbose"])
    create_app(args)

if __name__ == "__main__":
    main()