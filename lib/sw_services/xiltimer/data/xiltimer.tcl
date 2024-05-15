###############################################################################
# Copyright (C) 2021-2022 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.0   adk   24/11/21 First release
# 	adk   20/12/21 Fix TTC Device ID handling.
# 1.1	adk   08/08/22 Added support for versal net.
# 2.0   ml    08/04/24 Add support for sleep and tick timers
# 2.0   ml    15/05/24 Add support to use canonical definitions for
#                      sleep and interval timers
#
##############################################################################

set sleep_timer_is_ttc 0
set sleep_timer_is_axitimer 0
set sleep_timer_is_scutimer 0
set sleep_timer_is_default 0
set interval_timer_is_default 0

#---------------------------------------------
# timer_drc
#---------------------------------------------
proc timer_drc {lib_handle} {
        set intr_wrap [common::get_property CONFIG.xil_interrupt [hsi::get_os]]
        if { [string match -nocase $intr_wrap "false"] > 0} {
	        set enable_xil_interrupt [common::set_property CONFIG.xil_interrupt true [hsi::get_os]]
	}
}

proc xtimer_drc {lib_handle} {
	global sleep_timer_is_ttc
	global sleep_timer_is_axitimer
	global sleep_timer_is_scutimer
	global sleep_timer_is_default
	global interval_timer_is_default

	# check processor type
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]

	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];
	set default_dir "src/core/defalut/"
	set ttcps_dir "src/core/ttcps/"
	set axitmr_dir "src/core/axi_timer/"
	set scutimer_dir "src/core/scutimer/"


	if {$sleep_timer_is_ttc != 0} {
		foreach entry [glob -nocomplain -types f [file join $ttcps_dir *]] {
			file copy -force $entry "./src"
		}
	}
	if {$sleep_timer_is_axitimer != 0} {
		foreach entry [glob -nocomplain -types f [file join $axitmr_dir *]] {
			file copy -force $entry "./src"
		}
	}
	if {$sleep_timer_is_scutimer != 0} {
		foreach entry [glob -nocomplain -types f [file join $scutimer_dir *]] {
			file copy -force $entry "./src"
		}
	}
	if {$sleep_timer_is_default != 0 || $interval_timer_is_default != 0} {
		if {$proc_type == "psu_cortexa53" || $proc_type == "psv_cortexa72" || $proc_type == "psxl_cortexa78" || $proc_type == "psx_cortexa78"} {
			file copy -force "src/core/default_timer/globaltimer_sleep.c" "./src"
		}
		if {$proc_type == "psu_cortexr5" || $proc_type == "psv_cortexr5" || $proc_type == "psxl_cortexr52" || $proc_type == "psx_cortexr52"} {
			file copy -force "src/core/default_timer/cortexr5_sleep.c" "./src"
		}
		if {$proc_type == "microblaze" || $proc_type == "psu_pmu"
			|| $proc_type == "psv_pmc" || $proc_type == "psv_psm"} {
			file copy -force "src/core/default_timer/microblaze_sleep.c" "./src"
		}
		if {$proc_type == "ps7_cortexa9"} {
			file copy -force "src/core/default_timer/globaltimer_sleep_zynq.c" "./src"
		}
	}
	file delete -force ./src/core
}

proc generate {lib_handle} {
	global sleep_timer_is_ttc
	global sleep_timer_is_axitimer
	global sleep_timer_is_scutimer
	global sleep_timer_is_default
	global interval_timer_is_default

        set sleep_timer_is_ttc 0
        set sleep_timer_is_axitimer 0
        set sleep_timer_is_scutimer 0
	set sleep_timer_is_default 0
	set interval_timer_is_default 0

	# check processor type
	set proc_instance [hsi::get_sw_processor]
	set hw_proc_handle [::hsi::get_cells -hier [common::get_property hw_instance $proc_instance]]
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]]
	set os [hsi::get_os]
	set xiltimer_interval_timer [common::get_property CONFIG.xiltimer_interval_timer $lib_handle]
	set xiltimer_sleep_timer [common::get_property CONFIG.xiltimer_sleep_timer $lib_handle]
	# for interval functionality interrupt connection is manadatory
	set ttc_ips [::hsi::get_mem_ranges -of_objects $hw_proc_handle [hsi::get_cells -hier -filter {IP_NAME == "psv_ttc"  || IP_NAME == "psu_ttc" || IP_NAME == "ps7_ttc" || IP_NAME == "psxl_ttc" || IP_NAME == "psx_ttc"}]]
	set axitmr_ips [hsi::get_cells -hier -filter {IP_NAME == "axi_timer"}]
	set scutmr_ips [hsi::get_cells -hier -filter {IP_NAME == "ps7_scutimer"}]
	set sorted_ttc_ips [lsort $ttc_ips]
	if { $os == "standalone" && $xiltimer_sleep_timer == "none" && ($proc_type == "psv_cortexr5" || $proc_type == "psu_cortexr5")} {
		set xiltimer_sleep_timer [lindex $sorted_ttc_ips end]
	}
	if {$proc_type == "ps7_cortexa9"} {
		set timer_ips [concat $scutmr_ips $sorted_ttc_ips $axitmr_ips]
	} elseif {$proc_type == "microblaze"} {
		set timer_ips [concat $axitmr_ips $scutmr_ips $sorted_ttc_ips]
	} else {
		set timer_ips [concat $sorted_ttc_ips $axitmr_ips $scutmr_ips]
	}
        set timer_len [llength $timer_ips]
	set is_zynqmp_fsbl_bsp [common::get_property CONFIG.ZYNQMP_FSBL_BSP [hsi::get_os]]
	set cortexa53proc [hsi::get_cells -hier -filter {IP_NAME=="psu_cortexa53_0"}]
	set is_intervaltimer_en [common::get_property CONFIG.en_interval_timer $lib_handle]

	if {$proc_instance == "psv_pmc_0" || $proc_instance == "psu_pmu_0" || $proc_instance == "psv_psm_0"} {
		incr sleep_timer_is_default
		incr interval_timer_is_default
	} elseif {$proc_instance == "psu_cortexa53_0" && $is_zynqmp_fsbl_bsp == true} {
		incr sleep_timer_is_default
		incr interval_timer_is_default
	} elseif { [expr [llength $timer_ips] != 0] } {
		if { $xiltimer_sleep_timer == "none"} {
			incr sleep_timer_is_default
		}
		if { $os == "standalone" && $xiltimer_interval_timer == "none" } {
                        incr interval_timer_is_default
		}
	} else {
		incr sleep_timer_is_default
		incr interval_timer_is_default
		puts "No timer IP's available in the design"
	}

	set file_handle [::hsi::utils::open_include_file "xparameters.h"]
	puts $file_handle "\#define XPAR_XILTIMER_ENABLED"
	close $file_handle

	set tmrcfg_file "src/xtimer_config.h"
	set fd [open $tmrcfg_file w]
	puts $fd "\#ifndef _XTIMER_CONFIG_H"
	puts $fd "\#define _XTIMER_CONFIG_H"
	puts $fd ""
	puts $fd "#include \"xparameters.h\""
	puts $fd ""
	puts $fd "\//NOTE: Canonical definitions are used for below peripherals"
	if {$sleep_timer_is_default != 0} {
		puts $fd "\ /* Sleep Timer */"
                puts $fd "\#define XTIMER_IS_DEFAULT_TIMER"
        } else {
		if {[llength $ttc_ips] != 0} {
			set instnum [lsearch -exact $sorted_ttc_ips $xiltimer_sleep_timer]
			if {$instnum >= 0} {
				puts $fd "\#define XSLEEPTIMER_IS_TTCPS"
				set device_id [expr {$instnum * 3}]
				puts $fd "\#define XSLEEPTIMER_DEVICEID XPAR_XTTCPS_${device_id}_DEVICE_ID"
				incr sleep_timer_is_ttc
			}
		}
		if {[llength $axitmr_ips] != 0} {
			set instnum [lsearch -exact $axitmr_ips $xiltimer_sleep_timer]
			if {$instnum >= 0} {
				puts $fd "\#define XSLEEPTIMER_IS_AXITIMER"
				puts $fd "\#define XSLEEPTIMER_DEVICEID XPAR_TMRCTR_${instnum}_DEVICE_ID"
				incr sleep_timer_is_axitimer
			}
		}
		if {[llength $scutmr_ips] != 0} {
			set instnum [lsearch -exact $scutmr_ips $xiltimer_sleep_timer]
			if {$instnum >= 0} {
				puts $fd "\#define XSLEEPTIMER_IS_SCUTIMER"
				puts $fd "\#define XSLEEPTIMER_DEVICEID XPAR_XSCUTIMER_${instnum}_DEVICE_ID"
				incr sleep_timer_is_scutimer
			}
		}
	}
        if {$interval_timer_is_default != 0} {
                puts $fd "\#define XTIMER_NO_TICK_TIMER"
        } else {
		if { $xiltimer_interval_timer == "none" } {
			set xiltimer_interval_timer [lindex $timer_ips 0]
			common::set_property CONFIG.xiltimer_interval_timer $xiltimer_interval_timer $lib_handle
		}
		if {[llength $ttc_ips] != 0} {
			set instnum [lsearch -exact $sorted_ttc_ips $xiltimer_interval_timer]
			if {$instnum >= 0} {
				puts $fd "\#define XTICKTIMER_IS_TTCPS"
				set device_id [expr {$instnum * 3}]
				puts $fd "\#define XTICKTIMER_DEVICEID XPAR_XTTCPS_${device_id}_DEVICE_ID"
				incr sleep_timer_is_ttc
			}
		}
		if {[llength $axitmr_ips] != 0} {
			set instnum [lsearch -exact $axitmr_ips $xiltimer_interval_timer]
			if {$instnum >= 0} {
				puts $fd "\#define XTICKTIMER_IS_AXITIMER"
				puts $fd "\#define XTICKTIMER_DEVICEID XPAR_TMRCTR_${instnum}_DEVICE_ID"
				incr sleep_timer_is_axitimer
			}
		}
		if {[llength $scutmr_ips] != 0} {
			set instnum [lsearch -exact $scutmr_ips $xiltimer_interval_timer]
			if {$instnum >= 0} {
				set lib [hsi::get_libs xiltimer]
				common::set_property CONFIG.xiltimer_interval_timer ps7_scutimer_0 $lib
				puts $fd "\#define XTICKTIMER_IS_SCUTIMER"
				puts $fd "\#define XTICKTIMER_DEVICEID XPAR_XSCUTIMER_${instnum}_DEVICE_ID"
				incr sleep_timer_is_scutimer
			}
		}
	}
	puts $fd ""
	puts $fd "\#endif /* XTIMER_CONFIG_H */"
	close $fd

        if { $xiltimer_sleep_timer != "none" && $xiltimer_interval_timer != "none"} {
                if {$xiltimer_sleep_timer == $xiltimer_interval_timer} {
                    error "ERROR: For sleep and interval functionality same timer instance got selected, please select different timers" "mdt_error"
                }
        }

	xtimer_drc $lib_handle
}
