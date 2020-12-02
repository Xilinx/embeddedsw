###############################################################################
# Copyright (C) 2004 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 9.0     adk    12/10/13 Updated as per the New Tcl API's
##############################################################################

## @BEGIN_CHANGELOG EDK_I_SP1
##
##  - Added support for generation of multiple applications.
##    All TCL procedures are now required to have a software
##    project type as its first argument
##    
## @END_CHANGELOG

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
    set inc_file_lines {xhwicap.h hwicap_header.h}
    return $inc_file_lines
  }
}

proc gen_src_files {swproj mhsinst} {
  if {$swproj == 0} {
    return ""
  }
  if {$swproj == 1} {
    set inc_file_lines {examples/xhwicap_testapp_example.c data/hwicap_header.h}
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
 
  set ipname [common::get_property NAME  $mhsinst]
  set deviceid [::hsi::utils::get_ip_param_name $mhsinst "DEVICE_ID"]
  set stdout [common::get_property CONFIG.STDOUT [get_os]]
  if { $stdout == "" || $stdout == "none" } {
       set hasStdout 0
  } else {
       set hasStdout 1
  }

  set testfunc_call ""

  if {${hasStdout} == 0} {

      append testfunc_call "

   {
      XStatus status;
                        
      status = HwIcapTestAppExample(${deviceid});

   }"
  } else {

      append testfunc_call "

   {
      XStatus status;
            
      
      print(\"\\r\\n Running HwIcapTestAppExample() for ${ipname}...\\r\\n\");
      
      status = HwIcapTestAppExample(${deviceid});
      
      if (status == 0) {
         print(\"HwIcapTestAppExample PASSED\\r\\n\");
      }
      else {
         print(\"HwIcapTestAppExample FAILED\\r\\n\");
      }
   }"
  }

  return $testfunc_call
}

