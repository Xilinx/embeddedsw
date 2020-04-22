/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xil_io.h"

#include "xpm_common.h"
#include "xpm_regs.h"
#include "xpm_pmc.h"
#include "xpm_psfpdomain.h"
#include "xpm_pslpdomain.h"

#define MAX_BYTEBUFFER_SIZE	(32U * 1024U)
static u8 ByteBuffer[MAX_BYTEBUFFER_SIZE];
static u8 *FreeBytes = ByteBuffer;

u32 Platform;
u32 PlatformVersion;
u32 SlrType;

void *XPm_AllocBytes(u32 Size)
{
	void *Bytes = NULL;
	u32 BytesLeft = (u32)ByteBuffer + MAX_BYTEBUFFER_SIZE - (u32)FreeBytes;
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
	NumWords = Size / 4U;
	Words = (u32 *)Bytes;
	for (i = 0; i < NumWords; i++) {
		Words[i] = 0U;
	}

done:
	return Bytes;
}

void XPm_DumpMemUsage(void)
{
	xil_printf("Total buffer size = %d bytes\n\r", MAX_BYTEBUFFER_SIZE);
	xil_printf("Used = %d bytes\n\r", FreeBytes - ByteBuffer);
	xil_printf("Free = %d bytes\n\r", MAX_BYTEBUFFER_SIZE - (u32)FreeBytes - (u32)ByteBuffer);
	xil_printf("\n\r");
}

u32 XPm_In32(u32 RegAddress)
{

	return Xil_In32(RegAddress);
}

void XPm_Out32(u32 RegAddress, u32 l_Val)
{
	XPm_Pmc *Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
	XPm_PsLpDomain *PsLpd = (XPm_PsLpDomain *)XPmPower_GetById(PM_POWER_LPD);
	XPm_PsFpDomain *PsFpd = (XPm_PsFpDomain *)XPmPower_GetById(PM_POWER_FPD);

	if ((NULL != Pmc) && ((RegAddress & 0xFFFF0000U) == Pmc->PmcIouSlcrBaseAddr)) {
		Xil_Out32(Pmc->PmcIouSlcrBaseAddr + PMC_IOU_SLCR_WPROT0_OFFSET,
			  0x0U);
		Xil_Out32(RegAddress, l_Val);
		Xil_Out32(Pmc->PmcIouSlcrBaseAddr + PMC_IOU_SLCR_WPROT0_OFFSET,
			  0x1U);
	} else if ((NULL != PsLpd) && ((RegAddress & 0xFFFF0000U) ==
			     PsLpd->LpdIouSlcrBaseAddr)) {
                Xil_Out32(PsLpd->LpdIouSlcrBaseAddr + LPD_IOU_SLCR_WPROT0_OFFSET,
			  0x0U);
                Xil_Out32(RegAddress, l_Val);
                Xil_Out32(PsLpd->LpdIouSlcrBaseAddr + LPD_IOU_SLCR_WPROT0_OFFSET,
			  0x1U);
        }  else if ((NULL != PsLpd) && ((RegAddress & 0xFFFF0000U) ==
			      PsLpd->LpdSlcrBaseAddr)) {
                Xil_Out32(PsLpd->LpdSlcrBaseAddr + LPD_SLCR_WPROT0_OFFSET,
			  0x0U);
                Xil_Out32(RegAddress, l_Val);
                Xil_Out32(PsLpd->LpdSlcrBaseAddr + LPD_SLCR_WPROT0_OFFSET,
			  0x1U);
        }  else if ((NULL != PsFpd) && ((RegAddress & 0xFFFF0000U) ==
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

	return ((TimeOut == 0U) ? XPM_PM_TIMEOUT : XST_SUCCESS);
}


XStatus XPm_PollForZero(u32 RegAddress, u32 Mask, u32 TimeOutCount)
{
        u32 l_RegValue;
        u32 TimeOut = TimeOutCount;
        /* Read the Register value       */
        l_RegValue = XPm_In32(RegAddress);
        /* Loop while the MAsk is not set or we timeout */
        while(((l_RegValue & Mask) != 0U) && (TimeOut > 0U))
        {
                /* Latch up the Register value again */
                l_RegValue = XPm_In32(RegAddress);
                /* Decrement the TimeOut Count */
                TimeOut--;
        }

        return ((TimeOut == 0U) ? XPM_PM_TIMEOUT : XST_SUCCESS);
}

u32 XPm_ComputeParity(u32 Value)
{
	Value ^= (Value >> 16U);
	Value ^= (Value >> 8U);
	Value ^= (Value >> 4U);
	Value ^= (Value >> 2U);
	Value ^= (Value >> 1U);

	return (Value & 1U);
}
