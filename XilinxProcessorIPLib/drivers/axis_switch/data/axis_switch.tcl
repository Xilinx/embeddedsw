##############################################################################
# Copyright (C) 2015 - 2022 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
##
#############################################################################

proc generate {drv_handle} {
    ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XAxis_Switch" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "NUM_SI" "NUM_MI"
    ::hsi::utils::define_config_file $drv_handle "xaxis_switch_g.c" "XAxis_Switch" "DEVICE_ID" "C_BASEADDR" "NUM_SI" "NUM_MI"
    ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "Axis_Switch" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "NUM_SI" "NUM_MI"
}