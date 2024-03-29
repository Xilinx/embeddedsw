# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
"""
This module creates a domain and a bsp for the passed processor, os and system
device tree combination.
"""

import argparse, textwrap
import sys
import os
import lopper
import utils
import re
from library_utils import Library
from repo import Repo
from validate_bsp import Validation


class Domain(Repo):
    """
    This class helps in creating a software domain. This contains functions
    to create the domain's directory structure, validate the user inputs for
    the domain on demand and manipulate the cmake toolchain file as per the
    user inputs.
    """

    def __init__(self, args):
        super().__init__(repo_yaml_path=args["repo_info"])
        self.domain_dir = utils.get_abs_path(args["ws_dir"])
        self.proc = args["proc"]
        self.os = args["os"]
        self.app = args["template"]
        self.compiler_flags = ""
        self.toolchain_file = None
        self.sdt = utils.get_abs_path(args["sdt"])
        self.family = self._get_family()
        self.lops_dir = os.path.join(utils.get_dir_path(lopper.__file__), "lops")
        self.include_folder = os.path.join(self.domain_dir, "include")
        self.lib_folder = os.path.join(self.domain_dir, "lib")
        self.libsrc_folder = os.path.join(self.domain_dir, "libsrc")
        self.sdt_folder = os.path.join(self.domain_dir, "hw_artifacts")
        self.domain_config_file = os.path.join(self.domain_dir, "bsp.yaml")
        self.repo_paths_list = self.repo_schema['paths']
        self.drv_info = {}
        self.os_info = {}
        utils.mkdir(self.domain_dir)
        self._validate_inputs()

    def _get_family(self):
        """
        This function intends to fetch the platform/family name from the input
        sdt. Usage of this is under discussion and this may be removed later.

        Returns:
            family (str): versal/zynqmp
        """
        with open(self.sdt, "r") as file:
            content = file.read()
            if "cpus_a53" in content:
                return "ZynqMP"
            elif "cpus_a72" in content:
                return "Versal"
            elif "cpus_a9" in content:
                return "Zynq"
            elif "cpus_a78" in content:
                return "VersalNet"

    def _validate_inputs(self):
        """
        If User wants to validate the inputs before creating the domain,
        'VALIDATE_ARGS' needs to be set over console. Once set, this function
        will come into action and validate if the processor, the os, the
        template app passed over command line are valid or not for the
        sdt input.
        """
        cpu_list_file = os.path.join(self.domain_dir, "cpulist.yaml")
        if not utils.is_file(cpu_list_file):
            utils.runcmd(
                f"lopper --werror -f -O {self.domain_dir} -i {self.lops_dir}/lop-cpulist.dts {self.sdt} > nul",
                cwd = self.domain_dir
            )
        if os.environ.get("VALIDATE_ARGS"):
            app_list_file = os.path.join(self.domain_dir, "app_list.yaml")
            lib_list_file = os.path.join(self.domain_dir, "lib_list.yaml")

            avail_cpu_data = utils.fetch_yaml_data(cpu_list_file, "cpulist")
            if self.proc not in avail_cpu_data.keys():
                utils.remove(self.domain_dir)
                print(
                    f"[ERROR]: Please pass a valid processor name. Valid Processor Names for the given SDT are: {list(avail_cpu_data.keys())}"
                )
                sys.exit(1)
            if not utils.is_file(app_list_file) or not utils.is_file(lib_list_file):
                utils.runcmd(
                    f"lopper --werror -f -O {self.domain_dir} {self.sdt} -- baremetal_getsupported_comp_xlnx {self.proc} {self.repo_yaml_path}",
                    cwd = self.domain_dir
                )
            proc_data = utils.fetch_yaml_data(app_list_file, "app_list")[self.proc]
            Validation.validate_template_name(
                self.domain_dir, proc_data, self.os, self.app
            )

    def build_dir_struct(self):
        """
        Creates the include, lib and libsrc folder inside bsp directory.
        """
        utils.mkdir(self.include_folder)
        utils.mkdir(self.lib_folder)
        utils.mkdir(self.libsrc_folder)
        utils.mkdir(self.sdt_folder)
        if utils.is_dir(os.path.join(utils.get_dir_path(self.sdt), "drivers"), silent_discard=True):
            utils.copy_directory(os.path.join(utils.get_dir_path(self.sdt), "drivers"),
                                 os.path.join((self.sdt_folder), "drivers"))

        if self.family == "ZynqMP":
            init_file = "psu_init"
        elif self.family == "Zynq":
            init_file = "ps7_init"
        else:
            init_file = ""

        if init_file:
            utils.copy_file(os.path.join(utils.get_dir_path(self.sdt), f"{init_file}.c"),
                            os.path.join(self.sdt_folder, f"{init_file}.c"), silent_discard=True)
            utils.copy_file(os.path.join(utils.get_dir_path(self.sdt), f"{init_file}.h"),
                            os.path.join(self.sdt_folder, f"{init_file}.h"), silent_discard=True)

    def toolchain_intr_mapping(self):
        """
        We have reference toolchain files in embeddedsw which contains default
        compiler related cmake inputs. This function copies the toolchain file
        according to user os and processor input in the domain directory. Once
        copied, it manipulates few entries in the file needed for specific proc
        /os/app scenario.

        In addition, this function also processes the sdt directory to create a
        a single system dts file that has interrupts correctly mapped as per
        the input processor.

        Returns:
            sdt (str):
                Processed system device tree file that would be used across the
                created domain for further processing.
            toolchain_file (str):
                Toolchain file for cmake infra that would be used across the
                created domain for builds.
        """

        # no gic mapping is needed for procs other than APU and RPU
        proc_lops_specs_map = {
            "a53": ("cortexa53", "lop-a53-imux", "arm"),
            "a72": ("cortexa72", "lop-a72-imux", "arm"),
            "a78": ("cortexa78", "lop-a78-imux", "arm"),
            "r52": ("cortexr52", "lop-r52-imux", "arm"),
            "r5": ("cortexr5", "lop-r5-imux", "arm"),
            "a9": ("cortexa9", "", "arm"),
            "pmu": ("microblaze-pmu", "", "microblaze"),
            "pmc": ("microblaze-plm", "", "microblaze"),
            "psm": ("microblaze-psm", "", "microblaze"),
            "microblaze": ("microblaze", "", "microblaze"),
        }
        lops_file = ""
        out_dts_path = os.path.join(self.sdt_folder, f"{self.proc}_baremetal.dts")

        # Save unpruned SDT as it may be used later
        ori_sdt_path = os.path.join(self.sdt_folder, "sdt.dts")
        utils.runcmd(f"lopper -f -O {self.domain_dir} --enhanced  --permissive {self.sdt} {ori_sdt_path} > nul")

        toolchain_file_copy = None
        for val in proc_lops_specs_map.keys():
            if val in self.proc:
                toolchain_file_name = f"{proc_lops_specs_map[val][0]}_toolchain.cmake"
                toolchain_file_path = utils.get_high_precedence_path(
                    self.repo_paths_list, "toolchain File", "cmake", "toolchainfiles", toolchain_file_name
                )
                lops_file = os.path.join(self.lops_dir, f"{proc_lops_specs_map[val][1]}.dts")
                toolchain_file_copy = os.path.join(self.domain_dir, toolchain_file_name)
                utils.copy_file(toolchain_file_path, toolchain_file_copy)
                specs_file = utils.get_high_precedence_path(
                    self.repo_paths_list, "Xilinx.spec File", "scripts", "specs", proc_lops_specs_map[val][2], "Xilinx.spec"
                )
                specs_copy_file = os.path.join(self.domain_dir, 'Xilinx.spec')
                utils.copy_file(specs_file, specs_copy_file)
                break

        if self.proc == "psx_pmc_0" or self.proc == "psx_psm_0":
            utils.replace_line(
                toolchain_file_copy,
                'CMAKE_MACHINE "Versal',
                f'set( CMAKE_MACHINE "VersalNet" CACHE STRING "cmake machine" FORCE)',
            )
            utils.replace_string(
                toolchain_file_copy,
                "-mcpu=v10.0",
                "-mcpu=v11.0")

        if "r5" in self.proc:
            utils.replace_line(
                toolchain_file_copy,
                'CMAKE_MACHINE "Versal',
                f'set( CMAKE_MACHINE "{self.family}")',
            )

        if "microblaze" in self.proc:
            lops_file = os.path.join(self.lops_dir, "lop-microblaze.dts")
            vitis_path = os.environ.get("XILINX_VITIS")

            utils.runcmd(
                f"lopper -f -O {self.domain_dir} --enhanced -i {lops_file} {self.sdt} > nul",
                cwd = self.domain_dir
            )
            cflags_file = os.path.join(self.domain_dir, "cflags.yaml")
            avail_cflag_data = utils.fetch_yaml_data(cflags_file, "cflags")
            cflags = avail_cflag_data.get("cflags")

            utils.replace_line(
                toolchain_file_copy,
                f'set( CMAKE_HW_FLAGS "" )',
                f'set( CMAKE_HW_FLAGS "{cflags}" )\n',
            )

            #TODO: Handle libpath in cflags.yaml based on OS
            relative_libpath = avail_cflag_data.get("libpath")
            vitis_path = os.environ.get("XILINX_VITIS")
            if os.name == "nt":
                libpath = os.path.join(vitis_path, "gnu", "microblaze", "nt", relative_libpath)
            else:
                libpath = os.path.join(vitis_path, "gnu", "microblaze", "lin", relative_libpath)
            libpath = libpath.replace('\\', '/')

            assert utils.is_dir(libpath), f"Microblaze compiler library path {libpath} is not found."

            utils.replace_line(
                toolchain_file_copy,
                f'set( CMAKE_COMPILER_LIB_PATH "" )',
                f'set( CMAKE_COMPILER_LIB_PATH "{libpath}" )\n',
            )

        # freertos needs a separate CMAKE_SYSTEM_NAME
        if "freertos" in self.os:
            utils.add_newline(toolchain_file_copy, "set( CMAKE_SYSTEM_NAME FreeRTOS)")
        # Do the gic pruning in the sdt for APU/RPU.
        if self.sdt != out_dts_path:
            if utils.is_file(lops_file):
                utils.runcmd(
                    f"lopper -f --enhanced -O {self.domain_dir} -i {lops_file} {self.sdt} {out_dts_path}"
                )
            else:
                utils.runcmd(
                    f"lopper -f --enhanced -O {self.domain_dir} {self.sdt} {out_dts_path}"
                )
        else:
            out_dts_path = self.sdt

        self.compiler_flags = self.apps_cflags_update(
            toolchain_file_copy, self.app, self.proc
        )

        return out_dts_path, toolchain_file_copy, specs_copy_file

    def apps_cflags_update(self, toolchain_file, app_name, proc):
        """
        This function acts as a helper for toolchain_intr_mapping. This adds
        template application specific compiler entries in the cmake toolchain
        file of the domain.

        Args:
            | toolchain_file (str): The toolchain file that needs to be updated
            | app_name (str): Specific app name that needs new entries
            | proc (str): Proc specific data pertaining to the app.

        Returns:
            compiler_flags (str): returns the new compiler flags that were set.
        """
        compiler_flags = ""
        if app_name == "zynqmp_fsbl":
            if "a53" in proc:
                compiler_flags = "-Os -flto -ffat-lto-objects -DARMA53_64"
            if "r5" in proc:
                compiler_flags = "-Os -flto -ffat-lto-objects -DARMR5"

            utils.add_newline(
                toolchain_file,
                f'set( CMAKE_C_FLAGS "${{TOOLCHAIN_C_FLAGS}} ${{TOOLCHAIN_DEP_FLAGS}} -specs=${{CMAKE_SPECS_FILE}} {compiler_flags} -I${{CMAKE_INCLUDE_PATH}}" CACHE STRING "CFLAGS")',
            )
            utils.add_newline(
                toolchain_file,
                f'set( CMAKE_CXX_FLAGS "${{TOOLCHAIN_CXX_FLAGS}} ${{TOOLCHAIN_DEP_FLAGS}} -specs=${{CMAKE_SPECS_FILE}} {compiler_flags} -I${{CMAKE_INCLUDE_PATH}}")',
            )
            utils.add_newline(
                toolchain_file,
                f'set( CMAKE_ASM_FLAGS "${{TOOLCHAIN_ASM_FLAGS}} ${{TOOLCHAIN_DEP_FLAGS}} -specs=${{CMAKE_SPECS_FILE}} {compiler_flags} -I${{CMAKE_INCLUDE_PATH}}")',
            )
        else:
            utils.add_newline(
                toolchain_file,
                f'set( CMAKE_C_FLAGS "${{TOOLCHAIN_C_FLAGS}} ${{TOOLCHAIN_DEP_FLAGS}} -specs=${{CMAKE_SPECS_FILE}} -I${{CMAKE_INCLUDE_PATH}}" CACHE STRING "CFLAGS")',
            )
            utils.add_newline(
                toolchain_file,
                f'set( CMAKE_CXX_FLAGS "${{TOOLCHAIN_CXX_FLAGS}} ${{TOOLCHAIN_DEP_FLAGS}} -specs=${{CMAKE_SPECS_FILE}} -I${{CMAKE_INCLUDE_PATH}}" CACHE STRING "CXXFLAGS")',
            )
            utils.add_newline(
                toolchain_file,
                f'set( CMAKE_ASM_FLAGS "${{TOOLCHAIN_ASM_FLAGS}} ${{TOOLCHAIN_DEP_FLAGS}} -specs=${{CMAKE_SPECS_FILE}} -I${{CMAKE_INCLUDE_PATH}}" CACHE STRING "ASMFLAGS")',
            )
        return compiler_flags


def cmake_add_target(comp_name, output_dir, sdt, cmd, output):
    cmake_cmd = f"""
add_custom_target(
	{comp_name} ALL
	COMMAND lopper -O {output_dir} {sdt} -- {cmd}
	BYPRODUCTS {output}
)
"""
    return cmake_cmd

def cmake_drv_custom_target(proc, libsrc_folder, sdt, cmake_drv_name_list, cmake_drv_path_list):
    cmake_cmd = f'''
set(DRIVER_TARGETS {cmake_drv_name_list})
set(DRIVER_LOCATIONS {cmake_drv_path_list})

list(LENGTH DRIVER_TARGETS no_of_drivers)
set(index 0)

while(${{index}} LESS ${{no_of_drivers}})
    list(GET DRIVER_TARGETS ${{index}} drv)
    list(GET DRIVER_LOCATIONS ${{index}} drv_dir)
    set(src_dir "${{drv_dir}}/src")
    add_custom_target(
        ${{drv}} ALL
        COMMAND lopper -O {libsrc_folder}/${{drv}}/src {sdt} -- baremetalconfig_xlnx {proc} ${{src_dir}}
        BYPRODUCTS x${{drv}}_g.c)
    MATH(EXPR index "${{index}}+1")
endwhile()
'''
    return cmake_cmd

def create_domain(args):
    """
    Function that uses the above Domain class to create the baremetal domain.
    Args:
        args (dict): Takes all the user inputs in a dictionary.
    """

    # Initialize the Domain class
    obj = Domain(args)

    # Create the bsp directory structure.
    obj.build_dir_struct()

    # Create the Domain specific sdt and the toolchain file.
    obj.sdt, obj.toolchain_file, obj.specs_file = obj.toolchain_intr_mapping()

    # Common cmake variables to support cmake build infra.
    cmake_paths_append = f" -DCMAKE_LIBRARY_PATH={obj.lib_folder} \
            -DCMAKE_INCLUDE_PATH={obj.include_folder} \
            -DCMAKE_MODULE_PATH={obj.domain_dir} \
            -DCMAKE_TOOLCHAIN_FILE={obj.toolchain_file} \
            -DCMAKE_SPECS_FILE={obj.specs_file} \
            -DCMAKE_VERBOSE_MAKEFILE=ON"

    # Create top level CMakeLists.txt inside domain dir
    cmake_file = os.path.join(obj.domain_dir, "CMakeLists.txt")

    # Copy the standalone bsp src file.
    os_dir_path = obj.get_comp_dir("standalone")
    os_srcdir = os.path.join(os_dir_path, "src")
    bspsrc = os.path.join(obj.libsrc_folder, "standalone", "src")
    utils.copy_directory(os_srcdir, bspsrc)
    obj.os_info['standalone'] = {'path': os_dir_path}

    cmake_header = """
cmake_minimum_required(VERSION 3.15)
project(bsp)
find_package(common)
    """
    cmake_file_cmds = cmake_header

    cmd = f"baremetal_bspconfig_xlnx {obj.proc} {os_srcdir}"
    cmake_file_cmds += cmake_add_target("xilstandalone_config", bspsrc, obj.sdt, cmd, "MemConfig.cmake")
    bspcomsrc = os.path.join(obj.libsrc_folder, "standalone", "src", "common")
    cmd = f"bmcmake_metadata_xlnx {obj.proc} {os_srcdir} hwcmake_metadata {obj.repo_yaml_path}"
    cmake_file_cmds += cmake_add_target("xilstandalone_meta", bspcomsrc, obj.sdt, cmd, "StandaloneExample.cmake")

    # Copy cmake file that contains cmake utility APIs to a common location.
    find_common_cmake_path = utils.get_high_precedence_path(
            obj.repo_paths_list, "Findcommon.cmake", "cmake", "Findcommon.cmake"
        )
    utils.copy_file(
        find_common_cmake_path,
        os.path.join(obj.domain_dir, "Findcommon.cmake"),
        silent_discard=False,
    )

    """
    Generate metadata for driver compilation. Metadata includes driver
    list available in sdt and a cmake file that enables the generation of
    _g.c files for available drivers in parallel.
    """
    utils.runcmd(
        f"lopper -O {obj.libsrc_folder} -f {obj.sdt} -- baremetaldrvlist_xlnx {obj.proc} {obj.repo_yaml_path}"
    )

    # Read the driver list available in libsrc folder
    drv_list_file = os.path.join(obj.libsrc_folder, "distro.conf")
    cmake_drv_path_list = ""
    cmake_drv_name_list = ""

    with open(drv_list_file, "r") as fd:
        drv_names = (
            re.search('DISTRO_FEATURES = "(.*)"', fd.readline())
            .group(1)
            .replace("-", "_")
        )
        drv_list = drv_names.split()

    drv_lib_dep = []
    for drv in drv_list:
        drv_path = obj.get_comp_dir(drv, obj.sdt_folder)

        drv_srcdir = os.path.join(drv_path, "src")
        drvsrc = os.path.join(obj.libsrc_folder, drv, "src")
        utils.copy_directory(drv_srcdir, drvsrc)
        yaml_file = os.path.join(drv_path, "data", f"{drv}.yaml")
        schema = utils.load_yaml(yaml_file)
        if schema and schema.get("depends_libs", {}):
           drv_lib_dep.extend(list(schema["depends_libs"].keys()))

        if not drv_path:
            print(f"[ERROR]: Couldnt find the src directory for {drv}. {drv_path} doesnt exist.")
            sys.exit(1)
        cmake_drv_path_list += f"{drv_path};"

    ip_drv_map_file = os.path.join(obj.libsrc_folder, "ip_drv_map.yaml")
    ip_drv_map = utils.load_yaml(ip_drv_map_file)
    for ip,data in ip_drv_map.items():
        driver = data[1]
        if driver != "None":
            drv_path = obj.get_comp_dir(driver, obj.sdt_folder)
            if not drv_path:
                print(f"[ERROR]: Couldnt find the src directory for {drv}. {drv_path} doesnt exist.")
                sys.exit(1)
            obj.drv_info[ip] = {'driver': driver,
                                'ip_name': data[0],
                                'path' : drv_path}
        else:
            obj.drv_info[ip] = "None"

    cmake_drv_name_list += ';'.join(drv_list)
    cmake_file_cmds += cmake_drv_custom_target(obj.proc, obj.libsrc_folder, obj.sdt, cmake_drv_name_list, cmake_drv_path_list)
    cmd = f"baremetal_xparameters_xlnx {obj.proc} {obj.repo_yaml_path}"
    cmake_file_cmds += cmake_add_target("xparam", obj.include_folder, obj.sdt, cmd, "xparameters.h")

    """
    Create a dictionary that will contain the current status of the domain.
    This data will later be used during bsp configuartion, bsp build and
    app creation stages.
    """
    data = {
        "sdt": utils.get_rel_path(obj.sdt, obj.domain_dir),
        "family": obj.family,
        "path": obj.domain_dir,
        "os": obj.os,
        "os_info": obj.os_info,
        "os_config": {},
        "toolchain_file": utils.get_rel_path(obj.toolchain_file, obj.domain_dir),
        "specs_file": utils.get_rel_path(obj.specs_file, obj.domain_dir),
        "proc": obj.proc,
        "proc_config": {},
        "template": obj.app,
        "compiler_flags": obj.compiler_flags,
        "include_folder": utils.get_rel_path(obj.include_folder, obj.domain_dir),
        "lib_folder": utils.get_rel_path(obj.lib_folder, obj.domain_dir),
        "libsrc_folder": utils.get_rel_path(obj.libsrc_folder, obj.domain_dir),
        "drv_info": obj.drv_info,
        "lib_info": {},
        "lib_config": {}
    }

    # Write the domain specific data as a configuration file in yaml format.
    utils.write_yaml(obj.domain_config_file, data)

    # If domain has to be created for a certain template, few libraries need
    # to be added in the bsp.
    lib_list = ["xiltimer"]
    cmake_cmd_append = ""
    lib_obj = Library(
        obj.domain_dir, obj.proc, obj.os, obj.sdt, cmake_paths_append, obj.libsrc_folder, obj.repo_yaml_path
    )

    if obj.app:
        # If template app is passed, read the app's yaml file and add
        # lib accordingly.
        lib_list, cmake_cmd_append = lib_obj.add_lib_for_apps(obj.app)
    else:
        # If no app is passed and bsp is created add xiltimer by default.
        lib_obj.copy_lib_src("xiltimer")

    if drv_lib_dep:
        #Remove duplicate entries
        drv_lib_dep = list(set(drv_lib_dep))
        lib_list.extend(drv_lib_dep)
        for lib in drv_lib_dep:
            lib_obj.copy_lib_src(lib)

    if obj.os == "freertos":
        # Copy the freertos source code to libsrc folder
        os_dir_path = obj.get_comp_dir("freertos10_xilinx")
        os_srcdir = os.path.join(os_dir_path, "src")
        bspsrc = os.path.join(obj.libsrc_folder, "freertos10_xilinx/src")
        utils.copy_directory(os_srcdir, bspsrc)
        obj.os_info['freertos10_xilinx'] = {'path': os_dir_path}
        lib_list.append("freertos10_xilinx")


    if lib_list:
        for lib in lib_list:
            lib_dir_path = obj.get_comp_dir(lib)
            srcdir = os.path.join(lib_dir_path, "src")
            dstdir = os.path.join(obj.libsrc_folder, f"{lib}/src")
            cmd = f"bmcmake_metadata_xlnx {obj.proc} {srcdir} hwcmake_metadata {obj.repo_yaml_path}"
            outfile = f"{lib}Example.cmake"
            cmake_file_cmds += cmake_add_target(lib, dstdir, obj.sdt, cmd, outfile)
            if ("xilpm" in lib) and ("ZynqMP" in obj.family):
                dstdir = os.path.join(obj.libsrc_folder, f"{lib}/src/zynqmp/client/common/")
                cmd = f"generate_config_object pm_cfg_obj.c {obj.proc}"
                cmake_file_cmds += cmake_add_target("xilpm_cfg_obj", dstdir, obj.sdt, cmd, "pm_cfg_obj.c")

    cmake_file_cmds += f"\nadd_library(bsp INTERFACE)"
    cmake_file_cmds = cmake_file_cmds.replace('\\', '/')
    cmake_file_cmds += f"\nadd_dependencies(bsp xilstandalone_config xilstandalone_meta xparam {' '.join(lib_list)} {cmake_drv_name_list})"
    utils.write_into_file(cmake_file, cmake_file_cmds)

    # Create a build directory for cmake to generate all _g.c files.
    build_metadata = os.path.join(obj.libsrc_folder, "build_configs", "gen_bsp")
    utils.mkdir(build_metadata)

    # Run cmake configure and build to generate _g.c files
    obj.domain_dir = obj.domain_dir.replace('\\', '/')
    obj.toolchain_file = obj.toolchain_file.replace('\\', '/')
    cmake_paths_append = cmake_paths_append.replace('\\', '/')
    build_metadata = build_metadata.replace('\\', '/')
    utils.runcmd(
        f'cmake -G "Unix Makefiles" {obj.domain_dir} {cmake_paths_append}',
        cwd = build_metadata
    )

    utils.runcmd("make -f CMakeFiles/Makefile2 -j22 > nul", cwd = build_metadata)

    # Copy the actual drivers cmake file in the libsrc folder.
    # This is to compile all the available driver sources.
    libxil_cmake = utils.get_high_precedence_path(
        obj.repo_paths_list, "Driver compilation CMake File", "XilinxProcessorIPLib", "drivers", "CMakeLists.txt"
    )
    utils.copy_file(libxil_cmake, f"{obj.libsrc_folder}/")

    bsp_libsrc_cmake_subdirs = "libsrc standalone " + " ".join(lib_list)

    # Create new CMakeLists.txt
    cmake_file_cmds = f'''{cmake_header}
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${{CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}})
    set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${{CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES}})
endif()

ADD_DEFINITIONS(-c ${{proc_extra_compiler_flags}})
include_directories(${{CMAKE_BINARY_DIR}}/include)

if (EXISTS ${{metal_BINARY_DIR}})
include_directories(${{metal_BINARY_DIR}}/lib/include)
endif()
set (BSP_LIBSRC_SUBDIRS {bsp_libsrc_cmake_subdirs})

if (SUBDIR_LIST STREQUAL "ALL")
    set (SUBDIR_LIST ${{BSP_LIBSRC_SUBDIRS}})
endif()

foreach(entry ${{SUBDIR_LIST}})
    if(entry STREQUAL "libsrc")
        set (path "${{CMAKE_LIBRARY_PATH}}/../libsrc")
    else()
        set (path "${{CMAKE_LIBRARY_PATH}}/../libsrc/${{entry}}/src")
    endif()
    if(EXISTS ${{path}})
        add_subdirectory(${{path}})
    endif()
endforeach()
    '''

    utils.write_into_file(cmake_file, cmake_file_cmds)

    build_metadata = os.path.join(obj.libsrc_folder, "build_configs", "gen_bsp")
    lib_list += ["standalone"]
    if obj.app:
        lib_obj.config_lib(obj.app, lib_list, cmake_cmd_append, is_app=True)
    else:
        # If no app is passed and bsp is created xiltimer got added default
        # Update config entries for the same.
        lib_obj.config_lib(None, lib_list, "", is_app=False)

    # Run cmake configuration with all the default cache entries
    cmake_paths_append = cmake_paths_append.replace('\\', '/')
    build_metadata = build_metadata.replace('\\', '/')

    if obj.os == "freertos":
        os_config = lib_obj.get_default_lib_params(build_metadata, ["freertos"])
    else:
        os_config = lib_obj.get_default_lib_params(build_metadata, ["standalone"])
    proc_config = lib_obj.get_default_lib_params(build_metadata,[obj.proc])

    if lib_obj.os_config:
        props = lib_obj.os_config.items()
        if props:
            for key, value in props:
                if os_config[obj.os].get(key, {}):
                    os_config[obj.os][key]['value'] = value

    if "microblaze" in obj.proc:
        cmake_config = lib_obj.get_default_lib_params(build_metadata, ["cmake"])
        if cmake_config['cmake'].get('CMAKE_MACHINE', {}):
            obj.family = cmake_config['cmake']['CMAKE_MACHINE']['value']
            utils.update_yaml(obj.domain_config_file, "domain", "family", obj.family)

    utils.update_yaml(obj.domain_config_file, "domain", "os_config", os_config)
    utils.update_yaml(obj.domain_config_file, "domain", "proc_config", proc_config)
    utils.update_yaml(obj.domain_config_file, "domain", "os_info", obj.os_info)
    utils.update_yaml(obj.domain_config_file, "domain", "lib_info", lib_obj.lib_info)

    # Remove the metadata files that are no longer needed.
    utils.remove(drv_list_file)
    utils.remove(ip_drv_map_file)
    utils.remove(os.path.join(obj.libsrc_folder, "libxil.conf"))

    utils.remove(os.path.join(obj.domain_dir, "*.dtb"), pattern=True)
    utils.remove(os.path.join(obj.domain_dir, "*.pp"), pattern=True)

    '''
    compile_commands.json file needs to be kept inside libsrc directory.
    '''
    utils.copy_file(os.path.join(build_metadata, "compile_commands.json"), obj.libsrc_folder)

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
    clangd_ignore_file = os.path.join(obj.libsrc_folder, ".clangd")
    utils.write_into_file(clangd_ignore_file, clangd_ignore_content)

    # Success prints if everything went well till this point
    if utils.is_file(obj.domain_config_file):
        print(f"Successfully created Domain at {obj.domain_dir}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Create bsp for the given sdt, os, processor and template app",
        usage='use "python %(prog)s --help" for more information',
        formatter_class=argparse.RawTextHelpFormatter,
    )
    required_argument = parser.add_argument_group("Required arguments")
    required_argument.add_argument(
        "-p",
        "--proc",
        action="store",
        help="Specify the processor name",
        required=True,
    )
    required_argument.add_argument(
        "-s",
        "--sdt",
        action="store",
        help="Specify the System device-tree path (till system-top.dts file)",
        required=True,
    )
    parser.add_argument(
        "-w",
        "--ws_dir",
        action="store",
        help="Workspace directory where domain will be created (Default: Current Work Directory)",
        default='.',
    )
    parser.add_argument(
        "-o",
        "--os",
        action="store",
        default="standalone",
        help="Specify OS (Default: standalone)",
        choices=["standalone", "freertos"],
    )
    parser.add_argument(
        "-t",
        "--template",
        action="store",
        default="",
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


    args = vars(parser.parse_args())
    create_domain(args)
