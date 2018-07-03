###############################################################################
#
# Copyright (C) 2011 - 2018 Xilinx, Inc.  All rights reserved.
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
# XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Except as contained in this notice, the name of the Xilinx shall not be used
# in advertising or otherwise to promote the sale, use or other dealings in
# this Software without prior written authorization from Xilinx.
#
###############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00a sdm  05/16/10 Updated to support AXI common::version of the core
# 2.0   adk  10/12/13 Updated as per the New Tcl API's
# 2.1	pkp  06/27/14 Updated the tcl to create empty libxil for IAR support in BSP
# 2.2	pkp  02/24/16 Updated tcl for extra compiler flags different toochain
# 2.2	pkp  03/02/16 Append the extra compiler flag only when it contains any
#		      extra flags apart from default ones for linaro toolchain
# 2.2	pkp  03/02/16 Added --cpu=Cortex-A9 compiler flag for iccarm
# 2.2   asa  03/05/16 Updated for accepting only the toolchain name when a
#                     complete path is passed. Also made changes to have
#                     separate case for code sourcery (arm-xilinx-eabi-gcc)
#                     and armcc while generating extra_compiler_flags.
#                     These changes fix CR#939108.
# 2.3	pkp  06/24/16 Updated tcl to remove logic for removing extra space and
#		      update the extra compiler flag for particular compiler only
#		      when some flag apart from default while generating BSP.
#		      These modifications fix CR#951335
# 2.4   pkp  12/23/16 Updated tcl to check each extra compiler flag individually
#		      for linaro toolchain and if any default flags are missing,
#		      it adds the required flags. This change allows users
#		      to modify default flag value. This change fixes CR#965023.
# 2.4   mus  01/24/17 Updated tcl to add "-Wall -Wextra" flags to extra compiler
#                     flags for gcc.
# 2.4   mus  02/20/17 Updated tcl to guard xparameters.h by protection macros
# 2.5   ms   04/18/17 Modified tcl file to add suffix U for XPAR_CPU_ID
#                     parameter of cpu_cortexa9 in xparameters.h
# 2.6   mus  02/20/18 Updated tcl to add "-g" flag in extra compiler flags, for
#                     linaro toolchain. It fixes CR#995214
# 2.7   mus  07/03/18 Updated tcl to not to add default flags forcefully into
#                     extra compiler flags. Now, user can remove default flags
#                     from extra compiler flags. It fixes CR#998768.
##############################################################################
#uses "xillib.tcl"

############################################################
# "generate" procedure
############################################################
proc generate {drv_handle} {

    xdefine_cortexa9_params $drv_handle
    ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XCPU_CORTEXA9" "C_CPU_CLK_FREQ_HZ"
    ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "CPU_CORTEXA9" "C_CPU_CLK_FREQ_HZ"
    xdefine_addr_params_for_ext_intf $drv_handle "xparameters.h"
}

proc xdefine_cortexa9_params {drvhandle} {

    set sw_proc_handle [hsi::get_sw_processor]
    set hw_proc_handle [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc_handle ]]
    set procdrv [hsi::get_sw_processor]
    set compiler [common::get_property CONFIG.compiler $procdrv]
    set compiler_name [file tail $compiler]
    set archiver [common::get_property CONFIG.archiver $procdrv]
    set extra_flags [::common::get_property VALUE [hsi::get_comp_params -filter { NAME == extra_compiler_flags } ] ]
    if {[string compare -nocase $compiler_name "iccarm"] == 0 || [string compare -nocase $compiler_name "armcc"] == 0} {
	set temp_flag $extra_flags
	#
	# Default flags set by mdd parameter extra_compiler_flags are
	# applicable only for gcc compiler. Remove those flags,
	# if compiler is other than gcc.
	#
	if {[string compare -nocase $temp_flag "-mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -nostartfiles -g -Wall -Wextra"] == 0} {
		regsub -- {-mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -nostartfiles -g -Wall -Wextra} $temp_flag "" temp_flag
		set extra_flags $temp_flag
		common::set_property -name VALUE -value $extra_flags -objects  [hsi::get_comp_params -filter { NAME == extra_compiler_flags } ]
	}

	if {[string compare -nocase $compiler_name "iccarm"] == 0} {
		set compiler_flags [::common::get_property VALUE [hsi::get_comp_params -filter { NAME == compiler_flags } ] ]
		if {[string compare -nocase $compiler_flags "-Om --cpu=Cortex-A9"] != 0} {
			regsub -- "-O2 -c" $compiler_flags "" compiler_flags
			regsub -- {-Om --cpu=Cortex-A9 } $compiler_flags "" compiler_flags
			set compiler_flags "-Om --cpu=Cortex-A9 $compiler_flags"
			common::set_property -name VALUE -value $compiler_flags -objects  [hsi::get_comp_params -filter { NAME == compiler_flags } ]
		}
	}
    } else {
	common::set_property -name VALUE -value $extra_flags -objects  [hsi::get_comp_params -filter { NAME == extra_compiler_flags } ]
    }

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
    set lprocs [hsi::get_cells -hier -filter "IP_NAME==ps7_cortexa9"]
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
               set xparam_name [::hsi::utils::format_xparam_name $name]
	       puts $file_handle "#define $xparam_name $value"

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
               set xparam_name [::hsi::utils::format_xparam_name $name]
	       puts $file_handle "#define $xparam_name $value"

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
