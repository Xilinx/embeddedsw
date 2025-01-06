# Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

import os
import sys

if os.environ.get("LOCAL_EMPYRO_SCRIPTS"):
    local_scripts_path = os.environ.get("LOCAL_EMPYRO_SCRIPTS")
    sys.path.insert(0, local_scripts_path)
    print(f"[INFO]: LOCAL_EMPYRO_SCRIPTS variable is set to {local_scripts_path}")

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
from utils import is_file

def info():
    info=f"""\bUsage: empyro [COMMAND] [OPTIONS]...
Create, Configure, Organize and Build the BSP and the Applications
targeted for AMD-Xilinx SOCs and FPGAs

List of COMMANDS

  create_app          Create a template application for the given BSP
  create_bsp          Create BSP for the given sdt, os, processor and template
                      application
  build_app           Build the given application.
                      It expects either -w <app ws path> or
                      --src_dir <app src dir path> and --build_dir <app build
                      dir path> passed during create_app
  build_bsp           Build the BSP
  config_bsp          Configure the BSP settings
  create_example      Create driver or library example using the BSP path
  get_template_data   Fetch all the template application related data from
                      EmbeddedSW yamls and put it into a yaml file
  load_example        Load example meta-data for given BSP
  reconfig_bsp        Reconfigure the BSP
  regen_bsp           Regenerate the BSP
  regen_linker        Regenerate the linker script for given application
  repo                Set EmbeddedSW repository path
  retarget_app        Re-target the application to a different BSP
  validate_bsp        Validate given BSP against a template application

COMMAND without OPTIONS
  -h, --help          Show this help message and exit
  --version           Show the binary version along with commit details if
                      available

Examples:

Create BSP and hello world application using:

  # Set EmbeddedSW repository path
  empyro repo -st ./embeddedsw

  # Create BSP for hello world application targeting APU_0 of a ZynqMP Soc
  empyro create_bsp -w hello_world_bsp -s /home/abc/dts/system-top.dts
  -p psu_cortexa53_0 -t hello_world

  # Create the hello world application template, link it to the created BSP
  empyro create_app -d hello_world_bsp -t hello_world -w hello_world_app

  # Build BSP and hello world application
  empyro build_app -w hello_world_app

Use "empyro [COMMAND] -h" for [OPTIONS] available with [COMMAND].
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
    elif command in ("--version"):
        version_file_path=os.path.join(os.path.dirname(os.path.realpath(__file__)), 'VERSION')
        if (is_file(version_file_path)):
            with open(version_file_path, 'r') as f:
                version = f.read().strip()
        else:
            version="1.0"
        print(version)
    elif command in func_map.keys():
        func_map[command](sys.argv[2:])
    else:
        print(f"Unknown command: {command}")
        info()
        sys.exit(255)

if __name__ == '__main__':
    main()
