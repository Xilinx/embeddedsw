###############################################################################
#
# Copyright (C) 2010 - 2018 Xilinx, Inc.  All rights reserved.
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
# MODIFICATION HISTORY:
#
# Ver      Who    Date     Changes
# -------- ------ -------- ----------------------------------------------------
#  1.00.a  asa    05/12/10 First Release
#  4.0     adk    10/12/13 Updated as per the New Tcl API's
# 4.1 	   adk    21/4/14  Fixed the CR:780537 Modified the get_dma_info proc
#		   	   logic as appropriate(In case of multiple dma's in the
#			   system some connected to ethernet some not).
#
###############################################################################

# -----------------------------------------------------------------
# Software Project Types (swproj):
#   0 : MemoryTest - Calls basic  memorytest routines from common driver dir
#   1 : PeripheralTest - Calls any existing polled_example and/or selftest
# -----------------------------------------------------------------

# -----------------------------------------------------------------
# TCL Procedures:
# -----------------------------------------------------------------

proc gen_include_files {swproj mhsinst} {
   if {$swproj == 0} {
      return ""
   }
   if {$swproj == 1} {
      set ifintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
      set dmaType [get_dma_type $mhsinst]

      if {$ifintr == 1} {
         set inc_file_lines {xaxiethernet.h xaxiethernet_example.h}

         if {$dmaType == 1} {
            append inc_file_lines " xllfifo.h"
            append inc_file_lines " axiethernet_header.h"
            append inc_file_lines " axiethernet_fifo_intr_header.h"
         }
         if {$dmaType == 2} {
            append inc_file_lines " xmcdma.h"
            append inc_file_lines " axiethernet_mcdma_intr_header.h"
         }
         if {$dmaType == 3} {
            append inc_file_lines " xaxidma.h"
            append inc_file_lines " axiethernet_intr_header.h"
         }
      } else {
         if {$dmaType == 1} {
            set inc_file_lines {xaxiethernet.h xaxiethernet_example.h axiethernet_header.h}
         } else {
            return ""
         }
      }

      return $inc_file_lines
   }
}

proc gen_src_files {swproj mhsinst} {
  if {$swproj == 0} {
    return ""
  }
  if {$swproj == 1} {
        set ifintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]
        set dmaType [get_dma_type $mhsinst]

      if {$ifintr == 1} {
          if {$dmaType == 1} {
              set inc_file_lines {examples/xaxiethernet_example.h examples/xaxiethernet_example_polled.c examples/xaxiethernet_example_util.c examples/xaxiethernet_example_intr_fifo.c data/axiethernet_header.h data/axiethernet_fifo_intr_header.h}
          } elseif {$dmaType == 3} {
              set inc_file_lines {examples/xaxiethernet_example.h examples/xaxiethernet_example_util.c examples/xaxiethernet_example_intr_sgdma.c data/axiethernet_intr_header.h}
          } elseif {$dmaType == 2} {
              set inc_file_lines {examples/xaxiethernet_example.h examples/xaxiethernet_example_util.c examples/xaxiethernet_example_intr_mcdma.c data/axiethernet_mcdma_intr_header.h}
	  }
      } else {
          if {$dmaType == 1} {
              set inc_file_lines {examples/xaxiethernet_example.h examples/xaxiethernet_example_polled.c examples/xaxiethernet_example_util.c data/axiethernet_header.h}
          } else {
		return ""
	  }
      }
      return $inc_file_lines
  }
}

proc gen_testfunc_def {swproj mhsinst} {
  return ""
}

proc gen_init_code {swproj mhsinst} {
   if {$swproj == 0} {
      return ""
   }
   if {$swproj == 1} {

      set ipname [get_property NAME  $mhsinst]
      set ifintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]

      if {$ifintr == 1} {
         set dmaType [get_dma_type $mhsinst]
         set decl "   static XAxiEthernet ${ipname}_AxiEthernet;"

         # FIFO
         if {$dmaType == 1} {
            set fifo_ipname [get_fifo_info $mhsinst "name"]

            append decl "
   static XLlFifo  ${fifo_ipname}_AxiFifo;

"
         }

         # DMA
         if {$dmaType == 3} {
            append decl "
   static XAxiDma  ${ipname}_AxiDma;

"
         }

	#MCDMA
	if {$dmaType == 2} {
            append decl "
   static XMcdma  ${ipname}_Mcdma;
"
	}

         set inc_file_lines $decl
         return $inc_file_lines
      }
   }

   return ""
}


proc gen_testfunc_call {swproj mhsinst} {

  if {$swproj == 0} {
    return ""
  }

  set ipname [get_property NAME $mhsinst]
  set deviceid [::hsi::utils::get_ip_param_name $mhsinst "DEVICE_ID"]
  set stdout [get_property CONFIG.STDOUT [get_os]]
  if { $stdout == "" || $stdout == "none" } {
       set hasStdout 0
  } else {
       set hasStdout 1
  }
  set dma [get_dma_type $mhsinst]
  set ifintr [::hsi::utils::is_ip_interrupting_current_proc $mhsinst]

  set fifo_deviceid [get_fifo_info $mhsinst "id"]
  set fifo_ipname   [get_fifo_info $mhsinst "name"]

  set dma_deviceid [get_dma_info $mhsinst "id"]
  set dma_ipname   [get_dma_info $mhsinst "name"]

  set mcdma_deviceid [get_mcdma_info $mhsinst "id"]
  set mcdma_ipname   [get_mcdma_info $mhsinst "name"]

  if {$ifintr == 1} {
	set intr_pin_name [get_pins -of_objects [get_cells -hier $ipname] INTERRUPT]
	set intcname [::hsi::utils::get_connected_intr_cntrl $ipname  $intr_pin_name]
	set intcvar intc
	set proc [get_property IP_NAME [get_cells -hier [get_sw_processor]]]
  }


  if { $dma == 1 } {
      set type "Fifo"
  }
  if { $dma == 3 || $dma == 2} {
      set type "SgDma"
  }

  set testfunc_call ""

   # BEGIN: FIFO
   if { $dma == 1 } {
      append testfunc_call "

   {
      int Status;
"

      if {${hasStdout} == 1} {
         append testfunc_call "
      print(\"\\r\\n Running AxiEthernetPolledExample() for ${ipname}...\\r\\n\");
"
      }

      append testfunc_call "
      Status = AxiEthernetPolledExample( ${deviceid},
                                   ${fifo_deviceid} );
"

      if {${hasStdout} == 1} {
         append testfunc_call "
      if (Status == 0) {
         print(\"AxiEthernetPolledExample PASSED\\r\\n\");
      }
      else {
         print(\"AxiEthernetPolledExample FAILED\\r\\n\");
      }
"
      }

      append testfunc_call "
   }
"
   }
   # END: FIFO

   # BEGIN: DMA
   if { $dma == 3 } {
      append testfunc_call "
   /* AxiEthernetPolledExample does not support AXI DMA   */
"
   }
   # END: DMA

   # BEGIN: INTERRUPT
   if { ${ifintr} == 1 } {

      # AXIETHERNET
        if {
           $proc == "microblaze"
	} then {
		set intr_id   "XPAR_${intcname}_${ipname}_${intr_pin_name}_INTR"
	} else {
		set intr_id "XPAR_FABRIC_${ipname}_${intr_pin_name}_INTR"
	}
      set intr_id   [string toupper $intr_id]


      # BEGIN: FIFO & INTERRUPT
      if {$dma == 1} {
         # AXIFIFO
	  if {
           $proc == "microblaze"
	} then {
		set fifo_intr_id "XPAR_${intcname}_${fifo_ipname}_${intr_pin_name}_INTR"
	} else {
		set fifo_intr_id "XPAR_FABRIC_${fifo_ipname}_${intr_pin_name}_INTR"
	}

         set fifo_intr_id   [string toupper $fifo_intr_id]

         append testfunc_call "
   {
      int Status;
"

         if {${hasStdout} == 1} {
            append testfunc_call "
      print(\"\\r\\nRunning AxiEthernet${type}IntrExample() for ${ipname}...\\r\\n\");
"
         }

         append testfunc_call "
      Status = AxiEthernet${type}IntrExample(&${intcvar}, &${ipname}_AxiEthernet,
                  &${fifo_ipname}_AxiFifo,
                  ${deviceid},
                  ${fifo_deviceid},
                  ${intr_id},
                  ${fifo_intr_id});
"

         if {${hasStdout} == 1} {
            append testfunc_call "
      if(Status == 0) {
         print(\"AxiEthernet Interrupt Test PASSED.\\r\\n\");
      }
      else {
         print(\"AxiEthernet Interrupt Test FAILED.\\r\\n\");
      }
"
         }

         append testfunc_call "
   }
"
      }
      # END: FIFO & INTERRUPT

      # BEGIN: MCDMA & INTERRUPT
      if { $dma == 2} {
         set dmaDriverInst "${ipname}_Mcdma"
         append testfunc_call "
   {
      int Status;
"

         if {${hasStdout} == 1} {
            append testfunc_call "
      print(\"\\r\\nRunning AxiEthernet${type}IntrExample() for ${ipname}...\\r\\n\");
"
         }

         append testfunc_call "
      Status = AxiEthernet${type}IntrExample(&${intcvar}, &${ipname}_AxiEthernet,
                     &${dmaDriverInst},
                     ${deviceid},
                     ${mcdma_deviceid},
                     ${intr_id});
"

         if {${hasStdout} == 1} {
            append testfunc_call "
      if (Status == 0) {
         print(\"AxiEthernet Interrupt Test PASSED.\\r\\n\");
      }
      else {
         print(\"AxiEthernet Interrupt Test FAILED.\\r\\n\");
      }
"
         }

         append testfunc_call "
   }"
      }
      # END: MCDMA & INTERRUPT
      # BEGIN: DMA & INTERRUPT
      if {$dma == 3} {
         # DMA
         set dmaDriverInst "${ipname}_AxiDma"
         set dmaRxIntrId "XPAR_AXIETHERNET_0_CONNECTED_DMARX_INTR"
         set dmaTxIntrId "XPAR_AXIETHERNET_0_CONNECTED_DMATX_INTR"

         append testfunc_call "
   {
      int Status;
"

         if {${hasStdout} == 1} {
            append testfunc_call "
      print(\"\\r\\nRunning AxiEthernet${type}IntrExample() for ${ipname}...\\r\\n\");
"
         }

         append testfunc_call "
      Status = AxiEthernet${type}IntrExample(&${intcvar}, &${ipname}_AxiEthernet,
                     &${dmaDriverInst},
                     ${deviceid},
                     ${dma_deviceid},
                     ${intr_id},
                     ${dmaRxIntrId},
                     ${dmaTxIntrId});
"

         if {${hasStdout} == 1} {
            append testfunc_call "
      if (Status == 0) {
         print(\"AxiEthernet Interrupt Test PASSED.\\r\\n\");
      }
      else {
         print(\"AxiEthernet Interrupt Test FAILED.\\r\\n\");
      }
"
         }

         append testfunc_call "
   }"
      }
      # END: DMA & INTERRUPT
   }
   # END: INTERRUPT

   return $testfunc_call
}

proc get_fifo_info {mhsHandle type} {

   set ipinst_list [get_cells -hier $mhsHandle "*"]

   foreach ipinst $ipinst_list {
      set coreName [get_property IP_NAME $ipinst]
      set instName [get_property NAME  $ipinst]

      if {[string compare -nocase $coreName "axi_fifo_mm_s"] == 0} {

         if {[string compare -nocase $type "id"] == 0} {
            set deviceid [::hsi::utils::get_ip_param_name $ipinst "DEVICE_ID"]
            return $deviceid
         }
         if {[string compare -nocase $type "name"] == 0} {
            return $instName
         }
      }
   }
}

proc get_dma_info {mhsinst type} {
    set ipinst_list [get_cells -hier  $mhsinst "*"]

	set p2p_busifs_i [get_intf_pins -of_objects $mhsinst -filter "TYPE==INITIATOR"]
	# Add p2p periphs
        foreach p2p_busif $p2p_busifs_i {
	    set busif_name [string toupper [get_property NAME  $p2p_busif]]
            set conn_busif_handle [::hsi::utils::get_connected_intf $mhsinst $busif_name]
	    if { [string compare -nocase $conn_busif_handle ""] == 0} {
                continue
            } else {
		# if there is a single match, we know if it is FIFO or DMA
		# no need for further iterations
		set conn_busif_name [get_property NAME  $conn_busif_handle]
		set target_periph [get_cells -of_objects $conn_busif_handle]
		set target_periph_type [get_property IP_NAME $target_periph]
                if { [string compare -nocase $target_periph_type "tri_mode_ethernet_mac"] == 0 } {
			continue
		}
		set target_periph_name [string toupper [get_property NAME $target_periph]]
		set instName [get_property NAME  $target_periph]
		## If Chiscope is connected b/w DMA/FIFO and Ethernet
                if {[llength $target_periph] > 1} {
                        foreach peri_name $target_periph {
                                set target_periph_type [get_property IP_NAME $peri_name]
				set instName [get_property NAME  $peri_name]
                                if {[string compare -nocase $target_periph_type "axi_dma"] == 0} {
                                        if {[string compare -nocase $type "id"] == 0} {
                                                set deviceid [::hsi::utils::get_ip_param_name $peri_name "DEVICE_ID"]
                                                return $deviceid
                                        }
                                        if {[string compare -nocase $type "name"] == 0} {
                                                return $instName
                                        }
                                }
                        }

                }
		if {[string compare -nocase $target_periph_type "axi_dma"] == 0} {
			if {[string compare -nocase $type "id"] == 0} {
				set deviceid [::hsi::utils::get_ip_param_name $target_periph "DEVICE_ID"]
				return $deviceid
			}
			if {[string compare -nocase $type "name"] == 0} {
				return $instName
			}
		}
	}
    }
}

proc get_mcdma_info {mhsinst type} {
    set ipinst_list [get_cells -hier  $mhsinst "*"]

	set p2p_busifs_i [get_intf_pins -of_objects $mhsinst -filter "TYPE==INITIATOR"]
	# Add p2p periphs
        foreach p2p_busif $p2p_busifs_i {
	    set busif_name [string toupper [get_property NAME  $p2p_busif]]
            set conn_busif_handle [::hsi::utils::get_connected_intf $mhsinst $busif_name]
	    if { [string compare -nocase $conn_busif_handle ""] == 0} {
                continue
            } else {
		# if there is a single match, we know if it is FIFO or DMA
		# no need for further iterations
		set conn_busif_name [get_property NAME  $conn_busif_handle]
		set target_periph [get_cells -of_objects $conn_busif_handle]
		set target_periph_type [get_property IP_NAME $target_periph]
                if { [string compare -nocase $target_periph_type "tri_mode_ethernet_mac"] == 0 } {
			continue
		}
		set target_periph_name [string toupper [get_property NAME $target_periph]]
		set instName [get_property NAME  $target_periph]
		## If Chiscope is connected b/w DMA/FIFO and Ethernet
                if {[llength $target_periph] > 1} {
                        foreach peri_name $target_periph {
                                set target_periph_type [get_property IP_NAME $peri_name]
				set instName [get_property NAME  $peri_name]
                                if {[string compare -nocase $target_periph_type "axi_mcdma"] == 0} {
                                        if {[string compare -nocase $type "id"] == 0} {
                                                set deviceid [::hsi::utils::get_ip_param_name $peri_name "DEVICE_ID"]
                                                return $deviceid
                                        }
                                        if {[string compare -nocase $type "name"] == 0} {
                                                return $instName
                                        }
                                }
                        }

                }
		if {[string compare -nocase $target_periph_type "axi_mcdma"] == 0} {
			if {[string compare -nocase $type "id"] == 0} {
				set deviceid [::hsi::utils::get_ip_param_name $target_periph "DEVICE_ID"]
				return $deviceid
			}
			if {[string compare -nocase $type "name"] == 0} {
				return $instName
			}
		}
	}
    }
}

proc get_dma_type {mhsinst} {

   set dma_deviceid [get_dma_info $mhsinst "id"]
   set mcdma_deviceid [get_mcdma_info $mhsinst "id"]

  if { $mcdma_deviceid != "" } {
	set dma 2
  } elseif { $dma_deviceid != "" } {
        set dma 3
  } else {
        set dma 1
  }

  return $dma
}
