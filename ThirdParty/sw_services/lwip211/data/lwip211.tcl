###############################################################################
# Copyright (c) 2007 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################

set use_axieth_on_zynq 0
set use_emaclite_on_zynq 0

# emaclite hw requirements - interrupt needs to be connected
proc lwip_elite_hw_drc {libhandle emac} {
	set intr_port [hsi::get_pins -of_objects [hsi::get_cells -hier $emac] IP2INTC_Irpt]
	if {[string compare -nocase $intr_port ""] == 0} {
		error "ERROR: xps_ethernetlite core $emacname does not have its interrupt connected to interrupt controller. \
			lwIP operates only in interrupt mode, so please connect the interrupt port to \
			the interrupt controller.\n" "" "MDT_ERROR"
	}
	set tx_csum [common::get_property CONFIG.tcp_tx_checksum_offload $libhandle]
	set rx_csum [common::get_property CONFIG.tcp_rx_checksum_offload $libhandle]

	set tx_full_csum [common::get_property CONFIG.tcp_ip_tx_checksum_offload $libhandle]
	set rx_full_csum [common::get_property CONFIG.tcp_ip_rx_checksum_offload $libhandle]

	set igmp_val [common::get_property CONFIG.igmp_options $libhandle]

	if {$tx_full_csum} {
		error "ERROR: Full TCP/IP checksum offload on Tx path is not supported for emaclite" "" "MDT_ERROR"
	}

	if {$rx_full_csum} {
		error "ERROR: Full TCP/IP checksum offload on Rx path is not supported for emaclite" "" "MDT_ERROR"
	}
	if {$tx_csum} {
		error "ERROR: Partial TCP checksum offload on Tx path is not supported for emaclite" "" "MDT_ERROR"
	}
	if {$rx_csum} {
		error "ERROR: Partial TCP checksum offload on Rx path is not supported for emaclite" "" "MDT_ERROR"
	}
	if {$igmp_val == true} {
		error "ERROR: IGMP is not supported for emaclite" "" "MDT_ERROR"
	}
}

# temac support requires the following interrupts to be connected
#	- temac intr
#	- if sdma, sdma tx&rx intr (not verified)
#	- if fifo, fifo intr (not verified)
proc lwip_temac_channel_hw_drc {libhandle emac irpt_name llink_name tx_csum_name rx_csum_name} {
	set emacname [common::get_property NAME $emac]
	set mhs_handle [hsi::get_cells -hier $emac]

	set intr_port [hsi::get_pins -of_objects [hsi::get_cells -hier $emac] irpt_name]
	if {[string compare -nocase $intr_port ""] == 0} {
		error "ERROR: xps_ll_temac core $emacname does not have its interrupt connected to interrupt controller. \
			lwIP requires that this interrupt line be connected to \
			the interrupt controller.\n" "" "MDT_ERROR"
	}

	# find out what is connected to llink0
	set connected_bus [xget_hw_busif_handle $emac $llink_name]
	if {[string compare -nocase $intr_port ""] == 0} {
                error "ERROR: xps_ll_temac core $emacname does not have its local link port (LLINK0) connected" \
                    "" "MDT_ERROR"
        }

	set connected_bus_name [common::get_property NAME $connected_bus]
	set target_handle [xget_hw_connected_busifs_handle $mhs_handle $connected_bus_name "TARGET"]
	set parent_handle [hsi::get_cells -of_objects  $target_handle]
	set parent_name [common::get_property NAME $parent_handle]

	set tx_csum [common::get_property CONFIG.tcp_tx_checksum_offload $libhandle]
	set rx_csum [common::get_property CONFIG.tcp_rx_checksum_offload $libhandle]

	set tx_full_csum [common::get_property CONFIG.tcp_ip_tx_checksum_offload $libhandle]
	set rx_full_csum [common::get_property CONFIG.tcp_ip_rx_checksum_offload $libhandle]


	if {$tx_full_csum} {
		error "ERROR: Full TCP/IP checksum offload on Tx path is not supported for xps-ll-temac" "" "MDT_ERROR"
	}

	if {$rx_full_csum} {
		error "ERROR: Full TCP/IP checksum offload on Rx path is not supported for xps-ll-temac" "" "MDT_ERROR"
	}

	# check checksum parameters
	if {$tx_csum} {
		if {$parent_name == "xps_ll_fifo"} {
			error "ERROR: Checksum offload is possible only with a DMA engine" "" "MDT_ERROR"
		}
		set hw_tx_csum [common::get_property CONFIG.$tx_csum_name $emac]
		if {!$hw_tx_csum} {
			error "ERROR: lwIP cannot offload TX checksum calculation since hardware \
				does not support TX checksum offload" "" "MDT_ERROR"
		}
	}

	if {$rx_csum} {
		if {$parent_name == "xps_ll_fifo"} {
			error "ERROR: Checksum offload is possible only with a DMA engine" "" "MDT_ERROR"
		}
		set hw_rx_csum [common::get_property CONFIG.$rx_csum_name $emac]
		if {!$hw_rx_csum} {
			error "ERROR: lwIP cannot offload RX checksum calculation since hardware \
				does not support RX checksum offload" "" "MDT_ERROR"
		}
	}
}

proc lwip_temac_hw_drc {libhandle emac} {
	set emac_intr_port [hsi::get_pins -of_objects [hsi::get_cells -hier $emac] TemacIntc0_Irpt]
	set emac0_enabled "0"
	if {$emac_intr_port != ""} { set emac0_enabled "1" }
	if {$emac0_enabled == "1"} {
		lwip_temac_channel_hw_drc $libhandle $emac "TemacIntc0_Irpt" "LLINK0" "C_TEMAC0_TXCSUM" "C_TEMAC0_RXCSUM"
	}

	set emac1_enabled [common::get_property CONFIG.C_TEMAC1_ENABLED $emac]
	if {$emac1_enabled == "1"} {
		lwip_temac_channel_hw_drc $libhandle $emac "TemacIntc1_Irpt" "LLINK1" "C_TEMAC1_TXCSUM" "C_TEMAC1_RXCSUM"
	}
}

# AXI temac support requires the following interrupts to be connected
#	- temac intr
#	- if AXI DMA, dma tx&rx intr (not verified)
#	- if fifo, fifo intr (not verified)
proc lwip_axi_ethernet_hw_drc {libhandle emac} {

	set emacname [common::get_property IP_NAME [hsi::get_cells -hier $emac]]
	set mhs_handle [hsi::get_cells -hier $emac]

	set intr_port [hsi::get_pins -of_objects [hsi::get_cells -hier $emac] INTERRUPT]
	if {[string compare -nocase $intr_port ""] == 0} {
		error "ERROR: axi_ethernet core $emacname does not have its interrupt connected to interrupt controller. \
			lwIP requires that this interrupt line be connected to \
			the interrupt controller.\n" "" "MDT_ERROR"
	}

	# find out what is connected to AXI
	set target_periph_type [axieth_target_periph $emac]
	set tx_csum [common::get_property CONFIG.tcp_tx_checksum_offload $libhandle]
	set rx_csum [common::get_property CONFIG.tcp_rx_checksum_offload $libhandle]

	set tx_full_csum [common::get_property CONFIG.tcp_ip_tx_checksum_offload $libhandle]
	set rx_full_csum [common::get_property CONFIG.tcp_ip_rx_checksum_offload $libhandle]

	if {$tx_csum && $tx_full_csum} {
		error "ERROR: Both partial and full checksums on Tx path can not be enabled at the same time" "" "MDT_ERROR"
	}

	if {$rx_csum && $rx_full_csum} {
		error "ERROR: Both partial and full checksums on Rx path can not be enabled at the same time" "" "MDT_ERROR"
	}

	# check checksum parameters
	if {$tx_csum || $tx_full_csum} {
		if {$target_periph_type == "axi_fifo_mm_s"} {
			error "ERROR: Checksum offload is possible only with a DMA engine" "" "MDT_ERROR"
		}

		if {$emacname == "axi_ethernet_buffer" } {
			set hw_tx_csum [common::get_property CONFIG.C_TXCSUM $mhs_handle]
		} else {
			set hw_tx_csum [common::get_property CONFIG.TXCSUM $mhs_handle]
			set hw_tx_csum [get_checksum $hw_tx_csum]
		}

		if {$hw_tx_csum == "1" &&  $tx_full_csum } {
			error "ERROR: lwIP cannot offload full TX checksum calculation since hardware \
				supports partial TX checksum offload" "" "MDT_ERROR"
		}
		if {!$hw_tx_csum &&  $tx_full_csum } {
			error "ERROR: lwIP cannot offload full TX checksum calculation since hardware \
				does not support TX checksum offload" "" "MDT_ERROR"
		}
		if {$hw_tx_csum == "2" &&  $tx_csum } {
			error "ERROR: lwIP cannot offload partial TX checksum calculation since hardware \
				supports full TX checksum offload" "" "MDT_ERROR"
		}
		if {!$hw_tx_csum && $tx_csum } {
			error "ERROR: lwIP cannot offload partial TX checksum calculation since hardware \
				does not support partial TX checksum offload" "" "MDT_ERROR"
		}
	}

	if {$rx_csum || $rx_full_csum} {
		if {$target_periph_type == "axi_fifo_mm_s"} {
			error "ERROR: Checksum offload is possible only with a DMA engine" "" "MDT_ERROR"
		}
		if {$emacname == "axi_ethernet_buffer" } {
			set hw_rx_csum [common::get_property CONFIG.C_RXCSUM $mhs_handle]
		} else {
			set hw_rx_csum [common::get_property CONFIG.RXCSUM $mhs_handle]
			set hw_rx_csum [get_checksum $hw_rx_csum]
		}

		if {$hw_rx_csum == "1" &&  $rx_full_csum } {
			error "ERROR: lwIP cannot offload full RX checksum calculation since hardware \
				supports partial RX checksum offload" "" "MDT_ERROR"
		}
		if {!$hw_rx_csum &&  $rx_full_csum } {
			error "ERROR: lwIP cannot offload full RX checksum calculation since hardware \
				does not support RX checksum offload" "" "MDT_ERROR"
		}
		if {$hw_rx_csum == "2" &&  $rx_csum } {
			error "ERROR: lwIP cannot offload partial RX checksum calculation since hardware \
				supports full RX checksum offload" "" "MDT_ERROR"
		}
		if {!$hw_rx_csum && $rx_csum } {
			error "ERROR: lwIP cannot offload partial RX checksum calculation since hardware \
				does not support partial RX checksum offload" "" "MDT_ERROR"
		}
	}

}

#---
# perform basic sanity checks:
#	- interrupts are connected
# 	- for csum offload, sdma should be present
#--
proc lwip_hw_drc {libhandle emacs_list} {
	foreach ip $emacs_list {
		set iptype [common::get_property IP_NAME [get_cells -hier $ip]]
		if {$iptype == "xps_ethernetlite" || $iptype == "axi_ethernetlite"} {
			lwip_elite_hw_drc $libhandle $ip
		} elseif {$iptype == "xps_ll_temac"} {
			lwip_temac_hw_drc $libhandle $ip
		} elseif {$iptype == "axi_ethernet" || $iptype == "axi_ethernet_buffer"} {
			lwip_axi_ethernet_hw_drc $libhandle $ip
		}
	}
}

#--------
# Check the following s/w requirements for lwIP:
#	1. in SOCKET API is used, then xilkernel is required
#--------
proc lwip_sw_drc {libhandle emacs_list} {
	set api_mode [common::get_property CONFIG.api_mode $libhandle]
	set api_mode [string toupper $api_mode]
	if { [string compare -nocase "SOCKET_API" $api_mode] == 0} {
		set sw_proc_handle [hsi::get_sw_processor]
		set os_handle [hsi::get_os]
		set os_name [common::get_property NAME $os_handle]
		if { [string compare -nocase "xilkernel" $os_name] != 0} {
			if { [string compare -nocase "freertos10_xilinx" $os_name] != 0} {
				error "ERROR: lwIP with Sockets requires \"xilkernel or freertos\" OS" "" "mdt_error"
			}
		}
	}
}

proc get_emac_periphs {processor} {
	set periphs_list [::hsm::utils::get_proc_slave_periphs  $processor]
	set emac_periphs_list {}

	foreach periph $periphs_list {
		set periphname [common::get_property IP_NAME $periph]
		if {$periphname == "xps_ethernetlite"
			|| $periphname == "opb_ethernetlite"
			|| $periphname == "axi_ethernet"
			|| $periphname == "axi_ethernet_buffer"
			|| $periphname == "axi_ethernetlite"
			|| $periphname == "ps7_ethernet"
			|| $periphname == "psu_ethernet"
			|| $periphname == "psv_ethernet"} {
			lappend emac_periphs_list $periph
		} elseif {$periphname == "xps_ll_temac"} {
			set emac0_enabled "0"
			set emac1_enabled "0"
			set emac_name [common::get_property NAME $periph]

			# emac 0 is always enabled. just check for its interrupt port
			set emac_intr_port [hsi::get_pins -of_objects [hsi::get_cells -hier $periph] TemacIntc0_Irpt]
			if {$emac_intr_port != ""} {
				set emac0_enabled "1"
			} else {
				puts "Channel 0 of $emac_name cannot be used with lwIP \
					since it does not have its interrupt pin connected."
			}

			# for emac 1, check C_TEMAC1_ENABLED and then its interrupt port
			set emac1_on [common::get_property CONFIG.C_TEMAC1_ENABLED $periph "PARAMETER"]
			if {$emac1_on == "1"} {
				set emac_intr_port2 [hsi::get_pins -of_objects [hsi::get_cells -hier  $periph] TemacIntc1_Irpt]
				if {$emac_intr_port2 != ""} {
					set emac1_enabled "1"
				} else {
					puts "Channel 1 of $emac_name cannot be used with lwIP \
						since it does not have its interrupt pin connected."
				}
			}

			# we can use this emac if either of its channels are usable
			if {$emac0_enabled == "1" || $emac1_enabled == "1"} {
				lappend emac_periphs_list $periph
			}
		}
	}

	return $emac_periphs_list
}

#---------------------------------------------
# lwip_drc - check system configuration and make sure
# all components to run lwIP are available.
#---------------------------------------------
proc lwip_drc {libhandle} {
	#puts "Running DRC for lwIP library... \n"

	# find the list of xps_ethernetlite, xps_ll_temac, or axi_ethernet cores
	set sw_processor [hsi::get_sw_processor]
	set processor [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_processor]]
	set processor_type [common::get_property IP_NAME $processor]

	set emac_periphs_list [get_emac_periphs $processor]

	if { [llength $emac_periphs_list] == 0 } {
		#Retain correct compiler and processor details
		set makeconfig "src/Makefile.config"
		file delete $makeconfig
		set fd [open $makeconfig w]

		# determine the processor type so that we know the compiler to use
		switch -regexp $processor_type {
		    "microblaze" {
			puts $fd "GCC_COMPILER=mb-gcc"

			# AXI systems are Little Endian
			# 0 = BE, 1 = LE
			set endian [common::get_property CONFIG.C_ENDIANNESS $processor]
			if {$endian != 0} {
			    #puts "Little Endian system"
			    puts $fd "CONFIG_PROCESSOR_LITTLE_ENDIAN=y"
			} else {
			    #puts "Big Endian system"
			    puts $fd "CONFIG_PROCESSOR_BIG_ENDIAN=y"
			}
		    }
		    "ps7_cortexa9" {
			puts $fd "GCC_COMPILER=arm-none-eabi-gcc"
			#puts "Little Endian system"
			puts $fd "CONFIG_PROCESSOR_LITTLE_ENDIAN=y"
		    }
		    "psu_cortexr5" {
			puts $fd "GCC_COMPILER=armr5-none-eabi-gcc"
			#puts "Little Endian system"
			puts $fd "CONFIG_PROCESSOR_LITTLE_ENDIAN=y"
		    }
		    "psv_cortexr5" {
			puts $fd "GCC_COMPILER=armr5-none-eabi-gcc"
			#puts "Little Endian system"
			puts $fd "CONFIG_PROCESSOR_LITTLE_ENDIAN=y"
		    }
		    "psu_cortexa53" {
			set procdrv [hsi::get_sw_processor]
			set compiler [::common::get_property CONFIG.compiler $procdrv]
			puts $fd "GCC_COMPILER=$compiler"
			#puts "Little Endian system"
			puts $fd "CONFIG_PROCESSOR_LITTLE_ENDIAN=y"
		    }
		    "psv_cortexa72" {
			puts $fd "GCC_COMPILER=aarch64-none-elf-gcc"
			#puts "Little Endian system"
			puts $fd "CONFIG_PROCESSOR_LITTLE_ENDIAN=y"
		    }
		    default {
			puts "unknown processor type $proctype\n"
		    }
		    close $fd
		}

		set cpuname [common::get_property NAME $processor]
		error "ERROR: No Ethernet MAC cores are addressable from processor $cpuname. \
			lwIP requires atleast one EMAC (xps_ethernetlite | xps_ll_temac | axi_ethernet | axi_ethernet_buffer | axi_ethernetlite | ps7_ethernet | psu_ethernet | psv_ethernet ) core \
			with its interrupt pin connected to the interrupt controller.\n" "" "MDT_ERROR"
		return
	} else {
		set emac_names_list {}

		foreach emac $emac_periphs_list {
			lappend emac_names_list [common::get_property NAME $emac]
		}
		#puts "lwIP can be used with the following EMAC peripherals found in your system: $emac_names_list"
	}

	#----
	# check each MAC for correctness conditions
	#----
	lwip_hw_drc $libhandle $emac_names_list
	lwip_sw_drc $libhandle $emac_names_list
}

proc generate_license {fd} {
	puts $fd "/*"
	puts $fd " * Copyright (c) 2001, 2002 Swedish Institute of Computer Science."
	puts $fd " * Copyright (C) 2007 - 2018 Xilinx, Inc."
	puts $fd " * All rights reserved."
	puts $fd " *"
	puts $fd " * Redistribution and use in source and binary forms, with or without modification,"
	puts $fd " * are permitted provided that the following conditions are met:"
	puts $fd " *"
	puts $fd " * 1. Redistributions of source code must retain the above copyright notice,"
	puts $fd " *    this list of conditions and the following disclaimer."
	puts $fd " * 2. Redistributions in binary form must reproduce the above copyright notice,"
	puts $fd " *    this list of conditions and the following disclaimer in the documentation"
	puts $fd " *    and/or other materials provided with the distribution."
	puts $fd " * 3. The name of the author may not be used to endorse or promote products"
	puts $fd " *    derived from this software without specific prior written permission."
	puts $fd " *"
	puts $fd " * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED"
	puts $fd " * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF"
	puts $fd " * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT"
	puts $fd " * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,"
	puts $fd " * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT"
	puts $fd " * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS"
	puts $fd " * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN"
	puts $fd " * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING"
	puts $fd " * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY"
	puts $fd " * OF SUCH DAMAGE."
	puts $fd " *"
	puts $fd " * This file is part of the lwIP TCP/IP stack."
	puts $fd " *"
	puts $fd " * Author: Adam Dunkels <adam@sics.se>"
	puts $fd " *"
	puts $fd " */"
}

proc generate_lwip_opts {libhandle} {
	global use_axieth_on_zynq
	global use_emaclite_on_zynq
	set lwipopts_file "src/contrib/ports/xilinx/include/lwipopts.h"
	set sw_processor [hsi::get_sw_processor]
	set processor [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_processor]]
	set processor_type [common::get_property IP_NAME $processor]
	set emac_periphs_list [get_emac_periphs $processor]

	set have_emaclite 0
	foreach emac $emac_periphs_list {
		set iptype [common::get_property IP_NAME $emac]
		if {$iptype == "xps_ethernetlite" || $iptype == "opb_ethernetlite" || $iptype == "axi_ethernetlite"} {
			set have_emaclite 1
		}
		if {$iptype == "axi_ethernet"} {
			set checksum_txoption [common::get_property CONFIG.TXCSUM $emac]
			set checksum_txoption [get_checksum $checksum_txoption]
			set checksum_rxoption [common::get_property CONFIG.RXCSUM $emac]
			set checksum_rxoption [get_checksum $checksum_rxoption]
		}
		if {$iptype == "axi_ethernet_buffer" } {
			set checksum_txoption [common::get_property CONFIG.C_TXCSUM $emac]
			set checksum_rxoption [common::get_property CONFIG.C_RXCSUM $emac]
		}
	}

	file delete $lwipopts_file
	set lwipopts_fd [open $lwipopts_file w]

	generate_license $lwipopts_fd
	puts $lwipopts_fd "\#ifndef __LWIPOPTS_H_"
	puts $lwipopts_fd "\#define __LWIPOPTS_H_"
	puts $lwipopts_fd ""

        set proctype [common::get_property IP_NAME $processor]
	switch -regexp $proctype {
            "microblaze" {
                # AXI systems are Little Endian
                # 0 = BE, 1 = LE
                set endian [common::get_property CONFIG.C_ENDIANNESS $processor]
                if {$endian != 0} {
                    puts $lwipopts_fd "\#ifndef PROCESSOR_LITTLE_ENDIAN"
		    puts $lwipopts_fd "\#define PROCESSOR_LITTLE_ENDIAN"
		    puts $lwipopts_fd "\#endif\n"
                } else {
                    puts $lwipopts_fd "\#ifndef PROCESSOR_BIG_ENDIAN"
		    puts $lwipopts_fd "\#define PROCESSOR_BIG_ENDIAN"
		    puts $lwipopts_fd "\#endif\n"
                }
            }

            "ps7_cortexa9" {
		puts $lwipopts_fd "\#ifndef PROCESSOR_LITTLE_ENDIAN"
		puts $lwipopts_fd "\#define PROCESSOR_LITTLE_ENDIAN"
		puts $lwipopts_fd "\#endif\n"
            }

            "psu_cortexr5" {
		puts $lwipopts_fd "\#ifndef PROCESSOR_LITTLE_ENDIAN"
		puts $lwipopts_fd "\#define PROCESSOR_LITTLE_ENDIAN"
		puts $lwipopts_fd "\#endif\n"
            }

            "psv_cortexr5" {
		puts $lwipopts_fd "\#ifndef PROCESSOR_LITTLE_ENDIAN"
		puts $lwipopts_fd "\#define PROCESSOR_LITTLE_ENDIAN"
		puts $lwipopts_fd "\#endif\n"
            }

            "psu_cortexa53" {
		puts $lwipopts_fd "\#ifndef PROCESSOR_LITTLE_ENDIAN"
		puts $lwipopts_fd "\#define PROCESSOR_LITTLE_ENDIAN"
		puts $lwipopts_fd "\#endif\n"
            }

            "psv_cortexa72" {
		puts $lwipopts_fd "\#ifndef PROCESSOR_LITTLE_ENDIAN"
		puts $lwipopts_fd "\#define PROCESSOR_LITTLE_ENDIAN"
		puts $lwipopts_fd "\#endif\n"
            }

            default {
                puts $lwipopts_fd "\#ifndef PROCESSOR_BIG_ENDIAN"
		puts $lwipopts_fd "\#define PROCESSOR_BIG_ENDIAN"
		puts $lwipopts_fd "\#endif\n"
            }
	}

	# always use lightweight prot mechanism for critical regions
	puts $lwipopts_fd "\#define SYS_LIGHTWEIGHT_PROT 1"
	puts $lwipopts_fd ""

	set api_mode [common::get_property CONFIG.api_mode $libhandle]
	if {$api_mode == "RAW_API"} {
		# If RAW_API mode, NO_SYS should be 1, else this is set to zero by default in opt.h
		puts $lwipopts_fd "\#define NO_SYS 1"
		puts $lwipopts_fd "\#define LWIP_SOCKET 0"
		puts $lwipopts_fd "\#define LWIP_COMPAT_SOCKETS 0"
		puts $lwipopts_fd "\#define LWIP_NETCONN 0"
	}
	puts $lwipopts_fd ""

	set no_sys_no_timers	[expr [common::get_property CONFIG.no_sys_no_timers $libhandle] == true]
	puts $lwipopts_fd "\#define NO_SYS_NO_TIMERS $no_sys_no_timers"
	puts $lwipopts_fd ""

	set thread_prio [common::get_property CONFIG.socket_mode_thread_prio $libhandle]
	if {$api_mode == "SOCKET_API"} {
		set sw_proc_handle [hsi::get_sw_processor]
		set os_handle [hsi::get_os]
		set os_name [common::get_property NAME $os_handle]
		if { [string compare -nocase "xilkernel" $os_name] == 0} {
			puts $lwipopts_fd "\#define OS_IS_XILKERNEL"
			puts $lwipopts_fd "\#define TCPIP_THREAD_PRIO $thread_prio"
			puts $lwipopts_fd "\#define DEFAULT_THREAD_PRIO $thread_prio"
			puts $lwipopts_fd "\#define TCPIP_THREAD_STACKSIZE 4096"
			puts $lwipopts_fd "\#define LWIP_COMPAT_MUTEX 1"
			puts $lwipopts_fd "\#define LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT 0"
			puts $lwipopts_fd ""
		}
		if { [string compare -nocase "freertos10_xilinx" $os_name] == 0} {
			# mbox options
			set lwip_tcpip_core_locking_input	[common::get_property CONFIG.lwip_tcpip_core_locking_input $libhandle]
			set tcpip_mbox_size	[common::get_property CONFIG.tcpip_mbox_size $libhandle]
			set default_tcp_recvmbox_size	[common::get_property CONFIG.default_tcp_recvmbox_size $libhandle]
			set default_udp_recvmbox_size	[common::get_property CONFIG.default_udp_recvmbox_size $libhandle]
			puts $lwipopts_fd "\#define OS_IS_FREERTOS"
			puts $lwipopts_fd "\#define DEFAULT_THREAD_PRIO $thread_prio"
			if {$processor_type == "psu_cortexa53" || $processor_type == "psv_cortexa72" } {
				puts $lwipopts_fd "\#define TCPIP_THREAD_PRIO ($thread_prio)"
			} else {
			puts $lwipopts_fd "\#define TCPIP_THREAD_PRIO ($thread_prio + 1)"
			}
			puts $lwipopts_fd "\#define TCPIP_THREAD_STACKSIZE 1024"
			puts $lwipopts_fd "\#define DEFAULT_TCP_RECVMBOX_SIZE 	$default_tcp_recvmbox_size"
			puts $lwipopts_fd "\#define DEFAULT_ACCEPTMBOX_SIZE 	5"
			puts $lwipopts_fd "\#define TCPIP_MBOX_SIZE		$tcpip_mbox_size"
			puts $lwipopts_fd "\#define DEFAULT_UDP_RECVMBOX_SIZE 	$default_udp_recvmbox_size"
			puts $lwipopts_fd "\#define DEFAULT_RAW_RECVMBOX_SIZE	30"
			puts $lwipopts_fd "\#define LWIP_COMPAT_MUTEX 0"
			puts $lwipopts_fd "\#define LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT 1"
			if {$lwip_tcpip_core_locking_input == true} {
				puts $lwipopts_fd "\#define LWIP_TCPIP_CORE_LOCKING_INPUT 1"
			}
			puts $lwipopts_fd ""
		}
	}

	set use_axieth_on_zynq [common::get_property CONFIG.use_axieth_on_zynq $libhandle]
	set use_emaclite_on_zynq [common::get_property CONFIG.use_emaclite_on_zynq  $libhandle]

	set lwip_tcp_keepalive	[expr [common::get_property CONFIG.lwip_tcp_keepalive $libhandle] == true]
	puts $lwipopts_fd "\#define LWIP_TCP_KEEPALIVE $lwip_tcp_keepalive"
	puts $lwipopts_fd ""

	# memory options
	set mem_size 		[common::get_property CONFIG.mem_size $libhandle]
	set memp_n_pbuf 	[common::get_property CONFIG.memp_n_pbuf $libhandle]
	set memp_n_udp_pcb 	[common::get_property CONFIG.memp_n_udp_pcb $libhandle]
	set memp_n_tcp_pcb 	[common::get_property CONFIG.memp_n_tcp_pcb $libhandle]
	set memp_n_tcp_pcb_listen 	[common::get_property CONFIG.memp_n_tcp_pcb_listen $libhandle]
	set memp_n_tcp_seg 	[common::get_property CONFIG.memp_n_tcp_seg $libhandle]
	set memp_n_sys_timeout 	[common::get_property CONFIG.memp_n_sys_timeout $libhandle]
	set memp_num_netbuf 	[common::get_property CONFIG.memp_num_netbuf $libhandle]
	set memp_num_netconn 	[common::get_property CONFIG.memp_num_netconn $libhandle]
	set memp_num_api_msg 	[common::get_property CONFIG.memp_num_api_msg $libhandle]
	set memp_num_tcpip_msg 	[common::get_property CONFIG.memp_num_tcpip_msg $libhandle]

	puts $lwipopts_fd "\#define MEM_ALIGNMENT 64"
	puts $lwipopts_fd "\#define MEM_SIZE $mem_size"
	puts $lwipopts_fd "\#define MEMP_NUM_PBUF $memp_n_pbuf"
	puts $lwipopts_fd "\#define MEMP_NUM_UDP_PCB $memp_n_udp_pcb"
	puts $lwipopts_fd "\#define MEMP_NUM_TCP_PCB $memp_n_tcp_pcb"
	puts $lwipopts_fd "\#define MEMP_NUM_TCP_PCB_LISTEN $memp_n_tcp_pcb_listen"
	puts $lwipopts_fd "\#define MEMP_NUM_TCP_SEG $memp_n_tcp_seg"
	puts $lwipopts_fd "\#define MEMP_NUM_SYS_TIMEOUT $memp_n_sys_timeout"
	puts $lwipopts_fd "\#define MEMP_NUM_NETBUF $memp_num_netbuf"
	puts $lwipopts_fd "\#define MEMP_NUM_NETCONN $memp_num_netconn"
	puts $lwipopts_fd "\#define MEMP_NUM_TCPIP_MSG_API $memp_num_api_msg"
	puts $lwipopts_fd "\#define MEMP_NUM_TCPIP_MSG_INPKT $memp_num_tcpip_msg"

	# workaround for lwip mem_malloc bug
	# puts $lwipopts_fd "\#define MEM_LIBC_MALLOC 1"
	puts $lwipopts_fd ""

	# seq api
	if {$api_mode == "SOCKET_API"} {
		puts $lwipopts_fd "\#define MEMP_NUM_NETBUF     $memp_num_netbuf"
		puts $lwipopts_fd "\#define MEMP_NUM_NETCONN    $memp_num_netconn"
		puts $lwipopts_fd "\#define LWIP_PROVIDE_ERRNO  1"
	}
	puts $lwipopts_fd "\#define MEMP_NUM_SYS_TIMEOUT $memp_n_sys_timeout"

	# pbuf options
	set pbuf_pool_size	[common::get_property CONFIG.pbuf_pool_size $libhandle]
	set pbuf_pool_bufsize	[common::get_property CONFIG.pbuf_pool_bufsize $libhandle]
	set pbuf_link_hlen	[common::get_property CONFIG.pbuf_link_hlen $libhandle]

	puts $lwipopts_fd "\#define PBUF_POOL_SIZE $pbuf_pool_size"
	puts $lwipopts_fd "\#define PBUF_POOL_BUFSIZE $pbuf_pool_bufsize"
	puts $lwipopts_fd "\#define PBUF_LINK_HLEN $pbuf_link_hlen"
	puts $lwipopts_fd ""

	# ARP options
	set arp_table_size	[common::get_property CONFIG.arp_table_size $libhandle]
	set arp_queueing	[common::get_property CONFIG.arp_queueing $libhandle]
	puts $lwipopts_fd "\#define ARP_TABLE_SIZE $arp_table_size"
	puts $lwipopts_fd "\#define ARP_QUEUEING $arp_queueing"
	puts $lwipopts_fd ""

	# ICMP options
	set icmp_ttl 		[common::get_property CONFIG.icmp_ttl $libhandle]
	puts $lwipopts_fd "\#define ICMP_TTL $icmp_ttl"
	puts $lwipopts_fd ""

	# IPv6 options
	set ipv6_enable		[common::get_property CONFIG.ipv6_enable $libhandle]
	if {$ipv6_enable == true} {
	puts $lwipopts_fd "\#define LWIP_IPV6 1"
	puts $lwipopts_fd ""
	}

	# IGMP options
	set igmp_val 		[common::get_property CONFIG.igmp_options $libhandle]
	if {$igmp_val == true} {
	puts $lwipopts_fd "\#define LWIP_IGMP 1"
	puts $lwipopts_fd ""
	}

	# IP options
	set ip_forward          [common::get_property CONFIG.ip_forward $libhandle]
	set ip_options          [common::get_property CONFIG.ip_options $libhandle]
	set ip_reassembly       [common::get_property CONFIG.ip_reassembly $libhandle]
	set ip_frag             [common::get_property CONFIG.ip_frag $libhandle]
	set ip_reass_max_pbufs    [common::get_property CONFIG.ip_reass_max_pbufs $libhandle]
	set ip_frag_max_mtu     [common::get_property CONFIG.ip_frag_max_mtu $libhandle]
	set ip_default_ttl      [common::get_property CONFIG.ip_default_ttl $libhandle]

	puts $lwipopts_fd "\#define IP_OPTIONS $ip_options"
	puts $lwipopts_fd "\#define IP_FORWARD $ip_forward"
	puts $lwipopts_fd "\#define IP_REASSEMBLY $ip_reassembly"
	puts $lwipopts_fd "\#define IP_FRAG $ip_frag"
	puts $lwipopts_fd "\#define IP_REASS_MAX_PBUFS $ip_reass_max_pbufs"
	puts $lwipopts_fd "\#define IP_FRAG_MAX_MTU $ip_frag_max_mtu"
	puts $lwipopts_fd "\#define IP_DEFAULT_TTL $ip_default_ttl"
	puts $lwipopts_fd "\#define LWIP_CHKSUM_ALGORITHM 3"
	puts $lwipopts_fd ""

	# UDP options
	set lwip_udp 		[expr [common::get_property CONFIG.lwip_udp $libhandle] == true]
	set udp_ttl 		[common::get_property CONFIG.udp_ttl $libhandle]
	puts $lwipopts_fd "\#define LWIP_UDP $lwip_udp"
	puts $lwipopts_fd "\#define UDP_TTL $udp_ttl"
	puts $lwipopts_fd ""

	# TCP options
	set lwip_tcp 		[expr [common::get_property CONFIG.lwip_tcp $libhandle] == true]
	set tcp_mss 		[common::get_property CONFIG.tcp_mss $libhandle]
	set tcp_snd_buf 	[common::get_property CONFIG.tcp_snd_buf $libhandle]
	set tcp_wnd 		[common::get_property CONFIG.tcp_wnd $libhandle]
	set tcp_ttl		[common::get_property CONFIG.tcp_ttl $libhandle]
	set tcp_maxrtx          [common::get_property CONFIG.tcp_maxrtx $libhandle]
	set tcp_synmaxrtx       [common::get_property CONFIG.tcp_synmaxrtx $libhandle]
	set tcp_queue_ooseq     [common::get_property CONFIG.tcp_queue_ooseq $libhandle]

	puts $lwipopts_fd "\#define LWIP_TCP $lwip_tcp"
	puts $lwipopts_fd "\#define TCP_MSS $tcp_mss"
	puts $lwipopts_fd "\#define TCP_SND_BUF $tcp_snd_buf"
	puts $lwipopts_fd "\#define TCP_WND $tcp_wnd"
	puts $lwipopts_fd "\#define TCP_TTL $tcp_ttl"
	puts $lwipopts_fd "\#define TCP_MAXRTX $tcp_maxrtx"
	puts $lwipopts_fd "\#define TCP_SYNMAXRTX $tcp_synmaxrtx"
	puts $lwipopts_fd "\#define TCP_QUEUE_OOSEQ $tcp_queue_ooseq"
	puts $lwipopts_fd "\#define TCP_SND_QUEUELEN   16 * TCP_SND_BUF/TCP_MSS"

	set have_ethonzynq 0
	foreach emac $emac_periphs_list {
		set iptype [common::get_property IP_NAME $emac]
		if {$iptype == "axi_ethernet" || $iptype == "axi_ethernet_buffer" } {
			set have_ethonzynq 1
		}
	}

	if { $have_ethonzynq != 1} {
		set use_axieth_on_zynq 0
	}

	if {$proctype == "microblaze" || $use_axieth_on_zynq == 1} {
		set tx_full_csum_temp [common::get_property CONFIG.tcp_ip_tx_checksum_offload $libhandle]
		if {$tx_full_csum_temp == true} {
			if {$checksum_txoption != 2} {
				error "ERROR: Wrong Tx checksum options. The selected Tx checksum does not match with the HW supported Tx csum offload option"
				"" "mdt_error"
			} else {
				set tx_full_csum [expr ![common::get_property CONFIG.tcp_ip_tx_checksum_offload $libhandle]]
				puts $lwipopts_fd "\#define CHECKSUM_GEN_TCP $tx_full_csum"
				puts $lwipopts_fd "\#define CHECKSUM_GEN_UDP $tx_full_csum"
				puts $lwipopts_fd "\#define CHECKSUM_GEN_IP $tx_full_csum"
			}
		}
		set rx_full_csum_temp [common::get_property CONFIG.tcp_ip_rx_checksum_offload $libhandle]
		if {$rx_full_csum_temp == true} {
			if {$checksum_rxoption != 2} {
				error "ERROR: Wrong Rx checksum options. The selected Rx checksum does not match with the HW supported Rx csum offload option"
				"" "mdt_error"
			} else {
				set rx_full_csum [expr ![common::get_property CONFIG.tcp_ip_rx_checksum_offload $libhandle]]
				puts $lwipopts_fd "\#define CHECKSUM_CHECK_TCP $rx_full_csum"
				puts $lwipopts_fd "\#define CHECKSUM_CHECK_UDP $rx_full_csum"
				puts $lwipopts_fd "\#define CHECKSUM_CHECK_IP $rx_full_csum"
			}
		}

		set tx_csum_temp [common::get_property CONFIG.tcp_tx_checksum_offload $libhandle]
		if {$tx_csum_temp == true} {
			if {$checksum_txoption != 1} {
				error "ERROR: Wrong Tx checksum options. The selected Tx checksum does not match with the HW supported Tx csum offload option"
				"" "mdt_error"
			} else {
				set tx_csum [expr ![common::get_property CONFIG.tcp_tx_checksum_offload $libhandle]]
				puts $lwipopts_fd "\#define CHECKSUM_GEN_TCP $tx_csum"
			}
		}
		set rx_csum_temp [common::get_property CONFIG.tcp_rx_checksum_offload $libhandle]
		if {$rx_csum_temp == true} {
			if {$checksum_rxoption != 1} {
				error "ERROR: Wrong Rx checksum options. The selected Rx checksum does not match with the HW supported Rx csum offload option"
				"" "mdt_error"
			} else {
				set rx_csum [expr ![common::get_property CONFIG.tcp_rx_checksum_offload $libhandle]]
				puts $lwipopts_fd "\#define CHECKSUM_CHECK_TCP $rx_csum"
			}
		}

		if {$tx_full_csum_temp == false && $tx_csum_temp == false} {
			puts $lwipopts_fd "\#define CHECKSUM_GEN_TCP 	1"
			puts $lwipopts_fd "\#define CHECKSUM_GEN_UDP 	1"
			puts $lwipopts_fd "\#define CHECKSUM_GEN_IP  	1"
		}

		if {$rx_full_csum_temp == false && $rx_csum_temp == false} {
			puts $lwipopts_fd "\#define CHECKSUM_CHECK_TCP  1"
			puts $lwipopts_fd "\#define CHECKSUM_CHECK_UDP  1"
			puts $lwipopts_fd "\#define CHECKSUM_CHECK_IP 	1"
		}
		if {$tx_full_csum_temp == true} {
			puts $lwipopts_fd "\#define LWIP_FULL_CSUM_OFFLOAD_TX  1"
		}
		if {$rx_full_csum_temp == true} {
			puts $lwipopts_fd "\#define LWIP_FULL_CSUM_OFFLOAD_RX  1"
		}
		if {$tx_csum_temp == true} {
			puts $lwipopts_fd "\#define LWIP_PARTIAL_CSUM_OFFLOAD_TX  1"
		}
		if {$rx_csum_temp == true} {
			puts $lwipopts_fd "\#define LWIP_PARTIAL_CSUM_OFFLOAD_RX  1"
		}

	} else {
		if {$have_emaclite == 1} {
			puts $lwipopts_fd "\#define CHECKSUM_GEN_TCP 	1"
			puts $lwipopts_fd "\#define CHECKSUM_GEN_UDP 	1"
			puts $lwipopts_fd "\#define CHECKSUM_GEN_IP  	1"
			puts $lwipopts_fd "\#define CHECKSUM_CHECK_TCP  1"
			puts $lwipopts_fd "\#define CHECKSUM_CHECK_UDP  1"
			puts $lwipopts_fd "\#define CHECKSUM_CHECK_IP 	1"
		} else {
			puts $lwipopts_fd "\#define CHECKSUM_GEN_TCP 	0"
			puts $lwipopts_fd "\#define CHECKSUM_GEN_UDP 	0"
			puts $lwipopts_fd "\#define CHECKSUM_GEN_IP  	0"
			puts $lwipopts_fd "\#define CHECKSUM_CHECK_TCP  0"
			puts $lwipopts_fd "\#define CHECKSUM_CHECK_UDP  0"
			puts $lwipopts_fd "\#define CHECKSUM_CHECK_IP 	0"
			puts $lwipopts_fd "\#define LWIP_FULL_CSUM_OFFLOAD_RX  1"
			puts $lwipopts_fd "\#define LWIP_FULL_CSUM_OFFLOAD_TX  1"
		}
	}

	puts $lwipopts_fd ""


	puts $lwipopts_fd "\#define MEMP_SEPARATE_POOLS 1"
	puts $lwipopts_fd "\#define MEMP_NUM_FRAG_PBUF 256"
	puts $lwipopts_fd "\#define IP_OPTIONS_ALLOWED 0"
	if {$proctype == "microblaze"} {
		puts $lwipopts_fd "\#define TCP_OVERSIZE 0"
	} else {
		puts $lwipopts_fd "\#define TCP_OVERSIZE TCP_MSS"
	}

	puts $lwipopts_fd ""

	set jumbo_frames [common::get_property CONFIG.temac_use_jumbo_frames $libhandle]
	if {$jumbo_frames} {
		puts $lwipopts_fd "\#define USE_JUMBO_FRAMES 1"
		puts $lwipopts_fd ""

		if {$proctype == "ps7_cortexa9"} {
			puts "WARNING: Zynq Ethernet MAC does not support jumbo frames \n"
		}
	}

	# DHCP options
	set lwip_dhcp 		[expr [common::get_property CONFIG.lwip_dhcp $libhandle] == true]
	set dhcp_does_arp_check [expr [common::get_property CONFIG.dhcp_does_arp_check $libhandle] == true]
	puts $lwipopts_fd "\#define LWIP_DHCP $lwip_dhcp"
	puts $lwipopts_fd "\#define DHCP_DOES_ARP_CHECK $dhcp_does_arp_check"
	puts $lwipopts_fd ""


	# temac adapter options
	set linkspeed [common::get_property CONFIG.phy_link_speed $libhandle]
	puts $lwipopts_fd "\#define $linkspeed 1"
	puts $lwipopts_fd ""

	# lwIP stats
	set lwip_stats		[common::get_property CONFIG.lwip_stats $libhandle]
	if {$lwip_stats} {
		puts $lwipopts_fd "\#define LWIP_STATS 1"
		puts $lwipopts_fd "\#define LWIP_STATS_DISPLAY 1"
		puts $lwipopts_fd ""
	}

	# lwIP debug
	set lwip_debug		[expr [common::get_property CONFIG.lwip_debug $libhandle] == true]
	set ip_debug		[expr [common::get_property CONFIG.ip_debug $libhandle] == true]
	set tcp_debug		[expr [common::get_property CONFIG.tcp_debug $libhandle] == true]
	set udp_debug		[expr [common::get_property CONFIG.udp_debug $libhandle] == true]
	set icmp_debug		[expr [common::get_property CONFIG.icmp_debug $libhandle] == true]
	set igmp_debug		[expr [common::get_property CONFIG.igmp_debug $libhandle] == true]
	set netif_debug		[expr [common::get_property CONFIG.netif_debug $libhandle] == true]
	set sys_debug		[expr [common::get_property CONFIG.sys_debug $libhandle] == true]
	set pbuf_debug		[expr [common::get_property CONFIG.pbuf_debug $libhandle] == true]

	if {$lwip_debug == 1} {
		puts $lwipopts_fd "\#define LWIP_DEBUG 1"
		puts $lwipopts_fd "\#define DBG_TYPES_ON DBG_LEVEL_WARNING"
		if {$ip_debug} { puts $lwipopts_fd "\#define IP_DEBUG (LWIP_DBG_LEVEL_SEVERE | LWIP_DBG_ON)" }
		if {$tcp_debug} {
			puts $lwipopts_fd "\#define TCP_DEBUG (LWIP_DBG_LEVEL_SEVERE | LWIP_DBG_ON)"
			puts $lwipopts_fd "\#define TCP_DEBUG (LWIP_DBG_LEVEL_SEVERE | LWIP_DBG_ON)"
			puts $lwipopts_fd "\#define TCP_INPUT_DEBUG (LWIP_DBG_LEVEL_SEVERE | LWIP_DBG_ON)"
			puts $lwipopts_fd "\#define TCP_OUTPUT_DEBUG (LWIP_DBG_LEVEL_SEVERE | LWIP_DBG_ON)"
			puts $lwipopts_fd "\#define TCPIP_DEBUG (LWIP_DBG_LEVEL_SEVERE | LWIP_DBG_ON)"
		}
		if {$udp_debug} { puts $lwipopts_fd "\#define UDP_DEBUG (LWIP_DBG_LEVEL_SEVERE | LWIP_DBG_ON)" }
		if {$icmp_debug} { puts $lwipopts_fd "\#define ICMP_DEBUG (LWIP_DBG_LEVEL_SEVERE | LWIP_DBG_ON)" }
		if {$igmp_debug} { puts $lwipopts_fd "\#define IGMP_DEBUG (LWIP_DBG_LEVEL_SEVERE | LWIP_DBG_ON)" }
		if {$netif_debug} { puts $lwipopts_fd "\#define NETIF_DEBUG (LWIP_DBG_LEVEL_SEVERE | LWIP_DBG_ON)" }
		if {$sys_debug} {
			puts $lwipopts_fd "\#define SYS_DEBUG (LWIP_DBG_LEVEL_SEVERE | LWIP_DBG_ON)"
			puts $lwipopts_fd "\#define API_MSG_DEBUG (LWIP_DBG_LEVEL_SEVERE | LWIP_DBG_ON)"
		}
		if {$pbuf_debug} {
			puts $lwipopts_fd "\#define PBUF_DEBUG (LWIP_DBG_LEVEL_SEVERE | LWIP_DBG_ON)"
		}
		puts $lwipopts_fd "\#define MEMP_DEBUG (LWIP_DBG_LEVEL_SEVERE | LWIP_DBG_ON)"
		puts $lwipopts_fd ""
	}

	puts $lwipopts_fd "\#endif"
	close $lwipopts_fd
}


proc update_temac_topology {emac processor topologyvar} {
	upvar $topologyvar topology

        set topology(emac_baseaddr) [common::get_property CONFIG.C_BASEADDR $emac]
	set topology(emac_type) "xemac_type_xps_ll_temac"

	# find intc to which the interrupt line is connected
	set emac_intr_port [hsi::get_pins -of_objects [hsi::get_cells -hier $emac] TemacIntc0_Irpt]
	set intr_ports [::hsm::utils::get_sink_pins [hsi::get_pins -of_objects [hsi::get_cells -hier $emac] TemacIntc0_Irpt]]

	set l [llength $intr_ports]

	if { [llength $intr_ports] != 1 } {
		set emac_name [common::get_property NAME $emac]
		error "ERROR: xps_ll_temac ($emac_name) interrupt port connected to more than one IP.\
			lwIP requires that the interrupt line be connected only to the interrupt controller"
			"" "mdt_error"
	}

	set intr_port [lindex $intr_ports 0]
	set intc_handle [hsi::get_cells -of_objects $intr_port]

	# can we address this intc from the processor?
	set proc_connected_periphs [::hsm::utils::get_proc_slave_periphs $processor]
	if { [lsearch -exact $proc_connected_periphs $intc_handle] == -1 } {
		set intc_name [common::get_property NAME $intc_handle]
		set proc_name [common::get_property NAME $processor]
		error "ERROR: $intc_name to which xps_ll_temac interrupt is connected is not addressable \
			from processor $proc_name" "" "mdt_error"
	}

	set topology(intc_baseaddr) [common::get_property CONFIG.C_BASEADDR $intc_handle]
	set topology(intc_baseaddr) [::hsm::utils::format_addr_string $topology(intc_baseaddr) "C_BASEADDR"]
	set topology(scugic_baseaddr) "0x0"
	set topology(scugic_emac_intr) "0x0"
}

# redundant code
# repeat the same steps as update_temac_topology, but for temac1
proc update_temac1_topology {emac processor topologyvar} {
	upvar $topologyvar topology

        set topology(emac_baseaddr) [format 0x%x [expr [common::get_property CONFIG.C_BASEADDR $emac] + 0x40]]
	set topology(emac_type) "xemac_type_xps_ll_temac"

	# find intc to which the interrupt line is connected
	set emac_intr_port [hsi::get_pins -of_objects [hsi::get_cells -hier $emac] TemacIntc1_Irpt]
	set mhs_handle [hsi::get_cells -hier $emac]
	set intr_ports [xget_connected_ports_handle $mhs_handle $emac_intr_port "sink"]

	if { [llength $intr_ports] != 1 } {
		set emac_name [common::get_property NAME $emac]
		error "ERROR: xps_ll_temac ($emac_name) interrupt port connected to more than one IP.\
			lwIP requires that the interrupt line be connected only to the interrupt controller"
			"" "mdt_error"
	}

	set intr_port [lindex $intr_ports 0]
	set intc_handle [hsi::get_cells -of_objects  $intr_port]

	# can we address this intc from the processor?
	set proc_connected_periphs [::hsm::utils::get_proc_slave_periphs $processor]
	if { [lsearch -exact $proc_connected_periphs $intc_handle] == -1 } {
		set intc_name [common::get_property NAME $intc_handle]
		set proc_name [common::get_property NAME $processor]
		error "ERROR: $intc_name to which xps_ll_temac interrupt is connected is not addressable \
			from processor $proc_name" "" "mdt_error"
	}

	set topology(intc_baseaddr) [common::get_property CONFIG.C_BASEADDR $intc_handle]
	set topology(intc_baseaddr) [::hsm::utils::format_addr_string $topology(intc_baseaddr) "C_BASEADDR"]
	set topology(scugic_baseaddr) "0x0"
	set topology(scugic_emac_intr) "0x0"
}

proc update_emaclite_topology {emac processor topologyvar} {
	global use_emaclite_on_zynq
	upvar $topologyvar topology
	set sw_processor [hsi::get_sw_processor]
	set processor [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_processor]]
	set processor_type [common::get_property IP_NAME $processor]
	set force_emaclite_on_zynq 0

	if {$use_emaclite_on_zynq == 1} {
		if {$processor_type == "ps7_cortexa9" || $processor_type == "psu_cortexr5" || $processor_type == "psu_cortexa53" ||
			$processor_type == "psv_cortexr5" || $processor_type == "psv_cortexa72" } {
			set force_emaclite_on_zynq 1
		}
	}

	# get emac baseaddr
        set topology(emac_baseaddr) [common::get_property CONFIG.C_BASEADDR $emac]
	set topology(emac_type) "xemac_type_xps_emaclite"

	if {$force_emaclite_on_zynq == 1} {
		set topology(intc_baseaddr) "0x0"
		set topology(scugic_baseaddr) "0xF8F00100"
		set topology(scugic_emac_intr) "0x0"
		set topology(emac_intr_id) "XPAR_FABRIC_AXI_ETHERNETLITE_0_IP2INTC_IRPT_INTR"
	} else {

	# find intc to which the interrupt line is connected
	set emac_intr_port [hsi::get_pins -of_objects [hsi::get_cells -hier $emac] IP2INTC_Irpt]
	set intc_handle [::hsi::utils::get_connected_intr_cntrl $emac $emac_intr_port]
        if { $intc_handle == "" } {
               set topology(intc_baseaddr) "0x0"
	       set topology(emac_intr_id) "0x0"
               set topology(scugic_baseaddr) "0x0"
               set topology(scugic_emac_intr) "0x0"
               puts "Info: Target Periph Interrupt is not connected to interrupt controller"
               return
        }

	# can we address this intc from the processor?
	set proc_connected_periphs [::hsm::utils::get_proc_slave_periphs $processor]
	if { [lsearch -exact $proc_connected_periphs $intc_handle] == -1 } {
		set intc_name [common::get_property NAME $intc_handle]
		set proc_name [common::get_property NAME $processor]
		error "ERROR: $intc_name to which ethernetlite interrupt is connected is not addressable \
			from processor $proc_name" "" "mdt_error"
	}

	set topology(intc_baseaddr) [common::get_property CONFIG.C_BASEADDR $intc_handle]

	# find interrupt pin number
	set topology(emac_intr_id) [xget_port_interrupt_id $emac $emac_intr_port]
	set topology(scugic_baseaddr) "0x0"
	set topology(scugic_emac_intr) "0x0"
	}
}

proc update_axi_ethernet_topology {emac processor topologyvar} {
	upvar $topologyvar topology
	set sw_processor [hsi::get_sw_processor]
	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $sw_processor]]
	set topology(emac_baseaddr) [common::get_property CONFIG.C_BASEADDR $emac]
	set topology(emac_type) "xemac_type_axi_ethernet"

	# find intc to which the interrupt line is connected
	set emac_intr_port [hsi::get_pins -of_objects [hsi::get_cells -hier $emac] INTERRUPT]
       set intc_handle [::hsi::utils::get_connected_intr_cntrl $emac $emac_intr_port]
       if { $intc_handle == "" } {
               set topology(intc_baseaddr) "0x0"
               set topology(scugic_baseaddr) "0x0"
               set topology(scugic_emac_intr) "0x0"
               puts "Info: Target Periph Interrupt is not connected to interrupt controller"
               return
       }

    set intc_periph_type [common::get_property IP_NAME $intc_handle]
    set intc_name [common::get_property NAME $intc_handle]
    set proc_connected_periphs [::hsm::utils::get_proc_slave_periphs $processor]
    if { [llength $intc_periph_type] < 2} {
	if { [lsearch -exact $proc_connected_periphs $intc_handle] == -1} {
		set proc_name [common::get_property NAME $processor]
		error "ERROR: $intc_name to which axi_ethernet interrupt is connected is not addressable \
			from processor $proc_name" "" "mdt_error"
	}
    } else {
	set intc_periph_type [lindex $intc_periph_type [lsearch $intc_periph_type "psu_acpu_gic"]]
    }

	if { [llength $intc_handle] != 1 && $intc_periph_type != [format "psu_acpu_gic"]} {
		set emac_name [common::get_property NAME $emac]
		error "ERROR: axi_ethernet ($emac_name) interrupt port connected to more than one IP.\
			lwIP requires that the interrupt line be connected only to the interrupt controller"
			"" "mdt_error"
	}

	if { $intc_periph_type != [format "ps7_scugic"] && $intc_periph_type != [format "psu_scugic"] && $intc_periph_type != [format "psu_acpu_gic"] && $intc_periph_type != [format "psu_rcpu_gic"]} {
		set topology(intc_baseaddr) [common::get_property CONFIG.C_BASEADDR $intc_handle]
		set topology(intc_baseaddr) [::hsm::utils::format_addr_string $topology(intc_baseaddr) "C_BASEADDR"]
		set topology(scugic_baseaddr) "0x0"
		set topology(scugic_emac_intr) "0x0"
	} elseif { $intc_periph_type == [format "ps7_scugic"]} {
		set topology(intc_baseaddr) "0x0"
		set topology(scugic_baseaddr) "0xF8F00100"
		set topology(scugic_emac_intr) "0x0"
	} elseif { $intc_periph_type == [format "psu_acpu_gic"] && $proc_type == "psu_cortexa53"} {
		set topology(intc_baseaddr) "0x0"
		set topology(scugic_baseaddr) "0xF9020000"
		set topology(scugic_emac_intr) "0x0"
	} else {
		set topology(intc_baseaddr) "0x0"
		set topology(scugic_baseaddr) "0xF9001000"
		set topology(scugic_emac_intr) "0x0"
	}
}

proc update_ps_ethernet_topology {emac processor topologyvar} {
	upvar $topologyvar topology
	set topology(emac_baseaddr) [common::get_property CONFIG.C_S_AXI_BASEADDR $emac]
	set topology(emac_type) "xemac_type_emacps"
	set topology(intc_baseaddr) "0x0"
	set topology(emac_intr_id) "0x0"
	set topology(scugic_baseaddr) "0xF8F00100"
	set topology(scugic_emac_intr) "0x0"
	set ethernet_instance [common::get_property NAME $emac]
	if {$ethernet_instance == "ps7_ethernet_0" || $ethernet_instance == "ps7_enet0"} {
		set topology(scugic_emac_intr) "0x36"
	} elseif {$ethernet_instance == "ps7_ethernet_1" || $ethernet_instance == "ps7_enet1"} {
		set topology(scugic_emac_intr) "0x4D"
	} elseif {$ethernet_instance == "psu_ethernet_0" || $ethernet_instance == "psv_ethernet_0"} {
		set topology(scugic_emac_intr) "XPAR_XEMACPS_0_INTR"
	} elseif {$ethernet_instance == "psu_ethernet_1" || $ethernet_instance == "psv_ethernet_1"} {
		set topology(scugic_emac_intr) "XPAR_XEMACPS_1_INTR"
	} elseif {$ethernet_instance == "psu_ethernet_2"} {
		set topology(scugic_emac_intr) "XPAR_XEMACPS_2_INTR"
	} elseif {$ethernet_instance == "psu_ethernet_3"} {
		set topology(scugic_emac_intr) "XPAR_XEMACPS_3_INTR"
	}
}

proc generate_topology_per_emac {fd topologyvar} {
	upvar $topologyvar topology

	puts $fd "	{"
	puts $fd "		$topology(emac_baseaddr),"
	puts $fd "		$topology(emac_type),"
	puts $fd "		$topology(intc_baseaddr),"
	puts $fd "		$topology(emac_intr_id),"
	puts $fd "		$topology(scugic_baseaddr),"
	puts $fd "		$topology(scugic_emac_intr),"
	puts $fd "	},"
}

proc generate_topology {libhandle} {
	set sw_processor [hsi::get_sw_processor]
	set processor [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_processor]]
	set emac_periphs_list [get_emac_periphs $processor]
	set tempcntr 0


	set topology_file "src/contrib/ports/xilinx/netif/xtopology_g.c"
	set topology_size 0

	file delete $topology_file
	set tfd [open $topology_file w]

	puts $tfd "#include \"netif/xtopology.h\""
	puts $tfd "#include \"xparameters.h\""
	puts $tfd ""

	puts $tfd "struct xtopology_t xtopology\[\] = {"

	foreach emac $emac_periphs_list {
		# initialize topology variables
		set topology(emac_baseaddr) -1
		set topology(emac_type) "xemac_type_unknown"
		set topology(intc_baseaddr) -1
		set topology(emac_intr_id)  -1

		# get topology for the emac
		set iptype [common::get_property IP_NAME $emac]
		if {$iptype == "xps_ethernetlite" || $iptype == "opb_ethernetlite" || $iptype == "axi_ethernetlite"} {
			update_emaclite_topology $emac $processor topology
			generate_topology_per_emac $tfd topology
			incr topology_size 1
		} elseif {$iptype == "xps_ll_temac"} {
			set emac_intr_port [hsi::get_pins -of_objects [hsi::get_cells -hier $emac] TemacIntc0_Irpt]
			set temac0 "0"
			if {$emac_intr_port != ""} { set temac0 "1" }
			if {$temac0 == "1"} {
				update_temac_topology $emac $processor topology
				generate_topology_per_emac $tfd topology
				incr topology_size 1
			}

			set temac1 [common::get_property CONFIG.C_TEMAC1_ENABLED $emac]
			if {$temac1 == "1"} {
				update_temac1_topology $emac $processor topology
				generate_topology_per_emac $tfd topology
				incr topology_size 1
			}
		} elseif {$iptype == "axi_ethernet" || $iptype == "axi_ethernet_buffer"} {
			update_axi_ethernet_topology $emac $processor topology
			generate_topology_per_emac $tfd topology
			incr topology_size 1
		} elseif {$iptype == "ps7_ethernet"} {
			update_ps_ethernet_topology $emac $processor topology
			generate_topology_per_emac $tfd topology
			incr topology_size 1
		} elseif {$iptype == "psu_ethernet"} {
			update_ps_ethernet_topology $emac $processor topology
			generate_topology_per_emac $tfd topology
			incr topology_size 1
		} elseif {$iptype == "psv_ethernet"} {
			update_ps_ethernet_topology $emac $processor topology
			generate_topology_per_emac $tfd topology
			incr topology_size 1
		}
	}

	puts $tfd "};"
	puts $tfd ""

	puts $tfd "int xtopology_n_emacs = $topology_size;"


	close $tfd
}

proc generate_adapterconfig_makefile {libhandle} {
	global use_axieth_on_zynq
	global use_emaclite_on_zynq
	set sw_processor [hsi::get_sw_processor]
	set processor [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_processor]]
	set processor_type [common::get_property IP_NAME $processor]
	set emac_periphs_list [get_emac_periphs $processor]

	set have_emaclite 0
	set have_temac 0
	set have_axi_ethernet 0
	set have_axi_ethernet_fifo 0
	set have_axi_ethernet_dma 0
	set have_axi_ethernet_mcdma 0
	set have_ps_ethernet 0
	set force_axieth_on_zynq 0
	set force_emaclite_on_zynq 0

	if {$processor_type == "ps7_cortexa9" && $use_axieth_on_zynq == 1} {
		set force_axieth_on_zynq 1
	}
	if {$processor_type == "psu_cortexr5" && $use_axieth_on_zynq == 1} {
		set force_axieth_on_zynq 1
	}
	if {$processor_type == "psu_cortexa53" && $use_axieth_on_zynq == 1} {
		set force_axieth_on_zynq 1
	}
	if {$processor_type == "psv_cortexr5" && $use_axieth_on_zynq == 1} {
		set force_axieth_on_zynq 1
	}
	if {$processor_type == "psv_cortexa72" && $use_axieth_on_zynq == 1} {
		set force_axieth_on_zynq 1
	}
	if {$processor_type == "ps7_cortexa9" && $use_emaclite_on_zynq == 1} {
		set force_emaclite_on_zynq 1
	}
	if {$processor_type == "psu_cortexr5" && $use_emaclite_on_zynq == 1} {
		set force_emaclite_on_zynq 1
	}
	if {$processor_type == "psu_cortexa53" && $use_emaclite_on_zynq == 1} {
		set force_emaclite_on_zynq 1
	}
	if {$processor_type == "psv_cortexr5" && $use_emaclite_on_zynq == 1} {
		set force_emaclite_on_zynq 1
	}
	if {$processor_type == "psv_cortexa72" && $use_emaclite_on_zynq == 1} {
		set force_emaclite_on_zynq 1
	}
	foreach emac $emac_periphs_list {
		set iptype [common::get_property IP_NAME $emac]
		if {$iptype == "xps_ethernetlite" || $iptype == "opb_ethernetlite" || $iptype == "axi_ethernetlite"} {
			set have_emaclite 1
		} elseif {$iptype == "xps_ll_temac"} {
			set have_temac 1
		} elseif {$iptype == "axi_ethernet" || $iptype == "axi_ethernet_buffer"} {
			set have_axi_ethernet 1
			# Find the AXI FIFO or AXI DMA that this emac is connected to.
			set target_periph_type [axieth_target_periph $emac]
			if {$target_periph_type == "axi_fifo_mm_s"} {
				set have_axi_ethernet_fifo 1
			} elseif {$target_periph_type == "axi_mcdma"} {
				set have_axi_ethernet_mcdma 1
			} else {
				set have_axi_ethernet_dma 1
			}
		} elseif {$iptype == "ps7_ethernet" || $iptype == "psu_ethernet" || $iptype == "psv_ethernet" } {
			set have_ps_ethernet 1
		}
	}
	if {$force_axieth_on_zynq == 1 && $have_axi_ethernet == 1} {
		set have_ps_ethernet 0
		set have_axi_ethernet 1
	}
	if {$force_emaclite_on_zynq == 1 && $have_emaclite == 1} {
		set have_ps_ethernet 0
		set have_axi_ethernet 0
	}

	set makeconfig "src/Makefile.config"
	file delete $makeconfig
	set fd [open $makeconfig w]

        # determine the processor type so that we know the compiler to use
        set proctype [common::get_property IP_NAME $processor]
	switch -regexp $proctype {
            "microblaze" {
                puts $fd "GCC_COMPILER=mb-gcc"

                # AXI systems are Little Endian
                # 0 = BE, 1 = LE
                set endian [common::get_property CONFIG.C_ENDIANNESS $processor]
                if {$endian != 0} {
                    #puts "Little Endian system"
                    puts $fd "CONFIG_PROCESSOR_LITTLE_ENDIAN=y"
                } else {
                    #puts "Big Endian system"
                    puts $fd "CONFIG_PROCESSOR_BIG_ENDIAN=y"
                }
            }
            "ps7_cortexa9" {
		puts $fd "GCC_COMPILER=arm-none-eabi-gcc"
		#puts "Little Endian system"
                puts $fd "CONFIG_PROCESSOR_LITTLE_ENDIAN=y"
            }
            "psu_cortexr5" {
		puts $fd "GCC_COMPILER=armr5-none-eabi-gcc"
		#puts "Little Endian system"
		puts $fd "CONFIG_PROCESSOR_LITTLE_ENDIAN=y"
            }
            "psu_cortexa53" {
			set procdrv [hsi::get_sw_processor]
			set compiler [::common::get_property CONFIG.compiler $procdrv]
			puts $fd "GCC_COMPILER=$compiler"
		#puts "Little Endian system"
		puts $fd "CONFIG_PROCESSOR_LITTLE_ENDIAN=y"
            }
            "psv_cortexr5" {
		puts $fd "GCC_COMPILER=armr5-none-eabi-gcc"
		#puts "Little Endian system"
		puts $fd "CONFIG_PROCESSOR_LITTLE_ENDIAN=y"
            }
            "psv_cortexa72" {
		puts $fd "GCC_COMPILER=aarch64-none-elf-gcc"
		#puts "Little Endian system"
		puts $fd "CONFIG_PROCESSOR_LITTLE_ENDIAN=y"
            }
            default {
                puts "unknown processor type $proctype\n"
            }
        }

	if {$have_emaclite == 1} {
		puts $fd "CONFIG_XEMACLITE=y"
	}

	if {$have_temac == 1} {
		puts $fd "CONFIG_XLLTEMAC=y"
	}

	if {$force_axieth_on_zynq == 1 || $have_ps_ethernet == 0 } {
		if {$have_axi_ethernet == 1} {
			puts $fd "CONFIG_AXI_ETHERNET=y"
		}
		if {$have_axi_ethernet_dma == 1} {
			puts $fd "CONFIG_AXI_ETHERNET_DMA=y"
		}
		if {$have_axi_ethernet_mcdma == 1} {
			puts $fd "CONFIG_AXI_ETHERNET_MCDMA=y"
		}
		if {$have_axi_ethernet_fifo == 1} {
			puts $fd "CONFIG_AXI_ETHERNET_FIFO=y"
		}
	} elseif {$have_ps_ethernet == 1} {
		puts $fd "CONFIG_PS_ETHERNET=y"
	}

	set api_mode [common::get_property CONFIG.api_mode $libhandle]
	if {$api_mode == "SOCKET_API"} {
		puts $fd "CONFIG_SOCKETS=y"
	}

	close $fd
}

proc generate_adapterconfig_include {libhandle} {
	global use_axieth_on_zynq
	global use_emaclite_on_zynq
	set sw_processor [hsi::get_sw_processor]
	set processor [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_processor]]
	set processor_type [common::get_property IP_NAME $processor]
	set emac_periphs_list [get_emac_periphs $processor]

	set have_emaclite 0
	set have_temac 0
	set have_axi_ethernet 0
	set have_axi_ethernet_fifo 0
	set have_axi_ethernet_dma 0
	set have_1588_enabled 0
	set have_axi_ethernet_mcdma 0
	set have_ps_ethernet 0
	set force_axieth_on_zynq 0
	set force_emaclite_on_zynq 0

	if {$processor_type == "ps7_cortexa9" && $use_axieth_on_zynq == 1} {
		set force_axieth_on_zynq 1
	}
	if {$processor_type == "psu_cortexr5" && $use_axieth_on_zynq == 1} {
		set force_axieth_on_zynq 1
	}
	if {$processor_type == "psu_cortexa53" && $use_axieth_on_zynq == 1} {
		set force_axieth_on_zynq 1
	}
	if {$processor_type == "psv_cortexr5" && $use_axieth_on_zynq == 1} {
		set force_axieth_on_zynq 1
	}
	if {$processor_type == "psv_cortexa72" && $use_axieth_on_zynq == 1} {
		set force_axieth_on_zynq 1
	}
	if {$processor_type == "ps7_cortexa9" && $use_emaclite_on_zynq == 1} {
		set force_emaclite_on_zynq 1
	}
	if {$processor_type == "psu_cortexr5" && $use_emaclite_on_zynq == 1} {
		set force_emaclite_on_zynq 1
	}
	if {$processor_type == "psu_cortexa53" && $use_emaclite_on_zynq == 1} {
		set force_emaclite_on_zynq 1
	}
	if {$processor_type == "psv_cortexr5" && $use_emaclite_on_zynq == 1} {
		set force_emaclite_on_zynq 1
	}
	if {$processor_type == "psv_cortexa72" && $use_emaclite_on_zynq == 1} {
		set force_emaclite_on_zynq 1
	}

	foreach emac $emac_periphs_list {
		set iptype [common::get_property IP_NAME $emac]
		if {$iptype == "xps_ethernetlite" || $iptype == "opb_ethernetlite" || $iptype == "axi_ethernetlite"} {
			set have_emaclite 1
		} elseif {$iptype == "xps_ll_temac"} {
			set have_temac 1
		} elseif {$iptype == "axi_ethernet" || $iptype == "axi_ethernet_buffer"} {
			# Find the AXI FIFO or AXI DMA that this emac is connected to.
			set target_periph_type [axieth_target_periph $emac]
			if {$target_periph_type == "axi_fifo_mm_s"} {
				set have_axi_ethernet_fifo 1
			} elseif {$target_periph_type == "axi_mcdma"} {
				set have_axi_ethernet_mcdma 1
			} else {
				set have_axi_ethernet_dma 1
			}
			#Find 1588 Enabled or not
			set enable_1588 [common::get_property CONFIG.Enable_1588 $emac]
			set have_1588_enabled [is_property_set $enable_1588]

			set have_axi_ethernet 1
		} elseif {$iptype == "ps7_ethernet" || $iptype == "psu_ethernet" || $iptype == "psv_ethernet"} {
			set have_ps_ethernet 1
		}
	}
	if {$force_emaclite_on_zynq == 1 && $have_emaclite == 1} {
		set have_ps_ethernet 0
		set have_axi_ethernet 0
	}

	if {$force_axieth_on_zynq == 1 && $have_axi_ethernet == 1} {
		set have_ps7_ethernet 0
		set have_emaclite 0
		set have_axi_ethernet 1
	}
	set adapterconfig "src/contrib/ports/xilinx/include/xlwipconfig.h"
	file delete $adapterconfig
	set fd [open $adapterconfig w]
	generate_license $fd

	puts $fd "\#ifndef __XLWIPCONFIG_H_"
	puts $fd "\#define __XLWIPCONFIG_H_\n\n"
	puts $fd "/* This is a generated file - do not edit */"
	puts $fd ""

	if {$force_axieth_on_zynq == 1 && $have_axi_ethernet == 1} {
		puts $fd "\#define XLWIP_CONFIG_INCLUDE_AXIETH_ON_ZYNQ 1"
	}
	if {$force_emaclite_on_zynq == 1 && $have_emaclite == 1} {
		puts $fd "\#define XLWIP_CONFIG_INCLUDE_EMACLITE_ON_ZYNQ 1"
	}
	if {$have_emaclite == 1} {
		puts $fd "\#define XLWIP_CONFIG_INCLUDE_EMACLITE 1"
	}

	if {$force_axieth_on_zynq == 1 || $have_ps_ethernet == 0 } {
		if {$have_axi_ethernet == 1} {
			puts $fd "\#define XLWIP_CONFIG_INCLUDE_AXI_ETHERNET 1"
		}
		if {$have_axi_ethernet_dma == 1} {
			puts $fd "\#define XLWIP_CONFIG_INCLUDE_AXI_ETHERNET_DMA 1"
		}
		if {$have_axi_ethernet_fifo == 1} {
			puts $fd "\#define XLWIP_CONFIG_INCLUDE_AXI_ETHERNET_FIFO 1"
		}
		if {$have_1588_enabled == 1} {
			puts $fd "\#define XLWIP_CONFIG_AXI_ETHERNET_ENABLE_1588 1"
		}
		if {$have_axi_ethernet_mcdma == 1} {
			puts $fd "\#define XLWIP_CONFIG_INCLUDE_AXI_ETHERNET_MCDMA 1"
		}
	} elseif {$have_ps_ethernet == 1} {
			puts $fd "\#define XLWIP_CONFIG_INCLUDE_GEM 1"
	}

	if {$have_axi_ethernet == 1} {
		set ndesc [common::get_property CONFIG.n_tx_descriptors $libhandle]
		puts $fd "\#define XLWIP_CONFIG_N_TX_DESC $ndesc"
		set ndesc [common::get_property CONFIG.n_rx_descriptors $libhandle]
		puts $fd "\#define XLWIP_CONFIG_N_RX_DESC $ndesc"
		puts $fd ""

		set ncoalesce [common::get_property CONFIG.n_tx_coalesce $libhandle]
		puts $fd "\#define XLWIP_CONFIG_N_TX_COALESCE $ncoalesce"
		set ncoalesce [common::get_property CONFIG.n_rx_coalesce $libhandle]
		puts $fd "\#define XLWIP_CONFIG_N_RX_COALESCE $ncoalesce"
		puts $fd ""
	}
	if {$have_ps_ethernet == 1} {
		set emacnum [common::get_property CONFIG.emac_number $libhandle]
		puts $fd "\#define XLWIP_CONFIG_EMAC_NUMBER $emacnum"
		set ndesc [common::get_property CONFIG.n_tx_descriptors $libhandle]
		puts $fd "\#define XLWIP_CONFIG_N_TX_DESC $ndesc"
		set ndesc [common::get_property CONFIG.n_rx_descriptors $libhandle]
		puts $fd "\#define XLWIP_CONFIG_N_RX_DESC $ndesc"
		puts $fd ""
	}

	puts $fd "\#endif"

	close $fd
}

proc axieth_target_periph {emac} {
	set p2p_busifs_i [get_intf_pins -of_objects [get_cells -hier $emac] -filter "TYPE==INITIATOR"]
	foreach p2p_busif $p2p_busifs_i {
		set busif_name [string toupper [get_property NAME  $p2p_busif]]
		set conn_busif_handle [::hsi::utils::get_connected_intf $emac $busif_name]
		if { [string compare -nocase $conn_busif_handle ""] == 0} {
			continue
		} else {
		# if there is a single match, we know if it is FIFO or DMA
		set conn_busif_name [get_property NAME  $conn_busif_handle]
		set target_periph [get_cells -of_objects $conn_busif_handle]
		foreach target_peri $target_periph {
			set target_periph_type [get_property IP_NAME $target_periph]
			if {$target_periph_type == "axi_dma" || $target_periph_type == "axi_fifo_mm_s" || $target_periph_type == "axi_mcdma"} {
				set target_periph_name [string toupper [get_property NAME $target_periph]]
				return $target_periph_type
			}
			if { [string compare -nocase $target_periph_type "tri_mode_ethernet_mac"] == 0 } {
				continue
			}
			set target_periph_name [string toupper [get_property NAME $target_periph]]
				break
		}
		}
	}

	return $target_periph_type
}

proc get_checksum {value} {
	if {[string compare -nocase $value "None"] == 0} {
		set value 0
	} elseif {[string compare -nocase $value "Partial"] == 0} {
		set value 1
	} else {
		set value 2
	}

	return $value
}

proc is_property_set {value} {
	if {[string compare -nocase $value "true"] == 0} {
		set value 1
	} else {
		set value 0
	}

	return $value
}

#-------
# generate: called after OS and library files are copied into project dir
# 	we need to generate the following:
#		1. Makefile options
#		2. System Arch settings for lwIP to use
#-------
proc generate {libhandle} {
	generate_lwip_opts $libhandle
	generate_topology  $libhandle
	generate_adapterconfig_makefile $libhandle
	generate_adapterconfig_include $libhandle
}

#-------
# post_generate: called after generate called on all libraries
#-------
proc post_generate {libhandle} {
}

#-------
# execs_generate: called after BSP's, libraries and drivers have been compiled
#	This procedure builds the liblwip4.a library
#-------
proc execs_generate {libhandle} {
}
