/******************************************************************************
* Copyright (C) 2002 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xspi_stats.c
* @addtogroup spi_v4_7
* @{
*
* This component contains the implementation of statistics functions for the
* XSpi driver component.
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
* </pre>
*
******************************************************************************/

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
* Gets a copy of the statistics for an SPI device.
*
* @param	InstancePtr is a pointer to the XSpi instance to be worked on.
* @param	StatsPtr is a pointer to a XSpi_Stats structure which will get a
*		copy of current statistics.
*
* @return	None.
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
* Clears the statistics for the SPI device.
*
* @param	InstancePtr is a pointer to the XSpi instance to be worked on.
*
* @return	None.
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
