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
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 1.0      sa     04/05/17 First release
##############################################################################

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
	set iftmr_managerintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
	if {$iftmr_managerintr == 1} {
	    set inc_file_lines {xtmr_manager.h \
				tmr_manager_header.h \
				tmr_manager_intr_header.h}
	} else {
	    set inc_file_lines {xtmr_manager.h tmr_manager_header.h}
	}
	return $inc_file_lines
    }
}

proc gen_src_files {swproj mhsinst} {
    if {$swproj == 0} {
	return ""
    }

    if {$swproj == 1} {
	set iftmr_managerintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
	if {$iftmr_managerintr == 1} {
	    set inc_file_lines {examples/xtmr_manager_selftest_example.c \
				examples/xtmr_manager_intr_tapp_example.c \
				data/tmr_manager_header.h data/tmr_manager_intr_header.h}
	} else {
	    set inc_file_lines {examples/xtmr_manager_selftest_example.c \
				data/tmr_manager_header.h}
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
	set iftmr_managerintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
	if {$iftmr_managerintr == 1} {
	    set decl "   static XTMR_Manager ${ipname}_TMR_Manager;"
	    set inc_file_lines $decl
	    return $inc_file_lines
	} else {
	    return ""
	}
    }
}

proc gen_testfunc_call {swproj mhsinst} {
    set ipname [common::get_property NAME $mhsinst]
    set iftmr_managerintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
    set testfunc_call ""

    if {$swproj == 0} {
      return $testfunc_call
    }

    set deviceid [::hsi::utils::get_ip_param_name $mhsinst "DEVICE_ID"]
    if {$iftmr_managerintr == 1} {
	set intr_pin_name [hsi::get_pins -of_objects [hsi::get_cells -hier $ipname] \
					 -filter "TYPE==INTERRUPT"]
	set intcname [::hsi::utils::get_connected_intr_cntrl $ipname $intr_pin_name]
    }

    append testfunc_call "

   {
      int status;

      print(\"\\r\\nRunning TMR_ManagerSelfTestExample() for ${ipname}...\\r\\n\");
      status = TMR_ManagerSelfTestExample(${deviceid});
      if (status == 0) {
         print(\"TMR_ManagerSelfTestExample PASSED\\r\\n\");
      }
      else {
         print(\"TMR_ManagerSelfTestExample FAILED\\r\\n\");
      }

   }"
    if {$iftmr_managerintr == 1} {
	set intr_id "XPAR_${intcname}_${ipname}_${intr_pin_name}_INTR"
	set intr_id [string toupper $intr_id]
	append testfunc_call "
   {
      int Status;

      print(\"\\r\\n Running Interrupt Test  for ${ipname}...\\r\\n\");

      Status = TMR_ManagerIntrExample(&${intcvar}, &${ipname}_TMR_Manager, \\
                                  ${deviceid}, \\
                                  ${intr_id});

      if (Status == 0) {
         print(\"TMR_Manager Interrupt Test PASSED\\r\\n\");
      }
      else {
         print(\"TMR_Manager Interrupt Test FAILED\\r\\n\");
      }

   }"

    }

    return $testfunc_call
}
