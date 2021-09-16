/******************************************************************************
* Copyright (C) 2006 Vreelin Engineering, Inc.  All Rights Reserved.
* Copyright (C) 2007 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xusb_g.c
* @addtogroup usb_v5_5
* @{
*
* This file contains a configuration table that specifies the configuration of
* the USB devices in the system.
*
* @note		None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00a  hvm  2/22/07 First release
* 2.00a  hvm  12/2/08 Updated the configuration structure with the
*			INCLUDE_DMA option.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xusb.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Prototypes ******************************/

/**
 * This table contains the configuration information for each of the USB
 * devices in the system.
 */
XUsb_Config XUsb_ConfigTable[XPAR_XUSB_NUM_INSTANCES] = {
	{
	 XPAR_USB_0_DEVICE_ID,	/* Device ID for the instance */
	 XPAR_USB_0_BASEADDR,	/* Device base address */
	 XPAR_USB_0_INCLUDE_DMA /* DMA enable Option */
	}

};
/** @} */
