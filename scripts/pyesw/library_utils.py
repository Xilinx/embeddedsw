# Copyright (C) 2023 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
"""
This module acts as a supporting module to get/set library related information
inside the bsp. It helps in validating the library input, generating the
library paramters database and adding/modifying the library when all the
criteria are met. It doesnt have any main() function and running this
module independently is not intended.
"""

import os
import re
import sys

import utils
from open_amp import open_amp_copy_lib_src
from repo import Repo
from utils import log_time

logger = utils.get_logger(__name__)

class Library(Repo):
    """
    This class contains attributes and functions that help in validating
    library related inputs and adding a library to the created bsp.
    """

    def __init__(
        self, domain_path, proc, bsp_os, sdt, cmake_paths_append, libsrc_folder, repo_info
    ):
        super().__init__(repo_yaml_path= repo_info)
        self.domain_path = domain_path
        self.proc = proc
        self.os = bsp_os
        self.sdt = sdt
        self.domain_config_file = os.path.join(self.domain_path, "bsp.yaml")
        self.cmake_paths_append = cmake_paths_append
        self.libsrc_folder = libsrc_folder
        self.domain_data = utils.fetch_yaml_data(self.domain_config_file, "domain")
        self.lib_list = list(self.domain_data["lib_config"].keys()) + [self.proc, self.os]
        self.bsp_lib_config = self.domain_data["lib_config"]
        self.bsp_lib_config.update(self.domain_data["os_config"])
        self.bsp_lib_config.update(self.domain_data["proc_config"])
        self.lib_info = self.domain_data["lib_info"]
        self.os_config = None
        self.cmake_generator = utils.get_cmake_generator()

    def validate_lib_name(self, lib):
        """
        Checks if the passed library name from the user is valid for the sdt
        proc and os combination. Exits with valid assertion if the user input
        is wrong.

        Args:
            lib (str): Library name that needs to be validated
        """
        lib_list_yaml_path = os.path.join(self.domain_path, "lib_list.yaml")
        if os.environ.get("VALIDATE_ARGS") == "True":
            if not utils.is_file(lib_list_yaml_path):
                utils.runcmd(
                    f"lopper --werror -f -O {self.domain_path} {self.sdt} -- baremetal_getsupported_comp_xlnx {self.proc} {self.repo_yaml_path}",
                    log_message = "Extracting and validating supported bare-metal components for processor",
                    error_message = "Could not fetch the required supported component info",
                    verbose_level = 0
                )
            proc_os_data = utils.fetch_yaml_data(
                os.path.join(self.domain_path, "lib_list.yaml"), "lib_list"
            )
            lib_list_avail = list(proc_os_data[self.proc][self.os].keys())
            if lib not in lib_list_avail:
                logger.error(
                    f"{lib} is not a valid library for the given proc and os combination. Valid library names are {lib_list_avail}"
                )
                sys.exit(1)

    def validate_lib_in_bsp(self, lib):
        """
        Checks if the passed library name from the user exists in the bsp. This
        is a helper function to support remove library and set property usecases.

        Args:
            lib (str): Library name that needs to be validated
        """
        if os.environ.get("VALIDATE_ARGS") == "True" and lib not in self.bsp_lib_config.keys():
            logger.error(f"{lib} is not added to the bsp. Add it first using -al/--addlib")
            sys.exit(1)

    def validate_lib_param(self, lib, lib_param):
        """
        Checks if the passed library parameter that needs to be set in library
        configuration is valid. Exits with a valid assertion if parameter name
        is wrong. This acts as a helper in set property usecase.

        Args:
            | lib (str): Library name whose config needs to be changed
            | lib_param (str): Library parameter that needs to be changed
        """
        if (
            os.environ.get("VALIDATE_ARGS") == "True"
            and lib_param not in self.bsp_lib_config[lib].keys()
        ):
            logger.error(
                f"{lib_param} is not a valid param for {lib}. Valid list of params are {self.bsp_lib_config[lib].keys()}"
            )
            sys.exit(1)

    def copy_lib_src(self, lib, lib_path=None):
        """
        Copies the src directory of the passed library from the respective path
        of embeddedsw to the libsrc folder of bsp.

        Args:
            lib (str): library whose source code needs to be copied

        Returns:
            | libdir (str): Library path inside embeddedsw
            | srcdir (str): Path of src folder of library inside embeddedsw
            | dstdir (str): Path of src folder inside libsrc folder of bsp
        """
        libdir = self.get_comp_dir(lib) if lib_path is None else lib_path
        srcdir = os.path.join(libdir, "src")
        if lib in ['libmetal', 'openamp']:
            if lib_path is None:
                srcdir = os.path.join(os.environ.get('XILINX_VITIS'), 'data')
            if lib == 'openamp':
                # cached library name differs from directory name in repo.
                srcdir = os.path.join(srcdir, 'open-amp')
            else: # no workaround needed for libmetal
                srcdir = os.path.join(srcdir, lib)


        dstdir = os.path.join(self.libsrc_folder, lib, "src")
        utils.copy_directory(srcdir, dstdir)
        if lib in ['libmetal', 'openamp']:
            open_amp_copy_lib_src(libdir, dstdir, lib)

        self.lib_info[lib] = {'path': libdir}
        return libdir, srcdir, dstdir

    def get_default_lib_params(self, build_lib_dir, lib_list):
        """
        Creates a library configuration data that contains all the available
        parameters and their values of each library added in the bsp.

        Args:
            build_lib_dir (str):
                Cmake directory where the libraries are configured and compiled
            lib_list (str): List of libraries added in the bsp.

        Returns:
            default_lib_config (dict):
                A dictionary that contains all the available parameters and
                their values of each library added in the bsp.
        """

        # Read all the cmake variables that got set during cmake config.
        with open(
            os.path.join(build_lib_dir, "cmake_lib_configs.txt"), "r"
        ) as cmake_confs:
            line_entries = cmake_confs.readlines()

        # Read all the cmake entries that got cached during cmake config. This
        # will help us in getting all the possible options for a given param.
        with open(os.path.join(build_lib_dir, "CMakeCache.txt"), "r") as cmake_cache:
            cmake_cache_entries = cmake_cache.readlines()

        default_lib_config = {}
        for line_index in range(0, len(line_entries)):
            lib_config_entry = {}
            permission = "read_write"
            for lib in lib_list:
                if lib not in default_lib_config.keys():
                    default_lib_config[lib] = {}
                # lwip param names are starting with lwip in cmake file but
                # lib name is lwip220.
                prefix = "lwip" if lib == "lwip220" else lib
                proc_prefix = "proc" if lib == self.proc else lib
                if (re.search(f"^{prefix}", line_entries[line_index], re.I) or
                   re.search(f"^{proc_prefix}", line_entries[line_index], re.I)):
                    param_name = line_entries[line_index].split(":")[0]
                    # In cmake there are just two types of params: option and string.
                    param_type = line_entries[line_index].split(":")[1].split("=")[0]
                    # cmake syntax is using 'ON/OFF' option, 'True/False' is lagacy entry.
                    bool_match = {"ON": "true", "OFF": "false"}
                    param_opts = []
                    try:
                        param_value = (
                            line_entries[line_index]
                            .split(":")[1]
                            .split("=", 1)[1]
                            .rstrip("\n")
                        )
                        if param_type == "BOOL":
                            param_value = bool_match[param_value]
                            param_type = "boolean"
                            # Every entry from command line will come as string
                            param_opts = ["true", "false"]
                        elif param_type == "STRING":
                            # read only params
                            read_only_param = ["proc_archiver", "proc_assembler", "proc_compiler", "proc_compiler_flags"]
                            if param_name in read_only_param:
                                permission = "readonly"
                            # Showing it as integer (legacy entry)
                            if param_value.isdigit():
                                param_type = "integer"
                            else:
                                param_type = "string"
                            for line in cmake_cache_entries:
                                # Get the cached param options
                                if re.search(f"^{param_name}-STRINGS", line):
                                    try:
                                        param_opts = (
                                            line.rstrip("\n").split("=")[1].split(";")
                                        )
                                    except:
                                        param_opts = []
                    # For some entries there is no cache option (like start
                    # address which can be anything ), it's empty.
                    except:
                        param_value = ""
                        param_type = "integer"
                        param_opts = []

                    lib_config_entry = {
                        param_name: {
                            "name": param_name,
                            "permission": permission,
                            "type": param_type,
                            "value": param_value,
                            "default": param_value,
                            "options": param_opts,
                            "description": line_entries[line_index - 1]
                            .rstrip("\n")
                            .lstrip("// "),
                        }
                    }

                    default_lib_config[lib].update(lib_config_entry)

        return default_lib_config

    def validate_drv_for_lib(self, comp_name, drvlist):
        comp_dir = self.get_comp_dir(comp_name)
        yaml_file = os.path.join(comp_dir, "data", f"{comp_name}.yaml")
        schema = utils.load_yaml(yaml_file)
        if schema.get("depends"):
            dep_drvlist = list(schema.get("depends").keys())
            valid_lib = [drv for drv in dep_drvlist if drv in drvlist]
            """
            Since sleep related implementation is part of xiltimer library
            it needs to be pulled irrespective of the hardware dependency.
            """
            if valid_lib or re.search("xiltimer", comp_name) or re.search("xilflash", comp_name):
                return True
            else:
                return False
        return True

    def is_valid_lib(self, comp_name, silent_discard=True):
        comp_dir = self.get_comp_dir(comp_name)
        yaml_file = os.path.join(comp_dir, "data", f"{comp_name}.yaml")
        schema = utils.load_yaml(yaml_file)
        cpu_list_file = os.path.join(self.domain_path, "cpulist.yaml")
        avail_cpu_data = utils.fetch_yaml_data(cpu_list_file, "cpulist")
        if schema.get("supported_processors"):
            proc_list = schema.get("supported_processors")
            proc_ip_name = avail_cpu_data[self.proc]
            if proc_ip_name in proc_list:
                return True
            else:
                if not silent_discard:
                    logger.warning(
                        f"{comp_name} Library is not valid for the given processor: {self.proc}"
                    )
                return False
        return True

    def get_depends_libs(self, lib_name, lib_list=[]):
        app_dir = self.get_comp_dir(lib_name)
        yaml_file = os.path.join(app_dir, "data", f"{lib_name}.yaml")
        schema = utils.load_yaml(yaml_file)

        if schema and schema.get("depends_libs", {}):
           for name, props in schema["depends_libs"].items():
                if not self.is_valid_lib(name, silent_discard=False):
                    continue
                lib_list += [name]
                self.get_depends_libs(name, lib_list)

        return lib_list


    @log_time
    def add_lib_for_apps(self, app_name):
        """
        Adds library to the bsp. Creates metadata if needed for the library,
        runs cmake configure to prepare the build area for library
        compilation and creates the library configuration of the bsp.

        Args:
            app_name (str):
                template app name. If template depends on certain libs, it
                copies their source to the libsrc and it fetches the req
                library configurations for those libs
        """
        def _add_lib_config(props,cmake_cmd_append,bool_match,family):
            """"Adds the config depends libs from the yaml,
                helps to add device level config based on device family
            Args:
                props(dict)            : Property values
                cmake_cmd_append(str)  : cmake command string
                bool_match(dict)       : True and False values
                family(str)            : Device family name
            """
            for key, value in props.items():
                if isinstance(value,dict):
                    if key == family:
                        cmake_cmd_append = _add_lib_config(value,cmake_cmd_append,bool_match,family)
                    else:
                        continue
                else:
                    if value in bool_match:
                        value = bool_match[value]
                    cmake_cmd_append += f" -D{key}={value}"
            return cmake_cmd_append

        cmake_cmd_append = ""
        lib_list = []

        # Read the yaml file of the passed component.
        app_dir = self.get_comp_dir(app_name)
        yaml_file = os.path.join(app_dir, "data", f"{app_name}.yaml")
        schema = utils.load_yaml(yaml_file)
        lib_config = {}
        family = self.domain_data['family']
        if schema and schema.get("depends_libs", {}):
            # cmake syntax is using 'ON/OFF' option, 'True/False' is lagacy entry.
            bool_match = {True: "ON", False: "OFF"}
            # If the passed template has any lib dependency, add those dependencies.
            for name, props in schema["depends_libs"].items():
                if not self.is_valid_lib(name):
                    continue
                lib_list += [name]
                _, _, _ = self.copy_lib_src(name)
                if props:
                    # If the template needs specific config param of the lib.
                    cmake_cmd_append = _add_lib_config(props,cmake_cmd_append,bool_match,family)
        if app_name == "zynqmp_fsbl":
            cmake_cmd_append += " -Dstandalone_zynqmp_fsbl_bsp=ON"
        # for freertos os we need to enable interval timer always
        if "freertos" in self.os:
            cmake_cmd_append += " -DXILTIMER_en_interval_timer=ON"
            toolchain_file = os.path.join(self.domain_path, self.domain_data['toolchain_file'])
            utils.add_newline(toolchain_file, 'ADD_DEFINITIONS(-DFREERTOS_BSP)')

        if schema and schema.get("os_config", {}):
            if schema.get("os_config", {})[self.os]:
                self.os_config = schema.get("os_config", {})[self.os]
                props = schema.get("os_config", {})[self.os].items()
                if props:
                    # If the template needs specific config param of the os.
                    for key, value in props:
                        cmake_cmd_append += f" -D{key}={value}"
        return lib_list, cmake_cmd_append

    @log_time
    def config_lib(self, comp_name, lib_list, cmake_cmd_append, is_app=False):
        lib_config = {}
        if lib_list:
            # Run cmake configuration with all the default cache entries
            build_metadata = os.path.join(self.libsrc_folder, "build_configs/gen_bsp")
            if ("libmetal" in lib_list):
                toolchain_file = os.path.join(self.domain_path, self.domain_data['toolchain_file'])
                utils.add_newline(toolchain_file, 'ADD_DEFINITIONS(-DXLNX_PLATFORM)')
                if ("standalone" in self.domain_data['os']):
                    utils.add_newline(toolchain_file, 'ADD_DEFINITIONS(-D__BAREMETAL__)')
            self.cmake_paths_append = self.cmake_paths_append.replace('\\', '/')
            self.domain_path = self.domain_path.replace('\\', '/')
            build_metadata = build_metadata.replace('\\', '/')
            cmake_lib_list = ";".join(lib_list)
            try:
                # BSP_LIBSRC_SUBDIRS needs to be modified to avoid checking dependency chain
                # during regen_bsp
                self.modify_cmake_subdirs(lib_list, action='add')
                if is_app:
                    cmake_cmd_append = cmake_cmd_append.replace('\\', '/')
                    utils.runcmd(
                        f'cmake -G "{self.cmake_generator}" {self.domain_path} {self.cmake_paths_append} -DSUBDIR_LIST="{cmake_lib_list}" {cmake_cmd_append} -LH > cmake_lib_configs.txt',
                        cwd = build_metadata,
                        log_message = f"Running CMake Configuration with {cmake_lib_list}",
                        error_message = f"Failed to run CMake Configuration with {cmake_lib_list}"
                    )
                else:
                    utils.runcmd(
                        f'cmake -G "{self.cmake_generator}" {self.domain_path} {self.cmake_paths_append} -DSUBDIR_LIST="{cmake_lib_list}" -LH > cmake_lib_configs.txt',
                        cwd = build_metadata,
                        log_message = f"Running CMake Configuration with {cmake_lib_list}",
                        error_message = f"Failed to run CMake Configuration with {cmake_lib_list}"
                    )
                    utils.update_yaml(self.domain_config_file, "domain", "config", "reconfig")
            except:
                lib_path = os.path.join(self.libsrc_folder, comp_name)
                # Remove library src folder from libsrc
                utils.remove(lib_path)
                self.modify_cmake_subdirs(lib_list, action='remove')
                sys.exit(1)

            # Get the default cmake entries into yaml configuration file
            ignore_default_data_list = ["standalone", "freertos10_xilinx", "libsrc"]
            for entry in ignore_default_data_list:
                if entry in lib_list:
                    lib_list.remove(entry)
            lib_config = self.get_default_lib_params(build_metadata, lib_list)
            if is_app:
                comp_dir = self.get_comp_dir(comp_name)
                yaml_file = os.path.join(comp_dir, "data", f"{comp_name}.yaml")
                schema = utils.load_yaml(yaml_file)
                # Add the modified lib param values in yaml configuration dict
                if schema.get("depends_libs", {}):
                    for name, props in schema["depends_libs"].items():
                        if not self.is_valid_lib(name):
                            continue
                        if props:
                            for key, value in props.items():
                                if not isinstance(value,dict):
                                    lib_config[name][key]["value"] = str(value)

            for lib in lib_list:
                # Update examples if any for the library
                comp_dir = self.get_comp_dir(lib)
                yaml_file = os.path.join(comp_dir, "data", f"{lib}.yaml")
                schema = utils.load_yaml(yaml_file)
                example_schema = schema.get('examples',{})
                if example_schema:
                    # check for platform dependency
                    #supported_platforms
                    example_dict = {}
                    for ex,deps in example_schema.items():
                        if deps:
                            # Read the supported_platforms check if any
                            dep_plat_list = [dep for dep in deps if "supported_platforms" in dep]
                            dep_file_list = [dep for dep in deps if "dependency_files" in dep]
                            if dep_plat_list:
                                plat_list = dep_plat_list[0]['supported_platforms']
                                if self.domain_data['family'] in plat_list:
                                    if dep_file_list:
                                        example_dict.update({ex:dep_file_list[0]['dependency_files']})
                                    else:
                                        example_dict.update({ex:[]})
                            elif dep_file_list:
                                example_dict.update({ex:dep_file_list[0]['dependency_files']})
                        else:
                            example_dict.update({ex:[]})
                    self.lib_info[lib].update({"examples":example_dict})
            # Update the yaml config file with new entries.
            utils.update_yaml(self.domain_config_file, "domain", "lib_config", lib_config)
            utils.update_yaml(self.domain_config_file, "domain", "lib_info", self.lib_info)

    def gen_lib_metadata(self, lib, lib_path=None):
        _, src_dir, dst_dir = self.copy_lib_src(lib,lib_path)
        lopper_cmd = f"lopper -O {dst_dir} -f {self.sdt} -- bmcmake_metadata_xlnx {self.proc} {src_dir} hwcmake_metadata {self.repo_yaml_path}"
        utils.runcmd(
            lopper_cmd,
            cwd = dst_dir,
            log_message = f"Generating CMake Metadata for {lib} ",
            error_message = f"Failed to generate CMake Metadata for {lib}"
        )
        if ("xilpm" in lib) and ("ZynqMP" in self.domain_data['family']):
            dstdir = os.path.join(self.libsrc_folder, lib, "src", "zynqmp", "client", "common")
            ori_sdt_path = os.path.join(self.domain_path, "hw_artifacts", "sdt.dts")
            #TODO: Update bsp.yaml before running the below command so that,
            #      the default properties of lib get updated successfully.
            lopper_cmd = f"lopper -O {dstdir} -f {ori_sdt_path} -- generate_config_object pm_cfg_obj.c {self.proc}"
            utils.runcmd(
                lopper_cmd,
                cwd = dst_dir,  # Ensure dst_dir is defined correctly
                log_message = "Generating PM config object",
                error_message = "Failed to generate PM config object"
            )

    def modify_cmake_subdirs(self, lib_list, action="add"):
        cmake_file = os.path.join(self.domain_path, "CMakeLists.txt")
        cmake_lines = []
        bsp_sudirs_substr = "set (BSP_LIBSRC_SUBDIRS "
        with open(cmake_file, 'r') as file:
            cmake_lines = file.readlines()
        for line_index in range(0,len(cmake_lines)):
            if cmake_lines[line_index].startswith(bsp_sudirs_substr):
                subdir_line = cmake_lines[line_index]
                subdir_space_sep_str = re.search(rf'{re.escape(bsp_sudirs_substr)}(.+?)\)', subdir_line)
                subdir_list = subdir_space_sep_str.group(1).split()
                if action=='add':
                    subdir_list = list(dict.fromkeys(subdir_list + lib_list))
                elif action=='remove':
                    for lib_name in lib_list:
                        if lib_name in subdir_list:
                            subdir_list.remove(lib_name)
                cmake_lines[line_index] = f'{bsp_sudirs_substr}{" ".join(subdir_list)})\n'
                break
        with open(cmake_file, 'w') as file:
            file.writelines(cmake_lines)

    def remove_lib(self, lib):
        self.validate_lib_in_bsp(lib)
        lib_path = os.path.join(self.libsrc_folder, lib)
        base_lib_build_dir = os.path.join(self.libsrc_folder, "build_configs", "gen_bsp")
        lib_build_dir = os.path.join(base_lib_build_dir, "libsrc", lib)
        targetdir_list = os.path.join(base_lib_build_dir, "CMakeFiles", "TargetDirectories.txt")
        has_lib_cmake_cache = utils.check_if_line_in_file(targetdir_list, f'{lib}.dir')
        if not has_lib_cmake_cache:
            self.domain_path = self.domain_path.replace('\\','/')
            self.cmake_paths_append = self.cmake_paths_append.replace('\\','/')
            utils.runcmd(
                f'cmake -G "{self.cmake_generator}" {self.domain_path} -DSUBDIR_LIST="ALL" {self.cmake_paths_append}',
                cwd = base_lib_build_dir,
                log_message = "Configuring CMake with all Subdir list",
                error_message = "CMake Configuration with all Subdir list failed"
            )

        # Run make clean to remove the respective headers and .a from lib and include folder.
        if os.name == "nt":
            base_lib_build_dir = base_lib_build_dir.replace('\\', '/')
            utils.runcmd(
                r'cmake -DCONFIG="" -P CMakeFiles\clean_additional.cmake',
                cwd = base_lib_build_dir,
                log_message = "Running CMake clean ",
                error_message = "Failed to run CMake clean "
            )
        else:
            utils.runcmd(
                'cmake -DCONFIG="" -P CMakeFiles/clean_additional.cmake',
                cwd = base_lib_build_dir,
                log_message = "Running CMake clean ",
                error_message = "Failed to run CMake clean "
            )
        # Remove library src folder from libsrc
        utils.remove(lib_path)
        # Remove cmake build folder from cmake build area.
        utils.remove(lib_build_dir)
        # Update library config file
        utils.update_yaml(self.domain_config_file, "domain", lib, None, action="remove")
        dump = utils.discard_dump()
        utils.runcmd(
            f"ninja CMakeFiles/rebuild_cache.util > {dump}",
            cwd = base_lib_build_dir,
            log_message = " Rebuilding CMake cache.util ",
            error_message = " Failed to rebuild CMake cache.util "
        )
        self.modify_cmake_subdirs([lib], action="remove")
