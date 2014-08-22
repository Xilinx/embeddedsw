/******************************************************************************
*
*       XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
*       AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND
*       SOLUTIONS FOR XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE,
*       OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
*       APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION
*       THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
*       AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
*       FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY
*       WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
*       IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
*       REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
*       INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*       FOR A PARTICULAR PURPOSE.
*
*       (c) Copyright 2014 Xilinx Inc.
*       All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file scaler_intg_util.c
*
* This file contains utility functions used by all integration test on
* Scaler driver.
*
* @note
*
* This code works for Zynq702 system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------
* 7.0   adk   22/08/14 First release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "scaler_intgtest.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

/* Instance of SCALER driver core struct XScaler. */
XScaler ScalerInst;		/**< Instance of XScaler */

/* Instance of the XIntc driver. */
XScuGic InterruptController;

/*****************************************************************************/
/**
*
* This function initializes the Scaler core.
*
* @param	InstancePtr to the instance under test.
* @param	DeviceId is unique ID which identifies the device
*
* @return	- XST_FAILURE if there are errors.
*		- XST_SUCCESS if there are no errors.
*
* @note		None.
*
******************************************************************************/
int Scaler_Initialize(XScaler *InstancePtr, u16 DeviceId)
{
	XScaler_Config *Config;
	int Status;

	/*
	 * Initialize the Scaler driver so that it's ready to use
	 * Look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XScaler_LookupConfig(DeviceId);
	if (NULL == Config)
	{
		return XST_FAILURE;
	}

	Status = XScaler_CfgInitialize(InstancePtr, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	/*
	 * Need to set standard and data_width if at all core is supported.
	 */

	return XST_SUCCESS;
}
