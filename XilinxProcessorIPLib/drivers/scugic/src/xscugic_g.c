/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xscugic_g.c
* @addtogroup scugic_v4_4
* @{
*
* This file contains a configuration table that specifies the configuration of
* interrupt controller devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a drg  01/19/10 First release
* 3.00  kvn  02/13/15 Modified code for MISRA-C:2012 compliance.
* 3.10  mus  07/17/18 Updated file to fix the various coding style issues
*                     reported by checkpatch. It fixes CR#1006344.
*
* </pre>
*
* @internal
*
* This configuration table contains entries that are modified at runtime by the
* driver. This table reflects only the hardware configuration of the device.
* This Intc configuration table contains software information in addition to
* hardware configuration.
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xscugic.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for each GIC device
 * in the system. The XScuGic driver must know when to acknowledge the
 * interrupt. The entry which specifies this as a bit mask where each bit
 * corresponds to a specific interrupt.  A bit set indicates to ACK it
 * before servicing it. Generally, acknowledge before service is used when
 * the interrupt signal is edge-sensitive, and after when the signal is
 * level-sensitive.
 *
 * Refer to the XScuGic_Config data structure in xscugic.h for details on how
 * this table should be initialized.
 */
XScuGic_Config XScuGic_ConfigTable[XPAR_XSCUGIC_NUM_INSTANCES] = {
	{
		(u16)XPAR_SCUGIC_0_DEVICE_ID,  /* Unique ID  of device */
		(u32)XPAR_SCUGIC_0_CPU_BASEADDR,  /* CPU Interface base address */
		(u32)XPAR_SCUGIC_0_DIST_BASEADDR,  /* Distributor base address */
		{{0}}  /**< Initialize the HandlerTable to 0 */
	}
};
/** @} */
