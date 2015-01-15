/*******************************************************************************
 *
 * Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
/******************************************************************************/
/**
 *
 * @file xdprx.c
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdprx.h"
#include "xstatus.h"

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function retrieves the configuration for this DisplayPort RX instance
 * and fills in the InstancePtr->Config structure.
 *
 * @param	InstancePtr is a pointer to the XDprx instance.
 * @param	ConfigPtr is a pointer to the configuration structure that will
 *		be used to copy the settings from.
 * @param	EffectiveAddr is the device base address in the virtual memory
 *		space. If the address translation is not used, then the physical
 *		address is passed.
 *
 * @return	None.
 *
 * @note	Unexpected errors may occur if the address mapping is changed
 *		after this function is invoked.
 *
*******************************************************************************/
void XDprx_CfgInitialize(XDprx *InstancePtr, XDp_Config *ConfigPtr,
							u32 EffectiveAddr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ConfigPtr != NULL);
	Xil_AssertVoid(EffectiveAddr != 0x0);

	InstancePtr->IsReady = 0;

	InstancePtr->Config.DeviceId = ConfigPtr->DeviceId;
	InstancePtr->Config.BaseAddr = EffectiveAddr;
	InstancePtr->Config.SAxiClkHz = ConfigPtr->SAxiClkHz;

	InstancePtr->Config.MaxLaneCount = ConfigPtr->MaxLaneCount;
	InstancePtr->Config.MaxLinkRate = ConfigPtr->MaxLinkRate;

	InstancePtr->Config.MaxBitsPerColor = ConfigPtr->MaxBitsPerColor;
	InstancePtr->Config.QuadPixelEn = ConfigPtr->QuadPixelEn;
	InstancePtr->Config.DualPixelEn = ConfigPtr->DualPixelEn;
	InstancePtr->Config.YCrCbEn = ConfigPtr->YCrCbEn;
	InstancePtr->Config.YOnlyEn = ConfigPtr->YOnlyEn;
	InstancePtr->Config.PayloadDataWidth = ConfigPtr->PayloadDataWidth;

	InstancePtr->Config.SecondaryChEn = ConfigPtr->SecondaryChEn;
	InstancePtr->Config.NumAudioChs = ConfigPtr->NumAudioChs;

	InstancePtr->Config.MstSupport = ConfigPtr->MstSupport;
	InstancePtr->Config.NumMstStreams = ConfigPtr->NumMstStreams;

	InstancePtr->Config.DpProtocol = ConfigPtr->DpProtocol;

	InstancePtr->Config.IsRx = ConfigPtr->IsRx;

	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
}
