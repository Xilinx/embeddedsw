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
 * @file xvid.c
 *
 * Contains common utility functions that are typically used by video-related
 * drivers and applications.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  01/10/15 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xvid.h"

u32 XVid_GetPixelClockHz(XVid_VideoMode VmId)
{
	u32 ClkHz;
	XVid_VideoTimingMode *VmPtr;

	VmPtr = &XVid_VideoTimingModes[VmId];

	/* For pixel clock calculation, use frame with the larger vertical
	 * total. This is useful for interlaced modes with frames that don't
	 * have exactly the same vertical total. For progressive modes,
	 * F0PVTotal will be used since F1PVTotal will be equal to 0. */
	if (VmPtr->Timing.F0PVTotal >= VmPtr->Timing.F1PVTotal) {
		ClkHz = VmPtr->Timing.F0PVTotal;
	}
	else {
		ClkHz = VmPtr->Timing.F1PVTotal;
	}

	/* Multiply the vertical total by the horizontal total for number of
	 * pixels. */
	ClkHz *= VmPtr->Timing.HTotal;

	/* Multiply the number of pixels by the frame rate. */
	ClkHz *= VmPtr->FrameRate;

	return ClkHz;
}
