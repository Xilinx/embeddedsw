###############################################################################
# Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
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

