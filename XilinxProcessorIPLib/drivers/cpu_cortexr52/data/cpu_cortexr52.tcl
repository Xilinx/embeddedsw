###############################################################################
# Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.0   mus  12/07/21 Initial common::version
##############################################################################
#uses "xillib.tcl"

############################################################
# "generate" procedure
############################################################
proc generate {drv_handle} {

    xdefine_cortexr52_params $drv_handle
    xdefine_include_file $drv_handle "xparameters.h" "XCPU_CORTEXR52" "C_CPU_CLK_FREQ_HZ" "C_TIMESTAMP_CLK_FREQ"
    xdefine_canonical_xpars $drv_handle "xparameters.h" "CPU_CORTEXR52" "C_CPU_CLK_FREQ_HZ" "C_TIMESTAMP_CLK_FREQ"
    xdefine_addr_params_for_ext_intf $drv_handle "xparameters.h"
}

proc xdefine_cortexr52_params {drvhandle} {

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
    set lprocs [hsi::get_cells -hier -filter {IP_NAME=="psxl_cortexr52" || IP_NAME=="psx_cortexr52"}]
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
	if {[string compare -nocase $temp_flag "--debug -DARMR5 -DARMR52 --fpu=VFPv3_D16"] != 0} {
		regsub -- {-g -DARMR5 -DARMR52 -Wall -Wextra} $temp_flag "" temp_flag
		regsub -- {--debug} $temp_flag "" temp_flag
		regsub -- {-mfloat-abi=hard} $temp_flag "" temp_flag
                regsub -- {-mfpu=vfpv3-d16} $temp_flag "" temp_flag
		regsub -- {--fpu=VFPv3_D16} $temp_flag "" temp_flag
		regsub -- {-DARMR52} $temp_flag "" temp_flag
		regsub -- {-fno-tree-loop-distribute-patterns} $temp_flag "" temp_flag
		set extra_flags "--debug -DARMR5 -DARMR52 --fpu=VFPv3_D16 -e $temp_flag"
		common::set_property -name VALUE -value $extra_flags -objects  [hsi::get_comp_params -filter { NAME == extra_compiler_flags } ]
	}

	set compiler_flags [::common::get_property VALUE [hsi::get_comp_params -filter { NAME == compiler_flags } ] ]
	if {[string compare -nocase $compiler_flags "-Om --cpu=Cortex-R52"] != 0} {
		regsub -- {-O2 -c} $compiler_flags "" compiler_flags
		regsub -- {-mcpu=cortex-r52} $compiler_flags "" compiler_flags
		regsub -- {-Om --cpu=Cortex-R52 } $compiler_flags "" compiler_flags
		set compiler_flags "-Om --cpu=Cortex-R52 $compiler_flags"
		common::set_property -name VALUE -value $compiler_flags -objects  [hsi::get_comp_params -filter { NAME == compiler_flags } ]
	}

	set assembler_value "iasmarm"
    common::set_property -name {ASSEMBLER} -value $assembler_value -objects  [hsi::get_sw_processor]
	regsub -all {\{|\}}  {--dependencies=m {$}(@D)/$*.d} "" dependency_flags
	common::set_property -name VALUE -value $dependency_flags -objects  [hsi::get_comp_params -filter { NAME == dependency_flags } ]
   }  elseif {[string compare -nocase $compiler_name "armclang"] == 0} {
	set temp_flag $extra_flags
	if {[string compare -nocase $temp_flag "-g -DARMR5 -DARMR52 -Wall -Wextra -mfloat-abi=hard -mfpu=vfpv3-d16 --target=arm-arm-none-eabi"] != 0} {
		regsub -- {-g -DARMR5 -DARMR52 -Wall -Wextra} $temp_flag "" temp_flag
		regsub -- {-mfloat-abi=hard} $temp_flag "" temp_flag
		regsub -- {-mfpu=vfpv3-d16} $temp_flag "" temp_flag
		regsub -- {-fno-tree-loop-distribute-patterns} $temp_flag "" temp_flag
		set extra_flags "-g -DARMR5 -DARMR52 -Wall -Wextra -mfloat-abi=hard -mfpu=vfpv3-d16 --target=arm-arm-none-eabi $temp_flag"
		common::set_property -name value -value $extra_flags -objects  [hsi::get_comp_params -filter { name == extra_compiler_flags } ]
	}

	set compiler_flags [::common::get_property value [hsi::get_comp_params -filter { name == compiler_flags } ] ]
	if {[string compare -nocase $compiler_flags "-O2 -c -mcpu=cortex-r52"] != 0} {
		regsub -- {-O2 -c} $compiler_flags "" compiler_flags
		regsub -- {-mcpu=cortex-r52} $compiler_flags "" compiler_flags
		regsub -- {-Om --cpu=cortex-r52 } $compiler_flags "" compiler_flags
		set compiler_flags "-O2 -c -mcpu=cortex-r52 $compiler_flags"
		common::set_property -name value -value $compiler_flags -objects  [hsi::get_comp_params -filter { name == compiler_flags } ]
	}
	set assembler_value "armasm"
    common::set_property -name {assembler} -value $assembler_value -objects  [hsi::get_sw_processor]
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
