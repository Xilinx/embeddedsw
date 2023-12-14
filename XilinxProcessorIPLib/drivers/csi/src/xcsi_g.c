/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/
#ifndef SDT
#include "xparameters.h"
#else
#include "xcsi.h"
#endif

#ifndef SDT
/*
* The configuration table for devices
*/

XCsi_Config XCsi_ConfigTable[] =
{
	{
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_CSI2_RX_CTRL_0_DEVICE_ID,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_CSI2_RX_CTRL_0_BASEADDR,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_CSI2_RX_CTRL_0_CSI_LANES,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_CSI2_RX_CTRL_0_CSI_OFFLOAD_NONIMAGE,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_CSI2_RX_CTRL_0_CSI_EN_VC_SUPPORT,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_CSI2_RX_CTRL_0_CSI_FIXED_VC,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_CSI2_RX_CTRL_0_CSI_OPT3_FIXEDLANES
	}
};

#else
XCsi_Config XCsi_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,mipi-csi2-rx-ctrl-1.0", /* compatible */
		0x0, /* reg */
		0x4, /* xlnx,csi-lanes */
		0x0, /* xlnx,csi-offload-non-image */
		0x1, /* xlnx,csi-en-vc-support */
		0x0, /* xlnx,csi-fixed-vc */
		0x0 /* xlnx,csi-optt3-fixed-lanes */
	},
	 {
		 NULL
	}
};
#endif
