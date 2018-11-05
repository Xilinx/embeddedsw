###############################################################################
#
# Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
##############################################################################
#
# Modification History
#
# Ver      Who    Date     Changes
# -------- ------ -------- ----------------------------------------------------
# 1.5      adk    11/22/17 Added peripheral test app support.
##############################################################################

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
            set inc_file_lines {xzdma.h zdma_header.h}
    }
        return $inc_file_lines
}

proc gen_src_files {swproj mhsinst} {
  if {$swproj == 0} {
    return ""
  }
  set isintr [::hsm::utils::is_ip_interrupting_current_proc $mhsinst]
  if {$swproj == 1} {
        if {$isintr == 1} {
		set inc_file_lines {examples/xzdma_simple_example.c examples/xzdma_selftest_example.c data/zdma_header.h}
	} else {
		set inc_file_lines {examples/xzdma_selftest_example.c data/zdma_header.h}
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
      set decl "   static XZDma ${ipname};"
      set inc_file_lines $decl
      return $inc_file_lines

    }

}

proc gen_testfunc_call {swproj mhsinst} {
    if {$swproj == 0} {
        return ""
    }

    set ipname [common::get_property NAME $mhsinst]
    set deviceid [::hsm::utils::get_ip_param_name $mhsinst "DEVICE_ID"]
    set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
    if { $stdout == "" || $stdout == "none" } {
       set hasStdout 0
    } else {
       set hasStdout 1
    }
    set isintr [::hsm::utils::is_ip_interrupting_current_proc $mhsinst]
    set intcvar intc
    if {$isintr == 1} {
    set intr_pin_name [hsi::get_pins -of_objects [hsi::get_cells -hier $ipname] INTERRUPT]
    set intcname [::hsi::utils::get_connected_intr_cntrl $ipname  $intr_pin_name]
    set proc [get_property IP_NAME [hsi::get_cells -hier [hsi::get_sw_processor]]]
    }

    set testfunc_call ""

    if {${hasStdout} == 0} {

      append testfunc_call "
   {
      int Status;

      Status = XZDma_SelfTestExample(${deviceid});

   }"
	if {$isintr == 1} {
            if {
                $proc == "microblaze"
            } then {
		set intr_id "XPAR_${intcname}_${ipname}_${intr_pin_name}_INTR"
            } else {
		set intr_id "XPAR_${ipname}_INTR"
            }
	    set intr_id [string toupper $intr_id]

   {
      int Status;
      Status = XZDma_SimpleExample(&${intcvar}, &${ipname}, \\
                                 ${deviceid}, \\
                                 ${intr_id});
   }"

   }
} else {

      append testfunc_call "

   {
      int Status;

      print(\"\\r\\n Running XZDma_SelfTestExample() for ${ipname}...\\r\\n\");

      Status = XZDma_SelfTestExample(${deviceid});

      if (Status == 0) {
         print(\"XZDma_SelfTestExample PASSED\\r\\n\");
      }
      else {
         print(\"XZDma_SelfTestExample FAILED\\r\\n\");
      }
   }"

	if {$isintr ==1 } {
            if {
                $proc == "microblaze"
            } then {
                    set intr_id "XPAR_${intcname}_${ipname}_${intr_pin_name}_INTR"
            } else {
        set intr_id "XPAR_${ipname}_INTR"
            }
	set intr_id [string toupper $intr_id]

      append testfunc_call "
   {
      int Status;

      print(\"\\r\\n Running Interrupt Test  for ${ipname}...\\r\\n\");

      Status = XZDma_SimpleExample(&${intcvar}, &${ipname}, \\
                                 ${deviceid}, \\
                                 ${intr_id});

      if (Status == 0) {
         print(\"ZDMA Simple Example PASSED\\r\\n\");
      }
      else {
         print(\"ZDMA Simple Example FAILED\\r\\n\");
      }

   }"
 }


  }

  return $testfunc_call
}
