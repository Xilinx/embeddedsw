###############################################################################
#
# Copyright (C) 2010 - 2014 Xilinx, Inc.  All rights reserved.
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
###############################################################################
#
# This file is part of Xilkernel.
#
# $Id: xilkernel_v2_1_0.tcl,v 1.1.2.2 2011/12/08 08:17:56 anirudh Exp $
###############################################################################

proc kernel_drc {os_handle} {
    set sw_proc_handle [hsi::get_sw_processor]
    set hw_proc_handle [hsi::get_cells [common::get_property HW_INSTANCE $sw_proc_handle]]
    set proctype [common::get_property IP_NAME $hw_proc_handle]
    set compiler [common::get_property CONFIG.compiler $sw_proc_handle]

    # System timer frequency must be specified
    set systmr_spec [common::get_property CONFIG.systmr_spec $os_handle]
    if { $systmr_spec == "false" }  {
        error "ERROR: Xilkernel requires some or all of the parameters in the systmr_spec category to be defined. Please configure this categry as 'true'." "" "mdt_error"
    }

    set stacksiz [common::get_property CONFIG.pthread_stack_size $os_handle]
    switch -regexp $proctype {
	"microblaze" {
            if { [string first "mb-gcc" $compiler] == 0 && [string first "mb-g++" $compiler] == 0} {
                error "ERROR: Wrong compiler requested. Xilkernel can be compiled only with the GNU compiler for MicroBlaze." "" "mdt_error"
            }

            set systmr_dev [common::get_property CONFIG.systmr_dev $os_handle]
            if { $systmr_dev == "none" } {
                error "ERROR: Xilkernel for Microblaze requires a system timer device to be specified. Please choose a valid peripheral instance in the systmr_dev parameter." "" "mdt_error"
            }
            set systmr_handle [hsi::get_cells $systmr_dev]
            set systmr_type [common::get_property IP_NAME $systmr_handle]
            if { $systmr_type != "fit_timer" && $systmr_type != "opb_timer" && $systmr_type != "xps_timer" && $systmr_type != "axi_timer" } {
                error "ERROR: Xilkernel for Microblaze can work only with an axi_timer, xps_timer, opb_timer or fit_timer. Please choose a valid device as the system timer with the parameter systmr_dev." "" "mdt_error"
            }

            if { [expr $stacksiz % 4] != 0 } {
                error "ERROR: pthread_stack_size must be a multiple of 4."
            }

            set procver [common::get_property CONFIG.HW_VER $hw_proc_handle]
	}
	ppc* {
	    if { $compiler != "powerpc-eabi-gcc" && $compiler != "powerpc-eabi-g++" } {
		error "ERROR: Wrong compiler requested. Xilkernel can be compiled only with the GNU compiler for PPC." "" "mdt_error"
	    }

            if { [expr $stacksiz % 8] != 0 } {
                error "ERROR: pthread_stack_size must be a multiple of 8."
            }
	}
	"default" {
		error "ERROR: Unsupported processor type: $proctype. Xilkernel supported only for PPC and Microblaze." "" "mdt_error"
	}
    }

    set systmr_freq [common::get_property CONFIG.systmr_freq $os_handle]
    if { $systmr_freq == -1 } {
        error "ERROR: System timer frequency not specified." "" "mdt_error"
    }

    set systmr_interval_ms [common::get_property CONFIG.systmr_interval $os_handle]
    if { $systmr_interval_ms <= 0 } {
        error "ERROR: Invalid value for parameter systmr_interval specified. Please specify a positive value." "" "mdt_error"
    }

    set config_bufmalloc [common::get_property CONFIG.config_bufmalloc $os_handle]
    if { $config_bufmalloc == "true" } {
        set memtable_handle [hsi::get_arrays mem_table -of_objects $os_handle]
        #set memtable_elements [xget_handle $memtable_handle "ELEMENTS" "*"]
	set memtable_elements [llength [common::get_property PARAM.mem_nblks $memtable_handle]]
	   foreach ele $memtable_elements {
		set bsiz  [common::get_property PARAM.mem_bsize $memtable_handle]
                set nblks  [common::get_property PARAM.mem_nblks $memtable_handle]
                if { $bsiz < 4 } {
                    error "ERROR: mem_table mem_bsize specification of $bsiz is incorrect. Block size should be >= 4." "" "mdt_error"
                }
                if { $nblks <= 0 } {
                    error "ERROR: mem_table mem_nblks specification of $nblks is incorrect. Block count should be positive." "" "mdt_error"
               }
          }
    }

    set config_msgq [common::get_property CONFIG.config_msgq $os_handle]
    if { $config_msgq == "true" } {
        set use_malloc [common::get_property CONFIG.use_malloc $os_handle]
        if { $use_malloc != "true" && $config_bufmalloc != "true" } {
            error "ERROR: Message queues require memory allocation support. Please configure parameters config_bufmalloc or use_malloc to be true." "" "mdt_error"
        }
    }
}

proc generate {os_handle} {
    variable standalone_version
    set sw_proc_handle [hsi::get_sw_processor]
    set hw_proc_handle [hsi::get_cells [common::get_property HW_INSTANCE $sw_proc_handle]]
    set proctype [common::get_property IP_NAME $hw_proc_handle]
    set procver [common::get_property CONFIG.HW_VER $hw_proc_handle]

    set need_config_file "false"

    # proctype should be "microblaze"
    set mbsrcdir  "../standalone/src/microblaze"
    set ppcsrcdir "../standalone/src/ppc405"
    set ppc440srcdir "../standalone/src/ppc440"
    set commondir   "../standalone/src/common"
    set datadir   "../standalone/data"

    foreach entry [glob -nocomplain [file join $commondir *]] {
        file copy -force $entry [file join ".." "standalone" "src"]
    }

    # proctype should be "microblaze"
    switch -regexp $proctype {
	"microblaze" {

	    file copy -force "./src/Makefile_mb.sh" "./src/Makefile"
            foreach entry [glob -nocomplain [file join $mbsrcdir *]] {
		if { [string first "hw_exception_handler" $entry] == -1 } { ;# Do not copy over the Standalone BSP exception handler
		    file copy -force $entry [file join ".." "standalone" "src"]
		}
            }
	   file rename -force -- "../standalone/src/Makefile" "../standalone/src/Makefile_depends"
	    set need_config_file "true"
	}
	ppc*  {
	    file copy -force "./src/Makefile_ppc.sh" "./src/Makefile"

            # Write the arch.make file
            set makecpu [open "./src/cpu.make" w]
            ::hsi::utils::write_tcl_header $makecpu "Configuration parameters for PPC Xilkernel Makefile"
            if { [string match -nocase ppc440* $proctype] } {
                puts $makecpu "CPU_TYPE=440"
            } else {
                puts $makecpu "CPU_TYPE=405"
            }
            close $makecpu

            if { [string match -nocase ppc440* $proctype] } {
                set ppcsrcdir $ppc440srcdir
            }

	    foreach entry [glob -nocomplain [file join $ppcsrcdir *]] {
                if { [string first "xvectors" $entry] == -1 } {      ;# Do not copy xvectors.S. Xilkernel provides its own.
                    file copy -force $entry [file join ".." "standalone" "src"]
                }
	    }
	}
	"default" {puts "unknown processor type $proctype\n"}
    }

    # Write the config.make file
    set makeconfig [open "../standalone/src/config.make" w]
    #::hsi::utils::write_tcl_header $makeconfig "Configuration parameters for Standalone Makefile"

    if { $proctype == "microblaze" } {
	if { [mb_has_exceptions $hw_proc_handle] } {
	    puts $makeconfig "LIBSOURCES = *.c *.S"
	} else {
	    puts $makeconfig "LIBSOURCES = *.s *.c"
	}
    }
    puts $makeconfig "LIBS = standalone_libs"
    close $makeconfig

    # Remove microblaze directories...
    file delete -force $mbsrcdir
    file delete -force $ppcsrcdir
    file delete -force $datadir

    # Handle stdin and stdout
    ::hsi::utils::handle_stdin $os_handle
    ::hsi::utils::handle_stdout $os_handle

	# Modify Makefile based on whether inbyte.c and outbyte.c been created
	if {[file exists "./src/inbyte.c"] && [file exists "./src/inbyte.c"]} {

		set source [open "./src/Makefile" r]
		set destination [open Makefile.txt w]
		set contents [read $source]
		close $source

		set lines [split $contents \n]

		foreach line $lines {
			if {[regexp -- "standalone:" $line]} {
				puts $destination "standalone:"
				puts $destination "\t\$(CC) \$(CFLAGS) -c \$(INCLUDES) \$(STANDALONE_STDIN_SRC)"
				puts $destination "\t\$(AR) -r \$(LIBDIR)/\$(LIBXIL) \$(STANDALONE_STDIN_OBJ)"
				puts $destination "\t\$(CC) \$(CFLAGS) -c \$(INCLUDES) \$(STANDALONE_STDOUT_SRC)"
				puts $destination "\t\$(AR) -r \$(LIBDIR)/\$(LIBXIL) \$(STANDALONE_STDOUT_OBJ)"

			} else {
				puts $destination $line
			}
		}

		close $destination

		file delete "./src/Makefile"
		file rename Makefile.txt "./src/Makefile"
	}

    # Create config file for microblaze interrupt handling
    if {[string compare -nocase $need_config_file "true"] == 0} {
	xhandle_mb_interrupts
    }

    # Create config files for Microblaze exception handling
    if { $proctype == "microblaze" && [mb_has_exceptions $hw_proc_handle] } {
        xcreate_mb_exc_config_file
    }

    # Create bspconfig file
    set bspcfg_fn [file join ".." "standalone" "src"  "bspconfig.h"]
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
    }

    close $bspcfg_fh

    set config_file [xopen_new_include_file "./src/include/os_config.h" "XilKernel Configuration parameters"]
    set init_file [xopen_new_include_file  "./src/include/config/config_init.h" "XilKernel Configuration parameters"]
    ::hsi::utils::write_c_header $init_file "LibXilKernel Initialization structures"
    puts $init_file "\#include <sys/init.h>"
    puts $init_file "\#include <os_config.h>\n\n"

    switch -regexp $proctype {
	"microblaze" {
	    xput_define $config_file "MB_XILKERNEL" ""
            if {[mb_has_exceptions $hw_proc_handle] } {
                xput_define $config_file "CONFIG_HARDWARE_EXCEPTIONS" ""
            }
            if {[mb_has_exceptions $hw_proc_handle] } {
                xput_define $config_file "CONFIG_HARDWARE_EXCEPTIONS" ""
            }
            set base_vectors_handle [common::get_property CONFIG.C_BASE_VECTORS $hw_proc_handle]
	    if { $base_vectors_handle != "" } {
	        set base_vectors [common::get_property CONFIG.C_BASE_VECTORS $hw_proc_handle]
	    } else {
	        set base_vectors 0x00000000
	    }
	    xput_define $config_file "CONFIG_BASE_VECTORS" "$base_vectors"
	}
	ppc* {
	    xput_define $config_file "PPC_XILKERNEL" ""
            if { [string match -nocase ppc440* $proctype] } {
                xput_define $config_file "PPC_CPU_440" ""
            }
        }
    }

    # Create defines and struct initializations

    set config_debug_support [common::get_property CONFIG.config_debug_support $os_handle]
    if { $config_debug_support == "true" } {
        set config_debug_mode [common::get_property CONFIG.verbose $os_handle]
        if { $config_debug_mode == "true" } {
            xadd_define $config_file $os_handle "verbose"
        }

        ;# set config_debug_mon [xget_value $os_handle "PARAMETER" "debug_mon"]
        ;# if { $config_debug_mon == "true" } {
        ;# xadd_define $config_file $os_handle "config_debugmon"
        ;# }
    }

    set config_enhanced_features [common::get_property CONFIG.enhanced_features $os_handle]
    if { $config_enhanced_features == "true" } {
	set config_kill [common::get_property CONFIG.config_kill $os_handle]
	if { $config_kill == "true" } {
	    xadd_define $config_file $os_handle "config_kill"
	}

	set config_yield [common::get_property CONFIG.config_yield $os_handle]
	if { $config_yield == "true" } {
	    xadd_define $config_file $os_handle "config_yield"
	}
    }

    set config_elf_process [common::get_property CONFIG.config_elf_process $os_handle]
    if { $config_elf_process == "true" } {
        xadd_define $config_file $os_handle "config_elf_process"
        xadd_define $config_file $os_handle "max_procs"

        # Get the Entry Point address, priority for static ELF processes table
        set static_elf_process_table_handle [hsi::get_arrays -of_objects $os_handle "static_elf_process_table"]
        if { $static_elf_process_table_handle != "" } {
#SRI FIX THIS
            set n_init_process [common::get_property SIZE $static_elf_process_table_handle]
            xput_define $config_file "n_init_process" $n_init_process
            xadd_define $config_file $os_handle "config_static_elf_process_support"
            xadd_struct $init_file $os_handle "_process_init" "se_process_table" "static_elf_process_table" "process_start_addr" "process_prio"
        }
    }

    set config_pthread [common::get_property CONFIG.config_pthread_support $os_handle]
    if { $config_pthread == "true" } {
	xadd_define $config_file $os_handle "config_pthread_support"
	xadd_define $config_file $os_handle "max_pthreads"
	xadd_define $config_file $os_handle "pthread_stack_size"

	set static_pthread_table_handle [hsi::get_arrays static_pthread_table -of_objects $os_handle]
	if { $static_pthread_table_handle != "" } {
	    #set n_init_self_pthreads [llength  $static_pthread_table_handle]
	    set n_init_self_pthreads [llength [common::get_property PARAM.pthread_prio [hsi::get_arrays $static_pthread_table_handle -of_objects $os_handle]]]
	    #set n_init_self_pthreads [common::get_property CONFIG.static_pthread_table $os_handle]
	     if {$n_init_self_pthreads != "" } {
		xput_define $config_file "config_static_pthread_support" "true"
		xput_define $config_file "n_init_self_pthreads" $n_init_self_pthreads
		xadd_extern_fname $init_file $os_handle "static_pthread_table" "pthread_start_func"
		xadd_struct $init_file $os_handle "_elf_pthread_init" "kb_pthread_table" "static_pthread_table" "pthread_start_func" "pthread_prio"
	    }
	}

	set config_pthread_mutex [common::get_property CONFIG.config_pthread_mutex $os_handle]
	if { $config_pthread_mutex == "true" } {
	    xadd_define $config_file $os_handle "config_pthread_mutex"
	    xadd_define $config_file $os_handle "max_pthread_mutex"
	    set max_pthread_mutex_waitq [common::get_property CONFIG.max_pthread_mutex_waitq $os_handle]
	    xput_define $config_file "max_pthread_mutex_waitq" $max_pthread_mutex_waitq
	}
    }

    # System timer configuration (Microblaze only. kernel DRC ensures this)
    set systmr_spec [common::get_property CONFIG.systmr_spec $os_handle]
    if { $proctype == "microblaze" } {
        set systmr_dev [common::get_property CONFIG.systmr_dev  $os_handle]
        set systmr_handle [hsi::get_cells $systmr_dev]
	set systmr_type [common::get_property IP_NAME $systmr_handle]
    }

    if { $proctype == "microblaze" } {
        if { $systmr_type == "opb_timer" || $systmr_type == "xps_timer" || $systmr_type == "axi_timer"} {
            xput_define $config_file "CONFIG_TIMER_PIT" "true"
            set systmr_baseaddr [common::get_property CONFIG.C_BASEADDR $systmr_handle]
            xput_define $config_file "systmr_baseaddr" $systmr_baseaddr
        }
    }

    set systmr_freq [common::get_property CONFIG.systmr_freq $os_handle]
    if { $proctype == "ppc405" || $proctype == "ppc405_virtex4" || $proctype == "ppc440_virtex5" } {
        set systmr_interval_ms [common::get_property CONFIG.systmr_interval $os_handle]
        set systmr_interval [expr [expr double($systmr_freq) * double($systmr_interval_ms)] / 1000]
        set systmr_interval [expr int($systmr_interval)]
    } elseif {$proctype == "microblaze" && [expr {$systmr_type == "opb_timer" || $systmr_type == "xps_timer" || $systmr_type == "axi_timer"}]} {
        set systmr_interval_ms [common::get_property CONFIG.systmr_interval $os_handle]
        set systmr_interval [expr [expr double($systmr_freq) * double($systmr_interval_ms)] / 1000]
        set systmr_interval [expr int($systmr_interval)]
    } elseif { $proctype == "microblaze" && $systmr_type == "fit_timer" } {
        set systmr_interval [common::get_property CONFIG.C_NO_CLOCKS $systmr_handle]
    }
    xput_define $config_file "SYSTMR_INTERVAL" $systmr_interval
    xput_define $config_file "SYSTMR_CLK_FREQ" $systmr_freq
    xput_define $config_file "SYSTMR_CLK_FREQ_KHZ" [expr $systmr_freq / 1000]

    # Interrupt controller configuration
    set sysintc_spec [common::get_property CONFIG.sysintc_spec $os_handle]
    if { $sysintc_spec != "none" } {
	xput_define $config_file "CONFIG_INTC" "true"
	set sysintc_dev_handle [hsi::get_cells $sysintc_spec]
	set sysintc_baseaddr [::hsi::utils::get_ip_param_name $sysintc_dev_handle "C_BASEADDR"]
	set sysintc_device_id [::hsi::utils::get_ip_param_name $sysintc_dev_handle "DEVICE_ID"]
	xput_define $config_file "sysintc_baseaddr" $sysintc_baseaddr
	xput_define $config_file "sysintc_device_id" $sysintc_device_id

	# Additionally for microblaze, figure out which interrupt
	# input is the system timer interrupt and define its ID
	if { $proctype == "microblaze" } {
	     set systmr_intr [hsi::get_pins -of_objects [hsi::get_cells $systmr_handle] Interrupt]
	     #set systmr_intr [xget_value $systmr_handle "PORT" "Interrupt"]
	    if { [string compare -nocase $systmr_intr ""] == 0 } {
                error "ERROR: System Timer Interrupt PORT is not specified" "" "mdt_error"
	    }
	    #set mhs_handle [hsi::get_cells -of_object $systmr_handle]
	    set intr_ports [::hsi::utils::get_sink_pins [hsi::get_pins -of_objects [hsi::get_cells $systmr_intr] INTERRUPT]]
	    #set intr_ports [xget_connected_ports_handle $mhs_handle $systmr_intr "sink"]
	    foreach intr_port $intr_ports {
                set intr_port_type [common::get_property TYPE $intr_port]
                if { [string compare -nocase $intr_port_type "global"] == 0 } {
                    continue
                }

                set intc_handle [hsi::get_cells -of_object $intr_port]
                set intc_name [common::get_property NAME $intc_handle]
                set proc_intc_handle [hsi::get_cells $intc_name]
                if { [string compare -nocase $sysintc_dev_handle $intc_handle] == 0 } {
                    continue
                }
                set systmr_intrpin  [hsi::get_pins -of_objects [hsi::get_cells $systmr_handle] -filter "TYPE == INTERRUPT"]
		set intr_id [::hsi::utils::get_port_intr_id $systmr_handle $systmr_intrpin]
		puts $config_file "#define SYSTMR_INTR_ID $intr_id\n"
            }
	}
    }


    set config_sched [common::get_property CONFIG.config_sched $os_handle]
    if { $config_sched == "true" } {
	xadd_define $config_file $os_handle "config_sched"
	xadd_define $config_file $os_handle "sched_type"
	xadd_define $config_file $os_handle "n_prio"
        set sched_type [common::get_property CONFIG.sched_type $os_handle]
        if { $sched_type == "SCHED_PRIO" } {
            xput_define $config_file "config_priosched" "true"
        } else {
            xput_define $config_file "config_rrsched" "true"
        }
	xadd_define $config_file $os_handle "max_readyq"
    }

    set config_sema [common::get_property CONFIG.config_sema $os_handle]
    if { $config_sema == "true" } {
	xadd_define $config_file $os_handle "config_sema"
	xadd_define $config_file $os_handle "max_sem"
	xadd_define $config_file $os_handle "max_sem_waitq"
        set config_named_sema [common::get_property CONFIG.config_named_sema $os_handle]
        if { $config_named_sema == "true" } {
            xadd_define $config_file $os_handle "config_named_sema"
        }
    }

    set config_msgq [common::get_property CONFIG.config_msgq $os_handle]
    if { $config_msgq == "true" } {
	set num_msgqs [common::get_property CONFIG.num_msgqs $os_handle]
	set msgq_capacity [common::get_property CONFIG.msgq_capacity $os_handle]
        set use_malloc [common::get_property CONFIG.use_malloc $os_handle]
	xadd_define $config_file $os_handle "config_msgq"
	xadd_define $config_file $os_handle "num_msgqs"
	xadd_define $config_file $os_handle "msgq_capacity"
        if { $use_malloc == "true" } {
            xput_define $config_file "config_enhanced_msgq" "true"
            xput_define $config_file "use_malloc"   "true"
        }
    }

    set config_shm [common::get_property CONFIG.config_shm $os_handle]
    if { $config_shm == "true" } {
	xadd_define $config_file $os_handle "config_shm"
	set shm_handle [hsi::get_arrays -of_objects $os_handle "shm_table"]
	if { $shm_handle == "" } {
	    error "ERROR: SHM configuration needs shm_table specification." "" "mdt_error"
	}
	set n_shm [llength [hsi::get_arrays $shm_handle -of_objects $os_handle]]
	xput_define $config_file "n_shm" $n_shm
	set shm_msize [get_field_sum $os_handle "shm_table"  "shm_size"]
	xput_define $config_file "shm_msize" $shm_msize
	xadd_struct $init_file $os_handle "_shm_init" "shm_config" "shm_table" "shm_size"
    }

    set config_bufmalloc [common::get_property CONFIG.config_bufmalloc $os_handle]
    if { $config_bufmalloc == "true" } {
	xadd_define $config_file $os_handle "config_bufmalloc"
        #set memtable_handle [xget_handle $os_handle "ARRAY" "mem_table"]
        set memtable_elements [hsi::get_arrays -of_objects $os_handle "mem_table"]
	set n_static_bufs [llength $memtable_elements]
        set max_bufs [common::get_property CONFIG.max_bufs $os_handle]


        set bufmalloc_msize [get_field_product_sum $os_handle "mem_table"  "mem_bsize" "mem_nblks"]
        xput_define $config_file "bufmalloc_msize" $bufmalloc_msize
	xput_define $config_file "n_mbufs" [expr $max_bufs + $n_static_bufs]
	xput_define $config_file "n_static_bufs" $n_static_bufs
	xadd_struct $init_file $os_handle "bufmalloc_init_s" "bufmalloc_cfg" "mem_table" "mem_bsize" "mem_nblks"
    }

    set config_time [common::get_property CONFIG.config_time $os_handle]
    if { $config_time == "true" } {
	xadd_define $config_file $os_handle "config_time"
	xadd_define $config_file $os_handle "max_tmrs"
    }

    #set config_stats [xget_value $os_handle "PARAMETER" "config_stats"]
    #if { $config_stats == "true" } {
    #	xadd_define $config_file $os_handle "config_stats"
    #}

    # Handle I/O ranges for MicroBlaze MPU here
    if { $proctype == "microblaze" } {
        #set mhs_handle [hsi::get_cells -of_object $hw_proc_handle]
        set mmu [common::get_property CONFIG.C_USE_MMU $hw_proc_handle]
        if { $mmu >= 2 } {

            # Enumerate all the I/O ranges into a structure
            set interconnect [common::get_property CONFIG.C_INTERCONNECT $hw_proc_handle]
            if { $interconnect == "" || $interconnect == 0 } {
                set dbus_if_name "DOPB"
            } elseif { $interconnect == 1 } {
                set dbus_if_name "DPLB"
            } else {
                set dbus_if_name "M_AXI_DP"
            }

            set dbus_name [::hsi::utils::get_intfnet_name $hw_proc_handle $dbus_if_name]
            set dbus_handle [hsi::get_cells $dbus_name]
            if { $interconnect == 2 } {
                set dcachelink_handle [hsi::get_cells "DXCL"]
            } else {
                set dcachelink_handle [hsi::get_cells "M_AXI_DC"]
            }

            #set addrlist [xget_hw_bus_slave_addrpairs $dbus_handle]
	    set addrlists [hsi::get_mem_ranges -of_objects [hsi::get_cells $sw_proc_handle]]
	    set addrlist [list]
	    foreach addrist $addrlists {
			set ip_name [common::get_property IP_NAME [hsi::get_cells $addrist]]
			if { $ip_name == "axi_emc" || $ip_name == "mig_7series" } {
						set mem  [lindex [hsi::get_mem_ranges $addrist] 0]
						set mc_base [common::get_property BASE_VALUE  $mem]
						set mc_high [common::get_property HIGH_VALUE $mem]
						lappend addrlist $mc_base $mc_high
			} else {
					set mem [hsi::utils::get_ip_mem_ranges $addrist]
					set mc_base [common::get_property BASE_VALUE  $mem]
					set mc_high [common::get_property HIGH_VALUE $mem]
					lappend addrlist $mc_base $mc_high
			}
	   }

            if { $dcachelink_handle != "" } {
                #set xcl_addrlist [xget_hw_bus_slave_addrpairs $dcachelink_handle]
		set xcl_addrlist [hsi::get_mem_ranges -of_objects [hsi::get_cells $sw_proc_handle]]
                set addrlist [concat addrlist xcl_addrlist]
            }

            set io_addrlist [list]

            # Get the list of memory controllers in the mhs. We want to filter
            # "memories" from the above addrlist
            set memcon_handles [xget_memory_controller_handles [hsi::get_cells $sw_proc_handle]]
	    #set memcon_handles [hsi::get_mem_ranges -of_objects [hsi::get_cells $sw_proc_handle]]
            set n_ioranges 0
            foreach {base high} $addrlist {
                set skip 0
                foreach {memcon_handle} $memcon_handles {
                    set memcon_addrlist [::hsi::utils::get_ip_mem_ranges $memcon_handle]
		    foreach  mem_range $memcon_addrlist {
		    set mc_base [common::get_property BASE_VALUE $mem_range]
		    set mc_high [common::get_property HIGH_VALUE $mem_range]

                        if {$mc_base == $base && $mc_high == $high} {
                            set skip 1
                        }
                    }
                }

                if { $skip == 0 } {
                    # Don't add duplicates
                    set io_skip 0
                    foreach {io_base io_high} $io_addrlist {
                        if { $io_base == $base && $io_high == $high } {
                            set io_skip 1
                        }
                    }

                    if { $io_skip == 0 } {
                        lappend io_addrlist $base $high
                        incr n_ioranges
                    }
                }
            }

            puts $config_file "#define XILKERNEL_IO_NRANGES $n_ioranges"
            puts $init_file "xilkernel_io_range_t system_io_range\[XILKERNEL_IO_NRANGES\] = \{"

            set count 0
            foreach {base high} $io_addrlist {
                set base [format "0x%X" $base]
                set high [format "0x%X" $high]
                incr count
                puts -nonewline $init_file "\t\{$base, $high, MPU_PROT_READWRITE"
                if {$count < $n_ioranges} {
                    puts $init_file "\},"
                } else {
                    puts $init_file "\}"
                }
            }
            puts $init_file "\}\;"
        }
    }

    # complete the header protectors
    puts $config_file "\#endif"
    close $config_file
    puts $init_file "\#endif"
    close $init_file
}

proc xopen_new_include_file { filename description } {
    set inc_file [open $filename w]
    ::hsi::utils::write_c_header $inc_file $description
    set newfname [string map {. _} [lindex [split $filename {\/}] end]]
    puts $inc_file "\#ifndef _[string toupper $newfname]"
    puts $inc_file "\#define _[string toupper $newfname]\n\n"
    return $inc_file
}

proc xadd_define { config_file os_handle parameter } {
    set param_value [common::get_property CONFIG.$parameter $os_handle]
    puts $config_file "#define [string toupper $parameter] $param_value\n"

    # puts "creating #define [string toupper $parameter] $param_value\n"
}

proc xput_define { config_file parameter param_value } {
    puts $config_file "#define [string toupper $parameter] $param_value\n"

    # puts "creating #define [string toupper $parameter] $param_value\n"
}


# args field of the array
proc xadd_extern_fname {initfile oshandle arrayname arg} {
    set arrahandle [hsi::get_arrays $arrayname -of_objects $oshandle]
    set elements [llength [common::get_property PARAM.$arg $arrahandle]]
    foreach  ele $elements {
	set thread_names [common::get_property PARAM.$arg $arrahandle]
	foreach thread_name $thread_names {
		puts $initfile "extern void $thread_name\(void\)\;"
	}
    }
    puts $initfile ""
}

# args is variable no - fields of the array
proc xadd_struct {initfile oshandle structtype structname arrayname args} {

    #set arrhandle [hsi::get_arrays $arrayname -of_objects $oshandle]
    set arrhandle [hsi::get_arrays $arrayname -of_objects $oshandle]
    foreach arg $args {
	set max_count [llength [common::get_property PARAM.$arg $arrhandle]]
    }

    #set elements [xget_handle $arrhandle "ELEMENTS" "*"]
    set count 0
    #set max_count [llength $elements]
    set num_list ""
    set name_list ""
    set index 0
    puts $initfile "struct $structtype $structname\[$max_count\] = \{"
    foreach arg $args {
	set field_values [common::get_property PARAM.$arg $arrhandle]
	set field_value [list]
	if {$index == 0} {
		set name_list $field_values
	} else {
		set num_list $field_values
	}
	incr index
    }
    for {set i 0} {$i < [llength $name_list]} {incr i} {
		incr count
		puts -nonewline $initfile "\t\{"
		puts -nonewline $initfile "[lindex $name_list $i]"
		if {$num_list != ""} {
			puts -nonewline $initfile ","
			puts -nonewline $initfile "[lindex $num_list $i]"
		}
		if {$count < $max_count} {
			puts $initfile "\},"
		} else {
			puts $initfile "\}"
		}
    }
    puts $initfile "\}\;"
}


# return the sum of all the arg field values in arrayname
proc get_field_sum {oshandle arrayname arg} {

    #set arrhandle [hsi::get_arrays -of_objects $oshandle $arrayname]
    set elements [hsi::get_arrays -of_objects $oshandle $arrayname]
    #set elements [xget_handle $arrhandle "ELEMENTS" "*"]
    set count 0
    set max_count [llength $elements]

    foreach ele $elements {
	set field_value [common::get_property CONFIG.$arg $ele]
	set count [expr $field_value+$count]
    }
    return $count
}

# return the sum of the product of field values in arrayname
proc get_field_product_sum {oshandle arrayname field1 field2} {

    #set arrhandle [hsi::get_arrays -of_objects $oshandle $arrayname]
    set elements [hsi::get_arrays -of_objects $oshandle $arrayname]
    #set elements [xget_handle $arrhandle "ELEMENTS" "*"]
    set count 0
    set max_count [llength $elements]
    set field1_list ""
    set field2_list ""

    foreach ele $elements {
	    #set field_value [common::get_property PARAM.$field $ele]
	set field1_value [common::get_property PARAM.$field1 $ele]
	set field1_list $field1_value
	set field2_value [common::get_property PARAM.$field2 $ele]
	set field2_list $field2_value
    }

     for {set i 0} {$i < [llength $field1_value]} {incr i} {
	set field1_valuee [lindex $field1_list $i]
	set field2_valuee [lindex $field2_list $i]
	set incr_value [expr $field1_valuee*$field2_valuee]
	set count [expr $count+$incr_value]
    }
    return $count
}

proc xhandle_mb_interrupts {} {

    set default_interrupt_handler "XNullHandler"
    set default_arg "XNULL"

    set source_interrupt_handler $default_interrupt_handler
    set source_handler_arg $default_arg

    # Handle the interrupt pin
    set sw_proc_handle [hsi::get_sw_processor]
    set periph [hsi::get_cells [common::get_property HW_INSTANCE $sw_proc_handle]]
    set source_ports [::hsi::utils::get_interrupt_sources $periph]
    if {[llength $source_ports] > 1} {
	error "ERROR: Too many interrupting ports on the MicroBlaze.  Should only find 1" "" "hsi_error"
	return
    }

    if {[llength $source_ports] == 1} {
	set source_port [lindex $source_ports 0]
	if {[llength $source_port] != 0} {
	    set source_port_name [common::get_property NAME $source_port]
	    set source_periph [hsi::get_cells -of_object $source_port]
	    set source_name [common::get_property NAME $source_periph]
	    set source_driver [hsi::get_drivers $source_name]

	    if {[string compare -nocase $source_driver ""] != 0} {
		#set int_array [hsi::get_arrays interrupt_handler -of_objects $source_driver]
		set int_array_elems [hsi::get_arrays interrupt_handler -of_objects $source_driver]
		#if {[llength $int_array] != 0} {
		    #set int_array_elems [xget_handle $int_array "ELEMENTS" "*"]
		    if {[llength $int_array_elems] != 0} {
			foreach int_array_elem $int_array_elems {
			    set int_port [common::get_property CONFIG.int_port $int_array_elem]
			    if {[llength $int_port] != 0} {
				if {[string compare -nocase $int_port $source_port_name] == 0 } {
				    set source_interrupt_handler [common::get_property CONFIG.int_handler $int_array_elem]
				    set source_handler_arg [common::get_property CONFIG.int_handler_arg $int_array_elem]
				    if {[string compare -nocase $source_handler_arg DEVICE_ID] == 0 } {
					set source_handler_arg [::hsi::utils::get_ip_param_name $source_periph "DEVICE_ID"]
				    } else {
					if {[string compare -nocase "global" [common::get_property TYPE $source_port]] == 0} {
					    set source_handler_arg $default_arg
					} else {
					    set source_handler_arg [::hsi::utils::get_ip_param_name $source_periph "C_BASEADDR"]
					}
				    }
				    break
				}
			    }
			}
		    }
		#}
	    }
	}
    }

    # Generate microblaze_interrupts_g.c file...
    xcreate_mb_intr_config_file $source_interrupt_handler $source_handler_arg

}


proc xcreate_mb_intr_config_file {handler arg} {

    variable standalone_version
    set mb_table "MB_InterruptVectorTable"

    set filename [file join "../standalone/src" "microblaze_interrupts_g.c"]
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
proc xcreate_mb_exc_config_file { } {

    set hfilename [file join "src" "include" "microblaze_exceptions_g.h"]
    file delete $hfilename
    set hconfig_file [open $hfilename w]

    ::hsi::utils::write_c_header $hconfig_file "Exception Handling Header for MicroBlaze Processor"

    puts $hconfig_file "\n"

    set sw_proc_handle [hsi::get_sw_processor]
    set hw_proc_handle [hsi::get_cells [common::get_property HW_INSTANCE $sw_proc_handle]]
    set procver [common::get_property CONFIG.HW_VER $hw_proc_handle]

    if { ![mb_has_exceptions $hw_proc_handle]} { ;# NO exceptions are enabled
        close $hconfig_file              ;# Do not generate any info in either the header or the C file
        return
    }

    puts $hconfig_file "\#define MICROBLAZE_EXCEPTIONS_ENABLED 1"
    if { [mb_can_handle_exceptions_in_delay_slots $procver] } {
        puts $hconfig_file "#define MICROBLAZE_CAN_HANDLE_EXCEPTIONS_IN_DELAY_SLOTS"
    }

    close $hconfig_file
}

# --------------------------------------
# Tcl procedure post_generate
# This proc removes from libxil.a the basic
# and standalone BSP versions of
# _interrupt_handler and _hw_exception_handler
# routines
# --------------------------------------
proc post_generate {os_handle} {
    set sw_proc_handle [hsi::get_sw_processor]
    set hw_proc_handle [hsi::get_cells [common::get_property HW_INSTANCE $sw_proc_handle]]
    set proctype [common::get_property IP_NAME $hw_proc_handle]
    set procname [common::get_property NAME $hw_proc_handle]

    set procdrv [hsi::get_sw_processor]
    set archiver [common::get_property CONFIG.archiver $procdrv]

    if {[string compare -nocase $proctype "microblaze"] == 0 } {
        # Remove _interrupt_handler.o from libxil.a for Xilkernel
	set libgloss_a [file join .. .. lib libgloss.a]
        if { ![file exists $libgloss_a] } {
		set libgloss_a [file join .. .. lib libxil.a]
        }
        exec $archiver -d $libgloss_a   _interrupt_handler.o

        # We have linkage problems due to how these platforms are defined. Can't do this right now.
        # # Remove _exception_handler.o from libxil.a for Xilkernel
        # exec bash -c "$archiver -d ../../lib/libxil.a _exception_handler.o"

        # Remove _hw_exception_handler.o from libxil.a for microblaze cores with exception support
        # if {[mb_has_exceptions $hw_proc_handle]} {
        #     exec bash -c "$archiver -d ../../lib/libxil.a _hw_exception_handler.o"
        # }
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

    set ee [common::get_property CONFIG.C_USE_MMU $hw_proc_handle]
    if { $ee != "" && $ee != 0 } {
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



# --------------------------------------------------------------------------
# Gets all the handles that are memory controller cores.
# --------------------------------------------------------------------------
proc xget_memory_controller_handles { mhs } {
   set ret_list ""

   # Gets all MhsInsts in the system
   set mhsinsts [hsi::get_cells *]

   # Loop thru each MhsInst and determine if have "ADDR_TYPE = MEMORY" in
   # the parameters.
   foreach mhsinst $mhsinsts {
      # Gets all parameters of the component

      set mem_ranges [::hsi::utils::get_ip_mem_ranges $mhsinst]

      # Loop thru each param and find tag "ADDR_TYPE = MEMORY"
      foreach mem_range $mem_ranges {
         if {$mem_range == ""} {
            continue
         }
         #set addrTypeValue [xget_hw_subproperty_value $param "ADDR_TYPE" ]
	 if {[string compare -nocase [common::get_property MEM_TYPE $mem_range ] MEMORY] == 0} {
		lappend ret_list $mhsinst
		break;
         }


      }
   }

   return $ret_list
}
