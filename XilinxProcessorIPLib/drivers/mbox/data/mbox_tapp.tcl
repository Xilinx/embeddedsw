###############################################################################
# Copyright (C) 2012 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
#
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
      set inc_file_lines {mbox_header.h}

      return $inc_file_lines
   }

   return ""
}

proc gen_src_files {swproj mhsinst} {
   if {$swproj == 0} {
      return ""
   }
   if {$swproj == 1} {
      set src_file_lines {examples/xmbox_tapp_example.c data/mbox_header.h}
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

    set numproc [::hsi::utils::get_procs]
    set numproc [llength $numproc]
      
    if {$numproc == 1} {
    	append testfunc_call "
       /*
      	* Peripheral SelfTest will not be run for ${ipname}
      	* because design contains only one Processor.
      	*/
    "
     	return $testfunc_call
    }
    
    set deviceid "XPAR_${ipname}_TESTAPP_ID"

    append testfunc_call "

    {
        XStatus status;
      
        print(\"\\r\\nRunning MailboxExample() for ${ipname}...\\r\\n\");
        status = MailboxExample(${deviceid});
        if (status == 0) {
            print(\"MailboxExample PASSED\\r\\n\");
        }
        else {
            print(\"MailboxExample FAILED\\r\\n\");
        }
     }"

  return $testfunc_call
}

