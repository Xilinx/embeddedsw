###############################################################################
# Copyright (C) 2014 - 2021 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 3.0   pkp  12/01/15 Initial Release
# 3.13  mus  02/03/21 Updated to support CIPS3 based HW designs. It fixes
#                     CR#1087461
# 3.15  adk  14/12/21 Updated checks to support versal R5 processor. It fixes
# 		      CR#1117725
# 3.21   ml  12/19/24 In Versal designs, exclude last TTC instance variable for
#                     R5 processor to fix the GCC warning [wunused variable]
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
    # exclude ttc3 if present for cortexR5 since R5 uses ttc3 for sleep when present

	set periph_include 1
	set procdrv [hsi::get_sw_processor]
	if {[string match "*psu_cortexr5_*" $procdrv]} {
		if {[string compare -nocase "psu_ttc_3" $mhsinst] == 0} {
			set periph_include 0
		}
	}
        set isintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
	if {$periph_include && $isintr} {
		set inc_file_lines {xttcps.h ttcps_header.h}
		return $inc_file_lines
	} else {
		return ""
        }
    }
}

proc gen_src_files {swproj mhsinst} {
  if {$swproj == 0} {
    return ""
  }
  if {$swproj == 1} {
  # exclude ttc3 if present for cortexR5 since R5 uses ttc3 for sleep when present
	set periph_include 1
	set procdrv [hsi::get_sw_processor]
	if {[string match "*psu_cortexr5_*" $procdrv] || [string match "*psv_cortexr5_*" $procdrv]} {
		if {[string compare -nocase "psu_ttc_3" $mhsinst] == 0 || [string match -nocase "*psv_ttc_3" $mhsinst]} {
			set periph_include 0
		}
	}
        set isintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
	if {$periph_include && $isintr} {
		set inc_file_lines {examples/xttcps_tapp_example.c data/ttcps_header.h}
		return $inc_file_lines
	} else {
		return ""
	}
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
    # exclude ttc3 if present for cortexR5 since R5 uses ttc3 for sleep when present
	set periph_include 1
	set procdrv [hsi::get_sw_processor]
	if {[string match "*psu_cortexr5_*" $procdrv] || [string match "*psv_cortexr5_*" $procdrv]} {
		if {[string compare -nocase "psu_ttc_3" $mhsinst] == 0 || [string match -nocase "*psv_ttc_3" $mhsinst]} {
			set periph_include 0
		}
	}
        set isintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
	if {$periph_include && $isintr} {
		set ipname [get_property NAME $mhsinst]
		set decl "   static XTtcPs ${ipname};"
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

    set ipname [get_property NAME $mhsinst]
    set deviceid [::hsi::utils::get_ip_param_name $mhsinst "DEVICE_ID"]
    set stdout [get_property CONFIG.STDOUT [get_os]]
    if { $stdout == "" || $stdout == "none" } {
       set hasStdout 0
    } else {
       set hasStdout 1
    }

    set intsnum [string index $mhsinst end]
    set device_id [expr {$intsnum * 3}]
    set deviceidname $deviceid
    set deviceid [format "XPAR_%s_%d_DEVICE_ID" [string toupper [string range [common::get_property NAME $mhsinst] 0 end-2]] $device_id]
    set isintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
    set intcvar intc
    set testfunc_call ""
    # exclude ttc3 if present for cortexR5 since R5 uses ttc3 for sleep when present
    set periph_include 1
    set procdrv [hsi::get_sw_processor]
    set ip_name [get_property IP_NAME $mhsinst]
    set ip_name [format %s_%s $ip_name $intsnum]
    if {[string match "*psu_cortexr5_*" $procdrv] || [string match "*psv_cortexr5_*" $procdrv]} {
	if {[string compare -nocase "psu_ttc_3" $mhsinst] == 0 } {
		set periph_include 0
	} elseif {[string compare -nocase "psv_ttc_3" $ip_name] == 0} {
		set periph_include 0
	}
    }
    if {$periph_include} {

    if {${hasStdout} == 0} {

	if {$isintr == 1} {
        set intr_id [format XPAR_XTTCPS_%s_INTR $device_id]
	set intr_id [string toupper $intr_id]

      append testfunc_call "

   {
      int Status;
      Status = TmrInterruptExample(&$ipname, \\
				$deviceid, \\
				$intr_id, &${intcvar});
   }"

   }


  } else {

	if {$isintr == 1} {
        set intr_id [format XPAR_XTTCPS_%s_INTR $device_id]
	set intr_id [string toupper $intr_id]

      append testfunc_call "
   {
      int Status;

      print(\"\\r\\n Running Interrupt Test  for ${ipname}...\\r\\n\");

      Status = TmrInterruptExample(&$ipname, \\
				$deviceid, \\
				$intr_id, &${intcvar});

      if (Status == 0) {
         print(\"TtcIntrExample PASSED\\r\\n\");
      }
      else {
         print(\"TtcIntrExample FAILED\\r\\n\");
      }

   }"

   }

 }
}
  return $testfunc_call
}
