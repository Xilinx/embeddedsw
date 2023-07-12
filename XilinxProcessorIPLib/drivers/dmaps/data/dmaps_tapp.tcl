###############################################################################
# Copyright (C) 2011 - 2022 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 2.0   adk  10/12/13 Updated as per the New Tcl API's
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
            set inc_file_lines {xdmaps.h dmaps_header.h}
        }    
        return $inc_file_lines
}
    
proc gen_src_files {swproj mhsinst} {
  if {$swproj == 0} {
    return ""
  }
  if {$swproj == 1} {
            
      set inc_file_lines {examples/xdmaps_example_w_intr.c data/dmaps_header.h}
      
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
	if {"ps7_dma_ns" == $ipname} {
	     return ""
	}	
    set deviceid [::hsi::utils::get_ip_param_name $mhsinst "DEVICE_ID"]
    set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
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
      append testfunc_call "

   {
      int Status;
                        
      Status = XDmaPs_Example_W_Intr(&${intcvar},${deviceid});

   }"
  
   }

  } else {

	if {$isintr == 1} {
      append testfunc_call "

   {
      int Status;
      
      print(\"\\r\\n Running XDmaPs_Example_W_Intr() for ${ipname}...\\r\\n\");
      
      Status = XDmaPs_Example_W_Intr(&${intcvar},${deviceid});
      
      if (Status == 0) {
         print(\"XDmaPs_Example_W_Intr PASSED\\r\\n\");
      }
      else {
         print(\"XDmaPs_Example_W_Intr FAILED\\r\\n\");
      }
   }"

   }

  }

  return $testfunc_call
}
