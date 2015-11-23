##############################################################################
#
# (c) Copyright 2004-2014 Xilinx, Inc. All rights reserved.
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
# -------- ------ -------- ----------------------------------------------------
# 4.0     adk    10/12/13 Updated as per the New Tcl API's
##############################################################################

## @BEGIN_CHANGELOG EDK_I
## - Add a new argument to gen_include_files.
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
##  - Migrated to HAL phase 1 using new xil_testmem API.
##  - Added ECC support
##    
## @END_CHANGELOG

## @BEGIN_CHANGELOG EDK_P
##
##  - Updated the logic which checks whether interrupt is connected
##    
## @END_CHANGELOG

# Uses $XILINX_EDK/bin/lib/xillib_sw.tcl

# -----------------------------------------------------------------
# Software Project Types (swproj):
#   0 : MemoryTest - Calls basic  memorytest routines from common driver dir
#   1 : PeripheralTest - Calls any existing polled_example and/or selftest
# -----------------------------------------------------------------

# -----------------------------------------------------------------
# Global variables
# Each global string defined here is a C test function definition.
# Each function defined here must have a unique function prototype.
# Ie. ALL functions defined in this file must be capable of 
#     co-existing in the same C file!

# -----------------------------------------------------------------
# TCL Procedures:
# -----------------------------------------------------------------

proc gen_include_files {swproj mhsinst} {
  if {$swproj == 1} {
    if {! [has_ecc_support $mhsinst]} {
      return ""
    }

    set bram_intr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
    if { ${bram_intr} == 1 } {
      set inc_file_lines {xbram.h bram_header.h bram_intr_header.h}
    } else {
        set inc_file_lines {xbram.h bram_header.h}
    }
    return $inc_file_lines
  }
  if {$swproj == 0} {
    set inc_file_lines {xil_testmem.h xstatus.h }
    return $inc_file_lines
  }
  return ""
}

proc gen_src_files {swproj mhsinst} {
  if {$swproj != 1 || ! [has_ecc_support $mhsinst]} {
    return ""
  }
  if {$swproj == 1} {
    set bram_intr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
    if { ${bram_intr} == 1 } {
      set inc_file_lines {examples/xbram_example.c examples/xbram_intr_example.c data/bram_header.h data/bram_intr_header.h}
    } else {
      set inc_file_lines {examples/xbram_example.c data/bram_header.h}
    }
    return $inc_file_lines
  }
}

proc gen_testfunc_def {swproj mhsinst} {
   return ""
}

proc gen_init_code {swproj mhsinst} {
  if {$swproj != 1 || ! [has_ecc_support $mhsinst]} {
    return ""
  }
  if {$swproj == 1} {
    set bram_intr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
    set ipname [common::get_property NAME $mhsinst]
    if { ${bram_intr} == 1 } {
      set decl "   static XBram ${ipname}_Bram;"
      set inc_file_lines $decl
      return $inc_file_lines
    } else {
      return ""
    }
  }
}

proc gen_testfunc_call {swproj mhsinst} {
  if {$swproj == 1} {
    if {! [has_ecc_support $mhsinst]} {
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
    set bram_intr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]

    if { ${bram_intr} == 1 } {
       set intr_pin_name [hsi::get_pins -of_objects [hsi::get_cells -hier $ipname]  -filter "TYPE==INTERRUPT"]
       set intcname [::hsi::utils::get_connected_intr_cntrl $ipname  $intr_pin_name]
       set intcvar intc
       set proc [hsi::get_sw_processor]
       set mbproc [string first "microblaze" $proc]
       set intr_port_name [get_intr_port_name $mhsinst]
       set intr_id "XPAR_${intcname}_${ipname}_${intr_port_name}_INTR"
       set intr_id [string toupper $intr_id]
    }
    set testfunc_call ""

    if {${hasStdout} == 0} {
      append testfunc_call "
   {
      int status;

      status = BramExample(${deviceid});
   }
"
      if { ${bram_intr} == 1 } {
        append testfunc_call "
    {
       int Status;

       Status = BramIntrExample(&${intcvar}, \\
                                &${ipname}_Bram, \\
                                ${deviceid}, \\
                                ${intr_id});
    }
"
      }
    }

    if {${hasStdout} == 1} {
      append testfunc_call "

   {
      int status;
      
      print(\"\\r\\nRunning Bram Example() for ${ipname}...\\r\\n\");

      status = BramExample(${deviceid});

      if (status == 0) {
         xil_printf(\"Bram Example PASSED.\\r\\n\");
      }
      else {
         print(\"Bram Example FAILED.\\r\\n\");
      }
   }
"
      if { ${bram_intr} == 1 } {
        append testfunc_call "
   {
      int Status;

      Status = BramIntrExample(&${intcvar}, \\
                               &${ipname}_Bram, \\
                               ${deviceid}, \\
                               ${intr_id});

      if (Status == 0 ) {
         print(\"Bram Interrupt Test PASSED. \\r\\n\"); 
      }
      else {
         print(\"Bram Interrupt Test FAILED.\\r\\n\");
      }
   }
"
      }
    }

    return $testfunc_call
  }

  set ipname [xget_value $mhsinst "NAME"]
  set hasStdout [xhas_stdout $mhsinst]

  set testfunc_call ""

  set prog_memranges [xget_program_mem_ranges $mhsinst]
  if {[string compare ${prog_memranges} ""] != 0} {
    foreach progmem $prog_memranges {
      set baseaddrval [xget_value $mhsinst "PARAMETER" ${progmem}]
      append testfunc_call "
   /* 
    * MemoryTest routine will not be run for the memory at 
    * ${baseaddrval} (${ipname})
    * because it is being used to hold a part of this application program
    */
"
    }
  }

  set romemranges [xget_readonly_mem_ranges $mhsinst]
  if {[string compare ${romemranges} ""] != 0} {
    foreach romem $romemranges {
      set baseaddrval [xget_value $mhsinst "PARAMETER" ${romem}]
      append testfunc_call "
   /* 
    * MemoryTest routine will not be run for the memory at 
    * ${baseaddrval} (${ipname})
    * because it is a read-only memory
    */
"
    }
  }

  set baseaddrs [xget_writeable_mem_ranges $mhsinst]
  if {[string compare ${baseaddrs} ""] == 0} {
     return $testfunc_call
  }

  append testfunc_call "
   /* Testing BRAM Memory (${ipname})*/
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

      Xil_TestMem32((u32*)${baseaddr}, 512, 0xAAAA5555, XIL_TESTMEM_ALLMEMTESTS);
      Xil_TestMem16((u16*)${baseaddr}, 1024, 0xAA55, XIL_TESTMEM_ALLMEMTESTS);
      Xil_TestMem8((u8*)${baseaddr}, 2048, 0xA5, XIL_TESTMEM_ALLMEMTESTS);"
    } else {

      append testfunc_call "

      print(\"Starting MemoryTest for ${ipname}:\\r\\n\");
      print(\"  Running 32-bit test...\");
      status = Xil_TestMem32((u32*)${baseaddr}, 512, 0xAAAA5555, XIL_TESTMEM_ALLMEMTESTS);
      if (status == 0) {
         print(\"PASSED!\\r\\n\");
      }
      else {
         print(\"FAILED!\\r\\n\");
      }
      print(\"  Running 16-bit test...\");
      status = Xil_TestMem16((u16*)${baseaddr}, 1024, 0xAA55, XIL_TESTMEM_ALLMEMTESTS);
      if (status == 0) {
         print(\"PASSED!\\r\\n\");
      }
      else {
         print(\"FAILED!\\r\\n\");
      }
      print(\"  Running 8-bit test...\");
      status = Xil_TestMem8((u8*)${baseaddr}, 2048, 0xA5, XIL_TESTMEM_ALLMEMTESTS);
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

proc has_ecc_support {mhsinst} {
  set ecc [common::get_property CONFIG.C_ECC $mhsinst]

  if {$ecc != 0} {
    return 1
  }
  return 0
}

proc get_intr_port_name {mhsinst} {
    set ipname [common::get_property NAME $mhsinst]
    set port_intr [hsi::get_pins -of_objects [hsi::get_cells -hier $mhsinst] "Interrupt"]
    if {$port_intr != ""} {
      return "INTERRUPT"
    }
    set port_intr [hsi::get_pins -of_objects [hsi::get_cells -hier $mhsinst] "ECC_Interrupt"]
    if {$port_intr != ""} {
      return "ECC_INTERRUPT"
    }
    return "INTERRUPT"
}
