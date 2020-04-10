################################################################################
# Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
################################################################################
################################################################################

proc generate {drv_handle} {
  ::hsi::utils::define_include_file $drv_handle "xparameters.h" "XSPDIF" \
	"NUM_INSTANCES" \
	"DEVICE_ID" \
	"C_BASEADDR" \
	"C_HIGHADDR"

  ::hsi::utils::define_config_file $drv_handle "xspdif_g.c" "XSpdif" \
	"DEVICE_ID" \
	"C_BASEADDR"

  ::hsi::utils::define_canonical_xpars $drv_handle "xparameters.h" "XSPDIF" \
	"DEVICE_ID" \
	"C_BASEADDR"

}
