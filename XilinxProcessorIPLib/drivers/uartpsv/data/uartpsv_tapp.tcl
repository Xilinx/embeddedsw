###############################################################################
# Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
###############################################################################
# MODIFICATION HISTORY:
# Ver  Who Date     Changes
# ---- --- -------- -----------------------------------------------
# 1.0  sg  09/12/17 First Release
# 1.4  sne 02/03/21 Updated uartpsv_tapp.tcl to support CIPS3.0 designs
#		    In CIPS3.0 interrupt id not generating properly.
#		    This patch fixes the interrupt id issue.
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
    return
  }
  if {$swproj == 1} {
    set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
    set isStdout [string match $stdout $mhsinst]
    if {${isStdout} == 0} {
        set inc_file_lines { xuartpsv.h uartpsv_header.h }
	set isintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
	if {$isintr == 1} {
		set inc_file_lines {xuartpsv.h uartpsv_header.h uartpsv_intr_header.h }
	}
        return $inc_file_lines
    }

    return ""
  }
}

proc gen_src_files {swproj mhsinst} {
  if {$swproj == 0} {
    return ""
  }
  if {$swproj == 1} {
	set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
	set isStdout [string match $stdout $mhsinst]
    if {${isStdout} == 0} {
            set inc_file_lines {examples/xuartpsv_polled_example.c data/uartpsv_header.h }
	    set isintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
	    if {$isintr == 1} {
		set inc_file_lines {examples/xuartpsv_intr_example.c examples/xuartpsv_polled_example.c data/uartpsv_header.h data/uartpsv_intr_header.h }
	    }
        return $inc_file_lines
    }
  }
    return ""
}

proc gen_testfunc_def {swproj mhsinst} {
  return ""
}

proc gen_init_code {swproj mhsinst} {


 set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
 set isStdout [string match $stdout $mhsinst]
 set ipname [common::get_property NAME $mhsinst]
 if {${isStdout} == 0} {
    if {$swproj == 1} {
		set decl "   static XUartPsv ${ipname};"
		set inc_file_lines $decl
		return $inc_file_lines
    }
      return ""
  }

}

proc gen_testfunc_call {swproj mhsinst} {

  set ipname [common::get_property NAME $mhsinst]
  set ip_name [common::get_property IP_NAME $mhsinst]
  set intsnum [string index $mhsinst end]
  set testfunc_call ""

  if {$swproj == 0} {
    return $testfunc_call
  }

  set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
  set isStdout [string match $stdout $mhsinst]
  if {${isStdout} == 1} {
    append testfunc_call "
   /*
    * Peripheral Test will not be run for ${ipname}
    * because it has been selected as the STDOUT device
    */
"
    return $testfunc_call
  }

  set deviceid [::hsi::utils::get_ip_param_name $mhsinst "DEVICE_ID"]
  set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
  if { $stdout == "" || $stdout == "none" } {
     set hasStdout 0
  } else {
     set hasStdout 1
  }

  set isintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
  set intcvar intc
  if {$isintr == 1} {
      set intr_pin_name [hsi::get_pins -of_objects [hsi::get_cells -hier $ipname] INTERRUPT]
      set intcname [::hsi::utils::get_connected_intr_cntrl $ipname  $intr_pin_name]
      set proc [get_property IP_NAME [hsi::get_cells -hier [hsi::get_sw_processor]]]
  }


  if {${hasStdout} == 0} {

      append testfunc_call "

   {
      int Status;

      Status = UartPsvPolledExample(${deviceid});
   }"

	if {$isintr == 1} {
            if {
                $proc == "microblaze"
            } then {
                    set intr_id "XPAR_${intcname}_${ipname}_${intr_pin_name}_INTR"
            } else {
        set intr_id "XPAR_${ipname}_INTR"
            }
	set intr_id [string toupper $intr_id]

      append testfunc_call "

   {
      int Status;
      Status = UartPsvIntrExample(&${intcvar}, &${ipname}, \\
                                  ${deviceid}, \\
                                  ${intr_id});
   }"

   }


  } else {

      append testfunc_call "

   {
      int Status;

      print(\"\\r\\nRunning UartPsvPolledExample() for ${ipname}...\\r\\n\");
      Status = UartPsvPolledExample(${deviceid});
      if (Status == 0) {
         print(\"UartPsvPolledExample PASSED\\r\\n\");
      }
      else {
         print(\"UartPsvPolledExample FAILED\\r\\n\");
      }
   }"

	if {$isintr == 1} {
            if {
                $proc == "microblaze"
            } then {
                    set intr_id "XPAR_${intcname}_${ipname}_${intr_pin_name}_INTR"
            } else {
        set intr_id "XPAR_${ip_name}_${intsnum}_INTR"
            }
	set intr_id [string toupper $intr_id]

	  append testfunc_call "
   {
      int Status;

      print(\"\\r\\n Running Interrupt Test for ${ipname}...\\r\\n\");

      Status = UartPsvIntrExample(&${intcvar}, &${ipname}, \\
                                  ${deviceid}, \\
                                  ${intr_id});

      if (Status == 0) {
         print(\"UartPsvIntrExample PASSED\\r\\n\");
      }
      else {
         print(\"UartPsvIntrExample FAILED\\r\\n\");
      }

   }"

   }

  }
  return $testfunc_call
}
