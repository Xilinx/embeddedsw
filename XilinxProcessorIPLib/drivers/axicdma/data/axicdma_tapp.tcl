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
# -------- ------ -------- ----------------------------------------------------
#  3.0     adk    12/10/13 Updated as per the New Tcl API's
##############################################################################

## @BEGIN_CHANGELOG EDK_I_SP1
##
##  - Added support for generation of multiple applications.
##    All TCL procedures are now required to have a software
##    project type as its first argument
##  - Add a new argument to gen_include_files.
##  - Added logic to check if DDR is present. (CR 700806)
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
  set ddrpresent [check_if_ddr_is_present $mhsinst]
  if {$ddrpresent < 0} {
      return ""
  }

  if {$swproj == 1} {
  
      set axicdmaintr [is_ip_interrupting_current_processor $mhsinst]
      set sg [get_property CONFIG.C_INCLUDE_SG $mhsinst]      
            
      if {$sg == 1} {
      	if {$axicdmaintr == 1} {
                set inc_file_lines {xaxicdma.h xaxicdma_simple_poll_header.h xaxicdma_simple_intr_header.h xaxicdma_sg_poll_header.h xaxicdma_sg_intr_header.h}
            } else {
                set inc_file_lines {xaxicdma.h xaxicdma_simple_poll_header.h xaxicdma_sg_poll_header.h}
      	}
      } else {
      	if {$axicdmaintr == 1} {
          set inc_file_lines {xaxicdma.h xaxicdma_simple_poll_header.h xaxicdma_simple_intr_header.h}
      	} else {
          set inc_file_lines {xaxicdma.h xaxicdma_simple_poll_header.h}
      	}
      
      }
      
      return $inc_file_lines
  }
}

proc gen_src_files {swproj mhsinst} {
  if {$swproj == 0} {
    return ""
  }
  set ddrpresent [check_if_ddr_is_present $mhsinst]
  if {$ddrpresent < 0} {
      return ""
  }

  if {$swproj == 1} {
      set axicdmaintr [is_ip_interrupting_current_processor $mhsinst]   
      set sg [get_property CONFIG.C_INCLUDE_SG $mhsinst]      
            
      if {$sg == 1} {
       	 if {$axicdmaintr == 1} {
               set inc_file_lines { examples/xaxicdma_example_simple_poll.c data/xaxicdma_simple_poll_header.h 
                		 examples/xaxicdma_example_simple_intr.c data/xaxicdma_simple_intr_header.h 
               			 examples/xaxicdma_example_sg_poll.c data/xaxicdma_sg_poll_header.h 
                		 examples/xaxicdma_example_sg_intr.c data/xaxicdma_sg_intr_header.h }
       	 } else {
                set inc_file_lines { examples/xaxicdma_example_simple_poll.c data/xaxicdma_simple_poll_header.h 
                		 examples/xaxicdma_example_sg_poll.c data/xaxicdma_sg_poll_header.h }
      	}
         
      } else {
      	if {$axicdmaintr == 1} { 
      	      set inc_file_lines { examples/xaxicdma_example_simple_poll.c data/xaxicdma_simple_poll_header.h 
          			  examples/xaxicdma_example_simple_intr.c data/xaxicdma_simple_intr_header.h }
      	} else {
              set inc_file_lines { examples/xaxicdma_example_simple_poll.c data/xaxicdma_simple_poll_header.h }
       	}
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

     set ddrpresent [check_if_ddr_is_present $mhsinst]
     if {$ddrpresent < 0} {
       return ""
     }

     if {$swproj == 1} {
         
       set ipname [get_property NAME $mhsinst]
       set axicdmaintr [is_ip_interrupting_current_processor $mhsinst]
       if {$axicdmaintr == 1} {
           set decl "   static XAxiCdma ${ipname};"
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
  set ddrpresent [check_if_ddr_is_present $mhsinst]
  if {$ddrpresent < 0} {
      return ""
  }

  set axicdmaintr [is_ip_interrupting_current_processor $mhsinst]
  set ipname [get_property NAME  $mhsinst] 
  set deviceid [xget_name $mhsinst "DEVICE_ID"]
  set stdout [get_property CONFIG.STDOUT [get_os]]
    if { $stdout == "" || $stdout == "none" } {
       set hasStdout 0
    } else {
       set hasStdout 1
    }
  set sg [get_property CONFIG.C_INCLUDE_SG $mhsinst]
  
  if {$axicdmaintr == 1} {
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
                        
      	status = XAxiCdma_SimplePollExample(${deviceid});

   }"
   
   if {$axicdmaintr == 1} {
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
    	int status;
    	status = XAxiCdma_SimpleIntrExample(&${intcvar}, &${ipname}, \\
                         	${deviceid},
                         	${intr_id});
   }"
   
   }
   
   if {$sg == 1} {
   
 	append testfunc_call "
   {
   	int status;
                        
      	status = XAxiCdma_SgPollExample(${deviceid});

   }"
   
   if {$axicdmaintr == 1} {
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
    	int status;
    	status = XAxiCdma_SgIntrExample(&${intcvar}, &${ipname}, \\
                             	${deviceid},
                             	${intr_id});
   }"
   
   }

    }
   
  } else {

      append testfunc_call "

   {
      int status;
            
      print(\"\\r\\n Runnning XAxiCdma_SimplePollExample() for ${ipname}...\\r\\n\");
      
      status = XAxiCdma_SimplePollExample(${deviceid});
      
      if (status == 0) {
         print(\"XAxiCdma_SimplePollExample Test PASSED\\r\\n\");
      }
      else {
         print(\"XAxiCdma_SimplePollExample Test FAILED\\r\\n\");
      }
   }"
   
   if {$axicdmaintr == 1} {
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
      int status;
   
      print(\"\\r\\n Running XAxiCdma_SimpleIntrExample  for ${ipname}...\\r\\n\");
         
      status = XAxiCdma_SimpleIntrExample(&${intcvar}, &${ipname}, \\   
				${deviceid},
				${intr_id});
   	
      if (status == 0) {
          print(\"XAxiCdma_SimpleIntrExample Interrupt Test PASSED\\r\\n\");
      } 
       else {
           print(\"XAxiCdma_SimpleIntrExample Interrupt Test FAILED\\r\\n\");
      }
    }"
       } 
       
   if {$sg == 1} {    
   
    append testfunc_call "

   {
      int status;

      print(\"\\r\\n Runnning XAxiCdma_SgPollExample() for ${ipname}...\\r\\n\");

      status = XAxiCdma_SgPollExample(${deviceid});

      if (status == 0) {
	   print(\"XAxiCdma_SgPollExample Test PASSED\\r\\n\");
      }
      else {
	   print(\"XAxiCdma_SgPollExample Test FAILED\\r\\n\");
      }
   }"

    if {$axicdmaintr == 1} {
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
      int status;
   
      print(\"\\r\\n Running XAxiCdma_SgIntrExample  for ${ipname}...\\r\\n\");
         
      status = XAxiCdma_SgIntrExample(&${intcvar}, &${ipname}, \\   
				${deviceid},
				${intr_id});
   	
      if (status == 0) {
          print(\"XAxiCdma_SgIntrExample Interrupt Test PASSED\\r\\n\");
      } 
       else {
           print(\"XAxiCdma_SgIntrExample Interrupt Test FAILED\\r\\n\");
      }
   }"
       
    }
     }   
  }

  return $testfunc_call
}

proc check_if_ddr_is_present {mhsinst} {
     set ips [get_cells $mhsinst "*"]
     set ddrpresent -1
     set migddrpresent -1

     foreach ip $ips {
	set periph [get_property IP_NAME $ip]
	set ddrpresent [string first "ddr" $periph]
	set migddrpresent [string first "mig" $periph]
	if {$ddrpresent >=0 || $migddrpresent >=0} {
		return 1
	}
     }

     return -1 
}
