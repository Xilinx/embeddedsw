###############################################################################
#
# Copyright (C) 2014 Xilinx, Inc.  All rights reserved.
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
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
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
# 1.00  hk   27/01/14 Initial Release
# 1.1   hk   04/16/14 Error out when processor is not cortexa9. CR # 783276.
# 1.2   sk   01/28/16 Added support for CortexA9 with Linaro Tool chain.
#
##############################################################################

#---------------------------------------------
# rsa_drc 
#---------------------------------------------
proc rsa_drc {libhandle} {
    # check processor type
    set proc_instance [hsi::get_sw_processor];
    set hw_processor [common::get_property HW_INSTANCE $proc_instance]

    set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];
    
    if { $proc_type != "ps7_cortexa9" } {
                error "ERROR: This library is supported only for CortexA9 processors.";
                return;
    }    

}

proc generate {libhandle} {

}

#-------
# post_generate: called after generate called on all libraries
#-------
proc post_generate {libhandle} {
	
	xgen_opts_file $libhandle
}

#-------
# execs_generate: called after BSP's, libraries and drivers have been compiled
#-------
proc execs_generate {libhandle} {



}

proc xgen_opts_file {libhandle} {

}
