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
###############################################################################
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 3.0      adk    12/10/13 Updated as per the New Tcl API's
##############################################################################

## @BEGIN_CHANGELOG EDK_M
##
##  19/11/09 ktn  Replaced the call to XUartNs550_mSetLineControlReg
##                with a call to XUartNs550_SetLineControlReg as the
##                name of these macros has changed.
##
## @END_CHANGELOG

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
    return "xuartns550_l.h"
  }
  if {$swproj == 1} {
    set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
    set isStdout [string match $stdout $mhsinst]
    if {${isStdout} == 0} {
	set ifuartns550intr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
        if {$ifuartns550intr == 1} {
            set inc_file_lines {xuartns550_l.h uartns550_header.h xuartns550.h uartns550_intr_header.h}
        } else {
            set inc_file_lines {xuartns550_l.h uartns550_header.h}
        }
    } else {
        set inc_file_lines {xuartns550_l.h}
    }

    return $inc_file_lines
  }
}

proc gen_src_files {swproj mhsinst} {
  if {$swproj == 0} {
    return ""
  }
  if {$swproj == 1} {
    set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
    set isStdout [string match $stdout $mhsinst]
    if {${isStdout} == 0} {
        set ifuartns550intr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
        if {$ifuartns550intr == 1} {
            set inc_file_lines {examples/xuartns550_selftest_example.c examples/xuartns550_intr_example.c data/uartns550_header.h data/uartns550_intr_header.h}
        } else {
            set inc_file_lines {examples/xuartns550_selftest_example.c data/uartns550_header.h}
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

    set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
    set isStdout [string match $stdout $mhsinst]
    if {${isStdout} == 0} {
       if {$swproj == 1} {
	    set ipname [common::get_property NAME  $mhsinst]
	    set ifuartns550intr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
	    if {$ifuartns550intr == 1} {
		set decl "   static XUartNs550 ${ipname}_UartNs550;"
		set inc_file_lines $decl
		return $inc_file_lines
	    } else {
                return ""
            }
       }
       return ""
    }

  set clockhz [::hsi::utils::get_driver_param_name "XUartNs550" "CLOCK_HZ"]
  set baseaddr [::hsi::utils::get_ip_param_name $mhsinst "BASEADDR"]
  set ipname [common::get_property NAME  $mhsinst]

  append testfunc_call "
   /* Initialize ${ipname} - Set baudrate and number of stop bits */
   XUartNs550_SetBaud(${baseaddr}, ${clockhz}, 9600);
   XUartNs550_SetLineControlReg(${baseaddr}, XUN_LCR_8_DATA_BITS);"

  return $testfunc_call
}

proc gen_testfunc_call {swproj mhsinst} {

  set ipname [common::get_property NAME  $mhsinst]
  set ifuartns550intr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
  set testfunc_call ""

  if {$swproj == 0} {
    return $testfunc_call
  }

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
  if {$ifuartns550intr == 1} {
      set intr_pin_name [hsi::get_pins -of_objects [hsi::get_cells -hier $ipname]  -filter "TYPE==INTERRUPT"]
      set intcname [::hsi::utils::get_connected_intr_cntrl $ipname  $intr_pin_name]
      set intcvar intc
      set proc [common::get_property IP_NAME [hsi::get_cells -hier [hsi::get_sw_processor]]]
  }

  if {${hasStdout} == 0} {

      append testfunc_call "

   {
      XStatus status;

      status = UartNs550SelfTestExample(${deviceid});
   }"
      if {$ifuartns550intr == 1} {
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
      XStatus Status;
      Status = UartNs550IntrExample(&${intcvar}, &${ipname}_UartNs550, \\
                                  ${deviceid}, \\
                                  ${intr_id});
   }"

      }

  } else {

      append testfunc_call "

   {
      XStatus status;

      print(\"\\r\\nRunning UartNs550SelfTestExample() for ${ipname}...\\r\\n\");
      status = UartNs550SelfTestExample(${deviceid});
      if (status == 0) {
         print(\"UartNs550SelfTestExample PASSED\\r\\n\");
      }
      else {
         print(\"UartNs550SelfTestExample FAILED\\r\\n\");
      }
   }"

      if {$ifuartns550intr == 1} {
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
      XStatus Status;

      print(\"\\r\\n Running Interrupt Test for ${ipname}...\\r\\n\");

      Status = UartNs550IntrExample(&${intcvar}, &${ipname}_UartNs550, \\
                                  ${deviceid}, \\
                                  ${intr_id});

      if (Status == 0) {
         print(\"UartNs550 Interrupt Test PASSED\\r\\n\");
      }
      else {
         print(\"UartNs550 Interrupt Test FAILED\\r\\n\");
      }

   }"
      }
  }
  return $testfunc_call
}
