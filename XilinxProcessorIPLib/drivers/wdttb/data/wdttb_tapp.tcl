###############################################################################
#
# Copyright (C) 2005 - 2014 Xilinx, Inc.  All rights reserved.
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

## @BEGIN_CHANGELOG EDK_Im_SP2
##
##  - Added Interrupt support 
##
## @END_CHANGELOG

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
           set inc_file_lines {xwdttb.h wdttb_header.h wdttb_intr_header.h}      
        } else {
            set inc_file_lines {xwdttb.h wdttb_header.h}      
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
            set inc_file_lines {examples/xwdttb_selftest_example.c examples/xwdttb_intr_example.c data/wdttb_header.h data/wdttb_intr_header.h}
        } else {
            set inc_file_lines {examples/xwdttb_selftest_example.c data/wdttb_header.h}
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
          set decl "   static XWdtTb ${ipname}_Wdttb;"
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
  set ipname [common::get_property NAME $mhsinst]
  set deviceid [::hsi::utils::get_ip_param_name $mhsinst "DEVICE_ID"]
  set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
  if { $stdout == "" || $stdout == "none" } {
       set hasStdout 0
  } else {
       set hasStdout 1
  }
  
  if {$iftmrintr == 1} {
       set intr_pin_name [hsi::get_pins -of_objects [hsi::get_cells -hier $ipname] WDT_INTERRUPT]
       set intcname [::hsi::utils::get_connected_intr_cntrl $ipname  $intr_pin_name]
       set intcvar intc
       set proc [common::get_property IP_NAME [hsi::get_cells -hier [hsi::get_sw_processor]]]
  }
  
  set testfunc_call ""

  if {${hasStdout} == 0} {

      append testfunc_call "

   {
      int status;
                  
      status = WdtTbSelfTestExample(${deviceid});
      
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
      Status = WdtTbIntrExample(&${intcvar}, &${ipname}_Wdttb, \\
                                 ${deviceid}, \\
                                 ${intr_id});
   }"
          
      }
  } else {

      append testfunc_call "

   {
      int status;
      
      print(\"\\r\\n Running WdtTbSelfTestExample() for ${ipname}...\\r\\n\");
      
      status = WdtTbSelfTestExample(${deviceid});
      
      if (status == 0) {
         print(\"WdtTbSelfTestExample PASSED\\r\\n\");
      }
      else {
         print(\"WdtTbSelfTestExample FAILED\\r\\n\");
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
      
      Status = WdtTbIntrExample(&${intcvar}, &${ipname}_Wdttb, \\
                                 ${deviceid}, \\
                                 ${intr_id});
	
      if (Status == 0) {
         print(\"Wdttb Interrupt Test PASSED\\r\\n\");
      } 
      else {
         print(\"Wdttb Interrupt Test FAILED\\r\\n\");
      }


   }"
          
      }
  }

  return $testfunc_call
}


