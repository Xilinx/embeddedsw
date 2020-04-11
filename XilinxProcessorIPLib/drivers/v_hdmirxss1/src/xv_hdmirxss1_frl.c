/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmirxss1_frl.c
*
* This is main code of Xilinx HDMI Transmitter Subsystem device driver.
* Please see xv_hdmirxss1.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  EB   22/05/18 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#include "xv_hdmirxss1_frl.h"
#include "xv_hdmirxss1.h"

/************************** Function Prototypes ******************************/

/************************** Function Definition ******************************/

/*****************************************************************************/
/**
*
* This function is called when the FRL link training requires configuration
* from application.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiRxSs1_FrlConfigCallback(void *CallbackRef)
{
	XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;

	/* Check if user callback has been registered */
	if (HdmiRxSs1Ptr->FrlConfigCallback) {
	  HdmiRxSs1Ptr->FrlConfigCallback(HdmiRxSs1Ptr->FrlConfigRef);
	}
}

/*****************************************************************************/
/**
*
* This function is called when the FRL link training passes and sink is ready
* to receive video, audio and control packets.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiRxSs1_FrlStartCallback(void *CallbackRef)
{
	XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;

#ifdef XV_HDMIRXSS1_LOG_ENABLE
	XV_HdmiRxSs1_LogWrite(HdmiRxSs1Ptr, XV_HDMIRXSS1_LOG_EVT_FRL_START, 0);
#endif

	/* Check if user callback has been registered */
	if (HdmiRxSs1Ptr->FrlStartCallback) {
	  HdmiRxSs1Ptr->FrlStartCallback(HdmiRxSs1Ptr->FrlStartRef);
	}
}

/*****************************************************************************/
/**
*
* This function is called when during FRL link training, it is decided to
* fallback to the legacy HDMI TMDS mode.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiRxSs1_TmdsConfigCallback(void *CallbackRef)
{
	XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;

	/* Check if user callback has been registered */
	if (HdmiRxSs1Ptr->TmdsConfigCallback) {
	  HdmiRxSs1Ptr->TmdsConfigCallback(HdmiRxSs1Ptr->TmdsConfigRef);
	}
}

void XV_HdmiRxSs1_FrlLinkRetrain(XV_HdmiRxSs1 *InstancePtr, u8 LtpThreshold,
		XV_HdmiRx1_FrlLtp DefaultLtp)
{
	XV_HdmiRx1_FrlLinkRetrain(InstancePtr->HdmiRx1Ptr, LtpThreshold,
							DefaultLtp);
}

void XV_HdmiRxSs1_FrlModeEnable(XV_HdmiRxSs1 *InstancePtr, u8 LtpThreshold,
							XV_HdmiRx1_FrlLtp DefaultLtp, u8 FfeSuppFlag)
{
	XV_HdmiRx1_FrlModeEnable(InstancePtr->HdmiRx1Ptr, LtpThreshold,
							DefaultLtp, FfeSuppFlag);
}

int XV_HdmiRxSs1_ExecFrlState(XV_HdmiRxSs1 *InstancePtr)
{
	return XV_HdmiRx1_ExecFrlState(InstancePtr->HdmiRx1Ptr);
}

void XV_HdmiRxSs1_SetFrlFltNoTimeout(XV_HdmiRxSs1 *InstancePtr)
{
	XV_HdmiRx1_SetFrlFltNoTimeout(InstancePtr->HdmiRx1Ptr);
}

void XV_HdmiRxSs1_ClearFrlFltNoTimeout(XV_HdmiRxSs1 *InstancePtr)
{
	XV_HdmiRx1_ClearFrlFltNoTimeout(InstancePtr->HdmiRx1Ptr);
}
