/******************************************************************************
* Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
* @file xaxipcie_g.c
* @addtogroup axipcie_v3_3
* @{
*
* This file contains a configuration table that specifies the configuration
* of AXI PCIe devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.00a rkv  03/03/11 Original code.
* 2.00a rkv  07/19/11  Added support of pcie root complex functionality.
*
* </pre>
*
* @note
*
* None.
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "xparameters.h"
#include "xaxipcie.h"

/************************** Constant Definitions ****************************/


/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/


/************************** Function Prototypes *****************************/


/************************** Variable Prototypes *****************************/

XAxiPcie_Config XAxiPcie_ConfigTable[] = {
	{
	 XPAR_AXIPCIE_0_DEVICE_ID,
	 XPAR_AXIPCIE_0_BASEADDR,
	 XPAR_AXIPCIE_0_AXI_BAR_NUM,
	 XPAR_AXIPCIE_0_INCLUDE_BAROFFSET_REG,
	 XPAR_AXI_PCIE_0_INCLUDE_RC
	}
};

/** @} */
