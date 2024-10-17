###############################################################################
# Copyright (C) 2004 - 2022 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 3.0      adk    12/10/13 Updated as per the New Tcl API's
# 3.7	   adk	  31/01/22 Fix interrupt controller name in SMP designs.
# 3.7	   adk	  14/02/22 When uartlite is configured as TMR SEM, Selftest
# 			   example is failing as TMR SEM continuously
# 			   receives data, So don't pull the driver examples
# 			   for this configuration, updated the tcl checks
# 			   accordingly.
# 	   adk    21/02/22 Fix ipname variable usage in gen_testfunc_call API(),
#			   ipname variable in the gen_testfunc_call  API() is
#			   getting overwritten for the tmr_sem IP validation
#			   check, due to which wrong instance name being
#			   generated in the testperiph.c file.
# 3.10     ap     11/12/23 Added support for microblaze risc-v
##############################################################################

## BEGIN_CHANGELOG EDK_I
##
##  - include header files
##    
## END_CHANGELOG

## BEGIN_CHANGELOG EDK_H
##
##  - Initial Revision
##    
## END_CHANGELOG

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
    set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
    set isStdout [string match $stdout $mhsinst]
    set ipname [common::get_property IP_NAME $mhsinst]
    if {$ipname == "tmr_sem"} {
      return ""
    }
    if {${isStdout} == 0} {
	set ifuartliteintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
        if {$ifuartliteintr == 1} {
		if {$ipname == "mdm" || $ipname == "mdm_riscv"} {
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
	 set ipname [common::get_property IP_NAME $mhsinst]
	 if {$ipname == "tmr_sem"} {
	    return ""
	 }
	 set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
	set isStdout [string match $stdout $mhsinst]
	if {${isStdout} == 0} {
	    set ifuartliteintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
	    if {$ifuartliteintr == 1} {
		if {$ipname == "mdm" || $ipname == "mdm_riscv"} {
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
	  set ipname [common::get_property IP_NAME $mhsinst]
	  if {$ipname == "tmr_sem"} {
	    return ""
	  }
	  set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
	  set isStdout [string match $stdout $mhsinst]
	if {${isStdout} == 0} {
	    
	    set ipname [common::get_property NAME $mhsinst]
	    set ifuartliteintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
	    set mdm_name [common::get_property IP_NAME $mhsinst]
	    if {$ifuartliteintr == 1} {
		if {$mdm_name == "mdm" || $mdm_name == "mdm_riscv"} {
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

  set ipname [common::get_property NAME $mhsinst]
  set ifuartliteintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
  set testfunc_call ""

  if {$swproj == 0} {
    return $testfunc_call
  }

  set ip_name [common::get_property IP_NAME $mhsinst]
  if {$ip_name == "tmr_sem"} {
    return $testfunc_call
  }

  # Don't generate test code if this is the STDOUT device
  # We will be using this to generate print stmts for other tests
  set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
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
  
  set deviceid [::hsi::utils::get_ip_param_name $mhsinst "DEVICE_ID"]
  set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
  if { $stdout == "" || $stdout == "none" } {
       set hasStdout 0
  } else {
       set hasStdout 1
  }
   if {$ifuartliteintr == 1} {
        set intr_pin_name [hsi::get_pins -of_objects [hsi::get_cells -hier $ipname]  -filter "TYPE==INTERRUPT"]
	set intcname [::hsi::utils::get_connected_intr_cntrl $ipname $intr_pin_name]
        set intcname [hsi::get_mem_ranges -of_objects [hsi::get_cells -hier [hsi::get_sw_processor]] $intcname]
	set intcvar intc
	set proc [common::get_property IP_NAME [hsi::get_cells -hier [hsi::get_sw_processor]]]
	set mdm_name [common::get_property IP_NAME $mhsinst]
  }

  if {${hasStdout} == 0} {

      append testfunc_call "

   {
      int status;
      
      status = UartLiteSelfTestExample(${deviceid});
   }"

       if {$ifuartliteintr == 1} {
	if {$mdm_name == "mdm" || $mdm_name == "mdm_riscv"} {
		 return $testfunc_call
	}
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
        if {$mdm_name == "mdm" || $mdm_name == "mdm_riscv"} {
		 return $testfunc_call
	}
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

