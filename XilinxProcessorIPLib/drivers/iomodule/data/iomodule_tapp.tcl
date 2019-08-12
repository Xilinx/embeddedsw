###############################################################################
#
# Copyright (C) 2011 - 2014 Xilinx, Inc.  All rights reserved.
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
        set ifintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
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
        set ifintr [::hsi::utils::is_ip_interrupting_current_proc  $mhsinst]
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
        set ipname [common::get_property NAME  $mhsinst]
        set ifintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
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

    set ipname [common::get_property NAME $mhsinst]
    set deviceid [::hsi::utils::get_ip_param_name $mhsinst "DEVICE_ID"]
    set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
    if { $stdout == "" || $stdout == "none" } {
       set hasStdout 0
    } else {
       set hasStdout 1
    }
    set iomodulevar "${ipname}_IOModule"
    set ifintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
    set extintr [common::get_property CONFIG.C_INTC_USE_EXT_INTR $mhsinst]
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
