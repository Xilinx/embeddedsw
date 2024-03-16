###############################################################################
# Copyright (C) 2014 - 2021 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00  pkp  07/21/14 Initial common::version
# 1.2   mus  02/20/17 Updated tcl to guard xparameters.h by protection macros
# 1.4   ms   04/18/17 Modified tcl file to add suffix U for XPAR_CPU_ID
#                     parameter of cpu_cortexr5 in xparameters.h
# 1.4   srm  02/21/18 Updated freertos to 10.0
# 1.5   asa  03/01/19 Updated to add hard float support for R5 FreeRTOS
#                     BSP.
# 1.5   mus  03/19/19 Updated to add hard float support for IAR R5
#                     BSP.
# 1.6   aru  04/18/19 Updated tcl to add assembler for ARMCC and IAR
# 1.8   dp   06/25/20 Updated tcl to support for armclang
# 1.8   mus  08/18/20 Updated mdd file with new parameter dependency_flags,
#                     it would be used to generate appropriate flags
#                     required for dependency files configuration
# 1.9   mus  05/23/21 Added -fno-tree-loop-distribute-patterns to prevent for loops
#                     to memset conversions. It fixes CR#1090083.
# 2.2   mus  03/15/24 Export flag ZYNQMP_R5_FSBL_BSP to identify R5 FSBL BSP.
##############################################################################
#uses "xillib.tcl"

############################################################
# "generate" procedure
############################################################
proc generate {drv_handle} {

    xdefine_cortexr5_params $drv_handle
    xdefine_include_file $drv_handle "xparameters.h" "XCPU_CORTEXR5" "C_CPU_CLK_FREQ_HZ"
    xdefine_canonical_xpars $drv_handle "xparameters.h" "CPU_CORTEXR5" "C_CPU_CLK_FREQ_HZ"
    xdefine_addr_params_for_ext_intf $drv_handle "xparameters.h"
}

proc xdefine_cortexr5_params {drvhandle} {

    set sw_proc_handle [hsi::get_sw_processor]
    set hw_proc_handle [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc_handle ]]
    set procdrv [hsi::get_sw_processor]
    set archiver [common::get_property CONFIG.archiver $procdrv]
    if {[string first "iarchive" $archiver] < 0 } {
    } else {
	 set libxil_a [file join .. .. lib libxil.a]
	 if { ![file exists $libxil_a] } {
	 # create empty libxil.a
		set fd [open "test.a" a+]
		close $fd
		exec $archiver --create --output $libxil_a test.a
		file delete -force test.a
	}
    }
    set periphs [::hsi::utils::get_common_driver_ips $drvhandle]
    set lprocs [hsi::get_cells -hier -filter {IP_NAME=="psu_cortexr5" || IP_NAME=="psv_cortexr5"}]
    set lprocs [lsort $lprocs]

    set config_inc [::hsi::utils::open_include_file "xparameters.h"]
    puts $config_inc "#ifndef XPARAMETERS_H   /* prevent circular inclusions */"
    puts $config_inc "#define XPARAMETERS_H   /* by using protection macros */"
    puts $config_inc ""
    puts $config_inc "/* Definition for CPU ID */"

    foreach periph $periphs {
        set iname [common::get_property NAME $periph]

	#-----------
	# Set CPU ID
	#-----------
	set id 0
	set uSuffix "U"
	foreach processor $lprocs {
	    if {[string compare -nocase $processor $iname] == 0} {
		puts $config_inc "#define XPAR_CPU_ID $id$uSuffix"
	    }
	    incr id
	}
    }

    close $config_inc

    set oslist [hsi::get_os];
    if { [llength $oslist] != 1 } {
        return 0;
    }
    set os [lindex $oslist 0];

    set procdrv [hsi::get_sw_processor]
    set compiler [common::get_property CONFIG.compiler $procdrv]
    set compiler_name [file tail $compiler]
    set extra_flags [::common::get_property VALUE [hsi::get_comp_params -filter { NAME == extra_compiler_flags } ] ]
    if {[string compare -nocase $compiler_name "iccarm"] == 0} {
	set temp_flag $extra_flags
	if {[string compare -nocase $temp_flag "--debug -DARMR5 --fpu=VFPv3_D16"] != 0} {
		regsub -- {-g -DARMR5 -Wall -Wextra} $temp_flag "" temp_flag
		regsub -- {--debug} $temp_flag "" temp_flag
		regsub -- {-mfloat-abi=hard} $temp_flag "" temp_flag
                regsub -- {-mfpu=vfpv3-d16} $temp_flag "" temp_flag
		regsub -- {--fpu=VFPv3_D16} $temp_flag "" temp_flag
		regsub -- {-DARMR5} $temp_flag "" temp_flag
		regsub -- {-fno-tree-loop-distribute-patterns} $temp_flag "" temp_flag
		set extra_flags "--debug -DARMR5 --fpu=VFPv3_D16 -e $temp_flag"
		common::set_property -name VALUE -value $extra_flags -objects  [hsi::get_comp_params -filter { NAME == extra_compiler_flags } ]
	}

	set compiler_flags [::common::get_property VALUE [hsi::get_comp_params -filter { NAME == compiler_flags } ] ]
	if {[string compare -nocase $compiler_flags "-Om --cpu=Cortex-R5"] != 0} {
		regsub -- {-O2 -c} $compiler_flags "" compiler_flags
		regsub -- {-mcpu=cortex-r5} $compiler_flags "" compiler_flags
		regsub -- {-Om --cpu=Cortex-R5 } $compiler_flags "" compiler_flags
		set compiler_flags "-Om --cpu=Cortex-R5 $compiler_flags"
		common::set_property -name VALUE -value $compiler_flags -objects  [hsi::get_comp_params -filter { NAME == compiler_flags } ]
	}

	set assembler_value "iasmarm"
    common::set_property -name {ASSEMBLER} -value $assembler_value -objects  [hsi::get_sw_processor]
	regsub -all {\{|\}}  {--dependencies=m {$}(@D)/$*.d} "" dependency_flags
	common::set_property -name VALUE -value $dependency_flags -objects  [hsi::get_comp_params -filter { NAME == dependency_flags } ]
   }  elseif {[string compare -nocase $compiler_name "armclang"] == 0} {
	set temp_flag $extra_flags
	if {[string compare -nocase $temp_flag "-g -DARMR5 -Wall -Wextra -mfloat-abi=hard -mfpu=vfpv3-d16 --target=arm-arm-none-eabi"] != 0} {
		regsub -- {-g -DARMR5 -Wall -Wextra} $temp_flag "" temp_flag
		regsub -- {-mfloat-abi=hard} $temp_flag "" temp_flag
		regsub -- {-mfpu=vfpv3-d16} $temp_flag "" temp_flag
		regsub -- {-fno-tree-loop-distribute-patterns} $temp_flag "" temp_flag
		set extra_flags "-g -DARMR5 -Wall -Wextra -mfloat-abi=hard -mfpu=vfpv3-d16 --target=arm-arm-none-eabi $temp_flag"
		common::set_property -name value -value $extra_flags -objects  [hsi::get_comp_params -filter { name == extra_compiler_flags } ]
	}

	set compiler_flags [::common::get_property value [hsi::get_comp_params -filter { name == compiler_flags } ] ]
	if {[string compare -nocase $compiler_flags "-O2 -c -mcpu=cortex-r5"] != 0} {
		regsub -- {-O2 -c} $compiler_flags "" compiler_flags
		regsub -- {-mcpu=cortex-r5} $compiler_flags "" compiler_flags
		regsub -- {-Om --cpu=cortex-r5 } $compiler_flags "" compiler_flags
		set compiler_flags "-O2 -c -mcpu=cortex-r5 $compiler_flags"
		common::set_property -name value -value $compiler_flags -objects  [hsi::get_comp_params -filter { name == compiler_flags } ]
	}
	set assembler_value "armasm"
    common::set_property -name {assembler} -value $assembler_value -objects  [hsi::get_sw_processor]
   } else {
		#Append LTO flag in EXTRA_COMPILER_FLAGS for zynqmp_fsbl_bsp
		set is_zynqmp_fsbl_bsp [common::get_property CONFIG.ZYNQMP_FSBL_BSP [hsi::get_os]]
		if {$is_zynqmp_fsbl_bsp == true} {
			set file_handle [::hsi::utils::open_include_file "xparameters.h"]
			puts $file_handle "#define ZYNQMP_R5_FSBL_BSP"
			close $file_handle

			set extra_flags [common::get_property CONFIG.extra_compiler_flags [hsi::get_sw_processor]]
			#Append LTO flag in EXTRA_COMPILER_FLAGS if not exist previoulsy.
			if {[string first "-flto" $extra_flags] == -1 } {
				append extra_flags " -Os -flto -ffat-lto-objects"
				common::set_property -name {EXTRA_COMPILER_FLAGS} -value $extra_flags -objects [hsi::get_sw_processor]
			}
			set compiler_flags [common::get_property CONFIG.compiler_flags [hsi::get_sw_processor]]
			set substring "-O2"
			set compiler_flags [string map [list $substring ""] $compiler_flags]
			common::set_property -name {COMPILER_FLAGS} -value $compiler_flags -objects [hsi::get_sw_processor]
		}
   }
	# Add "versal" flag to extra compiler flags, if device is versal
	set cortexa72proc [hsi::get_cells -hier -filter {IP_NAME=="psu_cortexa72" || IP_NAME=="psv_cortexa72"}]
	if {[llength $cortexa72proc] > 0} {
		set extra_flags [common::get_property CONFIG.extra_compiler_flags [hsi::get_sw_processor]]
		if {[string first "-Dversal" $extra_flags] == -1 } {
			append extra_flags " -Dversal"
			common::set_property -name {EXTRA_COMPILER_FLAGS} -value $extra_flags -objects [hsi::get_sw_processor]
                }
	}
}

proc xdefine_addr_params_for_ext_intf {drvhandle file_name} {
    set sw_proc_handle [hsi::get_sw_processor]
    set hw_proc_handle [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc_handle ]]

 # Open include file
   set file_handle [::hsi::utils::open_include_file $file_name]

   set mem_ranges [hsi::get_mem_ranges -of_objects $hw_proc_handle]
   foreach mem_range $mem_ranges {
       set inst [common::get_property INSTANCE $mem_range]
       if {$inst != ""} {
            continue
       }


       set bparam_name [common::get_property BASE_NAME $mem_range]
       set bparam_value [common::get_property BASE_VALUE $mem_range]
       set hparam_name [common::get_property HIGH_NAME $mem_range]
       set hparam_value [common::get_property HIGH_VALUE $mem_range]

       # Print all parameters for all peripherals


           set name [string toupper [common::get_property NAME $mem_range]]
	   puts $file_handle ""
           puts $file_handle "/* Definitions for interface [string toupper $name] */"
           set name [format "XPAR_%s_" $name]


           if {$bparam_value != ""} {
               set value [::hsi::utils::format_addr_string $bparam_value $bparam_name]
                   set param [string toupper $bparam_name]
                   if {[string match C_* $param]} {
                       set name [format "%s%s" $name [string range $param 2 end]]
                   } else {
                       set name [format "%s%s" $name $param]
                   }

               puts $file_handle "#define $name $value"
           }

	   set name [string toupper [common::get_property NAME $mem_range]]
           set name [format "XPAR_%s_" $name]
           if {$hparam_value != ""} {
               set value [::hsi::utils::format_addr_string $hparam_value $hparam_name]
                set param [string toupper $hparam_name]
                   if {[string match C_* $param]} {
                       set name [format "%s%s" $name [string range $param 2 end]]
                   } else {
                       set name [format "%s%s" $name $param]
                   }

               puts $file_handle "#define $name $value"
           }


           puts $file_handle ""
      }

    close $file_handle
}

proc post_generate_final {drv_handle} {

	set type [get_property CLASS $drv_handle]
	if {[string equal $type "driver"]} {
	   return
	}

	set file_handle [::hsi::utils::open_include_file "xparameters.h"]
	puts $file_handle "#endif  /* end of protection macro */"
	close $file_handle
}
