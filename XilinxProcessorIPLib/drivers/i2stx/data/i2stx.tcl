################################################################################
# Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
################################################################################

proc generate {drv_handle} {
  ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XI2STX" \
	"NUM_INSTANCES" \
	"DEVICE_ID" \
	"C_BASEADDR" \
	"C_HIGHADDR" \
	"C_DWIDTH" \
	"C_IS_MASTER" \
	"C_NUM_CHANNELS" \
	"C_32BIT_LR"

  ::hsi::utils::define_config_file $drv_handle "xi2stx_g.c" "XI2stx" \
	"DEVICE_ID" \
	"C_BASEADDR" \
	"C_DWIDTH" \
	"C_IS_MASTER" \
	"C_NUM_CHANNELS" \
	"C_32BIT_LR"

  ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "XI2STX" \
	"DEVICE_ID" \
	"C_BASEADDR" \
	"C_HIGHADDR" \
	"C_DWIDTH" \
	"C_IS_MASTER" \
	"C_NUM_CHANNELS" \
	"C_32BIT_LR"
}
