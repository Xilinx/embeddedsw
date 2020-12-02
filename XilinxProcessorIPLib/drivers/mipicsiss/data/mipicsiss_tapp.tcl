###############################################################################
# Copyright (C) 2016 - 2020 Xilinx, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
#
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
    set inc_file_lines {xcsiss_header.h}
    return $inc_file_lines
  }
}

proc gen_src_files {swproj mhsinst} {
  if {$swproj == 0} {
    return ""
  }
  if {$swproj == 1} {
    set inc_file_lines {examples/xcsiss_selftest_example.c data/xcsiss_header.h}
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

      Status = CsiSs_SelfTestExample(${deviceid});

   }"
  } else {

      append testfunc_call "

   {
      int Status;


      print(\"\\r\\nRunning CsiSsSelfTestExample() for ${ipname}...\\r\\n\");

      Status = CsiSs_SelfTestExample(${deviceid});

      if (Status == 0) {
         print(\"CsiSs_SelfTestExample PASSED\\r\\n\");
      }
      else {
         print(\"CsiSs_SelfTestExample FAILED\\r\\n\");
      }
   }"
  }

  return $testfunc_call
}
