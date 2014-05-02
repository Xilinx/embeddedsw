##############################################################################
#
# (c) Copyright 2011-2014 Xilinx, Inc. All rights reserved.
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
# -------- ------ -------- ------------------------------------
# 2.0      adk    12/10/13 Updated as per the New Tcl API's
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
        set ifintr [is_ip_interrupting_current_processor $mhsinst]
        if {$ifintr == 1} {
            set inc_file_lines {xiomodule.h iomodule_header.h \
                                xil_exception.h iomodule_intr_header.h}
        } else {
            set inc_file_lines {xiomodule.h iomodule_header.h}
        }
        return $inc_file_lines
    }
}

proc gen_src_files {swproj mhsinst} {
    if {$swproj == 0} {
        return ""
    }

    if {$swproj == 1} {
        set ifintr [is_ip_interrupting_current_processor  $mhsinst]
        if {$ifintr == 1} {
            set inc_file_lines {examples/xiomodule_selftest_example.c \
                                examples/xiomodule_intr_example.c \
                                data/iomodule_header.h \
                                data/iomodule_intr_header.h}
        } else {
            set inc_file_lines {examples/xiomodule_selftest_example.c \
                                data/iomodule_header.h}
        }
        return $inc_file_lines
    }
}

proc gen_testfunc_def {swproj mhsinst} {
    return ""
}

proc gen_init_code {swproj mhsinst} {
    if {$swproj == 1} {
        set ipname [get_property NAME  $mhsinst]
        set ifintr [is_ip_interrupting_current_processor $mhsinst]
        if {$ifintr == 1} {
            set decl "   static XIOModule ${ipname}_IOModule;"
            set inc_file_lines $decl
            return $inc_file_lines
        }
    }
    return ""
}

proc gen_testfunc_call {swproj mhsinst} {
    if {$swproj == 0} {
        return ""
    }

    set ipname [get_property NAME $mhsinst] 
    set deviceid [xget_name $mhsinst "DEVICE_ID"]
    set stdout [get_property CONFIG.STDOUT [get_os]]
    if { $stdout == "" || $stdout == "none" } {
       set hasStdout 0
    } else {
       set hasStdout 1
    }
    set iomodulevar "${ipname}_IOModule"
    set ifintr [is_ip_interrupting_current_processor $mhsinst]
    set extintr [get_property CONFIG.C_INTC_USE_EXT_INTR $mhsinst]
    set testfunc_call ""

    if {${hasStdout} == 0} {
        append testfunc_call "
   {
      XStatus status;

      status = IOModuleSelfTestExample(${deviceid});
   }"
        if {$ifintr != 0} {
            append testfunc_call "
   {
      XStatus status;

      status = IOModuleIntrExample(&${iomodulevar}, ${deviceid});
   }"
            if {$extintr != 0} {
                append testfunc_call "
   {
      XStatus status;

      status = IOModuleInterruptSetup(&${iomodulevar}, ${deviceid});
   }"
            }
        }
    } else {
        append testfunc_call "
   {
      XStatus status;

      print(\"\\r\\nRunning IOModuleSelfTestExample() for ${ipname}...\\r\\n\");

      status = IOModuleSelfTestExample(${deviceid});

      if (status == 0) {
         print(\"IOModuleSelfTestExample PASSED\\r\\n\");
      }
      else {
         print(\"IOModuleSelfTestExample FAILED\\r\\n\");
      }
   }"
        if {$ifintr != 0} {
            append testfunc_call "
   {
      XStatus status;

      print(\"\\r\\nRunning Interrupt Test for ${ipname}...\\r\\n\");

      status = IOModuleIntrExample(&${iomodulevar}, ${deviceid});

      if (status == 0) {
         print(\"IOModule Interrupt Test PASSED\\r\\n\");
      } 
      else {
         print(\"IOModule Interrupt Test FAILED\\r\\n\");
      }
   }"
            if {$extintr != 0} {
                append testfunc_call "
   {
      XStatus status;

      status = IOModuleInterruptSetup(&${iomodulevar}, ${deviceid});
      if (status == 0) {
         print(\"IOModule Setup PASSED\\r\\n\");
      } else {
         print(\"IOModule Setup FAILED\\r\\n\");
      }
   }"
            }
        }
    }
    return $testfunc_call
}
