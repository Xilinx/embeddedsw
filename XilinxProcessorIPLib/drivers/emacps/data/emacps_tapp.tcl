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
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 2.0   adk  10/12/13 Updated as per the New Tcl API's
# 3.0   adk  08/1/15  Don't include gem in peripheral test when gem is
#		      configured with PCS/PMA Core.
# 3.0   kpc  01/20/15 Don't include examples when interrupt is not connected
# 3.2   mus  02/20/16 Added support for microblaze
# 3.2   hk   03/10/16 Removed support for Zynq Ultrascale+ MPSoC
##############################################################################

# Uses $XILINX_EDK/bin/lib/xillib_sw.tcl
# -----------------------------------------------------------------
# Software Project Types (swproj):
#   0 : MemoryTest - Calls basic  memorytest routines from common driver dir
#   1 : PeripheralTest - Calls any existing polled_example and/or selftest
# -----------------------------------------------------------------

# -----------------------------------------------------------------
# TCL Procedures:
# -----------------------------------------------------------------
set ispcs_pma 0
set processor_type 0

proc gen_include_files {swproj mhsinst} {

	global ispcs_pma
	global processor_type

    if {$swproj == 0} {
            return ""
    }

	set isintr [::hsm::utils::is_ip_interrupting_current_proc $mhsinst]

	if {$isintr == 0} {
		return ""
	}

	set ispcs_pma 0
	set ipname [get_property NAME $mhsinst]
	set ips [get_cells -hier "*"]
	set ipconv 0
	foreach ip $ips {
		set convipname [get_property NAME  $ip]
		set periph [get_property IP_NAME $ip]
		if { [string compare -nocase $periph "gig_ethernet_pcs_pma"] == 0} {
			set sgmii_param [get_property CONFIG.c_is_sgmii $ip]
			set PhyStandarrd [get_property CONFIG.Standard $ip]
			if {$sgmii_param == true || $PhyStandarrd == "1000BASEX"} {
				set ipconv $ip
			}
			break
		}
	}

	if { $ipconv != 0 }  {
		set port_value [get_pins -of_objects [get_nets -of_objects [get_pins -of_objects $ipconv gmii_txd]]]
		if { $port_value != 0 } {
			set tmp [string first "ENET0" $port_value]
		}
		if { $tmp >= 0 } {
			if { [string compare -nocase $ipname "ps7_ethernet_0"] == 0} {
				set ispcs_pma 1
			}
		}
		set tmp0 [string first "ENET1" $port_value]
		if { $tmp0 >= 0 } {
					if { [string compare -nocase $ipname "ps7_ethernet_1"] == 0} {
						set ispcs_pma 1
					}
		}
	}

	set sw_processor [hsi::get_sw_processor]
	set processor [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_processor]]
	set processor_type [common::get_property IP_NAME $processor]

	if {$ispcs_pma == 0} {
		if {$swproj == 1} {
			if { $processor_type != "psu_cortexr5" && $processor_type != "psu_cortexa53"} {
				set inc_file_lines {xemacps.h xemacps_example.h emacps_header.h}
				return $inc_file_lines
			}
		}
	}
    return ""
}

proc gen_src_files {swproj mhsinst} {
  global ispcs_pma
  global processor_type

  if {$swproj == 0} {
    return ""
  }

  set isintr [::hsm::utils::is_ip_interrupting_current_proc $mhsinst]

  if {$isintr == 0} {
     return ""
  }

  if {$ispcs_pma == 0} {
		if {$swproj == 1} {

			if { $processor_type != "psu_cortexr5" && $processor_type != "psu_cortexa53"} {
				set inc_file_lines {examples/xemacps_example_intr_dma.c examples/xemacps_example_util.c examples/xemacps_example.h data/emacps_header.h}

				return $inc_file_lines
			}
		}
	}
	 return ""
}

proc gen_testfunc_def {swproj mhsinst} {
  return ""
}

proc gen_init_code {swproj mhsinst} {
	global ispcs_pma
	global processor_type

    if {$swproj == 0} {
        return ""
    }

    set isintr [::hsm::utils::is_ip_interrupting_current_proc $mhsinst]

    if {$isintr == 0} {
	return ""
    }

	if {$ispcs_pma == 0} {
		if {$swproj == 1} {

			if { $processor_type != "psu_cortexr5" && $processor_type != "psu_cortexa53"} {
				set ipname [common::get_property NAME $mhsinst]
				set decl "   static XEmacPs ${ipname};"
				set inc_file_lines $decl
				return $inc_file_lines

			}
		}
	}
	 return ""

}

proc gen_testfunc_call {swproj mhsinst} {
	global ispcs_pma
	global processor_type

    if {$swproj == 0} {
        return ""
    }
    set isintr [::hsm::utils::is_ip_interrupting_current_proc $mhsinst]

    if {$isintr == 0} {
	return ""
    }

    set ipname [common::get_property NAME $mhsinst]
    set deviceid [::hsi::utils::get_ip_param_name $mhsinst "DEVICE_ID"]
    set stdout [common::get_property CONFIG.STDOUT [hsi::get_os]]
    if { $stdout == "" || $stdout == "none" } {
       set hasStdout 0
    } else {
       set hasStdout 1
    }

    set isintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
    set intcvar intc
    if {$isintr == 1} {
        set intr_pin_name [hsi::get_pins -of_objects [hsi::get_cells -hier $ipname] INTERRUPT]
        set intcname [::hsi::utils::get_connected_intr_cntrl $ipname  $intr_pin_name]
        set proc [get_property IP_NAME [hsi::get_cells -hier [hsi::get_sw_processor]]]
    }

    set testfunc_call ""

  if {${hasStdout} == 0} {

	if {$ispcs_pma == 0} {
	if { $processor_type != "psu_cortexr5" && $processor_type != "psu_cortexa53"} {

	if {$isintr == 1} {
            if {
                $proc == "microblaze"
            } then {
                if { [string compare -nocase $ipname "ps7_ethernet_0"] == 0} {
                    set intr_id "XPAR_${intcname}_PROCESSING_SYSTEM7_0_IRQ_P2F_ENET0_INTR"
                } else {
                    set intr_id "XPAR_${intcname}_PROCESSING_SYSTEM7_1_IRQ_P2F_ENET1_INTR"
		}
            } else {
        set intr_id "XPAR_${ipname}_INTR"
            }
	set intr_id [string toupper $intr_id]

      append testfunc_call "

   {
      int Status;
      Status = EmacPsDmaIntrExample(&${intcvar}, &${ipname}, \\
                                 ${deviceid}, \\
                                 ${intr_id});
   }"

   }

	}
	}
  } else {

	if {$ispcs_pma == 0} {
	if { $processor_type != "psu_cortexr5" && $processor_type != "psu_cortexa53"} {
	if {$isintr == 1} {
           if {
                $proc == "microblaze"
            } then {
                if { [string compare -nocase $ipname "ps7_ethernet_0"] == 0} {
                    set intr_id "XPAR_${intcname}_PROCESSING_SYSTEM7_0_IRQ_P2F_ENET0_INTR"
                } else {
                    set intr_id "XPAR_${intcname}_PROCESSING_SYSTEM7_1_IRQ_P2F_ENET1_INTR"
		}
            } else {
        set intr_id "XPAR_${ipname}_INTR"
            }
	set intr_id [string toupper $intr_id]

      append testfunc_call "
   {
      int Status;

      print(\"\\r\\n Running Interrupt Test  for ${ipname}...\\r\\n\");

      Status = EmacPsDmaIntrExample(&${intcvar}, &${ipname}, \\
                                 ${deviceid}, \\
                                 ${intr_id});

      if (Status == 0) {
         print(\"EmacPsDmaIntrExample PASSED\\r\\n\");
      }
      else {
         print(\"EmacPsDmaIntrExample FAILED\\r\\n\");
      }

   }"

   }
	}
	}
 }

  return $testfunc_call
}
