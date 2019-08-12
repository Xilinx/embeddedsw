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
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ----------------------------------------------------
#  4.0     adk    12/10/13 Updated as per the New Tcl API's
###############################################################

## @BEGIN_CHANGELOG EDK_I
##
##  - Add a new argument to gen_include_files.
##
## @END_CHANGELOG

## @BEGIN_CHANGELOG EDK_H
##
##  - Added support for generation of multiple applications.
##    All TCL procedures are now required to have a software
##    project type as its first argument
##    
## @END_CHANGELOG

## @BEGIN_CHANGELOG EDK_M
##
##  - HAL phase 1 migration to use new test memory functions.
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
  if {$swproj == 1} {
    return ""
  }
  if {$swproj == 0} {
    set inc_file_lines {xil_testmem.h xstatus.h}
    return $inc_file_lines
  }
}

proc gen_src_files {swproj mhsinst} {
  return ""
}

proc gen_testfunc_def {swproj mhsinst} {
  return ""
}

proc gen_init_code {swproj mhsinst} {
  return ""
}

proc gen_testfunc_call {swproj mhsinst} {

  if {$swproj == 1} {
    return ""
  }
  
  set ipname [common::get_property NAME $mhsinst]
  set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
  if { $stdout == "" || $stdout == "none" } {
       set hasStdout 0
  } else {
       set hasStdout 1
  }

  set testfunc_call ""

  set prog_memranges [hsi::get_mem_ranges -filter "IS_INSTRUCTION==1" $mhsinst]
  if {[string compare ${prog_memranges} ""] != 0} {
    foreach progmem $prog_memranges {
      set baseaddrval [common::get_property CONFIG.${progmem} $mhsinst]
      append testfunc_call "
   /* 
    * MemoryTest routine will not be run for the memory at 
    * ${baseaddrval} (${ipname})
    * because it is being used to hold a part of this application program
    */
"
    }
  }

  set romemranges [hsi::get_mem_ranges $mhsinst]
  if {[string compare ${romemranges} ""] != 0} {
    foreach romem $romemranges {
      set baseaddrval [common::get_property CONFIG.${romem} $mhsinst]
      append testfunc_call "
   /* 
    * MemoryTest routine will not be run for the memory at 
    * ${baseaddrval} (${ipname})
    * because it is a read-only memory
    */
"
    }
  }

  set baseaddrs [hsi::get_mem_ranges $mhsinst]
  if {[string compare ${baseaddrs} ""] == 0} {
     return $testfunc_call
  }

  append testfunc_call "
   /* Testing EMC Memory (${ipname})*/
   \{"

  if {${hasStdout} == 1} {
    append testfunc_call "
      int status;"
  }

  # Get XPAR_ macro for each baseaddr param
  foreach baseaddr $baseaddrs {
    lappend baseaddr_macros [::hsi::utils::get_ip_param_name $mhsinst $baseaddr]
  }

  foreach baseaddr $baseaddr_macros {
    if {${hasStdout} == 0} {

      append testfunc_call "

      Xil_TestMem32((u32*)${baseaddr}, 1024, 0xAAAA5555, XIL_TESTMEM_ALLMEMTESTS);
      Xil_TestMem16((u16*)${baseaddr}, 2048, 0xAA55, XIL_TESTMEM_ALLMEMTESTS);
      Xil_TestMem8((u8*)${baseaddr}, 4096, 0xA5, XIL_TESTMEM_ALLMEMTESTS);"
    } else {

      append testfunc_call "

      print(\"Starting MemoryTest for ${ipname}:\\r\\n\");
      print(\"  Running 32-bit test...\");
      status = Xil_TestMem32((u32*)${baseaddr}, 1024, 0xAAAA5555, XIL_TESTMEM_ALLMEMTESTS);
      if (status == 0) {
         print(\"PASSED!\\r\\n\");
      }
      else {
         print(\"FAILED!\\r\\n\");
      }
      print(\"  Running 16-bit test...\");
      status = Xil_TestMem16((u16*)${baseaddr}, 2048, 0xAA55, XIL_TESTMEM_ALLMEMTESTS);
      if (status == 0) {
         print(\"PASSED!\\r\\n\");
      }
      else {
         print(\"FAILED!\\r\\n\");
      }
      print(\"  Running 8-bit test...\");
      status = Xil_TestMem8((u8*)${baseaddr}, 4096, 0xA5, XIL_TESTMEM_ALLMEMTESTS);
      if (status == 0) {
         print(\"PASSED!\\r\\n\");
      }
      else {
         print(\"FAILED!\\r\\n\");
      }"

    }
  }

  append testfunc_call "
   \}"

  return $testfunc_call
}
