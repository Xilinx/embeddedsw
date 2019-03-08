##############################################################################
#
# Copyright (C) 2014 - 2019 Xilinx, Inc. All rights reserved.
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
###############################################################################
###############################################################################
#
# MODIFICATION HISTORY:
#
# Ver   Who  Date     Changes
# ----- ---- -------- ---------------------------------------------------
# 6.4   ms   05/23/17 Defined PSU_PMU macro in xparameters.h to support
#                     XGetPSVersion_Info function for PMUFW.
# 6.6   srm  10/18/17 Added xsleep_timer_config function to support the
#                     sleep configuration using timers as specifed by the
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
#
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

# --------------------------------------
# Tcl procedure standalone_drc
# -------------------------------------
proc standalone_drc {os_handle} {
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
				# We found a master thats cache coherent, so return true
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

    set need_config_file "false"
    # Copy over the right set of files as src based on processor type
    set sw_proc_handle [hsi::get_sw_processor]
    set hw_proc_handle [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_proc_handle] ]
    set proctype [common::get_property IP_NAME $hw_proc_handle]
    set procname [common::get_property NAME    $hw_proc_handle]
    set boardname [common::get_property BOARD [hsi::current_hw_design]]
    set enable_sw_profile [common::get_property CONFIG.enable_sw_intrusive_profiling $os_handle]
    set mb_exceptions false

    # proctype should be "microblaze" or psu_cortexa53 or psu_cortexr5 or ps7_cortexa9
    set mbsrcdir "./src/microblaze"
    set cortexa53srcdir "./src/arm/cortexa53"
    set cortexr5srcdir "./src/arm/cortexr5"
    set cortexa9srcdir "./src/arm/cortexa9"
    set procdrv [hsi::get_sw_processor]
    set commonsrcdir "./src/common"
    set armcommonsrcdir "./src/arm/common"
    set armsrcdir "./src/arm"

    foreach entry [glob -nocomplain [file join $commonsrcdir *]] {
        file copy -force $entry "./src"
    }
    if { $proctype == "psu_cortexa53" || $proctype == "psu_cortexa72" || $proctype == "ps7_cortexa9" || $proctype == "psu_cortexr5" || $proctype == "psv_cortexr5" || $proctype == "psv_cortexa72"} {
        set compiler [common::get_property CONFIG.compiler $procdrv]
        foreach entry [glob -nocomplain [file join $armcommonsrcdir *]] {
            file copy -force $entry "./src"
            file delete -force "./src/gcc"
            file delete -force "./src/iccarm"
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
        }

    }
   
    set cortexa72proc [hsi::get_cells -hier -filter {IP_NAME=="psu_cortexa72" || IP_NAME=="psv_cortexa72"}]



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
        }
	"psu_pmc" -
	"psv_pmc"
	{
	    foreach entry [glob -nocomplain [file join $mbsrcdir *]] {
                # Copy over only files that are not related to exception handling. All such files have exception in their names
                file copy -force $entry "./src/"
            }
            set need_config_file "true"
            set mb_exceptions [mb_has_exceptions $hw_proc_handle]
        }
	"psu_psm" -
	"psv_psm"
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
	"psu_cortexa72"
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
		    set platformsrcdir "./src/arm/ARMv8/64bit/platform/versal"
		 }
	        set pvconsoledir "./src/arm/ARMv8/64bit/xpvxenconsole"
	        set hypervisor_guest [common::get_property CONFIG.hypervisor_guest $os_handle ]
	        if { $hypervisor_guest == "true" } {
	             foreach entry [glob -nocomplain [file join $pvconsoledir *]] {
			file copy -force $entry "./src/"
		     }
		}
	    }

	    set includedir "./src/arm/ARMv8/includes_ps"
            foreach entry [glob -nocomplain [file join $cortexa53srcdir1 *]] {
                file copy -force $entry "./src/"
            }
            foreach entry [glob -nocomplain [file join $ccdir *]] {
                file copy -force $entry "./src/"
            }
	    foreach entry [glob -nocomplain [file join $platformsrcdir *]] {
	    	file copy -force $entry "./src/"
	    }
	    file delete -force $platformsrcdir     
	    file copy -force $includedir "./src/"
            file delete -force "./src/gcc"
	    file delete -force "./src/armclang"
            file delete -force "./src/profile"
	    file delete -force "./src/xpvxenconsole"
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
	"psv_cortexr5"
	{
	    set procdrv [hsi::get_sw_processor]
	    set includedir "./src/arm/ARMv8/includes_ps"
	    if {[string compare -nocase $compiler "iccarm"] == 0} {
	           set ccdir "./src/arm/cortexr5/iccarm"
            } else {
	           set ccdir "./src/arm/cortexr5/gcc"
	   }
	    foreach entry [glob -nocomplain [file join $cortexr5srcdir *]] {
		file copy -force $entry "./src/"
	    }
	    foreach entry [glob -nocomplain [file join $ccdir *]] {
		file copy -force $entry "./src/"
	    }
	    
	    if {[llength $cortexa72proc] == 0} {
	        file copy -force $includedir "./src/"
	    }
	    file delete -force "./src/gcc"
	    file delete -force "./src/iccarm"
	    file delete -force "./src/profile"
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
	    if {[llength $cortexa72proc] > 0} { 
		set platformsrcdir "./src/arm/cortexr5/platform/versal"
	    } else {
	        set platformsrcdir "./src/arm/cortexr5/platform/ZynqMP"
	    }

	    foreach entry [glob -nocomplain [file join $platformsrcdir *]] {
		file copy -force $entry "./src/"
	    }
            file delete -force $platformsrcdir   
	    
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
                set psu_ddr_1_baseaddr [common::get_property CONFIG.C_S_AXI_BASEADDR [hsi::get_cells -hier "psu_ddr_1"]]
                set psu_ddr_1_highaddr [common::get_property CONFIG.C_S_AXI_HIGHADDR [hsi::get_cells -hier "psu_ddr_1"]]
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
                   foreach entry [glob -nocomplain [file join $cortexa9srcdir *]] {
                       file copy -force $entry "./src/"
                   }
                   foreach entry [glob -nocomplain [file join $ccdir *]] {
                       file copy -force $entry "./src/"
                   }
                       file delete -force "./src/armcc"
                       file delete -force "./src/gcc"
			file delete -force "./src/iccarm"
                   if {[string compare -nocase $compiler "armcc"] == 0} {
                       file delete -force "./src/profile"
                       set enable_sw_profile "false"
	    }
		if {[string compare -nocase $compiler "iccarm"] == 0} {
                           file delete -force "./src/profile"
                           set enable_sw_profile "false"
                   }
                   set file_handle [::hsi::utils::open_include_file "xparameters.h"]
                   puts $file_handle "#include \"xparameters_ps.h\""
                   puts $file_handle ""
                   close $file_handle
        }
        "default" {puts "unknown processor type $proctype\n"}
    }

    # Write the Config.make file
    set makeconfig [open "./src/config.make" w]
#    print_generated_header_tcl $makeconfig "Configuration parameters for Standalone Makefile"
    if { $proctype == "microblaze" || $proctype == "psu_pmu" || $proctype == "psu_psm" || $proctype == "psu_pmc" || $proctype == "psv_psm" || $proctype == "psv_pmc" } {
        puts $makeconfig "LIBSOURCES = *.c *.S"
        puts $makeconfig "PROFILE_ARCH_OBJS = profile_mcount_mb.o"
    } elseif { $proctype == "psu_cortexr5" ||  $proctype == "psv_cortexr5" } {
	puts $makeconfig "LIBSOURCES = *.c *.S"
    } elseif { $proctype == "psu_cortexa53" || $proctype == "psu_cortexa72" || $proctype == "psv_cortexa72"}  {
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
    } else {
        error "ERROR: processor $proctype is not supported"
    }
    if { $enable_sw_profile == "true" } {
        puts $makeconfig "LIBS = standalone_libs profile_libs"
    } else {
        puts $makeconfig "LIBS = standalone_libs"
    }
    close $makeconfig

    # Remove microblaze,  cortexr5, cortexa53 and common directories...
    file delete -force $mbsrcdir
    file delete -force $commonsrcdir
    file delete -force $armsrcdir

    # Handle stdin
    set stdin [common::get_property CONFIG.stdin $os_handle]
    if { $stdin == "" || $stdin == "none" } {
            handle_stdin_parameter $os_handle
    } else {
            ::hsi::utils::handle_stdin $os_handle
    }

    # Handle stdout
    set stdout [common::get_property CONFIG.stdout $os_handle]
    if { $stdout == "" || $stdout == "none" } {
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

    # Create bspconfig file
    set bspcfg_fn [file join "src" "bspconfig.h"]
    file delete $bspcfg_fn
    set bspcfg_fh [open $bspcfg_fn w]
    ::hsi::utils::write_c_header $bspcfg_fh "Configurations for Standalone BSP"
    puts $bspcfg_fh "#ifndef BSPCONFIG_H  /* prevent circular inclusions */"
    puts $bspcfg_fh "#define BSPCONFIG_H  /* by using protection macros */"
    puts $bspcfg_fh ""

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
    } elseif { $proctype == "psu_cortexa72" || $proctype == "psv_cortexa72"} {
                set extra_flags [common::get_property CONFIG.extra_compiler_flags [hsi::get_sw_processor]]
                set flagindex [string first {-DARMA72_EL3} $extra_flags 0]
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

    if {[llength $cortexa72proc] > 0} {
	puts $file_handle "#ifndef versal"
        puts $file_handle "#define versal"
        puts $file_handle "#endif"
        puts $file_handle ""
    } elseif { $proctype == "psu_cortexr5"} {
	 set lockstep_debug [common::get_property CONFIG.lockstep_mode_debug $os_handle]
	 puts $file_handle " "
	 puts $file_handle "/* Definitions for debug logic configuration in lockstep mode */"
	 if { $lockstep_debug == "true" } {
		puts $file_handle "#define LOCKSTEP_MODE_DEBUG 1U"
	 } else {
		puts $file_handle "#define LOCKSTEP_MODE_DEBUG 0U"
	 }
     }
	 puts $file_handle " "
	 puts $file_handle "/* Definitions for sleep timer configuration */"
	 xsleep_timer_config $proctype $os_handle $file_handle
	 puts $file_handle " "
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
	if { $sleep_timer == "ps7_globaltimer_0" || $sleep_timer == "psu_iou_scntr" || $sleep_timer == "psu_iou_scntrs" || $sleep_timer == "psv_iou_scntr" || $sleep_timer == "psv_iou_scntrs"} {
		if { $proctype == "psu_cortexr5" ||  $proctype == "psv_cortexr5"} {
			error "ERROR: $proctype does not support $sleep_timer "
		}
    } elseif { $sleep_timer == "none" } {
		if { $proctype == "psu_cortexr5" || $proctype == "psv_cortexr5" } {
			set periphs [hsi::get_cells -hier]
			foreach periph $periphs {
				if {[string compare -nocase "psu_ttc_3" $periph] == 0} {
					puts $file_handle "#define SLEEP_TIMER_BASEADDR XPAR_PSU_TTC_9_BASEADDR"
					puts $file_handle "#define SLEEP_TIMER_FREQUENCY XPAR_PSU_TTC_9_TTC_CLK_FREQ_HZ"
					puts $file_handle "#define XSLEEP_TTC_INSTANCE 3"
				} elseif {[string compare -nocase "psv_ttc_3" $periph] == 0} {
					puts $file_handle "#define SLEEP_TIMER_BASEADDR XPAR_PSV_TTC_9_BASEADDR"
					puts $file_handle "#define SLEEP_TIMER_FREQUENCY XPAR_PSV_TTC_9_TTC_CLK_FREQ_HZ"
					puts $file_handle "#define XSLEEP_TTC_INSTANCE 3"
				}
			}
		}
		puts $file_handle "#define XSLEEP_TIMER_IS_DEFAULT_TIMER"
    } elseif {[string match "axi_timer_*" $sleep_timer]} {
		if { $proctype == "microblaze" } {
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
			puts $file_handle "#define SLEEP_TIMER_BASEADDR [format "XPAR_PSU_TTC_%d_BASEADDR" [ expr 3 * $module + $timer ] ] "
			puts $file_handle "#define SLEEP_TIMER_FREQUENCY [format "XPAR_PSU_TTC_%d_TTC_CLK_FREQ_HZ" [ expr 3 * $module + $timer ] ] "
		} elseif {$proctype == "psv_cortexr5" || $proctype == "psv_cortexa72"} {
			puts $file_handle "#define SLEEP_TIMER_BASEADDR [format "XPAR_PSV_TTC_%d_BASEADDR" [ expr 3 * $module + $timer ] ] "
			puts $file_handle "#define SLEEP_TIMER_FREQUENCY [format "XPAR_PSV_TTC_%d_TTC_CLK_FREQ_HZ" [ expr 3 * $module + $timer ] ] "
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
    puts $config_file [format "extern void %s (void *);" $handler]
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
* can be overwritten thru run configuration in SDK"
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
       puts $config_file "void outbyte(char c) {"
       puts $config_file [format "\t %s(STDOUT_BASEADDRESS, c);" $outbyte_name]
       puts $config_file "}"
       close $config_file
       set config_file [::hsi::utils::open_include_file "xparameters.h"]
       set stdout_mem_range [::hsi::get_mem_ranges -of_objects $hw_proc_handle -filter "INSTANCE==$stdout && IS_DATA==1" ]
       if { [llength $stdout_mem_range] > 1 } {
           set stdout_mem_range [::hsi::get_mem_ranges -of_objects $hw_proc_handle -filter "INSTANCE==$stdout&& (BASE_NAME==C_BASEADDR||BASE_NAME==C_S_AXI_BASEADDR)"]
       }
       set base_name [common::get_property BASE_NAME $stdout_mem_range]
       set base_value [common::get_property BASE_VALUE $stdout_mem_range]
       puts $config_file "\#define STDOUT_BASEADDRESS [::hsi::utils::format_addr_string $base_value $base_name]"
       close $config_file
   } else {
            if { $stdout == "" || $stdout == "none" } {
                    #
                    # UART is not present in the system, add dummy implementatin for outbyte
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
		    puts $config_file "void outbyte(char c) {"
		    puts $config_file "}"
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
                    # UART is not present in the system, add dummy implementatin for inbyte
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
