###############################################################################
# Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
# MODIFICATION HISTORY:
#
# Ver      Who    Date     Changes
# -------- ------ -------- ----------------------------------------------------
# 03/22/10 rkv First Release 
# 09/06/13 srt Fixed CR 734175:
#              C_BASEADDR and C_HIGHADDR configuration parameters are renamed
#	       to BASEADDR and HIGHADDR in Vivado builds. Modified the tcl
#              for this change. 
#
# 3.0     adk    10/12/13 Updated as per the New Tcl API's 
#
###############################################################################
#uses "xillib.tcl"

proc generate {drv_handle} {
    xdefine_pcie_include_file $drv_handle "xparameters.h" "XAxiPcie" \
        "NUM_INSTANCES" \
        "DEVICE_ID" \
        "C_FAMILY" \
        "C_BASEADDR" \
        "C_HIGHADDR" \
        "C_INCLUDE_BAROFFSET_REG"\
        "C_AXIBAR_NUM"\
	"C_AXIBAR_0"\
	"C_AXIBAR_HIGHADDR_0"\
	"C_AXIBAR_AS_0"\
	"C_AXIBAR2PCIEBAR_0"\
	"C_AXIBAR_1"\
	"C_AXIBAR_HIGHADDR_1"\
	"C_AXIBAR_AS_1"\
	"C_AXIBAR2PCIEBAR_1"\
	"C_AXIBAR_2"\
	"C_AXIBAR_HIGHADDR_2"\
	"C_AXIBAR_AS_2"\
	"C_AXIBAR2PCIEBAR_2"\
	"C_AXIBAR_3"\
	"C_AXIBAR_HIGHADDR_3"\
	"C_AXIBAR_AS_3"\
	"C_AXIBAR2PCIEBAR_3"\
	"C_AXIBAR_4"\
	"C_AXIBAR_HIGHADDR_4"\
	"C_AXIBAR_AS_4"\
	"C_AXIBAR2PCIEBAR_4"\
	"C_AXIBAR_5"\
	"C_AXIBAR_HIGHADDR_5"\
	"C_AXIBAR_AS_5"\
	"C_AXIBAR2PCIEBAR_5"\
	"C_PCIEBAR_NUM"\
	"C_PCIEBAR_AS"\
	"C_PCIEBAR_LEN_0"\
	"C_PCIEBAR2AXIBAR_0"\
	"C_PCIEBAR_LEN_1"\
	"C_PCIEBAR2AXIBAR_1"\
	"C_PCIEBAR_LEN_2"\
	"C_PCIEBAR2AXIBAR_2"\
	"C_PCIEBAR_LEN_3"\
	"C_PCIEBAR2AXIBAR_3"\
	"C_PCIEBAR_LEN_4"\
	"C_PCIEBAR2AXIBAR_4"\
	"C_PCIEBAR_LEN_5"\
	"C_PCIEBAR2AXIBAR_5"\
	"C_INCLUDE_RC" 
  
        ::hsi::utils::define_config_file $drv_handle "xaxipcie_g.c" "XAxiPcie" \
        "DEVICE_ID" \
        "C_BASEADDR" \
        "C_AXIBAR_NUM" \
        "C_INCLUDE_BAROFFSET_REG" \
	"C_INCLUDE_RC" 
  
  
        xdefine_pcie_canonical_xpars $drv_handle "xparameters.h" "AxiPcie" \
        "DEVICE_ID" \
        "C_FAMILY" \
        "C_BASEADDR" \
        "C_HIGHADDR" \
        "C_INCLUDE_BAROFFSET_REG"\
        "C_AXIBAR_NUM"\
	"C_AXIBAR_0"\
	"C_AXIBAR_HIGHADDR_0"\
	"C_AXIBAR_AS_0"\
	"C_AXIBAR2PCIEBAR_0"\
	"C_AXIBAR_1"\
	"C_AXIBAR_HIGHADDR_1"\
	"C_AXIBAR_AS_1"\
	"C_AXIBAR2PCIEBAR_1"\
	"C_AXIBAR_2"\
	"C_AXIBAR_HIGHADDR_2"\
	"C_AXIBAR_AS_2"\
	"C_AXIBAR2PCIEBAR_2"\
	"C_AXIBAR_3"\
	"C_AXIBAR_HIGHADDR_3"\
	"C_AXIBAR_AS_3"\
	"C_AXIBAR2PCIEBAR_3"\
	"C_AXIBAR_4"\
	"C_AXIBAR_HIGHADDR_4"\
	"C_AXIBAR_AS_4"\
	"C_AXIBAR2PCIEBAR_4"\
	"C_AXIBAR_5"\
	"C_AXIBAR_HIGHADDR_5"\
	"C_AXIBAR_AS_5"\
	"C_AXIBAR2PCIEBAR_5"\
	"C_PCIEBAR_NUM"\
	"C_PCIEBAR_AS"\
	"C_PCIEBAR_LEN_0"\
	"C_PCIEBAR2AXIBAR_0"\
	"C_PCIEBAR_LEN_1"\
	"C_PCIEBAR2AXIBAR_1"\
	"C_PCIEBAR_LEN_2"\
	"C_PCIEBAR2AXIBAR_2"\
	"C_PCIEBAR_LEN_3"\
	"C_PCIEBAR2AXIBAR_3"\
	"C_PCIEBAR_LEN_4"\
	"C_PCIEBAR2AXIBAR_4"\
	"C_PCIEBAR_LEN_5"\
	"C_PCIEBAR2AXIBAR_5"\
 	"C_INCLUDE_RC" 
}

proc xdefine_pcie_include_file {drv_handle file_name drv_string args} {
    # Open include file
    set file_handle [::hsi::utils::open_include_file $file_name]

    # Get all peripherals connected to this driver
    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]
    set ipname [common::get_property IP_NAME [get_cells -hier $drv_handle]]
    # Handle special cases
    set arg "NUM_INSTANCES"
    set posn [lsearch -exact $args $arg]
    if {$posn > -1} {
        puts $file_handle "/* Definitions for driver [string toupper [common::get_property NAME $drv_handle]] */"
        # Define NUM_INSTANCES
        puts $file_handle "#define [::hsi::utils::get_driver_param_name $drv_string $arg] [llength $periphs]"
        set args [lreplace $args $posn $posn]
    }
    # Check if it is a driver parameter

    lappend newargs
    foreach arg $args {
        set value [common::get_property CONFIG.$arg $drv_handle]
        if {[llength $value] == 0} {
            lappend newargs $arg
        } else {
            puts $file_handle "#define [::hsi::utils::get_driver_param_name $drv_string $arg] [common::get_property CONFIG.$arg $drv_handle]"
        }
    }
    set args $newargs

    # Print all parameters for all peripherals
    set device_id 0
    foreach periph $periphs {
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

	    # For Vivado, C_BASEADDR is renamed to BASEADDR
	    if { $value == 0 && $arg == "C_BASEADDR" } {
		set arg "BASEADDR"
                set value [::hsi::utils::get_param_value $periph $arg]
	    }	

	    # For Vivado, C_HIGHADDR is renamed to HIGHADDR
	    if { $value == 0 && $arg == "C_HIGHADDR" } {
		set arg "HIGHADDR"
                set value [::hsi::utils::get_param_value $periph $arg]
	    }	
	    if { $value == 0 && $arg == "C_PCIEBAR_NUM" && [string match -nocase $ipname "axi_pcie3"]} {
		set arg "PCIEBAR_NUM"
                set value [::hsi::utils::get_param_value $periph $arg]
	    }

            set value [::hsi::utils::format_addr_string $value $arg]
            if {[string compare -nocase "HW_VER" $arg] == 0} {
                puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg] \"$value\""
            } else {
                puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg] $value"
            }
        }
        puts $file_handle ""
    }
    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}

proc xdefine_pcie_canonical_xpars {drv_handle file_name drv_string args} {
    # Open include file
    set file_handle [::hsi::utils::open_include_file $file_name]

    set ipname [common::get_property IP_NAME [get_cells -hier $drv_handle]]
    # Get all the peripherals connected to this driver
    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

    # Get the names of all the peripherals connected to this driver
    foreach periph $periphs {
        set peripheral_name [string toupper [common::get_property NAME $periph]]
        lappend peripherals $peripheral_name
    }

    # Get possible canonical names for all the peripherals connected to this
    # driver
    set device_id 0
    foreach periph $periphs {
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
    foreach periph $periphs {
        set periph_name [string toupper [common::get_property NAME $periph]]

        # Generate canonical definitions only for the peripherals whose
        # canonical name is not the same as hardware instance name
        if { [lsearch $canonicals $periph_name] < 0 } {
            puts $file_handle "/* Canonical definitions for peripheral $periph_name */"
            set canonical_name [format "%s_%s" $drv_string [lindex $indices $i]]

            foreach arg $args {
                set lvalue [::hsi::utils::get_driver_param_name $canonical_name $arg]

                # The commented out rvalue is the name of the instance-specific constant
                # set rvalue [::hsi::utils::get_ip_param_name $periph $arg]
                # The rvalue set below is the actual value of the parameter
                set rvalue [::hsi::utils::get_param_value $periph $arg]
                if {[llength $rvalue] == 0} {
                    set rvalue 0
                }
	    
		# For Vivado, C_BASEADDR is renamed to BASEADDR
	    	if { $rvalue == 0 && $arg == "C_BASEADDR" } {
		    set arg "BASEADDR"
                    set rvalue [::hsi::utils::get_param_value $periph $arg]
	        }	

	        # For Vivado, C_HIGHADDR is renamed to HIGHADDR
	        if { $rvalue == 0 && $arg == "C_HIGHADDR" } {
		    set arg "HIGHADDR"
                    set rvalue [::hsi::utils::get_param_value $periph $arg]
	        }	
	        if { $rvalue == 0 && $arg == "C_PCIEBAR_NUM" && [string match -nocase $ipname "axi_pcie3"]} {
		    set arg "PCIEBAR_NUM"
		    set rvalue [::hsi::utils::get_param_value $periph $arg]
		}

                set rvalue [::hsi::utils::format_addr_string $rvalue $arg]

                puts $file_handle "#define $lvalue $rvalue"

            }
            puts $file_handle ""
            incr i
        }
    }

    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}

 
