###############################################################################
# Copyright (C) 2011 - 2022 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
###############################################################################
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 2.0      adk    12/10/13 Updated as per the New Tcl API's
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
    if {$swproj == 1} {
            set inc_file_lines {xspips.h spips_header.h}
        }
        return $inc_file_lines
}

proc gen_src_files {swproj mhsinst} {
  if {$swproj == 0} {
    return ""
  }
  if {$swproj == 1} {

      set inc_file_lines {examples/xspips_selftest_example.c data/spips_header.h}

      return $inc_file_lines
  }
}

proc gen_testfunc_def {swproj mhsinst} {
  return ""
}

proc gen_init_code {swproj mhsinst} {

    if {$swproj == 0} {
        return ""
    }
    if {$swproj == 1} {
        return ""
    }

}

proc gen_testfunc_call {swproj mhsinst} {

    if {$swproj == 0} {
        return ""
    }

    set ipname [common::get_property NAME $mhsinst]
    set deviceid [::hsi::utils::get_ip_param_name $mhsinst "DEVICE_ID"]
    set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
    if { $stdout == "" || $stdout == "none" } {
       set hasStdout 0
    } else {
       set hasStdout 1
    }

    set testfunc_call ""

  if {${hasStdout} == 0} {

      append testfunc_call "

   {
      int Status;

      Status = SpiPsSelfTestExample(${deviceid});

   }"


  } else {

      append testfunc_call "

   {
      int Status;

      print(\"\\r\\n Running SpiPsSelfTestExample() for ${ipname}...\\r\\n\");

      Status = SpiPsSelfTestExample(${deviceid});

      if (Status == 0) {
         print(\"SpiPsSelfTestExample PASSED\\r\\n\");
      }
      else {
         print(\"SpiPsSelfTestExample FAILED\\r\\n\");
      }
   }"

  }

  return $testfunc_call
}
