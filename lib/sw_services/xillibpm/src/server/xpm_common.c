/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#include "xil_io.h"

#include "xpm_common.h"
#include "xpm_regs.h"

#define MAX_BYTEBUFFER_SIZE	(32 * 1024)
static u8 ByteBuffer[MAX_BYTEBUFFER_SIZE];
static u8 *FreeBytes = ByteBuffer;

void *XPm_AllocBytes(u32 Size)
{
	void *Bytes = NULL;
	u32 BytesLeft = ByteBuffer + MAX_BYTEBUFFER_SIZE - FreeBytes;
	u32 i;
	u32 NumWords;
	u32 *Words;

	/* Round size to the next multiple of 4 */
	Size += 3U;
	Size &= ~0x3U;

	if (Size > BytesLeft) {
		goto done;
	}

	Bytes = FreeBytes;
	FreeBytes += Size;

	/* Zero the bytes */
	NumWords = Size / 4;
	Words = (u32 *)Bytes;
	for (i = 0; i < NumWords; i++) {
		Words[i] = 0U;
	}

done:
	VERIFY(Bytes != NULL);
	return Bytes;
}

void XPm_DumpMemUsage(void)
{
	xil_printf("Total buffer size = %d bytes\n\r", MAX_BYTEBUFFER_SIZE);
	xil_printf("Used = %d bytes\n\r", FreeBytes - ByteBuffer);
	xil_printf("Free = %d bytes\n\r", MAX_BYTEBUFFER_SIZE - (FreeBytes - ByteBuffer));
	xil_printf("\n\r");
}

u32 XPm_In32(u32 RegAddress)
{

	return Xil_In32(RegAddress);
}

void XPm_Out32(u32 RegAddress, u32 l_Val)
{
	if ((RegAddress & 0xFFFF0000) == PMC_IOU_SLCR_BASEADDR) {
		Xil_Out32(PMC_IOU_SLCR_WPROT0, 0x0U);
		Xil_Out32(RegAddress, l_Val);
		Xil_Out32(PMC_IOU_SLCR_WPROT0, 0x1U);
	} else if ((RegAddress & 0xFFFF0000) == LPD_IOU_SLCR_BASEADDR) {
                Xil_Out32(LPD_IOU_SLCR_WPROT0, 0x0U);
                Xil_Out32(RegAddress, l_Val);
                Xil_Out32(LPD_IOU_SLCR_WPROT0, 0x1U);
        }  else if ((RegAddress & 0xFFFF0000) == LPD_SLCR_BASEADDR) {
                Xil_Out32(LPD_SLCR_WPROT0, 0x0U);
                Xil_Out32(RegAddress, l_Val);
                Xil_Out32(LPD_SLCR_WPROT0, 0x1U);
        }  else if ((RegAddress & 0xFFFF0000) == FPD_SLCR_BASEADDR) {
                Xil_Out32(FPD_SLCR_WPROT0, 0x0U);
                Xil_Out32(RegAddress, l_Val);
                Xil_Out32(FPD_SLCR_WPROT0, 0x1U);
        } else {
		Xil_Out32(RegAddress, l_Val);
	}
}

void XPm_RMW32(u32 RegAddress, u32 Mask, u32 Value)
{
	u32 l_Val;

	l_Val = XPm_In32(RegAddress);
	l_Val = (l_Val & (~Mask)) | (Mask & Value);

	XPm_Out32(RegAddress, l_Val);
}

void XPm_Wait(u32 TimeOutCount)
{
	u32 TimeOut = TimeOutCount;
	while (TimeOut > 0U) {
		TimeOut--;
	}
}

XStatus XPm_PollForMask(u32 RegAddress, u32 Mask, u32 TimeOutCount)
{
	u32 l_RegValue;
	u32 TimeOut = TimeOutCount;
	/* Read the Register value	 */
	l_RegValue = XPm_In32(RegAddress);
	/* Loop while the MAsk is not set or we timeout */
	while(((l_RegValue & Mask) != Mask) && (TimeOut > 0U))
	{
		/* Latch up the Register value again */
		l_RegValue = XPm_In32(RegAddress);
		/* Decrement the TimeOut Count */
		TimeOut--;
	}

	return ((TimeOut == 0U) ? XST_FAILURE : XST_SUCCESS);
}
