###############################################################################
#
# Copyright (C) 2004 - 2014 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
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

proc swapp_get_name {} {
    return "SREC Bootloader";
}

proc swapp_get_description {} {
    return "Simple bootloader for loading SREC images from non volatile memory. This program assumes that you have an SREC image programmed into BPI flash. The program also assumes that the target SREC image is an application for this processor that does not overlap the bootloader and resides in separate physical memory in the hardware. Typically this application is initialized into BRAM so that it bootloads the SREC image when the FPGA is powered up.

Don't forget to modify blconfig.h to reflect the physical address where your SREC image resides in non-volatile memory!";
}

proc check_standalone_os {} {
    set os [hsi::get_os];
    if { $os == "" } {
        error "No Operating System specified in the Board Support Package";
    }

    if { $os != "standalone" } {
        error "This application is supported only on the Standalone Board Support Package.";
    }

}

proc generate_stdout_config { fid } {
    set stdout [get_stdout]
    set stdout [hsi::get_cells -hier $stdout]

    # if stdout is uartns550, generate stdout config
    set stdout_type [common::get_property IP_NAME $stdout];


    if { [regexp -nocase "uartlite" $stdout_type] || [string match -nocase "mdm" $stdout_type] } {
        return;
    } elseif { [regexp -nocase "uart16550" $stdout_type] } {
	# mention that we have a 16550
        puts $fid "#define STDOUT_IS_16550";

        # and note down its base address
	set prefix "XPAR_";
	set postfix "_BASEADDR";
	set stdout_baseaddr_macro $prefix$stdout$postfix;
	set stdout_baseaddr_macro [string toupper $stdout_baseaddr_macro];
	puts $fid "#define STDOUT_BASEADDR $stdout_baseaddr_macro";
    }
}

proc get_stdout {} {
    set os [hsi::get_os];
    if { $os == "" } {
        error "No Operating System specified in the Board Support Package";
    }

    set stdout [common::get_property CONFIG.STDOUT $os];
    return $stdout;
}

proc check_stdout_hw {} {
	set slaves [common::get_property SLAVES [hsi::get_cells -hier [hsi::get_sw_processor]]]
	foreach slave $slaves {
		set slave_type [common::get_property IP_NAME [hsi::get_cells -hier $slave]];
		# Check for MDM-Uart peripheral. The MDM would be listed as a peripheral
		# only if it has a UART interface. So no further check is required
		if { $slave_type == "ps7_uart" || $slave_type == "axi_uartlite" ||
			 $slave_type == "axi_uart16550" || $slave_type == "iomodule" ||
			 $slave_type == "mdm" } {
			return;
		}
	}

	error "This application requires a Uart IP in the hardware."

#    set p7_uarts [hsi::get_cells -filter { ip_name == "ps7_uart" }];
#    set uartlites [hsi::get_cells -filter { ip_name == "axi_uartlite" }];
#    set uart16550s [hsi::get_cells -filter { ip_name == "axi_uart16550" }];
#    set mcs_iomodule [hsi::get_cells -filter { ip_name == "iomodule" }];
#    if { ([llength $p7_uarts] == 0) && ([llength $uartlites] == 0) &&
#	 ([llength $uart16550s] == 0) && ([llength $mcs_iomodule] == 0) } {
#        # Check for MDM-Uart peripheral. The MDM would be listed as a peripheral
#        # only if it has a UART interface. So no further check is required
#        set mdmlist [hsi::get_cells -filter { ip_name == "mdm" }]
#        if { [llength $mdmlist] == 0 } {
#	    error "This application requires a Uart IP in the hardware."
#        }
#    }
}

proc check_stdout_sw {} {
    set stdout [get_stdout];
    if { $stdout == "none" } {
        error "The STDOUT parameter is not set on the OS. The bootloader requires STDOUT to be set."
    }
}

# for microblaze, we need 8k of memory.
proc get_required_mem_size {} {
    return "8192"
}

proc swapp_is_supported_hw {} {
    # bootloader is supported only for microblaze targets
    set proc_instance [hsi::get_sw_processor];
    set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $proc_instance]];

    if { ($proc_type != "microblaze") } {
	error "This application is supported only for MicroBlaze processor";
    }

    # check for uart peripheral
    check_stdout_hw;
}

proc swapp_is_supported_sw {} {
    # check for standalone OS
    check_standalone_os;

    # check for stdout being set
    check_stdout_sw;
}

proc swapp_generate {} {
    # cleanup this file for writing
    set fid [open "platform_config.h" "w+"];
    puts $fid "#ifndef __PLATFORM_CONFIG_H_";
    puts $fid "#define __PLATFORM_CONFIG_H_\n";

    # if we have a uart16550 as stdout, then generate some config for that
    generate_stdout_config $fid;

    puts $fid "#endif";
    close $fid;
}

proc get_mem_type { mem } {
    set mem_type [::hsi::utils::get_ip_sub_type [hsi::get_cells -hier $mem]]
    if { $mem_type == "BRAM_CTRL" } {
        return "BRAM"
    }
    return "OTHER"
}

proc get_program_code_memory {} {
    # obtain a unique list if "I", and "ID" memories
    set proc_instance [hsi::get_sw_processor];
    set imemlist [hsi::get_mem_ranges -of_objects [hsi::get_cells -hier $proc_instance] -filter { IS_INSTRUCTION == true && MEM_TYPE == "MEMORY"}];
    set idmemlist [hsi::get_mem_ranges -of_objects [hsi::get_cells -hier $proc_instance] -filter { IS_INSTRUCTION == true && IS_DATA == true && MEM_TYPE == "MEMORY" }];

    # concatenate "I", and "ID" memories
    set memlist [concat $imemlist $idmemlist];
    # create a unique list
    set unique_memlist {}
    foreach mem $memlist {
        set match [lsearch $unique_memlist $mem ]
        if { $match == -1 } {
            lappend unique_memlist $mem
        }
    }

    set required_mem_size [get_required_mem_size]
    # check for a memory with required size
    foreach mem $unique_memlist {
        set base [common::get_property BASE_VALUE $mem]
        set high [common::get_property HIGH_VALUE $mem]
        if { [expr $high - $base + 1] >= $required_mem_size } {
            if { [get_mem_type $mem] == "BRAM" } {
                return $mem;
            }
        }
    }

    error "This application requires atleast [expr $required_mem_size/1024] KB of BRAM memory for code.";
}

proc get_program_data_memory {} {
    # obtain a unique list if "D" and "ID" memories
    set proc_instance [hsi::get_sw_processor];
    set dmemlist [hsi::get_mem_ranges -of_objects [hsi::get_cells -hier $proc_instance] -filter { IS_DATA == true && MEM_TYPE == "MEMORY"}];
    set idmemlist [hsi::get_mem_ranges -of_objects [hsi::get_cells -hier $proc_instance] -filter { IS_INSTRUCTION == true && IS_DATA == true && MEM_TYPE == "MEMORY" }];

    # concatenate "D", and "ID" memories
    set memlist [concat $dmemlist $idmemlist];
    # create a unique list
    set unique_memlist {}
    foreach mem $memlist {
        set match [lsearch $unique_memlist $mem ]
        if { $match == -1 } {
            lappend unique_memlist $mem
        }
    }

    set required_mem_size [get_required_mem_size]
    # check for a memory with required size
    foreach mem $unique_memlist {
        set base [common::get_property BASE_VALUE $mem]
        set high [common::get_property HIGH_VALUE $mem]
        if { [expr $high - $base + 1] >= $required_mem_size } {
            if { [get_mem_type $mem] == "BRAM" } {
                return $mem;
            }
        }
    }

    error "This application requires atleast [expr $required_mem_size/1024] KB of BRAM memory for data.";
}

proc swapp_get_linker_constraints {} {
    set code_memory [get_program_code_memory];
    set data_memory [get_program_data_memory];

    # set code & data memory to point to bram
    # no need for heap
    return "code_memory $code_memory data_memory $data_memory heap 0";
}

proc swapp_get_supported_processors {} {
    return "microblaze";
}

proc swapp_get_supported_os {} {
    return "standalone";
}
