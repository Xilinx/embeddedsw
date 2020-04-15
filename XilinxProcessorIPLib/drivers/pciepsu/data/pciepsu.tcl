###############################################################################
# Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 0.1	bs	08/21/2018	First release
#
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XPciePsu" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR"
    xdefine_pciemode $drv_handle "xparameters.h" "psu_pcie" "C_PCIE_MODE"

    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "XPciePsu" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_PCIE_MODE"

    xdefine_config_file $drv_handle "xpciepsu_g.c" "XPciePsu" "C_S_AXI_BASEADDR"

}
proc xdefine_pciemode {drv_handle file_name periph param} {
        set file_handle [::hsi::utils::open_include_file $file_name]
        set arg_name [::hsi::utils::get_param_value [hsi::get_cells -hier $periph] $param]
        if {[string compare -nocase "Endpoint Device" $arg_name] == 0} {
            set arg_name "0x0"
        } else {
            set arg_name "0x1"
        }
        puts $file_handle "#define [::hsi::utils::get_ip_param_name [hsi::get_cells -hier $periph] $param] $arg_name"
        puts $file_handle ""
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

        set arg_name [get_parameter $periphs "psu_pcie" "DEVICE_ID"]
        puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]
        set comma ",\n"

        set arg_name [get_parameter $periphs "psu_pcie" "C_S_AXI_BASEADDR"]
        puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]

        set arg_name [get_parameter $periphs "psu_pcie_attrib_0" "C_S_AXI_BASEADDR"]
        puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]

        set arg_name [get_parameter $periphs "psu_pcie_high2" "C_S_AXI_BASEADDR"]
        puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]

        set arg_name [get_parameter $periphs "psu_pcie_low" "C_S_AXI_BASEADDR"]
        puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]

        set arg_name [get_parameter $periphs "psu_pcie_high1" "C_S_AXI_BASEADDR"]
        puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]

        set arg_name [get_parameter $periphs "psu_pcie_low" "C_S_AXI_HIGHADDR"]
        puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]

        set arg_name [get_parameter $periphs "psu_pcie_high1" "C_S_AXI_HIGHADDR"]
        puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]

        set arg_name [get_parameter $periphs "psu_pcie_dma" "C_S_AXI_BASEADDR"]
        puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]

        set arg_name [get_parameter $periphs "psu_pcie" "C_PCIE_MODE"]
        puts -nonewline $config_file [format "%s\t\t%s" $comma $arg_name]

        puts $config_file "\n\t\}"

        puts $config_file "\};"
        puts $config_file "\n"
        puts $config_file "size_t [format "%s_ConfigTableSize" $drv_string] = ARRAY_SIZE\([format "%s_ConfigTable" $drv_string]\);"
        puts $config_file "\n"
        close $config_file
}
