###############################################################################
# Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
###############################################################################
#
# Modification History
#
# Ver  Who Date     Changes
# ---- --- -------- -----------------------------------------------
# 1.0  sg  09/12/17 First Release
#
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XUartPsv" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_UART_CLK_FREQ_HZ" "C_HAS_MODEM"

    ::hsi::utils::define_zynq_config_file $drv_handle "xuartpsv_g.c" "XUartPsv"  "DEVICE_ID" "C_S_AXI_BASEADDR" "C_UART_CLK_FREQ_HZ" "C_HAS_MODEM"

    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "XUartPsv" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_UART_CLK_FREQ_HZ" "C_HAS_MODEM"

}
