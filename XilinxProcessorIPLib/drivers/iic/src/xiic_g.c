/******************************************************************************
* Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xiic_g.c
* @brief Provides the IIC configuration table.
*
* This file contains a configuration table that specifies the configuration of
* IIC devices in the system. Each IIC device should have an entry in this table.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- --- ------- -----------------------------------------------
* 1.01a rfp  10/19/01 release
* 1.01c ecm  12/05/02 new rev
* 1.01d jhl  10/08/03 Added general purpose output feature
* 1.13a wgr  03/22/07 Converted to new coding style.
* 3.16  tb   09/22/26 Align Doxygen comments with AMD documentation standards
* </pre>
*
******************************************************************************/

/**
* @addtogroup iic_api IIC APIs
* @{
*/

/***************************** Include Files *********************************/

#include "xiic.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

/**
 * @brief Holds the IIC configuration table.
 *
 * Sized by the instance count in xparameters.h.
 */
XIic_Config XIic_ConfigTable[XPAR_XIIC_NUM_INSTANCES] = {
	{
	 XPAR_IIC_0_DEVICE_ID,	/* Device ID for instance */
	 XPAR_IIC_0_BASEADDR,	/* Base address */
	 XPAR_IIC_0_TEN_BIT_ADR,/* Uses 10 bit addressing */
	 XPAR_IIC_0_GPO_WIDTH	/* Number of bits in GPO register */
	}
	,
	{
	 XPAR_IIC_1_DEVICE_ID,	/* Device ID for instance */
	 XPAR_IIC_1_BASEADDR,	/* Base address */
	 XPAR_IIC_1_TEN_BIT_ADR, /* Uses 10 bit addressing */
	 XPAR_IIC_1_GPO_WIDTH	/* Number of bits in GPO register */
	}
};
/** @} */
