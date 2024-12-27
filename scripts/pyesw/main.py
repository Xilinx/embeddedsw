# Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

import os
import sys

if os.environ.get("LOCAL_PYESW_SCRIPTS"):
    local_scripts_path = os.environ.get("LOCAL_PYESW_SCRIPTS")
    if not os.path.isdir(local_scripts_path):
        print(f"[WARNING]: LOCAL_PYESW_SCRIPTS variable is set but {local_scripts_path} doesnt exist")
    else:
        sys.path.insert(0, local_scripts_path)
        print(f"[INFO]: LOCAL_PYESW_SCRIPTS variable is set to {local_scripts_path}")

from build_app import main as build_app_main
from build_bsp import main as build_bsp_main
from config_bsp import main as config_bsp_main
from create_app import main as create_app_main
from create_bsp import main as create_bsp_main
from create_example import main as create_example_main
from get_template_data import main as get_template_data_main
from load_example import main as load_example_main
from reconfig_bsp import main as reconfig_bsp_main
from regen_bsp import main as regen_bsp_main
from regen_linker import main as regen_liner_main
from repo import main as repo_main
from retarget_app import main as retarget_app_main
from validate_bsp import main as validate_bsp_main


def info():
    info=f"""\b
pyesw command line tool

    Positional arguments:
        create_app          Create a template App using the BSP path
        create_bsp          Create bsp for the given sdt, os, processor and template app
        build_app           Build the created app.
                            It expects either -w <app ws path> or
                            -s <app src dir path> and -b <app build dir path> passed during create_app
        build_bsp           Build the created bsp
        config_bsp          Modify BSP Settings
        create_example      Create driver or library example using the BSP path
        get_template_data   Fetches all the template app related data from esw yamls and puts it into a yaml file
        load_example        Load the example meta-data for a given domain
        reconfig_bsp        Reconfig the BSP
        regen_bsp           Regenerate the BSP
        regen_linker        Create a template App using the BSP path
        repo                Set ESW Repo Path
        retarget_app        Change platform for a given application
        validate_bsp        Validate the given BSP for a given template

    Optional arguments:
        -h, --help          Show this help message and exit

    Examples:

    create_bsp:
        pyesw create_bsp -w <bsp path> -s <system device-tree system-top.dts path> -p <processor name> -t <template name>

        sample command:
        pyesw create_bsp -t hello_world -s /home/abc/dts/system-top.dts -w bsp_ws -p psu_cortexa53_0

        Output inside test_bsp:
            app_list.yaml  CMakeLists.txt             cpulist.yaml      include  lib_list.yaml  psu_cortexa53_0_baremetal.dts
            bsp.yaml       cortexa53_toolchain.cmake  Findcommon.cmake  lib      libsrc         Xilinx.spec

    build_bsp:
        pyesw build_bsp -d <Directory Path till bsp>

        sample command:
        pyesw build_bsp -d bsp_ws

    create_app:
        pyesw create_app -d <Specify the Directory path till BSP> -t <template app name> -w <app_ws>

        sample command:
        pyesw create_app -d bsp_ws -t lwip_echo_server -w app_ws

        Output inside app_ws/src:
            CMakeLists.txt  i2c_access.c          lscript.ld              Lwip_echo_serverExample.cmake
            memory.ld       platform_config.h.in  README.txt              si5324.c
            echo.c          iic_phyreset.c        lwip_echo_server.cmake  main.c

    build_app:
        pyesw build_app -w <Specify the app workspace>

        sample command:
        pyesw build_app -w app_ws

        Output inside app_ws/build:
        CMakeCache.txt  CMakeFiles            cmake_install.cmake  compile_commands.json
        include         lwip_echo_server.elf  Makefile

Use "pyesw [command] -h" for more information on the available sub commands.

examples to get sub command help:
    pyesw config_bsp -h
    pyesw load_example -h
    pyesw create_example -h
"""
    print(info)

func_map={"create_app" : create_app_main,
    "create_bsp" : create_bsp_main,
    "build_app" : build_app_main,
    "build_bsp" : build_bsp_main,
    "config_bsp" : config_bsp_main,
    "create_example" : create_example_main,
    "get_template_data" : get_template_data_main,
    "load_example" : load_example_main,
    "reconfig_bsp" : reconfig_bsp_main,
    "regen_linker" : regen_liner_main,
    'repo' : repo_main,
    "retarget_app" : retarget_app_main,
    "validate_bsp" : validate_bsp_main,
    "regen_bsp" : regen_bsp_main,
}

def main():
    # Check if a command is provided
    if len(sys.argv) < 2:
        info()
        sys.exit(255)

    # Get the command
    command = sys.argv[1]

    # Route to appropriate module's main function based on command
    if command in ("-h","--help"):
        info()
    elif command in func_map.keys():
        func_map[command](sys.argv[2:])
    else:
        print(f"Unknown command: {command}")
        print('use "pyesw [command] --help" for more information')
        sys.exit(255)

if __name__ == '__main__':
    main()
