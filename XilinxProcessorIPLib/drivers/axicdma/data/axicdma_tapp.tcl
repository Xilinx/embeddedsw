###############################################################################
#
# Copyright (C) 2004 - 2019 Xilinx, Inc.  All rights reserved.
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
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
#
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ----------------------------------------------------
#  3.0     adk    12/10/13 Updated as per the New Tcl API's
#  4.5     rsp    07/06/18 Remove space b/w backslash and newline
#  4.5     rsp    10/03/18 Fix typos
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
  
      set axicdmaintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
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
      set axicdmaintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
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
       set axicdmaintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
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

  set axicdmaintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
  set ipname [get_property NAME  $mhsinst] 
  set deviceid [::hsi::utils::get_ip_param_name $mhsinst "DEVICE_ID"]
  set stdout [get_property CONFIG.STDOUT [hsi::get_os]]
    if { $stdout == "" || $stdout == "none" } {
       set hasStdout 0
    } else {
       set hasStdout 1
    }
  set sg [get_property CONFIG.C_INCLUDE_SG $mhsinst]
  
  if {$axicdmaintr == 1} {
      set intr_pin_name [hsi::get_pins -of_objects [hsi::get_cells -hier $ipname]  -filter "TYPE==INTERRUPT"]
      set intcname [::hsi::utils::get_connected_intr_cntrl $ipname  $intr_pin_name]
      set intcvar intc
      set proc [get_property IP_NAME [hsi::get_cells -hier [hsi::get_sw_processor]]]
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
            
      print(\"\\r\\n Running XAxiCdma_SimplePollExample() for ${ipname}...\\r\\n\");
      
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

      print(\"\\r\\n Running XAxiCdma_SgPollExample() for ${ipname}...\\r\\n\");

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
     set ips [hsi::get_cells -hier $mhsinst "*"]
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
