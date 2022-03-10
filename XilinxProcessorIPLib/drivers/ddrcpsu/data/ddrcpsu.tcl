###############################################################################
# Copyright (C) 2015 - 2022 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 1.0	    ssc   04/28/16 First Release.
# 1.4       mus   11/17/21 Updated to export base and high
#                          address for psu_ddr instances.
# 1.4       mus   12/22/21 Updated xdefine_include_file and
#                          xdefine_canonical_xpars procs to initialize the
#                          the variables before using them. It fixes CR#1118044
# 1.4       dp    03/08/22 Updated define_addr_params to handle special case
#                          for FSBL to define whole DDR map irrespective of DDR
#                          mapped for the core. Also, updated to define all
#                          regions of DDR mapped to the core and not just the
#                          first region. It fixes CR#1118988
#
###############################################################################

proc generate {drv_handle} {
    global paramlist
    set paramlist ""
    xdefine_include_file $drv_handle "xparameters.h" "XDdrcPsu" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_HAS_ECC" "C_DDRC_CLK_FREQ_HZ"

    get_ddr_config_info $drv_handle "xparameters.h"

    xdefine_canonical_xpars $drv_handle "xparameters.h" "DdrcPsu" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_DDRC_CLK_FREQ_HZ"

    get_ddr_config_info_canonical $drv_handle "xparameters.h"

}



proc xdefine_include_file {drv_handle file_name drv_string args} {
    # Get all peripherals connected to this driver
    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]
    set ddr_devid 0
    set ddrc_periphs ""

    # Handle special cases
    set arg "NUM_INSTANCES"
    set posn [lsearch -exact $args $arg]
    if {$posn > -1} {
	# Open include file
	set file_handle [::hsi::utils::open_include_file $file_name]
	puts $file_handle "/*Definitions for driver [string toupper [common::get_property NAME $drv_handle]] */"
	# Define NUM_INSTANCES, consider only psu_ddrc instances
	foreach periph $periphs {
		set ip_type [common::get_property IP_NAME $periph]
		# Skip "psu_ddr" instances
		if {$ip_type != "psu_ddr"} {
			lappend ddrc_periphs $periph
		}
	}
	puts $file_handle "#define [::hsi::utils::get_driver_param_name $drv_string $arg] [llength $ddrc_periphs]"
	set args [lreplace $args $posn $posn]
	close $file_handle
    }
    foreach periph $periphs {
        set device_id 0
	set ip_type [common::get_property IP_NAME $periph]

	if {$ip_type == "psu_ddrc"} {
		set file_handle [::hsi::utils::open_include_file $file_name]
		puts $file_handle ""
		puts $file_handle "/* Definitions for peripheral [string toupper [common::get_property NAME $periph]] */"
		foreach arg $args {
			if {[string compare -nocase "DEVICE_ID" $arg] == 0} {
				set value $device_id
				incr device_id
			} else {
				set value [::hsi::utils::get_param_value $periph $arg]
			}

			if {[llength $value] == 0} {
				set value 0
			}
			set value [::hsi::utils::format_addr_string $value $arg]
			if {[string match C_* $arg]} {
				set arg_name [string range $arg 2 end]
			} else {
				set arg_name $arg
			}
			set arg_name [format "XPAR_%s_%s" [string toupper [common::get_property NAME $periph]] $arg_name]
			regsub "S_AXI_" $arg_name "" arg_name
			puts $file_handle "#define $arg_name $value"
		}
		puts $file_handle "/******************************************************************/"
		close $file_handle
	} else {
		# For psu_ddr instances, just export base
		# and high address to xparameters.h
		define_addr_params $periph "xparameters.h" "peripheral" $ddr_devid
		incr ddr_devid
	}
    }
}

proc define_addr_params {periph file_name type device_id} {

	global paramlist
   # Open include file
   set file_handle [::hsi::utils::open_include_file $file_name]

   set sw_proc [hsi::get_sw_processor]

   set ip_name [common::get_property IP_NAME $periph]

   set addr_params [list]
   set interface_base_names [get_property BASE_NAME [get_mem_ranges \
	-of_objects [get_cells -hier $sw_proc] $periph]]
   set interface_high_names [get_property HIGH_NAME [get_mem_ranges \
	-of_objects [get_cells -hier $sw_proc] $periph]]
   set i 0

   set is_zynqmp_fsbl_bsp [common::get_property CONFIG.ZYNQMP_FSBL_BSP [hsi::get_os]]
   set sw_proc_handle [hsi::get_sw_processor]
   set hw_proc_handle [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc_handle] ]
   set proctype [common::get_property IP_NAME $hw_proc_handle]

   foreach interface_base $interface_base_names interface_high $interface_high_names {
		set base_name [common::get_property BASE_NAME [lindex [get_mem_ranges \
			-of_objects [get_cells -hier $sw_proc] $periph] $i]]
		set base_value [common::get_property BASE_VALUE [lindex [get_mem_ranges \
			-of_objects [get_cells -hier $sw_proc] $periph] $i]]
		set high_name [common::get_property HIGH_NAME [lindex [get_mem_ranges \
			-of_objects [get_cells -hier $sw_proc] $periph] $i]]
		set high_value [common::get_property HIGH_VALUE [lindex [get_mem_ranges \
			-of_objects [get_cells -hier $sw_proc] $periph] $i]]
		set bposn [lsearch -exact $addr_params $base_name]
		set hposn [lsearch -exact $addr_params $high_name]
		if {$bposn > -1  || $hposn > -1 } {
			continue
		}

		if {[string match -nocase $type "peripheral"]} {
			puts $file_handle ""
			puts $file_handle "/* Peripheral Definitions for peripheral [string toupper [common::get_property NAME $periph]] */"
			set lvalue [::hsi::utils::get_ip_param_name $periph $base_name]
			set paramlist [lappend paramlist $lvalue]

			set base_value [::hsi::utils::get_param_value $periph $base_name]
			set base_value [::hsi::utils::format_addr_string $base_value $base_name]
			set high_value [::hsi::utils::get_param_value $periph $high_name]
			set high_value [::hsi::utils::format_addr_string $high_value $high_name]
			puts $file_handle "#define $lvalue $base_value"
			set lvalue [::hsi::utils::get_ip_param_name $periph $high_name]
			puts $file_handle "#define $lvalue $high_value"
			define_ddr_xpars $periph $file_handle $interface_base_names $type $device_id
		} else {
			set lvalue [format "XPAR_%s%s%s" [common::get_property IP_NAME $periph] "_${device_id}_" [string range $base_name 2 end]]
			set lvalue [string toupper $lvalue]
			if {[lsearch -nocase $paramlist $lvalue] == -1} {
				puts $file_handle ""
				puts $file_handle "/* Canonical Definitions for peripheral [string toupper [common::get_property NAME $periph]] */"
				set paramlist [lappend paramlist $lvalue]
				set base_value [::hsi::utils::get_param_value $periph $base_name]
				set base_value [::hsi::utils::format_addr_string $base_value $base_name]
				set high_value [::hsi::utils::get_param_value $periph $high_name]
				set high_value [::hsi::utils::format_addr_string $high_value $high_name]
				puts $file_handle "#define $lvalue $base_value"
				set lvalue [format "XPAR_%s%s%s" [common::get_property IP_NAME $periph] "_${device_id}_" [string range $high_name 2 end]]
				set lvalue [string toupper $lvalue]
				puts $file_handle "#define $lvalue $high_value"
				define_ddr_xpars $periph $file_handle $interface_base_names $type $device_id
			} else {
				continue
			}

		}
		incr i
		lappend addr_params $base_name
		lappend addr_params $high_name

	}

   close $file_handle
}


proc define_ddr_xpars { periph file_handle interface_base_names type dev_id} {

	set sw_proc [hsi::get_sw_processor]
	set num_ddr_regions [llength $interface_base_names]
	set loop 0
	set lo_ddr 0
	set hi_ddr 0
	set base_name [common::get_property BASE_NAME [lindex [get_mem_ranges \
                        -of_objects [get_cells -hier $sw_proc] $periph] 0]]
	set high_name [common::get_property HIGH_NAME [lindex [get_mem_ranges \
                        -of_objects [get_cells -hier $sw_proc] $periph] 0]]

	while {$loop < $num_ddr_regions} {

		set base_value [common::get_property BASE_VALUE [lindex [get_mem_ranges \
				-of_objects [get_cells -hier $sw_proc] $periph] $loop]]
		set high_value [common::get_property HIGH_VALUE [lindex [get_mem_ranges \
				-of_objects [get_cells -hier $sw_proc] $periph] $loop]]

		if {$loop == 0 } {
			set lo_ddr $base_value
			set hi_ddr $high_value
			if { $num_ddr_regions == 1 } {
				break
			}
		} else {
			if { $base_value < $lo_ddr } {
				set lo_ddr $base_value
			}
			if { $high_value > $hi_ddr } {
				set hi_ddr $high_value
			}
		}
		if {[string match -nocase $type "peripheral"]} {
			set lvalue [common::get_property NAME $periph]
			set lvalue [string toupper $lvalue]
			set lvalue [format "XPAR_%s" $lvalue]
			set lvalue [format "%s_%s_%s" $lvalue $loop $base_name]
		} else {
			set lvalue [format "XPAR_%s%s%s_%s" [common::get_property IP_NAME $periph] \
				"_${dev_id}_" "${loop}" [string range $base_name 2 end]]
			set lvalue [string toupper $lvalue]
		}
		puts $file_handle "#define $lvalue $base_value"

		if {[string match -nocase $type "peripheral"]} {
			set lvalue [common::get_property NAME $periph]
			set lvalue [string toupper $lvalue]
			set lvalue [format "XPAR_%s" $lvalue]
			set lvalue [format "%s_%s_%s" $lvalue $loop $high_name]
		} else {
			set lvalue [format "XPAR_%s%s%s_%s" [common::get_property IP_NAME $periph] \
				"_${dev_id}_" "${loop}" [string range $high_name 2 end]]
			set lvalue [string toupper $lvalue]

		}
		puts $file_handle "#define $lvalue $high_value"
		incr loop
	}
	set hw_proc_handle [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc] ]
        set proctype [common::get_property IP_NAME $hw_proc_handle]

	if {[string match -nocase $type "peripheral"] && $proctype == "psu_cortexr5" } {
		set lvalue [common::get_property NAME $periph]
		set lvalue [string toupper $lvalue]
		set lvalue [format "XPAR_%s_%s" $lvalue "LOW_ADDR"]
		puts $file_handle "#define $lvalue $lo_ddr"
		set lvalue [common::get_property NAME $periph]
		set lvalue [string toupper $lvalue]
		set lvalue [format "XPAR_%s_%s" $lvalue "HIGH_ADDR"]
		puts $file_handle "#define $lvalue $hi_ddr"

	}
	puts $file_handle "\n/******************************************************************/\n"
}

proc xdefine_canonical_xpars {drv_handle file_name drv_string args} {
    set ddrc_periphs ""
    set peripherals ""
    set indices ""
    set canonicals ""

    # Open include file
    set file_handle [::hsi::utils::open_include_file $file_name]
    set ddr_devid 0

    # Get all the peripherals connected to this driver
    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

    # Get the names of all the peripherals connected to this driver
    foreach periph $periphs {
        set ip_type [common::get_property IP_NAME $periph]
	if {$ip_type == "psu_ddr"} {
		define_addr_params $periph "xparameters.h" "canonical" $ddr_devid
		incr ddr_devid
	} else {
		set peripheral_name [string toupper [common::get_property NAME $periph]]
		lappend peripherals $peripheral_name
		lappend ddrc_periphs $periph
	}
    }

    # Get possible canonical names for all the peripherals connected to this
    # driver
    set device_id 0
    foreach periph $ddrc_periphs {
        set canonical_name [string toupper [format "%s_%s" $drv_string $device_id]]
        lappend canonicals $canonical_name

        # Create a list of IDs of the peripherals whose hardware instance name
        # doesn't match the canonical name. These IDs can be used later to
        # generate canonical definitions
        if { [lsearch $peripherals $canonical_name] < 0 } {
            lappend indices $device_id
        }
        incr device_id
    }

    set i 0
    set device_id_l 0
    foreach periph $ddrc_periphs {
        # Set device_id to ttc instance number
        set device_id 0
        set periph_name [string toupper [common::get_property NAME $periph]]

        # Generate canonical definitions only for the peripherals whose
        # canonical name is not the same as hardware instance name
        if { [lsearch $canonicals $periph_name] < 0 } {
            puts $file_handle "/* Canonical definitions for peripheral $periph_name */"
            set canonical_name [format "%s_%s" $drv_string [lindex $indices $i]]

            foreach arg $args {
		if {[string match C_* $arg]} {
                        set arg_name [string range $arg 2 end]
                } else {
                        set arg_name $arg
                }
                set lvalue [format "XPAR_%s_%d_%s" [string toupper $drv_string] $device_id_l $arg_name]
                regsub "S_AXI_" $lvalue "" lvalue

                # The commented out rvalue is the name of the instance-specific constant
                # set rvalue [::hsi::utils::get_ip_param_name $periph $arg]
                # The rvalue set below is the actual value of the parameter
                if {[string compare -nocase "DEVICE_ID" $arg] == 0} {
			set rvalue [format "XPAR_%s_%s" [string toupper [common::get_property NAME $periph]] $arg]
                } else {
			set rvalue [::hsi::utils::get_param_value $periph $arg]
                        if {[llength $rvalue] == 0} {
                            set rvalue 0
                        }
                        set rvalue [::hsi::utils::format_addr_string $rvalue $arg]
                }
                puts $file_handle "#define $lvalue $rvalue"
            }
            puts $file_handle ""
            incr i

        }
        incr device_id
	incr device_id_l
    }

    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}

proc get_ddr_config_info {drv_handle file_name} {
	set file_handle [::hsi::utils::open_include_file $file_name]
	set periph_list [get_cells -hier]
	foreach periph $periph_list {
		set zynq_ultra_ps [get_property IP_NAME $periph]
		if {[string match -nocase $zynq_ultra_ps "zynq_ultra_ps_e"] } {
			set avail_param [list_property [get_cells -hier $periph]]
			if {[lsearch -nocase $avail_param "CONFIG.PSU__DDRC__DDR4_ADDR_MAPPING"] >= 0} {
				set nr_addrmap [get_property CONFIG.PSU__DDRC__DDR4_ADDR_MAPPING [get_cells -hier $periph]]
				if {$nr_addrmap == 1} {
					puts $file_handle "#define XPAR_PSU_DDRC_0_DDR4_ADDR_MAPPING 1"
				} else {
					puts $file_handle "#define XPAR_PSU_DDRC_0_DDR4_ADDR_MAPPING 0"
				}
			}
			if {[lsearch -nocase $avail_param "CONFIG.PSU__ACT_DDR_FREQ_MHZ"] >= 0} {
				set nr_freq [get_property CONFIG.PSU__ACT_DDR_FREQ_MHZ [get_cells -hier $periph]]
				puts $file_handle "#define XPAR_PSU_DDRC_0_DDR_FREQ_MHZ $nr_freq"
			}
			if {[lsearch -nocase $avail_param "CONFIG.PSU__DDRC__VIDEO_BUFFER_SIZE"] >= 0} {
				set nr_vbs [get_property CONFIG.PSU__DDRC__VIDEO_BUFFER_SIZE [get_cells -hier $periph]]
				puts $file_handle "#define XPAR_PSU_DDRC_0_VIDEO_BUFFER_SIZE $nr_vbs"
			}
			if {[lsearch -nocase $avail_param "CONFIG.PSU__DDRC__BRC_MAPPING"] >= 0} {
				set nr_brc [get_property CONFIG.PSU__DDRC__BRC_MAPPING [get_cells -hier $periph]]
				if { [string compare -nocase $nr_brc "ROW_BANK_COL"] == 0 } {
					puts $file_handle "#define XPAR_PSU_DDRC_0_BRC_MAPPING 0"
				} else {
					puts $file_handle "#define XPAR_PSU_DDRC_0_BRC_MAPPING 1"
				}
			}
		}
	}
	set ips [::hsi::utils::get_common_driver_ips $drv_handle]
	foreach ip $ips {
		set avail_param [list_property [get_cells -hier $ip]]
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_DYNAMIC_DDR_CONFIG_ENABLED"] >= 0} {
			set ddr_dyn [get_property CONFIG.C_DDRC_DYNAMIC_DDR_CONFIG_ENABLED [get_cells -hier $ip]]
			if {$ddr_dyn == 1} {
				puts $file_handle "#define XPAR_DYNAMIC_DDR_ENABLED"
			}
		}
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_ECC"] >= 0} {
			set ecc_enable [get_property CONFIG.C_DDRC_ECC [get_cells -hier $ip]]
			puts $file_handle "#define XPAR_PSU_DDRC_0_DDR_ECC $ecc_enable"
		}
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_MEMORY_TYPE"] >= 0} {
			set mem_type [get_property CONFIG.C_DDRC_MEMORY_TYPE [get_cells -hier $ip]]
			puts $file_handle "#define XPAR_PSU_DDRC_0_DDR_MEMORY_TYPE $mem_type"
		}
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_MEMORY_ADDRESS_MAP"] >= 0} {
			set addr_map [get_property CONFIG.C_DDRC_MEMORY_ADDRESS_MAP [get_cells -hier $ip]]
			puts $file_handle "#define XPAR_PSU_DDRC_0_DDR_MEMORY_ADDRESS_MAP $addr_map"
		}
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_DATA_MASK_AND_DBI"] >= 0} {
			set dm_dbi [get_property CONFIG.C_DDRC_DATA_MASK_AND_DBI [get_cells -hier $ip]]
			puts $file_handle "#define XPAR_PSU_DDRC_0_DDR_DATA_MASK_AND_DBI $dm_dbi"
		}
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_ADDRESS_MIRRORING"] >= 0} {
			set addr_mirr [get_property CONFIG.C_DDRC_ADDRESS_MIRRORING [get_cells -hier $ip]]
			puts $file_handle "#define XPAR_PSU_DDRC_0_DDR_ADDRESS_MIRRORING $addr_mirr"
		}
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_2ND_CLOCK"] >= 0} {
			set clock2 [get_property CONFIG.C_DDRC_2ND_CLOCK [get_cells -hier $ip]]
			puts $file_handle "#define XPAR_PSU_DDRC_0_DDR_2ND_CLOCK $clock2"
		}
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_PARITY"] >= 0} {
			set parity [get_property CONFIG.C_DDRC_PARITY [get_cells -hier $ip]]
			if {$parity == 1} {
				puts $file_handle "#define XPAR_PSU_DDRC_0_DDR_PARITY 1"
			} else {
				puts $file_handle "#define XPAR_PSU_DDRC_0_DDR_PARITY 0"
			}
		}
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_POWER_DOWN_ENABLE"] >= 0} {
			set pwr_dwn [get_property CONFIG.C_DDRC_POWER_DOWN_ENABLE [get_cells -hier $ip]]
			if {$pwr_dwn == 1} {
				puts $file_handle "#define XPAR_PSU_DDRC_0_DDR_POWER_DOWN_ENABLE 1"
			} else {
				puts $file_handle "#define XPAR_PSU_DDRC_0_DDR_POWER_DOWN_ENABLE 0"
			}
		}
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_CLOCK_STOP"] >= 0} {
			set clk_stop [get_property CONFIG.C_DDRC_CLOCK_STOP [get_cells -hier $ip]]
			if {$clk_stop == 1} {
				puts $file_handle "#define XPAR_PSU_DDRC_0_CLOCK_STOP 1"
			} else {
				puts $file_handle "#define XPAR_PSU_DDRC_0_CLOCK_STOP 0"
			}
		}
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_LOW_POWER_AUTO_SELF_REFRESH"] >= 0} {
			set lpasf [get_property CONFIG.C_DDRC_LOW_POWER_AUTO_SELF_REFRESH [get_cells -hier $ip]]
			if {$lpasf == 1} {
				puts $file_handle "#define XPAR_PSU_DDRC_0_DDR_LOW_POWER_AUTO_SELF_REFRESH 1"
			} else {
				puts $file_handle "#define XPAR_PSU_DDRC_0_DDR_LOW_POWER_AUTO_SELF_REFRESH 0"
			}
		}
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_TEMP_CONTROLLED_REFRESH"] >= 0} {
			set temp_ref [get_property CONFIG.C_DDRC_TEMP_CONTROLLED_REFRESH [get_cells -hier $ip]]
			if {$temp_ref == 1} {
				puts $file_handle "#define XPAR_PSU_DDRC_0_DDR_TEMP_CONTROLLED_REFRESH 1"
			} else {
				puts $file_handle "#define XPAR_PSU_DDRC_0_DDR_TEMP_CONTROLLED_REFRESH 0"
			}
		}
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_MAX_OPERATING_TEMPARATURE"] >= 0} {
			set max_temp [get_property CONFIG.C_DDRC_MAX_OPERATING_TEMPARATURE [get_cells -hier $ip]]
			if {$max_temp == 1} {
				puts $file_handle "#define XPAR_PSU_DDRC_0_DDR_MAX_OPERATING_TEMPARATURE 1"
			} else {
				puts $file_handle "#define XPAR_PSU_DDRC_0_DDR_MAX_OPERATING_TEMPARATURE 0"
			}
		}
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_FINE_GRANULARITY_REFRESH_MODE"] >= 0} {
			set fgrm [get_property CONFIG.C_DDRC_FINE_GRANULARITY_REFRESH_MODE [get_cells -hier $ip]]
			if {$fgrm >= 1} {
				puts $file_handle "#define XPAR_PSU_DDRC_0_DDR_FINE_GRANULARITY_REFRESH_MODE $fgrm"
			} else {
				puts $file_handle "#define XPAR_PSU_DDRC_0_DDR_FINE_GRANULARITY_REFRESH_MODE 0"
			}
		}
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_SELF_REFRESH_ABORT"] >= 0} {
			set ref_abort [get_property CONFIG.C_DDRC_SELF_REFRESH_ABORT [get_cells -hier $ip]]
			if {$ref_abort == 1} {
				puts $file_handle "#define XPAR_PSU_DDRC_0_DDR_SELF_REFRESH_ABORT 1"
			} else {
				puts $file_handle "#define XPAR_PSU_DDRC_0_DDR_SELF_REFRESH_ABORT 0"
			}
		}
	}
	close $file_handle
}

proc get_ddr_config_info_canonical {drv_handle file_name} {
	set file_handle [::hsi::utils::open_include_file $file_name]
	set periph_list [get_cells -hier]
	foreach periph $periph_list {
		set zynq_ultra_ps [get_property IP_NAME $periph]
		if {[string match -nocase $zynq_ultra_ps "zynq_ultra_ps_e"] } {
			set avail_param [list_property [get_cells -hier $periph]]
			if {[lsearch -nocase $avail_param "CONFIG.PSU__DDRC__DDR4_ADDR_MAPPING"] >= 0} {
				set nr_addrmap [get_property CONFIG.PSU__DDRC__DDR4_ADDR_MAPPING [get_cells -hier $periph]]
				if {$nr_addrmap == 1} {
					puts $file_handle "#define XPAR_DDRCPSU_0_DDR4_ADDR_MAPPING 1"
				} else {
					puts $file_handle "#define XPAR_DDRCPSU_0_DDR4_ADDR_MAPPING 0"
				}
			}
			if {[lsearch -nocase $avail_param "CONFIG.PSU__ACT_DDR_FREQ_MHZ"] >= 0} {
				set nr_freq [get_property CONFIG.PSU__ACT_DDR_FREQ_MHZ [get_cells -hier $periph]]
				puts $file_handle "#define XPAR_DDRCPSU_0_DDR_FREQ_MHZ $nr_freq"
			}
			if {[lsearch -nocase $avail_param "CONFIG.PSU__DDRC__VIDEO_BUFFER_SIZE"] >= 0} {
				set nr_vbs [get_property CONFIG.PSU__DDRC__VIDEO_BUFFER_SIZE [get_cells -hier $periph]]
				puts $file_handle "#define XPAR_DDRCPSU_0_VIDEO_BUFFER_SIZE $nr_vbs"
			}
			if {[lsearch -nocase $avail_param "CONFIG.PSU__DDRC__BRC_MAPPING"] >= 0} {
				set nr_brc [get_property CONFIG.PSU__DDRC__BRC_MAPPING [get_cells -hier $periph]]
				if { [string compare -nocase $nr_brc "ROW_BANK_COL"] == 0 } {
					puts $file_handle "#define XPAR_DDRCPSU_0_BRC_MAPPING 0"
				} else {
					puts $file_handle "#define XPAR_DDRCPSU_0_BRC_MAPPING 1"
				}
			}
		}
	}

	set ips [::hsi::utils::get_common_driver_ips $drv_handle]
	foreach ip $ips {
		set avail_param [list_property [get_cells -hier $ip]]
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_ECC"] >= 0} {
			set ecc_enable [get_property CONFIG.C_DDRC_ECC [get_cells -hier $ip]]
			puts $file_handle "#define XPAR_DDRCPSU_0_DDR_ECC $ecc_enable"
		}
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_MEMORY_TYPE"] >= 0} {
			set mem_type [get_property CONFIG.C_DDRC_MEMORY_TYPE [get_cells -hier $ip]]
			puts $file_handle "#define XPAR_DDRCPSU_0_DDR_MEMORY_TYPE $mem_type"
		}
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_MEMORY_ADDRESS_MAP"] >= 0} {
			set addr_map [get_property CONFIG.C_DDRC_MEMORY_ADDRESS_MAP [get_cells -hier $ip]]
			puts $file_handle "#define XPAR_DDRCPSU_0_DDR_MEMORY_ADDRESS_MAP $addr_map"
		}
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_DATA_MASK_AND_DBI"] >= 0} {
			set dm_dbi [get_property CONFIG.C_DDRC_DATA_MASK_AND_DBI [get_cells -hier $ip]]
			puts $file_handle "#define XPAR_DDRCPSU_0_DDR_DATA_MASK_AND_DBI $dm_dbi"
		}
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_ADDRESS_MIRRORING"] >= 0} {
			set addr_mirr [get_property CONFIG.C_DDRC_ADDRESS_MIRRORING [get_cells -hier $ip]]
			puts $file_handle "#define XPAR_DDRCPSU_0_DDR_ADDRESS_MIRRORING $addr_mirr"
		}
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_2ND_CLOCK"] >= 0} {
			set clock2 [get_property CONFIG.C_DDRC_2ND_CLOCK [get_cells -hier $ip]]
			puts $file_handle "#define XPAR_DDRCPSU_0_DDR_2ND_CLOCK $clock2"
		}
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_PARITY"] >= 0} {
			set parity [get_property CONFIG.C_DDRC_PARITY [get_cells -hier $ip]]
			if {$parity == 1} {
				puts $file_handle "#define XPAR_DDRCPSU_0_DDR_PARITY 1"
			} else {
				puts $file_handle "#define XPAR_DDRCPSU_0_DDR_PARITY 0"
			}
		}
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_POWER_DOWN_ENABLE"] >= 0} {
			set pwr_dwn [get_property CONFIG.C_DDRC_POWER_DOWN_ENABLE [get_cells -hier $ip]]
			if {$pwr_dwn == 1} {
				puts $file_handle "#define XPAR_DDRCPSU_0_DDR_POWER_DOWN_ENABLE 1"
			} else {
				puts $file_handle "#define XPAR_DDRCPSU_0_DDR_POWER_DOWN_ENABLE 0"
			}
		}
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_CLOCK_STOP"] >= 0} {
			set clk_stop [get_property CONFIG.C_DDRC_CLOCK_STOP [get_cells -hier $ip]]
			if {$clk_stop == 1} {
				puts $file_handle "#define XPAR_DDRCPSU_0_CLOCK_STOP 1"
			} else {
				puts $file_handle "#define XPAR_DDRCPSU_0_CLOCK_STOP 0"
			}
		}
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_LOW_POWER_AUTO_SELF_REFRESH"] >= 0} {
			set lpasf [get_property CONFIG.C_DDRC_LOW_POWER_AUTO_SELF_REFRESH [get_cells -hier $ip]]
			if {$lpasf == 1} {
				puts $file_handle "#define XPAR_DDRCPSU_0_DDR_LOW_POWER_AUTO_SELF_REFRESH 1"
			} else {
				puts $file_handle "#define XPAR_DDRCPSU_0_DDR_LOW_POWER_AUTO_SELF_REFRESH 0"
			}
		}
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_TEMP_CONTROLLED_REFRESH"] >= 0} {
			set temp_ref [get_property CONFIG.C_DDRC_TEMP_CONTROLLED_REFRESH [get_cells -hier $ip]]
			if {$temp_ref == 1} {
				puts $file_handle "#define XPAR_DDRCPSU_0_DDR_TEMP_CONTROLLED_REFRESH 1"
			} else {
				puts $file_handle "#define XPAR_DDRCPSU_0_DDR_TEMP_CONTROLLED_REFRESH 0"
			}
		}
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_MAX_OPERATING_TEMPARATURE"] >= 0} {
			set max_temp [get_property CONFIG.C_DDRC_MAX_OPERATING_TEMPARATURE [get_cells -hier $ip]]
			if {$max_temp == 1} {
				puts $file_handle "#define XPAR_DDRCPSU_0_DDR_MAX_OPERATING_TEMPARATURE 1"
			} else {
				puts $file_handle "#define XPAR_DDRCPSU_0_DDR_MAX_OPERATING_TEMPARATURE 0"
			}
		}
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_FINE_GRANULARITY_REFRESH_MODE"] >= 0} {
			set fgrm [get_property CONFIG.C_DDRC_FINE_GRANULARITY_REFRESH_MODE [get_cells -hier $ip]]
			if {$fgrm >= 1} {
				puts $file_handle "#define XPAR_DDRCPSU_0_DDR_FINE_GRANULARITY_REFRESH_MODE $fgrm"
			} else {
				puts $file_handle "#define XPAR_DDRCPSU_0_DDR_FINE_GRANULARITY_REFRESH_MODE 0"
			}
		}
		if {[lsearch -nocase $avail_param "CONFIG.C_DDRC_SELF_REFRESH_ABORT"] >= 0} {
			set ref_abort [get_property CONFIG.C_DDRC_SELF_REFRESH_ABORT [get_cells -hier $ip]]
			if {$ref_abort == 1} {
				puts $file_handle "#define XPAR_DDRCPSU_0_DDR_SELF_REFRESH_ABORT 1"
			} else {
				puts $file_handle "#define XPAR_DDRCPSU_0_DDR_SELF_REFRESH_ABORT 0"
			}
		}
	}
	close $file_handle
}
