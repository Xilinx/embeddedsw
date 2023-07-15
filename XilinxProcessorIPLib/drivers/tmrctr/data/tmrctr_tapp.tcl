###############################################################################
# Copyright (C) 2004 - 2020 Xilinx, Inc.  All rights reserved.
# Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 3.0      adk    12/10/13 Updated as per the New Tcl API's
# 4.6      mus    03/13/19 Updated to support scenario where AXI TIMER is
#                          interrupting ARM processor through more than
#                          one interrupt pin. Fix for CR#1024699
# 4.11     mus    01/05/23 Updated to support Microblaze RISC-V
##############################################################################
## @BEGIN_CHANGELOG EDK_I
##
##  - include header files
##
## @END_CHANGELOG

## @BEGIN_CHANGELOG EDK_H
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
        set iftmrintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
        if {$iftmrintr == 1} {
            set inc_file_lines {xtmrctr.h tmrctr_header.h tmrctr_intr_header.h}
        } else {
            set inc_file_lines {xtmrctr.h tmrctr_header.h}
        }    
        return $inc_file_lines
    }
}
    
proc gen_src_files {swproj mhsinst} {
  if {$swproj == 0} {
    return ""
  }
  if {$swproj == 1} {
      set iftmrintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
      
      if {$iftmrintr == 1} {
          set inc_file_lines {examples/xtmrctr_selftest_example.c examples/xtmrctr_intr_example.c data/tmrctr_header.h data/tmrctr_intr_header.h}
      } else {
          set inc_file_lines {examples/xtmrctr_selftest_example.c data/tmrctr_header.h}
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
      set iftmrintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
      if {$iftmrintr == 1} {
          set decl "   static XTmrCtr ${ipname}_Timer;"
          set inc_file_lines $decl
          return $inc_file_lines
      } else {
          return ""
      }
  }
    
}

proc gen_testfunc_call {swproj mhsinst} {
    
    if {$swproj == 0} {
        return ""
    }

    set iftmrintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
    set proc [common::get_property IP_NAME [hsi::get_cells -hier [hsi::get_sw_processor]]]
    set ipname [common::get_property NAME  $mhsinst]
    set deviceid [::hsi::utils::get_ip_param_name $mhsinst "DEVICE_ID"]
    set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
    if { $stdout == "" || $stdout == "none" } {
	set hasStdout 0
    } else {
       set hasStdout 1
    }
    
    if {$iftmrintr == 1} {
       set intr_pin_name [hsi::get_pins -of_objects [hsi::get_cells -hier $ipname]  -filter "TYPE==INTERRUPT"]
       set intcname [::hsi::utils::get_connected_intr_cntrl $ipname  $intr_pin_name]
        if {$proc == "microblaze" && [llength $intcname] > 1} {
              set intcname [lindex $intcname 1]
        }
       set intcvar intc
    }
    
    set testfunc_call ""

  if {${hasStdout} == 0} {

      append testfunc_call "

   {
      int status;
                        
      status = TmrCtrSelfTestExample(${deviceid});

   }"
  

  if {$iftmrintr == 1} {
	if {
           $proc == "microblaze" || $proc == "microblaze_riscv"
	} then {
		set intr_id "XPAR_${intcname}_${ipname}_${intr_pin_name}_INTR"
	} else {
		set intr_id "XPAR_FABRIC_${ipname}_${intr_pin_name}_INTR"
	}
	set intr_id [string toupper $intr_id]
	
      append testfunc_call "
        
   {
      int Status;
      Status = TmrCtrIntrExample(&${intcvar}, &${ipname}_Timer, \\
                                 ${deviceid}, \\
                                 ${intr_id});
   }"

	}


  } else {

      append testfunc_call "

   {
      int status;
      
      print(\"\\r\\n Running TmrCtrSelfTestExample() for ${ipname}...\\r\\n\");
      
      status = TmrCtrSelfTestExample(${deviceid});
      
      if (status == 0) {
         print(\"TmrCtrSelfTestExample PASSED\\r\\n\");
      }
      else {
         print(\"TmrCtrSelfTestExample FAILED\\r\\n\");
      }
   }"


  if {$iftmrintr == 1} {
	if {
           $proc == "microblaze" || $proc == "microblaze_riscv"
	} then {
		set intr_id "XPAR_${intcname}_${ipname}_${intr_pin_name}_INTR"
	} else {
		if {[llength $intcname] > 2} {
			set intr_id "XPAR_FABRIC_${ipname}_${intr_pin_name}0_INTR"
		} else {
			set intr_id "XPAR_FABRIC_${ipname}_${intr_pin_name}_INTR"
		}
	}
	set intr_id [string toupper $intr_id]
	
      append testfunc_call "
   {
      int Status;

      print(\"\\r\\n Running Interrupt Test  for ${ipname}...\\r\\n\");
      
      Status = TmrCtrIntrExample(&${intcvar}, &${ipname}_Timer, \\
                                 ${deviceid}, \\
                                 ${intr_id});
	
      if (Status == 0) {
         print(\"Timer Interrupt Test PASSED\\r\\n\");
      } 
      else {
         print(\"Timer Interrupt Test FAILED\\r\\n\");
      }

   }"

    }


  }

  return $testfunc_call
}



