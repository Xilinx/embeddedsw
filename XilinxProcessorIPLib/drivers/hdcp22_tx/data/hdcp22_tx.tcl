################################################################################
#
# Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and#or sell
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
################################################################################

proc generate {drv_handle} {
  ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XHdcp22_Tx" \
	"NUM_INSTANCES" \
	"DEVICE_ID" \
	"C_BASEADDR" \
	"C_HIGHADDR" \
	"C_PROTOCOL" \
	"C_MODE"

  hier_ip_define_config_file $drv_handle "xhdcp22_tx_g.c" "XHdcp22_Tx" \
	"DEVICE_ID" \
	"C_BASEADDR" \
	"C_PROTOCOL" \
	"C_MODE"

  ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "XHdcp22_Tx" \
	"NUM_INSTANCES" \
	"DEVICE_ID" \
	"C_BASEADDR" \
	"C_HIGHADDR"
}

proc hier_ip_define_config_file {drv_handle file_name drv_string args} {
  set args [::hsi::utils::get_exact_arg_list $args]
  set hw_instance_name [::common::get_property HW_INSTANCE $drv_handle];

  set filename [file join "src" $file_name]
  set config_file [open $filename w]

  ::hsi::utils::write_c_header $config_file "Driver configuration"

  # Generate list of relavent parent cells
  set parent_cells [::hsi::get_cells -filter {IP_NAME==hdcp22_tx}]

  # Create a dict of relavent parents(key) and children(value)
  foreach parent_cell $parent_cells {
    puts "parent_cell: ${parent_cell}"
    # Generate list of all relavent child cells
    #set child_cells [::hsi::get_cells -hier -filter {HIER_NAME =~ "${parent_cell}"}]
    set cells [::hsi::get_cells -hier]

    set child_cells {}

    foreach cell $cells {
      if { [string match "${parent_cell}/*" [get_property HIER_NAME $cell]] && [string match "PERIPHERAL" [get_property IP_TYPE $cell]] } {
        puts "child_cell: ${cell}"
        lappend child_cells $cell
      }
    }

    dict set cell_dict $parent_cell $child_cells

    puts $cell_dict
  }

  puts $config_file "#include \"xparameters.h\""
  puts $config_file "#include \"[string tolower $drv_string].h\""
  puts $config_file "\n/*"
  puts $config_file "* The configuration table for devices"
  puts $config_file "*/\n"
  puts $config_file [format "%s_Config %s_ConfigTable\[\] =" $drv_string $drv_string]
  puts $config_file "\{"

  set periphs [::hsi::utils::get_common_driver_ips $drv_handle]
  set start_comma ""

  # Loop through each hdcp22_tx subsystem
  foreach periph $periphs {
    puts $config_file [format "%s\t\{" $start_comma]
    set comma ""

    # Loop through each parameter argument and update the config structure
    foreach arg $args {
      if {[string compare -nocase "DEVICE_ID" $arg] == 0} {
        puts -nonewline $config_file [format "%s\t\t%s,\n" $comma [::hsi::utils::get_ip_param_name $periph $arg]]
        continue
      }

      # Check if this is a driver parameter or a peripheral parameter
      set value [common::get_property CONFIG.$arg $drv_handle]
      if {[llength $value] == 0} {
        set local_value [common::get_property CONFIG.$arg $periph ]
        # If a parameter isn't found locally (in the current
        # peripheral), we will (for some obscure and ancient reason)
        # look in peripherals connected via point to point links
        if { [string compare -nocase $local_value ""] == 0} {
            set p2p_name [::hsi::utils::get_p2p_name $periph $arg]
            if { [string compare -nocase $p2p_name ""] == 0} {
                puts -nonewline $config_file [format "%s\t\t%s" $comma [::hsi::utils::get_ip_param_name $periph $arg]]
            } else {
                puts -nonewline $config_file [format "%s\t\t%s" $comma $p2p_name]
            }
        } else {
            puts -nonewline $config_file [format "%s\t\t%s" $comma [::hsi::utils::get_ip_param_name $periph $arg]]
        }
      } else {
        puts -nonewline $config_file [format "%s\t\t%s" $comma [::hsi::utils::get_driver_param_name $drv_string $arg]]
      }
      set comma ",\n"
    }

    puts -nonewline $config_file ",\n"

    # Loop through subcores inside subsystem and update config structure
    set comma ""
    foreach cell [dict get $cell_dict $periph] {
      set child_cell_deviceid "XPAR_${cell}_DEVICE_ID"
      puts -nonewline $config_file [format "%s\t\t%s" $comma [string toupper ${child_cell_deviceid}]]
      set comma ",\n"
    }

    ::hsi::current_hw_instance

    puts -nonewline $config_file "\n\t\}"
    set start_comma ",\n"
  }

  puts $config_file "\n\};"
  puts $config_file "\n";
  close $config_file
}
