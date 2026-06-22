/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/
#include "xcsi.h"

XCsi_Config XCsi_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,mipi-csi2-rx-ctrl-1.0", /* compatible */
		0x0, /* reg */
		0x4, /* xlnx,csi-lanes */
		0x0, /* xlnx,csi-offload-non-image */
		0x1, /* xlnx,csi-en-vc-support */
		0x0, /* xlnx,csi-fixed-vc */
		0x0 /* xlnx,csi-opt3-fixedlanes */
	},
	 {
		 NULL
	}
};
