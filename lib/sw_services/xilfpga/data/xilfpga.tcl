###############################################################################
#
# Copyright (C) 2016 - 2019 Xilinx, Inc.  All rights reserved.
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
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# 
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
# 4.1	Nava   27/03/18 For Secure Bitstream loading to avoid the Security violations
#                      Need to Re-validate the User Crypto flags with the Image
#                      Crypto operation by using the internal memory.To Fix this
#                      added a new API XFpga_ReValidateCryptoFlags().
# 4.1	Nava   16/04/18  Added partial bitstream loading support.
# 4.2	Nava  30/05/18 Refactor the xilfpga library to support
#                      different PL programming Interfaces.
# 4.2	adk   24/07/18 Added proper error message if xilsecure is not enabled
#			in the bsp.
# 5.0   Nava  11/05/18  Added full bitstream loading support for versal Platform.
# 5.0	sne   27/03/19 Fixed Misra-C violations.
# 5.0   Nava  29/03/19  Removed vesal platform related changes.As per the new
#                       design, the Bitstream loading for versal platform is
#                       done by PLM based on the CDO's data exists in the PDI
#                       images. So there is no need of xilfpga API's for versal
#                       platform to configure the PL.
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

    file copy "src/xilfpga.h"  "../../include/xilfpga.h"

    set conffile  [xfpga_open_include_file "xfpga_config.h"]
    set zynqmp "src/interface/zynqmp/"
    set interface "src/interface/"
    set cortexa53proc [hsi::get_cells -hier -filter "IP_NAME==psu_cortexa53"]
    if {[llength $cortexa53proc] > 0} {
	set iszynqmp 1
    } else {
	set iszynqmp 0
    }
    if { $iszynqmp == 1} {
	set librarylist [hsi::get_libs -filter "NAME==xilsecure"];
	if { [llength $librarylist] == 0 } {
	    error "This library requires xilsecure library in the Board Support Package.";
	}
	set def_flags [common::get_property APP_LINKER_FLAGS [hsi::current_sw_design]]
	set new_flags "-Wl,--start-group,-lxilfpga,-lxil,-lxilsecure,-lgcc,-lc,--end-group $def_flags"
	set_property -name APP_LINKER_FLAGS -value $new_flags -objects [current_sw_design]

	foreach entry [glob -nocomplain [file join $zynqmp *]] {
            file copy -force $entry "./src"
        }
    } else {
		error "This library supports Only ZyqnMP platform."
    }
    file delete -force $interface
    puts $conffile "#ifndef _XFPGA_CONFIG_H"
    puts $conffile "#define _XFPGA_CONFIG_H"
    puts $conffile "#include <xilfpga.h>"
    set value  [common::get_property CONFIG.ocm_address $lib_handle]
    puts  $conffile "#define XFPGA_OCM_ADDRESS ${value}U"
    set value  [common::get_property CONFIG.base_address $lib_handle]
    puts  $conffile "#define XFPGA_BASE_ADDRESS ${value}U"
    set value  [common::get_property CONFIG.secure_mode $lib_handle]

    if {$value == true} {
	puts $conffile "#define XFPGA_SECURE_MODE"
    }

   set value  [common::get_property CONFIG.debug_mode $lib_handle]

   if {$value == true} {
        puts $conffile "#define XFPGA_DEBUG	(1U)"
    } else {
	puts $conffile "#define XFPGA_DEBUG     (0U)"
    }
    puts $conffile "#endif"
    close $conffile
}
