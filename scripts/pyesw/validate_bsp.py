# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
"""
This module facilitates the validation of a created BSP with respect to a
template application.
"""

import argparse, textwrap
from library_utils import Library
from build_bsp import BSP
import os, sys
import utils


class Validation(BSP, Library):
    """
    This class contains attributes and functions to validate the given bsp
    w.r.t. the user input template application. This inherits BSP class to
    inherit all the domain specific data and Repo class to get the
    embeddedsw related paths.
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
        self.template = args.get("template", self.template)
        self.proc_data = self._get_template_lib_data(args.get("app_list_yaml"))
        self.app_data = self.proc_data[self.os]
        self.supported_app_list = list(self.app_data.keys())
        self.avail_libs = list(self.lib_config.keys())

    def _get_template_lib_data(self, app_list_file):
        """
        To validate bsp for a given template, the template specific data for
        the given os,proc combination is needed. This function fetches that
        data either from a user provided file or creates the metadata as per
        user entries during domain creation and returns that.

        Args:
            app_list_file (str): If user has passed its own file during validation

        Returns:
            proc_data (dict): Returns the data read from the app specific config file.
        """
        if app_list_file:
            app_list_file = utils.get_abs_path(app_list_file)
            if not utils.is_file(app_list_file):
                print(f"[ERROR]: File {app_list_file} doesn't exist.")
                sys.exit(1)
        else:
            app_list_file = os.path.join(self.domain_path, "app_list.yaml")
            if not utils.is_file(app_list_file):
                utils.runcmd(
                    f"lopper --werror -f -O {self.domain_path} {self.sdt} -- baremetal_getsupported_comp_xlnx {self.proc} {self.repo_yaml_path}"
                )
        proc_data = utils.fetch_yaml_data(app_list_file, "app_list")[self.proc]
        return proc_data

    @staticmethod
    def validate_template_name(domaindir, proc_data, bsp_os, app):
        """
        This function verifies the template name passed by the user. It checks
        if the name is valid for the given os and proc combination. If not,
        it raises the suitable assertion. This is being used during domain
        creation as well (when domain needs to be created for a particular
        template). Hence, a static function in nature.

        Args:
            | domaindir (str): domain path that contains all the domain specific data
            | proc_data (dict): App specific data read during initialization
            | bsp_os (str): os used during domain creation
            | app (str): template name to be validated

        """
        app_list_yaml_path = os.path.join(domaindir, "app_list.yaml")
        if app:
            app_data = proc_data[bsp_os]
            supported_app_list = list(app_data.keys())
            os_found = ""
            for key, value in proc_data.items():
                if app in proc_data[key].keys():
                    os_found = key
                    break

            if os_found and app not in supported_app_list:
                print(
                    f"[ERROR]: {app} is not a valid template name. It needs bsp with {os_found} os. Valid templates for the BSP are: {supported_app_list}"
                )
                sys.exit(1)

            if app not in supported_app_list:
                print(
                    f"[ERROR]: {app} is not a valid template name. Valid templates are: {supported_app_list}"
                )
                sys.exit(1)

    def validate_template_for_bsp(self):
        """
        This fucntion validates the library dependency of the passed template
        within the bsp. If the required libs are not available in the bsp, it
        throws the suitable assertion.
        """
        Validation.validate_template_name(
            self.domain_path, self.proc_data, self.os, self.template
        )
        required_libs = self.app_data[self.template].get("depends_libs", [])
        diff_libs = []
        for ele in required_libs:
            if not self.is_valid_lib(ele):
                continue
            if ele not in self.avail_libs:
                diff_libs.append(ele)

        if diff_libs:
            print(
                f"[ERROR]: {self.template} is not a valid template name for the given BSP. BSP is missing {diff_libs}."
            )
            self.get_valid_template_list()
            sys.exit(1)

    def get_valid_template_list(self):
        """
        This function provides the list of templates that can built using the
        passed bsp.
        """
        lib_app_dict = {}

        for app in self.supported_app_list:
            app_depends_lib = self.app_data[app].get("depends_libs", ["NA"])
            for entry in app_depends_lib:
                if not self.is_valid_lib(entry):
                    continue
                if app not in lib_app_dict.get(entry, []):
                    try:
                        lib_app_dict[entry] += [app]
                    except KeyError:
                        lib_app_dict[entry] = [app]

        template_possible = []

        for ele in self.avail_libs:
            confirmed = False
            for entry in lib_app_dict[ele]:
                required_libs = self.app_data[entry].get("depends_libs", [])
                for lib in required_libs:
                    if not self.is_valid_lib(lib):
                        continue
                    if lib not in self.avail_libs:
                        confirmed = False
                        break
                    else:
                        confirmed = True

                if confirmed and entry not in template_possible:
                    template_possible += [entry]

        print(f"Available Templates for the given BSP are {template_possible}.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Use this script to validate the given BSP for a given template",
        usage='use "python %(prog)s --help" for more information',
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
        "-g",
        "--get_apps",
        action="store",
        default=False,
        help="Give True if you want to list down valid app names for the bsp.",
    )
    parser.add_argument(
        "-f",
        "--app_list_yaml",
        action="store",
        default=None,
        help="Provide an external app_list.yaml file if created separately",
    )
    parser.add_argument(
        "-r",
        "--repo_info",
        action="store",
        help="Specify the .repo.yaml absolute path to use the set repo info",
        default='.repo.yaml',
    )

    args = vars(parser.parse_args())
    obj = Validation(args)
    if args.get("get_apps"):
        obj.get_valid_template_list()
    elif args.get("template"):
        obj.validate_template_for_bsp()
    else:
        assert False, "Either use get_apps option or provide a template name."
