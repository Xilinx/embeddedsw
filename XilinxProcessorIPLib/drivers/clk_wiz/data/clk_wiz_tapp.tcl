###############################################################################
# Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
#
# MODIFICATION HISTORY:
# Ver Who Date     Changes
# -------- ------ -------- ------------------------------------
# 1.0 ram 02/15/16 Initial version for Clock Wizard
# 1.1 siv 08/17/16 Added support for Zynq MPSoC and 64-bit addressing
# 1.3 sd  05/04/20 Added check for interrupts for the interrupt example
##############################################################################

## @BEGIN_CHANGELOG EDK_I
##
##  - include header files
##
## @END_CHANGELOG

## @BEGIN_CHANGELOG EDK_H
##
##  - Initial Revision
##
## @END_CHANGELOG

##############################################################################
# Uses $XILINX_EDK/bin/lib/xillib_sw.tcl

# -----------------------------------------------------------------
# Software Project Types (swproj):
#   0 : MemoryTest - Calls basic  memorytest routines from common driver dir
#   1 : PeripheralTest - Calls any existing polled_example and/or selftest
# -----------------------------------------------------------------

# -----------------------------------------------------------------
# TCL Procedures:
# -----------------------------------------------------------------

proc gen_include_files {swproj mhsinst} {
  if {$swproj == 0} {
    return ""
  }
  set isintr [::hsm::utils::is_ip_interrupting_current_proc $mhsinst]
  if {$swproj == 1} {
    if {$isintr == 0} {
      return ""
    }
    set inc_file_lines {clk_wiz_header.h}
    return $inc_file_lines
  }
}

proc gen_src_files {swproj mhsinst} {
  set inc_file_lines ""
  if {$swproj == 0} {
    return ""
  }
  set isintr [::hsm::utils::is_ip_interrupting_current_proc $mhsinst]
  if {$swproj == 1} {
    if {$isintr == 1} {
	set inc_file_lines {examples/xclk_wiz_intr_example.c data/clk_wiz_header.h}
    }
    return $inc_file_lines
  }
}

proc gen_testfunc_def {swproj mhsinst} {
  return ""
}

proc gen_init_code {swproj mhsinst} {
  return ""
}

proc gen_testfunc_call {swproj mhsinst} {

  if {$swproj == 0} {
    return ""
  }

  set ipname [get_property NAME $mhsinst]
  set deviceid [::hsi::utils::get_ip_param_name $mhsinst "DEVICE_ID"]
  set stdout [get_property CONFIG.STDOUT [get_os]]
   if { $stdout == "" || $stdout == "none" } {
	set hasStdout 0
   } else {
	set hasStdout 1
   }

  set testfunc_call ""
  set isintr [::hsm::utils::is_ip_interrupting_current_proc $mhsinst]
  if {$isintr == 0} {
	return $testfunc_call
  }

  if {${hasStdout} == 0} {

	append testfunc_call "

   {
      int Status;

	Status = ClkWiz_IntrExample(${deviceid});

   }"
  } else {

	append testfunc_call "

   {
	int Status;


	print(\"\\r\\nRunning ClkWiz_Intr Example() for ${ipname}...\\r\\n\");

	Status = ClkWiz_IntrExample(${deviceid});

      if (Status == 0) {
	print(\"ClkWiz_IntrExample PASSED\\r\\n\");
      }
      else {
	print(\"ClkWiz_IntrExample FAILED\\r\\n\");
      }
   }"
  }

  return $testfunc_call
}
