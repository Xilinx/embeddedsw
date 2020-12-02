##############################################################################
# Copyright (c) 2011 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
##
#############################################################################

#uses "xillib.tcl"


proc generate {drv_handle} {
    ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XDEINT" "NUM_INSTANCES" "DEVICE_ID" "C_DIAG" "C_MOTION" "C_DEPTH" "C_STREAMS" "C_TRIPLE_PORT" "C_MAX_XSIZE" "C_BASEADDR" "C_HIGHADDR"

    ::hsi::utils::define_config_file $drv_handle "xdeint_g.c" "XDeint" "DEVICE_ID" "C_BASEADDR"

    ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "Deinterlacer" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR"
}
