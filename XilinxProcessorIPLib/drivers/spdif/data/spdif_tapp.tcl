###############################################################################
# Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
# Uses $XILINX_EDK/bin/lib/xillib_sw.tcl
#
# - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - - - -
# Software Project Types (swproj):
# 0 : MemoryTest - Calls basic  memorytest routines from common driver dir
# 1 : PeripheralTest - Calls any existing polled_example and/or selftest
# - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - - - -
#
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
# TCL Procedures:
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

proc gen_include_files {swproj mhsinst} {
  if {$swproj == 0} {
    return ""
  }
  if {$swproj == 1} {
    set inc_file_lines {xspdif_header.h}
    return $inc_file_lines
  }
}

proc gen_src_files {swproj mhsinst} {
 if {$swproj == 0} {
    return ""
  }
  if {$swproj == 1} {
   set inc_file_lines {examples/xspdif_selftest_example.c data/xspdif_header.h}
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

  if {${hasStdout} == 0} {

      append testfunc_call "

   {
      int Status;

      Status = SpdifSelfTestExample(${deviceid});

   }"
  } else {

      append testfunc_call "

   {
      int Status;


      print(\"\\r\\nRunning SpdifSelfTestExample() for ${ipname}...\\r\\n\");

      Status = SpdifSelfTestExample(${deviceid});

      if (Status == 0) {
         print(\"Spdif Self Test Example PASSED\\r\\n\");
      }
      else {
         print(\"Spdif Self Test Example FAILED\\r\\n\");
      }
   }"
  }
  return $testfunc_call
}
