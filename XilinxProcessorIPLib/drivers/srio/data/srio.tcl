###############################################################################
# Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- --------------------------------------------------------
# 1.0   adk  16/04/14 Initial release 
#
##############################################################################

proc generate {drv_handle} {
    ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XSrio" "NUM_INSTANCES" \
				     "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR"  \
				     "C_DEVICEID" 
    ::hsi::utils::define_config_file $drv_handle "xsrio_g.c" "XSrio"  "DEVICE_ID" "C_BASEADDR"

    ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "Srio" "DEVICE_ID" "C_BASEADDR"
}




