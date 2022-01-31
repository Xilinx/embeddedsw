###############################################################################
# Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 1.0      sa     04/05/17 First release
# 1.1	   adk	  31/01/22 Fixed compilation error.
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
	set iftmr_managerintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
	if {$iftmr_managerintr == 1} {
	    set inc_file_lines {xtmr_manager.h \
				tmr_manager_header.h \
				tmr_manager_intr_header.h}
	} else {
	    set inc_file_lines {xtmr_manager.h tmr_manager_header.h}
	}
	return $inc_file_lines
    }
}

proc gen_src_files {swproj mhsinst} {
    if {$swproj == 0} {
	return ""
    }

    if {$swproj == 1} {
	set iftmr_managerintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
	if {$iftmr_managerintr == 1} {
	    set inc_file_lines {examples/xtmr_manager_selftest_example.c \
				examples/xtmr_manager_intr_tapp_example.c \
				data/tmr_manager_header.h data/tmr_manager_intr_header.h}
	} else {
	    set inc_file_lines {examples/xtmr_manager_selftest_example.c \
				data/tmr_manager_header.h}
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
	set iftmr_managerintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
        set ipname [common::get_property NAME $mhsinst]
	if {$iftmr_managerintr == 1} {
	    set decl "   static XTMR_Manager ${ipname}_TMR_Manager;"
	    set inc_file_lines $decl
	    return $inc_file_lines
	} else {
	    return ""
	}
    }
}

proc gen_testfunc_call {swproj mhsinst} {
    set ipname [common::get_property NAME $mhsinst]
    set iftmr_managerintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
    set testfunc_call ""

    if {$swproj == 0} {
      return $testfunc_call
    }

    set deviceid [::hsi::utils::get_ip_param_name $mhsinst "DEVICE_ID"]
    if {$iftmr_managerintr == 1} {
	set intr_pin_name [hsi::get_pins -of_objects [hsi::get_cells -hier $ipname] \
					 -filter "TYPE==INTERRUPT"]
	set intcname [::hsi::utils::get_connected_intr_cntrl $ipname $intr_pin_name]
        set intcname [hsi::get_mem_ranges -of_objects [hsi::get_cells -hier [hsi::get_sw_processor]] $intcname]
    }

    append testfunc_call "

   {
      int status;

      print(\"\\r\\nRunning TMR_ManagerSelfTestExample() for ${ipname}...\\r\\n\");
      status = TMR_ManagerSelfTestExample(${deviceid});
      if (status == 0) {
         print(\"TMR_ManagerSelfTestExample PASSED\\r\\n\");
      }
      else {
         print(\"TMR_ManagerSelfTestExample FAILED\\r\\n\");
      }

   }"
    if {$iftmr_managerintr == 1} {
	set intr_id "XPAR_${intcname}_${ipname}_${intr_pin_name}_INTR"
	set intr_id [string toupper $intr_id]
        set intcvar intc
	append testfunc_call "
   {
      int Status;

      print(\"\\r\\n Running Interrupt Test  for ${ipname}...\\r\\n\");

      Status = TMR_ManagerIntrExample(&${intcvar}, &${ipname}_TMR_Manager, \\
                                  ${deviceid}, \\
                                  ${intr_id});

      if (Status == 0) {
         print(\"TMR_Manager Interrupt Test PASSED\\r\\n\");
      }
      else {
         print(\"TMR_Manager Interrupt Test FAILED\\r\\n\");
      }

   }"

    }

    return $testfunc_call
}
