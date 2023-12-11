###############################################################################
# Copyright (C) 2004 - 2021 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
##############################################################################
#
# Modification History
#
# Ver      Who    Date     Changes
# -------- ------ -------- ----------------------------------------------------
# 2.0     adk    10/12/13 Updated as per the New Tcl API's
# 3.1     mus    01/14/16 Added support for microblaze
# 3.8     ap     12/11/23 Added support for microblaze risc-v
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
            set inc_file_lines {xcanps.h canps_header.h}
    }
        return $inc_file_lines
}

proc gen_src_files {swproj mhsinst} {
  if {$swproj == 0} {
    return ""
  }
  set isintr [::hsm::utils::is_ip_interrupting_current_proc $mhsinst]
  if {$swproj == 1} {
            if {$isintr == 1} {
		set inc_file_lines {examples/xcanps_polled_example.c examples/xcanps_intr_example.c data/canps_header.h}
	     } else {
		set inc_file_lines {examples/xcanps_polled_example.c data/canps_header.h}
	     }
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

      set ipname [common::get_property NAME $mhsinst]
      set decl "   static XCanPs ${ipname};"
      set inc_file_lines $decl
      return $inc_file_lines

    }

}

proc gen_testfunc_call {swproj mhsinst} {

    if {$swproj == 0} {
        return ""
    }

    set ipname [common::get_property NAME $mhsinst]
    set deviceid [::hsm::utils::get_ip_param_name $mhsinst "DEVICE_ID"]
    set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
    if { $stdout == "" || $stdout == "none" } {
       set hasStdout 0
    } else {
       set hasStdout 1
    }
    set isintr [::hsm::utils::is_ip_interrupting_current_proc $mhsinst]
    set intcvar intc
    if {$isintr == 1} {
    set intr_pin_name [hsi::get_pins -of_objects [hsi::get_cells -hier $ipname] INTERRUPT]
    set intcname [::hsi::utils::get_connected_intr_cntrl $ipname  $intr_pin_name]
    set proc [get_property IP_NAME [hsi::get_cells -hier [hsi::get_sw_processor]]]
    }

    set testfunc_call ""

    if {${hasStdout} == 0} {

      append testfunc_call "

   {
      int Status;

      Status = CanPsPolledExample(${deviceid});

   }"
	if {$isintr == 1} {
            if {
                $proc == "microblaze" || $proc == "microblaze_risc"
            } then {
                    set intr_id "XPAR_${intcname}_${ipname}_${intr_pin_name}_INTR"
            } else {
        set intr_id "XPAR_${ipname}_INTR"
            }
	set intr_id [string toupper $intr_id]

      append testfunc_call "

   {
      int Status;
      Status = CanPsIntrExample(&${intcvar}, &${ipname}, \\
                                 ${deviceid}, \\
                                 ${intr_id});
   }"

   }



  } else {

      append testfunc_call "

   {
      int Status;

      print(\"\\r\\n Running CanPsPolledExample() for ${ipname}...\\r\\n\");

      Status = CanPsPolledExample(${deviceid});

      if (Status == 0) {
         print(\"CanPsPolledExample PASSED\\r\\n\");
      }
      else {
         print(\"CanPsPolledExample FAILED\\r\\n\");
      }
   }"

	if {$isintr ==1 } {
            if {
                $proc == "microblaze" || $proc == "microblaze_risc"
            } then {
                    set intr_id "XPAR_${intcname}_${ipname}_${intr_pin_name}_INTR"
            } else {
        set intr_id "XPAR_${ipname}_INTR"
            }
	set intr_id [string toupper $intr_id]

      append testfunc_call "
   {
      int Status;

      print(\"\\r\\n Running Interrupt Test  for ${ipname}...\\r\\n\");

      Status = CanPsIntrExample(&${intcvar}, &${ipname}, \\
                                 ${deviceid}, \\
                                 ${intr_id});

      if (Status == 0) {
         print(\"CanPsIntrExample PASSED\\r\\n\");
      }
      else {
         print(\"CanPsIntrExample FAILED\\r\\n\");
      }

   }"
 }


  }

  return $testfunc_call
}
