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
	close $file_handle
}
