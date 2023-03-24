##############################################################################
# Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
#
# MODIFICATION HISTORY:
#
# Ver   Who  Date     Changes
# ----- ---- -------- -------------------------------------------------------
# 1.0   sa   02/22/23 Initial version
#
# uses xillib.tcl
##############################################################################

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

    # Don't generate cpu macros, when the processor itself is connected
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
    # Start of riscv64-unknown-elf-gcc specific processing..
    # 1. Copy libc, libm and libxil files..
    # 2. Generate the attribute interrupt_handler for the interrupting source...
    #---------------------------------------------------------------------------
    set compiler [common::get_property CONFIG.compiler $drv_handle]
    # preserve case
    set temp $compiler
    set compiler [string tolower $compiler]
    if { $compiler == "riscv64-unknown-elf-gcc" || $compiler == "riscv64-unknown-elf-g++" || $compiler == "riscv64-unknown-elf-c++" } {
        # If the user has just specified the compiler without specifying a path,
        # we default to the compiler root being the Vitis installation
        if {[string compare "riscv64-unknown-elf-gcc" $compiler] == 0 ||
            [string compare "riscv64-unknown-elf-g++" $compiler] == 0 ||
            [string compare "riscv64-unknown-elf-c++" $compiler] == 0 } {

            set compiler_root ""

            set xilinx_gnu [array get env XILINX_GNU]
            set xilinx_sdk [array get env XILINX_SDK]
            set xilinx_vitis [array get env XILINX_VITIS]
            set xilinx_approot [array get env HDI_APPROOT]

            set gnu_osdir $osname
            if {[string first "win64" $osname] != -1  || [string first "win" $osname] != -1 } {
                set gnu_osdir "nt"
            } elseif {[string first "lnx64" $osname] != -1   || [string first "lnx" $osname] != -1 } {
                set gnu_osdir "lin"
            }
            if { $xilinx_gnu != "" } {
                append compiler_root $env(XILINX_GNU) "/gnu/riscv/" $gnu_osdir
            } elseif { $xilinx_approot != "" } {
                append compiler_root $env(HDI_APPROOT) "/gnu/riscv/" $gnu_osdir
            } elseif { $xilinx_vitis != "" } {
                append compiler_root $env(XILINX_VITIS) "/gnu/riscv/" $gnu_osdir
            } elseif { $xilinx_sdk != "" } {
                append compiler_root $env(XILINX_SDK) "/gnu/riscv/" $gnu_osdir
            }
        } else {
            set compiler_root [file dirname $temp]

            ## Big time kludge here. We rely on the compiler toolchain name staying the same forever here.
            set compiler_root [string range $compiler_root 0 [expr [string length $compiler_root] - 4]]
        }
        puts $compiler_root

        # Copy the library files - libc.a, libm.a, libxil.a
        set arch "i"
        set abi ""

        set libc "libc"
        set libm "libm"

        set sw_proc_handle [hsi::get_sw_processor]
        set periph [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc_handle]]
        set proctype [common::get_property IP_NAME $periph]

        set use_muldiv [common::get_property CONFIG.C_USE_MULDIV $periph]
        if {[string compare "1" $use_muldiv] == 0 || [string compare "2" $use_muldiv] == 0} {
            append arch "m"
        }

        set use_atomic [common::get_property CONFIG.C_USE_ATOMIC $periph]
        if {[string compare "1" $use_atomic] == 0} {
            append arch "a"
        }

        set hard_float [common::get_property CONFIG.C_USE_FPU $periph]
        if {[string compare "1" $hard_float] == 0} {
            append arch "f"
            append abi "f"
        }
        if {[string compare "2" $hard_float] == 0} {
            append arch "fd"
            append abi "d"
        }

        set use_compr  [common::get_property CONFIG.C_USE_COMPRESSION $periph]
        if {[string compare "1" $use_compr] == 0} {
            append arch "c"
        }

        set data_size [common::get_property CONFIG.C_DATA_SIZE $periph]
        if {[string compare "32" $data_size] == 0 } {
            set arch "rv32$arch"
            set abi "ilp32$abi"
        }
        if {[string compare "64" $data_size] == 0 } {
            set arch "rv64$arch"
            set abi "lp64$abi"
        }

        set libc [file join $arch $abi "${libc}.a"]
        set libm [file join $arch $abi "${libm}.a"]
        set libxil "libgloss.a"
        set libgcc "libgcc.a"
        set targetdir "../../lib/"

        #------------------------------------------------------
        # Copy libc, libm , libxil files...
        #
        # rv32i/ilp32
        # rv32im/ilp32
        # rv32iac/ilp32
        # rv32imac/ilp32
        # rv32imafc/ilp32f
        # rv64imac/lp64
        # rv64imafdc/lp64d
        #------------------------------------------------------
        set libcfilename [file join $targetdir "libc.a"]
        set libmfilename [file join $targetdir "libm.a"]

        set library_dir [file join $compiler_root "riscv64-unknown-elf/riscv32-xilinx-elf/usr/lib"]

        set libgcc_dir $library_dir
        if { ![file exists $library_dir] } {
            set library_dir [file join $compiler_root "riscv64-unknown-elf/lib"]
            if { ![file exists $library_dir] } {
                error "Couldn't figure out compiler's library directory: tried \"$library_dir\"" "" "hsi_error"
            }
            set libgcc_dir_root [file join $compiler_root "lib/gcc/riscv64-unknown-elf"]
            set libgcc_dirs [glob -dir $libgcc_dir_root *]
            set dumpversion [exec $compiler -dumpversion]
            foreach libgcc_dir $libgcc_dirs {
                if { [file exists $libgcc_dir] && [string first $dumpversion $libgcc_dir] > 0 } {
                    break
                }
            }
            if { ![file exists "$libgcc_dir"] } {
                error "Couldn't figure out compiler's GCC library directory: tried \"$libgcc_dir\"" "" "hsi_error"
            }
        }

        if {[file exist [file join $library_dir $libc]]} {
            file copy -force [file join $library_dir $libc] $libcfilename
            make_writable $osname $libcfilename
        } else {
            error "Couldn't figure out compiler's library directory: tried \"[file join $library_dir $libc]\"" "" "hsi_error"
        }
        if {[file exist [file join $library_dir $libm]]} {
            file copy -force [file join $library_dir $libm] $libmfilename
            make_writable $osname $libmfilename
        } else {
            error "Couldn't figure out compiler's library directory: tried \"[file join $library_dir $libm]\"" "" "hsi_error"
        }

        set libxil_path [file join $library_dir $arch $abi $libxil]
        set symlink [file type $libxil_path]
        if { ![file exists $libxil_path] || $symlink == "link"} {
            # no libgloss.a in older SDK use libxil.a
            set libxil "libxil.a"
            set libxil_path [file join $library_dir $arch $abi $libxil]
        }
        if { ![file exists $libxil_path] } {
            if { $xilinx_gnu != "" } {
                set libxil_path [file join $env(XILINX_GNU) "data/embeddedsw/lib/riscv/" $libxil]
            } elseif { $xilinx_approot != "" } {
                set libxil_path [file join $env(HDI_APPROOT) "data/embeddedsw/lib/riscv/" $libxil]
            } elseif { $xilinx_vitis != "" } {
                set libxil_path [file join $env(XILINX_VITIS) "data/embeddedsw/lib/riscv/" $libxil]
            } elseif  { $xilinx_sdk != "" } {
                set libxil_path [file join $env(XILINX_SDK) "data/embeddedsw/lib/riscv/" $libxil]
            }
        }
        if {[file exist $libxil_path]} {
            file copy -force $libxil_path $targetdir
            make_writable $osname [file join $targetdir $libxil]
        }

        set libgcc_path [file join $libgcc_dir $arch $abi $libgcc]
        if {[file exist $libgcc_path]} {
            file copy -force [file dirname [file normalize $libgcc_path/_]] $targetdir
            make_writable $osname [file join $targetdir $libgcc]
        }

    } else {
        error  "ERROR: Wrong compiler type selected please use riscv64-unknown-elf-gcc or riscv64-unknown-elf-g++ or riscv64-unknown-elf-c++"
        return;
    }
    # End of riscv64-unknown-elf-gcc specific processing...

    # Setup the compiler flags as per HW Params
    if {[string compare "riscv" $proctype] > 0 } {
        set use_muldiv [common::get_property CONFIG.C_USE_MULDIV $periph]
        set use_atomic [common::get_property CONFIG.C_USE_ATOMIC $periph]
        set use_fpu    [common::get_property CONFIG.C_USE_FPU $periph]
        set use_compr  [common::get_property CONFIG.C_USE_COMPRESSION $periph]
        set data_size  [common::get_property CONFIG.C_DATA_SIZE $periph]
        set use_dcache [common::get_property CONFIG.C_USE_DCACHE $periph]
        set use_icache [common::get_property CONFIG.C_USE_ICACHE $periph]

        set vlnv_string [common::get_property VLNV $periph]
        set cpu_version [lindex [lreverse [split $vlnv_string :]] 0]

        set arch_flag "i"
        set abi_flag ""
        if {[string compare "1" $use_muldiv] == 0 || [string compare "2" $use_muldiv] == 0} {
            append arch_flag "m"
        }

        if {[string compare "1" $use_atomic] == 0} {
            append arch_flag "a"
        }

        if {[string compare "1" $hard_float] == 0 } {
            append arch_flag "f"
            set abi_flag "f"
        }
        if {[string compare "2" $hard_float] == 0 } {
            append arch_flag "fd"
            set abi_flag "d"
        }

        if {[string compare "1" $use_compr] == 0} {
            append arch_flag "c"
        }

        append arch_flag "_zicsr_zifencei"

        if {[string compare "1" $use_dcache] == 0 || [string compare "1" $use_icache] == 0} {
            append arch_flag "_zicbom"
        }

        if {[string compare "32" $data_size] == 0 } {
            set arch_flag "rv32${arch_flag}"
            set abi_flag "ilp32${abi_flag}"
        }
        if {[string compare "64" $data_size] == 0 } {
            set arch_flag "rv64${arch_flag}"
            set abi_flag "lp64${abi_flag}"
        }

        set compiler_flags [common::get_property CONFIG.compiler_flags $drv_handle]
        set compiler_flags " ${compiler_flags} -march=$arch_flag -mabi=$abi_flag"

        common::set_property CONFIG.compiler_flags $compiler_flags $drv_handle
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

proc post_generate {drv_handle} {

    set type [get_property CLASS $drv_handle]
    if {[string equal $type "driver"]} {
        return
    }

    set file_handle [::hsi::utils::open_include_file "xparameters.h"]
    puts $file_handle "#endif  /* end of protection macro */"
    close $file_handle
}
