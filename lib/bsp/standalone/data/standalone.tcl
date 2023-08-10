##############################################################################
# Copyright (c) 2014 - 2022 Xilinx, Inc.  All rights reserved.
# Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#
# MODIFICATION HISTORY:
#
# Ver   Who  Date     Changes
# ----- ---- -------- ---------------------------------------------------
# 6.4   ms   05/23/17 Defined PSU_PMU macro in xparameters.h to support
#                     XGetPSVersion_Info function for PMUFW.
# 6.6   srm  10/18/17 Added xsleep_timer_config function to support the
#                     sleep configuration using timers as specified by the
#					  user.
# 6.6   hk   12/15/17 Define platform macros based on the processor in use.
# 6.6   mus  01/29/18 Updated to add xen PV console support in Cortexa53 64
#                     bit BSP.
# 6.6   mus  02/02/18 Updated get_connected_if proc to detect the HPC port
#                     configured with smart interconnect.
# 6.6   mus  02/19/18 Updated handle_profile_opbtimer proc to cover the
#                     scenario, where AXI timer is connected to the INTC
#                     through concat IP.
# 6.6   mus  02/23/18 Export macro for the debug logic configuration in
# 		      Cortex R5 BSP, macro value is based on the
#		      mld parameter "lockstep_mode_debug".
# 6.8   mus  04/27/18 Updated tcl to export definition for
#                     FPU_HARD_FLOAT_ABI_ENABLED flag to bspconfig.h,based
#                     on -mfpu-abi option in extra compiler flags.
# 6.8   mus  09/10/18 Updated tcl to add -hier option while using
#                     get_cells command.
# 7.1   mus  03/27/19 Added procs to check if specific address space is
#                     accessible to the cortexr5 processor CR#1015725
# 7.1   mus  05/20/19 Updated outbyte/inbyte in case stdout/stdin is set as
#                     "none". This is done to fix warnings CR#1031423
# 7.2   mus  10/11/19 Updated logic to export LOCKSTEP_MODE_DEBUG for Versal
#                     as well. Fix for CR#1046243.
# 7.2   mus  11/14/19 Remove logic for deletion of source directories, to
#                     avoid race condition in tcl. Deletion of source
#                     directories would happen through makefiles. It fixes
#                      CR#1038151.
# 7.2   mus  09/01/20 Updated to add armclang compiler support for Cortexa72.
#                     It fixes CR#1051552
# 7.2   mus  01/29/20 Updated xsleep_timer_config proc to use TTC2 for sleep
#                     routines, if TTC3 is not present in HW design. If TTC2
#                     also not present, then CortexR5 PMU cycle counter would
#                     be used in sleep routines. If user dont want to use PMU
#                     cycle counter, -DDONT_USE_PMU_FOR_SLEEP_ROUTINES flag
#                     needs to be added in BSP compiler flags.
#
# 7.2   ma   02/10/20 Add VERSAL_PLM macro in xparameters.h file for psv_pmc
#                     processor. Also make outbyte function weak for PLM so
#                     that PLM specific outbyte function can be called
#                     instead of this.
# 7.2   mus  02/23/20 Added workaround to handle_stdout_parameter to fix PLM
#                     BSP creation CR#1055177
# 7.2   sd   03/20/20 Added clocking support
# 7.2   sd   03/27/20 Fix the hierarchcal design case
# 7.3   kal  07/06/20 Export XPAR_PSU_PSS_REF_CLK_FREQ_HZ macro in
#                     xparameters.h file for psv_pmc and psu_pmc processors.
# 7.3   mus  07/02/20 Fix xsleep_timer_config proc for CortexR5 BSP.
#                     is_ttc_accessible_from_processor returns 1 if TTC is
#                     accessible to the processor. Updated conditions in
#                     xsleep_timer_config proc accordingly. It fixes
#                     CR#1069210
# 7.4   mus  12/14/20 Updated generate proc to support CIPS3.
# 7.6   mus  06/25/21 Updated tcl logic to access base address/high address
#                     of specific IP block. This change has been done to
#                     support SSIT devices.
# 7.7   adk  14/12/21 Updated xsleep_timer_config proc to use TTC3 for sleep
#                     routines if present in the HW design.
# 8.0   mus  22/02/22 Added support for VERSAL NET
# 8.0	sk   03/17/22 Update microblaze_interrupts_g.h to fix misra_c_2012_
# 		      directive_4_10 violation.
# 8.0	sk   03/17/22 Update parameter function declaration parameter name in
# 		      microblaze_interrupts_g.c to fix misra_c_2012_rule_8_6
# 		      violation.
#       adk  05/18/22 Added pmu_sleep_timer config parameter to perform sleep
#       	      functionality from PMU counters for CortexR5 BSP.
#       bm   07/06/22 Added logic to include files from versal_net directory
#       adk  09/08/22 When xiltimer is enabled don't pull xpm_counter.c file.
# 8.1   sd   21/11/22 Export XPAR_PSU_PSS_REF_CLK_FREQ_HZ macro in
#                     xparameters.h file for microblaze processors.
# 8.2   adk  13/03/23 Don't delete the xpm_counter.c file when xiltimer library
#       	      is enabled, as xpm_counter.c file contains PM event API's
#       	      which are generic.
# 9.0   sa   05/01/23 Added support for Microblaze RISC-V.
# 9.0   dp   29/03/23 Added support to use ttc as sleeptimer for VersalNet
#                     Cortex-R52
#       mus  20/04/23 MPU related files for CortexR5 and CortexR52 have been
#                     separated out to make code readable. Update tcl file
#                     copy appropriate files based on processor and SoC.
# 9.1   ml   10/08/23 Updated tcl not to set coresight as stdout/stdin for
#                     non ARM based processors.
##############################################################################

# ----------------------------------------------------------------------------
# The following are hardcoded for Zynq.
# We can obtain the scu timer/gic baseaddr from the xml, but other parameters
# need to be hardcoded. hardcode everything..
# ----------------------------------------------------------------------------
#TODO these hardcoding parameters can be removed. It can directly come from PS7 IP
set scutimer_baseaddr	0xF8F00600
set scutimer_intr	29
set scugic_cpu_base	0xF8F00100
set scugic_dist_base	0xF8F01000
set is_xiltimer_enabled 0

# --------------------------------------
# Tcl procedure standalone_drc
# -------------------------------------
proc standalone_drc {os_handle} {
        global is_xiltimer_enabled
	if {[lsearch -nocase [hsi::get_libs] "xiltimer"] >= 0} {
		set is_xiltimer_enabled 1
	} else {
		set is_xiltimer_enabled 0
	}
}

# -------------------------------------------------------------------------
# Tcl procedure get_base_value
# Returns base address of provided IP instance
# -------------------------------------------------------------------------
proc get_base_value {ip_instance} {
		set val 0

		set list [get_mem_ranges -of_objects [hsi::get_cells -hier [get_sw_processor]]]
		set index [lsearch $list $ip_instance]
		if {$index >= 0} {
			set val [common::get_property BASE_VALUE [lindex [get_mem_ranges -of_objects [hsi::get_cells -hier [get_sw_processor]]] $index]]
		}
		return $val
}

# -------------------------------------------------------------------------
# Tcl procedure lpd_is_coherent
# Returns true(1) if any one of LPD masters has CCI enabled, else false(0)
# -------------------------------------------------------------------------
proc lpd_is_coherent {} {
	#List of all LPD masters that can have cache coherency enabled
	set lpd_master_names {psu_adma psu_qspi psu_nand psu_sd psu_ethernet psu_cortexr5 psu_usb}
	foreach master_name $lpd_master_names {
		# Get all the enabled instances of each IP
		set filter_txt [list IP_NAME == $master_name]
		set mlist [hsi::get_cells -hier -filter $filter_txt]
		# Iterate through each instance and check for CONFIG.IS_CACHE_COHERENT
		foreach master $mlist {
			if { [common::get_property CONFIG.IS_CACHE_COHERENT $master] == "1" } {
				# We found a master that's cache coherent, so return true
				return 1
			}
		}
	}
	# None of the masters were cache coherent, so return false
	return 0
}

# -------------------------------------------------------------------------
# Tcl procedure fpd_is_coherent
# Returns true(1) if any one of the FPD masters has CCI enabled, else false(0)
# -------------------------------------------------------------------------
proc fpd_is_coherent {} {
    #List of all FPD masters that can have cache coherency enabled
    set fpd_master_names {psu_sata psu_pcie}
    foreach master_name $fpd_master_names {
        # Get all the enabled instances of each IP
        set filter_txt [list IP_NAME == $master_name]
        set mlist [hsi::get_cells -hier -filter $filter_txt]
        # Iterate through each instance and check for CONFIG.IS_CACHE_COHERENT
        foreach master $mlist {
            if { [common::get_property CONFIG.IS_CACHE_COHERENT $master] == "1" } {
                # We found a FPD master that is cache coherent, so return true
                return 1
            }
        }
    }
    # None of the masters were cache coherent, so return false
    return 0
}

#---------------------------------------------------------------------
# Tcl procedure is_pl_coherent
# Returns true(1) if HPC0 or HPC1 port is enabled in the design and got
# connected to a DMA capable peripheral
#----------------------------------------------------------------------
proc is_pl_coherent {} {
     set periphs [hsi::get_cells -hier]
     foreach periph $periphs {
	set ipname [common::get_property IP_NAME $periph]
	if {$ipname == "zynq_ultra_ps_e"} {
		set is_hpcdesign [get_connected_if $periph "S_AXI_HPC0_FPD"]
	        if {$is_hpcdesign} {
			return $is_hpcdesign
		}
		set is_hpcdesign [get_connected_if $periph "S_AXI_HPC1_FPD"]
	        if {$is_hpcdesign} {
			return $is_hpcdesign
		}
	}
     }

     return 0
}

#---------------------------------------------------------------------
# Tcl procedure is get_processor_access
# Returns processor access info. Each bit of return value signifies
# processor access to specific address space/slave.
# If specific bit is 0 that means address space
# corresponding to that bit position is not accessible from processor,
# else processor has privilege to access the same. As of now only 2 bits
# are being used others are kept as reserved.
# 0th bit - RPU address space
# 1st bit - IOU SLCR address space
# Note: This proc is applicable only for cortexr5 processor
#----------------------------------------------------------------------
proc get_processor_access {} {
	set sw_proc_handle [hsi::get_sw_processor]
	set hw_proc_handle [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc_handle] ]
	set r5_access 0
	set cnt 0
	set cortexa72proc [hsi::get_cells -hier -filter {IP_NAME=="psu_cortexa72" || IP_NAME=="psv_cortexa72"}]
	set cortexa78proc [hsi::get_cells -hier -filter {IP_NAME=="psxl_cortexa78" || IP_NAME=="psx_cortexa78"}]
	set rpu_instance [get_mem_ranges -of_objects [get_cells -hier $sw_proc_handle] -filter { INSTANCE == "psu_rpu" || INSTANCE == "psv_rpu"}]
	set slcr_instance [get_mem_ranges -of_objects [get_cells -hier $sw_proc_handle] -filter { INSTANCE == "psu_iouslcr_0" }]

	set r5_tz [common::get_property CONFIG.C_TZ_NONSECURE $hw_proc_handle]
	if {$r5_tz == "" || $r5_tz == "0"} {
		set r5_access 0xff
	} else {
		if {[llength $rpu_instance] > 0} {
			set rpu_tz [string toupper [get_property TRUSTZONE [get_mem_ranges \
                            -of_objects [get_cells -hier $sw_proc_handle] *rpu*]]]
			if {([string compare -nocase $rpu_tz "NONSECURE"] == 0)} {
				set r5_access [expr $r5_access + pow(2,$cnt)]
			}
		}
		incr cnt

		if {[llength $cortexa72proc] == 0 && [llength $slcr_instance] > 0} {
			set iou_slcr_tz [string toupper [get_property TRUSTZONE [get_mem_ranges \
                         -of_objects [get_cells -hier $sw_proc_handle] psu_iouslcr_0]]]
			if {([string compare -nocase $iou_slcr_tz "NONSECURE"] == 0)} {
				set r5_access [expr $r5_access + pow(2,$cnt)]
			}
		}
	}
	return [expr round($r5_access)]

}

#---------------------------------------------------------------------
# Tcl procedure is_ttc_accessible_from_processor
# Returns true(1) if specific ttc instance is accessible from processor
#----------------------------------------------------------------------
proc is_ttc_accessible_from_processor {ttc_instance} {
	set sw_proc_handle [hsi::get_sw_processor]
	set hw_proc_handle [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc_handle] ]
	set ttc_instance [get_mem_ranges -of_objects [get_cells -hier $sw_proc_handle] -filter { INSTANCE == "$ttc_instance" }]

	set r5_tz [common::get_property CONFIG.C_TZ_NONSECURE $hw_proc_handle]
	if {$r5_tz == "" || $r5_tz == "0"} {
		return 1
	} else {
		if {[llength $ttc_instance] > 0} {
			set ttc_tz [string toupper [get_property TRUSTZONE [get_mem_ranges \
                        -of_objects [get_cells -hier $sw_proc_handle] $ttc_instance]]]
			if {([string compare -nocase $ttc_tz "NONSECURE"] == 0)} {
			return 1
			} else {
				return 0
			}
		} else {
			return 0
		}
	}

}

proc get_connected_if {drv_handle hpc_pin} {
	set iphandle [::hsi::utils::get_connected_stream_ip $drv_handle $hpc_pin]
        if { $iphandle == "" } {
		return 0
	} else {
		set ipname [get_property IP_NAME $iphandle]
		if {$ipname == "axi_interconnect" || $ipname == "smartconnect"} {
		     set iphandle [::hsi::utils::get_connected_stream_ip $iphandle S00_AXI]
		     if { $iphandle == ""} {
			 return 0
		     }
		     set iptype [get_property IP_TYPE $iphandle]
		     if {$iptype == "PERIPHERAL"} {
			return 1
		     }
		}
	}
	return 0
}

# --------------------------------------
# Tcl procedure generate
# -------------------------------------
proc generate {os_handle} {
    global env

    global is_xiltimer_enabled
    set need_config_file "false"
    # Copy over the right set of files as src based on processor type
    set sw_proc_handle [hsi::get_sw_processor]
    set hw_proc_handle [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc_handle] ]
    set proctype [common::get_property IP_NAME $hw_proc_handle]
    set procname [common::get_property NAME    $hw_proc_handle]
    set boardname [common::get_property BOARD [hsi::current_hw_design]]
    set enable_sw_profile [common::get_property CONFIG.enable_sw_intrusive_profiling $os_handle]
    set mb_exceptions false

    # proctype should be "microblaze" or psu_cortexa53 or psu_cortexr5 or ps7_cortexa9 or "microblaze_riscv"
    set mbsrcdir "./src/microblaze"
    set cortexa53srcdir "./src/arm/cortexa53"
    set cortexr5srcdir "./src/arm/cortexr5"
    set cortexa9srcdir "./src/arm/cortexa9"
    set procdrv [hsi::get_sw_processor]
    set commonsrcdir "./src/common"
    set armcommonsrcdir "./src/arm/common"
    set armsrcdir "./src/arm"
    set clksrcdir "./src/common/clocking"
    set intrsrcdir "./src/common/intr"
    set versalsrcdir "./src/common/versal"
    set versalnetsrcdir "./src/common/versal_net"
    set riscvsrcdir "./src/riscv"

    set is_versal [hsi::get_cells -hier -filter {IP_NAME=="psu_cortexa72" || IP_NAME=="psv_cortexa72"}]
    set is_versal_net [hsi::get_cells -hier -filter {IP_NAME=="psx_cortexr52" || IP_NAME=="psxl_cortexr52" || IP_NAME=="psx_cortexa78" || IP_NAME=="psxl_cortexa78"}]

    foreach entry [glob -nocomplain [file join $commonsrcdir *]] {
        file copy -force $entry "./src"
    }
    foreach entry [glob -nocomplain [file join $clksrcdir *]] {
        file copy -force $entry "./src"
    }
    foreach entry [glob -nocomplain [file join $intrsrcdir *]] {
        file copy -force $entry "./src"
    }

    if { $proctype == "psu_cortexa53" || $proctype == "psu_cortexa72" || $proctype == "ps7_cortexa9" || $proctype == "psu_cortexr5" || $proctype == "psv_cortexr5" || $proctype == "psv_cortexa72" || $proctype == "psxl_cortexa78" || $proctype == "psxl_cortexr52" || $proctype == "psx_cortexa78" || $proctype == "psx_cortexr52"} {
        set compiler [common::get_property CONFIG.compiler $procdrv]
        foreach entry [glob -nocomplain -types f [file join $armcommonsrcdir *]] {
            file copy -force $entry "./src"
        }
        if {[string compare -nocase $compiler "armcc"] != 0 && [string compare -nocase $compiler "iccarm"] != 0
	    &&  [string compare -nocase $compiler "armclang"] != 0} {
            set commonccdir "./src/arm/common/gcc"
            foreach entry [glob -nocomplain [file join $commonccdir *]] {
	         file copy -force $entry "./src/"
            }
        } elseif {[string compare -nocase $compiler "iccarm"] == 0} {
            set commonccdir "./src/arm/common/iccarm"
            foreach entry [glob -nocomplain [file join $commonccdir *]] {
                 file copy -force $entry "./src/"
            }
        } elseif {[string compare -nocase $compiler "armclang"] == 0} {
            set commonccdir "./src/arm/common/armclang"
            foreach entry [glob -nocomplain [file join $commonccdir *]] {
                 file copy -force $entry "./src/"
            }
        }

    }

    set cortexa72proc [hsi::get_cells -hier -filter {IP_NAME=="psu_cortexa72" || IP_NAME=="psv_cortexa72"}]
    set cortexa78proc [hsi::get_cells -hier -filter {IP_NAME=="psxl_cortexa78" || IP_NAME=="psx_cortexa78"}]
    set cortexa53proc [hsi::get_cells -hier -filter {IP_NAME=="psu_cortexa53"}]


    # Only processor specific file should be copied to specified standalone folder
    # write a API which needs compiler,
    switch $proctype {
        "microblaze" {
            foreach entry [glob -nocomplain [file join $mbsrcdir *]] {
                # Copy over only files that are not related to exception handling. All such files have exception in their names
                file copy -force $entry "./src/"
            }
            set need_config_file "true"
            set mb_exceptions [mb_has_exceptions $hw_proc_handle]
            if {[llength $cortexa53proc] > 0} {
               set pss_ref_clk_mhz [common::get_property CONFIG.C_PSS_REF_CLK_FREQ $hw_proc_handle]
               if { $pss_ref_clk_mhz == "" } {
                    puts "WARNING: CONFIG.C_PSS_REF_CLK_FREQ not found. Using default value for XPAR_PSU_PSS_REF_CLK_FREQ_HZ."
                    set pss_ref_clk_mhz 33333000
               }
               set file_handle [::hsi::utils::open_include_file "xparameters.h"]
               puts $file_handle " /* Definition for PSS REF CLK FREQUENCY */"
               puts $file_handle [format %s%.0f%s "#define XPAR_PSU_PSS_REF_CLK_FREQ_HZ " [expr $pss_ref_clk_mhz]  "U"]
               puts $file_handle ""
               close $file_handle
           }
        }
	"psu_pmc" -
	"psv_pmc" -
	"psxl_pmc" -
	"psx_pmc"
	{
	    foreach entry [glob -nocomplain [file join $mbsrcdir *]] {
                # Copy over only files that are not related to exception handling. All such files have exception in their names
                file copy -force $entry "./src/"
            }
            set need_config_file "true"
            set mb_exceptions [mb_has_exceptions $hw_proc_handle]
            set pss_ref_clk_mhz [common::get_property CONFIG.C_PSS_REF_CLK_FREQ $hw_proc_handle]
            if { $pss_ref_clk_mhz == "" } {
                puts "WARNING: CONFIG.C_PSS_REF_CLK_FREQ not found. Using default value for XPAR_PSU_PSS_REF_CLK_FREQ_HZ."
                set pss_ref_clk_mhz 33333000
             }
            set file_handle [::hsi::utils::open_include_file "xparameters.h"]
            puts $file_handle " /* Definition for PSS REF CLK FREQUENCY */"
            puts $file_handle [format %s%.0f%s "#define XPAR_PSU_PSS_REF_CLK_FREQ_HZ " [expr $pss_ref_clk_mhz]  "U"]
            puts $file_handle ""
            close $file_handle
        }
	"psu_psm" -
	"psv_psm" -
	"psxl_psm" -
	"psx_psm"
	{
	    foreach entry [glob -nocomplain [file join $mbsrcdir *]] {
                # Copy over only files that are not related to exception handling. All such files have exception in their names
                file copy -force $entry "./src/"
            }
            set need_config_file "true"
            set mb_exceptions [mb_has_exceptions $hw_proc_handle]
        }
        "psu_pmu" {
	    foreach entry [glob -nocomplain [file join $mbsrcdir *]] {
                # Copy over only files that are not related to exception handling. All such files have exception in their names
                file copy -force $entry "./src/"
            }
            set need_config_file "true"
            set mb_exceptions [mb_has_exceptions $hw_proc_handle]
            set pss_ref_clk_mhz [get_psu_config "PSU__PSS_REF_CLK__FREQMHZ"]

            if { $pss_ref_clk_mhz == "" } {
                puts "WARNING: CONFIG.PSU__PSS_REF_CLK__FREQMHZ not found. Using default value for XPAR_PSU_PSS_REF_CLK_FREQ_HZ."
                set pss_ref_clk_mhz 4
            }
            set file_handle [::hsi::utils::open_include_file "xparameters.h"]
            puts $file_handle ""
            puts $file_handle [format %s%.0f%s "#define XPAR_PSU_PSS_REF_CLK_FREQ_HZ " [expr $pss_ref_clk_mhz*1e6]  "U"]
            puts $file_handle ""
	    puts $file_handle "#define PSU_PMU 1U"
            # Define XPS_BOARD_* : For use in PMUFW to perform board specific configs
            if { [string length $boardname] != 0 } {
                set fields [split $boardname ":"]
                lassign $fields prefix board suffix
                if { [string length $board] != 0 } {
                    set def "#define XPS_BOARD_"
                    append def [string toupper $board]
                    puts $file_handle $def
                    puts $file_handle ""
                }
            }

            if {[lpd_is_coherent]} {
                set def "#define XPAR_LPD_IS_CACHE_COHERENT"
                puts $file_handle $def
                puts $file_handle ""
            }

            if {[fpd_is_coherent]} {
                set def "#define XPAR_FPD_IS_CACHE_COHERENT"
                puts $file_handle $def
                puts $file_handle ""
            }

	    if {[is_pl_coherent]} {
                set def "#define XPAR_PL_IS_CACHE_COHERENT"
                puts $file_handle $def
                puts $file_handle ""
	    }
            xdefine_fabric_reset $file_handle
            close $file_handle

        }
        "psu_cortexa53" -
	"psv_cortexa72" -
	"psu_cortexa72" -
	"psxl_cortexa78" -
	"psx_cortexa78"
	{
            set procdrv [hsi::get_sw_processor]
            set compiler [get_property CONFIG.compiler $procdrv]
            if {[string compare -nocase $compiler "arm-none-eabi-gcc"] == 0} {
		set ccdir "./src/arm/ARMv8/32bit/gcc"
		set cortexa53srcdir1 "./src/arm/ARMv8/32bit"
		set platformsrcdir "./src/arm/ARMv8/32bit/platform/ZynqMP"
	    } else {
	      if {[string compare -nocase $compiler "armclang"] == 0} {
                       set ccdir "./src/arm/ARMv8/64bit/armclang"
               } else {
                       set ccdir "./src/arm/ARMv8/64bit/gcc"
               }

	        set cortexa53srcdir1 "./src/arm/ARMv8/64bit"
		if { $proctype == "psu_cortexa53" }  {
		    file copy -force [file join $cortexa53srcdir1 platform ZynqMP xparameters_ps.h] ./src
		    if {[string compare -nocase $compiler "armclang"] == 0} {
		        set platformsrcdir "./src/arm/ARMv8/64bit/platform/ZynqMP/armclang"
		    } else {
			set platformsrcdir "./src/arm/ARMv8/64bit/platform/ZynqMP/gcc"
		    }
		 } else {
		if { $proctype == "psv_cortexa72" } {
                   file copy -force [file join $cortexa53srcdir1 platform versal xparameters_ps.h] ./src
	        } else {
                   file copy -force [file join "./src/arm" platform versal_net xparameters_ps.h] ./src
	        }
             if {[string compare -nocase $compiler "armclang"] == 0} {
                 set platformsrcdir "./src/arm/ARMv8/64bit/platform/versal/armclang"
             } else {
                 set platformsrcdir "./src/arm/ARMv8/64bit/platform/versal/gcc"
             }
		 }
	        set pvconsoledir "./src/arm/ARMv8/64bit/xpvxenconsole"
	        set hypervisor_guest [common::get_property CONFIG.hypervisor_guest $os_handle ]
	        if { $hypervisor_guest == "true" } {
	             foreach entry [glob -nocomplain [file join $pvconsoledir *]] {
			file copy -force $entry "./src/"
		     }
		}
	    }

	    set includedir "./src/arm/ARMv8/includes_ps/"
	    file copy -force $includedir "./src/"
	    if {[llength $cortexa72proc] > 0} {
	        set platformincludedir "./src/arm/ARMv8/includes_ps/platform/Versal"
	    } else {
	        set platformincludedir "./src/arm/ARMv8/includes_ps/platform/ZynqMP"
	    }

            foreach entry [glob -nocomplain -types f [file join $cortexa53srcdir1 *]] {
                file copy -force $entry "./src/"
            }
            foreach entry [glob -nocomplain [file join $ccdir *]] {
                file copy -force $entry "./src/"
            }
	    foreach entry [glob -nocomplain -types f [file join $platformsrcdir *]] {
	    	file copy -force $entry "./src/"
	    }
	    foreach entry [glob -nocomplain -types f [file join $platformincludedir *]] {
	        file copy -force $entry "./src/includes_ps/"
	    }
            if { $enable_sw_profile == "true" } {
                error "ERROR: Profiling is not supported for A53/A72"
            }
	    set pss_ref_clk_mhz [common::get_property CONFIG.C_PSS_REF_CLK_FREQ $hw_proc_handle]
            if { $pss_ref_clk_mhz == "" } {
                puts "WARNING: CONFIG.C_PSS_REF_CLK_FREQ not found. Using default value for XPAR_PSU_PSS_REF_CLK_FREQ_HZ."
                set pss_ref_clk_mhz 33333000
             }
            set file_handle [::hsi::utils::open_include_file "xparameters.h"]
	    puts $file_handle " /* Definition for PSS REF CLK FREQUENCY */"
            puts $file_handle [format %s%.0f%s "#define XPAR_PSU_PSS_REF_CLK_FREQ_HZ " [expr $pss_ref_clk_mhz]  "U"]
            puts $file_handle ""
            puts $file_handle "#include \"xparameters_ps.h\""
            puts $file_handle ""
            # If board name is valid, define corresponding symbol in xparameters
            if { [string length $boardname] != 0 } {
                set fields [split $boardname ":"]
                lassign $fields prefix board suffix
                if { [string length $board] != 0 } {
                    set def "#define XPS_BOARD_"
                    append def [string toupper $board]
                    puts $file_handle $def
                    puts $file_handle ""
                }
            }
            xdefine_fabric_reset $file_handle
            close $file_handle
        }
        "psu_cortexr5" -
	"psv_cortexr5" -
	"psxl_cortexr52" -
	"psx_cortexr52"
	{
	    set procdrv [hsi::get_sw_processor]
	    set includedir "./src/arm/ARMv8/includes_ps/"
	    file copy -force $includedir "./src/"
	    if {$proctype == "psv_cortexr5"} {
	        set platformincludedir "./src/arm/ARMv8/includes_ps/platform/Versal"
	    } elseif {$proctype == "psxl_cortexr52" || $proctype == "psx_cortexr52"} {
	        set platformincludedir "./src/arm/platform/versal_net"
	    } else {
	        set platformincludedir "./src/arm/ARMv8/includes_ps/platform/ZynqMP"
	    }
	    if {[string compare -nocase $compiler "iccarm"] == 0} {
	           set ccdir "./src/arm/cortexr5/iccarm"
	    } elseif {[string compare -nocase $compiler "armclang"] == 0} {
	           set ccdir "./src/arm/cortexr5/armclang"
           } else {
	           set ccdir "./src/arm/cortexr5/gcc"
	   }
	    foreach entry [glob -nocomplain -types f [file join $cortexr5srcdir *]] {
		file copy -force $entry "./src/"
	    }
	    foreach entry [glob -nocomplain [file join $ccdir *]] {
		file copy -force $entry "./src/"
	    }

	    foreach entry [glob -nocomplain [file join $platformincludedir *]] {
	        file copy -force $entry "./src/includes_ps/"
	    }

            if { $enable_sw_profile == "true" } {
                error "ERROR: Profiling is not supported for R5"
            }
	    set pss_ref_clk_mhz [common::get_property CONFIG.C_PSS_REF_CLK_FREQ $hw_proc_handle]
	    if { $pss_ref_clk_mhz == "" } {
		puts "WARNING: CONFIG.C_PSS_REF_CLK_FREQ not found. Using default value for XPAR_PSU_PSS_REF_CLK_FREQ_HZ."
		set pss_ref_clk_mhz 33333000
	    }
	    set file_handle [::hsi::utils::open_include_file "xparameters.h"]
	    puts $file_handle " /* Definition for PSS REF CLK FREQUENCY */"
	    puts $file_handle [format %s%.0f%s "#define XPAR_PSU_PSS_REF_CLK_FREQ_HZ " [expr $pss_ref_clk_mhz]  "U"]
	    puts $file_handle ""
	    puts $file_handle "#include \"xparameters_ps.h\""
	    puts $file_handle ""
	    if {[llength $is_versal_net] > 0} {
		set platformsrcdir "./src/arm/cortexr5/platform/versal-net"
		set procsrcdir "./src/arm/cortexr5/platform/CortexR52"
		file copy -force $platformsrcdir/mpu_r52.c "./src/mpu.c"
		file copy -force $procsrcdir/xil_mpu_r52.c "./src/xil_mpu.c"
		file copy -force $procsrcdir/xil_mpu_r52.h "./src/xil_mpu.h"
	    }  elseif {[llength $is_versal] > 0 } {
		set platformsrcdir "./src/arm/cortexr5/platform/versal"
		set procsrcdir "./src/arm/cortexr5/platform/CortexR5"
		file copy -force $platformsrcdir/mpu_r5.c "./src/mpu.c"
		file copy -force $platformsrcdir/xparameters_ps.h "./src/"
		file copy -force $procsrcdir/xil_mpu_r5.c "./src/xil_mpu.c"
		file copy -force $procsrcdir/xil_mpu_r5.h "./src/xil_mpu.h"
	    } else {
	        set platformsrcdir "./src/arm/cortexr5/platform/ZynqMP"
		set procsrcdir "./src/arm/cortexr5/platform/CortexR5"
		file copy -force $platformsrcdir/mpu_r5.c "./src/mpu.c"
		file copy -force $platformsrcdir/xparameters_ps.h "./src/"
		file copy -force $procsrcdir/xil_mpu_r5.c "./src/xil_mpu.c"
		file copy -force $procsrcdir/xil_mpu_r5.h "./src/xil_mpu.h"
	    }
            # If board name is valid, define corresponding symbol in xparameters
            if { [string length $boardname] != 0 } {
                set fields [split $boardname ":"]
                lassign $fields prefix board suffix
                if { [string length $board] != 0 } {
                    set def "#define XPS_BOARD_"
                    append def [string toupper $board]
                    puts $file_handle $def
                    puts $file_handle ""
                }
            }
	   set design_list [hsi::get_cells -hier]
           if {[lsearch  -nocase $design_list "psu_ddr_1"] >= 0} {
                set psu_ddr_1_baseaddr [::hsi::utils::get_param_value [hsi::get_cells -hier "psu_ddr_1"] C_S_AXI_BASEADDR]
                set psu_ddr_1_highaddr [::hsi::utils::get_param_value [hsi::get_cells -hier "psu_ddr_1"] C_S_AXI_HIGHADDR ]
                puts $file_handle "/******************************************************************/"
                puts $file_handle ""
                puts $file_handle " /*Definitions for peripheral PSU_R5_DDR_1 */"
                puts $file_handle [format %s0x%x "#define XPAR_PSU_R5_DDR_1_S_AXI_BASEADDR " [expr $psu_ddr_1_baseaddr]]
                puts $file_handle [format %s0x%x "#define XPAR_PSU_R5_DDR_1_S_AXI_HIGHADDR " [expr $psu_ddr_1_highaddr]]
                puts $file_handle ""
            }
            xdefine_fabric_reset $file_handle
	    close $file_handle
        }
       "ps7_cortexa9"  {
                   set procdrv [hsi::get_sw_processor]
                   set compiler [common::get_property CONFIG.compiler $procdrv]
                   if {[string compare -nocase $compiler "armcc"] == 0} {
                       set ccdir "./src/arm/cortexa9/armcc"
	    } elseif {[string compare -nocase $compiler "iccarm"] == 0} {
		set ccdir "./src/arm/cortexa9/iccarm"
                   } else {
                       set ccdir "./src/arm/cortexa9/gcc"
                   }
                   foreach entry [glob -nocomplain -types f [file join $cortexa9srcdir *]] {
                       file copy -force $entry "./src/"
                   }
                   foreach entry [glob -nocomplain [file join $ccdir *]] {
                       file copy -force $entry "./src/"
                   }
                   if {[string compare -nocase $compiler "armcc"] == 0} {
                       set enable_sw_profile "false"
	    }
		if {[string compare -nocase $compiler "iccarm"] == 0} {
                           set enable_sw_profile "false"
                   }
                   set file_handle [::hsi::utils::open_include_file "xparameters.h"]
                   puts $file_handle "#include \"xparameters_ps.h\""
                   puts $file_handle ""
                   close $file_handle
        }
	"microblaze_riscv" {
            foreach entry [glob -nocomplain [file join $riscvsrcdir *]] {
                file copy -force $entry "./src/"
            }

        }
        "default" {puts "unknown processor type $proctype\n"}
    }

    set sleep_file_list "sleep.h sleep.c usleep.c xtime_l.c xtime_l.h microblaze_sleep.c microblaze_sleep.h xil_sleepcommon.c xil_sleeptimer.h xil_sleeptimer.c"
    if {$is_xiltimer_enabled != 0} {
        foreach entry $sleep_file_list {
            file delete -force "./src/$entry"
        }
    }
    # Write the Config.make file
    set makeconfig [open "./src/config.make" w]
#    print_generated_header_tcl $makeconfig "Configuration parameters for Standalone Makefile"
    if { $proctype == "microblaze" || $proctype == "psu_pmu" || $proctype == "psu_psm" || $proctype == "psu_pmc" || $proctype == "psv_psm" || $proctype == "psv_pmc" || $proctype == "psxl_pmc" || $proctype == "psxl_psm" || $proctype == "psx_pmc" || $proctype == "psx_psm"} {
        puts $makeconfig "LIBSOURCES = *.c *.S"
        puts $makeconfig "PROFILE_ARCH_OBJS = profile_mcount_mb.o"
    } elseif { $proctype == "psu_cortexr5" ||  $proctype == "psv_cortexr5" || $proctype == "psxl_cortexr52" || $proctype == "psx_cortexr52"} {
	puts $makeconfig "LIBSOURCES = *.c *.S"
    } elseif { $proctype == "psu_cortexa53" || $proctype == "psu_cortexa72" || $proctype == "psv_cortexa72" || $proctype == "psxl_cortexa78" || $proctype == "psx_cortexa78"}  {
            puts $makeconfig "LIBSOURCES = *.c *.S"
    } elseif { $proctype == "ps7_cortexa9" } {
        if {[string compare -nocase $compiler "armcc"] == 0} {
            puts $makeconfig "LIBSOURCES = *.c *.s"
        } elseif {[string compare -nocase $compiler "iccarm"] == 0} {
            puts $makeconfig "LIBSOURCES = *.c *.s"
		} else {
            puts $makeconfig "LIBSOURCES = *.c *.S"
            puts $makeconfig "PROFILE_ARCH_OBJS = profile_mcount_arm.o"
        }
    } elseif { $proctype == "microblaze_riscv" } {
        puts $makeconfig "LIBSOURCES = *.c *.S"
        puts $makeconfig "PROFILE_ARCH_OBJS = profile_mcount_riscv.o"
    } else {
        error "ERROR: processor $proctype is not supported"
    }
    if { $enable_sw_profile == "true" } {
        puts $makeconfig "LIBS = standalone_libs profile_libs"
    } else {
        puts $makeconfig "LIBS = standalone_libs"
    }
    close $makeconfig


    # Handle stdin
    set stdin [common::get_property CONFIG.stdin $os_handle]
    set stdin_ipname ""
    if { $stdin != "none" } {
        set stdin_ipname [common::get_property IP_NAME [hsi::get_cells -hier $stdin]]
    }
    if { ($proctype == "psv_pmc" && $stdin_ipname != "psv_sbsauart") || ($proctype == "psxl_pmc" && $stdin_ipname != "psxl_sbsauart") || ($proctype == "psx_pmc" && $stdin_ipname != "psx_sbsauart")} {
            common::set_property CONFIG.stdin "none" $os_handle
            handle_stdin_parameter $os_handle
    } elseif { $stdin == "" || $stdin == "none" } {
            handle_stdin_parameter $os_handle
    } elseif { (($proctype == "psx_psm" || $proctype == "psv_psm" || $proctype == "psu_pmu"  || $proctype == "microblaze") && ([string match "*coresight*" $stdin_ipname]))} {
       common::set_property CONFIG.stdin "none" $os_handle
       handle_stdin_parameter $os_handle
    } else {
            ::hsi::utils::handle_stdin $os_handle
    }

    # Handle stdout
    set stdout [common::get_property CONFIG.stdout $os_handle]
    set stdout_ipname ""
    if { $stdout != "none" } {
        set stdout_ipname [common::get_property IP_NAME [hsi::get_cells -hier $stdout]]
    }
    if { $proctype == "psv_pmc" || $proctype == "psxl_pmc" || $proctype == "psx_pmc"} {
		if {$stdout_ipname != "psv_sbsauart" && $stdout_ipname != "psxl_sbsauart" && $stdout_ipname != "psx_sbsauart"} {
			common::set_property CONFIG.stdout "none" $os_handle
		}
                handle_stdout_parameter $os_handle
    } elseif { $stdout == "" || $stdout == "none" } {
                handle_stdout_parameter $os_handle
    } elseif { (($proctype == "psx_psm" || $proctype == "psv_psm" || $proctype == "psu_pmu"  || $proctype == "microblaze") && ([string match "*coresight*" $stdout_ipname]))} {
       common::set_property CONFIG.stdout "none" $os_handle
       handle_stdout_parameter $os_handle
    } else {
                ::hsi::utils::handle_stdout $os_handle
    }


    #Handle Profile configuration
    if { $enable_sw_profile == "true" } {
        handle_profile $os_handle $proctype
    }

    set file_handle [::hsi::utils::open_include_file "xparameters.h"]
    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle

    # Create config file for microblaze interrupt handling
    if { [string compare -nocase $need_config_file "true"] == 0 } {
        xhandle_mb_interrupts
    }

    # Create config files for Microblaze exception handling
    if { $proctype == "microblaze" && [mb_has_exceptions $hw_proc_handle] } {
        xcreate_mb_exc_config_file $os_handle
    }

    # Create config files for Microblaze RISC-V exception handling
    if { $proctype == "microblaze_riscv" } {
        xcreate_riscv_exc_config_file $os_handle
    }

    # Create bspconfig file
    set bspcfg_fn [file join "src" "bspconfig.h"]
    file delete $bspcfg_fn
    set bspcfg_fh [open $bspcfg_fn w]
    ::hsi::utils::write_c_header $bspcfg_fh "Configurations for Standalone BSP"
    puts $bspcfg_fh "#ifndef BSPCONFIG_H  /* prevent circular inclusions */"
    puts $bspcfg_fh "#define BSPCONFIG_H  /* by using protection macros */"
    puts $bspcfg_fh ""

    set slaves [common::get_property   SLAVES [  hsi::get_cells -hier $sw_proc_handle]]

    set clocking_supported [common::get_property CONFIG.clocking $os_handle ]
    set is_zynqmp_fsbl_bsp [common::get_property CONFIG.ZYNQMP_FSBL_BSP [hsi::get_os]]
    #Currently clocking is supported for zynqmp only
    if {$is_zynqmp_fsbl_bsp != true &&  $clocking_supported == true  &&  [llength $cortexa53proc] > 0} {
        foreach slave $slaves {
            if {[string compare -nocase "psu_crf_apb" $slave] == 0 } {
               puts $bspcfg_fh "#define XCLOCKING"
            }
        }
    }

    set xpm_supported [common::get_property CONFIG.xpm_support $os_handle ]
    #Currently xpm is supported for versal and versal net only
    if {$xpm_supported == true  &&  ([llength $cortexa72proc] > 0 || [llength $cortexa78proc] > 0)} {
        puts $bspcfg_fh "#define XPM_SUPPORT"
    }

    if { $proctype == "microblaze" && [mb_has_pvr $hw_proc_handle] } {

        set pvr [common::get_property CONFIG.C_PVR $hw_proc_handle]

        switch $pvr {
            "0" {
                puts $bspcfg_fh "#define MICROBLAZE_PVR_NONE"
            }
            "1" {
                puts $bspcfg_fh "#define MICROBLAZE_PVR_BASIC"
            }
            "2" {
                puts $bspcfg_fh "#define MICROBLAZE_PVR_FULL"
            }
            "default" {
                puts $bspcfg_fh "#define MICROBLAZE_PVR_NONE"
            }
        }
    } else {
        puts $bspcfg_fh "#define MICROBLAZE_PVR_NONE"
    }

    if { $proctype == "psu_cortexa53" } {
	if {[string compare -nocase $compiler "arm-none-eabi-gcc"] != 0} {
		set hypervisor_guest [common::get_property CONFIG.hypervisor_guest $os_handle ]
		if { $hypervisor_guest == "true" } {
			puts $bspcfg_fh "#define EL3 0"
			puts $bspcfg_fh "#define EL1_NONSECURE 1"
			puts $bspcfg_fh "#define HYP_GUEST 1"
		} else {
			puts $bspcfg_fh "#define EL3 1"
			puts $bspcfg_fh "#define EL1_NONSECURE 0"
			puts $bspcfg_fh "#define HYP_GUEST 0"
		}
	}
    } elseif { $proctype == "psu_cortexa72" || $proctype == "psv_cortexa72" || $proctype == "psxl_cortexa78" || $proctype == "psx_cortexa78"} {
                set extra_flags [common::get_property CONFIG.extra_compiler_flags [hsi::get_sw_processor]]
		if {$proctype == "psxl_cortexa78" || $proctype == "psx_cortexa78"} {
                        set flagindex [string first {-DARMA78_EL3} $extra_flags 0]
		} else {
			set flagindex [string first {-DARMA72_EL3} $extra_flags 0]
		}
                if { $flagindex == -1 } {
                     puts $bspcfg_fh "#define EL3 0"
                     puts $bspcfg_fh "#define EL1_NONSECURE 1"
                     puts $bspcfg_fh "#define HYP_GUEST 0"
               } else {
                    puts $bspcfg_fh "#define EL3 1"
                    puts $bspcfg_fh "#define EL1_NONSECURE 0"
                    puts $bspcfg_fh "#define HYP_GUEST 0"
               }

    } elseif { $proctype == "ps7_cortexa9" } {
		if {[string compare -nocase $compiler "arm-none-eabi-gcc"] == 0} {
			set extra_flags [::common::get_property VALUE [hsi::get_comp_params -filter { NAME == extra_compiler_flags } ] ]
			set flagindex [string first {-mfloat-abi=hard} $extra_flags 0]
			puts $bspcfg_fh ""
			puts $bspcfg_fh "/* Definition for hard-float ABI */"
			if { $flagindex != -1 } {
				puts $bspcfg_fh "#define FPU_HARD_FLOAT_ABI_ENABLED 1"
			} else {
				puts $bspcfg_fh "#define FPU_HARD_FLOAT_ABI_ENABLED 0"
			}
		}
    }
	puts $bspcfg_fh ""
    puts $bspcfg_fh "\#endif /*end of __BSPCONFIG_H_*/"
    close $bspcfg_fh

	set file_handle [::hsi::utils::open_include_file "xparameters.h"]

	puts $file_handle "/* Platform specific definitions */"
    if { $proctype == "psu_cortexa53" || $proctype == "psu_cortexr5"} {
	puts $file_handle "#define PLATFORM_ZYNQMP"
    }
    if { $proctype == "ps7_cortexa9"} {
	puts $file_handle "#define PLATFORM_ZYNQ"
    }
    if { $proctype == "microblaze"} {
	puts $file_handle "#define PLATFORM_MB"
    }
    if { $proctype == "microblaze_riscv"} {
	puts $file_handle "#define PLATFORM_RISCV"
    }

    if { $proctype == "psv_pmc"} {
	puts $file_handle "#define VERSAL_PLM"
	foreach entry [glob -nocomplain [file join $versalsrcdir *]] {
		file copy -force $entry "./src/"
	}
    }

    if { $proctype == "psxl_pmc" || $proctype == "psx_pmc"} {
	puts $file_handle "#define VERSAL_PLM"
	puts $file_handle "#define VERSALNET_PLM"
	foreach entry [glob -nocomplain [file join $versalnetsrcdir *]] {
		file copy -force $entry "./src/"
	}
    }

    if {[llength $cortexa72proc] > 0} {
	puts $file_handle "#ifndef versal"
        puts $file_handle "#define versal"
        puts $file_handle "#endif"
        puts $file_handle ""
	# C convention expects macros to be all caps
	puts $file_handle "#ifndef VERSAL"
        puts $file_handle "#define VERSAL"
        puts $file_handle "#endif"
        puts $file_handle ""

	foreach entry [glob -nocomplain [file join $versalsrcdir *]] {
		file copy -force $entry "./src/"
	}
    }

    if {[llength $cortexa78proc] > 0} {
        puts $file_handle "#ifndef versal"
        puts $file_handle "#define versal"
        puts $file_handle "#endif"
        puts $file_handle ""

        puts $file_handle "#ifndef VERSAL_NET"
        puts $file_handle "#define VERSAL_NET"
        puts $file_handle "#endif"
	foreach entry [glob -nocomplain [file join $versalnetsrcdir *]] {
		file copy -force $entry "./src/"
	}
    }
    if { $proctype == "psu_cortexr5" || $proctype == "psv_cortexr5" || $proctype == "psxl_cortexr52" || $proctype == "psx_cortexr52"} {
	 set lockstep_debug [common::get_property CONFIG.lockstep_mode_debug $os_handle]
	 puts $file_handle " "
	 puts $file_handle "/* Definitions for debug logic configuration in lockstep mode */"
	 if { $lockstep_debug == "true" } {
		puts $file_handle "#define LOCKSTEP_MODE_DEBUG 1U"
	 } else {
		puts $file_handle "#define LOCKSTEP_MODE_DEBUG 0U"
	 }
	if { $proctype == "psu_cortexr5" || $proctype == "psv_cortexr5" } {
		 foreach entry [glob -nocomplain [file join $versalsrcdir *]] {
			file copy -force $entry "./src/"
		}
	}
	if { $proctype == "psxl_cortexr52" || $proctype == "psx_cortexr52" } {
		 foreach entry [glob -nocomplain [file join $versalnetsrcdir *]] {
			file copy -force $entry "./src/"
		}
	}
     }
     set interrupt_wrap_supported [common::get_property CONFIG.xil_interrupt $os_handle ]
     if {$interrupt_wrap_supported == true} {
	 puts $file_handle " "
	 puts $file_handle "/* Definition for xilinx interrupt wrapper support  */"
         puts $file_handle "#define XIL_INTERRUPT"
     }
	 puts $file_handle " "
	 puts $file_handle "/* Definitions for sleep timer configuration */"
	 xsleep_timer_config $proctype $os_handle $file_handle
	 puts $file_handle " "
	if { $proctype == "psu_cortexr5" || $proctype == "psv_cortexr5" || $proctype == "psxl_cortexr52" || $proctype == "psx_cortexr52"} {
		puts $file_handle "/* Definitions for processor access to RPU/IOU slcr address space*/"
		set r5_access [get_processor_access]
		puts $file_handle "#define PROCESSOR_ACCESS_VALUE $r5_access"
	}
	 puts $file_handle " "
	 puts $file_handle "/******************************************************************/"
	close $file_handle
#	xcreate_cmake_toolchain_file $os_handle $cortexa72proc
}

proc xcreate_cmake_toolchain_file {os_handle is_versal} {
	# Get the processor
	set proc_instance [hsi::get_sw_processor]
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];
	set os [common::get_property NAME [hsi::get_os]]
	set compiler_str [common::get_property CONFIG.compiler -object ${proc_instance}]
	set compiler_l [split ${compiler_str}]
	set compiler [lindex ${compiler_l} 0]
	set empty_string ""
	set crosscompiler_postfix [lindex [split $compiler "-"] end]
	set crosscompile [string map [list $crosscompiler_postfix $empty_string] "${compiler}"]
	set c_flags [common::get_property CONFIG.compiler_flags -object ${proc_instance}]
	set extra_flags [common::get_property CONFIG.extra_compiler_flags -object ${proc_instance}]
	set linclude [file normalize "../.."]
	set extra_flags "${extra_flags} -I${linclude}/include"

	# Generate cmake toolchain file
	set toolchain_cmake "toolchain"
	set fd [open "src/${toolchain_cmake}.cmake" w]

	if { "${proc_type}" == "psu_cortexr5" } {
		puts $fd "set (CMAKE_SYSTEM_PROCESSOR \"cortexr5\" CACHE STRING \"\")"
		if { [llength $is_versal] > 0} {
			puts $fd "set (MACHINE \"versal\")"
		} else {
			puts $fd "set (MACHINE \"ZynqMP\")"
		}
	} elseif { "${proc_type}" == "psu_cortexa53"} {
		if { "${compiler}" == "arm-none-eabi-gcc" } {
			puts $fd "set (CMAKE_SYSTEM_PROCESSOR \"cortexa53-32\" CACHE STRING \"\")"
			puts $fd "set (MACHINE \"ZynqMP\")"
		} else {
			puts $fd "set (CMAKE_SYSTEM_PROCESSOR \"cortexa53\" CACHE STRING \"\")"
			puts $fd "set (MACHINE \"ZynqMP\")"
		}
	} elseif { "${proc_type}" == "psu_cortexa72"} {
			puts $fd "set (CMAKE_SYSTEM_PROCESSOR \"cortexa72\" CACHE STRING \"\")"
			puts $fd "set (MACHINE \"versal\")"
	} elseif { "${proc_type}" == "ps7_cortexa9" } {
		puts $fd "set (CMAKE_SYSTEM_PROCESSOR \"cortexa9\" CACHE STRING \"\")"
		puts $fd "set (MACHINE \"Zynq\")"
	} elseif { "${proc_type}" == "microblaze" || "${proc_type}" == "psu_pmc" || "${proc_type}" == "psu_psm" || "${proc_type}" == "psv_pmc" || "${proc_type}" == "psv_psm" || "${proc_type}" == "psu_pmu"} {
		puts $fd "set (CMAKE_SYSTEM_PROCESSOR \"microblaze\" CACHE STRING \"\")"
		puts $fd "set (MACHINE \"microblaze_generic\")"
		set c_flags "${c_flags}  -mlittle-endian"
	}
	puts $fd "set (CROSS_PREFIX \"${crosscompile}\" CACHE STRING \"\")"
	puts $fd "set (CMAKE_C_FLAGS \"${c_flags} ${extra_flags}\" CACHE STRING \"\")"
	puts $fd "set (CMAKE_ASM_FLAGS \"${c_flags} ${extra_flags}\" CACHE STRING \"\")"
	if { [string match "freertos*" "${os}"] > 0 } {
		puts $fd "set (CMAKE_SYSTEM_NAME \"FreeRTOS\" CACHE STRING \"\")"
	} else {
		puts $fd "set (CMAKE_SYSTEM_NAME \"Generic\" CACHE STRING \"\")"
	}
	puts $fd "include (CMakeForceCompiler)"
	puts $fd "CMAKE_FORCE_C_COMPILER (\"\$\{CROSS_PREFIX\}gcc\" GNU)"
	puts $fd "CMAKE_FORCE_CXX_COMPILER (\"\$\{CROSS_PREFIX\}g++\" GNU)"

	puts $fd "set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER CACHE STRING \"\")"
	puts $fd "set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER CACHE STRING \"\")"
	puts $fd "set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER CACHE STRING \"\")"
	close $fd

	# Run cmake to generate make file
	set bdir "build_xilstandalone"
	if { [catch {file mkdir "${bdir}"} msg] } {
		error "Failed to create xilstandalone build directory."
	}
	set workdir [pwd]
	cd "${bdir}"
	set cmake_cmd "../src/run_cmake"
	set os_platform_type "$::tcl_platform(platform)"

	#copy toolchain file to libsrc directory of BSP
	file copy -force ../src/toolchain.cmake ../../

	set cmake_opt "-DCMAKE_TOOLCHAIN_FILE=../src/toolchain.cmake"
	append cmake_opt " -DCMAKE_INSTALL_PREFIX=/"
	append cmake_opt " -DCMAKE_VERBOSE_MAKEFILE=on"
	append cmake_opt " -DWITH_DEFAULT_LOGGER=off"

	if { [string match -nocase "windows*" "${os_platform_type}"] == 0 } {
		# Linux
		file attributes ${cmake_cmd} -permissions ugo+rx
		if { [catch {exec ${cmake_cmd} "../src/"  ${cmake_opt}} msg] } {
			error "Failed to generate cmake files.${msg}"
		} else {
			puts "${msg}"
		}
	} else {
		# Windows
		# Note: windows tcl exec does not do well when trying to provide ${cmake_opt}
		#       for now hardcoding the values directly on the command line.
		if { [catch {exec ${cmake_cmd} "../src/" -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=../src/toolchain.cmake -DCMAKE_INSTALL_PREFIX=/ -DCMAKE_VERBOSE_MAKEFILE=on -DWITH_DEFAULT_LOGGER=off -DWITH_DOC=off } msg] } {
			error "Failed to generate cmake files.${msg}"
		} else {
			puts "${msg}"
		}
	}

}

# --------------------------------------------------------
#  Tcl procedure xsleep_timer_config
# --------------------------------------------------------
proc xsleep_timer_config {proctype os_handle file_handle} {

    set sleep_timer [common::get_property CONFIG.sleep_timer $os_handle ]
	set is_versal [hsi::get_cells -hier -filter {IP_NAME=="psv_cortexa72" || IP_NAME=="psxl_cortexa78" || IP_NAME=="psx_cortexa78"}]
	if { $sleep_timer == "ps7_globaltimer_0" || $sleep_timer == "psu_iou_scntr" || $sleep_timer == "psu_iou_scntrs" || $sleep_timer == "psv_iou_scntr" || $sleep_timer == "psv_iou_scntrs"} {
		if { $proctype == "psu_cortexr5" ||  $proctype == "psv_cortexr5"} {
			error "ERROR: $proctype does not support $sleep_timer "
		}
    } elseif { $sleep_timer == "none" } {
		if { $proctype == "psu_cortexr5" || $proctype == "psv_cortexr5" || $proctype == "psxl_cortexr52" || $proctype == "psx_cortexr52"} {
			set is_ttc_present 0
			set periphs [hsi::get_cells -hier -filter {IP_NAME==ps7_ttc || IP_NAME==psu_ttc || IP_NAME==psv_ttc || IP_NAME==psxl_ttc || IP_NAME==psx_ttc}]
			set periphs [lsort -decreasing $periphs]
                        set en_pmu_sleep_timer [common::get_property CONFIG.pmu_sleep_timer $os_handle ]

			foreach periph $periphs {
				set base_addr [get_base_value $periph]
				set base_addr [string trimleft $base_addr "0x"]
				set base_addr [string toupper $base_addr]

				if {[string compare -nocase "psu_ttc_3" $periph] == 0 && [is_ttc_accessible_from_processor $periph] == 1} {
					set is_ttc_present 1
					if {$en_pmu_sleep_timer == false} {
						puts $file_handle "#define SLEEP_TIMER_BASEADDR XPAR_PSU_TTC_9_BASEADDR"
						puts $file_handle "#define SLEEP_TIMER_FREQUENCY XPAR_PSU_TTC_9_TTC_CLK_FREQ_HZ"
						puts $file_handle "#define XSLEEP_TTC_INSTANCE 3"
					}
					break
				} elseif {[string compare -nocase "psu_ttc_2" $periph] == 0 && [is_ttc_accessible_from_processor $periph] == 1} {
					set is_ttc_present 1
					if {$en_pmu_sleep_timer == false} {
						puts $file_handle "#define SLEEP_TIMER_BASEADDR XPAR_PSU_TTC_6_BASEADDR"
						puts $file_handle "#define SLEEP_TIMER_FREQUENCY XPAR_PSU_TTC_6_TTC_CLK_FREQ_HZ"
						puts $file_handle "#define XSLEEP_TTC_INSTANCE 2"
					}
					break
				} elseif {([llength $is_versal] > 0) && ([string compare -nocase "psv_ttc_3" $periph] == 0 || [string match -nocase $base_addr "FF110000"]) && [is_ttc_accessible_from_processor $periph] == 1} {
					set is_ttc_present 1
					if {$en_pmu_sleep_timer == false} {
						puts $file_handle "#define SLEEP_TIMER_BASEADDR [format XPAR_%s_9_BASEADDR [string toupper [string range [common::get_property NAME $periph] 0 end-2]]]"
						puts $file_handle "#define SLEEP_TIMER_FREQUENCY [format XPAR_%s_9_TTC_CLK_FREQ_HZ [string toupper [string range [common::get_property NAME $periph] 0 end-2]]]"
						puts $file_handle "#define XSLEEP_TTC_INSTANCE 3"
					}
					break
				} elseif {([llength $is_versal] > 0) && ([string compare -nocase "psv_ttc_2" $periph] == 0 || [string match -nocase $base_addr "FF100000"]) && [is_ttc_accessible_from_processor $periph] == 1} {
					set is_ttc_present 1
					if {$en_pmu_sleep_timer == false} {
						puts $file_handle "#define SLEEP_TIMER_BASEADDR [format XPAR_%s_6_BASEADDR [string toupper [string range [common::get_property NAME $periph] 0 end-2]]]"
						puts $file_handle "#define SLEEP_TIMER_FREQUENCY [format XPAR_%s_6_TTC_CLK_FREQ_HZ [string toupper [string range [common::get_property NAME $periph] 0 end-2]]]"
						puts $file_handle "#define XSLEEP_TTC_INSTANCE 2"
					}
					break
				}
			}
			if { $is_ttc_present == 0 } {
				puts "WARNING: Either TTC2/TTC3 is not present in the HW design or not accessible to processor, \
				CortexR5 PMU cycle counter would be used for sleep routines until and unless DONT_USE_PMU_FOR_SLEEP_ROUTINES \
				flag is used in BSP compiler flags"
			} else {
				if {$en_pmu_sleep_timer == false} {
					puts "$periph will be used in sleep routines for delay generation"
				}
			}
		}

		puts $file_handle "#define XSLEEP_TIMER_IS_DEFAULT_TIMER"
    } elseif {[string match "axi_timer_*" $sleep_timer]} {
		if { $proctype == "microblaze" || $proctype == "microblaze_riscv" } {
			set instance [string index $sleep_timer end]
			puts $file_handle "#define SLEEP_TIMER_BASEADDR [format "XPAR_AXI_TIMER_%d_BASEADDR" $instance ] "
			puts $file_handle "#define SLEEP_TIMER_FREQUENCY [format "XPAR_AXI_TIMER_%d_CLOCK_FREQ_HZ" $instance ] "
			puts $file_handle "#define XSLEEP_TIMER_IS_AXI_TIMER"
		} else {
			error "ERROR: $proctype does not support $sleep_timer "
		}
	} else {
        set module [string index $sleep_timer end]
		puts $file_handle "#define XSLEEP_TTC_INSTANCE $module"
	    set timer [common::get_property CONFIG.TTC_Select_Cntr $os_handle ]
		if { $proctype == "ps7_cortexa9" } {
			puts $file_handle "#define SLEEP_TIMER_BASEADDR [format "XPAR_PS7_TTC_%d_BASEADDR" [ expr 3 * $module + $timer ] ] "
			puts $file_handle "#define SLEEP_TIMER_FREQUENCY [format "XPAR_PS7_TTC_%d_TTC_CLK_FREQ_HZ" [ expr 3 * $module + $timer ] ] "
		} elseif { $proctype == "psu_cortexa53" || $proctype == "psu_cortexr5" || $proctype == "psu_cortexa72" } {
			if { $proctype == "psu_cortexr5" && [is_ttc_accessible_from_processor $sleep_timer] == 0 } {
				error "ERROR: $sleep_timer is secure and it is not accessible to the processor. Please select non secure ttc \
					instance as sleep_timer from BSP settings"
			}
			puts $file_handle "#define SLEEP_TIMER_BASEADDR [format "XPAR_PSU_TTC_%d_BASEADDR" [ expr 3 * $module + $timer ] ] "
			puts $file_handle "#define SLEEP_TIMER_FREQUENCY [format "XPAR_PSU_TTC_%d_TTC_CLK_FREQ_HZ" [ expr 3 * $module + $timer ] ] "
		} elseif {$proctype == "psv_cortexr5" || $proctype == "psv_cortexa72" || $proctype == "psx_cortexa78" || $proctype == "psx_cortexr52"} {
			if { ($proctype == "psv_cortexr5" || $proctype == "psx_cortexr52") && [is_ttc_accessible_from_processor $sleep_timer] == 0 } {
				error "ERROR: $sleep_timer is secure and it is not accessible to the processor. Please select non secure ttc \
					instance as sleep_timer from BSP settings"
			}
			puts $file_handle "#define SLEEP_TIMER_BASEADDR [format "XPAR_XTTCPS_%d_BASEADDR" [ expr 3 * $module + $timer ] ] "
			puts $file_handle "#define SLEEP_TIMER_FREQUENCY [format "XPAR_XTTCPS_%d_TTC_CLK_FREQ_HZ" [ expr 3 * $module + $timer ] ] "
		}
	}
	return
}
# --------------------------------------
# Tcl procedure get_psu_config
# --------------------------------------
proc get_psu_config {name} {
    set ps_periph [hsi::get_cells -hier zynq_ultra_ps_e_0]
    if { [llength $ps_periph] == 1} {
        return [common::get_property [format %s%s "CONFIG." $name] $ps_periph]
    } else {
        return ""
    }
}

# --------------------------------------
# Tcl procedure xhandle_mb_interrupts
# --------------------------------------
proc xhandle_mb_interrupts {} {

    set default_interrupt_handler "XNullHandler"
    set default_arg "XNULL"

    set source_interrupt_handler $default_interrupt_handler
    set source_handler_arg $default_arg

    # Handle the interrupt pin
    set sw_proc_handle [hsi::get_sw_processor]
    set periph [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc_handle] ]
    set source_ports [::hsi::utils::get_interrupt_sources $periph]
    if {[llength $source_ports] > 1} {
        error "ERROR: Too many interrupting ports on the MicroBlaze. Should only find 1" "" "hsi_error"
        return
    }
    if { [llength $source_ports] != 0 } {
        set source_periph [hsi::get_cells -of_objects $source_ports]
        if { [llength $source_periph] != 0 } {
            set source_driver [hsi::get_drivers -filter "HW_INSTANCE==$source_periph"]
            if { [llength $source_driver] != 0 } {
                set intr_array [hsi::get_arrays -of_objects $source_driver -filter "NAME==interrupt_handler"]
                if { [llength $intr_array] != 0 } {
                    set array_size [common::get_property PROPERTY.size $intr_array]
                    for { set i 0 } { $i < $array_size } { incr i } {
                        set int_port [lindex [common::get_property PARAM.int_port $intr_array] $i]
                        if { [llength $int_port] != 0 } {
                            if { [string compare -nocase $int_port $source_ports] == 0 } {
                                set source_interrupt_handler [lindex [common::get_property PARAM.int_handler $intr_array] $i]
                                set source_handler_arg [lindex [common::get_property PARAM.int_handler_arg $intr_array] $i]
                                if { [string compare -nocase $source_handler_arg DEVICE_ID] == 0 } {
                                    set source_handler_arg [::hsi::utils::get_ip_param_name $source_periph "DEVICE_ID"]
                                } else {
                                    set source_handler_arg [::hsi::utils::get_ip_param_name $source_periph "C_BASEADDR"]
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    # Generate microblaze_interrupts_g.c file...
    xcreate_mb_intr_config_file $source_interrupt_handler $source_handler_arg
}
# -------------------------------------------
# Tcl procedure xcreate_mb_intr_config file
# -------------------------------------------
proc xcreate_mb_intr_config_file {handler arg} {

    set mb_table "MB_InterruptVectorTable"

    set filename [file join "src" "microblaze_interrupts_g.c"]
    file delete $filename
    set config_file [open $filename w]

    ::hsi::utils::write_c_header $config_file "Interrupt Handler Table for MicroBlaze Processor"

    puts $config_file "#include \"microblaze_interrupts_i.h\""
    puts $config_file "#include \"xparameters.h\""
    puts $config_file "\n"
    puts $config_file [format "extern void %s (void *DeviceId);" $handler]
    puts $config_file "\n/*"
    puts $config_file "* The interrupt handler table for microblaze processor"
    puts $config_file "*/\n"
    puts $config_file [format "%sEntry %s\[\] =" $mb_table $mb_table]
    puts $config_file "\{"
    puts -nonewline $config_file [format "\{\t%s" $handler]
    puts -nonewline $config_file [format ",\n\t(void*) %s\}" $arg]
    puts -nonewline $config_file "\n\};"
    puts $config_file "\n"
    close $config_file
}

# -------------------------------------------
# Tcl procedure xcreate_mb_exc_config file
# -------------------------------------------
proc xcreate_mb_exc_config_file {os_handle} {

    set hfilename [file join "src" "microblaze_exceptions_g.h"]
    file delete $hfilename
    set hconfig_file [open $hfilename w]
    ::hsi::utils::write_c_header $hconfig_file "Exception Handling Header for MicroBlaze Processor"
    set sw_proc_handle [hsi::get_sw_processor]
    set hw_proc_handle [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc_handle] ]
    set procvlnv [common::get_property VLNV $hw_proc_handle]
    set procvlnv [split $procvlnv :]
    set procver [lindex $procvlnv 3]
    set ibus_ee [common::get_property CONFIG.C_M_AXI_I_BUS_EXCEPTION $hw_proc_handle]
    set dbus_ee [common::get_property CONFIG.C_M_AXI_D_BUS_EXCEPTION $hw_proc_handle]
    set ill_ee [common::get_property CONFIG.C_ILL_OPCODE_EXCEPTION $hw_proc_handle]
    set unalign_ee [common::get_property CONFIG.C_UNALIGNED_EXCEPTIONS $hw_proc_handle]
    set div0_ee [common::get_property CONFIG.C_DIV_ZERO_EXCEPTION $hw_proc_handle]
    set mmu_ee [common::get_property CONFIG.C_USE_MMU $hw_proc_handle]
    if { $mmu_ee == "" } {
        set mmu_ee 0
    }
    set fsl_ee [common::get_property CONFIG.C_FSL_EXCEPTION $hw_proc_handle]
    if { $fsl_ee == "" } {
        set fsl_ee 0
    }
    if { [mb_has_fpu_exceptions $hw_proc_handle] } {
        set fpu_ee [common::get_property CONFIG.C_FPU_EXCEPTION $hw_proc_handle]
    } else {
        set fpu_ee 0
    }
    set sp_ee [common::get_property CONFIG.C_USE_STACK_PROTECTION $hw_proc_handle]
    if { $sp_ee == "" } {
        set sp_ee 0
    }
    set ft_ee [common::get_property CONFIG.C_FAULT_TOLERANT $hw_proc_handle]
    if { $ft_ee == "" } {
        set ft_ee 0
    }

    if { $ibus_ee == 0 && $dbus_ee == 0 && $ill_ee == 0 && $unalign_ee == 0
         && $div0_ee == 0 && $fpu_ee == 0 && $mmu_ee == 0 && $fsl_ee == 0
         && $sp_ee == 0 && $ft_ee == 0} {
        ;# NO exceptions are enabled
        ;# Do not generate any info in either the header or the C file
        close $hconfig_file
        return
    }

    puts $hconfig_file "\#ifndef MICROBLAZE_EXCEPTIONS_G_H"
    puts $hconfig_file "\#define MICROBLAZE_EXCEPTIONS_G_H"
    puts $hconfig_file "\n"
    puts $hconfig_file "\#ifdef __cplusplus"
    puts $hconfig_file "extern \"C\" {"
    puts $hconfig_file "\#endif"
    puts $hconfig_file "\n"
    puts $hconfig_file "\#define MICROBLAZE_EXCEPTIONS_ENABLED 1"
    if { [mb_can_handle_exceptions_in_delay_slots $procver] } {
        puts $hconfig_file "#define MICROBLAZE_CAN_HANDLE_EXCEPTIONS_IN_DELAY_SLOTS"
    }
    if { $unalign_ee == 0 } {
        puts $hconfig_file "\#define NO_UNALIGNED_EXCEPTIONS 1"
    }
    if { $ibus_ee == 0 && $dbus_ee == 0 && $ill_ee == 0 && $div0_ee == 0
         && $fpu_ee == 0 && $mmu_ee == 0 && $fsl_ee == 0 } {
        ;# NO other exceptions are enabled
        puts $hconfig_file "\#define NO_OTHER_EXCEPTIONS 1"
    }

    if { $fpu_ee != 0 } {
        puts $hconfig_file "\#define MICROBLAZE_FP_EXCEPTION_ENABLED 1"
        set predecode_fpu_exceptions [common::get_property CONFIG.predecode_fpu_exceptions $os_handle]
        if {$predecode_fpu_exceptions != false } {
            puts $hconfig_file "\#define MICROBLAZE_FP_EXCEPTION_DECODE 1"
        }
    }

    puts $hconfig_file "\n"
    puts $hconfig_file "\#ifdef __cplusplus"
    puts $hconfig_file "}"
    puts $hconfig_file "\#endif \n"
    puts $hconfig_file "\#endif"
    puts $hconfig_file "\n"
    close $hconfig_file
}


# -------------------------------------------
# Tcl procedure xcreate_riscv_exc_config file
# -------------------------------------------
proc xcreate_riscv_exc_config_file {os_handle} {

    set hfilename [file join "src" "riscv_exceptions_g.h"]
    file delete $hfilename
    set hconfig_file [open $hfilename w]
    ::hsi::utils::write_c_header $hconfig_file "Exception Handling Header for MicroBlaze RISC-V Processor"
    set sw_proc_handle [hsi::get_sw_processor]
    set hw_proc_handle [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc_handle] ]
    set procvlnv [common::get_property VLNV $hw_proc_handle]
    set procvlnv [split $procvlnv :]
    set procver [lindex $procvlnv 3]

    set fsl_ee [common::get_property CONFIG.C_FSL_EXCEPTION $hw_proc_handle]
    if { $fsl_ee == "" } {
        set fsl_ee 0
    }

    puts $hconfig_file "\#ifndef RISCV_EXCEPTIONS_G_H"
    puts $hconfig_file "\#define RISCV_EXCEPTIONS_G_H"
    puts $hconfig_file "\n"
    puts $hconfig_file "\#ifdef __cplusplus"
    puts $hconfig_file "extern \"C\" {"
    puts $hconfig_file "\#endif"
    puts $hconfig_file "\n"
    puts $hconfig_file "\#define RISCV_EXCEPTIONS_ENABLED 1"

    if { $fsl_ee != 0 } {
        puts $hconfig_file "\#define RISCV_FSL_EXCEPTION 1"
    }

    puts $hconfig_file "\n"
    puts $hconfig_file "\#ifdef __cplusplus"
    puts $hconfig_file "}"
    puts $hconfig_file "\#endif \n"
    puts $hconfig_file "\#endif"
    puts $hconfig_file "\n"
    close $hconfig_file
}


# --------------------------------------
# Tcl procedure post_generate
#
# This proc removes _interrupt_handler.o
# from libxil.a
# --------------------------------------
proc post_generate {os_handle} {

    set sw_proc_handle [hsi::get_sw_processor]
    set hw_proc_handle [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc_handle] ]

    set procname [common::get_property NAME $hw_proc_handle]
    set proctype [common::get_property IP_NAME $hw_proc_handle]

    if {[string compare -nocase $proctype "microblaze"] == 0} {

        set procdrv [hsi::get_sw_processor]
        # Remove _interrupt_handler.o from libxil.a for mb-gcc
        set archiver [common::get_property CONFIG.archiver $procdrv]
        set libgloss_a [file join .. .. lib libgloss.a]
        if { ![file exists $libgloss_a] } {
		set libgloss_a [file join .. .. lib libxil.a]
        }
        exec $archiver -d $libgloss_a _interrupt_handler.o

        # Remove _hw_exception_handler.o from libgloss.a for microblaze_v3_00_a
        if { [mb_has_exceptions $hw_proc_handle] } {
		exec $archiver -d $libgloss_a _hw_exception_handler.o
        }
    }
}

# --------------------------------------
# Return true if this MB has
# exception handling support
# --------------------------------------
proc mb_has_exceptions { hw_proc_handle } {

    # Check if the following parameters exist on this MicroBlaze's MPD
    set ee [common::get_property CONFIG.C_UNALIGNED_EXCEPTIONS $hw_proc_handle]
    if { $ee != "" } {
        return true
    }

    set ee [common::get_property CONFIG.C_ILL_OPCODE_EXCEPTION $hw_proc_handle]
    if { $ee != "" } {
        return true
    }

    set ee [common::get_property CONFIG.C_IOPB_BUS_EXCEPTION $hw_proc_handle]
    if { $ee != "" } {
        return true
    }

    set ee [common::get_property CONFIG.C_DOPB_BUS_EXCEPTION $hw_proc_handle]
    if { $ee != "" } {
        return true
    }

    set ee [common::get_property CONFIG.C_IPLB_BUS_EXCEPTION $hw_proc_handle]
    if { $ee != "" } {
        return true
    }

    set ee [common::get_property CONFIG.C_DPLB_BUS_EXCEPTION $hw_proc_handle]
    if { $ee != "" } {
        return true
    }

    set ee [common::get_property CONFIG.C_M_AXI_I_BUS_EXCEPTION $hw_proc_handle]
    if { $ee != "" } {
        return true
    }

    set ee [common::get_property CONFIG.C_M_AXI_D_BUS_EXCEPTION $hw_proc_handle]
    if { $ee != "" } {
        return true
    }

    set ee [common::get_property CONFIG.C_DIV_BY_ZERO_EXCEPTION $hw_proc_handle]
    if { $ee != "" } {
        return true
    }

    set ee [common::get_property CONFIG.C_DIV_ZERO_EXCEPTION $hw_proc_handle]
    if { $ee != "" } {
        return true
    }

    set ee [common::get_property CONFIG.C_FPU_EXCEPTION $hw_proc_handle]
    if { $ee != "" } {
        return true
    }

    set ee [common::get_property CONFIG.C_FSL_EXCEPTION $hw_proc_handle]
    if { $ee != "" } {
        return true
    }

    set ee [common::get_property CONFIG.C_USE_MMU $hw_proc_handle]
    if { $ee != ""} {
        return true
    }

    set ee [common::get_property CONFIG.C_USE_STACK_PROTECTION $hw_proc_handle]
    if { $ee != ""} {
        return true
    }

    set ee [common::get_property CONFIG.C_FAULT_TOLERANT $hw_proc_handle]
    if { $ee != ""} {
        return true
    }

    return false
}
# --------------------------------------
# Return true if this MB has
# FPU exception handling support
# --------------------------------------
proc mb_has_fpu_exceptions { hw_proc_handle } {

    # Check if the following parameters exist on this MicroBlaze's MPD
    set ee [common::get_property CONFIG.C_FPU_EXCEPTION $hw_proc_handle]
    if { $ee != "" } {
        return true
    }

    return false
}

# --------------------------------------
# Return true if this MB has PVR support
# --------------------------------------
proc mb_has_pvr { hw_proc_handle } {

    # Check if the following parameters exist on this MicroBlaze's MPD
    set pvr [common::get_property CONFIG.C_PVR $hw_proc_handle]
    if { $pvr != "" } {
        return true
    }

    return false
}

# --------------------------------------
# Return true if MB ver 'procver' has
# support for handling exceptions in
# delay slots
# --------------------------------------
proc mb_can_handle_exceptions_in_delay_slots { procver } {

    set procmajorver [lindex [split $procver "."] 0]
    if { [string compare -nocase $procver "5.00.a"] >= 0 || $procmajorver > 5 } {
        return true
    } else {
        return false
    }
}

# --------------------------------------
# Generate Profile Configuration
# --------------------------------------
proc handle_profile { os_handle proctype } {
    global env
    variable scutimer_baseaddr
    variable scutimer_intr
    variable scugic_cpu_base
    variable scugic_dist_base

    set proc [hsi::get_sw_processor]

    if {$proctype == "ps7_cortexa9"} {
        set sw_proc_handle [hsi::get_sw_processor]
        set hw_proc_handle [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc_handle]]
        set cpu_freq [common::get_property CONFIG.C_CPU_CLK_FREQ_HZ $hw_proc_handle]
        if { [string compare -nocase $cpu_freq ""] == 0 } {
            puts "WARNING<profile> :: CPU Clk Frequency not specified, Assuming 666Mhz"
            set cpu_freq 666000000
        }
    } else {
        set cpu_freq [common::get_property CONFIG.C_FREQ  [hsi::get_cells -hier $proc]]
        if { [string compare -nocase $cpu_freq ""] == 0 } {
            puts "WARNING<profile> :: CPU Clk Frequency not specified, Assuming 100Mhz"
            set cpu_freq 100000000
        }
    }
    set filename [file join "src" "profile" "profile_config.h"]
    file delete -force $filename
    set config_file [open $filename w]

    ::hsi::utils::write_c_header $config_file "Profiling Configuration parameters. These parameters
* can be overwritten through run configuration in SDK"
    puts $config_file "#ifndef _PROFILE_CONFIG_H"
    puts $config_file "#define _PROFILE_CONFIG_H\n"

    puts $config_file "#define BINSIZE 4"
    puts $config_file "#define CPU_FREQ_HZ $cpu_freq"
    puts $config_file "#define SAMPLE_FREQ_HZ 100000"
    puts $config_file "#define TIMER_CLK_TICKS [expr $cpu_freq / 100000]"

    # proctype should be "microblaze" or "psu_cortexa9"
    switch $proctype {
        "microblaze" {
            # Microblaze Processor.
            puts $config_file "#define PROC_MICROBLAZE 1"
            set timer_inst [common::get_property CONFIG.profile_timer $os_handle]
            if { [string compare -nocase $timer_inst "none"] == 0 } {
            # Profile Timer Not Selected
                error "ERROR : Timer for Profiling NOT selected.\nS/W Intrusive Profiling on MicroBlaze requires an axi_timer." "" "mdt_error"
            } else {
                handle_profile_opbtimer $config_file $timer_inst
            }
        }
        "ps7_cortexa9" {
	            # Cortex A9 Processor.

	            puts $config_file "#define PROC_CORTEXA9 1"
	            set timer_inst [common::get_property CONFIG.profile_timer $os_handle]
	            if { [string compare -nocase $timer_inst "none"] == 0 } {
	                # SCU Timer
	                puts $config_file "#define ENABLE_SCU_TIMER 1"
	                puts $config_file "#define ENABLE_SYS_INTR 1"
	                puts $config_file "#define PROFILE_TIMER_BASEADDR $scutimer_baseaddr"
	                puts $config_file "#define PROFILE_TIMER_INTR_ID $scutimer_intr"
	                puts $config_file "#define SCUGIC_CPU_BASEADDR $scugic_cpu_base"
	                puts $config_file "#define SCUGIC_DIST_BASEADDR $scugic_dist_base"
	            }
        }
        "default" {error "ERROR: unknown processor type\n"}
    }

    puts $config_file "\n#endif"
    puts $config_file "\n/******************************************************************/\n"
    close $config_file
}

    #***--------------------------------***-----------------------------------***
    # Utility process to call a command and pipe it's output to screen.
    # Used instead of Tcl's exec
proc execpipe {COMMAND} {

  if { [catch {open "| $COMMAND 2>@stdout"} FILEHANDLE] } {
    return "Can't open pipe for '$COMMAND'"
  }

  set PIPE $FILEHANDLE
  fconfigure $PIPE -buffering none

  set OUTPUT ""

  while { [gets $PIPE DATA] >= 0 } {
    append OUTPUT $DATA "\n"
  }

  if { [catch {close $PIPE} ERRORMSG] } {

    if { [string compare "$ERRORMSG" "child process exited abnormally"] == 0 } {
      # this error means there was nothing on stderr (which makes sense) and
      # there was a non-zero exit code - this is OK as we intentionally send
      # stderr to stdout, so we just do nothing here (and return the output)
    } else {
      return "Error '$ERRORMSG' on closing pipe for '$COMMAND'"
    }

  }

  regsub -all -- "\n$" $OUTPUT "" STRIPPED_STRING
  return "$STRIPPED_STRING"

}
# - The xps/opb_timer can be connected directly to Microblaze External Intr Pin.
# - (OR) xps/opb_timer can be connected to xps/opb_intc
proc handle_profile_opbtimer { config_file timer_inst } {
    set timer_handle [hsi::get_cells -hier  $timer_inst]
    set timer_baseaddr [common::get_property CONFIG.C_BASEADDR $timer_handle]
    puts $config_file "#define PROFILE_TIMER_BASEADDR [::hsi::utils::format_addr_string $timer_baseaddr "C_BASEADDR"]"

    # Figure out how Timer is connected.
     set timer_intr [hsi::get_pins -of_objects [hsi::get_cells -hier $timer_handle] Interrupt]
    if { [string compare -nocase $timer_intr ""] == 0 } {
	error "ERROR <profile> :: Timer Interrupt PORT is not specified" "" "mdt_error"
    }
    #set mhs_handle [xget_handle $timer_handle "parent"]
    # CR 302300 - There can be multiple "sink" for the interrupt. So need to iterate through the list
    set intr_port_list [::hsi::utils::get_sink_pins [hsi::get_pins -of_objects [hsi::get_cells -hier $timer_handle] INTERRUPT]]
    set timer_connection 0
    foreach intr_port $intr_port_list {
	set intc_handle [hsi::get_cells -of_objects $intr_port]

	# Check if the Sink is a Concat IP
	if {[common::get_property IP_NAME [hsi::get_cells -hier $intc_handle]] == "xlconcat"} {
		set dout [hsi::get_pins -of_object [hsi::get_cells -hier $intc_handle] -filter DIRECTION=="O"]
		set intr_pins [::hsi::utils::get_sink_pins [hsi::get_pins $dout]]
		for {set intr_pins_index 0} {$intr_pins_index < [llength $intr_pins]} {incr intr_pins_index} {
			set intr_ip [hsi::get_cells -of_objects [lindex $intr_pins $intr_pins_index]]
			for {set intr_ip_index 0} {$intr_ip_index < [llength $intr_ip]} {incr intr_ip_index} {
				if {[common::get_property IP_TYPE [hsi::get_cells -hier [lindex $intr_ip $intr_ip_index]]] == "INTERRUPT_CNTLR"} {
					set intc_handle [hsi::get_cells -hier [lindex $intr_ip $intr_ip_index]]
					break;
				}
			}
		}
	}
	# Check if the Sink is a Global Port. If so, Skip the Port Connection

	if {  [::hsi::utils::is_external_pin $intr_port] } {
	    continue
	}
	set iptype [common::get_property CONFIG.EDK_IPTYPE $intc_handle]
	if { [string compare -nocase $iptype "PROCESSOR"] == 0 } {
	    # Timer Directly Connected to the Processor
	    puts $config_file "#define ENABLE_SYS_INTR 1"
	    set timer_connection 1
	    break
	}

	set ipsptype [common::get_property CONFIG.EDK_SPECIAL $intc_handle]
	if { [string compare -nocase $iptype "PERIPHERAL"] == 0  &&
	     [string compare -nocase $ipsptype "INTR_CTRL"] == 0 } {
	    # Timer connected to Interrupt controller
	    puts $config_file "#define TIMER_CONNECT_INTC 1"
	    puts $config_file "#define INTC_BASEADDR [common::get_property CONFIG.C_BASEADDR $intc_handle]"
	    set num_intr_inputs [common::get_property CONFIG.C_NUM_INTR_INPUTS $intc_handle]
	    # if { $num_intr_inputs == 1 } {  ## Always enable system interrupt CR 472288
		 puts $config_file "#define ENABLE_SYS_INTR 1"
	    # }

	    #set signals [split [xget_value $intr_port "VALUE"] "&"]
            set signals [::hsi::utils::get_source_pins $intr_port]
	    set i 1
	    foreach signal $signals {
		set signal [string trim $signal]
		if {[string compare -nocase $signal $timer_intr] == 0} {
		    set timer_id [expr ($num_intr_inputs - $i)]
		    set timer_mask [expr 0x1 << $timer_id]
		    puts $config_file "#define PROFILE_TIMER_INTR_ID $timer_id"
		    puts $config_file "#define PROFILE_TIMER_INTR_MASK [format "0x%x" $timer_mask]"
		    break
		}
		incr i
	    }
	    set timer_connection 1
	    break
	}
    }

    if { $timer_connection == 0 } {
	error "ERROR <profile> :: Profile Timer Interrupt Signal Not Connected Properly"
    }
}

#
# Handle the stdout parameter of a processor
#
proc handle_stdout_parameter {drv_handle} {
   set stdout [common::get_property CONFIG.stdout $drv_handle]
   set sw_proc_handle [::hsi::get_sw_processor]
   set hw_proc_handle [::hsi::get_cells -hier [common::get_property hw_instance $sw_proc_handle]]
   set processor [common::get_property NAME $hw_proc_handle]

   if {[llength $stdout] == 1 && [string compare -nocase "none" $stdout] != 0} {

       set stdout_drv_handle [::hsi::get_drivers -filter "HW_INSTANCE==$stdout"]
       if {[llength $stdout_drv_handle] == 0} {
           error "No driver for stdout peripheral $stdout. Check the following reasons: \n
                  1. $stdout is not accessible from processor $processor.\n
                  2. No Driver block is defined for $stdout in MSS file." "" "hsi_error"
           return
       }

       set interface_handle [::hsi::get_sw_interfaces -of_objects $stdout_drv_handle -filter "NAME==stdout"]
       if {[llength $interface_handle] == 0} {
         error "No stdout interface available for driver for peripheral $stdout" "" "hsi_error"
       }
       set outbyte_name [common::get_property FUNCTION.outbyte $interface_handle]
       if {[llength $outbyte_name] == 0} {
         error "No outbyte function available for driver for peripheral $stdout" "" "hsi_error"
       }
       set header [common::get_property PROPERTY.header $interface_handle]
       if {[llength $header] == 0} {
         error "No header property available in stdout interface for driver for peripheral $stdout" "" "hsi_error"
       }
       set config_file [open "src/outbyte.c" w]
       puts $config_file "\#include \"xparameters.h\""
       puts $config_file [format "\#include \"%s\"\n" $header ]
       puts $config_file "\#ifdef __cplusplus"
       puts $config_file "extern \"C\" {"
       puts $config_file "\#endif"
       puts $config_file "void outbyte(char c); \n"
       puts $config_file "\#ifdef __cplusplus"
       puts $config_file "}"
       puts $config_file "\#endif \n"
       puts $config_file "\#ifndef VERSAL_PLM"
       puts $config_file "void outbyte(char c)"
       puts $config_file "{"
       puts $config_file [format "\t %s(STDOUT_BASEADDRESS, c);" $outbyte_name]
       puts $config_file "}"
       puts $config_file "\#endif"
       close $config_file
       set config_file [::hsi::utils::open_include_file "xparameters.h"]
       set stdout_mem_range [::hsi::get_mem_ranges -of_objects $hw_proc_handle -filter "INSTANCE==$stdout && IS_DATA==1" ]
       if { [llength $stdout_mem_range] > 1 } {
           set stdout_mem_range [::hsi::get_mem_ranges -of_objects $hw_proc_handle -filter "INSTANCE==$stdout&& (BASE_NAME==C_BASEADDR||BASE_NAME==C_S_AXI_BASEADDR)"]
       }
       set base_name [common::get_property BASE_NAME $stdout_mem_range]
       set base_value [lindex [common::get_property BASE_VALUE $stdout_mem_range] 0]
       puts $config_file "\#define STDOUT_BASEADDRESS [::hsi::utils::format_addr_string $base_value $base_name]"
       close $config_file
   } else {
            if { $stdout == "" || $stdout == "none" } {
                    #
                    # UART is not present in the system, add dummy implementation for outbyte
                    #
                    set config_file [open "src/outbyte.c" w]
		    puts $config_file "\#include \"xparameters.h\""
		    puts $config_file "\#ifdef __cplusplus"
		    puts $config_file "extern \"C\" {"
		    puts $config_file "\#endif"
		    puts $config_file "void outbyte(char c); \n"
		    puts $config_file "\#ifdef __cplusplus"
		    puts $config_file "}"
		    puts $config_file "\#endif \n"
		    puts $config_file "\#ifndef VERSAL_PLM"
		    puts $config_file "void outbyte(char c)"
		    puts $config_file "{"
		    puts $config_file "    (void) c;"
		    puts $config_file "}"
		    puts $config_file "\#endif"
                    close $config_file
            }
     }
}

#
# Handle the stdin parameter of a processor
#
proc handle_stdin_parameter {drv_handle} {

   set stdin [common::get_property CONFIG.stdin $drv_handle]
   set sw_proc_handle [::hsi::get_sw_processor]
   set hw_proc_handle [::hsi::get_cells -hier [common::get_property hw_instance $sw_proc_handle]]

   set processor [common::get_property hw_instance $sw_proc_handle]
   if {[llength $stdin] == 1 && [string compare -nocase "none" $stdin] != 0} {
       set stdin_drv_handle [::hsi::get_drivers -filter "HW_INSTANCE==$stdin"]
       if {[llength $stdin_drv_handle] == 0} {
           error "No driver for stdin peripheral $stdin. Check the following reasons: \n
                  1. $stdin is not accessible from processor $processor.\n
                  2. No Driver block is defined for $stdin in MSS file." "" "hsi_error"
           return
       }

       set interface_handle [::hsi::get_sw_interfaces -of_objects $stdin_drv_handle -filter "NAME==stdin"]
       if {[llength $interface_handle] == 0} {
           error "No stdin interface available for driver for peripheral $stdin" "" "hsi_error"
       }

       set inbyte_name [common::get_property FUNCTION.inbyte $interface_handle ]
       if {[llength $inbyte_name] == 0} {
         error "No inbyte function available for driver for peripheral $stdin" "" "hsi_error"
       }
       set header [common::get_property PROPERTY.header $interface_handle]
       if {[llength $header] == 0} {
         error "No header property available in stdin interface for driver for peripheral $stdin" "" "hsi_error"
       }
       set config_file [open "src/inbyte.c" w]
       puts $config_file "\#include \"xparameters.h\""
       puts $config_file [format "\#include \"%s\"\n" $header]
       puts $config_file "\#ifdef __cplusplus"
       puts $config_file "extern \"C\" {"
       puts $config_file "\#endif"
       puts $config_file "char inbyte(void);"
       puts $config_file "\#ifdef __cplusplus"
       puts $config_file "}"
       puts $config_file "\#endif \n"
       puts $config_file "char inbyte(void) {"
       puts $config_file [format "\t return %s(STDIN_BASEADDRESS);" $inbyte_name]
       puts $config_file "}"
       close $config_file
       set config_file [::hsi::utils::open_include_file "xparameters.h"]
       set stdin_mem_range [::hsi::get_mem_ranges -of_objects $hw_proc_handle -filter "INSTANCE==$stdin && IS_DATA==1"]
       if { [llength $stdin_mem_range] > 1 } {
           set stdin_mem_range [::hsi::get_mem_ranges -of_objects $hw_proc_handle -filter "INSTANCE==$stdin&& (BASE_NAME==C_BASEADDR||BASE_NAME==C_S_AXI_BASEADDR)"]
       }
       set base_name [common::get_property BASE_NAME $stdin_mem_range]
       set base_value [common::get_property BASE_VALUE $stdin_mem_range]
       puts $config_file "\#define STDIN_BASEADDRESS [::hsi::utils::format_addr_string $base_value $base_name]"
       close $config_file
   } else {
            if { $stdin == "" || $stdin == "none" } {
                    #
                    # UART is not present in the system, add dummy implementation for inbyte
                    #
                    set config_file [open "src/inbyte.c" w]
                    puts $config_file "\#include \"xparameters.h\""
                    puts $config_file "\#ifdef __cplusplus"
                    puts $config_file "extern \"C\" {"
                    puts $config_file "\#endif"
                    puts $config_file "char inbyte(void);"
                    puts $config_file "\#ifdef __cplusplus"
                    puts $config_file "}"
                    puts $config_file "\#endif \n"
                    puts $config_file "char inbyte(void) {"
                    puts $config_file "    return (0);"
                    puts $config_file "}"
                    close $config_file
            }
     }
}

proc xdefine_fabric_reset {file_handle} {
	puts $file_handle ""
	puts $file_handle "/* Number of Fabric Resets */"
	set periph_list [get_cells -hier]
	foreach periph $periph_list {
		set zynq_ultra_ps [get_property IP_NAME $periph]
		if {[string match -nocase $zynq_ultra_ps "zynq_ultra_ps_e"] } {
			set avail_param [list_property [get_cells -hier $periph]]
			if {[lsearch -nocase $avail_param "CONFIG.C_NUM_FABRIC_RESETS"] >= 0} {
				set nr_rst [get_property CONFIG.C_NUM_FABRIC_RESETS [get_cells -hier $periph]]
				puts $file_handle "#define XPAR_NUM_FABRIC_RESETS $nr_rst"
			}
		}
	}
	puts $file_handle ""
}
