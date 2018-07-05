###############################################################################
# Copyright (C) 2005 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 3.0      adk    12/10/13 Updated as per the New Tcl API's
# 4.5      nsk    07/08/19 Updated tcl to not to generate Wdttb interrupt
#                          example, when Wdttb interrupt pin is not connected
#                          (CR-1035919).
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
    set ps_wdt [isps_wdttb $mhsinst]
    if {$swproj == 1} {
        set iftmrintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
        if {$iftmrintr == 1} {
            if {$ps_wdt == 0} {
                set intcname [get_intcname $mhsinst]
                if {$intcname == ""} {
                    set inc_file_lines {xwdttb.h wdttb_header.h}
                } else {
                    set inc_file_lines {xwdttb.h wdttb_header.h wdttb_intr_header.h}
                }
            } else {
               set inc_file_lines {xwdttb.h wdttb_header.h}
            }
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
    set ps_wdt [isps_wdttb $mhsinst]
    if {$swproj == 1} {
        set iftmrintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
        
        if {$iftmrintr == 1} {
            if {$ps_wdt == 0} {
                set intcname [get_intcname $mhsinst]
                if {$intcname == ""} {
                    set inc_file_lines {examples/xwdttb_selftest_example.c examples/xwdttb_example.c data/wdttb_header.h}
                } else {
                    set inc_file_lines {examples/xwdttb_selftest_example.c examples/xwdttb_intr_example.c data/wdttb_header.h data/wdttb_intr_header.h}
                }
            } else {
                set inc_file_lines {examples/xwdttb_selftest_example.c examples/xwdttb_gwdt_example.c data/wdttb_header.h}
            }
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
        
      set ps_wdt [isps_wdttb $mhsinst]
      set ipname [common::get_property NAME $mhsinst]
      set intr_pin_name [hsi::get_pins -of_objects [hsi::get_cells -hier $ipname] WDT_INTERRUPT]
      set intcname [get_intcname $mhsinst]
      set iftmrintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
      if {$iftmrintr == 1 && $ps_wdt == 0} {
          if {[string compare -nocase $intcname ""] != 0} {
              set decl "   static XWdtTb ${ipname}_Wdttb;"
              set inc_file_lines $decl
              return $inc_file_lines
          }
      } else {
          return ""
      }
  }

}

proc gen_testfunc_call {swproj mhsinst} {
  if {$swproj == 0} {
    return ""
  }
 
  set ps_wdt [isps_wdttb $mhsinst]
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
      if {$iftmrintr == 1 && $ps_wdt == 0} {
        if {
           $proc == "microblaze"
	} then {
		 set intr_id "XPAR_${intcname}_${ipname}_${intr_pin_name}_INTR"
	} else {
		set intr_id "XPAR_FABRIC_${ipname}_${intr_pin_name}_INTR"
	}  
          set intr_id [string toupper $intr_id]

          if {$intcname == ""} {
        append testfunc_call "

   {
      int Status;

      Status = WdtTbExample(${deviceid});

   }"
         } else {
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
      int Status;

      Status = GWdtTbExample(${deviceid});

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
      if {$iftmrintr == 1 && $ps_wdt == 0} {
        if {
           $proc == "microblaze"
	} then {
		 set intr_id "XPAR_${intcname}_${ipname}_${intr_pin_name}_INTR"
	} else {
		set intr_id "XPAR_FABRIC_${ipname}_${intr_pin_name}_INTR"
	} 
          set intr_id [string toupper $intr_id]
          if {$intcname == ""} {
    append testfunc_call "

   {
      int Status;

      print(\"\\r\\n Running WdtTbExample() for ${ipname}...\\r\\n\");

      Status = WdtTbExample(${deviceid});

      if (Status == 0) {
         print(\"WdtTbExample PASSED\\r\\n\");
      }
      else {
         print(\"WdtTbExample FAILED\\r\\n\");
      }
   }"
         } else {
          
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
       } else {
    append testfunc_call "

   {
      int Status;

      print(\"\\r\\n Running GWdtTbExample() for ${ipname}...\\r\\n\");

      Status = GWdtTbExample(${deviceid});

      if (Status == 0) {
         print(\"GWdtTbExample PASSED\\r\\n\");
      }
      else {
         print(\"GWdtTbExample FAILED\\r\\n\");
      }
   }"
       }
  }

  return $testfunc_call
}

proc isps_wdttb {mhsinst} {
    set is_pl [common::get_property IS_PL [::hsi::get_cells -hier $mhsinst]]
    set is_ps 1
    if { [string match -nocase $is_pl "true"] || [string match -nocase $is_pl "1"]} {
        set is_ps 0
    }
    return $is_ps
}

proc get_intcname {mhsinst} {

    set ipname [common::get_property NAME $mhsinst]
    set intr_pin_name [hsi::get_pins -of_objects [hsi::get_cells -hier $ipname] WDT_INTERRUPT]
    set intcname [::hsi::utils::get_connected_intr_cntrl $ipname  $intr_pin_name]

    return $intcname
}
