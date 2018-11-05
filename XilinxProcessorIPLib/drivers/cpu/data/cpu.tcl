###############################################################################
#
# Copyright (C) 2004 - 2018 Xilinx, Inc.  All rights reserved.
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
# Use of the Software is limited solely to applications:
# (a) running on a Xilinx device, or
# (b) that interact with a Xilinx device through a bus or interconnect.
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
##############################################################################
## @BEGIN_CHANGELOG EDK_L
##    Updated the Tcl to add the bus frequency to xparameters.h
## @END_CHANGELOG
## @BEGIN_CHANGELOG EDK_LS3
##    Updated the Tcl to check for  Extended FPU for pulling in libm compiled
##    with -mhard-float
## @END_CHANGELOG
## @BEGIN_CHANGELOG EDK_MS3
##    Updated the Tcl to pull appropriate libraries for Little Endian Microblaze
## @END_CHANGELOG
##
## MODIFICATION HISTORY:
##
## Ver   Who  Date	 Changes
## ----- ---- -------- -------------------------------------------------------
## 1.04a  asa  07/16/12 Updated the tcl to return 100 MHz for CR 668726 for an
##			IP integrator design when when cpu is directly connected
##			to axi slave peripheral
## 2.0     adk 10/12/13 Updated as per the New Tcl API's
## 2.1     bss 04/14/14 Updated to copy libgloss.a and libgcc.a libraries
## 2.1     bss 04/29/14 Updated to copy libgloss.a if exists otherwise libxil.a
##			 CR#794205
## 2.2     bss 08/04/14 Updated to add protection macros for xparameters.h
##			 CR#802257
## 2.4     nsk 11/05/15 Updated generate and post_generate procs, not to generate
##                      cpu macros, when microblaze is connected as one of
##                      the streaming slaves to itself. CR#876604
## 2.5     asa 04/20/16 Fix for CR#947179. While populating the CPU_CORE_FREQ first
##                      look for "Clk" pin and if not found use the CONFIG param
##                      C_FREQ for microblaze to populate the CPU_CORE_FREQ_HZ.
## 2.7     vns 04/13/18 Modified post_generate proc to post_generate_final
## 2.7     mus 04/17/18 Updated the generate proc to add HW parameter based compiler
##                      flags for microblaze. Till now this setting was being done by
##                      HSI.
# uses xillib.tcl

########################################
# Make file writable
########################################
proc make_writable {osname filename} {

    if {[string first "win64" $osname] != -1  || [string first "win" $osname] != -1 } {
        file attributes $filename -readonly no
    } else {
        file attributes $filename -permissions ugo+w
    }
}

proc generate {drv_handle} {
    global env
    global tcl_platform

    set osname "[::hsi::utils::get_hostos_platform]/"

    # Don't generate cpu macros, when microblaze itself is connected
    # as one of the streaming slave to itself.
    # when CLASS type of processor is driver ignore, and if it is of type
    # cpu then continue generation.
    set class_type [get_property CLASS $drv_handle]
    if {[string equal $class_type "driver"]} {
	puts "WARNING : Processor $drv_handle is connected as one of the streaming slaves\
	      to itself \n"
	return
    }
    #---------------------------------------------------------------------------
    # Start of mb-gcc specific processing..
    # 1. Copy libc, libm and libxil files..
    # 2. Generate the attribute interrupt_handler for the interrupting source...
    #---------------------------------------------------------------------------
    set compiler [common::get_property CONFIG.compiler $drv_handle]
    # preserve case
    set temp $compiler
    set compiler [string tolower $compiler]
    if { $compiler == "mb-gcc" || $compiler == "mb-g++" || $compiler == "mb-c++" } {
        # If the user has just specified the compiler without specifying a path,
        # we default to the compiler root being the EDK installation
        if {[string compare "mb-gcc" $compiler] == 0 ||
            [string compare "mb-g++" $compiler] == 0 ||
            [string compare "mb-c++" $compiler] == 0 } {

            set compiler_root ""
            set xilinx_edk_gnu [array get env XILINX_EDK_GNU]
			set gnu_osdir $osname
			if {[string first "win64" $osname] != -1  || [string first "win" $osname] != -1 } {
			    set gnu_osdir "nt"
			} elseif {[string first "lnx64" $osname] != -1   || [string first "lnx" $osname] != -1 } {
				set gnu_osdir "lin"
			}
            if { $xilinx_edk_gnu == "" } {
                append compiler_root $env(XILINX_SDK) "/gnu/microblaze/" $gnu_osdir
            } {
                append compiler_root $env(XILINX_EDK_GNU) "/microblaze/" $gnu_osdir
            }
        } else {
            set compiler_root [file dirname $temp]

            ## Big time kludge here. We rely on the compiler toolchain name staying the same forever here.
            set compiler_root [string range $compiler_root 0 [expr [string length $compiler_root] - 4]]
        }
	puts $compiler_root
        # Copy the library files - libc.a, libm.a, libxil.a
        set shifter ""
        set multiplier ""
        set libxil_shifter ""
        set libxil_multiplier ""
        set pattern ""
        set fpu ""

        set libc "libc"
        set libm "libm"

        set sw_proc_handle [hsi::get_sw_processor]
        set periph [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc_handle]]
        set proctype [common::get_property IP_NAME $periph]

        set endian [common::get_property CONFIG.C_ENDIANNESS $periph]
        if {[string compare -nocase "1" $endian] == 0 } {
            set endian "_le"
                set libxil_endian "le"
            } else {
                set endian ""
                set libxil_endian ""
            }

        set shift [common::get_property CONFIG.C_USE_BARREL $periph]
        if {[string compare -nocase "1" $shift] == 0 } {
            set shifter "_bs"
                set libxil_shifter "bs"
        }

        set hard_float [common::get_property CONFIG.C_USE_FPU $periph]
        if {[string compare -nocase "1" $hard_float] == 0 || [string compare -nocase "2" $hard_float] == 0} {
            set fpu "_fpd"
        }

        set pcmp [common::get_property CONFIG.C_USE_PCMP_INSTR $periph]
        if {[string compare -nocase "1" $pcmp] == 0 } {
            set pattern "_p"
        }

        #-------------------------------------------------
        # Check if MULTIPLIER PARAMETER is set in MSS file
        # If so, then use it. Else find the C_FAMILY
        # and set the multiplier accordingly
        #-------------------------------------------------
        set multiply [common::get_property CONFIG.C_USE_HW_MUL $periph]
        if {[string compare -nocase "" $multiply] == 0 } {
            set family [string tolower [common::get_property CONFIG.C_FAMILY $periph]
            if {[string first "virtex" $family] >= 0 } {
            if {[string compare -nocase "virtexe" $family] == 0 } {
                set multiplier ""
                set libxil_multiplier ""

            } else {
                set multiplier "_m"
                set libxil_multiplier "m"
            }
        } elseif {[string compare -nocase "spartan3" $family] == 0 } {
            set multiplier "_m"
            set libxil_multiplier "m"
	    }
	} elseif {[string compare -nocase "1" $multiply] == 0 } {
	    set multiplier "_m"
            set libxil_multiplier "m"
	}

	set libc [format "%s%s%s%s%s%s" $libc $endian $multiplier $shifter $pattern ".a"]
	set libm [format "%s%s%s%s%s%s%s" $libm $endian $multiplier $shifter $pattern $fpu ".a"]
	set libxil "libgloss.a"
	set libgcc "libgcc.a"
	set targetdir "../../lib/"

	#------------------------------------------------------
	# Copy libc, libm , libxil files...
        #
        # There are checks and flows to handle differences in
        # the GCC 3.4.1 toolchain (EDK 9.1i and prior) and
        # the GCC 4.1.1 toolchain (EDK 9.2i and later)
	#------------------------------------------------------
	set libcfilename [format "%s%s" $targetdir "libc.a"]
	set libmfilename [format "%s%s" $targetdir "libm.a"]

    set library_dir [file join $compiler_root "microblaze/lib"]

    if { ![file exists $library_dir] } {
        set library_dir [file join $compiler_root "microblaze-xilinx-elf/lib"]
         if { ![file exists $library_dir] } {
	            error "Couldn't figure out compiler's library directory" "" "hsi_error"
        }
        set libgcc_dir [file join $compiler_root "lib/gcc/microblaze-xilinx-elf"]
        set libgcc_dir [glob -dir $libgcc_dir *]
        if { ![file exists $libgcc_dir] } {
            error "Couldn't figure out compiler's GCC library directory" "" "hsi_error"
        }
    }


   file copy -force [file join $library_dir $libc] $libcfilename
   make_writable $osname $libcfilename

   file copy -force [file join $library_dir $libm] $libmfilename
   make_writable $osname $libmfilename

   set libxil_path [file join $library_dir $libxil_shifter $libxil_multiplier $libxil_endian $libxil]
   set symlink [file type $libxil_path]
   if { ![file exists $libxil_path] || $symlink == "link"} {
	# no libgloss.a in older SDK use libxil.a
	set libxil "libxil.a"
	set libxil_path [file join $library_dir $libxil_shifter $libxil_multiplier $libxil_endian $libxil]
   }
   set libgcc_path [file join $libgcc_dir $libxil_shifter $libxil_multiplier $libxil_endian $libgcc]
   if { ![file exists $libxil_path] } {
	set libxil_path [file join $env(XILINX_SDK) "data/embeddedsw/lib/microblaze/" $libxil]
   }

    file copy -force $libxil_path $targetdir
    make_writable $osname [file join $targetdir $libxil]
    file copy -force $libgcc_path $targetdir
    make_writable $osname [file join $targetdir $libgcc]

    } else {
	error  "ERROR: Wrong compiler type selected please use mb-gcc or mb-g++ or mb-c++"
	return;
    }



    # End of mb-gcc specific processing...

    #------------------------------------
    # Handle xmdstub generation
    #------------------------------------
    set xmdstub_periph [common::get_property CONFIG.xmdstub_peripheral $drv_handle]
    if {[string compare -nocase "none" $xmdstub_periph] != 0 } {
        set xmdstub_periph_handle [xget_hwhandle $xmdstub_periph]
        set targetdir "../../code"
        set filename "xmdstub.s"
        file copy -force [file join $env(XILINX_SDK) "data/embeddedsw/lib/microblaze/src" $filename] $targetdir
            file mtime [file join $targetdir $filename] [clock seconds]
        set filename "make.xmdstub"
        file copy -force [file join $env(XILINX_SDK) "data/embeddedsw/lib/microblaze/src" $filename] $targetdir
            file mtime [file join $targetdir $filename] [clock seconds]
        set xmd_addr_file [open "../../code/xmdstubaddr.s" w]
        set xmdstub_periph_baseaddr [::hsi::utils::format_addr_string [xget_value $xmdstub_periph_handle "PARAMETER" "C_BASEADDR"] "C_BASEADDR"]
        puts $xmd_addr_file ".equ DEBUG_PERIPHERAL_BASEADDRESS, $xmdstub_periph_baseaddr"
        close $xmd_addr_file
        # execute make
        set pwd [pwd]
        if [catch {cd "../../code"} err] {
	    error "Couldn't cd to code directory: $err" "" "hsi_error"
	    return
        }
        if [catch {exec make -f make.xmdstub xmdstub} err] {
            error "Couldn't make xmdstub: $err" "" "hsi_error"
            return
        }
        cd $pwd
    }

    # Setup the compiler flags as per HW Params
    set endian [common::get_property CONFIG.C_ENDIANNESS $periph]
    set shift [common::get_property CONFIG.C_USE_BARREL $periph]
    set pcmp [common::get_property CONFIG.C_USE_PCMP_INSTR $periph]

    set vlnv_string [common::get_property VLNV $periph]
    set cpu_version [lindex [lreverse [split $vlnv_string :]] 0]

    set compiler_flags ""

    if {[string compare -nocase "1" $endian] == 0 } {
		append compiler_flags " -mlittle-endian"
    }

    if {[string compare -nocase "1" $shift] == 0 } {
    	append compiler_flags " -mxl-barrel-shift"
    }

    if {[string compare -nocase "1" $pcmp] == 0 } {
    	append compiler_flags " -mxl-pattern-compare"
    }

    if {[string compare "psu_pmu" $proctype] == 0} {
    	set multiply [common::get_property CONFIG.C_USE_HW_MUL $periph]
	if {[string compare -nocase "0" $multiply] == 0 } {
		append compiler_flags " -mxl-soft-mul"
	}
    } elseif {[string compare "microblaze" $proctype] == 0 } {
	set hwmul [common::get_property CONFIG.C_USE_HW_MUL $periph]
	set reorder [common::get_property CONFIG.C_USE_REORDER_INSTR $periph]
	set hard_float [common::get_property CONFIG.C_USE_FPU $periph]
	set hard_div [common::get_property CONFIG.C_USE_DIV $periph]
	set freq_opt [common::get_property CONFIG.C_AREA_OPTIMIZED $periph]

	if {[string compare -nocase "1" $hwmul] == 0 || [string compare -nocase "2" $hwmul] == 0} {
		append compiler_flags " -mno-xl-soft-mul"
	} else {
		append compiler_flags " -mxl-soft-mul"
	}
	if {[string compare -nocase "2" $hwmul] == 0} {
		append compiler_flags " -mxl-multiply-high"
	}

	if {[string compare -nocase "0" $reorder] == 0 } {
		append compiler_flags " -mno-xl-reorder"
	}

	if {[string compare -nocase "1" $hard_float] == 0 } {
		append compiler_flags " -mhard-float"
	} elseif {[string compare -nocase "2" $hard_float] == 0} {
		append compiler_flags " -mhard-float -mxl-float-convert -mxl-float-sqrt"
	}

	if {[string compare -nocase "1" $hard_div] == 0 } {
		append compiler_flags " -mno-xl-soft-div"
	}

	if {[string compare -nocase "2" $freq_opt] == 0 } {
		append compiler_flags " -mxl-frequency"
	}
    }
    append compiler_flags " -mcpu=v" $cpu_version

    common::set_property CONFIG.compiler_flags $compiler_flags $drv_handle

	#------------------------------------------------------------------------------
	# If the processor is PMU Microblaze, then generate required params and return
	# We dont need the Parameters being generated after this code block
	#------------------------------------------------------------------------------
	if {[string compare "psu_pmu" $proctype] == 0} {

		# Generate the Parameters
		set file_handle [::hsi::utils::open_include_file "xparameters.h"]
		puts $file_handle "#ifndef XPARAMETERS_H   /* prevent circular inclusions */"
		puts $file_handle "#define XPARAMETERS_H   /* by using protection macros */"
		puts $file_handle ""
		set params [list]
		lappend reserved_param_list "C_DEVICE" "C_PACKAGE" "C_SPEEDGRADE" "C_FAMILY" "C_INSTANCE" "C_KIND_OF_EDGE" "C_KIND_OF_LVL" "C_KIND_OF_INTR" "C_NUM_INTR_INPUTS" "C_MASK" "C_NUM_MASTERS" "C_NUM_SLAVES" "C_LMB_AWIDTH" "C_LMB_DWIDTH" "C_LMB_MASK" "C_LMB_NUM_SLAVES" "INSTANCE" "HW_VER"
		# Print all parameters for psu_pmu with XPAR_MICROBLAZE prefix
		puts $file_handle ""
		puts $file_handle "/* Definitions for PMU Microblaze */"
		set params ""
		set params [common::list_property $periph CONFIG.*]
		foreach param $params {
			set param_name [string range $param [string length "CONFIG."] [string length $param]]
			set posn [lsearch -exact $reserved_param_list $param_name]
			if {$posn == -1 } {
				set param_value [common::get_property $param $periph]
				if {$param_value != ""} {
					set param_value [::hsi::utils::format_addr_string $param_value $param_name]
					set name "XPAR_MICROBLAZE_"
					if {[string match C_* $param_name]} {
						set name [format "%s%s" $name [string range $param_name 2 end]]
					} else {
						set name [format "%s%s" $name $param_name]
					}
					puts $file_handle "#define [string toupper $name] $param_value"
				}
			}
		}
		# Define the Clock Param
		puts $file_handle "\n/******************************************************************/\n"
		puts $file_handle "#define XPAR_CPU_CORE_CLOCK_FREQ_HZ XPAR_MICROBLAZE_FREQ"
		puts $file_handle "\n/******************************************************************/\n"
		close $file_handle
		# We have generated all the params required for PMU MICROBLAZE. So just return
		return
	}

    #--------------------------
    # Handle the Bus Frequency
    #--------------------------
    set file_handle [::hsi::utils::open_include_file "xparameters.h"]
    puts $file_handle "#ifndef XPARAMETERS_H   /* prevent circular inclusions */"
    puts $file_handle "#define XPARAMETERS_H   /* by using protection macros */"
    puts $file_handle ""
    puts $file_handle "/* Definitions for bus frequencies */"
    set bus_array {"M_AXI_DP" "M_AXI_IP"}
    set bus_freq [::hsi::utils::get_clk_pin_freq $periph "Clk"]
    if {[llength $bus_freq] == 0} {
        set bus_freq "100000000"
    }

    foreach bus_inst $bus_array {
        set bhandle [hsi::get_intf_pins $bus_inst -of_objects $periph]
        if { $bhandle == "" } {
          continue;
        }
       puts $file_handle "#define [::hsi::utils::get_driver_param_name "cpu" [format "%s_FREQ_HZ" $bus_inst]] $bus_freq"
    }

    puts $file_handle "/******************************************************************/"
    puts $file_handle ""
    puts $file_handle "/* Canonical definitions for bus frequencies */"
    set bus_id 0
    foreach bus $bus_array {
        set bhandle [hsi::get_intf_pins $bus_inst -of_objects $periph]
        if { $bhandle == "" } {
          continue;
        }
        puts $file_handle "#define [::hsi::utils::get_driver_param_name "PROC_BUS" [format "%d_FREQ_HZ" $bus_id]] $bus_freq"
        incr bus_id
    }
    puts $file_handle "/******************************************************************/"
    puts $file_handle ""

    #--------------------------
    # define CORE_CLOCK_FREQ_HZ
    #--------------------------
    set clk_freq [::hsi::utils::get_clk_pin_freq $periph "Clk"]
    if {[llength $clk_freq] == 0} {
	set clk_freq [common::get_property CONFIG.C_FREQ $periph]
    }
    puts $file_handle "#define [::hsi::utils::get_driver_param_name "cpu" CORE_CLOCK_FREQ_HZ] $clk_freq"
    puts $file_handle "#define [format "XPAR_%s_CORE_CLOCK_FREQ_HZ" [string toupper $proctype]] $clk_freq"

    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle

    #--------------------------
    # define all params
    #--------------------------
    ::hsi::utils::define_all_params $drv_handle "xparameters.h"

    #----------------------------------------
    # define all params without instance name
    #----------------------------------------
    ::hsi::utils::define_processor_params $drv_handle "xparameters.h"
    xdefine_addr_params_for_ext_intf $drv_handle "xparameters.h"
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

# Returns the frequency of a bus
#proc xget_busfreq {bus_name} {
#       set bus_freq ""
#       set bus_handle [xget_hwhandle $bus_name]
#       if {$bus_handle == ""} {
#           puts "WARNING: Bus Clock frequency information is not available in the design, for $bus_name. Assuming a default frequency of 100MHz\n"
#           return 100000000
#       }
#
#       set bus_type [xget_hw_value $bus_handle]
#
#       if { $bus_type == "axi_interconnect" } {
#           set port_name "INTERCONNECT_ACLK"
#       }
#       set clkhandle [xget_hw_port_handle $bus_handle $port_name]
#       if { [string compare -nocase $clkhandle ""] != 0 } {
#           set bus_freq [xget_hw_subproperty_value $clkhandle "CLK_FREQ_HZ"]
#       }
#       return $bus_freq
#}

# Returns the frequency of a bus for IPI system
#proc xget_bus_freq_value {periph_handle bus_name} {
#       set bus_freq ""
#       set bus_handle [xget_hw_ipinst_handle $periph_handle $bus_name]
#   if {$bus_handle == ""} {
#       puts "WARNING: Bus Clock frequency information is not available in the design, for $bus_name. Assuming a default frequency of 100MHz\n"
#       return 100000000
#       }
#
#       set bus_type [xget_hw_value $bus_handle]
#
#       if { $bus_type == "plb_v34" || $bus_type == "plb_v46" } {
#           set port_name "PLB_Clk"
#       } elseif { $bus_type == "axi_interconnect" } {
#           set port_name "INTERCONNECT_ACLK"
#       } elseif { $bus_type == "axi_crossbar" } {
#           set port_name "ACLK"
#       } else {
#           set port_name "OPB_Clk"
#       }
#       set clkhandle [xget_hw_port_handle $bus_handle $port_name]
#       if { [string compare -nocase $clkhandle ""] != 0 } {
#           set bus_freq [xget_hw_subproperty_value $clkhandle "CLK_FREQ_HZ"]
#       }
#       return $bus_freq
#}
