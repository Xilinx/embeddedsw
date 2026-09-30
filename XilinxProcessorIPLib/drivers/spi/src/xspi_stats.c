/******************************************************************************
* Copyright (C) 2002 - 2023 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xspi_stats.c
*
* @brief Implements the statistics functions for the XSpi driver component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00b jhl  03/14/02 First release
* 1.00b rpm  04/25/02 Changed macro naming convention
* 1.11a wgr  03/22/07 Converted to new coding style.
* 1.12a sv   03/28/08 Removed the call to the Macro for clearing statistics.
* 2.00a sv   07/30/08 Removed the call to the Macro for clearing statistics.
* 3.00a ktn  10/28/09 Updated all the register accesses as 32 bit access.
*		      Updated driver to use the HAL APIs/macros.
* 4.17  vn   08/26/26 Fix Doxygen warnings.
* 4.17  tb   09/30/26 Align Doxygen comments with AMD documentation standards
* </pre>
*
******************************************************************************/

/**
* @addtogroup spi_api SPI APIs
* @{
*/

/***************************** Include Files *********************************/

#include "xspi.h"
#include "xspi_i.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
*
* @brief Gets a copy of the statistics for an SPI device.
*
* @param[in]	InstancePtr Pointer to the XSpi instance to be worked on.
* @param[out] StatsPtr Pointer to a XSpi_Stats structure which gets a
*		copy of current statistics.
*
* @note		Statistics are not updated in polled mode of operation.
*
******************************************************************************/
void XSpi_GetStats(XSpi *InstancePtr, XSpi_Stats *StatsPtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(StatsPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	StatsPtr->ModeFaults = InstancePtr->Stats.ModeFaults;
	StatsPtr->XmitUnderruns = InstancePtr->Stats.XmitUnderruns;
	StatsPtr->RecvOverruns =  InstancePtr->Stats.RecvOverruns;
	StatsPtr->SlaveModeFaults = InstancePtr->Stats.SlaveModeFaults;
	StatsPtr->BytesTransferred = InstancePtr->Stats.BytesTransferred;
	StatsPtr->NumInterrupts = InstancePtr->Stats.NumInterrupts;
}

/*****************************************************************************/
/**
*
* @brief Clears the statistics for the SPI device.
*
* @param[in,out] InstancePtr Pointer to the XSpi instance to be worked on.
*
* @note		Statistics are not updated in polled mode of operation.
*
******************************************************************************/
void XSpi_ClearStats(XSpi *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->Stats.ModeFaults = 0;
	InstancePtr->Stats.XmitUnderruns = 0;
	InstancePtr->Stats.RecvOverruns = 0;
	InstancePtr->Stats.SlaveModeFaults = 0;
	InstancePtr->Stats.BytesTransferred = 0;
	InstancePtr->Stats.NumInterrupts = 0;

}
/** @} */
