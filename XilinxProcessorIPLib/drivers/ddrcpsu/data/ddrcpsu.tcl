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
 # THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 # IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 # FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 # XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 # WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 # OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 # SOFTWARE.
 #
 # Except as contained in this notice, the name of the Xilinx shall not be used
 # in advertising or otherwise to promote the sale, use or other dealings in
 # this Software without prior written authorization from Xilinx.
 #
 #
 # MODIFICATION HISTORY:
 # Ver      Who    Date     Changes
 # -------- ------ -------- ------------------------------------
 # 1.0	    ssc   04/28/16 First Release.
 #
 ################################################################################

proc generate {drv_handle} {
    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XDdrcPsu" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_HAS_ECC" "C_DDRC_CLK_FREQ_HZ"

    get_ddr_config_info $drv_handle "xparameters.h"

    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "DdrcPsu" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_DDRC_CLK_FREQ_HZ"

    get_ddr_config_info_canonical $drv_handle "xparameters.h"

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
