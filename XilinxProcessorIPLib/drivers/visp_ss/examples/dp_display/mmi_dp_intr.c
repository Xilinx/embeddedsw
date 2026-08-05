/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file mmi_dp_intr.c
*
* This file contains HPD interrupt callbacks and DP interrupt setup for
* the DPDC non-live application.
*
******************************************************************************/

#include <xil_printf.h>
#include <sleep.h>

#include "xmmidp.h"
#include "mmi_dp_intr.h"
#include "mmi_dp_init.h"

/************************** Constant Definitions *****************************/

#define XMMIDP_INTR_ID		0x40b2
#define XMMIDP_INTR_PARENT	0xe2000000

/*****************************************************************************/
/**
*
* This function disables DP-side audio SDP/timestamp streams.
*
* @param	DpPtr is a pointer to the XMmiDp instance.
*
* @return	None.
*
******************************************************************************/
void XDpDc_DisableDpAudioSdpStreams(XMmiDp *DpPtr)
{
	XMmiDp_SetSdpVertAudStreamEn(DpPtr, XMMIDP_STREAM_ID1, 0U);
	XMmiDp_SetSdpVertAudTimeStampEn(DpPtr, XMMIDP_STREAM_ID1, 0U);
	XMmiDp_SetSdpHorAudStreamEn(DpPtr, XMMIDP_STREAM_ID1, 0U);
	XMmiDp_SetSdpHorAudTimeStampEn(DpPtr, XMMIDP_STREAM_ID1, 0U);
	XMmiDp_ConfigureAudioController(DpPtr, XMMIDP_STREAM_ID1);
}

/*****************************************************************************/
/**
*

* This function is the HPD hot plug (connect) callback. It reads sink
* capabilities, runs link training, and sets up the video stream.
*
* @param	CallbackRef is a pointer to the RunConfig structure.
*
* @return	None.
*
******************************************************************************/
void XDpDc_HpdHotplugHandler(void *CallbackRef)
{
	RunConfig *RunCfgPtr = (RunConfig *)CallbackRef;
	XMmiDp *DpPtr = RunCfgPtr->DpPsuPtr;
	u32 Status;

	xil_printf("\r\n[HPD] Hot plug detected -- starting connect sequence\r\n");

	XMmiDp_SinkPowerUp(DpPtr);
	sleep(5);

	Status = XMmiDp_StartFullLinkTrainingCapped(DpPtr,
						    RunCfgPtr->MaxLaneCount,
						    RunCfgPtr->MaxLinkRate);
	if (Status != XST_SUCCESS) {
		xil_printf("[HPD] ERROR: Link training failed\r\n");
		return;
	}

	XMmiDp_SetupVideoStream(RunCfgPtr);
	if (RunCfgPtr->AudioEnable)
		XMmiDp_SetupAudioStream(RunCfgPtr);
	else if (RunCfgPtr->SdpEnable == 0U)
		XDpDc_DisableDpAudioSdpStreams(DpPtr);

	xil_printf("[HPD] Connect sequence complete\r\n");
}

/*****************************************************************************/
/**
*

* This function is the HPD hot unplug (disconnect) callback. It disables
* the video stream and logs the disconnect event.
*
* @param	CallbackRef is a pointer to the RunConfig structure.
*
* @return	None.
*
******************************************************************************/
void XDpDc_HpdHotunplugHandler(void *CallbackRef)
{
	RunConfig *RunCfgPtr = (RunConfig *)CallbackRef;
	XMmiDp *DpPtr = RunCfgPtr->DpPsuPtr;

	xil_printf("\r\n[HPD] Hot unplug detected\r\n");

	DpPtr->LinkConfig.LinkTrained = 0;
	XMmiDp_DisableVideoStream(DpPtr, XMMIDP_STREAM_ID1);
	xil_printf("[HPD] Video stream disabled\r\n");
}

/*****************************************************************************/
/**
*

* This function is the HPD IRQ (short pulse / sink request) callback.
* It reads the DPCD Device Service IRQ Vector and Link Service IRQ Vector
* to determine the cause of the interrupt and takes appropriate action.
*
* @param	CallbackRef is a pointer to the RunConfig structure.
*
* @return	None.
*
******************************************************************************/
void XDpDc_HpdIrqHandler(void *CallbackRef)
{
	RunConfig *RunCfgPtr = (RunConfig *)CallbackRef;
	XMmiDp *DpPtr = RunCfgPtr->DpPsuPtr;
	u8 DevIrqVec;
	u8 LinkIrqVec;
	u32 Status;

	xil_printf("\r\n[HPD] IRQ (sink request)\r\n");

	/* DPCD 0x201: Device Service IRQ Vector */
	Status = XMmiDp_AuxRead(DpPtr, 0x201, 1, &DevIrqVec);
	if (Status != XST_SUCCESS) {
		xil_printf("[HPD] ERROR: Failed to read DPCD 0x201\r\n");
		return;
	}

	/* DPCD 0x2005: Link Service IRQ Vector (DP 1.4+) */
	Status = XMmiDp_AuxRead(DpPtr, 0x2005, 1, &LinkIrqVec);
	if (Status != XST_SUCCESS)
		LinkIrqVec = 0;

	xil_printf("[HPD] DevIrqVec=0x%02X  LinkIrqVec=0x%02X\r\n",
		   DevIrqVec, LinkIrqVec);

	if (DevIrqVec & 0x04)
		xil_printf("[HPD] Automated test request (not handled)\r\n");

	if (DevIrqVec & 0x08)
		xil_printf("[HPD] CP (HDCP) IRQ\r\n");

	if (DevIrqVec & 0x40)
		xil_printf("[HPD] Sink-specific IRQ\r\n");

	/* Skip link status check if no link was previously established */
	if (!DpPtr->LinkConfig.LinkTrained) {
		xil_printf("[HPD] No link previously trained -- skipping retrain\r\n");
		return;
	}

	/* Check link status and retrain if degraded */
	Status = XMmiDp_CheckLinkStatus(DpPtr,
					DpPtr->LinkConfig.LaneCount);
	if (Status != XST_SUCCESS) {
		xil_printf("[HPD] Link status degraded -- retraining\r\n");

		DpPtr->LinkConfig.LinkTrained = 0;
		XMmiDp_DisableVideoStream(DpPtr, XMMIDP_STREAM_ID1);

		Status = XMmiDp_StartFullLinkTrainingCapped(DpPtr,
							    RunCfgPtr->MaxLaneCount,
							    RunCfgPtr->MaxLinkRate);
		if (Status != XST_SUCCESS) {
			xil_printf("[HPD] ERROR: Retrain failed\r\n");
			return;
		}

		XMmiDp_SetupVideoStream(RunCfgPtr);
		if (RunCfgPtr->AudioEnable)
			XMmiDp_SetupAudioStream(RunCfgPtr);
		else if (RunCfgPtr->SdpEnable == 0U)
			XDpDc_DisableDpAudioSdpStreams(DpPtr);
		xil_printf("[HPD] Retrain complete, video restored\r\n");
	} else {
		xil_printf("[HPD] Link status OK\r\n");
	}
}

/*****************************************************************************/
/**
*
* This function registers HPD callbacks and connects the DP interrupt to
* the GIC.
*
* @param	RunCfgPtr is a pointer to the application configuration structure.
*
* @return	None.
*
******************************************************************************/
void XDpDc_SetupDpInterrupts(RunConfig *RunCfgPtr)
{
	XMmiDp *DpPtr = RunCfgPtr->DpPsuPtr;
	u32 Status;

	XMmiDp_SetHpdHotPlugHandler(DpPtr, XDpDc_HpdHotplugHandler, RunCfgPtr);
	XMmiDp_SetHpdHotUnplugHandler(DpPtr, XDpDc_HpdHotunplugHandler, RunCfgPtr);
	XMmiDp_SetHpdIrqHandler(DpPtr, XDpDc_HpdIrqHandler, RunCfgPtr);

	Status = XSetupInterruptSystem(DpPtr, &XMmiDp_GeneralInterruptHandler,
				       XMMIDP_INTR_ID, XMMIDP_INTR_PARENT,
				       XINTERRUPT_DEFAULT_PRIORITY);
	if (Status != XST_SUCCESS) {
		xil_printf("ERROR: DP interrupt registration failed (status=0x%X)\r\n",
			   Status);
		return;
	}

	xil_printf("DP interrupt registered (ID=0x%X parent=0x%X)\r\n",
		   XMMIDP_INTR_ID, XMMIDP_INTR_PARENT);
}
