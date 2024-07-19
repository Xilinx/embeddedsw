###############################################################################
# Copyright (C) 2024 AMD, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
###############################################################################
#
# Modification History
#
# Ver  Who Date     Changes
# ---- --- -------- -----------------------------------------------
# 1.0  gm  02/09/24 First Release
#
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XI3c"  "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_AXI_CLK_FREQ" "C_WR_RD_FIFO_DEPTH" "C_WR_FIFO_PROG_FULL_THRESHOLD" "NUM_TARGETS" "C_IBI_CAPABLE" "C_HJ_CAPABLE"

    ::hsi::utils::define_zynq_config_file $drv_handle "xi3c_g.c" "XI3c"  "DEVICE_ID" "C_BASEADDR" "C_AXI_CLK_FREQ" "C_WR_RD_FIFO_DEPTH" "C_WR_FIFO_PROG_FULL_THRESHOLD" "NUM_TARGETS" "C_IBI_CAPABLE" "C_HJ_CAPABLE"

    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "XI3c" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_AXI_CLK_FREQ" "C_WR_RD_FIFO_DEPTH" "C_WR_FIFO_PROG_FULL_THRESHOLD" "NUM_TARGETS" "C_IBI_CAPABLE" "C_HJ_CAPABLE"

}
