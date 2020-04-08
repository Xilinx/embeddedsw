###############################################################################
# Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00a sdm  11/22/11 Created
# 2.02a hk   26/03/13 Added C_QSPI_MODE
# 3.4   nsk  31/07/17 Added C_QSPI_BUS_WIDTH
#
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XQspiPs" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_QSPI_CLK_FREQ_HZ" "C_QSPI_MODE" "C_QSPI_BUS_WIDTH"

    ::hsi::utils::define_zynq_config_file $drv_handle "xqspips_g.c" "XQspiPs"  "DEVICE_ID" "C_S_AXI_BASEADDR" "C_QSPI_CLK_FREQ_HZ" "C_QSPI_MODE"

    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "XQspiPs" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_QSPI_CLK_FREQ_HZ" "C_QSPI_MODE" "C_QSPI_BUS_WIDTH"

}
