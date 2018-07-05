###############################################################################
#
# Copyright (C) 2007 - 2014 Xilinx, Inc.  All rights reserved.
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
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 4.0      adk    12/10/13 Updated as per the New Tcl API's
##############################################################################

## @BEGIN_CHANGELOG EDK_K_SP2
##
##  - Initial Revision
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
   set inc_file_lines ""

   if {$swproj == 0} {
      return ""
   }
   if {$swproj == 1} {
      set inc_file_lines {mutex_header.h}

      return $inc_file_lines
   }

   return ""
}

proc gen_src_files {swproj mhsinst} {
   if {$swproj == 0} {
      return ""
   }
   if {$swproj == 1} {
      set src_file_lines {examples/xmutex_tapp_example.c data/mutex_header.h}
      return $src_file_lines
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

    set ipname [string toupper [common::get_property NAME $mhsinst]]
    set testfunc_call ""

    if {$swproj == 0} {
	return $testfunc_call
    }

    set deviceid "XPAR_${ipname}_TESTAPP_ID"
    
    append testfunc_call "

    {
        XStatus status;
      
        print(\"\\r\\nRunning MutexExample() for ${ipname}...\\r\\n\");
        status = MutexExample(${deviceid});
        if (status == 0) {
           print(\"MutexExample PASSED\\r\\n\");
        }
        else {
           print(\"MutexExample FAILED\\r\\n\");
        }
    }"

    return $testfunc_call
}

