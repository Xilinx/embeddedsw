# Copyright (C) 2023 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
"""
This module configures the bsp as per the passed library and os related
parameters.
"""

import argparse
import os
import sys
import utils
from build_bsp import BSP
from library_utils import Library
from regen_bsp import regenerate_bsp
from validate_hw import ValidateHW
from create_bsp import lop_create_target

logger = utils.get_logger(__name__)

class Bsp_config(BSP, Library):
    """
    This class contains attributes and functions that help in configuring the
    created bsp. This makes use of BSP and Library class attirbutes and
    functions to fetch the bsp confiration data and the supporting lib funcs.
    """

    def __init__(self, args):
        BSP.__init__(self, args)
        Library.__init__(
            self,
            self.domain_path,
            self.proc,
            self.os,
            self.sdt,
            self.cmake_paths_append,
            self.libsrc_folder,
            args['repo_info']
        )
        self.addlib = args["addlib"]
        self.rmlib = args["rmlib"]

def configure_bsp(args):
    """
    This function uses Bsp_config class and configures the bsp based on the
    user input arguments.

    Args:
        args (dict): User inputs in a dictionary format
    """
    obj = Bsp_config(args)
    # If user wants to add a library to the bsp
    if obj.addlib:
        lib_name, lib_path = obj.addlib[0], None
        logger.info(f"Adding {lib_name} ...")
        #check for the custom repo config
        if ":" in  obj.addlib[0]:
            lib_name, lib_path = obj.addlib[0].split(":")
            #Validate the library path
            if not utils.is_dir(os.path.join(lib_path,"src")):
                logger.error(f"""
                Wrong inputs passed while adding the library.
                {os.path.join(lib_path,"src")} doesnt exist..
                Please pass the library name and path in
                the below format
                    -al <lib_name>:<path to library>
                e.g. empyro config_bsp -d bsp -al libmetal:/home/abc/libmetal
                """)
                sys.exit(1)
        if lib_name in obj.bsp_lib_config.keys():
            logger.warning(f"""\b
                 {lib_name} is already added in the bsp. Nothing to do.
                 Use config_bsp.py if you want to configure the library.
            """)
            sys.exit(1)
        obj.gen_lib_list()
        obj.is_valid_lib(lib_name,silent_discard=False)
        lib_list = obj.get_depends_libs(lib_name, lib_list=[lib_name])
        validate_obj = ValidateHW(
            obj.domain_path, obj.proc, obj.os, obj.sdt,
            lib_name, obj.repo_yaml_path
        )
        validate_obj.validate_hw()
        # Remove duplicate libs
        lib_list = list(dict.fromkeys(lib_list))
        for lib in lib_list:
            if lib in obj.lib_list:
                continue
            obj.validate_lib_name(lib)
            obj.gen_lib_metadata(lib,lib_path)

        obj.config_lib(lib_name, lib_list, "", is_app=False)
        logger.info(f"Successfully added {lib_name}")

    # If user wants to remove a library from the bsp
    if obj.rmlib:
        if obj.rmlib == obj.os:
            logger.warning(f"Removal of {obj.os} library is not allowed.")
            sys.exit(1)

        logger.info(f"Removing {obj.rmlib} ...")
        obj.remove_lib(obj.rmlib)
        logger.info(f"Successfully removed {obj.rmlib}")


    # If user wants to set library parameters
    if args.get("set_property"):
        prop_params = args["set_property"]

        # Validate the command line inputs provided
        usage_print = """
            Please pass library name followed by param:value.
            e.g. -st xilffs XILFFS_read_only:ON XILFFS_use_lfn:1
            Wrong inputs passed with set_property.
        """
        if len(prop_params) < 2:
            logger.error(usage_print)
            sys.exit(1)
        lib_name = prop_params[0]
        # assert error if library is not added in bsp
        obj.validate_lib_in_bsp(lib_name)

        prop_dict = {lib_name: {}}
        for entries in prop_params[1:]:
            if ":" not in entries:
                logger.error(usage_print)
                sys.exit(1)
            else:
                prop_data = entries.split(":")
                prop_dict[lib_name].update({prop_data[0]: prop_data[1]})
                obj.validate_lib_param(lib_name, prop_data[0])
                # Set the passed value in lib config dictionary
            param = prop_data[0]
            value = prop_data[1]

            # Fetch options from bsp.yaml structure
            valid_list = obj.bsp_lib_config[lib_name][param].get("options", [])

            if valid_list and value not in valid_list:
                logger.error(f"Invalid value '{value}' for parameter '{param}',\nPossible values: {', '.join(valid_list)}")
                sys.exit(1)  # Exit the program with status code 1

            # Update the value regardless
            obj.bsp_lib_config[lib_name][param]["value"] = value

        if ('xilpm' == lib_name) and ("ZynqMP" in obj.domain_data['family']):
            dstdir = os.path.join(obj.libsrc_folder, lib_name, "src", "zynqmp", "client", "common")
            ori_sdt_path = os.path.join(obj.domain_path, "hw_artifacts", "sdt.dts")
            lopper_cmd_append = ""
            for key, values in obj.bsp_lib_config[lib_name].items():
                value = values['value']
                lopper_cmd_append += f' {key}:{value}'
            lopper_cmd = f"lopper -O {dstdir} -f {ori_sdt_path} --  generate_config_object pm_cfg_obj.c {obj.proc} {lopper_cmd_append}"
            utils.runcmd(
                lopper_cmd, cwd = dstdir,
                log_message = "Generating PM Config Object",
                error_message = "PM Config object generation Failed",
                verbose_level = 0
            )

        # set the cmake options to append lib param values.
        cmake_cmd_append = ""
        for key, value in prop_dict[lib_name].items():
            logger.debug(f"Setting {key} to {value}")
            cmake_cmd_append += f' -D{key}="{value}"'
            if key == "standalone_hypervisor_guest" or key == "freertos_hypervisor_guest":
                xen_config_lops_file = os.path.join(obj.domain_path, "hw_artifacts", "lop-xen-config.dts")
                drv_path = obj.get_comp_dir("scugic")
                drv_srcdir = os.path.join(drv_path, "src")
                drvsrc = os.path.join(obj.libsrc_folder, "scugic", "src")
                if utils.is_dir(drvsrc):
                    if not utils.is_file(xen_config_lops_file):
                        lop_cmds = []
                        lop_cmds.append([obj.include_folder, "module,baremetal_xparameters_xlnx", f"{obj.proc} {obj.repo_yaml_path}"])
                        lop_cmds.append([drvsrc, "module,baremetalconfig_xlnx", f"{obj.proc} {drv_srcdir}"])
                        if obj.os == "freertos":
                            lib = "freertos10_xilinx"
                            lib_dir_path = obj.get_comp_dir(lib)
                            srcdir = os.path.join(lib_dir_path, "src")
                            dstdir = os.path.join(obj.libsrc_folder, f"{lib}/src")
                            outfile = f"{lib}Example.cmake"
                            lop_cmds.append([dstdir, "module,bmcmake_metadata_xlnx", f"{obj.proc} {srcdir} hwcmake_metadata {obj.repo_yaml_path}"])
                        utils.write_into_file(xen_config_lops_file, lop_create_target(lop_cmds))
                    if value == "true":
                        utils.runcmd(
                            f"lopper -O {obj.domain_path} -i lop-gic-el1.dts -i {xen_config_lops_file} -f {obj.sdt}",
                            log_message = "Generating required metadata EL1 NS use case",
                            error_message = "Overall metadata generation failed",
                            verbose_level = 0
                        )
                    elif value == "false":
                        utils.runcmd(
                            f"lopper -O {obj.domain_path} -i {xen_config_lops_file} -f {obj.sdt}",
                            log_message = "Generating required metadata for EL1 NS use case",
                            error_message = "Overall metadata generation failed",
                            verbose_level = 0
                        )

        # configure the lib build area with new params
        build_metadata = os.path.join(obj.libsrc_folder, "build_configs/gen_bsp")
        if obj.os == "freertos" and lib_name == "freertos":
            utils.runcmd(
                f'cmake {obj.domain_path} {obj.cmake_paths_append} -DSUBDIR_LIST="freertos10_xilinx" {cmake_cmd_append}',
                cwd = build_metadata,
                log_message = f"Configuring FreeRTOS with {cmake_cmd_append}",
                error_message="Failed to configure FreeRTOS  ",
                verbose_level = 0
            )
        else:
            utils.runcmd(
                f'cmake {obj.domain_path} {obj.cmake_paths_append} -DSUBDIR_LIST="{lib_name}" {cmake_cmd_append}',
                cwd = build_metadata,
                log_message = f"Configuring {lib_name} with {cmake_cmd_append}",
                error_message = f"Failed to configure {lib_name}",
                verbose_level = 0
            )

        # Update the lib config file
        if obj.proc in obj.bsp_lib_config.keys():
            proc_config = obj.bsp_lib_config.pop(obj.proc)
            proc_config = {obj.proc:proc_config}
            utils.update_yaml(obj.domain_config_file, "domain", "proc_config", proc_config, action="add")
        if obj.os in obj.bsp_lib_config.keys():
            os_config = obj.bsp_lib_config.pop(obj.os)
            os_config = {obj.os:os_config}
            utils.update_yaml(obj.domain_config_file, "domain", "os_config", os_config, action="add")
        utils.update_yaml(obj.domain_config_file, "domain", "lib_config", obj.bsp_lib_config)
        utils.update_yaml(obj.domain_config_file, "domain", "config", "reconfig")
        logger.info(f"Successfully configured BSP")

    if args.get("set_repo_path"):
        prop_params = args["set_repo_path"]

        # Validate the command line inputs provided
        usage_print = """
            Please pass component and component name followed by path
            component name:path.
            e.g. -sr driver csudma:<path to driver source>
            Wrong inputs passed with set_repo_path.
        """
        if len(prop_params) < 2:
            logger.error(usage_print)
            sys.exit(1)

        # Copy the repo.yaml to the bsp path
        repo_yaml = os.path.join(obj.domain_path, '.repo.yaml')
        if not utils.is_file(repo_yaml):
            utils.copy_file(args['repo_info'], obj.domain_path)

        component = prop_params[0]
        for entries in prop_params[1:]:
            if ":" not in entries:
                logger.error(usage_print)
                sys.exit(1)
            else:
                prop_data = entries.split(":")
                component_name = prop_data[0]
                if os.name == "nt":
                    component_path = str(prop_data[1] + ":" + prop_data[2])
                else:
                    component_path = prop_data[1]
                # Update the .repo.yaml and the bsp.yaml paths
                repo_metadata = utils.load_yaml(repo_yaml)
                # Get the index
                comp_dict = repo_metadata[component]
                path = comp_dict[component_name]['path']

                try:
                    index = path.index(component_path)
                except ValueError:
                    logger.error(f"Provided {component_path} is not a valid path")
                    sys.exit(1)

                old_path = path[0]
                path.insert(0, path.pop(index))
                utils.update_yaml(repo_yaml, "repo_meta", component, comp_dict)
                utils.replace_line(
                    obj.domain_config_file,
                    old_path,
                    f'    path: {component_path}'
                )

        # Regenerate bsp only for OSF flow
        if os.environ.get("OSF"):
            args.update({'sdt':obj.sdt})
            regenerate_bsp(args)

def main(arguments=None):
    parser = argparse.ArgumentParser(
        description="Use this script to modify BSP Settings",
        usage='use "empyro config_bsp --help" for more information',
        formatter_class=argparse.RawTextHelpFormatter,
    )
    required_argument = parser.add_argument_group("Required arguments")
    required_argument.add_argument(
        "-d",
        "--domain_path",
        action="store",
        help="Domain directory Path",
        required=True,
    )

    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument(
        "-al",
        "--addlib",
        nargs='+',
        action="store",
        default=[],
        help="Specify libaries that needs to be added if any",
    )
    group.add_argument(
        "-rl",
        "--rmlib",
        action="store",
        default="",
        help="Specify libaries that needs to be removed if any",
    )
    group.add_argument(
        "-st",
        "--set_property",
        nargs="*",
        action="store",
        help="Specify libaries with the params that need to be configured",
    )
    group.add_argument(
        "-sr",
        "--set_repo_path",
        nargs="*",
        action="store",
        help="Update the path for a given component",
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
    args = vars(parser.parse_args(arguments))
    utils.setup_log(args["verbose"])
    configure_bsp(args)

if __name__ == "__main__":
    main()
