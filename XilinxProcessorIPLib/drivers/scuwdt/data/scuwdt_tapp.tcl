###############################################################################
# Copyright (C) 2011 - 2022 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
###############################################################################
#
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
            set inc_file_lines {xscuwdt.h scuwdt_header.h}
    }
        return $inc_file_lines
}

proc gen_src_files {swproj mhsinst} {
  if {$swproj == 0} {
    return ""
  }
  if {$swproj == 1} {

      set inc_file_lines {examples/xscuwdt_intr_example.c data/scuwdt_header.h}

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

      set ipname [get_property NAME $mhsinst]
      set decl "   static XScuWdt ${ipname};"
      set inc_file_lines $decl
      return $inc_file_lines

    }

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

    set isintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
    set intcvar intc


    set testfunc_call ""

  if {${hasStdout} == 0} {

	if {$isintr == 1} {
        set intr_id "XPAR_${ipname}_INTR"
	set intr_id [string toupper $intr_id]

      append testfunc_call "

   {
      int Status;
      Status = ScuWdtIntrExample(&${intcvar}, &${ipname}, \\
                                 ${deviceid}, \\
                                 ${intr_id});
   }"

   }


  } else {

	if {$isintr == 1} {
        set intr_id "XPAR_${ipname}_INTR"
	set intr_id [string toupper $intr_id]

      append testfunc_call "
   {
      int Status;

      print(\"\\r\\n Running Interrupt Test  for ${ipname}...\\r\\n\");

      Status = ScuWdtIntrExample(&${intcvar}, &${ipname}, \\
                                 ${deviceid}, \\
                                 ${intr_id});

      if (Status == 0) {
         print(\"ScuWdtIntrExample PASSED\\r\\n\");
      }
      else {
         print(\"ScuWdtIntrExample FAILED\\r\\n\");
      }

   }"

   }



  }

  return $testfunc_call
}
