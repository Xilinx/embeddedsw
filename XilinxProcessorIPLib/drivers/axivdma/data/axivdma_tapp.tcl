###############################################################################
# Copyright (C) 2005 - 2021 Xilinx, Inc.  All rights reserved.
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
    set inc_file_lines {axivdma_header.h}
    return $inc_file_lines
  }
}

proc gen_src_files {swproj mhsinst} {
  if {$swproj == 0} {
    return ""
  }
  if {$swproj == 1} {
    set inc_file_lines {examples/xaxivdma_example_selftest.c data/axivdma_header.h}
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
      int status;

      status = AxiVDMASelfTestExample(${deviceid});

   }"
  } else {

      append testfunc_call "

   {
      int status;


      print(\"\\r\\n Running AxiVDMASelfTestExample() for ${ipname}...\\r\\n\");

      status = AxiVDMASelfTestExample(${deviceid});

      if (status == 0) {
         print(\"AxiVDMASelfTestExample PASSED\\r\\n\");
      }
      else {
         print(\"AxiVDMASelfTestExample FAILED\\r\\n\");
      }
   }"
  }

  return $testfunc_call
}
