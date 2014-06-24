###############################################################################
#
# Copyright (C) 2003 - 2014 Xilinx, Inc.  All rights reserved.
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
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 9.0     adk    12/10/13 Updated as per the New Tcl API's
# 9.0     bss    02/20/14 Added kintex 8 and kintexu families
##############################################################################
#--------------------------------
# Tcl procedure generate 
#--------------------------------

proc generate {drv_handle} {

    # This returns the C_FAMILY parameter from the processor.
    # MicroBlaze has C_FAMILY defined, but PowerPC does not.
    set sw_proc_handle [get_sw_processor]
    set prochandle [get_cells [get_property HW_INSTANCE $sw_proc_handle] ]
    set proctype [string tolower [get_property SPECIAL $prochandle]]
    set family [string tolower [get_property CONFIG.C_FAMILY $prochandle]]
     
    # Create a definition in a header file
    set filename  "./src/xhwicap_family.h"
    set filehandle [ open $filename a ]
    xprint_generated_header $filehandle "Device family" 
    if {[string compare $family "kintex7"] == 0} {
    	puts $filehandle "#define XHI_FPGA_FAMILY 7\n"   	
    } elseif {[string compare $family "virtex7"] == 0} {
	    puts $filehandle "#define XHI_FPGA_FAMILY 8\n"
    } elseif {[string compare $family "artix7"] == 0} {
	    puts $filehandle "#define XHI_FPGA_FAMILY 9\n"
    } elseif {[string compare $family "zynq"] == 0} {
	    puts $filehandle "#define XHI_FPGA_FAMILY 10\n"
    } elseif {[string compare $family "kintex8"] == 0} {
    	    puts $filehandle "#define XHI_FPGA_FAMILY 11\n"
    } elseif {[string compare $family "kintexu"] == 0} {
    	    puts $filehandle "#define XHI_FPGA_FAMILY 11\n"
    } else {
            puts $filehandle "#define XHI_FPGA_FAMILY 1\n"
    }
    close $filehandle


    # Generate #defines in xparameters.h
    xdefine_include_file $drv_handle "xparameters.h" "XHwIcap" "NUM_INSTANCES" "C_BASEADDR" "C_HIGHADDR" "DEVICE_ID" "C_ICAP_DWIDTH" "C_MODE"
       
    # Generate the _g.c configuration file
    xdefine_config_file $drv_handle "xhwicap_g.c" "XHwIcap"  "DEVICE_ID" "C_BASEADDR" "C_ICAP_DWIDTH" "C_MODE" 
       
    xdefine_canonical_xpars $drv_handle "xparameters.h" "HwIcap" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_ICAP_DWIDTH" "C_MODE"

}


