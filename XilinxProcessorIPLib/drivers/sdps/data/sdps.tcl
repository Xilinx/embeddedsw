###############################################################################
# Copyright (C) 2013 - 2022 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00a hk   10/17/13 First release
# 2.0   adk  12/10/13 Updated as per the New Tcl API's
# 2.4	sk   12/04/14 Added CD and WP parameters
# 3.0   sk   07/16/16 Added BUS WIDTH, MIO BANK and HAS EMIO parameters.
# 3.3   mn   08/17/17 Enabled CCI support for A53 by adding cache coherency
#                     information.
# 3.6   mn   07/06/18 Generate canonical entry for IS_CACHE_COHERENT
# 3.8   mus  07/30/19 Added CCI support for Versal at EL1 NS
# 3.9   sd   03/20/20 Added clock support
# 3.9	sd   27/03/20 Added hier design fix
#
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XSdPs" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_SDIO_CLK_FREQ_HZ" "C_HAS_CD" "C_HAS_WP" "C_BUS_WIDTH" "C_MIO_BANK" "C_HAS_EMIO" "C_SLOT_TYPE" "C_CLK_50_SDR_ITAP_DLY" "C_CLK_50_SDR_OTAP_DLY" "C_CLK_50_DDR_ITAP_DLY" "C_CLK_50_DDR_OTAP_DLY" "C_CLK_100_SDR_OTAP_DLY" "C_CLK_200_SDR_OTAP_DLY"
	generate_cci_params $drv_handle "xparameters.h"

	set clocking [common::get_property CONFIG.clocking [hsi::get_os]]
	set is_zynqmp_fsbl_bsp [common::get_property CONFIG.ZYNQMP_FSBL_BSP [hsi::get_os]]
	set cortexa53proc [hsi::get_cells -hier -filter {IP_NAME=="psu_cortexa53"}]
	set isclocking [check_clocking]

		if { $isclocking == 1 && $is_zynqmp_fsbl_bsp != true   &&  [llength $cortexa53proc] > 0 && [string match -nocase $clocking "true"] > 0} {

    ::hsi::utils::define_zynq_config_file $drv_handle "xsdps_g.c" "XSdPs"  "DEVICE_ID" "C_S_AXI_BASEADDR" "C_SDIO_CLK_FREQ_HZ" "C_HAS_CD" "C_HAS_WP" "C_BUS_WIDTH" "C_MIO_BANK" "C_HAS_EMIO" "C_SLOT_TYPE" "IS_CACHE_COHERENT" "REF_CLK" "C_CLK_50_SDR_ITAP_DLY" "C_CLK_50_SDR_OTAP_DLY" "C_CLK_50_DDR_ITAP_DLY" "C_CLK_50_DDR_OTAP_DLY" "C_CLK_100_SDR_OTAP_DLY" "C_CLK_200_SDR_OTAP_DLY"
	} else {
    ::hsi::utils::define_zynq_config_file $drv_handle "xsdps_g.c" "XSdPs"  "DEVICE_ID" "C_S_AXI_BASEADDR" "C_SDIO_CLK_FREQ_HZ" "C_HAS_CD" "C_HAS_WP" "C_BUS_WIDTH" "C_MIO_BANK" "C_HAS_EMIO" "C_SLOT_TYPE" "IS_CACHE_COHERENT" "C_CLK_50_SDR_ITAP_DLY" "C_CLK_50_SDR_OTAP_DLY" "C_CLK_50_DDR_ITAP_DLY" "C_CLK_50_DDR_OTAP_DLY" "C_CLK_100_SDR_OTAP_DLY" "C_CLK_200_SDR_OTAP_DLY"
	}
    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "XSdPs" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_SDIO_CLK_FREQ_HZ" "C_HAS_CD" "C_HAS_WP" "C_BUS_WIDTH" "C_MIO_BANK" "C_HAS_EMIO" "C_SLOT_TYPE" "IS_CACHE_COHERENT" "C_CLK_50_SDR_ITAP_DLY" "C_CLK_50_SDR_OTAP_DLY" "C_CLK_50_DDR_ITAP_DLY" "C_CLK_50_DDR_OTAP_DLY" "C_CLK_100_SDR_OTAP_DLY" "C_CLK_200_SDR_OTAP_DLY"

}

proc check_clocking { } {
	set sw_proc_handle [hsi::get_sw_processor]
	set slaves [common::get_property   SLAVES [  hsi::get_cells -hier $sw_proc_handle]]
	foreach slave $slaves {
		if {[string compare -nocase "psu_crf_apb" $slave] == 0 } {
			return 1
		}
	}
	return 0
}

proc generate_cci_params {drv_handle file_name} {
	set file_handle [::hsi::utils::open_include_file $file_name]
	# Get all peripherals connected to this driver
	set ips [::hsi::utils::get_common_driver_ips $drv_handle]

	set isclocking [check_clocking]
	set sw_processor [hsi::get_sw_processor]
	set processor [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_processor]]
	set processor_type [common::get_property IP_NAME $processor]

	foreach ip $ips {
		set ref_tag 0xff
		set cci_enble 0
		if {$processor_type == "psu_cortexa53"} {
			set hypervisor [common::get_property CONFIG.hypervisor_guest [hsi::get_os]]
			if {[string match -nocase $hypervisor "true"]} {
				set cci_enble [common::get_property CONFIG.IS_CACHE_COHERENT $ip]
			}
			set ipname [common::get_property NAME $ip]
			set pos [string length $ipname]
			set num [ expr {$pos -1} ]
			set index [string index $ipname $num]
			set ref_tag [string toupper [format "SDIO%d_REF" $index ]]
		} elseif {$processor_type == "psv_cortexa72"} {
			set extra_flags [common::get_property CONFIG.extra_compiler_flags [hsi::get_sw_processor]]
			set flagindex [string first {-DARMA72_EL3} $extra_flags 0]
			if {$flagindex == -1} {
				set cci_enble [common::get_property CONFIG.IS_CACHE_COHERENT $ip]
			}
		}
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip "IS_CACHE_COHERENT"] $cci_enble"
		if { $isclocking == 1 } {
			puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip "REF_CLK"] $ref_tag"
		}
       }
       close $file_handle
}
