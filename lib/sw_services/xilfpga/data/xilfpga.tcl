###############################################################################
#
# Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.0   Nava 08/06/16 First release
# 1.1   Nava  16/11/16 Added PL power-up sequence.
# 2.0   Nava  10/1/17  Added Encrypted bitstream loading support.
# 2.0   Nava  16/02/17 Added Authenticated bitstream loading support.
# 2.1   Nava  06/05/17 Correct the check logic issues in
#                      XFpga_PL_BitStream_Load()
#                      to avoid the unwanted blocking conditions.
# 3.0   Nava  12/05/17 Added PL configuration registers readback support.
# 4.0   Nava  08/02/18 Added Authenticated and Encypted Bitstream loading support.
#
##############################################################################

#---------------------------------------------
# fpga_drc
#---------------------------------------------
proc fpga_drc {libhandle} {
	# check processor type
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]

	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];

}

proc xfpga_open_include_file {file_name} {
    set filename [file join "../../include/" $file_name]
    if {[file exists $filename]} {
        set config_inc [open $filename a]
    } else {
        set config_inc [open $filename a]
        ::hsi::utils::write_c_header $config_inc "MFS Parameters"
   }
    return $config_inc
}

proc generate {lib_handle} {

    puts "XFPGA generate ..."
    file copy "src/xilfpga_pcap.h"  "../../include/xilfpga_pcap.h"

    set conffile  [xfpga_open_include_file "xfpga_config.h"]

    puts $conffile "#ifndef _XFPGA_CONFIG_H"
    puts $conffile "#define _XFPGA_CONFIG_H"
    set value  [common::get_property CONFIG.ocm_address $lib_handle]
    puts  $conffile "#define XFPGA_OCM_ADDRESS $value"
    set value  [common::get_property CONFIG.base_address $lib_handle]
    puts  $conffile "#define XFPGA_BASE_ADDRESS $value"
    set value  [common::get_property CONFIG.secure_mode $lib_handle]

    if {$value == true} {
	puts $conffile "#define XFPGA_SECURE_MODE"
    }

    puts $conffile "#endif"
    close $conffile
}
