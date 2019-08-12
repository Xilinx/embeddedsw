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
    set inc_file_lines {xtmr_inject.h tmr_inject_header.h}
    return $inc_file_lines
  }
}

proc gen_src_files {swproj mhsinst} {
    if {$swproj == 0} {
	return ""
    }

    if {$swproj == 1} {
	set inc_file_lines {examples/xtmr_inject_selftest_example.c data/tmr_inject_header.h}
	return $inc_file_lines
    }
    return ""
}

proc gen_testfunc_def {swproj mhsinst} {
  return ""
}

proc gen_init_code {swproj mhsinst} {
    return ""
}

proc gen_testfunc_call {swproj mhsinst} {
    set ipname [common::get_property NAME $mhsinst]
    set iftmr_injectintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
    set testfunc_call ""

    if {$swproj == 0} {
	return $testfunc_call
    }

    set deviceid [::hsi::utils::get_ip_param_name $mhsinst "DEVICE_ID"]

    append testfunc_call "

   {
      int status;

      print(\"\\r\\nRunning TMR_InjectSelfTestExample() for ${ipname}...\\r\\n\");
      status = TMR_InjectSelfTestExample(${deviceid});
      if (status == 0) {
         print(\"TMR_InjectSelfTestExample PASSED\\r\\n\");
      }
      else {
         print(\"TMR_InjectSelfTestExample FAILED\\r\\n\");
      }
   }"

  return $testfunc_call
}
