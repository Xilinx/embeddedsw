###############################################################################
# Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#
##############################################################################
#include "xcsi2tx.h"

XCsi2Tx_Config XCsi2Tx_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,mipi-csi2-tx-ctrl-1.0", /* compatible */
		0x0, /* reg */
		0x4, /* xlnx,csi-lanes */
		0x0, /* xlnx,csi-en-activelanes */
		0x0 /* xlnx,en-reg-based-fe-gen */
	},
	 {
		 NULL
	}
};
