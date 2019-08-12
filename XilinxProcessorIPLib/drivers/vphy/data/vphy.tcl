##******************************************************************************
##
## Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
##
## Permission is hereby granted, free of charge, to any person obtaining a copy
## of this software and associated documentation files (the "Software"), to deal
## in the Software without restriction, including without limitation the rights
## to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
## copies of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
##
## The above copyright notice and this permission notice shall be included in
## all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
## IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
##
#
##
###############################################################################
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ----------------------------------------------------
# 1.0      als    10/19/15 Initial release.
# 1.4      gm     29/11/16 Added Transceiver_Width, C_Err_Irq_En,
#                            AXI_LITE_FREQ_HZ Parameters
#                          Fixed c++ compiler warnings/errors
#                            Added xdefine_config_file for _g.c generation
###############################################################################

proc generate {drv_handle} {
    xdefine_include_file $drv_handle "xparameters.h" "XVPHY" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "Transceiver" "C_Tx_No_Of_Channels" "C_Rx_No_Of_Channels" "C_Tx_Protocol" "C_Rx_Protocol" "C_TX_REFCLK_SEL" "C_RX_REFCLK_SEL" "C_TX_PLL_SELECTION" "C_RX_PLL_SELECTION" "C_NIDRU" "C_NIDRU_REFCLK_SEL" "C_INPUT_PIXELS_PER_CLOCK" "Tx_Buffer_Bypass" "C_Hdmi_Fast_Switch" "Transceiver_Width" "C_Err_Irq_En" "AXI_LITE_FREQ_HZ" "DRPCLK_FREQ" "C_Use_GT_CH4_HDMI"
    xdefine_config_file $drv_handle "xvphy_g.c" "XVphy" "DEVICE_ID" "C_BASEADDR" "TRANSCEIVER" "C_Tx_No_Of_Channels" "C_Rx_No_Of_Channels" "C_Tx_Protocol" "C_Rx_Protocol" "C_TX_REFCLK_SEL" "C_RX_REFCLK_SEL" "C_TX_PLL_SELECTION" "C_RX_PLL_SELECTION" "C_NIDRU" "C_NIDRU_REFCLK_SEL" "C_INPUT_PIXELS_PER_CLOCK" "Tx_Buffer_Bypass" "C_Hdmi_Fast_Switch" "Transceiver_Width" "C_Err_Irq_En" "AXI_LITE_FREQ_HZ" "DRPCLK_FREQ" "C_Use_GT_CH4_HDMI"
    xdefine_canonical_xpars $drv_handle "xparameters.h" "VPHY" "DEVICE_ID" "C_BASEADDR" "Transceiver" "C_Tx_No_Of_Channels" "C_Rx_No_Of_Channels" "C_Tx_Protocol" "C_Rx_Protocol" "C_TX_REFCLK_SEL" "C_RX_REFCLK_SEL" "C_TX_PLL_SELECTION" "C_RX_PLL_SELECTION" "C_NIDRU" "C_NIDRU_REFCLK_SEL" "C_INPUT_PIXELS_PER_CLOCK" "Tx_Buffer_Bypass" "C_Hdmi_Fast_Switch" "Transceiver_Width" "C_Err_Irq_En" "AXI_LITE_FREQ_HZ" "DRPCLK_FREQ" "C_Use_GT_CH4_HDMI"
}

# -----------------------------------------------------------------------------
# Create configuration C file as required by Xilinx drivers
# Use the config field list technique.
# -----------------------------------------------------------------------------
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
	puts $config_file [format "%s_Config %s_ConfigTable\[%s\] =" $drv_string $drv_string $num_insts]
	puts $config_file "\{"

	set start_comma ""
	foreach periph $periphs {

		puts $config_file [format "%s\t\{" $start_comma]
		set comma ""
		foreach arg $args {
            if {[string compare -nocase "TRANSCEIVER" $arg] == 0} {
                puts -nonewline $config_file [format "%s\t\t%s%s" $comma "(XVphy_GtType)" [::hsi::utils::get_ip_param_name $periph $arg]]
            } elseif {[string compare -nocase "C_Tx_Protocol" $arg] == 0} {
                puts -nonewline $config_file [format "%s\t\t%s%s" $comma "(XVphy_ProtocolType)" [::hsi::utils::get_ip_param_name $periph $arg]]
            } elseif {[string compare -nocase "C_Rx_Protocol" $arg] == 0} {
                puts -nonewline $config_file [format "%s\t\t%s%s" $comma "(XVphy_ProtocolType)" [::hsi::utils::get_ip_param_name $periph $arg]]
            } elseif {[string compare -nocase "C_TX_PLL_SELECTION" $arg] == 0} {
                puts -nonewline $config_file [format "%s\t\t%s%s" $comma "(XVphy_SysClkDataSelType)" [::hsi::utils::get_ip_param_name $periph $arg]]
            } elseif {[string compare -nocase "C_RX_PLL_SELECTION" $arg] == 0} {
                puts -nonewline $config_file [format "%s\t\t%s%s" $comma "(XVphy_SysClkDataSelType)" [::hsi::utils::get_ip_param_name $periph $arg]]
            } elseif {[string compare -nocase "C_TX_REFCLK_SEL" $arg] == 0} {
                puts -nonewline $config_file [format "%s\t\t%s%s" $comma "(XVphy_PllRefClkSelType)" [::hsi::utils::get_ip_param_name $periph $arg]]
            } elseif {[string compare -nocase "C_RX_REFCLK_SEL" $arg] == 0} {
                puts -nonewline $config_file [format "%s\t\t%s%s" $comma "(XVphy_PllRefClkSelType)" [::hsi::utils::get_ip_param_name $periph $arg]]
            } elseif {[string compare -nocase "C_NIDRU_REFCLK_SEL" $arg] == 0} {
                puts -nonewline $config_file [format "%s\t\t%s%s" $comma "(XVphy_PllRefClkSelType)" [::hsi::utils::get_ip_param_name $periph $arg]]
            } elseif {[string compare -nocase "C_INPUT_PIXELS_PER_CLOCK" $arg] == 0} {
                puts -nonewline $config_file [format "%s\t\t%s%s" $comma "(XVidC_PixelsPerClock)" [::hsi::utils::get_ip_param_name $periph $arg]]
            } else {
				puts -nonewline $config_file [format "%s\t\t%s" $comma [::hsi::utils::get_ip_param_name $periph $arg]]
			}
			set comma ",\n"
		}

		puts -nonewline $config_file "\n\t\}"
		set start_comma ",\n"
	}
	puts $config_file "\n\};\n"
	close $config_file
}

#
# Given a list of arguments, define them all in an include file.
# Handles IP model/user parameters, as well as the special parameters NUM_INSTANCES,
# DEVICE_ID
# Will not work for a processor.
#
proc xdefine_include_file {drv_handle file_name drv_string args} {
    set args [::hsi::utils::get_exact_arg_list $args]
    # Open include file
    set file_handle [::hsi::utils::open_include_file $file_name]

    # Get all peripherals connected to this driver
    set periphs [::hsi::utils::get_common_driver_ips $drv_handle]

    # Handle special cases
    set arg "NUM_INSTANCES"
    set posn [lsearch -exact $args $arg]
    if {$posn > -1} {
        puts $file_handle "/* Definitions for driver [string toupper [common::get_property name $drv_handle]] */"
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
            puts $file_handle "#define [::hsi::utils::get_driver_param_name $drv_string $arg] [common::get_property $arg $drv_handle]"
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
            } elseif {[string compare -nocase "AXI_LITE_FREQ_HZ" $arg] == 0} {
                set freq [::hsi::utils::get_clk_pin_freq  $periph "vid_phy_axi4lite_aclk"]
                if {[llength $freq] == 0} {
                    set freq "100000000"
                    puts "WARNING: AXIlite clock frequency information is not available in the design, \
                          for peripheral $periph_name. Assuming a default frequency of 100MHz. \
                          If this is incorrect, the peripheral $periph_name will be non-functional"
                }
                set value $freq
            } elseif {[string compare -nocase "DRPCLK_FREQ" $arg] == 0} {
                 set xcvr "Transceiver"
                 set xcvr [common::get_property CONFIG.$xcvr $periph]
                 if {[string compare -nocase "GTXE2" "$xcvr"] == 0} {
                             set freq [::hsi::utils::get_clk_pin_freq  $periph "vid_phy_axi4lite_aclk"]
                 } elseif {[string compare -nocase "GTHE2" "$xcvr"] == 0} {
                             set freq [::hsi::utils::get_clk_pin_freq  $periph "vid_phy_axi4lite_aclk"]
                 } elseif {[string compare -nocase "GTPE2" "$xcvr"] == 0} {
                             set freq [::hsi::utils::get_clk_pin_freq  $periph "vid_phy_axi4lite_aclk"]
                 } elseif {[string compare -nocase "GTHE3" "$xcvr"] == 0} {
                             set freq [::hsi::utils::get_clk_pin_freq  $periph "drpclk"]
                 } elseif {[string compare -nocase "GTHE4" "$xcvr"] == 0} {
                             set freq [::hsi::utils::get_clk_pin_freq  $periph "drpclk"]
                 } elseif {[string compare -nocase "GTYE4" "$xcvr"] == 0} {
                             set freq [::hsi::utils::get_clk_pin_freq  $periph "drpclk"]
                 } else {
                     puts $file_handle "#error \"Video PHY currently supports only GTYE4, GTHE4, GTHE3, GTHE2, GTPE2 and GTXE2; $xcvr not supported\""
                 }
                if {[llength $freq] == 0} {
                    set freq "100000000"
                    puts "WARNING: DRPCLK clock frequency information is not available in the design, \
                          for peripheral $periph_name. Assuming a default frequency of 100MHz. \
                          If this is incorrect, the peripheral $periph_name will be non-functional"
                }
                set value $freq
            } else {
                set value [common::get_property CONFIG.$arg $periph]
            }
            if {[llength $value] == 0} {
                set value 0
            }
            set value [::hsi::utils::format_addr_string $value $arg]
            if {[string compare -nocase "HW_VER" $arg] == 0} {
                puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg] \"$value\""
            } elseif {[string compare -nocase "TRANSCEIVER" $arg] == 0} {
                    puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg]_STR \"$value\""
                    if {[string compare -nocase "GTXE2" "$value"] == 0} {
                        puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg] 1"
                    } elseif {[string compare -nocase "GTHE2" "$value"] == 0} {
                                puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg] 2"
                    } elseif {[string compare -nocase "GTPE2" "$value"] == 0} {
                                puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg] 3"
                    } elseif {[string compare -nocase "GTHE3" "$value"] == 0} {
                                puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg] 4"
                    } elseif {[string compare -nocase "GTHE4" "$value"] == 0} {
                                puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg] 5"
                    } elseif {[string compare -nocase "GTYE4" "$value"] == 0} {
                                puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg] 6"
                    } else {
                        puts $file_handle "#error \"Video PHY currently supports only GTYE4, GTHE4, GTHE3, GTHE2, GTPE2 and GTXE2; $value not supported\""
                    }
            } else {
                puts $file_handle "#define [::hsi::utils::get_ip_param_name $periph $arg] $value"
            }
        }
        puts $file_handle ""
    }
    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}

#
# define_canonical_xpars - Used to print out canonical defines for a driver.
# Given a list of arguments, define each as a canonical constant name, using
# the driver name, in an include file.
#
proc xdefine_canonical_xpars {drv_handle file_name drv_string args} {
    set args [::hsi::utils::get_exact_arg_list $args]
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
               set rvalue [::hsi::utils::format_addr_string $rvalue $arg]

            if {[string compare -nocase "TRANSCEIVER" $arg] == 0} {
                   puts $file_handle "#define [string toupper $lvalue]_STR \"$rvalue\""
                   if {[string compare -nocase "GTXE2" "$rvalue"] == 0} {
                       puts $file_handle "#define [string toupper $lvalue] 1"
                   } elseif {[string compare -nocase "GTHE2" "$rvalue"] == 0} {
                               puts $file_handle "#define [string toupper $lvalue] 2"
                   } elseif {[string compare -nocase "GTPE2" "$rvalue"] == 0} {
                               puts $file_handle "#define [string toupper $lvalue] 3"
                   } elseif {[string compare -nocase "GTHE3" "$rvalue"] == 0} {
                               puts $file_handle "#define [string toupper $lvalue] 4"
                   } elseif {[string compare -nocase "GTHE4" "$rvalue"] == 0} {
                               puts $file_handle "#define [string toupper $lvalue] 5"
                   } elseif {[string compare -nocase "GTYE4" "$rvalue"] == 0} {
                               puts $file_handle "#define [string toupper $lvalue] 6"
                   } else {
                       puts $file_handle "#error \"Video PHY currently supports only GTYE4, GTHE4, GTHE3, GTHE2, GTEP2 and GTXE2; $rvalue not supported\""
                   }
            } elseif {[string compare -nocase "AXI_LITE_FREQ_HZ" $arg] == 0} {
                set rfreq [::hsi::utils::get_clk_pin_freq  $periph "vid_phy_axi4lite_aclk"]
                if {[llength $rfreq] == 0} {
                    set rfreq "100000000"
                    puts "WARNING: Clock frequency information is not available in the design, \
                          for peripheral $periph_name. Assuming a default frequency of 100MHz. \
                          If this is incorrect, the peripheral $periph_name will be non-functional"
                }
                set rvalue $rfreq
                puts $file_handle "#define [string toupper $lvalue] $rvalue"
            } elseif {[string compare -nocase "DRPCLK_FREQ" $arg] == 0} {
                 set rxcvr "Transceiver"
                 set rxcvr [common::get_property CONFIG.$rxcvr $periph]
                 if {[string compare -nocase "GTXE2" "$rxcvr"] == 0} {
                             set rfreq [::hsi::utils::get_clk_pin_freq  $periph "vid_phy_axi4lite_aclk"]
                 } elseif {[string compare -nocase "GTHE2" "$rxcvr"] == 0} {
                             set rfreq [::hsi::utils::get_clk_pin_freq  $periph "vid_phy_axi4lite_aclk"]
                 } elseif {[string compare -nocase "GTPE2" "$rxcvr"] == 0} {
                             set rfreq [::hsi::utils::get_clk_pin_freq  $periph "vid_phy_axi4lite_aclk"]
                 } elseif {[string compare -nocase "GTHE3" "$rxcvr"] == 0} {
                             set rfreq [::hsi::utils::get_clk_pin_freq  $periph "drpclk"]
                 } elseif {[string compare -nocase "GTHE4" "$rxcvr"] == 0} {
                             set rfreq [::hsi::utils::get_clk_pin_freq  $periph "drpclk"]
                 } elseif {[string compare -nocase "GTYE4" "$rxcvr"] == 0} {
                             set rfreq [::hsi::utils::get_clk_pin_freq  $periph "drpclk"]
                 } else {
                     puts $file_handle "#error \"Video PHY currently supports only GTYE4, GTHE4, GTHE3, GTHE2, GTPE2 and GTXE2; $rxcvr not supported\""
                 }
                if {[llength $rfreq] == 0} {
                    set rfreq "100000000"
                    puts "WARNING: DRPCLK clock frequency information is not available in the design, \
                          for peripheral $periph_name. Assuming a default frequency of 100MHz. \
                          If this is incorrect, the peripheral $periph_name will be non-functional"
                }
                set rvalue $rfreq
                puts $file_handle "#define [string toupper $lvalue] $rvalue"
            } else {
               puts $file_handle "#define [string toupper $lvalue] $rvalue"
            }

           }
           puts $file_handle ""
           incr i
       }
   }

   puts $file_handle "\n/******************************************************************/\n"
   close $file_handle
}
