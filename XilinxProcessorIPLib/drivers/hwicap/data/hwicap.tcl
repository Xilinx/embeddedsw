###############################################################################
# Copyright (C) 2003 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
#
# MODIFICATION HISTORY:
# Ver      Who    Date     Changes
# -------- ------ -------- ------------------------------------
# 9.0     adk    12/10/13 Updated as per the New Tcl API's
# 9.0     bss    02/20/14 Added kintex 8 and kintexu families
# 10.0	  bss 	 06/24/14 Modified not to generate family.h so that
#		  	  all families are allowed.
##############################################################################
#--------------------------------
# Tcl procedure generate 
#--------------------------------

proc generate {drv_handle} {
    
    # Generate #defines in xparameters.h
    xdefine_include_file $drv_handle "xparameters.h" "XHwIcap" "NUM_INSTANCES" "C_BASEADDR" "C_HIGHADDR" "DEVICE_ID" "C_ICAP_DWIDTH" "C_MODE"
       
    # Generate the _g.c configuration file
    xdefine_config_file $drv_handle "xhwicap_g.c" "XHwIcap"  "DEVICE_ID" "C_BASEADDR" "C_ICAP_DWIDTH" "C_MODE" 
       
    xdefine_canonical_xpars $drv_handle "xparameters.h" "HwIcap" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_ICAP_DWIDTH" "C_MODE"

}


