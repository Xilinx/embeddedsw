##############################################################################
#
# Copyright (C) 2014 Xilinx, Inc. All rights reserved.
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

# ----------------------------------------------------------------------------
# The following are hardcoded for Zynq.
# We can obtain the scu timer/gic baseaddr from the xml, but other parameters
# need to be hardcoded. hardcode everything..
# ----------------------------------------------------------------------------
#TODO these hardcoding parameters can be removed. It can directly come from PS7 IP
set scutimer_baseaddr	0xF8F00600
set scutimer_intr	29
set scugic_cpu_base	0xF8F00100
set scugic_dist_base	0xF8F01000

# --------------------------------------
# Tcl procedure standalone_drc
# -------------------------------------
proc standalone_drc {os_handle} {
}

# --------------------------------------
# Tcl procedure generate
# -------------------------------------
proc generate {os_handle} {
    global env

    set need_config_file "false"

    # Copy over the right set of files as src based on processor type
    set sw_proc_handle [hsi::get_sw_processor]
    set hw_proc_handle [hsi::get_cells [common::get_property HW_INSTANCE $sw_proc_handle] ]
    set proctype [common::get_property IP_NAME $hw_proc_handle]
    set procname [common::get_property NAME    $hw_proc_handle]

    set enable_sw_profile [common::get_property CONFIG.enable_sw_intrusive_profiling $os_handle]
    set mb_exceptions false

    # proctype should be "microblaze" or psu_cortexa53 or psu_cortexr5 or ps7_cortexa9
    set mbsrcdir "./src/microblaze"
    set cortexa53srcdir "./src/cortexa53"
    set cortexr5srcdir "./src/cortexr5"
    set cortexa9srcdir "./src/cortexa9"
    set procdrv [hsi::get_sw_processor]
    set commonsrcdir "./src/common"

    foreach entry [glob -nocomplain [file join $commonsrcdir *]] {
        file copy -force $entry "./src"
    }

    # Only processor specific file should be copied to specified standalone folder
    # write a API which needs compiler,
    switch $proctype {
        "microblaze" {
            foreach entry [glob -nocomplain [file join $mbsrcdir *]] {
                # Copy over only files that are not related to exception handling. All such files have exception in their names
                file copy -force $entry "./src/"
            }
            set need_config_file "true"
            set mb_exceptions [mb_has_exceptions $hw_proc_handle]
        }
        "psu_microblaze" {
            foreach entry [glob -nocomplain [file join $mbsrcdir *]] {
                # Copy over only files that are not related to exception handling. All such files have exception in their names
                file copy -force $entry "./src/"
            }
            set need_config_file "true"
            set mb_exceptions [mb_has_exceptions $hw_proc_handle]
        }
        "psu_cortexa53"  {
            set procdrv [hsi::get_sw_processor]
            set ccdir "./src/cortexa53/gcc"
            foreach entry [glob -nocomplain [file join $cortexa53srcdir *]] {
                file copy -force $entry "./src/"
            }
            foreach entry [glob -nocomplain [file join $ccdir *]] {
                file copy -force $entry "./src/"
            }

            file delete -force "./src/gcc"
            file delete -force "./src/profile"
            if { $enable_sw_profile == "true" } {
                error "ERROR: Profiling is not supported for A53"
            }
            set file_handle [::hsi::utils::open_include_file "xparameters.h"]
            puts $file_handle "#include \"xparameters_ps.h\""
            puts $file_handle ""
            close $file_handle
        }
        "psu_cortexr5"  {
	    set procdrv [hsi::get_sw_processor]
	    set ccdir "./src/cortexr5/gcc"
	    foreach entry [glob -nocomplain [file join $cortexr5srcdir *]] {
		file copy -force $entry "./src/"
	    }
	    foreach entry [glob -nocomplain [file join $ccdir *]] {
		file copy -force $entry "./src/"
	    }

	    file delete -force "./src/gcc"
	    file delete -force "./src/profile"
            if { $enable_sw_profile == "true" } {
                error "ERROR: Profiling is not supported for R5"
            }
	    set file_handle [::hsi::utils::open_include_file "xparameters.h"]
	    puts $file_handle "#include \"xparameters_ps.h\""
	    puts $file_handle ""
	    close $file_handle
        }
       "ps7_cortexa9"  {
                   set procdrv [hsi::get_sw_processor]
                   set compiler [common::get_property CONFIG.compiler $procdrv]
                   if {[string compare -nocase $compiler "armcc"] == 0} {
                       set ccdir "./src/cortexa9/armcc"
	    } elseif {[string compare -nocase $compiler "iccarm"] == 0} {
		set ccdir "./src/cortexa9/iccarm"
                   } else {
                       set ccdir "./src/cortexa9/gcc"
                   }
                   foreach entry [glob -nocomplain [file join $cortexa9srcdir *]] {
                       file copy -force $entry "./src/"
                   }
                   foreach entry [glob -nocomplain [file join $ccdir *]] {
                       file copy -force $entry "./src/"
                   }
                       file delete -force "./src/armcc"
                       file delete -force "./src/gcc"
			file delete -force "./src/iccarm"
                   if {[string compare -nocase $compiler "armcc"] == 0} {
                       file delete -force "./src/profile"
                       set enable_sw_profile "false"
	    }
		if {[string compare -nocase $compiler "iccarm"] == 0} {
                           file delete -force "./src/profile"
                           set enable_sw_profile "false"
                   }
                   set file_handle [::hsi::utils::open_include_file "xparameters.h"]
                   puts $file_handle "#include \"xparameters_ps.h\""
                   puts $file_handle ""
                   close $file_handle
        }
        "default" {puts "unknown processor type $proctype\n"}
    }

    # Write the Config.make file
    set makeconfig [open "./src/config.make" w]
#    print_generated_header_tcl $makeconfig "Configuration parameters for Standalone Makefile"
    if { $proctype == "microblaze" || $proctype == "psu_microblaze" } {
        puts $makeconfig "LIBSOURCES = *.c *.S"
        puts $makeconfig "PROFILE_ARCH_OBJS = profile_mcount_mb.o"
    } elseif { $proctype == "psu_cortexr5" } {
	puts $makeconfig "LIBSOURCES = *.c *.S"
    } elseif { $proctype == "psu_cortexa53" }  {
            puts $makeconfig "LIBSOURCES = *.c *.s *.S"
    } elseif { $proctype == "ps7_cortexa9" } {
        if {[string compare -nocase $compiler "armcc"] == 0} {
            puts $makeconfig "LIBSOURCES = *.c *.s"
        } elseif {[string compare -nocase $compiler "iccarm"] == 0} {
            puts $makeconfig "LIBSOURCES = *.c *.s"
		} else {
            puts $makeconfig "LIBSOURCES = *.c *.S"
            puts $makeconfig "PROFILE_ARCH_OBJS = profile_mcount_arm.o"
        }
    } else {
        error "ERROR: processor $proctype is not supported"
    }
    if { $enable_sw_profile == "true" } {
        puts $makeconfig "LIBS = standalone_libs profile_libs"
    } else {
        puts $makeconfig "LIBS = standalone_libs"
    }
    close $makeconfig

    # Remove microblaze,  cortexr5, cortexa53 and common directories...
    file delete -force $mbsrcdir
    file delete -force $cortexr5srcdir
    file delete -force $cortexa53srcdir
    file delete -force $cortexa9srcdir
    file delete -force $commonsrcdir

    # Handle stdin and stdout
    ::hsi::utils::handle_stdin $os_handle
    ::hsi::utils::handle_stdout $os_handle

    #Handle Profile configuration
    if { $enable_sw_profile == "true" } {
        handle_profile $os_handle $proctype
    }

    set file_handle [::hsi::utils::open_include_file "xparameters.h"]
    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle

    # Create config file for microblaze interrupt handling
    if { [string compare -nocase $need_config_file "true"] == 0 } {
        xhandle_mb_interrupts
    }

    # Create config files for Microblaze exception handling
    if { $proctype == "microblaze" && [mb_has_exceptions $hw_proc_handle] } {
        xcreate_mb_exc_config_file $os_handle
    }

    # Create bspconfig file
    set bspcfg_fn [file join "src" "bspconfig.h"]
    file delete $bspcfg_fn
    set bspcfg_fh [open $bspcfg_fn w]
    ::hsi::utils::write_c_header $bspcfg_fh "Configurations for Standalone BSP"

    if { $proctype == "microblaze" && [mb_has_pvr $hw_proc_handle] } {

        set pvr [common::get_property CONFIG.C_PVR $hw_proc_handle]

        switch $pvr {
            "0" {
                puts $bspcfg_fh "#define MICROBLAZE_PVR_NONE"
            }
            "1" {
                puts $bspcfg_fh "#define MICROBLAZE_PVR_BASIC"
            }
            "2" {
                puts $bspcfg_fh "#define MICROBLAZE_PVR_FULL"
            }
            "default" {
                puts $bspcfg_fh "#define MICROBLAZE_PVR_NONE"
            }
        }
    } else {
        puts $bspcfg_fh "#define MICROBLAZE_PVR_NONE"
    }

    close $bspcfg_fh
}

# --------------------------------------
# Tcl procedure xhandle_mb_interrupts
# --------------------------------------
proc xhandle_mb_interrupts {} {

    set default_interrupt_handler "XNullHandler"
    set default_arg "XNULL"

    set source_interrupt_handler $default_interrupt_handler
    set source_handler_arg $default_arg

    # Handle the interrupt pin
    set sw_proc_handle [hsi::get_sw_processor]
    set periph [hsi::get_cells [common::get_property HW_INSTANCE $sw_proc_handle] ]
    set source_ports [::hsi::utils::get_interrupt_sources $periph]
    if {[llength $source_ports] > 1} {
        error "ERROR: Too many interrupting ports on the MicroBlaze. Should only find 1" "" "hsi_error"
        return
    }
    if { [llength $source_ports] != 0 } {
        set source_periph [hsi::get_cells -of_objects $source_ports]
        if { [llength $source_periph] != 0 } {
            set source_driver [hsi::get_drivers -filter "HW_INSTANCE==$source_periph"]
            if { [llength $source_driver] != 0 } {
                set intr_array [hsi::get_arrays -of_objects $source_driver -filter "NAME==interrupt_handler"]
                if { [llength $intr_array] != 0 } {
                    set array_size [common::get_property PROPERTY.size $intr_array]
                    for { set i 0 } { $i < $array_size } { incr i } {
                        set int_port [lindex [common::get_property PARAM.int_port $intr_array] $i]
                        if { [llength $int_port] != 0 } {
                            if { [string compare -nocase $int_port $source_ports] == 0 } {
                                set source_interrupt_handler [lindex [common::get_property PARAM.int_handler $intr_array] $i]
                                set source_handler_arg [lindex [common::get_property PARAM.int_handler_arg $intr_array] $i]
                                if { [string compare -nocase $source_handler_arg DEVICE_ID] == 0 } {
                                    set source_handler_arg [::hsi::utils::get_ip_param_name $source_periph "DEVICE_ID"]
                                } else {
                                    set source_handler_arg [::hsi::utils::get_ip_param_name $source_periph "C_BASEADDR"]
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    # Generate microblaze_interrupts_g.c file...
    xcreate_mb_intr_config_file $source_interrupt_handler $source_handler_arg
}
# -------------------------------------------
# Tcl procedure xcreate_mb_intr_config file
# -------------------------------------------
proc xcreate_mb_intr_config_file {handler arg} {

    set mb_table "MB_InterruptVectorTable"

    set filename [file join "src" "microblaze_interrupts_g.c"]
    file delete $filename
    set config_file [open $filename w]

    ::hsi::utils::write_c_header $config_file "Interrupt Handler Table for MicroBlaze Processor"

    puts $config_file "#include \"microblaze_interrupts_i.h\""
    puts $config_file "#include \"xparameters.h\""
    puts $config_file "\n"
    puts $config_file [format "extern void %s (void *);" $handler]
    puts $config_file "\n/*"
    puts $config_file "* The interrupt handler table for microblaze processor"
    puts $config_file "*/\n"
    puts $config_file [format "%sEntry %s\[\] =" $mb_table $mb_table]
    puts $config_file "\{"
    puts -nonewline $config_file [format "\{\t%s" $handler]
    puts -nonewline $config_file [format ",\n\t(void*) %s\}" $arg]
    puts -nonewline $config_file "\n\};"
    puts $config_file "\n"
    close $config_file
}

# -------------------------------------------
# Tcl procedure xcreate_mb_exc_config file
# -------------------------------------------
proc xcreate_mb_exc_config_file {os_handle} {

    set hfilename [file join "src" "microblaze_exceptions_g.h"]
    file delete $hfilename
    set hconfig_file [open $hfilename w]
    ::hsi::utils::write_c_header $hconfig_file "Exception Handling Header for MicroBlaze Processor"
    set sw_proc_handle [hsi::get_sw_processor]
    set hw_proc_handle [hsi::get_cells [common::get_property HW_INSTANCE $sw_proc_handle] ]
    set procvlnv [common::get_property VLNV $hw_proc_handle]
    set procvlnv [split $procvlnv :]
    set procver [lindex $procvlnv 3]
    set ibus_ee [common::get_property CONFIG.C_M_AXI_I_BUS_EXCEPTION $hw_proc_handle]
    set dbus_ee [common::get_property CONFIG.C_M_AXI_D_BUS_EXCEPTION $hw_proc_handle]
    set ill_ee [common::get_property CONFIG.C_ILL_OPCODE_EXCEPTION $hw_proc_handle]
    set unalign_ee [common::get_property CONFIG.C_UNALIGNED_EXCEPTIONS $hw_proc_handle]
    set div0_ee [common::get_property CONFIG.C_DIV_ZERO_EXCEPTION $hw_proc_handle]
    set mmu_ee [common::get_property CONFIG.C_USE_MMU $hw_proc_handle]
    if { $mmu_ee == "" } {
        set mmu_ee 0
    }
    set fsl_ee [common::get_property CONFIG.C_FSL_EXCEPTION $hw_proc_handle]
    if { $fsl_ee == "" } {
        set fsl_ee 0
    }
    if { [mb_has_fpu_exceptions $hw_proc_handle] } {
        set fpu_ee [common::get_property CONFIG.C_FPU_EXCEPTION $hw_proc_handle]
    } else {
        set fpu_ee 0
    }
    set sp_ee [common::get_property CONFIG.C_USE_STACK_PROTECTION $hw_proc_handle]
    if { $sp_ee == "" } {
        set sp_ee 0
    }
    set ft_ee [common::get_property CONFIG.C_FAULT_TOLERANT $hw_proc_handle]
    if { $ft_ee == "" } {
        set ft_ee 0
    }

    if { $ibus_ee == 0 && $dbus_ee == 0 && $ill_ee == 0 && $unalign_ee == 0
         && $div0_ee == 0 && $fpu_ee == 0 && $mmu_ee == 0 && $fsl_ee == 0
         && $sp_ee == 0 && $ft_ee == 0} {
        ;# NO exceptions are enabled
        ;# Do not generate any info in either the header or the C file
        close $hconfig_file
        return
    }

    puts $hconfig_file "\#define MICROBLAZE_EXCEPTIONS_ENABLED 1"
    if { [mb_can_handle_exceptions_in_delay_slots $procver] } {
        puts $hconfig_file "#define MICROBLAZE_CAN_HANDLE_EXCEPTIONS_IN_DELAY_SLOTS"
    }
    if { $unalign_ee == 0 } {
        puts $hconfig_file "\#define NO_UNALIGNED_EXCEPTIONS 1"
    }
    if { $ibus_ee == 0 && $dbus_ee == 0 && $ill_ee == 0 && $div0_ee == 0
         && $fpu_ee == 0 && $mmu_ee == 0 && $fsl_ee == 0 } {
        ;# NO other exceptions are enabled
        puts $hconfig_file "\#define NO_OTHER_EXCEPTIONS 1"
    }

    if { $fpu_ee != 0 } {
        puts $hconfig_file "\#define MICROBLAZE_FP_EXCEPTION_ENABLED 1"
        set predecode_fpu_exceptions [common::get_property CONFIG.predecode_fpu_exceptions $os_handle]
        if {$predecode_fpu_exceptions != false } {
            puts $hconfig_file "\#define MICROBLAZE_FP_EXCEPTION_DECODE 1"
        }
    }

    puts $hconfig_file "\n"
    close $hconfig_file
}


# --------------------------------------
# Tcl procedure post_generate
#
# This proc removes _interrupt_handler.o
# from libxil.a
# --------------------------------------
proc post_generate {os_handle} {

    set sw_proc_handle [hsi::get_sw_processor]
    set hw_proc_handle [hsi::get_cells [common::get_property HW_INSTANCE $sw_proc_handle] ]

    set procname [common::get_property NAME $hw_proc_handle]
    set proctype [common::get_property IP_NAME $hw_proc_handle]

    if {[string compare -nocase $proctype "microblaze"] == 0} {

        set procdrv [hsi::get_sw_processor]
        # Remove _interrupt_handler.o from libxil.a for mb-gcc
        set archiver [common::get_property CONFIG.archiver $procdrv]
        set libgloss_a [file join .. .. lib libgloss.a]
        if { ![file exists $libgloss_a] } {
		set libgloss_a [file join .. .. lib libxil.a]
        }
        exec $archiver -d $libgloss_a _interrupt_handler.o

        # Remove _hw_exception_handler.o from libgloss.a for microblaze_v3_00_a
        if { [mb_has_exceptions $hw_proc_handle] } {
		exec $archiver -d $libgloss_a _hw_exception_handler.o
        }
    }
}

# --------------------------------------
# Return true if this MB has
# exception handling support
# --------------------------------------
proc mb_has_exceptions { hw_proc_handle } {

    # Check if the following parameters exist on this MicroBlaze's MPD
    set ee [common::get_property CONFIG.C_UNALIGNED_EXCEPTIONS $hw_proc_handle]
    if { $ee != "" } {
        return true
    }

    set ee [common::get_property CONFIG.C_ILL_OPCODE_EXCEPTION $hw_proc_handle]
    if { $ee != "" } {
        return true
    }

    set ee [common::get_property CONFIG.C_IOPB_BUS_EXCEPTION $hw_proc_handle]
    if { $ee != "" } {
        return true
    }

    set ee [common::get_property CONFIG.C_DOPB_BUS_EXCEPTION $hw_proc_handle]
    if { $ee != "" } {
        return true
    }

    set ee [common::get_property CONFIG.C_IPLB_BUS_EXCEPTION $hw_proc_handle]
    if { $ee != "" } {
        return true
    }

    set ee [common::get_property CONFIG.C_DPLB_BUS_EXCEPTION $hw_proc_handle]
    if { $ee != "" } {
        return true
    }

    set ee [common::get_property CONFIG.C_M_AXI_I_BUS_EXCEPTION $hw_proc_handle]
    if { $ee != "" } {
        return true
    }

    set ee [common::get_property CONFIG.C_M_AXI_D_BUS_EXCEPTION $hw_proc_handle]
    if { $ee != "" } {
        return true
    }

    set ee [common::get_property CONFIG.C_DIV_BY_ZERO_EXCEPTION $hw_proc_handle]
    if { $ee != "" } {
        return true
    }

    set ee [common::get_property CONFIG.C_DIV_ZERO_EXCEPTION $hw_proc_handle]
    if { $ee != "" } {
        return true
    }

    set ee [common::get_property CONFIG.C_FPU_EXCEPTION $hw_proc_handle]
    if { $ee != "" } {
        return true
    }

    set ee [common::get_property CONFIG.C_FSL_EXCEPTION $hw_proc_handle]
    if { $ee != "" } {
        return true
    }

    set ee [common::get_property CONFIG.C_USE_MMU $hw_proc_handle]
    if { $ee != ""} {
        return true
    }

    set ee [common::get_property CONFIG.C_USE_STACK_PROTECTION $hw_proc_handle]
    if { $ee != ""} {
        return true
    }

    set ee [common::get_property CONFIG.C_FAULT_TOLERANT $hw_proc_handle]
    if { $ee != ""} {
        return true
    }

    return false
}
# --------------------------------------
# Return true if this MB has
# FPU exception handling support
# --------------------------------------
proc mb_has_fpu_exceptions { hw_proc_handle } {

    # Check if the following parameters exist on this MicroBlaze's MPD
    set ee [common::get_property CONFIG.C_FPU_EXCEPTION $hw_proc_handle]
    if { $ee != "" } {
        return true
    }

    return false
}

# --------------------------------------
# Return true if this MB has PVR support
# --------------------------------------
proc mb_has_pvr { hw_proc_handle } {

    # Check if the following parameters exist on this MicroBlaze's MPD
    set pvr [common::get_property CONFIG.C_PVR $hw_proc_handle]
    if { $pvr != "" } {
        return true
    }

    return false
}

# --------------------------------------
# Return true if MB ver 'procver' has
# support for handling exceptions in
# delay slots
# --------------------------------------
proc mb_can_handle_exceptions_in_delay_slots { procver } {

    if { [string compare -nocase $procver "5.00.a"] >= 0 } {
        return true
    } else {
        return false
    }
}

# --------------------------------------
# Generate Profile Configuration
# --------------------------------------
proc handle_profile { os_handle proctype } {
    global env
    variable scutimer_baseaddr
    variable scutimer_intr
    variable scugic_cpu_base
    variable scugic_dist_base

    set proc [hsi::get_sw_processor]

    if {$proctype == "ps7_cortexa9"} {
        set sw_proc_handle [hsi::get_sw_processor]
        set hw_proc_handle [hsi::get_cells [common::get_property HW_INSTANCE $sw_proc_handle]]
        set cpu_freq [common::get_property CONFIG.C_CPU_CLK_FREQ_HZ $hw_proc_handle]
        if { [string compare -nocase $cpu_freq ""] == 0 } {
            puts "WARNING<profile> :: CPU Clk Frequency not specified, Assuming 666Mhz"
            set cpu_freq 666000000
        }
    } else {
        set cpu_freq [common::get_property CONFIG.C_FREQ  [hsi::get_cells $proc]]
        if { [string compare -nocase $cpu_freq ""] == 0 } {
            puts "WARNING<profile> :: CPU Clk Frequency not specified, Assuming 100Mhz"
            set cpu_freq 100000000
        }
    }
    set filename [file join "src" "profile" "profile_config.h"]
    file delete -force $filename
    set config_file [open $filename w]

    ::hsi::utils::write_c_header $config_file "Profiling Configuration parameters. These parameters
* can be overwritten thru run configuration in SDK"
    puts $config_file "#ifndef _PROFILE_CONFIG_H"
    puts $config_file "#define _PROFILE_CONFIG_H\n"

    puts $config_file "#define BINSIZE 4"
    puts $config_file "#define CPU_FREQ_HZ $cpu_freq"
    puts $config_file "#define SAMPLE_FREQ_HZ 100000"
    puts $config_file "#define TIMER_CLK_TICKS [expr $cpu_freq / 100000]"

    # proctype should be "microblaze" or "psu_cortexa9"
    switch $proctype {
        "microblaze" {
            # Microblaze Processor.
            puts $config_file "#define PROC_MICROBLAZE 1"
            set timer_inst [common::get_property CONFIG.profile_timer $os_handle]
            if { [string compare -nocase $timer_inst "none"] == 0 } {
            # Profile Timer Not Selected
                error "ERROR : Timer for Profiling NOT selected.\nS/W Intrusive Profiling on MicroBlaze requires an axi_timer." "" "mdt_error"
            } else {
                handle_profile_opbtimer $config_file $timer_inst
            }
        }
        "ps7_cortexa9" {
	            # Cortex A9 Processor.

	            puts $config_file "#define PROC_CORTEXA9 1"
	            set timer_inst [common::get_property CONFIG.profile_timer $os_handle]
	            if { [string compare -nocase $timer_inst "none"] == 0 } {
	                # SCU Timer
	                puts $config_file "#define ENABLE_SCU_TIMER 1"
	                puts $config_file "#define ENABLE_SYS_INTR 1"
	                puts $config_file "#define PROFILE_TIMER_BASEADDR $scutimer_baseaddr"
	                puts $config_file "#define PROFILE_TIMER_INTR_ID $scutimer_intr"
	                puts $config_file "#define SCUGIC_CPU_BASEADDR $scugic_cpu_base"
	                puts $config_file "#define SCUGIC_DIST_BASEADDR $scugic_dist_base"
	            }
        }

        "default" {error "ERROR: unknown processor type\n"}
    }

    puts $config_file "\n#endif"
    puts $config_file "\n/******************************************************************/\n"
    close $config_file
}

    #***--------------------------------***-----------------------------------***
    # Utility process to call a command and pipe it's output to screen.
    # Used instead of Tcl's exec
proc execpipe {COMMAND} {

  if { [catch {open "| $COMMAND 2>@stdout"} FILEHANDLE] } {
    return "Can't open pipe for '$COMMAND'"
  }

  set PIPE $FILEHANDLE
  fconfigure $PIPE -buffering none

  set OUTPUT ""

  while { [gets $PIPE DATA] >= 0 } {
    append OUTPUT $DATA "\n"
  }

  if { [catch {close $PIPE} ERRORMSG] } {

    if { [string compare "$ERRORMSG" "child process exited abnormally"] == 0 } {
      # this error means there was nothing on stderr (which makes sense) and
      # there was a non-zero exit code - this is OK as we intentionally send
      # stderr to stdout, so we just do nothing here (and return the output)
    } else {
      return "Error '$ERRORMSG' on closing pipe for '$COMMAND'"
    }

  }

  regsub -all -- "\n$" $OUTPUT "" STRIPPED_STRING
  return "$STRIPPED_STRING"

}
# - The xps/opb_timer can be connected directly to Microblaze External Intr Pin.
# - (OR) xps/opb_timer can be connected to xps/opb_intc
proc handle_profile_opbtimer { config_file timer_inst } {
    set timer_handle [hsi::get_cells  $timer_inst]
    set timer_baseaddr [common::get_property CONFIG.C_BASEADDR $timer_handle]
    puts $config_file "#define PROFILE_TIMER_BASEADDR [::hsi::utils::format_addr_string $timer_baseaddr "C_BASEADDR"]"

    # Figure out how Timer is connected.
     set timer_intr [hsi::get_pins -of_objects [hsi::get_cells $timer_handle] Interrupt]
    if { [string compare -nocase $timer_intr ""] == 0 } {
	error "ERROR <profile> :: Timer Interrupt PORT is not specified" "" "mdt_error"
    }
    #set mhs_handle [xget_handle $timer_handle "parent"]
    # CR 302300 - There can be multiple "sink" for the interrupt. So need to iterate through the list
    set intr_port_list [::hsi::utils::get_sink_pins [hsi::get_pins -of_objects [hsi::get_cells $timer_intr] INTERRUPT]]
    set timer_connection 0
    foreach intr_port $intr_port_list {
	set intc_handle [hsi::get_cells -of_object $intr_port]
	# Check if the Sink is a Global Port. If so, Skip the Port Connection

	if {  [::hsi::utils::is_external_pin $intr_port] } {
	    continue
	}
	set iptype [common::get_property CONFIG.EDK_IPTYPE $intc_handle]
	if { [string compare -nocase $iptype "PROCESSOR"] == 0 } {
	    # Timer Directly Connected to the Processor
	    puts $config_file "#define ENABLE_SYS_INTR 1"
	    set timer_connection 1
	    break
	}

	set ipsptype [common::get_property CONFIG.EDK_SPECIAL $intc_handle]
	if { [string compare -nocase $iptype "PERIPHERAL"] == 0  &&
	     [string compare -nocase $ipsptype "INTR_CTRL"] == 0 } {
	    # Timer connected to Interrupt controller
	    puts $config_file "#define TIMER_CONNECT_INTC 1"
	    puts $config_file "#define INTC_BASEADDR [xget_value $intc_handle "PARAMETER" "C_BASEADDR"]"
	    set num_intr_inputs [common::get_property CONFIG.C_NUM_INTR_INPUTS $intc_handle]
	    # if { $num_intr_inputs == 1 } {  ## Always enable system interrupt CR 472288
		 puts $config_file "#define ENABLE_SYS_INTR 1"
	    # }

	    #set signals [split [xget_value $intr_port "VALUE"] "&"]
            set signals [::hsi::utils::get_source_pins $intr_port]
	    set i 1
	    foreach signal $signals {
		set signal [string trim $signal]
		if {[string compare -nocase $signal $timer_intr] == 0} {
		    set timer_id [expr ($num_intr_inputs - $i)]
		    set timer_mask [expr 0x1 << $timer_id]
		    puts $config_file "#define PROFILE_TIMER_INTR_ID $timer_id"
		    puts $config_file "#define PROFILE_TIMER_INTR_MASK [format "0x%x" $timer_mask]"
		    break
		}
		incr i
	    }
	    set timer_connection 1
	    break
	}
    }

    if { $timer_connection == 0 } {
	error "ERROR <profile> :: Profile Timer Interrupt Signal Not Connected Properly"
    }
}
