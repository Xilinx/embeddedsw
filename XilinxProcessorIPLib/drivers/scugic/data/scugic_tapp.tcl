###############################################################################
# Copyright (C) 2011 - 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
##############################################################################
##
## MODIFICATION HISTORY:
## Ver   Who  Date     Changes
## ----- ---- -------- ----------------------------------------------------
## 1.00a sdm  05/24/11 First release
## 3.0   adk  12/10/13 Updated as per the New Tcl API's
##############################################################################

# Uses $XILINX_EDK/bin/lib/xillib_sw.tcl
# -----------------------------------------------------------------
# Software Project Types (swproj):
#   0 : MemoryTest - Calls basic  memorytest routines
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

        set inc_file_lines {xscugic.h xil_exception.h scugic_header.h}
	return $inc_file_lines
    }
}

proc gen_src_files {swproj mhsinst} {
  if {$swproj == 0} {
    return ""
  }
  if {$swproj == 1} {
    set inc_file_lines {examples/xscugic_tapp_example.c data/scugic_header.h}
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

  set ipname [common::get_property IP_NAME $mhsinst]
  set decl "   static XScuGic ${ipname};"
  set deviceid [::hsi::utils::get_ip_param_name $mhsinst "DEVICE_ID"]
  set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
  set sw_proc_handle [hsi::get_sw_processor]
  set hw_proc_handle [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc_handle] ]
  set proctype [common::get_property IP_NAME $hw_proc_handle]
  if {([string compare -nocase $proctype "ps7_cortexa9"] == 0)||
	    (([string compare -nocase $proctype "psu_cortexa53"] == 0)&&([string compare -nocase $ipname "psu_acpu_gic"] == 0))||
	    (([string compare -nocase $proctype "psu_cortexr5"] == 0)&&([string compare -nocase $ipname "psu_rcpu_gic"] == 0)) ||
	    (([string compare -nocase $proctype "psv_cortexa72"] == 0)&&([string compare -nocase $ipname "psv_acpu_gic"] == 0))||
	    (([string compare -nocase $proctype "psv_cortexr5"] == 0)&&([string compare -nocase $ipname "psv_rcpu_gic"] == 0))} {

	 if { $stdout == "" || $stdout == "none" } {
	     set hasStdout 0
	  } else {
	     set hasStdout 1
	  }

	  set intcvar intc

	  set testfunc_call ""

	  if {${hasStdout} == 0} {

	      append testfunc_call "

	   {
	      int Status;

	      Status = ScuGicSelfTestExample(${deviceid});
	      Status = ScuGicInterruptSetup(&${intcvar}, ${deviceid});

	   }"


	  } else {

	      append testfunc_call "

	   {
	      int Status;

	      print(\"\\r\\n Running ScuGicSelfTestExample() for ${ipname}...\\r\\n\");

	      Status = ScuGicSelfTestExample(${deviceid});

	      if (Status == 0) {
	         print(\"ScuGicSelfTestExample PASSED\\r\\n\");
	      }
	      else {
	         print(\"ScuGicSelfTestExample FAILED\\r\\n\");
	      }
	   }"

	          append testfunc_call "

	   {
	       int Status;

	       Status = ScuGicInterruptSetup(&${intcvar}, ${deviceid});
	       if (Status == 0) {
	          print(\"ScuGic Interrupt Setup PASSED\\r\\n\");
	       }
	       else {
	         print(\"ScuGic Interrupt Setup FAILED\\r\\n\");
	      }
	   }"


	  }
  return $testfunc_call

  }


}
