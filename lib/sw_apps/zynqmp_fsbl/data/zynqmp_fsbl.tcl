#/******************************************************************************
#* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/


proc swapp_get_name {} {
    return "Zynq MP FSBL";
}

proc swapp_get_description {} {
    return "First Stage Bootloader (FSBL) for Zynq Ultrascale+ MPSoC. The FSBL configures the FPGA with HW bit stream (if it exists) \
	and loads the Operating System (OS) Image or Standalone (SA) Image or 2nd Stage Boot Loader image from the \
	non-volatile memory (NAND/SD/QSPI) to RAM (DDR) and takes A53/R5 out of reset.  It supports multiple partitions, \
	and each partition can be a code image or a bit stream.";
}

proc swapp_get_supported_processors {} {
    return "psu_cortexa53 psu_cortexr5";
}

proc swapp_get_supported_os {} {
    return "standalone";
}

proc check_standalone_os {} {
    set oslist [hsi::get_os];

    if { [llength $oslist] != 1 } {
        return 0;
    }
    set os [lindex $oslist 0];

    if { $os != "standalone" } {
        error "This application is supported only on the Standalone Board Support Package.";
    }
}

proc get_ip_sub_type { ip_inst_object} {
    if { [string compare -nocase cell [common::get_property CLASS $ip_inst_object]] != 0 } {
        error "get_mem_type API expect only mem_range type object whereas $class type object is passed"
    }

    set ip_type [common::get_property CONFIG.EDK_SPECIAL $ip_inst_object]
    if { [llength $ip_type] != 0 } {
        return $ip_type
    }

    set ip_name [common::get_property IP_NAME $ip_inst_object]
    if { [string compare -nocase "$ip_name"  "lmb_bram_if_cntlr"] == 0
        || [string compare -nocase "$ip_name" "isbram_if_cntlr"] == 0
        || [string compare -nocase "$ip_name" "axi_bram_ctrl"] == 0
        || [string compare -nocase "$ip_name" "dsbram_if_cntlr"] == 0
        || [string compare -nocase "$ip_name" "ps7_ram"] == 0 } {
            set ip_type "BRAM_CTRL"
    } elseif { [string match -nocase *ddr* "$ip_name" ] == 1 } {
         set ip_type "DDR_CTRL"
     } elseif { [string compare -nocase "$ip_name" "mpmc"] == 0 } {
         set ip_type "DRAM_CTRL"
     } elseif { [string compare -nocase "$ip_name" "axi_emc"] == 0 } {
         set ip_type "SRAM_FLASH_CTRL"
     } elseif { [string compare -nocase "$ip_name" "psu_ocm_ram_0"] == 0
                || [string compare -nocase "$ip_name" "psu_ocm_ram_1"] == 0
                || [string compare -nocase "$ip_name" "psu_ocm_ram"] == 0 } {
         set ip_type "OCM_CTRL"
     } else {
         set ip_type [common::get_property IP_TYPE $ip_inst_object]
     }
     #puts "$ip_inst_object: $ip_type"; flush stdout
     return $ip_type
}

proc get_mem_type { mem } {
    set mem_type [get_ip_sub_type [hsi::get_cells -hier $mem]]
    if { $mem_type == "BRAM_CTRL" } {
        return "BRAM"
    }
    if { $mem_type == "OCM_CTRL" } {
	return "OCM"
    }
    return "OTHER"
}

proc check_program_memory {} {
    # Obtain a list of "ID" memories
    set proc_instance [hsi::get_sw_processor]
    set idmemlist [hsi::get_mem_ranges -of_objects [hsi::get_cells -hier $proc_instance] -filter { IS_INSTRUCTION == true && IS_DATA == true && MEM_TYPE == "MEMORY" }]
    set security [common::get_property CONFIG.security_state [::hsi::get_os]]

    set required_mem_size 0x2A000
    set ocm_ranges {}
    # Get a list of OCM regions
    foreach mem $idmemlist {
	# Skip memory regions if,
	# a. ACCESS_TYPE != RW
	# b. BSP security settings != TZ settings of the region
	# c. MEM TYPE != OCM
	set access [common::get_property ACCESS_TYPE $mem]
	if { $access != "Read/Write" } continue
	set tz [common::get_property TRUSTZONE $mem]
	if { $security == "secure" && $tz == "Strict-NonSecure" } continue
	if { $security == "non-secure" && $tz == "Secure" } continue
	if { [get_mem_type $mem] != "OCM" } continue
	set base [common::get_property BASE_VALUE $mem]
	set high [common::get_property HIGH_VALUE $mem]
	set size [expr $high - $base +1]
	if { $base == 0xfffc0000 && $size >= $required_mem_size } return
	lappend ocm_ranges [list $base $size]
    }
    if { [llength $ocm_ranges] == 0 } {
	error "This application requires at least [expr $required_mem_size/1024] KB of OCM memory at 0xfffc0000 to run"
    }

    # Sort the regions and contatenate sequential regions
    set ocm_ranges [lsort -integer -index 0 $ocm_ranges]
    set base0 [lindex $ocm_ranges 0 0]
    set size0 [lindex $ocm_ranges 0 1]
    set ocm_ranges [lrange $ocm_ranges 1 end]
    set concatenated_ranges {}
    foreach range $ocm_ranges {
	set base [lindex $range 0]
	set size [lindex $range 1]
	if { $base == [expr $base0 + $size0] } {
	    set size0 [expr $size0 + $size]
	    if { $base0 == 0xfffc0000 && $size0 >= $required_mem_size } return
	} else {
	    lappend concatenated_ranges [list $base0 $size0]
	    set base0 $base
	    set size0 $size
	}
    }

    # Check if any region is of required size
    lappend concatenated_ranges [list $base0 $size0]
    set concatenated_ranges [lsort -integer -index 0 $concatenated_ranges]
    foreach range $concatenated_ranges {
	set base [lindex $range 0]
	set size [lindex $range 1]
	if { $base == 0xfffc0000 && $size >= $required_mem_size } return
    }

    error "This application requires at least [expr $required_mem_size/1024] KB of OCM memory at 0xfffc0000 to run"
}

proc swapp_is_supported_sw {} {
    # make sure we are using standalone OS
    check_standalone_os;

    set lib_list_missing ""
    set has_missing_libs 0

    # make sure xilffs, xilrsa and xilpm are available
    set lib_needed "xilffs xilsecure xilpm"
    set lib_list [hsi::get_libs];

    foreach libs ${lib_needed} {
        # create a list of required libs that are not in BSP
        if {[lsearch $lib_list $libs] < 0 } {
            lappend lib_list_missing $libs
            set has_missing_libs [expr $has_missing_libs + 1]
        }
    }

    if {$has_missing_libs > 0} {
        error "These libraries which FSBL requires are missing in Board Support Package: $lib_list_missing"
    }
}

proc swapp_is_supported_hw {} {

    set ip_list_iso ""
    set ip_list_nodsgn ""
    set str_iso ""
    set str_nodsgn ""
    set has_iso_ips 0
    set has_nodsgn_ips 0

    # check processor type
    set proc_instance [hsi::get_sw_processor];
    set hw_processor [common::get_property HW_INSTANCE $proc_instance]

    set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];

    if { $proc_type != "psu_cortexr5" && $proc_type != "psu_cortexa53" } {
                error "This application is supported only for CortexA53/CortexR5 processors.";
    }

    if { $proc_instance != "psu_cortexr5_0" && $proc_instance != "psu_cortexa53_0" } {
                error "This application is supported only for CortexA53_0 and CortexR5_0.";
    }

    # csudma, amda_0 and any one instance of ipi are required IPs for FSBL.
    # In addition, for ZCU102 and ZCU106, i2c0 is needed for board-specific
    #  configuration done in FSBL. Hence, print the required (for FSBL) IPs
    # which are in design but are isolated from the selected processor.
    # Also, print the IPs which are required but are not present in design.

    set boardname [common::get_property BOARD [hsi::current_hw_design]]

    if { [string length $boardname] != 0 } {
        set fields [split $boardname ":"]
        lassign $fields prefix board suffix

        if { $board == "zcu102" || $board == "zcu106" } {
            set ip_list "psu_csudma psu_adma_0 psu_ipi_* psu_i2c_0"
        } else {
            set ip_list "psu_csudma psu_adma_0 psu_ipi_*"
        }
    } else {
        set ip_list "psu_csudma psu_adma_0 psu_ipi_*"
    }

    set ip_iso [::common::get_property SLAVES [::hsi::get_cells $proc_instance -hier]]
    set ip_nodsgn [::hsi::get_cells -hier]

    foreach ips ${ip_list} {
        # create a list of IPs that are in design but isolated for this processor
        if {[lsearch $ip_iso $ips] < 0 } {
            if {[lsearch $ip_nodsgn $ips] >= 0} {
            lappend ip_list_iso $ips
            set has_iso_ips [expr $has_iso_ips + 1]
            }
        }

        # create a list of IPs that are not in design
        if {[lsearch $ip_nodsgn $ips] < 0 } {
            lappend ip_list_nodsgn $ips
            set has_nodsgn_ips [expr $has_nodsgn_ips + 1]
        }
    }

    if {$has_iso_ips > 0} {
        set str_iso "FSBL uses these IPs but are isolated from the selected processor: $ip_list_iso."
    }

    if {$has_nodsgn_ips > 0} {
        set str_nodsgn " FSBL uses these IPs but are missing in the design: $ip_list_nodsgn"
    }

    append str_iso $str_nodsgn

    if { [llength $str_iso] != 0 } {
        error $str_iso
    }

    #check if the design has minimum memory for the program to run
    check_program_memory

    return 1;
}


proc get_stdout {} {
    set os [hsi::get_os];
    set stdout [common::get_property CONFIG.STDOUT $os];
    return $stdout;
}

proc check_stdout_hw {} {
    set pu_uarts [hsi::get_cells -hier -filter "IP_NAME=psu_uart"];
}

proc swapp_generate {} {
    # generate/copy ps init files
    ::hsi::utils::generate_psinit

    #delete unnecessary files

    set files(0) "psu_init.html"
    set files(1) "psu_init.tcl"
    set files(2) "psu_init_gpl.c"
    set files(3) "psu_init_gpl.h"
    set files(4) "psu_pmucfg.c"
    set files(5) "psu_clock_registers.log"
    set files(6) "psu_power_report.log"
    set files(7) "Makefile"

    foreach init_file [array get files] {
        file delete -force $init_file
    }

    set proc_instance [hsi::get_sw_processor];
    set hw_processor [common::get_property HW_INSTANCE $proc_instance]
    set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];

    # get the compiler flags, if set already
    set def_flags [common::get_property APP_COMPILER_FLAGS [hsi::current_sw_design]]
    set def_link_flags [common::get_property APP_LINKER_FLAGS [hsi::current_sw_design]]

    # based on the CPU (A53 64-bit, A53 32-bit or R5),
    # remove unnecessary linker script and retain just one: lscript.ld
    # copy the corresponding translation table for A53 (64-bit and 32-bit)
    # set the compiler flags
    set trans_tbl_a53_64 "xfsbl_translation_table_a53_64.S"
    set trans_tbl_a53_32 "xfsbl_translation_table_a53_32.S"
    set trans_tbl_a53 "xfsbl_translation_table.S"
    if { $proc_type == "psu_cortexr5" } {
        set ld_file_a53 "lscript_a53.ld"
        file delete -force $ld_file_a53

        file delete -force $trans_tbl_a53_64
        file delete -force $trans_tbl_a53_32

        set new_flags "-Wall -fmessage-length=0 -mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16 -DARMR5 -Os -flto -ffat-lto-objects $def_flags"
    } else {
        set compiler [common::get_property CONFIG.compiler $proc_instance]

        if {[string compare -nocase $compiler "arm-none-eabi-gcc"] == 0} {
            # A53 32-bit : Use same linker script as that of R5
            set ld_file_a53 "lscript_a53.ld"
            file delete -force $ld_file_a53

            file delete -force $trans_tbl_a53_64
            file rename -force $trans_tbl_a53_32 $trans_tbl_a53

            set new_flags "-Wall -fmessage-length=0 -march=armv7-a -mfpu=vfpv3 -mfloat-abi=hard -DARMA53_32 -Os -flto -ffat-lto-objects $def_flags"
        } else {
            #A53 64-bit
            set ld_file "lscript.ld"
            file delete -force $ld_file

            set ld_file_a53 "lscript_a53.ld"
            set ld_file_new "lscript.ld"
            file rename -force $ld_file_a53 $ld_file_new

            file delete -force $trans_tbl_a53_32
            file rename -force $trans_tbl_a53_64 $trans_tbl_a53

            set new_flags "-Wall -fmessage-length=0 -DARMA53_64 -Os -flto -ffat-lto-objects $def_flags"
        }
    }

    set new_link_flags "-n $def_link_flags"
    # Update compiler and linker flags
    common::set_property -name {APP_COMPILER_FLAGS} -value $new_flags -objects [hsi::current_sw_design]
    common::set_property -name {APP_LINKER_FLAGS} -value $new_link_flags -objects [hsi::current_sw_design]
}

proc swapp_get_linker_constraints {} {

    # don't generate a linker script. fsbl has its own linker script
    return "lscript no";
}
