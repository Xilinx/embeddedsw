###############################################################################
#
# Copyright (C) 2011 - 2014 Xilinx, Inc.  All rights reserved.
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
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF 
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Except as contained in this notice, the name of the Xilinx shall not be used
# in advertising or otherwise to promote the sale, use or other dealings in
# this Software without prior written authorization from Xilinx.
#
###############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00a sdm  11/22/11 Created
# 2.0   adk  10/12/13 Updated as per the New Tcl API's
# 2.1   adk  11/08/14 Fixed the CR#811288 when PCS/PMA core is present in the hw
#		      XPAR_GIGE_PCS_PMA_CORE_PRESENT and phy address values 
#		      should export to the xparameters.h file.
# 2.1   bss  09/08/14 Fixed CR#820349 to export phy address in xparameters.h
#		      when GMII to RGMII converter is present in hw.
#
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    xdefine_zynq_include_file $drv_handle "xparameters.h" "XEmacPs" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_ENET_CLK_FREQ_HZ" "C_ENET_SLCR_1000Mbps_DIV0" "C_ENET_SLCR_1000Mbps_DIV1" "C_ENET_SLCR_100Mbps_DIV0" "C_ENET_SLCR_100Mbps_DIV1" "C_ENET_SLCR_10Mbps_DIV0" "C_ENET_SLCR_10Mbps_DIV1"

    xdefine_zynq_config_file $drv_handle "xemacps_g.c" "XEmacPs"  "DEVICE_ID" "C_S_AXI_BASEADDR" 

    xdefine_zynq_canonical_xpars $drv_handle "xparameters.h" "XEmacPs" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_ENET_CLK_FREQ_HZ" "C_ENET_SLCR_1000Mbps_DIV0" "C_ENET_SLCR_1000Mbps_DIV1" "C_ENET_SLCR_100Mbps_DIV0" "C_ENET_SLCR_100Mbps_DIV1" "C_ENET_SLCR_10Mbps_DIV0" "C_ENET_SLCR_10Mbps_DIV1"
    
    generate_gmii2rgmii_params $drv_handle "xparameters.h"
    
    generate_sgmii_params $drv_handle "xparameters.h"

}

proc generate_gmii2rgmii_params {drv_handle file_name} {
	set file_handle [::hsm::utils::open_include_file $file_name]
	set ips [get_cells  "*"]
	foreach ip $ips {
		set ipname [get_property NAME  $ip]
		set periph [get_property IP_NAME  $ip]
		if { [string compare -nocase $periph "ps7_ethernet"] == 0} {
			set phya [is_gmii2rgmii_conv_present $ip]
			if { $phya == 0} {
				close $file_handle
				return 0
			}
			puts $file_handle "/* Definition for the MDIO address for the GMII2RGMII converter PL IP*/"
				
			if { [string compare -nocase $ipname "ps7_ethernet_0"] == 0} {
				puts $file_handle "\#define XPAR_GMII2RGMIICON_0N_ETH0_ADDR $phya"
			}
			if { [string compare -nocase $ipname "ps7_ethernet_1"] == 0} {
				puts $file_handle "\#define XPAR_GMII2RGMIICON_0N_ETH1_ADDR $phya"
			}
			puts $file_handle "\n/******************************************************************/\n"
		}

	}
	close $file_handle
}
	
proc is_gmii2rgmii_conv_present {slave} {
	set port_value 0
	set phy_addr 0
	set ipconv 0
	set ips [get_cells "*"]
	set enetipinstance_name [get_property NAME  $slave]
	
	foreach ip $ips {
		set convipname [get_property NAME  $ip]
		set periph [get_property IP_NAME $ip]
		if { [string compare -nocase $periph "gmii_to_rgmii"] == 0} {
			set ipconv $ip
			break
		}
	}
	if { $ipconv != 0 }  {
		set port_value [get_pins -of_objects [get_nets -of_objects [get_pins -of_objects $ipconv gmii_txd]]]
		if { $port_value != 0 } {
			set tmp [string first "ENET0" $port_value]
			if { $tmp >= 0 } {
				if { [string compare -nocase $enetipinstance_name "ps7_ethernet_0"] == 0} {
					set phyaddr [::hsm::utils::get_param_value $ipconv C_PHYADDR]
					set phy_addr [::hsm::utils::convert_binary_to_decimal $phyaddr] 
					if {[llength $phy_addr] == 0} {
						set phy_addr 0
					}
				}
			} else {
				set tmp0 [string first "ENET1" $port_value]
				if { $tmp0 >= 0 } {
					if { [string compare -nocase $enetipinstance_name "ps7_ethernet_1"] == 0} {
						set phyaddr [::hsm::utils::get_param_value $ipconv C_PHYADDR]
						set phy_addr [::hsm::utils::convert_binary_to_decimal $phyaddr] 
						if {[llength $phy_addr] == 0} {
							set phy_addr 0
						}
					}
				}
			}
		}
	}
	return $phy_addr
}

proc generate_sgmii_params {drv_handle file_name} {
	set file_handle [::hsm::utils::open_include_file $file_name]
	set proc_handle [get_sw_processor]
	set hwproc_handle [get_cells [get_property HW_INSTANCE $proc_handle]]
	set mhs_handle [get_cells]
		set ips [get_cells "*"]
		foreach ip $ips {
			set ipname [get_property NAME $ip]
			set periph [get_property IP_NAME  $ip]
			if { [string compare -nocase $periph "ps7_ethernet"] == 0} { 
				set phya [is_gige_pcs_pma_ip_present $ip]
				if { $phya == 0} {
					close $file_handle
					return 0
				}
				puts $file_handle "/* Definitions related to PCS PMA PL IP*/"
				puts $file_handle "\#define XPAR_GIGE_PCS_PMA_CORE_PRESENT 1"
				puts $file_handle "\#define XPAR_PCSPMA_SGMII_PHYADDR $phya"
				puts $file_handle "\n/******************************************************************/\n"
			}
			
		}
  close $file_handle
}
	
proc is_gige_pcs_pma_ip_present {slave} {
	set port_value 0
	set phy_addr 0
	set ipconv 0

	#set mhs_handle [get_cells -of_objects  $slave]
	set ips [get_cells "*"]
	set enetipinstance_name [get_property NAME  $slave]
	
	foreach ip $ips {
		set convipname [get_property NAME  $ip]
		set periph [get_property IP_NAME $ip]
		if { [string compare -nocase $periph "gig_ethernet_pcs_pma"] == 0} {
			set sgmii_param [get_property CONFIG.c_is_sgmii $ip]
			if {$sgmii_param == true} {
				set ipconv $ip
			} 
			break
		}
	}
	if { $ipconv != 0 }  {
		set port_value [::hsm::utils::get_net_name $ipconv "gmii_txd"]
		if { $port_value != 0 } {
			set tmp [string first "ENET0" $port_value]
			if { $tmp >= 0 } {
				if { [string compare -nocase $enetipinstance_name "ps7_ethernet_0"] == 0} {
					set phyaddr [::hsm::utils::get_param_value $ipconv C_PHYADDR]
					set phy_addr [::hsm::utils::convert_binary_to_decimal $phyaddr] 
					if {[llength $phy_addr] == 0} {
						set phy_addr 0
					}
				}
			} else {
				set tmp0 [string first "ENET1" $port_value]
				if { $tmp0 >= 0 } {
					if { [string compare -nocase $enetipinstance_name "ps7_ethernet_1"] == 0} {
						set phyaddr [::hsm::utils::get_param_value $ipconv C_PHYADDR]
						set phy_addr [::hsm::utils::convert_binary_to_decimal $phyaddr] 
						if {[llength $phy_addr] == 0} {
							set phy_addr 0
						}
					}
				}
			}
		}
	}
	return $phy_addr
}
