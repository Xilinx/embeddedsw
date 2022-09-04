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
    set xparam_gen 0
    foreach ip $ips {
	set periph [common::get_property IP_NAME $ip]
	set sw_processor [hsi::get_sw_processor]
	set processor [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_processor]]
	set processor_type [common::get_property IP_NAME $processor]

	if {[string compare -nocase $xparam_gen "1"] == 0} {
		break
	}

        if {[string compare -nocase "xdma" $periph] == 0 || [string compare -nocase "pcie_dma_versal" $periph] == 0 } {
		if {($processor_type == "psu_cortexr5")} {
			xdefine_pcie_include_file $drv_handle "xparameters.h" "XDmaPcie" \
			    "NUM_INSTANCES" \
			    "DEVICE_ID" \
			    "baseaddr" \
			    "C_INCLUDE_BAROFFSET_REG"\
			    "C_AXIBAR_NUM"\
			    "C_AXIBAR_HIGHADDR_0"\
			    "C_AXIBAR2PCIEBAR_0"\
			    "device_port_type"

			::hsi::utils::define_config_file $drv_handle "xdmapcie_g.c" "XDmaPcie" \
			     "DEVICE_ID" \
			     "baseaddr" \
			     "C_AXIBAR_NUM" \
			     "C_INCLUDE_BAROFFSET_REG"\
			     "device_port_type" \
			     "baseaddr" \
			     "C_AXIBAR2PCIEBAR_0"\
			     "C_AXIBAR_HIGHADDR_0"

			xdefine_pcie_canonical_xpars $drv_handle "xparameters.h" "XDmaPcie" \
			     "DEVICE_ID" \
			     "baseaddr" \
			     "C_INCLUDE_BAROFFSET_REG"\
			     "C_AXIBAR_NUM"\
			     "C_AXIBAR_HIGHADDR_0"\
			     "C_AXIBAR2PCIEBAR_0"\
			     "device_port_type"
	} else {
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
		set xparam_gen 1
	}

        if {[string compare -nocase "psv_pciea_attrib" $periph] == 0} {
		if {($processor_type == "psv_cortexr5")} {
			xdefine_pcie_include_file $drv_handle "xparameters.h" "XDmaPcie" \
			    "NUM_INSTANCES" \
			    "DEVICE_ID" \
			    "C_NOCPSPCIE0_REGION0" \
			    "C_CPM_PCIE0_AXIBAR_NUM"\
			    "C_CPM_PCIE0_PF0_AXIBAR2PCIE_HIGHADDR_0"\
			    "C_CPM_PCIE0_PF0_AXIBAR2PCIE_BASEADDR_0"\
			    "C_CPM_PCIE0_PORT_TYPE"

			::hsi::utils::define_config_file $drv_handle "xdmapcie_g.c" "XDmaPcie" \
			    "DEVICE_ID" \
			    "C_NOCPSPCIE0_REGION0" \
			    "C_CPM_PCIE0_AXIBAR_NUM" \
			    "C_CPM_PCIE0_PORT_TYPE" \
			    "C_NOCPSPCIE0_REGION0" \
			    "C_CPM_PCIE0_PF0_AXIBAR2PCIE_BASEADDR_0"\
			    "C_CPM_PCIE0_PF0_AXIBAR2PCIE_HIGHADDR_0"

			xdefine_pcie_canonical_xpars $drv_handle "xparameters.h" "XDmaPcie" \
			    "DEVICE_ID" \
			    "C_NOCPSPCIE0_REGION0" \
			    "C_CPM_PCIE0_AXIBAR_NUM"\
			    "C_CPM_PCIE0_PF0_AXIBAR2PCIE_HIGHADDR_0"\
			    "C_CPM_PCIE0_PF0_AXIBAR2PCIE_BASEADDR_0"\
			    "C_CPM_PCIE0_PORT_TYPE"
		} else {
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
		set xparam_gen 1
        }

        if {[string compare -nocase "qdma" $periph] == 0} {
		if {($processor_type == "psu_cortexr5") || ($processor_type == "psv_cortexr5")} {
			xdefine_pcie_include_file $drv_handle "xparameters.h" "XDmaPcie" \
			    "NUM_INSTANCES" \
			    "DEVICE_ID" \
			    "baseaddr" \
			    "C_INCLUDE_BAROFFSET_REG"\
			    "C_AXIBAR_NUM"\
			    "C_AXIBAR_HIGHADDR_0"\
			    "C_AXIBAR_0"\
			    "device_port_type"

			::hsi::utils::define_config_file $drv_handle "xdmapcie_g.c" "XDmaPcie" \
			    "DEVICE_ID" \
			    "baseaddr" \
			    "C_AXIBAR_NUM" \
			    "C_INCLUDE_BAROFFSET_REG"\
			    "device_port_type" \
			    "baseaddr" \
			    "C_AXIBAR_0"\
			    "C_AXIBAR_HIGHADDR_0"

			xdefine_pcie_canonical_xpars $drv_handle "xparameters.h" "XDmaPcie" \
			    "DEVICE_ID" \
			    "baseaddr" \
			    "C_INCLUDE_BAROFFSET_REG"\
			    "C_AXIBAR_NUM"\
			    "C_AXIBAR_HIGHADDR_0"\
			    "C_AXIBAR_0"\
			    "device_port_type"
		} else {
			xdefine_pcie_include_file $drv_handle "xparameters.h" "XDmaPcie" \
			    "NUM_INSTANCES" \
			    "DEVICE_ID" \
			    "baseaddr" \
			    "C_INCLUDE_BAROFFSET_REG"\
			    "C_AXIBAR_NUM"\
			    "C_AXIBAR_HIGHADDR_0"\
			    "C_AXIBAR_0"\
			    "C_AXIBAR_HIGHADDR_1"\
			    "C_AXIBAR_1"\
			    "device_port_type"

			::hsi::utils::define_config_file $drv_handle "xdmapcie_g.c" "XDmaPcie" \
			    "DEVICE_ID" \
			    "baseaddr" \
			    "C_AXIBAR_NUM" \
			    "C_INCLUDE_BAROFFSET_REG"\
			    "device_port_type" \
			    "baseaddr" \
			    "C_AXIBAR_0"\
			    "C_AXIBAR_1"\
			    "C_AXIBAR_HIGHADDR_0"\
			    "C_AXIBAR_HIGHADDR_1"

			xdefine_pcie_canonical_xpars $drv_handle "xparameters.h" "XDmaPcie" \
			    "DEVICE_ID" \
			    "baseaddr" \
			    "C_INCLUDE_BAROFFSET_REG"\
			    "C_AXIBAR_NUM"\
			    "C_AXIBAR_HIGHADDR_0"\
			    "C_AXIBAR_0"\
			    "C_AXIBAR_HIGHADDR_1"\
			    "C_AXIBAR_1"\
			    "device_port_type"
		}
		set xparam_gen 1
        }
        if {[string compare -nocase "psv_noc_pcie_0" $periph] == 0 ||
	    [string compare -nocase "psv_noc_pcie_1" $periph] == 0 ||
	    [string compare -nocase "psv_noc_pcie_2" $periph] == 0 } {
		::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XdmaPcie" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR"

		::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "XdmaPcie" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR"

		xdefine_config_file $drv_handle "xdmapcie_g.c" "XDmaPcie" "C_S_AXI_BASEADDR"
		set xparam_gen 1
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

proc get_parameter {periphs periph param} {
	set arg_name ""
	if {[lsearch $periphs [hsi::get_cells -hier $periph]] == -1} {
		set arg_name "0xFF"
	} else {
		set arg_name [::hsi::utils::get_ip_param_name [hsi::get_cells -hier $periph] $param]
	}
	regsub "S_AXI_" $arg_name "" arg_name
	return $arg_name
}

proc xdefine_config_file {drv_handle file_name drv_string args} {
        global periph_ninstances
        set args [::hsi::utils::get_exact_arg_list $args]
        set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

        set filename [file join "src" $file_name]
        set config_file [open $filename w]
        ::hsi::utils::write_c_header $config_file "Driver configuration"
        set num_insts [::hsi::utils::get_driver_param_name $drv_string "NUM_INSTANCES"]

	set sw_processor [hsi::get_sw_processor]
	set processor [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_processor]]
	set processor_type [common::get_property IP_NAME $processor]

        puts $config_file "\#include \"xparameters.h\""
        puts $config_file "\#include \"[string tolower $drv_string].h\""
        puts $config_file "\n/*"
        puts $config_file "* The configuration table for devices"
        puts $config_file "*/\n"
        puts $config_file [format "%s_Config %s_ConfigTable\[\] =" $drv_string $drv_string]
        puts $config_file "\{"

        set start_comma ""
        puts $config_file "\t\{"
        set comma ""
        set arg_name ""

	#Device ID
        set arg_name [get_parameter $periphs "*psv_noc_pcie_0" "DEVICE_ID"]
        puts -nonewline $config_file [format "\t\t%s" $arg_name]
        set comma ",\n"

	#Base Address

	#CPM REVISION PARAMETER is not present in older designs. Ensure design
	#backward compatibility by checking for the CPM IP string match
	set cpm_ip_str "versal_cips_0_cpm_0_psv_cpm"
	set cpm_ip_name [::hsi::get_cells -hier $cpm_ip_str]
	set cpm_rev "0"
	if {($cpm_ip_name == $cpm_ip_str)} {
	    set cpm_rev [::hsi::utils::get_param_value $cpm_ip_name "CPM_REVISION_NUMBER"]
	}
	#Revision is CPM5
	if {($cpm_rev == "1")} {
	    #Bridge base address for CPM5 QDMA defined seperately
	    set cpm5_qdma_bridge_off 0xe20000
	    set arg_val [::hsi::utils::get_param_value [hsi::get_cells -hier versal_cips_0_cpm_0_psv_cpm] "C_S_AXI_BASEADDR"]
	    set arg_val [expr {$arg_val + $cpm5_qdma_bridge_off}]
	    puts -nonewline $config_file [format "%s\t\t0x%x" $comma $arg_val]
	#Revision is CPM4
	} else {
	    if {($processor_type == "psv_cortexr5")} {
	        set arg_name [get_parameter $periphs "*psv_noc_pcie_0" "C_S_AXI_BASEADDR"]
		puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]
	    } else {
	        set arg_name [get_parameter $periphs "*psv_noc_pcie_1" "C_S_AXI_BASEADDR"]
		puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]
	    }
	}

	#Number of Bars
	puts -nonewline $config_file [format "%s\t\t%s" $comma "0x2"]

	#Port Type
	puts -nonewline $config_file [format "%s\t\t%s" $comma "0x1"]

	#ECAM
	if {($processor_type == "psv_cortexr5")} {
		set arg_name [get_parameter $periphs "*psv_noc_pcie_0" "C_S_AXI_BASEADDR"]
		puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]
	} else {
		set arg_name [get_parameter $periphs "*psv_noc_pcie_1" "C_S_AXI_BASEADDR"]
		puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]
	}

	#NP Mem Base
	set arg_name [get_parameter $periphs "*psv_noc_pcie_0" "C_S_AXI_BASEADDR"]
        puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]

	if {($processor_type == "psv_cortexa72")} {
		#P Mem Base
		set arg_name [get_parameter $periphs "*psv_noc_pcie_2" "C_S_AXI_BASEADDR"]
		puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]
	}

	#NP Mem high
        set arg_name [get_parameter $periphs "*psv_noc_pcie_0" "C_S_AXI_HIGHADDR"]
        puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]

	if {($processor_type == "psv_cortexa72")} {
		#P Mem high
		set arg_name [get_parameter $periphs "*psv_noc_pcie_2" "C_S_AXI_HIGHADDR"]
		puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]
	}

        puts $config_file "\n\t\}"

        puts $config_file "\};"
        puts $config_file "\n"
#        puts $config_file "size_t [format "%s_ConfigTableSize" $drv_string] = ARRAY_SIZE\([format "%s_ConfigTable" $drv_string]\);"
#        puts $config_file "\n"
        close $config_file
}
