/*******************************************************************
* Copyright (C) 2010-2022 Xilinx, Inc. All rights reserved.*
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef SDT
#include "xparameters.h"
#else
#include "xcsiss.h"
#endif

#ifndef SDT
/*
* List of Sub-cores included in the subsystem
* Sub-core device id will be set by its driver in xparameters.h
*/

#define XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_CSI2_RX_CTRL_0_PRESENT	 1
#define XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_PRESENT	 1


/*
* List of Sub-cores excluded from the subsystem
*   - Excluded sub-core device id is set to 255
*   - Excluded sub-core baseaddr is set to 0
*/

#define XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_AXI_IIC_0_PRESENT 0
#define XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_AXI_IIC_0_DEVICE_ID 255
#define XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_AXI_IIC_0_BASEADDR 0

XCsiSs_Config XCsiSs_ConfigTable[] =
{
	{
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_DEVICE_ID,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_BASEADDR,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_HIGHADDR,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_CMN_INC_IIC,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_CMN_NUM_LANES,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_CMN_NUM_PIXELS,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_CMN_PXL_FORMAT,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_CMN_VC,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_CSI_BUF_DEPTH,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_CSI_EMB_NON_IMG,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_DPY_EN_REG_IF,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_DPY_LINE_RATE,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_CSI_EN_CRC,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_CSI_EN_ACTIVELANES,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_EN_CSI_V2_0,
		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_EN_VCX,
		{
			XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_AXI_IIC_0_PRESENT,
			XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_AXI_IIC_0_DEVICE_ID,
			XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_AXI_IIC_0_BASEADDR
		},
		{
			XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_CSI2_RX_CTRL_0_PRESENT,
			XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_CSI2_RX_CTRL_0_DEVICE_ID,
			XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_CSI2_RX_CTRL_0_BASEADDR
		},
		{
			XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_PRESENT,
			XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_DEVICE_ID,
			XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_MIPI_DPHY_0_BASEADDR
		},
	}
};

#else

XCsiSs_Config XCsiSs_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,mipi-csi2-rx-subsystem-5.4", /* compatible */
		0x80000000, /* reg */
		0x8000ffff, /* xlnx,highaddr */
		0x0, /* xlnx,cmn-inc-iic */
		0x4, /* xlnx,cmn-num-lanes */
		0x2, /* xlnx,cmn-num-pixels */
		0x2b, /* xlnx,cmn-pxl-format */
		0x10, /* xlnx,cmn-vc */
		0x4096, /* xlnx,csi-buf-depth */
		0x0, /* xlnx,csi-emb-non-img */
		0x1, /* xlnx,dpy-en-reg-if */
		0x1440, /* xlnx,dpy-line-rate */
		0x1, /* xlnx,csi-en-crc */
		0x1, /* xlnx,csi-en-activelanes */
		0x0, /* xlnx,en-csi-v2 */
		0x4, /* xlnx,dphy-lanes */
		0x0, /* axii2c-present */
		0x0, /* axii2c-connected */
		0x1, /* csirx-present */
		0x0, /* csirx-connected */
		0x1, /* mipi-dphy-present */
		0x1000, /* mipi-dphy-connected */
		0x405c, /* interrupts */
		0xf9010000 /* interrupt-parent */
	},
	 {
		 NULL
	}
};

#endif
