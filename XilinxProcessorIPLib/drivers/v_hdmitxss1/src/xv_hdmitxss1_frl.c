/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmitxss1_frl.c
*
* This is main code of Xilinx HDMI Transmitter Subsystem device driver.
* Please see xv_hdmitxss1.h for more details of the driver.
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
#include "xv_hdmitxss1_frl.h"
#include "xv_hdmitxss1.h"

/************************** Function Prototypes ******************************/

/************************** Function Definition ******************************/

/*****************************************************************************/
/**
*
* This function sets the FFE Levels.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs1_SetFfeLevels(XV_HdmiTxSs1 *InstancePtr, u8 FfeLevel)
{
	InstancePtr->HdmiTx1Ptr->Stream.Frl.FfeLevels = FfeLevel;
}

/*****************************************************************************/
/**
*
* This function returns the FFE Level for the selected lane.
*
* @param  None.
*
* @return FFE Level.
*
* @note   None.
*
******************************************************************************/
u8 XV_HdmiTxSs1_GetTxFfe(XV_HdmiTxSs1 *InstancePtr, u8 Lane)
{
	return InstancePtr->HdmiTx1Ptr->Stream.Frl.LaneFfeAdjReq.Byte[Lane];
}

/*****************************************************************************/
/**
*
* This function returns the FRL Rate.
*
* @param  None.
*
* @return FRL Rate.
*
* @note   None.
*
******************************************************************************/
u8 XV_HdmiTxSs1_GetFrlRate(XV_HdmiTxSs1 *InstancePtr)
{
	return InstancePtr->HdmiTx1Ptr->Stream.Frl.FrlRate;
}

/*****************************************************************************/
/**
*
* This function returns the FRL Line Rate.
*
* @param  None.
*
* @return FRL Line Rate.
*
* @note   None.
*
******************************************************************************/
u8 XV_HdmiTxSs1_GetFrlLineRate(XV_HdmiTxSs1 *InstancePtr)
{
	return InstancePtr->HdmiTx1Ptr->Stream.Frl.LineRate;
}

/*****************************************************************************/
/**
*
* This function returns the the number of active FRL lanes.
*
* @param  None.
*
* @return Number of active FRL lanes.
*
* @note   None.
*
******************************************************************************/
u8 XV_HdmiTxSs1_GetFrlLanes(XV_HdmiTxSs1 *InstancePtr)
{
	return InstancePtr->HdmiTx1Ptr->Stream.Frl.Lanes;
}

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
void XV_HdmiTxSs1_FrlConfigCallback(void *CallbackRef)
{
	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)CallbackRef;

#ifdef XV_HDMITXSS1_LOG_ENABLE
	XV_HdmiTxSs1_LogWrite(HdmiTxSs1Ptr, XV_HDMITXSS1_LOG_EVT_FRL_CFG, 0);
#endif

	/* Check if user callback has been registered */
	if (HdmiTxSs1Ptr->FrlConfigCallback) {
	  HdmiTxSs1Ptr->FrlConfigCallback(HdmiTxSs1Ptr->FrlConfigRef);
	}
}

/*****************************************************************************/
/**
*
* This function is called when the FRL  link training requires configuring of
* FFE.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs1_FrlFfeCallback(void *CallbackRef)
{
	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)CallbackRef;

	/* Check if user callback has been registered */
	if (HdmiTxSs1Ptr->FrlFfeCallback) {
	  HdmiTxSs1Ptr->FrlFfeCallback(HdmiTxSs1Ptr->FrlFfeRef);
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
void XV_HdmiTxSs1_FrlStartCallback(void *CallbackRef)
{
	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)CallbackRef;

#ifdef XV_HDMITXSS1_LOG_ENABLE
	XV_HdmiTxSs1_LogWrite(HdmiTxSs1Ptr, XV_HDMITXSS1_LOG_EVT_FRL_LT_PASS, 0);
#endif

	/* Check if user callback has been registered */
	if (HdmiTxSs1Ptr->FrlStartCallback) {
	  HdmiTxSs1Ptr->FrlStartCallback(HdmiTxSs1Ptr->FrlStartRef);
	}
}

/*****************************************************************************/
/**
*
* This function is called when sink requested for FRL to be stopped.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs1_FrlStopCallback(void *CallbackRef)
{
	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)CallbackRef;

	/* Check if user callback has been registered */
	if (HdmiTxSs1Ptr->FrlStopCallback) {
	  HdmiTxSs1Ptr->FrlStopCallback(HdmiTxSs1Ptr->FrlStopRef);
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
void XV_HdmiTxSs1_TmdsConfigCallback(void *CallbackRef)
{
	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)CallbackRef;

#ifdef XV_HDMITXSS1_LOG_ENABLE
	XV_HdmiTxSs1_LogWrite(HdmiTxSs1Ptr, XV_HDMITXSS1_LOG_EVT_TMDS_START, 0);
#endif

	/* Check if user callback has been registered */
	if (HdmiTxSs1Ptr->TmdsConfigCallback) {
	  HdmiTxSs1Ptr->TmdsConfigCallback(HdmiTxSs1Ptr->TmdsConfigRef);
	}
}

/*****************************************************************************/
/**
*
* This function starts the Legacy HDMI TMDS Mode.
*
* @return	Status on if TMDS mode is successfully started or not.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTxSs1_TmdsStart(XV_HdmiTxSs1 *InstancePtr)
{
	int Status = XST_FAILURE;
/*#ifdef XV_HDMITXSS1_LOG_ENABLE*/
/*	XV_HdmiTxSs1_LogWrite(InstancePtr, XV_HDMITXSS1_LOG_EVT_FRL_START, 0);*/
/*#endif*/

	Status = XV_HdmiTx1_FrlRate(InstancePtr->HdmiTx1Ptr,
			XHDMIC_MAXFRLRATE_NOT_SUPPORTED);
	XV_HdmiTx1_FrlModeEn(InstancePtr->HdmiTx1Ptr, FALSE);
	XV_HdmiTx1_FrlExecute(InstancePtr->HdmiTx1Ptr);

	InstancePtr->HdmiTx1Ptr->Stream.IsFrl = FALSE;

	return Status;
}

/*****************************************************************************/
/**
*
* This function starts the TMDS mode.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return	Status on if TMDS can be started or not.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTxSs1_StartTmdsMode(XV_HdmiTxSs1 *InstancePtr)
{
	return XV_HdmiTx1_StartTmdsMode(InstancePtr->HdmiTx1Ptr);
}

/*****************************************************************************/
/**
*
* This function starts the Fixed Rate Link Training.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
*
* @param    FrlRate specifies the FRL rate to be attempted
* 			- 0 = FRL Not Supported
* 			- 1 = 3 Lanes 3Gbps
* 			- 2 = 4 Lanes 3Gbps
*			- 3 = 4 Lanes 6Gbsp
*			- 4 = 4 Lanes 8Gbps
*			- 5 = 4 Lanes 10Gbps
*			- 6 = 4 Lanes 12Gbps
*
* @return	Status on if FrlTraining can be started or not.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTxSs1_StartFrlTraining(XV_HdmiTxSs1 *InstancePtr,
		XHdmiC_MaxFrlRate FrlRate)
{
#ifdef XV_HDMITXSS1_LOG_ENABLE
	XV_HdmiTxSs1_LogWrite(InstancePtr, XV_HDMITXSS1_LOG_EVT_FRL_START,
			FrlRate);
#endif

	return XV_HdmiTx1_StartFrlTraining(InstancePtr->HdmiTx1Ptr,
			FrlRate);
}

/*****************************************************************************/
/**
*
* This function sets maximum FRL Rate supported by the system.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
*
* @param    MaxFrlRate specifies maximum rates supported
* 			- 0 = FRL Not Supported
* 			- 1 = 3 Lanes 3Gbps
* 			- 2 = 4 Lanes 3Gbps
*			- 3 = 4 Lanes 6Gbsp
*			- 4 = 4 Lanes 8Gbps
*			- 5 = 4 Lanes 10Gbps
*			- 6 = 4 Lanes 12Gbps
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSs1_SetFrlMaxFrlRate(XV_HdmiTxSs1 *InstancePtr,
		XHdmiC_MaxFrlRate MaxFrlRate)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	XV_HdmiTx1_SetFrlMaxFrlRate(InstancePtr->HdmiTx1Ptr, MaxFrlRate);
}

/*****************************************************************************/
/**
*
* This function starts FRL stream. This should be called after the bridge,
* video, audio are all active.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTxSs1_FrlStreamStart(XV_HdmiTxSs1 *InstancePtr)
{
	return XV_HdmiTx1_FrlStreamStart(InstancePtr->HdmiTx1Ptr);
}

/*****************************************************************************/
/**
*
* This function stops FRL video stream.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTxSs1_FrlStreamStop(XV_HdmiTxSs1 *InstancePtr)
{
	return XV_HdmiTx1_FrlStreamStop(InstancePtr->HdmiTx1Ptr);
}

/*****************************************************************************/
/**
*
* This function stops FRL video stream.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSs1_SetFrlWrongLtp(XV_HdmiTxSs1 *InstancePtr)
{
	return XV_HdmiTx1_SetFrlWrongLtp(InstancePtr->HdmiTx1Ptr);
}

/*****************************************************************************/
/**
*
* This function stops FRL video stream.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSs1_ClearFrlWrongLtp(XV_HdmiTxSs1 *InstancePtr)
{
	return XV_HdmiTx1_ClearFrlWrongLtp(InstancePtr->HdmiTx1Ptr);
}

/*****************************************************************************/
/**
*
* This function sets the FRL LTP
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
*
* @param	Lanes specifies FRL operation lane
* 			- 3  = 3 lanes
*			- 4  = 4 lanes
*
* @param	Ltp is a FRL LTP Type
*
* @return
*
* @note     Debug purpose.
*
******************************************************************************/
void XV_HdmiTxSs1_SetFrlLtp(XV_HdmiTxSs1 *InstancePtr, u8 Lane,
			XV_HdmiTx1_FrlLtpType Ltp)
{
	XV_HdmiTx1_SetFrlLtp(InstancePtr->HdmiTx1Ptr, Lane, Ltp);
}

/*****************************************************************************/
/**
*
* This function sets the CKE Source for External
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSs1_SetFrlExtVidCke(XV_HdmiTxSs1 *InstancePtr)
{
	XV_HdmiTx1_FrlExtVidCkeSource(InstancePtr->HdmiTx1Ptr, TRUE);
}

/*****************************************************************************/
/**
*
* This function sets the CKE Source for Interal Generated
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSs1_SetFrlIntVidCke(XV_HdmiTxSs1 *InstancePtr)
{
	XV_HdmiTx1_FrlExtVidCkeSource(InstancePtr->HdmiTx1Ptr, FALSE);
}
