###############################################################################
#
# Copyright (C) 2004 - 2014 Xilinx, Inc.  All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# Use of the Software is limited solely to applications:
# (a) running on a Xilinx device, or
# (b) that interact with a Xilinx device through a bus or interconnect.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Except as contained in this notice, the name of the Xilinx shall not be used
# in advertising or otherwise to promote the sale, use or other dealings in
# this Software without prior written authorization from Xilinx.
#
###############################################################################
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
    set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
    set isStdout [string match $stdout $mhsinst]
    set ipname [common::get_property IP_NAME $mhsinst]
    if {${isStdout} == 0} {
	set ifuartliteintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
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
	 set ipname [common::get_property IP_NAME $mhsinst]
	 set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
	set isStdout [string match $stdout $mhsinst]
	if {${isStdout} == 0} {
	    set ifuartliteintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
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
	  set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
	  set isStdout [string match $stdout $mhsinst]
	if {${isStdout} == 0} {
	    
	    set ipname [common::get_property NAME $mhsinst]
	    set ifuartliteintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
	    set mdm_name [common::get_property IP_NAME $mhsinst]
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

  set ipname [common::get_property NAME $mhsinst]
  set ifuartliteintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
  set testfunc_call ""

  if {$swproj == 0} {
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

