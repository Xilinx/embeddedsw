#/******************************************************************************
#* Copyright (c) 2016 - 2020 Xilinx, Inc.  All rights reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/


proc swapp_get_name {} {
	return "mba_fs_boot";
}

proc swapp_get_description {} {
	return "fs-boot for microblaze by Xilinx Inc..";
}

proc swapp_get_supported_processors {} {
	return "microblaze";
}

proc swapp_get_supported_os {} {
	return "standalone";
}

proc get_stdout {} {
	set os [hsi::get_os];
	set stdout [common::get_property CONFIG.STDOUT $os];
	return $stdout;
}

proc get_ipname { instance } {
	set ipname [common::get_property IP_NAME [get_cells -hier $instance]];
	return $ipname;
}

proc get_main_mem {} {
	set os [hsi::get_os];
	set mem [common::get_property CONFIG.MAIN_MEMORY $os];
	return $mem;
}

proc get_flash_mem {} {
	set os [hsi::get_os];
	set mem [common::get_property CONFIG.FLASH_MEMORY $os];
	return $mem;
}

proc swapp_is_supported_hw {} {

    # check processor type
    set proc_instance [hsi::get_sw_processor];
    set hw_processor [common::get_property HW_INSTANCE $proc_instance]

    set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];

    if { $proc_type != "microblaze" } {
                error "This application is supported only for Microblaze processor.";
    }

    return 1;
}

proc swapp_is_supported_sw {} {
	return 1;
}

# depending on the type of os (standalone|xilkernel), choose
# the correct source files
proc swapp_generate {} {
	set fid [open "auto-config.h" w+];
	puts $fid "#ifndef __AUTO_CONFIG_H_";
	puts $fid "#define __AUTO_CONFIG_H_\n";
	get_eram_config $fid;
	get_uart_config $fid;
	get_flash_config $fid;
	puts $fid "#endif";
	close $fid;
}

proc get_eram_config { fp } {
	set ip_name "";
	set mem [get_main_mem]
	if {$mem ne ""} {
		set ip_name [get_ipname $mem];
	}
	switch -exact $ip_name {
		"ddr4" {
                        set eram_start [common::get_property CONFIG.C0_DDR4_MEMORY_MAP_BASEADDR \
                                        [hsi::get_cells -hier $mem]];
			if {$eram_start eq ""} {
				set eram_start [common::get_property CONFIG.C_BASEADDR \
					[hsi::get_cells -hier $mem]];
			}
                        set eram_end [common::get_property CONFIG.C0_DDR4_MEMORY_MAP_HIGHADDR \
                                        [hsi::get_cells -hier $mem ]];
			if {$eram_end eq ""} {
				set eram_end [common::get_property CONFIG.C_HIGHADDR \
					[hsi::get_cells -hier $mem ]];
			}
			if {$eram_end ne ""} {
				set eram_end [format 0x%x [expr $eram_end + 1]];
			}
		}
		"ddr3" -
		"mig" -
		"mig_7series" {
			set eram_start [common::get_property CONFIG.C_BASEADDR \
					[hsi::get_cells -hier $mem]];
			set eram_end [format 0x%x [expr [common::get_property CONFIG.C_HIGHADDR \
					[hsi::get_cells -hier $mem ]] + 1]];
			}
		"axi_7series_ddrx" -
		"axi_v6_ddrx" {
			set eram_start [common::get_property CONFIG.C_S_AXI_BASEADDR \
					[hsi::get_cells -hier $mem]]
			set eram_end [format 0x%x [expr [common::get_property CONFIG.C_S_AXI_HIGHADDR \
					[hsi::get_cells -hier $mem ]] + 1]];
			}
		default {
			return 1
			}
	}
	set eram_size [ format 0x%x [expr $eram_end - $eram_start ] ];
	puts $fp "#define CONFIG_XILINX_ERAM_START	$eram_start";
	puts $fp "#define CONFIG_XILINX_ERAM_END	$eram_end";
	puts $fp "#define CONFIG_XILINX_ERAM_SIZE	$eram_size";
}


proc get_flash_config { fp } {
	set ipname "";
	set flash [get_flash_mem];
	if {$flash ne ""} {
		set ipname [get_ipname $flash];
	}
	switch -exact $ipname {
		"axi_quad_spi"  {
			set axi_type_value [common::get_property CONFIG.C_TYPE_OF_AXI4_INTERFACE [get_cells -hier $flash]]
			if {$axi_type_value == "0"} {
				set flash_start [common::get_property CONFIG.C_BASEADDR [get_cells -hier $flash]];
			} else {
				set flash_start [common::get_property CONFIG.C_S_AXI4_BASEADDR [get_cells -hier $flash]];
			}
			puts $fp "#define CONFIG_PRIMARY_FLASH_SPI_BASEADDR      $flash_start";
			puts $fp "#define CONFIG_PRIMARY_FLASH_SPI";
			set spi_mode [common::get_property CONFIG.C_SPI_MODE [get_cells -hier $flash]];
			set spi_fifo_depth [common::get_property CONFIG.C_FIFO_DEPTH [get_cells -hier $flash]];
			puts $fp "#define CONFIG_FLASH_SPI_MODE         $spi_mode";
			puts $fp "#define CONFIG_FLASH_SPI_FIFO_DEPTH   $spi_fifo_depth";
			}
		"axi_emc" {
			set flash_start [common::get_property CONFIG.C_S_AXI_MEM0_BASEADDR [get_cells -hier $flash]];
			set flash_end [format 0x%x [expr \
			[common::get_property CONFIG.C_S_AXI_MEM0_HIGHADDR \
			[hsi::get_cells -hier $flash]] + 1]];
			set flash_size [ format 0x%x [expr $flash_end - $flash_start ] ];
			puts $fp "#define CONFIG_XILINX_FLASH_START     $flash_start";
			puts $fp "#define CONFIG_XILINX_FLASH_END       $flash_end";
			puts $fp "#define CONFIG_XILINX_FLASH_SIZE      $flash_size";
		}
	}
}
proc get_uart_config { fp } {
	set ip_name ""
	set stdout [get_stdout];
	if {$stdout ne ""} {
		set ip_name [get_ipname $stdout];
	}
	if {$ip_name eq "mdm"} {
		if {[common::get_property CONFIG.C_USE_UART \
			[hsi::get_cells -hier $stdout ]] > 0 } {
			#DO NOTHING
		} else {
			return;
		}
	}
	if {$ip_name ne ""} {
		set uart_baseaddr [common::get_property CONFIG.C_BASEADDR \
			[hsi::get_cells -hier $stdout ]];
		switch -exact $ip_name {
			"axi_uart16550" {
					set uart_type "UART16550";
				}
			"axi_uartlite" -
			"mdm" {
				set uart_type "UARTLITE";
			}
		}
		puts $fp "#define CONFIG_STDINOUT_BASEADDR      $uart_baseaddr";
		puts $fp "#define CONFIG_$uart_type	1";
	}
}

proc get_mem_name { memlist } {
    return [lindex $memlist 0];
}

proc get_mem_type { mem } {
	set mem_type [::hsi::utils::get_ip_sub_type [hsi::get_cells -hier $mem]];
	if { $mem_type == "BRAM_CTRL" } {
		return "BRAM";
	}
    return "OTHER";
}

proc get_program_code_memory {} {
    # obtain a unique list if "I", and "ID" memories
    set proc_instance [hsi::get_sw_processor];
    set imemlist [hsi::get_mem_ranges -of_objects \
	[hsi::get_cells -hier $proc_instance] -filter \
	{ IS_INSTRUCTION == true && MEM_TYPE == "MEMORY"}];
    set idmemlist [hsi::get_mem_ranges -of_objects \
	[hsi::get_cells -hier $proc_instance] -filter \
	{ IS_INSTRUCTION == true && IS_DATA == true && MEM_TYPE == "MEMORY" }];

    # concatenate "I", and "ID" memories
    set memlist [concat $imemlist $idmemlist];
    # create a unique list
    set unique_memlist {};
    foreach mem $memlist {
        set match [lsearch $unique_memlist $mem ];
        if { $match == -1 } {
            lappend unique_memlist $mem;
        }
    }

    #set codemem [extract_bram_memories $unique_memlist];
    set required_mem_size [get_required_mem_size];
    # check for a memory with required size
    foreach mem $unique_memlist {
        set base [common::get_property BASE_VALUE $mem];
        set high [common::get_property HIGH_VALUE $mem];
        if { [expr $high - $base + 1] >= $required_mem_size } {
            if { [get_mem_type $mem] == "BRAM" } {
                return $mem;
            }
        }
    }

    error "This application requires at least [expr $required_mem_size/1024] KB of BRAM memory for code.";
}

proc get_program_data_memory {} {
    # obtain a unique list if "D", and "ID" memories
    set proc_instance [hsi::get_sw_processor];
    set dmemlist [hsi::get_mem_ranges -of_objects \
	[hsi::get_cells -hier $proc_instance] -filter \
	{ IS_DATA == true && MEM_TYPE == "MEMORY"}];
    set idmemlist [hsi::get_mem_ranges -of_objects \
	[hsi::get_cells -hier $proc_instance] -filter \
	{ IS_INSTRUCTION == true && IS_DATA == true && MEM_TYPE == "MEMORY" }];

    # concatenate "D", and "ID" memories
    set memlist [concat $dmemlist $idmemlist];
    # create a unique list
    set unique_memlist {};
    foreach mem $memlist {
        set match [lsearch $unique_memlist $mem ];
        if { $match == -1 } {
            lappend unique_memlist $mem;
        }
    }

    #set datamem [extract_bram_memories $unique_memlist];
    set required_mem_size [get_required_mem_size];
    # check for a memory with required size
    foreach mem $unique_memlist {
        set base [common::get_property BASE_VALUE $mem];
        set high [common::get_property HIGH_VALUE $mem];
        if { [expr $high - $base + 1] >= $required_mem_size } {
            if { [get_mem_type $mem] == "BRAM" } {
                return $mem;
            }
        }
    }
    error "This application requires at least \
	[expr $required_mem_size/1024] KB of BRAM memory for data.";
}

proc get_required_mem_size {} {
    return "8192";
}

proc swapp_get_linker_constraints {} {
    set code_memory [get_mem_name [get_program_code_memory]];
    set data_memory [get_mem_name [get_program_data_memory]];

    # set code & data memory to point to bram
    # no need for vectors section (affects PPC linker scripts only)
    # no need for heap
    return "code_memory $code_memory data_memory $data_memory \
	vector_section no heap 0 stack 1024";
}
