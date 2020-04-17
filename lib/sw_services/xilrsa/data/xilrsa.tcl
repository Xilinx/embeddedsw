###############################################################################
# Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
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

    set procdrv [hsi::get_sw_processor]
    set compiler [common::get_property CONFIG.compiler $procdrv]
        if {[string compare -nocase $compiler "arm-none-eabi-gcc"] == 0} {
		file delete -force ./src/librsa_armcc.a
        } elseif {[string compare -nocase $compiler "armcc"] == 0} {
		file delete -force ./src/librsa.a
		file rename -force ./src/librsa_armcc.a ./src/librsa.a
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
