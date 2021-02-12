###############################################################################
# Copyright (C) 2005 - 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 4.0      adk    12/10/13 Updated as per the New Tcl API's
##############################################################################

## BEGIN_CHANGELOG EDK_L_SP4
##
##  - Removed the checking of the parametr C_INTERRUPT_PRESENT as
##    this is not present in the core
##
## END_CHANGELOG

## BEGIN_CHANGELOG EDK_Im_SP2
##
##  - Added Interrupt support 
##
## END_CHANGELOG

## BEGIN_CHANGELOG EDK_I
##
##  - include header files
##
## END_CHANGELOG

## BEGIN_CHANGELOG EDK_H
##
##  - Added support for generation of multiple applications.
##    All TCL procedures are now required to have a software
##    project type as its first argument
##    
## END_CHANGELOG


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
    set spi_intr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
      if { ${spi_intr} == 1} {
          set inc_file_lines {xspi.h spi_header.h spi_intr_header.h}    
      } else {
          set inc_file_lines {xspi.h spi_header.h}    
      }

    return $inc_file_lines
  }
}

proc gen_src_files {swproj mhsinst} {
  if {$swproj == 0} {
    return ""
  }
  if {$swproj == 1} {
      set spi_intr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
      if { ${spi_intr} == 1} {
          set inc_file_lines {examples/xspi_selftest_example.c examples/xspi_intr_example.c data/spi_header.h data/spi_intr_header.h}
      } else {
          set inc_file_lines {examples/xspi_selftest_example.c data/spi_header.h}
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
        set spi_intr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
        set ipname [common::get_property NAME $mhsinst]
        
        if { ${spi_intr} == 1} {
            set decl "   static XSpi ${ipname}_Spi;"
            set inc_file_lines $decl
            return $inc_file_lines
        } else {
            return ""
        }
    }
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
  set spi_intr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
  
  if { ${spi_intr} == 1} {
      set intr_pin_name [hsi::get_pins -of_objects [hsi::get_cells -hier $ipname]  -filter "TYPE==INTERRUPT"]
      set intcname [::hsi::utils::get_connected_intr_cntrl $ipname  $intr_pin_name]
      set intcvar intc
      set proc [common::get_property IP_NAME [hsi::get_cells -hier [hsi::get_sw_processor]]]
  }

  set testfunc_call ""

  if {${hasStdout} == 0} {

      append testfunc_call "

   {
      XStatus status;
                  
      status = SpiSelfTestExample(${deviceid});

   }"
      if { ${spi_intr} == 1} {
        if {
            $proc == "microblaze"
	 } then {
		 set intr_id "XPAR_${intcname}_${ipname}_${intr_pin_name}_INTR"
	 } else {
		 set intr_id "XPAR_FABRIC_${ipname}_${intr_pin_name}_INTR"
	 }  
          set intr_id [string toupper $intr_id]
          append testfunc_call "
    {
       XStatus Status;

       Status = SpiIntrExample(&${intcvar}, &${ipname}_Spi, \\
                                ${deviceid}, \\
                                ${intr_id});
    }"  
        }
      
      
  } else {

      append testfunc_call "

   {
      XStatus status;
      
      print(\"\\r\\n Running SpiSelfTestExample() for ${ipname}...\\r\\n\");
      
      status = SpiSelfTestExample(${deviceid});
      
      if (status == 0) {
         print(\"SpiSelfTestExample PASSED\\r\\n\");
      }
      else {
         print(\"SpiSelfTestExample FAILED\\r\\n\");
      }
   }"
      if { ${spi_intr} == 1} {
         if {
            $proc == "microblaze"
	 } then {
		 set intr_id "XPAR_${intcname}_${ipname}_${intr_pin_name}_INTR"
	 } else {
		 set intr_id "XPAR_FABRIC_${ipname}_${intr_pin_name}_INTR"
	 }
          set intr_id [string toupper $intr_id]
          append testfunc_call "
    {
       XStatus Status;

       print(\"\\r\\n Running Interrupt Test for ${ipname}...\\r\\n\");

       Status = SpiIntrExample(&${intcvar}, &${ipname}_Spi, \\
                                ${deviceid}, \\
                                ${intr_id});
      if (Status == 0) {
         print(\"Spi Interrupt Test PASSED\\r\\n\");
      } 
      else {
         print(\"Spi Interrupt Test FAILED\\r\\n\");
      }

    }"  
      }
  }
  
  return $testfunc_call
}
