##############################################################################
#
# (c) Copyright 2004-2014 Xilinx, Inc. All rights reserved.
#
# This file contains confidential and proprietary information of Xilinx, Inc.
# and is protected under U.S. and international copyright and other
# intellectual property laws.
#
# DISCLAIMER
# This disclaimer is not a license and does not grant any rights to the
# materials distributed herewith. Except as otherwise provided in a valid
# license issued to you by Xilinx, and to the maximum extent permitted by
# applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
# FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
# IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
# MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
# and (2) Xilinx shall not be liable (whether in contract or tort, including
# negligence, or under any other theory of liability) for any loss or damage
# of any kind or nature related to, arising under or in connection with these
# materials, including for any direct, or any indirect, special, incidental,
# or consequential loss or damage (including loss of data, profits, goodwill,
# or any type of loss or damage suffered as a result of any action brought by
# a third party) even if such damage or loss was reasonably foreseeable or
# Xilinx had been advised of the possibility of the same.
#
# CRITICAL APPLICATIONS
# Xilinx products are not designed or intended to be fail-safe, or for use in
# any application requiring fail-safe performance, such as life-support or
# safety devices or systems, Class III medical devices, nuclear facilities,
# applications related to the deployment of airbags, or any other applications
# that could lead to death, personal injury, or severe property or
# environmental damage (individually and collectively, "Critical
# Applications"). Customer assumes the sole risk and liability of any use of
# Xilinx products in Critical Applications, subject only to applicable laws
# and regulations governing limitations on product liability.
#
# THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
# AT ALL TIMES.
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 3.0      adk    12/10/13 Updated as per the New Tcl API's
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
        set iftmrintr [is_ip_interrupting_current_processor $mhsinst]
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
      set iftmrintr [is_ip_interrupting_current_processor $mhsinst]
      
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
        
      set ipname [get_property NAME $mhsinst]
      set iftmrintr [is_ip_interrupting_current_processor $mhsinst]
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

    set iftmrintr [is_ip_interrupting_current_processor $mhsinst] 
    set ipname [get_property NAME  $mhsinst] 
    set deviceid [xget_name $mhsinst "DEVICE_ID"]
    set stdout [get_property CONFIG.STDOUT [get_os]]
    if { $stdout == "" || $stdout == "none" } {
	set hasStdout 0
    } else {
       set hasStdout 1
    }
    
    if {$iftmrintr == 1} {
       set intr_pin_name [get_pins -of_objects [get_cells $ipname]  -filter "TYPE==INTERRUPT"]
       set intcname [get_connected_interrupt_controller $ipname  $intr_pin_name]
       set intcvar intc
       set proc [get_property IP_NAME [get_cells [get_sw_processor]]]
    }
    
    set testfunc_call ""

  if {${hasStdout} == 0} {

      append testfunc_call "

   {
      int status;
                        
      status = TmrCtrSelfTestExample(${deviceid}, 0x0);

   }"
  

  if {$iftmrintr == 1} {
	if {
           $proc == "microblaze"
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
                                 ${intr_id}, 0);
   }"

	}


  } else {

      append testfunc_call "

   {
      int status;
      
      print(\"\\r\\n Running TmrCtrSelfTestExample() for ${ipname}...\\r\\n\");
      
      status = TmrCtrSelfTestExample(${deviceid}, 0x0);
      
      if (status == 0) {
         print(\"TmrCtrSelfTestExample PASSED\\r\\n\");
      }
      else {
         print(\"TmrCtrSelfTestExample FAILED\\r\\n\");
      }
   }"


  if {$iftmrintr == 1} {
	if {
           $proc == "microblaze"
	} then {
		set intr_id "XPAR_${intcname}_${ipname}_${intr_pin_name}_INTR"
	} else {
		set intr_id "XPAR_FABRIC_${ipname}_${intr_pin_name}_INTR"
	}
	set intr_id [string toupper $intr_id]
	
      append testfunc_call "
   {
      int Status;

      print(\"\\r\\n Running Interrupt Test  for ${ipname}...\\r\\n\");
      
      Status = TmrCtrIntrExample(&${intcvar}, &${ipname}_Timer, \\
                                 ${deviceid}, \\
                                 ${intr_id}, 0);
	
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



