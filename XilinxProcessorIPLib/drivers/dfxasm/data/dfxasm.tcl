###############################################################################
# Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
# Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
###############################################################################
#
# Modification History
#
#  Ver   Who      Date       Changes
# ----- ----  ----------  -----------------------------------------------
#  1.0   dp    07/14/20     First Release
#
##############################################################################

proc generate {drv_handle} {
    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XDFXASM" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR"

    ::hsi::utils::define_zynq_config_file $drv_handle "xdfxasm_g.c" "XDfxasm" "DEVICE_ID" "C_BASEADDR"

    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "DFX_ASM" "DEVICE_ID" "C_BASEADDR"

}
