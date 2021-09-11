/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtrngpsv_g.c
* @addtogroup trngpsv_v1_0
* @{
*
* This file contains a configuration table that specifies the configuration of
* TRNG in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00  ssc  09/05/21 First release
*
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xtrngpsv.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Prototypes ******************************/

/*
* The configuration table for devices
*/

XTrngpsv_Config XTrngpsv_ConfigTable[XPAR_XTRNGPSV_NUM_INSTANCES] =
{
	{
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_PMC_TRNG_DEVICE_ID,
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_PMC_TRNG_BASEADDR
	}
};
/** @} */
