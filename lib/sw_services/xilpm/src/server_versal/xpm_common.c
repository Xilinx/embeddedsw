/******************************************************************************
*
* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

#include "xil_io.h"

#include "xpm_common.h"
#include "xpm_regs.h"
#include "xpm_pmc.h"
#include "xpm_psfpdomain.h"
#include "xpm_pslpdomain.h"

#define MAX_BYTEBUFFER_SIZE	(48 * 1024)
static u8 ByteBuffer[MAX_BYTEBUFFER_SIZE];
static u8 *FreeBytes = ByteBuffer;

u32 Platform;
u32 PlatformVersion;

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
	XPm_Pmc *Pmc = (XPm_Pmc *)XPmDevice_GetById(XPM_DEVID_PMC);
	XPm_PsLpDomain *PsLpd = (XPm_PsLpDomain *)XPmPower_GetById(XPM_POWERID_LPD);
	XPm_PsFpDomain *PsFpd = (XPm_PsFpDomain *)XPmPower_GetById(XPM_POWERID_FPD);

	if (Pmc && ((RegAddress & 0xFFFF0000) == Pmc->PmcIouSlcrBaseAddr)) {
		Xil_Out32(Pmc->PmcIouSlcrBaseAddr + PMC_IOU_SLCR_WPROT0_OFFSET,
			  0x0U);
		Xil_Out32(RegAddress, l_Val);
		Xil_Out32(Pmc->PmcIouSlcrBaseAddr + PMC_IOU_SLCR_WPROT0_OFFSET,
			  0x1U);
	} else if (PsLpd && ((RegAddress & 0xFFFF0000) ==
			     PsLpd->LpdIouSlcrBaseAddr)) {
                Xil_Out32(PsLpd->LpdIouSlcrBaseAddr + LPD_IOU_SLCR_WPROT0_OFFSET,
			  0x0U);
                Xil_Out32(RegAddress, l_Val);
                Xil_Out32(PsLpd->LpdIouSlcrBaseAddr + LPD_IOU_SLCR_WPROT0_OFFSET,
			  0x1U);
        }  else if (PsLpd && ((RegAddress & 0xFFFF0000) ==
			      PsLpd->LpdSlcrBaseAddr)) {
                Xil_Out32(PsLpd->LpdSlcrBaseAddr + LPD_SLCR_WPROT0_OFFSET,
			  0x0U);
                Xil_Out32(RegAddress, l_Val);
                Xil_Out32(PsLpd->LpdSlcrBaseAddr + LPD_SLCR_WPROT0_OFFSET,
			  0x1U);
        }  else if (PsFpd && ((RegAddress & 0xFFFF0000) ==
			      PsFpd->FpdSlcrBaseAddr)) {
                Xil_Out32(PsFpd->FpdSlcrBaseAddr + FPD_SLCR_WPROT0_OFFSET,
			  0x0U);
                Xil_Out32(RegAddress, l_Val);
                Xil_Out32(PsFpd->FpdSlcrBaseAddr + FPD_SLCR_WPROT0_OFFSET,
			  0x1U);
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
