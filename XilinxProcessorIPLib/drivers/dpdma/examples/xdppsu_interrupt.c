/*******************************************************************************
 *
 * Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
*******************************************************************************/
/*****************************************************************************/
/**
*
* @file xdppsu_interrupt.c
*
* This file XDpPsu interrupt handlers. Upon Hot-
* Plug-Detect (HPD - DisplayPort cable is plugged/unplugged or the monitor is
* turned on/off), the main link will be trained.
*
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.0	aad 10/19/17	Initial Release
*</pre>
*
******************************************************************************/
#include "dpdma_video_example.h"

/******************************************************************************/
/**
 * This function is called to wake-up the monitor from sleep.
 *
 * @param	RunCfgPtr is a pointer to the application configuration structure.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 DpPsu_Wakeup(Run_Config *RunCfgPtr)
{
	u32 Status;
	u8 AuxData;

	AuxData = 0x1;
	Status = XDpPsu_AuxWrite(RunCfgPtr->DpPsuPtr,
			XDPPSU_DPCD_SET_POWER_DP_PWR_VOLTAGE, 1, &AuxData);
	if (Status != XST_SUCCESS)
		xil_printf("\t! 1st power wake-up - AUX write failed.\n\r");
	Status = XDpPsu_AuxWrite(RunCfgPtr->DpPsuPtr,
			XDPPSU_DPCD_SET_POWER_DP_PWR_VOLTAGE, 1, &AuxData);
	if (Status != XST_SUCCESS)
		xil_printf("\t! 2nd power wake-up - AUX write failed.\n\r");

	return Status;
}

/******************************************************************************/
/**
 * This function is called to initiate training with the source device, using
 * the predefined source configuration as well as the sink capabilities.
 *
 * @param	RunCfgPtr is a pointer to the application configuration structure.
 *
 * @return	XST_SUCCESS if the function executes correctly.
 * 			XST_FAILURE if the function fails to execute correctly
 *
 * @note	None.
*******************************************************************************/
static u32 DpPsu_Hpd_Train(Run_Config *RunCfgPtr)
{
	XDpPsu		 *DpPsuPtr    = RunCfgPtr->DpPsuPtr;
	XDpPsu_LinkConfig *LinkCfgPtr = &DpPsuPtr->LinkConfig;
	u32 Status;

	Status = XDpPsu_GetRxCapabilities(DpPsuPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("\t! Error getting RX caps.\n\r");
		return XST_FAILURE;
	}

	Status = XDpPsu_SetEnhancedFrameMode(DpPsuPtr,
			LinkCfgPtr->SupportEnhancedFramingMode ? 1 : 0);
	if (Status != XST_SUCCESS) {
		xil_printf("\t! EFM set failed.\n\r");
		return XST_FAILURE;
	}

	Status = XDpPsu_SetLaneCount(DpPsuPtr,
			(RunCfgPtr->UseMaxLaneCount) ?
				LinkCfgPtr->MaxLaneCount :
				RunCfgPtr->LaneCount);
	if (Status != XST_SUCCESS) {
		xil_printf("\t! Lane count set failed.\n\r");
		return XST_FAILURE;
	}

	Status = XDpPsu_SetLinkRate(DpPsuPtr,
			(RunCfgPtr->UseMaxLinkRate) ?
				LinkCfgPtr->MaxLinkRate :
				RunCfgPtr->LinkRate);
	if (Status != XST_SUCCESS) {
		xil_printf("\t! Link rate set failed.\n\r");
		return XST_FAILURE;
	}

	Status = XDpPsu_SetDownspread(DpPsuPtr,
			LinkCfgPtr->SupportDownspreadControl);
	if (Status != XST_SUCCESS) {
		xil_printf("\t! Setting downspread failed.\n\r");
		return XST_FAILURE;
	}

	xil_printf("Lane count =\t%d\n\r", DpPsuPtr->LinkConfig.LaneCount);
	xil_printf("Link rate =\t%d\n\r",  DpPsuPtr->LinkConfig.LinkRate);

	// Link training sequence is done
        xil_printf("\n\r\rStarting Training...\n\r");
	Status = XDpPsu_EstablishLink(DpPsuPtr);
	if (Status == XST_SUCCESS)
		xil_printf("\t! Training succeeded.\n\r");
	else
		xil_printf("\t! Training failed.\n\r");

	return Status;
}

/******************************************************************************/
/**
 * This function will configure and establish a link with the receiver device,
 * afterwards, a video stream will start to be sent over the main link.
 *
 * @param	RunCfgPtr is a pointer to the application configuration structure.
 *
 * @return
 *		- XST_SUCCESS if main link was successfully established.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
void DpPsu_Run(Run_Config *RunCfgPtr)
{
	u32 Status;
	XDpPsu  *DpPsuPtr = RunCfgPtr->DpPsuPtr;

	XDpPsu_EnableMainLink(DpPsuPtr, 0);

	if (!XDpPsu_IsConnected(DpPsuPtr)) {
		xil_printf("! Disconnected.\n\r");
		return;
	}
	else {
		xil_printf("! Connected.\n\r");
	}

	Status = DpPsu_Wakeup(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("! Wakeup failed.\n\r");
		return;
	}


	u8 Count = 0;
	do {
		usleep(100000);
		Count++;

		Status = DpPsu_Hpd_Train(RunCfgPtr);
		if (Status == XST_DEVICE_NOT_FOUND) {
			xil_printf("Lost connection\n\r");
			return;
		}
		else if (Status != XST_SUCCESS)
			continue;

		DpPsu_SetupVideoStream(RunCfgPtr);
		XDpPsu_EnableMainLink(DpPsuPtr, 1);

		Status = XDpPsu_CheckLinkStatus(DpPsuPtr, DpPsuPtr->LinkConfig.LaneCount);
		if (Status == XST_DEVICE_NOT_FOUND)
			return;
	} while ((Status != XST_SUCCESS) && (Count < 2));
}

/******************************************************************************/
/**
 * This function is called when a Hot-Plug-Detect (HPD) event is received by the
 * DisplayPort TX core. The XDPPSU_INTERRUPT_STATUS_HPD_EVENT_MASK bit of the
 * core's XDPPSU_INTERRUPT_STATUS register indicates that an HPD event has
 * occurred.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return	None.
 *
 * @note	Use the XDpPsu_SetHpdEventHandler driver function to set this
 *		function as the handler for HPD pulses.
 *
*******************************************************************************/
void DpPsu_IsrHpdEvent(void *ref)
{
	xil_printf("HPD event .......... ");
	DpPsu_Run(ref);
	xil_printf(".......... HPD event\n\r");
}
/******************************************************************************/
/**
 * This function is called when a Hot-Plug-Detect (HPD) pulse is received by the
 * DisplayPort TX core. The XDPPSU_INTERRUPT_STATUS_HPD_PULSE_DETECTED_MASK bit
 * of the core's XDPPSU_INTERRUPT_STATUS register indicates that an HPD event has
 * occurred.
 *
 * @param	InstancePtr is a pointer to the XDpPsu instance.
 *
 * @return	None.
 *
 * @note	Use the XDpPsu_SetHpdPulseHandler driver function to set this
 *		function as the handler for HPD pulses.
 *
*******************************************************************************/
void DpPsu_IsrHpdPulse(void *ref)
{
	u32 Status;
	XDpPsu *DpPsuPtr = ((Run_Config *)ref)->DpPsuPtr;
	xil_printf("HPD pulse ..........\n\r");

	Status = XDpPsu_CheckLinkStatus(DpPsuPtr, DpPsuPtr->LinkConfig.LaneCount);
	if (Status == XST_DEVICE_NOT_FOUND) {
		xil_printf("Lost connection .......... HPD pulse\n\r");
		return;
	}

	xil_printf("\t! Re-training required.\n\r");

	XDpPsu_EnableMainLink(DpPsuPtr, 0);

	u8 Count = 0;
	do {
		Count++;

		Status = DpPsu_Hpd_Train(ref);
		if (Status == XST_DEVICE_NOT_FOUND) {
			xil_printf("Lost connection .......... HPD pulse\n\r");
			return;
		}
		else if (Status != XST_SUCCESS) {
			continue;
		}

		DpPsu_SetupVideoStream(ref);
		XDpPsu_EnableMainLink(DpPsuPtr, 1);

		Status = XDpPsu_CheckLinkStatus(DpPsuPtr, DpPsuPtr->LinkConfig.LaneCount);
		if (Status == XST_DEVICE_NOT_FOUND) {
			xil_printf("Lost connection .......... HPD pulse\n\r");
			return;
		}
	} while ((Status != XST_SUCCESS) && (Count < 2));

	xil_printf(".......... HPD pulse\n\r");
}
/******************************************************************************/
/**
 * This function is called to setup the VideoStream with its video and MSA
 * attributes
 *
 * @param	RunCfgPtr is a pointer to the application configuration structure.
 *
 * @return	None.
 *
 * @note	Use the XDpPsu_SetHpdEventHandler driver function to set this
 *		function as the handler for HPD pulses.
 *
*******************************************************************************/
void DpPsu_SetupVideoStream(Run_Config *RunCfgPtr)
{
	XDpPsu		 *DpPsuPtr    = RunCfgPtr->DpPsuPtr;
	XDpPsu_MainStreamAttributes *MsaConfig = &DpPsuPtr->MsaConfig;

	XDpPsu_SetColorEncode(DpPsuPtr, RunCfgPtr->ColorEncode);
	XDpPsu_CfgMsaSetBpc(DpPsuPtr, RunCfgPtr->Bpc);
	XDpPsu_CfgMsaUseStandardVideoMode(DpPsuPtr, RunCfgPtr->VideoMode);

	/* Set pixel clock. */
	RunCfgPtr->PixClkHz = MsaConfig->PixelClockHz;
	XAVBuf_SetPixelClock(RunCfgPtr->PixClkHz);

	/* Reset the transmitter. */
	XDpPsu_WriteReg(DpPsuPtr->Config.BaseAddr, XDPPSU_SOFT_RESET, 0x1);
	usleep(10);
	XDpPsu_WriteReg(DpPsuPtr->Config.BaseAddr, XDPPSU_SOFT_RESET, 0x0);

	XDpPsu_SetMsaValues(DpPsuPtr);
	/* Issuing a soft-reset (AV_BUF_SRST_REG). */
	XDpPsu_WriteReg(DpPsuPtr->Config.BaseAddr, 0xB124, 0x3); // Assert reset.
	usleep(10);
	XDpPsu_WriteReg(DpPsuPtr->Config.BaseAddr, 0xB124, 0x0); // De-ssert reset.

	XDpPsu_EnableMainLink(DpPsuPtr, 1);

	xil_printf("DONE!\n\r");
}
