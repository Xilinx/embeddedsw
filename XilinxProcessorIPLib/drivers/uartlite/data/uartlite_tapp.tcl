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
##  - Initial Revision
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
  set inc_file_lines ""

  if {$swproj == 0} {
    return ""
  }
  if {$swproj == 1} {
    set stdout [get_property CONFIG.STDOUT [get_os]]
    set isStdout [string match $stdout $mhsinst]
    set ipname [get_property IP_NAME $mhsinst]
    if {${isStdout} == 0} {
	set ifuartliteintr [is_ip_interrupting_current_processor $mhsinst]
        if {$ifuartliteintr == 1} {
		if {$ipname == "mdm"} {
			set inc_file_lines {uartlite_header.h}
		} else {
			set inc_file_lines {xuartlite.h uartlite_header.h uartlite_intr_header.h}
		}
        } else {
            set inc_file_lines {uartlite_header.h}
        }    
        return $inc_file_lines
    }
    return $inc_file_lines
  }
}

proc gen_src_files {swproj mhsinst} {
    if {$swproj == 0} {
	return ""
    }
    
   
    if {$swproj == 1} {
	 set ipname [get_property IP_NAME $mhsinst]
	 set stdout [get_property CONFIG.STDOUT [get_os]]
	set isStdout [string match $stdout $mhsinst]
	if {${isStdout} == 0} {
	    set ifuartliteintr [is_ip_interrupting_current_processor $mhsinst]
	    if {$ifuartliteintr == 1} {
		if {$ipname == "mdm"} {
			set inc_file_lines {examples/xuartlite_selftest_example.c data/uartlite_header.h}    
		} else {
			set inc_file_lines {examples/xuartlite_selftest_example.c examples/xuartlite_intr_tapp_example.c data/uartlite_header.h data/uartlite_intr_header.h}
		}
		
	    } else {
		set inc_file_lines {examples/xuartlite_selftest_example.c data/uartlite_header.h}      
	    }
	    return $inc_file_lines
	}
    }
    return ""
}



proc gen_testfunc_def {swproj mhsinst} {
  return ""
}

proc gen_init_code {swproj mhsinst} {
     if {$swproj == 0} {
        return ""
    }
    if {$swproj == 1} {
	  set stdout [get_property CONFIG.STDOUT [get_os]]
	  set isStdout [string match $stdout $mhsinst]
	if {${isStdout} == 0} {
	    
	    set ipname [get_property NAME $mhsinst]
	    set ifuartliteintr [is_ip_interrupting_current_processor $mhsinst]
	    set mdm_name [get_property IP_NAME $mhsinst]
	    if {$ifuartliteintr == 1} {
		if {$mdm_name == "mdm"} {
			return ""
		}
		set decl "   static XUartLite ${ipname}_UartLite;"
		set inc_file_lines $decl
		return $inc_file_lines
	    } else {
		return ""
	    }
	}
    }   
    return ""
}

proc gen_testfunc_call {swproj mhsinst} {

  set ipname [get_property NAME $mhsinst]
  set ifuartliteintr [is_ip_interrupting_current_processor $mhsinst]  
  set testfunc_call ""

  if {$swproj == 0} {
    return $testfunc_call
  }

  # Don't generate test code if this is the STDOUT device
  # We will be using this to generate print stmts for other tests
  set stdout [get_property CONFIG.STDOUT [get_os]]
  set isStdout [string match $stdout $mhsinst]
  if {${isStdout} == 1} {
      append testfunc_call "
   /*
    * Peripheral SelfTest will not be run for ${ipname}
    * because it has been selected as the STDOUT device
    */
"
     return $testfunc_call
  }
  
  set deviceid [xget_name $mhsinst "DEVICE_ID"]
  set stdout [get_property CONFIG.STDOUT [get_os]]
  if { $stdout == "" || $stdout == "none" } {
       set hasStdout 0
  } else {
       set hasStdout 1
  }
   if {$ifuartliteintr == 1} {
        set intr_pin_name [get_pins -of_objects [get_cells $ipname]  -filter "TYPE==INTERRUPT"]
	set intcname [get_connected_interrupt_controller $ipname $intr_pin_name]
	set intcvar intc
	set proc [get_property IP_NAME [get_cells [get_sw_processor]]]
	set mdm_name [get_property IP_NAME $mhsinst]
  }

  if {${hasStdout} == 0} {

      append testfunc_call "

   {
      int status;
      
      status = UartLiteSelfTestExample(${deviceid});
   }"

       if {$ifuartliteintr == 1} {
	if {$mdm_name == "mdm"} {
		 return $testfunc_call
	}
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
      Status = UartLiteIntrExample(&${intcvar}, &${ipname}_UartLite, \\
                                  ${deviceid}, \\
                                  ${intr_id});
   }"

      }
  } else {

      append testfunc_call "

   {
      int status;
      
      print(\"\\r\\nRunning UartLiteSelfTestExample() for ${ipname}...\\r\\n\");
      status = UartLiteSelfTestExample(${deviceid});
      if (status == 0) {
         print(\"UartLiteSelfTestExample PASSED\\r\\n\");
      }
      else {
         print(\"UartLiteSelfTestExample FAILED\\r\\n\");
      }
      
   }"
       if {$ifuartliteintr == 1} {
        if {$mdm_name == "mdm"} {
		 return $testfunc_call
	}
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
      
      Status = UartLiteIntrExample(&${intcvar}, &${ipname}_UartLite, \\
                                  ${deviceid}, \\
                                  ${intr_id});
	
      if (Status == 0) {
         print(\"UartLite Interrupt Test PASSED\\r\\n\");
      } 
      else {
         print(\"UartLite Interrupt Test FAILED\\r\\n\");
      }

   }"

      }
  }

  return $testfunc_call
}

