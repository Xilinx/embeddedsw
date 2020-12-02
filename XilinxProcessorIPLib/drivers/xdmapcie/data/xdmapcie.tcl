###############################################################################
# Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.0	tk	01/30/2019	First release
#
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    set ips [hsi::get_cells -hier "*"]
    foreach ip $ips {
	set periph [common::get_property IP_NAME $ip]
        if {[string compare -nocase "xdma" $periph] == 0 || [string compare -nocase "pcie_dma_versal" $periph] == 0 } {
            xdefine_pcie_include_file $drv_handle "xparameters.h" "XDmaPcie" \
                "NUM_INSTANCES" \
                "DEVICE_ID" \
                "baseaddr" \
                "C_INCLUDE_BAROFFSET_REG"\
                "C_AXIBAR_NUM"\
                "C_AXIBAR_HIGHADDR_0"\
                "C_AXIBAR2PCIEBAR_0"\
                "C_AXIBAR_HIGHADDR_1"\
                "C_AXIBAR2PCIEBAR_1"\
                "device_port_type"

                ::hsi::utils::define_config_file $drv_handle "xdmapcie_g.c" "XDmaPcie" \
                "DEVICE_ID" \
                "baseaddr" \
                "C_AXIBAR_NUM" \
                "C_INCLUDE_BAROFFSET_REG"\
                "device_port_type" \
                "baseaddr" \
                "C_AXIBAR2PCIEBAR_0"\
                "C_AXIBAR2PCIEBAR_1"\
                "C_AXIBAR_HIGHADDR_0"\
                "C_AXIBAR_HIGHADDR_1"

                xdefine_pcie_canonical_xpars $drv_handle "xparameters.h" "XDmaPcie" \
                "DEVICE_ID" \
                "baseaddr" \
                "C_INCLUDE_BAROFFSET_REG"\
                "C_AXIBAR_NUM"\
                "C_AXIBAR_HIGHADDR_0"\
                "C_AXIBAR2PCIEBAR_0"\
                "C_AXIBAR_HIGHADDR_1"\
                "C_AXIBAR2PCIEBAR_1"\
                "device_port_type"
        }

        if {[string compare -nocase "psv_pciea_attrib" $periph] == 0} {
            xdefine_pcie_include_file $drv_handle "xparameters.h" "XDmaPcie" \
                "NUM_INSTANCES" \
                "DEVICE_ID" \
                "C_NOCPSPCIE0_REGION0" \
                "C_CPM_PCIE0_AXIBAR_NUM"\
                "C_CPM_PCIE0_PF0_AXIBAR2PCIE_HIGHADDR_0"\
                "C_CPM_PCIE0_PF0_AXIBAR2PCIE_BASEADDR_0"\
                "C_CPM_PCIE0_PF0_AXIBAR2PCIE_HIGHADDR_1"\
                "C_CPM_PCIE0_PF0_AXIBAR2PCIE_BASEADDR_1"\
                "C_CPM_PCIE0_PORT_TYPE"

                ::hsi::utils::define_config_file $drv_handle "xdmapcie_g.c" "XDmaPcie" \
                "DEVICE_ID" \
                "C_NOCPSPCIE0_REGION0" \
                "C_CPM_PCIE0_AXIBAR_NUM" \
                "C_CPM_PCIE0_PORT_TYPE" \
                "C_NOCPSPCIE0_REGION0" \
                "C_CPM_PCIE0_PF0_AXIBAR2PCIE_BASEADDR_0"\
                "C_CPM_PCIE0_PF0_AXIBAR2PCIE_BASEADDR_1"\
                "C_CPM_PCIE0_PF0_AXIBAR2PCIE_HIGHADDR_0"\
                "C_CPM_PCIE0_PF0_AXIBAR2PCIE_HIGHADDR_1"

                xdefine_pcie_canonical_xpars $drv_handle "xparameters.h" "XDmaPcie" \
                "DEVICE_ID" \
                "C_NOCPSPCIE0_REGION0" \
                "C_CPM_PCIE0_AXIBAR_NUM"\
                "C_CPM_PCIE0_PF0_AXIBAR2PCIE_HIGHADDR_0"\
                "C_CPM_PCIE0_PF0_AXIBAR2PCIE_BASEADDR_0"\
                "C_CPM_PCIE0_PF0_AXIBAR2PCIE_HIGHADDR_1"\
                "C_CPM_PCIE0_PF0_AXIBAR2PCIE_BASEADDR_1"\
                "C_CPM_PCIE0_PORT_TYPE"
        }
    }
}

proc xdefine_pcie_include_file {drv_handle file_name drv_string args} {
    # Open include file
    set file_handle [::hsi::utils::open_include_file $file_name]

    # Get all peripherals connected to this driver
    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

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
            if {$arg == "device_port_type" } {
                if {[string compare -nocase "RootPortofPCIExpressRootComplex" $value] == 0} {
                    set value 1
                } else {
                    set value 0
                }
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

                if {$arg == "device_port_type" } {
                    if {[string compare -nocase "RootPortofPCIExpressRootComplex" $rvalue] == 0} {
                        set rvalue 1
                    } else {
                        set rvalue 0
                    }
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
