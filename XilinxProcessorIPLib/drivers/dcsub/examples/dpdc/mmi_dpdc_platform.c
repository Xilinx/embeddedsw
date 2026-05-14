/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file mmi_dpdc_platform.c
*
* This file contains platform initialization functions
*
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <xdcsub.h>
#include <xil_exception.h>
#include <xil_printf.h>
#include <xil_cache.h>
#include <xil_types.h>
#include <sleep.h>

#include "mmi_dpdc_platform.h"
#include "mmi_dp_init.h"
#include "mmi_dp_intr.h"
#include "mmi_dc_cursor.h"
#include "mmi_dc_sdp.h"
#include "mmi_dc_setup_frames.h"

/************************** Constant Definitions *****************************/

#define CLK_LOCK			1
#define PL_AUD_CLK_MULT			512

#define XDCDMA_IEN_VSYNC_INT_MASK	0x0000000C
#define XDCDMA_INTR_ID			179
#define XDCDMA_INTR_PARENT		0xe2000000

static RunConfig *XDpDc_IrqRunCfgPtr;
static u8 XDpDc_CursorToSdpPending;
static u32 XDpDc_CursorFramesRemaining;

static void XDpDc_DmaInterruptHandler(void *CallbackRef)
{
	XDcDma *DmaPtr = (XDcDma *)CallbackRef;

	XDcDma_InterruptHandler(DmaPtr);

	if (XDpDc_CursorToSdpPending && XDpDc_IrqRunCfgPtr != NULL) {
		if (XDpDc_CursorFramesRemaining > 0U)
			XDpDc_CursorFramesRemaining--;

		if (XDpDc_CursorFramesRemaining == 0U) {
			DmaPtr->SDP.SDP_TriggerStatus = 0U;
			XDpDc_SetupSdpDescriptor(XDpDc_IrqRunCfgPtr);
			XDpDc_ConfigureSdpDMA(XDpDc_IrqRunCfgPtr);
			XDpDc_CursorToSdpPending = 0U;
			xil_printf("  Cursor phase complete, switched channel 7 to SDP\r\n");
		}
	}
}

/*****************************************************************************/
/**
*
* This function waits for the clock wizard to lock
*
* @param	CfgPtr is a pointer to the clock wizard configuration
*
* @return	XST_SUCCESS if locked, else XST_FAILURE
*
* @note		None.
*
*****************************************************************************/
u32 XClk_WaitForLock(XClk_Wiz_Config *CfgPtr)
{
	u32 Count = 0;

	while (!XClk_Wiz_ReadReg(CfgPtr->BaseAddr, XCLK_WIZ_REG4_OFFSET) & CLK_LOCK) {
		if (Count == 10000) {
			return XST_FAILURE;
		}
		usleep(100);
		Count++;
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function initializes the Clock Wizard IP for video and audio clocks
*
* @param	RunCfgPtr is a pointer to the application configuration structure
*
* @return	XST_SUCCESS if successful, else XST_FAILURE
*
* @note		None.
*
*****************************************************************************/
u32 XDpDc_InitClkWiz(RunConfig *RunCfgPtr)
{
	XClk_Wiz InstancePtr, AudInstancePtr;
	XClk_Wiz_Config *CfgPtr, *AudCfgPtr;
	u32 Status = XST_SUCCESS;
	u32 Reg;
	u64 Rate;

	CfgPtr = XClk_Wiz_LookupConfig(CLK_WIZ_BASEADDR);
	if (!CfgPtr) {
		xil_printf("  ERROR: Failed to lookup Clock Wizard IP\r\n");
		return XST_FAILURE;
	}

	Status = XClk_Wiz_CfgInitialize(&InstancePtr, CfgPtr, CfgPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("  ERROR: Failed to initialize Clock Wizard IP\r\n");
		return XST_FAILURE;
	}
	InstancePtr.MinErr = 20000; /* we need 20KHz accuracy */

	xil_printf("  ClkWiz: BaseAddr=0x%08X  PrimInClkFreq=%llu Hz  MinErr=%llu\r\n",
		   (u32)CfgPtr->BaseAddr,
		   InstancePtr.Config.PrimInClkFreq,
		   InstancePtr.MinErr);

	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, XCLK_WIZ_REG25_OFFSET, 0);

	Status = XClk_Wiz_SetRateHz(&InstancePtr, RunCfgPtr->PixelClkHz);
	if (Status != XST_SUCCESS) {
		xil_printf("  ERROR: Failed to set Clock Wizard rate (SetRateHz returned %d)\r\n",
			   Status);
		return XST_FAILURE;
	}

	xil_printf("  ClkWiz: SetRateHz(%llu) -> M=%d  D=%d  O=%d\r\n",
		   RunCfgPtr->PixelClkHz,
		   InstancePtr.MVal, InstancePtr.DVal, InstancePtr.OVal);
	if (InstancePtr.DVal != 0 && InstancePtr.OVal != 0) {
		u64 ActualHz = InstancePtr.Config.PrimInClkFreq *
			       InstancePtr.MVal /
			       ((u64)InstancePtr.DVal * InstancePtr.OVal);
		xil_printf("  ClkWiz: Expected output = %llu Hz  (%llu.%03llu MHz)\r\n",
			   ActualHz, ActualHz / 1000000, (ActualHz % 1000000) / 1000);
	}

	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, XCLK_WIZ_RECONFIG_OFFSET,
						(XCLK_WIZ_RECONFIG_LOAD | XCLK_WIZ_RECONFIG_SADDR));

	Status = XClk_WaitForLock(CfgPtr);
	if (Status != XST_SUCCESS) {
		Reg = XClk_Wiz_ReadReg(CfgPtr->BaseAddr, XCLK_WIZ_REG4_OFFSET) & CLK_LOCK;
		xil_printf("  ERROR: Video clock not locked: 0x%x (expected: 0x1)\r\n", Reg);
	} else {
		xil_printf("  ClkWiz: PLL locked\r\n");
	}

	XClk_Wiz_GetRate(&InstancePtr, 0, &Rate);
	xil_printf("  ClkWiz: GetRate returned %llu Hz  (%llu.%03llu MHz)\r\n",
		   Rate, Rate / 1000000, (Rate % 1000000) / 1000);

	if(RunCfgPtr->AudioEnable)
	{

		AudCfgPtr = XClk_Wiz_LookupConfig(CLK_WIZ_AUD_BASEADDR);
		if (!AudCfgPtr) {
			xil_printf("  ERROR: Failed to lookup Audio Clock Wizard IP\r\n");
			return XST_FAILURE;
		}

		Status = XClk_Wiz_CfgInitialize(&AudInstancePtr, AudCfgPtr, AudCfgPtr->BaseAddr);
		if (Status != XST_SUCCESS) {
			xil_printf("  ERROR: Failed to initialize Audio Clock Wizard IP\r\n");
			return XST_FAILURE;
		}

		XClk_Wiz_WriteReg(AudCfgPtr->BaseAddr, XCLK_WIZ_REG25_OFFSET, 0);

		Status = XClk_Wiz_SetRateHz(&AudInstancePtr, PL_AUD_CLK_MULT * 48000);
		if (Status != XST_SUCCESS) {
			xil_printf("  ERROR: Failed to set Audio Clock Wizard rate\r\n");
			return XST_FAILURE;
		}

		XClk_Wiz_WriteReg(AudCfgPtr->BaseAddr, XCLK_WIZ_RECONFIG_OFFSET,
							(XCLK_WIZ_RECONFIG_LOAD | XCLK_WIZ_RECONFIG_SADDR));

		Status = XClk_WaitForLock(AudCfgPtr);
		if (Status != XST_SUCCESS) {
			Reg = XClk_Wiz_ReadReg(AudCfgPtr->BaseAddr, XCLK_WIZ_REG4_OFFSET) & CLK_LOCK;
			xil_printf("  ERROR: Audio clock not locked: 0x%x (expected: 0x1)\r\n", Reg);
		}

	}
	return Status;
}

/*****************************************************************************/
/**
 *
 * This function initializes the Display Controller Subsystem
 *
 * @param	RunCfgPtr is a pointer to the application configuration
 *structure
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE
 *
 * @note		None.
 *
 *****************************************************************************/
u32 XDpDc_InitDcSubsystem(RunConfig *RunCfgPtr) {
        XDcSub *DcSubPtr = RunCfgPtr->DcSubPtr;
        XDc *DcPtr = DcSubPtr->DcPtr;
        XDcDma *DmaPtr = DcSubPtr->DmaPtr;

        xil_printf("  Initializing DC Subsystem...\r\n");
        XDcSub_Initialize(DcSubPtr);
        XDcDma_CfgInitialize(DmaPtr, DCDMA_BASEADDR);
        XDcDma_WriteProtDisable(DmaPtr);
        XDc_VidClkSelect(DcPtr);
        XDc_VideoSoftReset(DcPtr);
        XDc_SetVidFrameSwitch(DcPtr);

        if (RunCfgPtr->AudioEnable) {
                xil_printf("  Enabling audio subsystem...\r\n");
                XDc_AudioSoftReset(DcPtr);
                XDc_AudClkSelect(DcPtr);
        }

        if (RunCfgPtr->operatingmode == XDCSUB_OPMODE_FUNCTIONAL &&
            (RunCfgPtr->presentationmode == XDCSUB_PPTMODE_NONLIVE ||
			 RunCfgPtr->presentationmode == XDCSUB_PPTMODE_MIXED))
            XDc_SetVideoTiming(DcPtr);

        XDcSub_ConfigureDcVideo(DcPtr);

        XDpDc_CursorToSdpPending = 0;
        XDpDc_CursorFramesRemaining = 0;

        if (RunCfgPtr->operatingmode == XDCSUB_OPMODE_FUNCTIONAL &&
            RunCfgPtr->presentationmode == XDCSUB_PPTMODE_NONLIVE) {
                /* Video DMA: create descriptors and assign DMA channels per
                 * format mode */
                XDpDc_SetupStream1Descriptors(RunCfgPtr);
                XDpDc_SetupStream2Descriptors(RunCfgPtr);

                DmaPtr->Video.VideoInfo =
                    XDc_GetNonLiveVideoAttribute(RunCfgPtr->Stream1Format);
                DmaPtr->Gfx.VideoInfo =
                    XDc_GetNonLiveVideoAttribute(RunCfgPtr->Stream2Format);

                DmaPtr->Video.Video_TriggerStatus = XDCDMA_TRIGGER_EN;
                DmaPtr->Gfx.Graphics_TriggerStatus = XDCDMA_TRIGGER_EN;

                /* Cursor/SDP DMA on shared channel 7 */
                XDpDc_CursorToSdpPending = 0U;
                XDpDc_CursorFramesRemaining = 0U;
                if ((RunCfgPtr->CursorEnable == CB_ENABLE) &&
                    (RunCfgPtr->SdpEnable != 0U)) {
                        XDpDc_SetupCursorDescriptor(RunCfgPtr);
                        XDpDc_ConfigureCursorDMA(RunCfgPtr);
                        XDpDc_CursorToSdpPending = 1U;
                        XDpDc_CursorFramesRemaining = 1U;
                        xil_printf(
                            "  Cursor+SDP mode: cursor phase started\r\n");
                } else if (RunCfgPtr->CursorEnable == CB_ENABLE) {
                        XDpDc_SetupCursorDescriptor(RunCfgPtr);
                        XDpDc_ConfigureCursorDMA(RunCfgPtr);
                } else if (RunCfgPtr->SdpEnable) {
                        XDpDc_SetupSdpDescriptor(RunCfgPtr);
                        XDpDc_ConfigureSdpDMA(RunCfgPtr);
                }
        } else if (RunCfgPtr->operatingmode == XDCSUB_OPMODE_FUNCTIONAL &&
		   RunCfgPtr->presentationmode == XDCSUB_PPTMODE_MIXED) {
	        /* Mixed mode: stream 1 is live, stream 2 is non live DMA */
                XDpDc_SetupStream2Descriptors(RunCfgPtr);
                DmaPtr->Video.Video_TriggerStatus = 0;
                DmaPtr->Gfx.VideoInfo = XDc_GetNonLiveVideoAttribute(RunCfgPtr->Stream2Format);
                DmaPtr->Gfx.Graphics_TriggerStatus = XDCDMA_TRIGGER_EN;
	} else if (RunCfgPtr->operatingmode == XDCSUB_OPMODE_FUNCTIONAL &&
		   RunCfgPtr->presentationmode == XDCSUB_PPTMODE_LIVE) {
                /* Live mode uses stream interfaces. So no DMA required */
                DmaPtr->Video.Video_TriggerStatus = 0;
                DmaPtr->Gfx.Graphics_TriggerStatus = 0;
        }

        /* Audio DMA: enable only when audio feature is enabled */
        if (RunCfgPtr->AudioEnable) {
                DmaPtr->Audio.Channel.Current = AudDesc0;
                DmaPtr->Audio.Audio_TriggerStatus = XDCDMA_TRIGGER_EN;
        } else {
                DmaPtr->Audio.Audio_TriggerStatus = 0U;
        }

        return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets up interrupt handlers for DMA interrupts
*
* @param	RunCfgPtr is a pointer to the application configuration structure
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XDpDc_SetupInterrupts(RunConfig *RunCfgPtr)
{
	XDcDma *DmaPtr = RunCfgPtr->DcSubPtr->DmaPtr;

	XDpDc_IrqRunCfgPtr = RunCfgPtr;

	XSetupInterruptSystem(DmaPtr, &XDpDc_DmaInterruptHandler, XDCDMA_INTR_ID,
			      XDCDMA_INTR_PARENT, XINTERRUPT_DEFAULT_PRIORITY);
	XDcDma_InterruptEnable(DmaPtr, XDCDMA_IEN_VSYNC_INT_MASK);

	XDpDc_SetupDpInterrupts(RunCfgPtr);
}

/*****************************************************************************/
/**
*
* This function initializes all platform blocks (clocks, DC, DP, interrupts)
*
* @param	RunCfgPtr is a pointer to the application configuration structure
*
* @return	XST_SUCCESS if successful, else XST_FAILURE
*
* @note		None.
*
*****************************************************************************/
u32  XDpDc_InitPlatform(RunConfig *RunCfgPtr)
{
	u32 Status = XST_SUCCESS;

	/* Configure Video/Audio Clock */
	xil_printf("  Configuring clock wizard...\r\n");
	Status = XDpDc_InitClkWiz(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("  ERROR: Failed to initialize Clock Wizard\r\n");
		return Status;
	}

	xil_printf("  Configuring DC subsystem...\r\n");
	/* Initialize DcSubsystem */
	Status = XDpDc_InitDcSubsystem(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("  ERROR: Failed to initialize DC Subsystem\r\n");
		return Status;
	}

	xil_printf("  Enabling DisplayPort output...\r\n");
	/* Initialize DpSubsystem */
	Status = XDpDc_InitDpPsuSubsystem(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("  ERROR: DisplayPort subsystem initialization failed\r\n");
		return Status;
	}

	XDpDc_SetupInterrupts(RunCfgPtr);

	return Status;
}
