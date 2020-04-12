###############################################################################
# Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.0   vve  14/08/18 Initial release
#
##############################################################################

proc generate {drv_handle} {
    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XAudioFormatter" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_INCLUDE_MM2S" "C_INCLUDE_S2MM" "C_MAX_NUM_CHANNELS_MM2S" "C_MAX_NUM_CHANNELS_S2MM" "C_MM2S_ADDR_WIDTH" "C_MM2S_DATAFORMAT" "C_PACKING_MODE_MM2S" "C_PACKING_MODE_S2MM" "C_S2MM_ADDR_WIDTH" "C_S2MM_DATAFORMAT"

    ::hsi::utils::define_zynq_config_file $drv_handle "xaudioformatter_g.c" "XAudioFormatter" "DEVICE_ID" "C_BASEADDR" "C_INCLUDE_MM2S" "C_INCLUDE_S2MM" "C_MAX_NUM_CHANNELS_MM2S" "C_MAX_NUM_CHANNELS_S2MM" "C_MM2S_ADDR_WIDTH" "C_MM2S_DATAFORMAT" "C_PACKING_MODE_MM2S" "C_PACKING_MODE_S2MM" "C_S2MM_ADDR_WIDTH" "C_S2MM_DATAFORMAT"

    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "XAudioFormatter" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" "C_INCLUDE_MM2S" "C_INCLUDE_S2MM" "C_MAX_NUM_CHANNELS_MM2S" "C_MAX_NUM_CHANNELS_S2MM" "C_MM2S_ADDR_WIDTH" "C_MM2S_DATAFORMAT" "C_PACKING_MODE_MM2S" "C_PACKING_MODE_S2MM" "C_S2MM_ADDR_WIDTH" "C_S2MM_DATAFORMAT"
}
