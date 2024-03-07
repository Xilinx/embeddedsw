# Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
"""
This module acts as a supporting module to validate required hardware
present for a given component. It reads the component yaml dependencies
and creates a lops file which calls the lopper validate assist.
It doesnt have any main() function and running this module independently
is not intended.
"""

import os
import utils
from repo import Repo

def lop_create_target(lop_cmds):
    lop_file = f'''
/dts-v1/;
/ {{
        compatible = "system-device-tree-v1,lop";
        lops {{
                lop_0 {{
                        compatible = "system-device-tree-v1,lop,load";
                        load = "assists/baremetal_validate_comp_xlnx.py";
                }};
    '''
    for index,cmd in enumerate(lop_cmds, 1):
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

class ValidateHW(Repo):
    """
    This class contains attributes and functions that help in validating
    esw component for a required hardware is present or not.
    """

    def __init__(
        self, domain_path, proc, bsp_os, sdt, name, repo_info
    ):
        super().__init__(repo_yaml_path= repo_info)
        self.domain_dir = domain_path
        self.proc = proc
        self.os = bsp_os
        self.name = name
        self.sdt = sdt

    def validate_hw(self):
        app_dir = self.get_comp_dir(self.name)
        yaml_file = os.path.join(app_dir, "data", f"{self.name}.yaml")
        if utils.is_file(yaml_file, silent_discard=True):
            schema = utils.load_yaml(yaml_file)
            comp_list = []
            """
            Use Cases:
            1. BSP
            2. ADD LIB
            3. Create APP
            """
            if schema:
                if self.os == "freertos":
                    comp_list.append("freertos10_xilinx")
                if schema.get("depends") or schema.get("required_mem") or ("library" in schema.get("type")):
                    comp_list.append(self.name)
                comp_list += list(schema.get("depends_libs",{}).keys())
                if 'xiltimer' in comp_list:
                    comp_list.remove('xiltimer')

            lop_cmds = []
            config_lops_file = os.path.join(self.domain_dir, "lop-config.dts")
            for comp in comp_list:
                comp_path = self.get_comp_dir(comp)
                comp_srcdir = os.path.join(comp_path, "src")
                lop_cmds.append([self.domain_dir, "module,baremetal_validate_comp_xlnx", f"{self.proc} {comp_srcdir} {self.repo_yaml_path}"])

            if comp_list:
                utils.write_into_file(config_lops_file, lop_create_target(lop_cmds))
                utils.runcmd(
                    f"lopper -i {config_lops_file} -f {self.sdt}",
                    cwd = self.domain_dir
                )
                utils.remove(config_lops_file)
