/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
# Copyright 2024-2025 Advanced Micro Devices, Inc. All Rights Reserved.
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

#ifdef XPAR_XV_HDMI_RX_FRL_ENABLE
/************************** Function Prototypes ******************************/

/************************** Function Definition ******************************/

/*****************************************************************************/
/**
*
* This function is called when the FRL link training requires configuration
* from application.
*
* @param    CallbackRef is a callback reference passed in by the upper
* 	    layer when setting the callback functions, and passed back to
* 	    the upper layer when the callback is invoked
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
* @param    CallbackRef is a callback reference passed in by the upper
* 	    layer when setting the callback functions, and passed back to
* 	    the upper layer when the callback is invoked
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
* Initiates FRL (Fixed Rate Link) link retraining on the HDMI RX subsystem.
* This function calls the lower-level FRL link retrain function using the HDMI RX1
* instance pointer from the subsystem. It is typically used to reinitialize the FRL
* link with a new LTP (Link Training Pattern) threshold and default LTP settings.
*
* @param	InstancePtr   Pointer to the HDMI RX Subsystem instance.
* @param	LtpThreshold  Threshold value for LTP error tolerance.
* @param	DefaultLtp    Default LTP configuration to use during retraining.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_HdmiRxSs1_FrlLinkRetrain(XV_HdmiRxSs1 *InstancePtr, u8 LtpThreshold,
		XV_HdmiRx1_FrlLtp DefaultLtp)
{
	XV_HdmiRx1_FrlLinkRetrain(InstancePtr->HdmiRx1Ptr, LtpThreshold,
							DefaultLtp);
}
/*****************************************************************************/
/**
*
* This function enables the FRL mode.
*
* @param    InstancePtr pointer to the HDMI RX Subsystem instance.
*
* @param    LtpThreshold specifies the number of times the LTP matching module
*           must match against the incoming link training pattern before a
*           match is indicated
*
* @param    DefaultLtp specify the link training pattern which will be used
*                       for link training purposes
*                       - XV_HDMIRX1_LTP_LFSR0
*                       - XV_HDMIRX1_LTP_LFSR1
*                       - XV_HDMIRX1_LTP_LFSR2
*                       - XV_HDMIRX1_LTP_LFSR3
* @param    FfeSuppFlag to specify the support of FFE Levels
*
* @note     None.
*
******************************************************************************/
void XV_HdmiRxSs1_FrlModeEnable(XV_HdmiRxSs1 *InstancePtr, u8 LtpThreshold,
							XV_HdmiRx1_FrlLtp DefaultLtp, u8 FfeSuppFlag)
{
	XV_HdmiRx1_FrlModeEnable(InstancePtr->HdmiRx1Ptr, LtpThreshold,
							DefaultLtp, FfeSuppFlag);
}

/*****************************************************************************/
/**
*
* This function executes the different of states of FRL.
*
* @param    InstancePtr pointer to the HDMI RX Subsystem instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
int XV_HdmiRxSs1_ExecFrlState(XV_HdmiRxSs1 *InstancePtr)
{
	return XV_HdmiRx1_ExecFrlState(InstancePtr->HdmiRx1Ptr);
}

/*****************************************************************************/
/**
*
* This function configures the HDMI RX to either enable or simulate the FLT_NO_TIMEOUT
* condition based on the current state of the `FltNoTimeout` flag in the FRL stream context.
* This is typically used for testing or managing FRL link training behavior in HDMI 2.1.
*
* @param        InstancePtr pointer to the HDMI RX Subsystem instance
*
* @note         None.
*
******************************************************************************/
void XV_HdmiRxSs1_SetFrlFltNoTimeout(XV_HdmiRxSs1 *InstancePtr)
{
	XV_HdmiRx1_SetFrlFltNoTimeout(InstancePtr->HdmiRx1Ptr);
}

/*****************************************************************************/
/**
* This function typically used to restore normal FRL link training timeout behavior
* after it was previously disabled for testing or debugging purposes.
*
* @param        InstancePtr pointer to the HDMI RX Subsystem instance.
*
* @note         None.
*
******************************************************************************/
void XV_HdmiRxSs1_ClearFrlFltNoTimeout(XV_HdmiRxSs1 *InstancePtr)
{
	XV_HdmiRx1_ClearFrlFltNoTimeout(InstancePtr->HdmiRx1Ptr);
}

#endif /* XPAR_XV_HDMI_RX_FRL_ENABLE */
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
