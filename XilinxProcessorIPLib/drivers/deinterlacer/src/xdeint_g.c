/******************************************************************************
* Copyright (c) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file XDeint_g.c
* @addtogroup deinterlacer_v3_4
* @{
*
* This file contains a template for configuration table of Xilinx Video
* Deinterlacer For a real hardware system, Xilinx Platform Studio (XPS) will
* automatically generate a real configuration table to match the configuration
* of the Deinterlacer devices.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------ -------- -------------------------------------------------------
* 1.00a rjh    07/10/11 First release
* 2.00a rjh    18/01/12 Updated for v_deinterlacer 2.00
* 3.2   adk    02/13/14 Adherence to Xilinx coding, Doxygen guidelines.
* </pre>
*
******************************************************************************/


/***************************** Include Files *********************************/

#include "xdeint.h"
#include "xparameters.h"

/**
* The configuration table for Video Deinterlacers devices
*/
XDeint_Config XDeint_ConfigTable[] = {
	{
		XPAR_FMC_SENSOR_INPUT_V_DEINTERLACER_1_DEVICE_ID,
		XPAR_FMC_SENSOR_INPUT_V_DEINTERLACER_1__BASEADDR
	}
};
/** @} */
