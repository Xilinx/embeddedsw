###############################################################################
# Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 1.0      sa     04/05/17 First release
##############################################################################

# -----------------------------------------------------------------
# Software Project Types (swproj):
#   0 : MemoryTest - Calls basic  memorytest routines from common driver dir
#   1 : PeripheralTest - Calls any existing polled_example and/or selftest
# -----------------------------------------------------------------

# -----------------------------------------------------------------
# TCL Procedures:
# -----------------------------------------------------------------

proc gen_include_files {swproj mhsinst} {
  set inc_file_lines ""

  if {$swproj == 0} {
    return ""
  }
  if {$swproj == 1} {
    set inc_file_lines {xtmr_inject.h tmr_inject_header.h}
    return $inc_file_lines
  }
}

proc gen_src_files {swproj mhsinst} {
    if {$swproj == 0} {
	return ""
    }

    if {$swproj == 1} {
	set inc_file_lines {examples/xtmr_inject_selftest_example.c data/tmr_inject_header.h}
	return $inc_file_lines
    }
    return ""
}

proc gen_testfunc_def {swproj mhsinst} {
  return ""
}

proc gen_init_code {swproj mhsinst} {
    return ""
}

proc gen_testfunc_call {swproj mhsinst} {
    set ipname [common::get_property NAME $mhsinst]
    set iftmr_injectintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
    set testfunc_call ""

    if {$swproj == 0} {
	return $testfunc_call
    }

    set deviceid [::hsi::utils::get_ip_param_name $mhsinst "DEVICE_ID"]

    append testfunc_call "

   {
      int status;

      print(\"\\r\\nRunning TMR_InjectSelfTestExample() for ${ipname}...\\r\\n\");
      status = TMR_InjectSelfTestExample(${deviceid});
      if (status == 0) {
         print(\"TMR_InjectSelfTestExample PASSED\\r\\n\");
      }
      else {
         print(\"TMR_InjectSelfTestExample FAILED\\r\\n\");
      }
   }"

  return $testfunc_call
}
