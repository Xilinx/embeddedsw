# Copyright (C) 2023 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
"""
This module creates a domain and a bsp for the passed processor, os and system
device tree combination.
"""

import argparse
import os
import re
import sys
import textwrap

import utils
from library_utils import Library
from repo import Repo
from utils import log_time
from validate_bsp import Validation
from validate_hw import ValidateHW

logger = utils.get_logger(__name__)

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
        self.include_folder = os.path.join(self.domain_dir, "include")
        self.lib_folder = os.path.join(self.domain_dir, "lib")
        self.libsrc_folder = os.path.join(self.domain_dir, "libsrc")
        self.sdt_folder = os.path.join(self.domain_dir, "hw_artifacts")
        self.domain_config_file = os.path.join(self.domain_dir, "bsp.yaml")
        self.repo_paths_list = self.repo_schema['paths']
        self.drv_info = {}
        self.os_info = {}
        self.proc_mode = args["mode"]
        self.compiler = args["compiler"]
        utils.mkdir(self.domain_dir)
        self.proc_ip_name = self._validate_inputs()

    def _get_family(self):
        """
        This function intends to fetch the platform/family name from the input
        sdt. Usage of this is under discussion and this may be removed later.

        Returns:
            family (str): versal/zynqmp
        """
        with open(self.sdt, "r") as file:
            content = file.read()
            family_str = re.findall(r"[ \t\n]*family = \"(.*?)\"", content)
            if "cpus_a53" in content or 'family = "ZynqMP";' in content:
                return "ZynqMP"
            elif "cpus_a72" in content or 'family = "Versal";' in content:
                return "Versal"
            elif "cpus_a9" in content or 'family = "Zynq";' in content:
                return "Zynq"
            elif 'family = "VersalNet";' in content:
                return "VersalNet"
            elif 'family = "VersalGen2";' in content:
                return "Versal_2VE_2VM"
            elif family_str:
                return family_str[0]

    def _validate_inputs(self):
        """
        If User wants to validate the inputs before creating the domain,
        'VALIDATE_ARGS' needs to be set over console. Once set, this function
        will come into action and validate if the processor, the os, the
        template app passed over command line are valid or not for the
        sdt input.
        """

        cpu_list_file = os.path.join(self.domain_dir, "cpulist.yaml")
        dump = utils.discard_dump()
        if not utils.is_file(cpu_list_file):
            utils.runcmd(
                f"lopper --werror -f -O {self.domain_dir} -i lop-cpulist.dts {self.sdt}",
                cwd = self.domain_dir,
                log_message = "Generating CPU list",
                error_message = "CPU List generation failed."
            )
        cpulist_metafiles = ["cpulist.yaml","lop-cpulist.dts.dtb","lop-cpulist.dts.pp"]
        avail_cpu_data = utils.fetch_yaml_data(cpu_list_file, "cpulist")
        if self.proc not in avail_cpu_data.keys():
            utils.remove_directory(self.domain_dir, cpulist_metafiles, force_remove=False)
            logger.error(
                f"Please pass a valid processor name. Valid Processor Names for the given SDT are: {list(avail_cpu_data.keys())}"
            )
            sys.exit(1)

        proc_ip_name = avail_cpu_data[self.proc]
        if proc_ip_name in ("cortexa78", "cortexr52") and "freertos" in self.os:
            utils.remove_directory(self.domain_dir, cpulist_metafiles, force_remove=False)
            logger.error(
                f"Invalid os selection, {self.os} is not supported for the {self.proc} processor"
            )
            sys.exit(1)

        if "a53" not in self.proc and self.proc_mode == "32-bit":
            utils.remove_directory(self.domain_dir, cpulist_metafiles, force_remove=False)
            logger.error(
                f"Invalid mode configuration, 32-bit mode configuration is applicable only for ZynqMP A53 processors"
            )
            sys.exit(1)

        if self.app:
            validate_obj = ValidateHW(
                self.domain_dir, self.proc, self.os, self.sdt,
                self.app, self.repo_yaml_path
            )
            validate_obj.validate_hw()
        if os.environ.get("VALIDATE_ARGS") == "True":
            logger.info("Validating inputs")
            app_list_file = os.path.join(self.domain_dir, "app_list.yaml")
            lib_list_file = os.path.join(self.domain_dir, "lib_list.yaml")

            if not utils.is_file(app_list_file) or not utils.is_file(lib_list_file):
                utils.runcmd(
                    f"lopper --werror -f -O {self.domain_dir} {self.sdt} -- baremetal_getsupported_comp_xlnx {self.proc} {self.repo_yaml_path}",
                    cwd = self.domain_dir,
                    log_message = "Getting supported component list",
                    error_message = "Could not get the supported component list"
                )
            proc_data = utils.fetch_yaml_data(app_list_file, "app_list")[self.proc]
            Validation.validate_template_name(
                self.domain_dir, proc_data, self.os, self.app
            )
        return avail_cpu_data[self.proc]

    def build_dir_struct(self):
        """
        Creates the include, lib and libsrc folder inside bsp directory.
        """
        utils.mkdir(self.include_folder)
        utils.mkdir(self.lib_folder)
        utils.mkdir(self.libsrc_folder)
        utils.mkdir(self.sdt_folder)
        if utils.is_dir(os.path.join(utils.get_dir_path(self.sdt), "drivers"), silent_discard=True) and self.app != "versal_plm":
            utils.copy_directory(os.path.join(utils.get_dir_path(self.sdt), "drivers"),
                                 os.path.join((self.sdt_folder), "drivers"))
            logger.info("Found HLS driver within SDT, copying the sources")

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
        dump = utils.discard_dump()
        # no gic mapping is needed for procs other than APU and RPU
        proc_lops_specs_map = {
            "a53": ("cortexa53", "lop-a53-imux", "arm"),
            "a72": ("cortexa72", "lop-a72-imux", "arm"),
            "a78": ("cortexa78", "lop-a78-imux", "arm"),
            "r52": ("cortexr52", "lop-r52-imux", "arm"),
            "r5": ("cortexr5", "lop-r5-imux", "arm"),
            "a9": ("cortexa9", "", "arm"),
            "pmu": ("microblaze-pmu", "", "microblaze"),
            "pmc_riscv": ("microblaze_riscv", "", "microblaze_riscv"),
            "pmc": ("microblaze-plm", "", "microblaze"),
            "psm": ("microblaze-psm", "", "microblaze"),
            "asu": ("microblaze_riscv", "", "microblaze"),
            "microblaze_riscv": ("microblaze_riscv", "", "microblaze_riscv"),
            "microblaze": ("microblaze", "", "microblaze"),
        }
        lops_file = ""
        out_dts_path = os.path.join(self.sdt_folder, f"{self.proc}_baremetal.dts")

        # Save unpruned SDT as it may be used later
        ori_sdt_path = os.path.join(self.sdt_folder, "sdt.dts")
        if self.app != "versal_plm":
            utils.runcmd(f"lopper -f -O {self.domain_dir} --enhanced  --permissive {self.sdt} {ori_sdt_path} > {dump}",
                         log_message = "Generating Flat DT from SDT (Unpruned)",
                         error_message = "Flat DT generation (Unpruned) failed"
            )

        toolchain_file_copy = None
        for val in proc_lops_specs_map.keys():
            if val in self.proc_ip_name:
                if "gcc" in self.compiler:
                    toolchain_file_name = f"{proc_lops_specs_map[val][0]}_toolchain.cmake"
                else:
                    toolchain_file_name = f"{proc_lops_specs_map[val][0]}_{self.compiler}_toolchain.cmake"
                toolchain_file_path = utils.get_high_precedence_path(
                    self.repo_paths_list, "toolchain File", "cmake", "toolchainfiles", toolchain_file_name
                )
                if proc_lops_specs_map[val][1]:
                    lops_file = f"{proc_lops_specs_map[val][1]}.dts"
                toolchain_file_copy = os.path.join(self.domain_dir, toolchain_file_name)
                utils.copy_file(toolchain_file_path, toolchain_file_copy)
                specs_file = utils.get_high_precedence_path(
                    self.repo_paths_list, "Xilinx.spec File", "scripts", "specs", proc_lops_specs_map[val][2], "Xilinx.spec"
                )
                specs_copy_file = os.path.join(self.domain_dir, 'Xilinx.spec')
                utils.copy_file(specs_file, specs_copy_file)
                break

        if self.proc in ["psx_pmc_0", "psx_psm_0", "pmc_0", "asu"]:
            updated_string = '''
set( CMAKE_MACHINE "VersalNet" CACHE STRING "cmake machine" FORCE)
set( CMAKE_SUBMACHINE "VersalNet" CACHE STRING "cmake submachine" FORCE)
'''
            if self.proc == "asu":
                utils.add_newline(
                    toolchain_file_copy,
                    updated_string
                    )
            else:
                utils.replace_line(
                    toolchain_file_copy,
                    'CMAKE_MACHINE "Versal',
                    updated_string
                    )

        if "r5" in self.proc and "ZynqMP" in self.family:
            utils.replace_line(
                toolchain_file_copy,
                'CMAKE_MACHINE "Versal',
                f'set( CMAKE_MACHINE "{self.family}")',
            )

        if "a53" in self.proc and self.proc_mode == "32-bit":
            utils.replace_line(
                toolchain_file_copy,
                'CMAKE_SYSTEM_PROCESSOR "cortexa53"',
                'set( CMAKE_SYSTEM_PROCESSOR "cortexa53-32")',
            )

        if self.family == "Versal_2VE_2VM":
            utils.replace_line(
                toolchain_file_copy,
                'CMAKE_SUBMACHINE "VersalNet',
                f'set( CMAKE_SUBMACHINE "Versal_2VE_2VM" CACHE STRING "cmake submachine" FORCE)',
            )

        if self.family == "SpartanUP":
            utils.add_newline(
                toolchain_file_copy,
                f'set(CMAKE_MACHINE "{self.family}")'
            )

        if self.app:
            utils.add_newline(
                toolchain_file_copy,
                f'set(TEMPLATE "{self.app}")'
            )

        # freertos needs a separate CMAKE_SYSTEM_NAME
        if "freertos" in self.os:
            utils.add_newline(toolchain_file_copy, "set( CMAKE_SYSTEM_NAME FreeRTOS)")
        if os.name == "nt":
            utils.add_newline(toolchain_file_copy, "set( CMAKE_HOST_NAME Windows)")
        else:
            utils.add_newline(toolchain_file_copy, "set( CMAKE_HOST_NAME Linux)")
        # Do the gic pruning in the sdt for APU/RPU.
        if self.sdt != out_dts_path:
            iss_file = utils.find_file("*.iss", utils.get_dir_path(self.sdt))
            domain_dts = None
            if iss_file:
                domain_yaml = os.path.join(self.sdt_folder, "domains.yaml")
                logger.info("Found isospec file inside SDT, Processing")
                # Convert ISS to domain YAML
                utils.runcmd(
                    f"lopper -f -O {self.sdt_folder} --enhanced {self.sdt} -- isospec --audit {iss_file} {domain_yaml}",
                    log_message = "Isospec processing",
                    error_message = "Isospec processing failed"
                )
                if utils.is_file(domain_yaml, silent_discard=True):
                    domain_name = utils.get_domain_name(self.proc, domain_yaml)
                    if domain_name:
                        domain_dts = True
                        if lops_file:
                            utils.runcmd(
                                f"lopper -f -O {self.sdt_folder} --enhanced -i {lops_file} -i lop-ttc-split.dts -t {domain_name} -a domain_access --auto  -x '*.yaml' -i {domain_yaml} {self.sdt} {out_dts_path}",
                                log_message = "Processing domain_yaml",
                                error_message = "Could not process the domain_yaml"
                            )
                        else:
                            utils.runcmd(
                                f"lopper -f -O {self.sdt_folder} --enhanced -t {domain_name} -a domain_access --auto  -x '*.yaml' -i {domain_yaml} {self.sdt} {out_dts_path}",
                                log_message = "Processing domain_yaml",
                                error_message ="Could not process the domain_yaml"
                            )
            if not domain_dts:
                if lops_file:
                    utils.runcmd(
                        f"lopper -f --enhanced -O {self.domain_dir} -i {lops_file} -i lop-ttc-split.dts {self.sdt} {out_dts_path} -- gen_domain_dts {self.proc} {self.app}",
                        log_message = f"Generating Domain-specific DTS for {self.proc}",
                        error_message = f"Domain specific DT generation for {self.proc} failed",
                        verbose_level = 0
                    )
                else:
                    utils.runcmd(
                        f"lopper -f --enhanced -O {self.domain_dir} -i lop-ttc-split.dts {self.sdt} {out_dts_path} -- gen_domain_dts {self.proc}",
                        log_message = f"Generating Domain-specific DTS for {self.proc}",
                        error_message = f"Domain specific DT generation for {self.proc} failed",
                        verbose_level = 0
                    )
        else:
            out_dts_path = self.sdt

        if self.proc_ip_name in ["microblaze", "microblaze_riscv", "asu", "pmc_riscv"]:
            if self.proc_ip_name in ["microblaze_riscv", "asu", "pmc_riscv"] :
                lops_file = "lop-microblaze-riscv.dts"
                processor = "riscv"
            else:
                lops_file = "lop-microblaze.dts"
                processor = "microblaze"

            utils.runcmd(
                f"lopper -f -O {self.domain_dir} --enhanced -i {lops_file} {out_dts_path} > {dump}",
                cwd = self.domain_dir,
                log_message = f"Executing {lops_file}",
                error_message = f"{lops_file} execution failed"
            )
            cflags_file = os.path.join(self.domain_dir, "cflags.yaml")
            avail_cflag_data = utils.fetch_yaml_data(cflags_file, "cflags")
            cflags = avail_cflag_data.get("cflags")


            utils.replace_line(
                toolchain_file_copy,
                f'set( CMAKE_HW_FLAGS "" )',
                f'set( CMAKE_HW_FLAGS "{cflags}" )\n',
            )

            if self.proc_ip_name in ["microblaze_riscv", "asu"]:
                bsp_linkflags = avail_cflag_data.get("linkflags")
                utils.replace_line(
                    toolchain_file_copy,
                    f'set( CMAKE_BSP_HW_LINK_FLAGS "" )',
                    f'set( CMAKE_BSP_HW_LINK_FLAGS "{bsp_linkflags}" )\n',
                )

            relative_libpath = avail_cflag_data.get("libpath")
            vitis_path = os.environ.get("XILINX_VITIS")
            if os.name == "nt":
                libpath = os.path.join(vitis_path, "gnu", processor, "nt", relative_libpath)
            else:
                libpath = os.path.join(vitis_path, "gnu", processor, "lin", relative_libpath)

            libpath = libpath.replace('\\', '/')

            assert utils.is_dir(libpath), f"{processor} compiler library path {libpath} is not found."

            utils.replace_line(
                toolchain_file_copy,
                f'set( CMAKE_COMPILER_LIB_PATH "" )',
                f'set( CMAKE_COMPILER_LIB_PATH "{libpath}" )\n',
            )

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
        if app_name:
            app_yaml_dict = utils.load_yaml(os.path.join(self.get_comp_dir(app_name), "data", f"{app_name}.yaml"))
            if app_yaml_dict.get("extra_compiler_flags"):
                utils.replace_line(
                    toolchain_file,
                    f'set( APPLICATION_SPECIFIC_FLAGS ""  CACHE STRING "Extra Application specific Flags")',
                    f'set( APPLICATION_SPECIFIC_FLAGS "{app_yaml_dict.get("extra_compiler_flags","")}"  CACHE STRING "Extra Application specific Flags")\n',
                )

        if app_name == "zynqmp_fsbl" or self.proc_mode == "32-bit":
            if app_name == "zynqmp_fsbl":
                if "a53" in proc:
                    if self.proc_mode == "64-bit":
                        compiler_flags = "-Os -flto -ffat-lto-objects -DARMA53_64"
                    else:
                        compiler_flags = "-Os -flto -ffat-lto-objects -DARMA53_32 -march=armv7-a -mfpu=vfpv3 -mfloat-abi=hard"
                if "r5" in proc:
                    compiler_flags = "-Os -flto -ffat-lto-objects -DARMR5"
            else:
                compiler_flags = "-march=armv7-a -DARMA53_32 -mfpu=vfpv3 -mfloat-abi=hard"

            utils.add_newline(
                toolchain_file,
                f'set( CMAKE_C_FLAGS "${{TOOLCHAIN_C_FLAGS}} ${{TOOLCHAIN_DEP_FLAGS}} -specs=${{CMAKE_SPECS_FILE}} {compiler_flags} -I${{CMAKE_INCLUDE_PATH}}" CACHE STRING "CFLAGS" FORCE)',
            )
            utils.add_newline(
                toolchain_file,
                f'set( CMAKE_CXX_FLAGS "${{TOOLCHAIN_CXX_FLAGS}} ${{TOOLCHAIN_DEP_FLAGS}} -specs=${{CMAKE_SPECS_FILE}} {compiler_flags} -I${{CMAKE_INCLUDE_PATH}}" CACHE STRING "CXX FLAGS" FORCE)',
            )
            utils.add_newline(
                toolchain_file,
                f'set( CMAKE_ASM_FLAGS "${{TOOLCHAIN_ASM_FLAGS}} ${{TOOLCHAIN_DEP_FLAGS}} -specs=${{CMAKE_SPECS_FILE}} {compiler_flags} -I${{CMAKE_INCLUDE_PATH}}" CACHE STRING "ASM FLAGS" FORCE)',
            )
        return compiler_flags

def lop_create_target(lop_cmds):
    lop_file = f'''
/dts-v1/;
/ {{
        compatible = "system-device-tree-v1,lop";
        lops {{
                lop_0 {{
                        compatible = "system-device-tree-v1,lop,load";
                        load = "assists/baremetalconfig_xlnx.py";
                }};
                lop_1 {{
                        compatible = "system-device-tree-v1,lop,load";
                        load = "assists/baremetal_bspconfig_xlnx";
                }};
                lop_2 {{
                        compatible = "system-device-tree-v1,lop,load";
                        load = "assists/bmcmake_metadata_xlnx.py";
                }};
                lop_3 {{
                        compatible = "system-device-tree-v1,lop,load";
                        load = "assists/baremetal_xparameters_xlnx.py";
                }};
    '''
    for index,cmd in enumerate(lop_cmds, 4):
        lop_file += f'''
                lop_{index} {{
                    compatible = "system-device-tree-v1,lop,assist-v1";
                    node = "/";
                    outdir = "{cmd[0]}";
                    id = "{cmd[1]}";
                 options = "{cmd[2]}";
                }};
        '''
    lop_file += f'''
        }};
    }};
    '''
    return lop_file.replace('\\', '/')

@log_time
def create_domain(args):
    """
    Function that uses the above Domain class to create the baremetal domain.
    Args:
        args (dict): Takes all the user inputs in a dictionary.
    """

    # Initialize the Domain class
    obj = Domain(args)
    logger.info( "Starting domain creation" )

    # Create the bsp directory structure.
    obj.build_dir_struct()

    # Create the Domain specific sdt and the toolchain file.
    obj.sdt, obj.toolchain_file, obj.specs_file = obj.toolchain_intr_mapping()

    # Common cmake variables to support cmake build infra.
    cmake_paths_append = f" -DCMAKE_LIBRARY_PATH={obj.lib_folder} \
-DCMAKE_INCLUDE_PATH={obj.include_folder} \
-DCMAKE_MODULE_PATH={obj.domain_dir} \
-DCMAKE_TOOLCHAIN_FILE={obj.toolchain_file} \
-DCMAKE_VERBOSE_MAKEFILE=ON"

    if "gcc" in obj.compiler:
        cmake_paths_append += f" -DCMAKE_SPECS_FILE={obj.specs_file} "
    else:
        cmake_paths_append += " -Wno-dev "

    # Copy the standalone bsp src file.
    os_dir_path = obj.get_comp_dir("standalone")
    os_srcdir = os.path.join(os_dir_path, "src")
    bspsrc = os.path.join(obj.libsrc_folder, "standalone", "src")
    bspcomsrc = os.path.join(obj.libsrc_folder, "standalone", "src", "common")
    utils.copy_directory(os_srcdir, bspsrc)
    obj.os_info['standalone'] = {'path': os_dir_path}

    # Create lops file
    config_lops_file = os.path.join(obj.domain_dir, "lop-config.dts")
    lop_cmds = []
    lop_cmds.append([bspsrc, "module,baremetal_bspconfig_xlnx", f"{obj.proc} {os_srcdir} {obj.app}"])
    lop_cmds.append([bspcomsrc, "module,bmcmake_metadata_xlnx", f"{obj.proc} {os_srcdir} hwcmake_metadata {obj.repo_yaml_path}"])

    # Copy cmake file that contains cmake utility APIs to a common location.
    find_common_cmake_path = utils.get_high_precedence_path(
            obj.repo_paths_list, "Findcommon.cmake", "cmake", "Findcommon.cmake"
        )
    find_common_domain_path = os.path.join(obj.domain_dir, "Findcommon.cmake")
    utils.copy_file(
        find_common_cmake_path,
        find_common_domain_path,
        silent_discard=False,
    )

    # Below replace command is meant only for maintaining the backward compatibility
    # with 2023.2
    utils.replace_line(
        find_common_domain_path,
        'option(NON_YOCTO "Non Yocto embeddedsw FLOW" OFF)',
        'option(NON_YOCTO "Non Yocto embeddedsw FLOW" ON)',
    )

    """
    Generate metadata for driver compilation. Metadata includes driver
    list available in sdt and a cmake file that enables the generation of
    _g.c files for available drivers in parallel.
    """
    utils.runcmd(
        f"lopper -O {obj.libsrc_folder} -f {obj.sdt} -- baremetaldrvlist_xlnx {obj.proc} {obj.repo_yaml_path}",
        log_message = "Generating driver list",
        error_message = "Could not generate the Driver list, baremetaldrvlist_xlnx failed"
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
            logger.error(f"Couldnt find the src directory for {drv}. {drv_path} doesnt exist.")
            sys.exit(1)
        cmake_drv_path_list += f"{drv_path};"
        lop_cmds.append([drvsrc, "module,baremetalconfig_xlnx", f"{obj.proc} {drv_srcdir}"])

    ip_drv_map_file = os.path.join(obj.libsrc_folder, "ip_drv_map.yaml")
    ip_drv_map = utils.load_yaml(ip_drv_map_file)
    for ip,data in ip_drv_map.items():
        driver = data[1]
        if driver != "None":
            drv_path = obj.get_comp_dir(driver, obj.sdt_folder)
            if not drv_path:
                logger.error(f"Couldnt find the src directory for {drv}. {drv_path} doesnt exist.")
                sys.exit(1)
            obj.drv_info[ip] = {'driver': driver,
                                'ip_name': data[0],
                                'path' : drv_path}
        else:
            obj.drv_info[ip] = "None"


    """
    Create a dictionary that will contain the current status of the domain.
    This data will later be used during bsp configuartion, bsp build and
    app creation stages.
    """
    data = {
        "sdt": utils.get_rel_path(obj.sdt, obj.domain_dir),
        "family": obj.family,
        "path": obj.domain_dir,
        "config": "default",
        "os": obj.os,
        "os_info": obj.os_info,
        "os_config": {},
        "toolchain_file": utils.get_rel_path(obj.toolchain_file, obj.domain_dir),
        "specs_file": utils.get_rel_path(obj.specs_file, obj.domain_dir),
        "proc": obj.proc,
        "proc_config": {},
        "template": obj.app,
        "compiler": obj.compiler,
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
            outfile = f"{lib}Example.cmake"
            lop_cmds.append([dstdir, "module,bmcmake_metadata_xlnx", f"{obj.proc} {srcdir} hwcmake_metadata {obj.repo_yaml_path}"])
            if ("xilpm" in lib) and ("ZynqMP" in obj.family):
                dstdir = os.path.join(obj.libsrc_folder, lib, "src", "zynqmp", "client", "common")
                ori_sdt_path = os.path.join(obj.sdt_folder, "sdt.dts")
                #TODO: Update bsp.yaml before running the below command so that,
                #      the default properties of lib get updated successfully.
                lopper_cmd = f"lopper -O {dstdir} -f {ori_sdt_path} --  generate_config_object pm_cfg_obj.c {obj.proc}"
                utils.runcmd(
                    lopper_cmd, cwd = dstdir,
                    log_message = "generate_config_object for PM use case",
                    error_message = "Failed to generate the config object for PM use cases"
                )

    build_metadata = os.path.join(obj.libsrc_folder, "build_configs", "gen_bsp")
    utils.mkdir(build_metadata)

    lop_cmds.append([obj.include_folder, "module,baremetal_xparameters_xlnx", f"{obj.proc} {obj.repo_yaml_path} {obj.app}"])
    utils.write_into_file(config_lops_file, lop_create_target(lop_cmds))
    utils.runcmd(
        f"lopper -O {obj.domain_dir} -i {config_lops_file} -f {obj.sdt}",
        log_message = "Generating required metadata from domain specific DT",
        error_message = "Overall metadata generation failed",
        verbose_level = 0
    )

    # Copy the common cmake meta-data file to domain directory so that other modules can consume it
    cmake_metadata = os.path.join(obj.libsrc_folder, "standalone", "src", "common", "StandaloneExample.cmake")
    utils.copy_file(cmake_metadata, f"{obj.domain_dir}/Findcommonmeta.cmake")
    # Copy the actual drivers cmake file in the libsrc folder.
    # This is to compile all the available driver sources.
    libxil_cmake = utils.get_high_precedence_path(
        obj.repo_paths_list, "Driver compilation CMake File", "XilinxProcessorIPLib", "drivers", "CMakeLists.txt"
    )
    utils.copy_file(libxil_cmake, f"{obj.libsrc_folder}/")

    bsp_libsrc_cmake_subdirs = "libsrc standalone " + " ".join(lib_list)

    # Create new CMakeLists.txt
    cmake_file = os.path.join(obj.domain_dir, "CMakeLists.txt")
    cmake_file_cmds = f'''
cmake_minimum_required(VERSION 3.15)
project(bsp)
find_package(common)
find_package(commonmeta)
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${{CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}})
    set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${{CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES}})
endif()

ADD_DEFINITIONS(-c ${{proc_extra_compiler_flags}})
include_directories(${{CMAKE_BINARY_DIR}}/include)
include_directories(${{CMAKE_INCLUDE_PATH}})

if("${{CMAKE_SYSTEM_NAME}}" STREQUAL "FreeRTOS")
add_compile_definitions(FREERTOS_BSP)
endif()
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
cmake_language(DEFER DIRECTORY ${{CMAKE_SOURCE_DIR}} CALL _my_hook_end_of_configure())
function(_my_hook_end_of_configure)
    set(SUBDIR_LIST "ALL" CACHE STRING "sub dir list" FORCE)
endfunction(_my_hook_end_of_configure)
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

    if "microblaze" in obj.proc_ip_name:
        cmake_config = lib_obj.get_default_lib_params(build_metadata, ["cmake"])
        if cmake_config['cmake'].get('CMAKE_MACHINE', {}):
            obj.family = cmake_config['cmake']['CMAKE_MACHINE']['value']
            utils.update_yaml(obj.domain_config_file, "domain", "family", obj.family)

    if "a53" in obj.proc and obj.proc_mode == "32-bit":
        utils.update_yaml(obj.domain_config_file, "domain", "mode", obj.proc_mode)

    utils.update_yaml(obj.domain_config_file, "domain", "os_config", os_config)
    utils.update_yaml(obj.domain_config_file, "domain", "proc_config", proc_config)
    utils.update_yaml(obj.domain_config_file, "domain", "os_info", obj.os_info)
    utils.update_yaml(obj.domain_config_file, "domain", "lib_info", lib_obj.lib_info)

    # Remove the metadata files that are no longer needed.
    utils.remove(config_lops_file)
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
    if "gcc" in obj.compiler:
        cmd_to_add = "Add: [-Wno-unknown-warning-option, -U__linux__, -U__clang__]"
    else:
        cmd_to_add = "Add: [-Wno-unknown-warning-option, -U__linux__]"
    clangd_ignore_content = f'''
CompileFlags:
    {cmd_to_add}
    Remove: [-m*, -f*]
'''
    clangd_ignore_file = os.path.join(obj.libsrc_folder, ".clangd")
    utils.write_into_file(clangd_ignore_file, clangd_ignore_content)

    # Success prints if everything went well till this point
    if utils.is_file(obj.domain_config_file):
        logger.info(f"Successfully created Domain at {obj.domain_dir}")

def main(arguments=None):
    parser = argparse.ArgumentParser(
        description="Create bsp for the given sdt, os, processor and template app",
        usage='use "empyro create_bsp --help" for more information',
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
            - spartanup_plm
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
        "-m",
        "--mode",
        action="store",
        default="64-bit",
        help="Specify the processor mode (Default: 64-bit)",
        choices=["32-bit", "64-bit"],
    )
    parser.add_argument(
        "-v",
        "--verbose",
        action="count",
        default=0,
        help='Increase output verbosity'
    )
    parser.add_argument(
        "-c",
        "--compiler",
        action="store",
        default="gcc",
        help="Specify the compiler (Default: gcc)",
        choices=["gcc", "armclang", "armcc", "iar"],
    )

    args = vars(parser.parse_args(arguments))
    utils.setup_log(args["verbose"])
    create_domain(args)

if __name__ == "__main__":
    main()
