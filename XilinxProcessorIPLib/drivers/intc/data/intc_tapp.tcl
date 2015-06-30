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
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Except as contained in this notice, the name of the Xilinx shall not be used
# in advertising or otherwise to promote the sale, use or other dealings in
# this Software without prior written authorization from Xilinx.
#
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 3.0      adk    12/10/13 Updated as per the New Tcl API's
# 3.1      adk    8/4/14   Fix the CR:783248 Modified the Cascade logic
#			   in the app tcl
##############################################################################

## @BEGIN_CHANGELOG EDK_I_SP1
##
##  - Initial Revision
##
## 01/30/13 bss Modified script to skip tests for Slave controllers in Cascade
##              mode
## 02/26/13 bss Modified get_intr procedure to support Vivado designs
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
	    set inc_file_lines {xintc.h intc_header.h}
	    return $inc_file_lines
    }
}

proc gen_src_files {swproj mhsinst} {
  if {$swproj == 0} {
    return ""
  }
  if {$swproj == 1} {
    set inc_file_lines {examples/xintc_tapp_example.c data/intc_header.h}
    return $inc_file_lines
  }
}

proc gen_testfunc_def {swproj mhsinst} {
    return ""
}

proc gen_init_code {swproj mhsinst} {
  return ""
}

proc gen_testfunc_call {swproj mhsinst} {

  if {$swproj == 0} {
    return ""
  }

set cascade [check_cascade $mhsinst]
if {$cascade == 1} {
	set iscascade [common::get_property CONFIG.C_EN_CASCADE_MODE $mhsinst]
	set ismaster [common::get_property CONFIG.C_CASCADE_MASTER $mhsinst]
	if {!($iscascade == 1 && $ismaster == 1)} {
		return ""
	}
 }

  set ipname [common::get_property NAME  $mhsinst]
  set deviceid [::hsi::utils::get_ip_param_name $mhsinst "DEVICE_ID"]
  set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
  if { $stdout == "" || $stdout == "none" } {
       set hasStdout 0
  } else {
       set hasStdout 1
  }
  set intcvar intc
  set testfunc_call ""

  if {${hasStdout} == 0} {

      append testfunc_call "

   {
      int status;

      status = IntcSelfTestExample(${deviceid});

   }"

      set ifintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
      if {$ifintr != 0} {
          append testfunc_call "

   {
       int Status;

       Status = IntcInterruptSetup(&${intcvar}, ${deviceid});

   }"
      }

  } else {

      append testfunc_call "

   {
      int status;

      print(\"\\r\\n Running IntcSelfTestExample() for ${ipname}...\\r\\n\");

      status = IntcSelfTestExample(${deviceid});

      if (status == 0) {
         print(\"IntcSelfTestExample PASSED\\r\\n\");
      }
      else {
         print(\"IntcSelfTestExample FAILED\\r\\n\");
      }
   }"

      set ifintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
      if {$ifintr != 0} {

          append testfunc_call "

   {
       int Status;

       Status = IntcInterruptSetup(&${intcvar}, ${deviceid});
       if (Status == 0) {
          print(\"Intc Interrupt Setup PASSED\\r\\n\");
       }
       else {
         print(\"Intc Interrupt Setup FAILED\\r\\n\");
      }
   }"

      }
  }

  return $testfunc_call

}


proc check_cascade {mhsinst} {
	set periphs [::hsi::utils::get_common_driver_ips $mhsinst]
    foreach periph $periphs {
		set i 0
		set source_pins [::hsi::utils::get_interrupt_sources $periph]
        foreach source_pin $source_pins {
            set source_pin_name($i) [common::get_property NAME $source_pin]
            if { [::hsi::utils::is_external_pin $source_pin] } {
                continue
            }
            set source_periph [hsi::get_cells -of_objects $source_pin ]
            set source_type [common::get_property IP_TYPE $source_periph]
            if {[string compare -nocase $source_type "INTERRUPT_CNTLR"] == 0} {
		return 1
            }
        }
    }
	return 0
}
