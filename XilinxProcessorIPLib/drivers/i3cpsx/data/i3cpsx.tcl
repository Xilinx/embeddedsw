###############################################################################
# Copyright (C) 2022 AMD, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
###############################################################################
#
# Modification History
#
# Ver  Who Date     Changes
# ---- --- -------- -----------------------------------------------
# 1.0  sd  07/15/22 First Release
#
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XI3cPsx"  "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_I3C_CLK_FREQ_HZ" "SLAVES"

    ::hsi::utils::define_zynq_config_file $drv_handle "xi3cpsx_g.c" "XI3cPsx"  "DEVICE_ID" "C_S_AXI_BASEADDR" "C_I3C_CLK_FREQ_HZ" "SLAVES"

    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "XI3cPsx" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_I3C_CLK_FREQ_HZ" "SLAVES"

}
