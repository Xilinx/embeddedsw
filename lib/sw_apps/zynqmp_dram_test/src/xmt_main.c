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

/*****************************************************************************/
/**
 *
 * @file xmt_main.c
 *
 * This is the main file for ZynqMP DRAM test. This includes various DRAM tests
 * like Memory Tests of different sizes, Read Eye test and Write Eye test.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   mn   08/17/18 Initial release
 *       mn   09/21/18 Modify code manually enter the DDR memory test size
 *       mn   09/27/18 Modify code to add 2D Read/Write Eye Tests support
 *       mn   04/09/19 Add check for Carriage return when entering the test size
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xmt_common.h"

/************************** Constant Definitions *****************************/

#define XMT_DEFAULT_TEST_LEN		1024U
#define XMT_DEFAULT_TEST_PATTERN	0U
#define XMT_MAX_MODE_NUM		15U

#ifdef XPAR_PSU_DDR_0_S_AXI_BASEADDR
#ifdef XPAR_PSU_DDR_1_S_AXI_BASEADDR
/*
 * If the Upper DDR is enabled calculate the DDR total size by adding both
 * DDR (Lower and Upper) regions sizes
 */
#define XMT_DDR_MAX_SIZE		((XPAR_PSU_DDR_1_S_AXI_HIGHADDR -\
				XPAR_PSU_DDR_1_S_AXI_BASEADDR) +\
				(XPAR_PSU_DDR_0_S_AXI_HIGHADDR -\
				XPAR_PSU_DDR_0_S_AXI_BASEADDR) + 2U)
#define XMT_DDR_1_BASEADDR		XPAR_PSU_DDR_1_S_AXI_BASEADDR

#else
/* Calculate the DDR size for Lower DDR */
#define XMT_DDR_MAX_SIZE		(XPAR_PSU_DDR_0_S_AXI_HIGHADDR -\
				XPAR_PSU_DDR_0_S_AXI_BASEADDR + 1U)
#define XMT_DDR_1_BASEADDR		XPAR_PSU_DDR_0_S_AXI_BASEADDR
#endif
#else
#define XMT_DDR_MAX_SIZE		0U
#define XMT_DDR_1_BASEADDR		0U
#endif


/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
u8 Verbose;

/* Pattern for a 64Bit Memory */
static u64 Pattern64Bit[16] = {
	0x0000000000000000, 0x0000000000000000,
	0xFFFFFFFFFFFFFFFF, 0x0000000000000000,
	0x0000000000000000, 0xFFFFFFFFFFFFFFFF,
	0x0000000000000000, 0xFFFFFFFFFFFFFFFF,
	0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF,
	0x0000000000000000, 0xFFFFFFFFFFFFFFFF,
	0xFFFFFFFFFFFFFFFF, 0x00000000FFFFFFFF,
	0xFFFFFFFFFFFFFFFF, 0x00000000FFFFFFFF
};

/* Pattern for a 32Bit Memory */
static u32 Pattern32Bit[16] = {
	0x00000000, 0x00000000,
	0xFFFFFFFF, 0x00000000,
	0xFFFFFFFF, 0x00000000,
	0x00000000, 0xFFFFFFFF,
	0x00000000, 0xFFFFFFFF,
	0xFFFFFFFF, 0x00000000,
	0xFFFFFFFF, 0x00000000,
	0x00000000, 0xFFFFFFFF
};

/* Invert Mask for inverting the Pattern for 64Bit Memory */
static u64 InvertMask64Bit[8] = {
	0x0101010101010101, 0x0202020202020202,
	0x0404040404040404, 0x0808080808080808,
	0x1010101010101010, 0x2020202020202020,
	0x4040404040404040, 0x8080808080808080
};

/* Invert Mask for inverting the Pattern for 64Bit Memory */
static u32 InvertMask32Bit[8] = {
	0x01010101, 0x02020202,
	0x04040404, 0x08080808,
	0x10101010, 0x20202020,
	0x40404040, 0x80808080
};

/* Aggressor Pattern for 64Bit Eye Test */
static u64 AggressorPattern64Bit[] = {
	0x0101010101010101, 0x0101010101010101,
	0xFEFEFEFEFEFEFEFE, 0x0101010101010101,
	0x0101010101010101, 0xFEFEFEFEFEFEFEFE,
	0x0101010101010101, 0xFEFEFEFEFEFEFEFE,
	0xFEFEFEFEFEFEFEFE, 0xFEFEFEFEFEFEFEFE,
	0x0101010101010101, 0xFEFEFEFEFEFEFEFE,
	0xFEFEFEFEFEFEFEFE, 0x0101010101010101,
	0xFEFEFEFEFEFEFEFE, 0x0101010101010101,
	0x0202020202020202, 0x0202020202020202,
	0xFDFDFDFDFDFDFDFD, 0x0202020202020202,
	0x0202020202020202, 0xFDFDFDFDFDFDFDFD,
	0x0202020202020202, 0xFDFDFDFDFDFDFDFD,
	0xFDFDFDFDFDFDFDFD, 0xFDFDFDFDFDFDFDFD,
	0x0202020202020202, 0xFDFDFDFDFDFDFDFD,
	0xFDFDFDFDFDFDFDFD, 0x0202020202020202,
	0xFDFDFDFDFDFDFDFD, 0x0202020202020202,
	0x0404040404040404, 0x0404040404040404,
	0xFBFBFBFBFBFBFBFB, 0x0404040404040404,
	0x0404040404040404, 0xFBFBFBFBFBFBFBFB,
	0x0404040404040404, 0xFBFBFBFBFBFBFBFB,
	0xFBFBFBFBFBFBFBFB, 0xFBFBFBFBFBFBFBFB,
	0x0404040404040404, 0xFBFBFBFBFBFBFBFB,
	0xFBFBFBFBFBFBFBFB, 0x0404040404040404,
	0xFBFBFBFBFBFBFBFB, 0x0404040404040404,
	0x0808080808080808, 0x0808080808080808,
	0xF7F7F7F7F7F7F7F7, 0x0808080808080808,
	0x0808080808080808, 0xF7F7F7F7F7F7F7F7,
	0x0808080808080808, 0xF7F7F7F7F7F7F7F7,
	0xF7F7F7F7F7F7F7F7, 0xF7F7F7F7F7F7F7F7,
	0x0808080808080808, 0xF7F7F7F7F7F7F7F7,
	0xF7F7F7F7F7F7F7F7, 0x0808080808080808,
	0xF7F7F7F7F7F7F7F7, 0x0808080808080808,
	0x1010101010101010, 0x1010101010101010,
	0xEFEFEFEFEFEFEFEF, 0x1010101010101010,
	0x1010101010101010, 0xEFEFEFEFEFEFEFEF,
	0x1010101010101010, 0xEFEFEFEFEFEFEFEF,
	0xEFEFEFEFEFEFEFEF, 0xEFEFEFEFEFEFEFEF,
	0x1010101010101010, 0xEFEFEFEFEFEFEFEF,
	0xEFEFEFEFEFEFEFEF, 0x1010101010101010,
	0xEFEFEFEFEFEFEFEF, 0x1010101010101010,
	0x2020202020202020, 0x2020202020202020,
	0xDFDFDFDFDFDFDFDF, 0x2020202020202020,
	0x2020202020202020, 0xDFDFDFDFDFDFDFDF,
	0x2020202020202020, 0xDFDFDFDFDFDFDFDF,
	0xDFDFDFDFDFDFDFDF, 0xDFDFDFDFDFDFDFDF,
	0x2020202020202020, 0xDFDFDFDFDFDFDFDF,
	0xDFDFDFDFDFDFDFDF, 0x2020202020202020,
	0xDFDFDFDFDFDFDFDF, 0x2020202020202020,
	0x4040404040404040, 0x4040404040404040,
	0xBFBFBFBFBFBFBFBF, 0x4040404040404040,
	0x4040404040404040, 0xBFBFBFBFBFBFBFBF,
	0x4040404040404040, 0xBFBFBFBFBFBFBFBF,
	0xBFBFBFBFBFBFBFBF, 0xBFBFBFBFBFBFBFBF,
	0x4040404040404040, 0xBFBFBFBFBFBFBFBF,
	0xBFBFBFBFBFBFBFBF, 0x4040404040404040,
	0xBFBFBFBFBFBFBFBF, 0x4040404040404040,
	0x8080808080808080, 0x8080808080808080,
	0x7F7F7F7F7F7F7F7F, 0x8080808080808080,
	0x8080808080808080, 0x7F7F7F7F7F7F7F7F,
	0x8080808080808080, 0x7F7F7F7F7F7F7F7F,
	0x7F7F7F7F7F7F7F7F, 0x7F7F7F7F7F7F7F7F,
	0x8080808080808080, 0x7F7F7F7F7F7F7F7F,
	0x7F7F7F7F7F7F7F7F, 0x8080808080808080,
	0x7F7F7F7F7F7F7F7F, 0x8080808080808080
};

/* Aggressor Pattern for 32Bit Eye Test */
static u32 AggressorPattern32Bit[] = {
	0x01010101, 0x01010101, 0x01010101, 0x01010101,
	0xFEFEFEFE, 0xFEFEFEFE, 0x01010101, 0x01010101,
	0x01010101, 0x01010101, 0xFEFEFEFE, 0xFEFEFEFE,
	0x01010101, 0x01010101, 0xFEFEFEFE, 0xFEFEFEFE,
	0xFEFEFEFE, 0xFEFEFEFE, 0xFEFEFEFE, 0xFEFEFEFE,
	0x01010101, 0x01010101, 0xFEFEFEFE, 0xFEFEFEFE,
	0xFEFEFEFE, 0xFEFEFEFE, 0x01010101, 0x01010101,
	0xFEFEFEFE, 0xFEFEFEFE, 0x01010101, 0x01010101,
	0x02020202, 0x02020202, 0x02020202, 0x02020202,
	0xFDFDFDFD, 0xFDFDFDFD, 0x02020202, 0x02020202,
	0x02020202, 0x02020202, 0xFDFDFDFD, 0xFDFDFDFD,
	0x02020202, 0x02020202, 0xFDFDFDFD, 0xFDFDFDFD,
	0xFDFDFDFD, 0xFDFDFDFD, 0xFDFDFDFD, 0xFDFDFDFD,
	0x02020202, 0x02020202, 0xFDFDFDFD, 0xFDFDFDFD,
	0xFDFDFDFD, 0xFDFDFDFD, 0x02020202, 0x02020202,
	0xFDFDFDFD, 0xFDFDFDFD, 0x02020202, 0x02020202,
	0x04040404, 0x04040404, 0x04040404, 0x04040404,
	0xFBFBFBFB, 0xFBFBFBFB, 0x04040404, 0x04040404,
	0x04040404, 0x04040404, 0xFBFBFBFB, 0xFBFBFBFB,
	0x04040404, 0x04040404, 0xFBFBFBFB, 0xFBFBFBFB,
	0xFBFBFBFB, 0xFBFBFBFB, 0xFBFBFBFB, 0xFBFBFBFB,
	0x04040404, 0x04040404, 0xFBFBFBFB, 0xFBFBFBFB,
	0xFBFBFBFB, 0xFBFBFBFB, 0x04040404, 0x04040404,
	0xFBFBFBFB, 0xFBFBFBFB, 0x04040404, 0x04040404,
	0x08080808, 0x08080808, 0x08080808, 0x08080808,
	0xF7F7F7F7, 0xF7F7F7F7, 0x08080808, 0x08080808,
	0x08080808, 0x08080808, 0xF7F7F7F7, 0xF7F7F7F7,
	0x08080808, 0x08080808, 0xF7F7F7F7, 0xF7F7F7F7,
	0xF7F7F7F7, 0xF7F7F7F7, 0xF7F7F7F7, 0xF7F7F7F7,
	0x08080808, 0x08080808, 0xF7F7F7F7, 0xF7F7F7F7,
	0xF7F7F7F7, 0xF7F7F7F7, 0x08080808, 0x08080808,
	0xF7F7F7F7, 0xF7F7F7F7, 0x08080808, 0x08080808,
	0x10101010, 0x10101010, 0x10101010, 0x10101010,
	0xEFEFEFEF, 0xEFEFEFEF, 0x10101010, 0x10101010,
	0x10101010, 0x10101010, 0xEFEFEFEF, 0xEFEFEFEF,
	0x10101010, 0x10101010, 0xEFEFEFEF, 0xEFEFEFEF,
	0xEFEFEFEF, 0xEFEFEFEF, 0xEFEFEFEF, 0xEFEFEFEF,
	0x10101010, 0x10101010, 0xEFEFEFEF, 0xEFEFEFEF,
	0xEFEFEFEF, 0xEFEFEFEF, 0x10101010, 0x10101010,
	0xEFEFEFEF, 0xEFEFEFEF, 0x10101010, 0x10101010,
	0x20202020, 0x20202020, 0x20202020, 0x20202020,
	0xDFDFDFDF, 0xDFDFDFDF, 0x20202020, 0x20202020,
	0x20202020, 0x20202020, 0xDFDFDFDF, 0xDFDFDFDF,
	0x20202020, 0x20202020, 0xDFDFDFDF, 0xDFDFDFDF,
	0xDFDFDFDF, 0xDFDFDFDF, 0xDFDFDFDF, 0xDFDFDFDF,
	0x20202020, 0x20202020, 0xDFDFDFDF, 0xDFDFDFDF,
	0xDFDFDFDF, 0xDFDFDFDF, 0x20202020, 0x20202020,
	0xDFDFDFDF, 0xDFDFDFDF, 0x20202020, 0x20202020,
	0x40404040, 0x40404040, 0x40404040, 0x40404040,
	0xBFBFBFBF, 0xBFBFBFBF, 0x40404040, 0x40404040,
	0x40404040, 0x40404040, 0xBFBFBFBF, 0xBFBFBFBF,
	0x40404040, 0x40404040, 0xBFBFBFBF, 0xBFBFBFBF,
	0xBFBFBFBF, 0xBFBFBFBF, 0xBFBFBFBF, 0xBFBFBFBF,
	0x40404040, 0x40404040, 0xBFBFBFBF, 0xBFBFBFBF,
	0xBFBFBFBF, 0xBFBFBFBF, 0x40404040, 0x40404040,
	0xBFBFBFBF, 0xBFBFBFBF, 0x40404040, 0x40404040,
	0x80808080, 0x80808080, 0x80808080, 0x80808080,
	0x7F7F7F7F, 0x7F7F7F7F, 0x80808080, 0x80808080,
	0x80808080, 0x80808080, 0x7F7F7F7F, 0x7F7F7F7F,
	0x80808080, 0x80808080, 0x7F7F7F7F, 0x7F7F7F7F,
	0x7F7F7F7F, 0x7F7F7F7F, 0x7F7F7F7F, 0x7F7F7F7F,
	0x80808080, 0x80808080, 0x7F7F7F7F, 0x7F7F7F7F,
	0x7F7F7F7F, 0x7F7F7F7F, 0x80808080, 0x80808080,
	0x7F7F7F7F, 0x7F7F7F7F, 0x80808080, 0x80808080
};

/* Test Pattern for Simple Memory tests */
u64 TestPattern[12][4] = {
	{0xFFFF0000FFFF0000, 0xFFFF0000FFFF0000,
	0xFFFF0000FFFF0000, 0xFFFF0000FFFF0000},
	{0x0000FFFF0000FFFF, 0x0000FFFF0000FFFF,
	0x0000FFFF0000FFFF, 0x0000FFFF0000FFFF},
	{0xAAAA5555AAAA5555, 0xAAAA5555AAAA5555,
	0xAAAA5555AAAA5555, 0xAAAA5555AAAA5555},
	{0x5555AAAA5555AAAA, 0x5555AAAA5555AAAA,
	0x5555AAAA5555AAAA, 0x5555AAAA5555AAAA},
	{0x0000000000000000, 0x0000000000000000,
	0x0000000000000000, 0x0000000000000000},
	{0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF,
	0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF},
	{0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA,
	0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA},
	{0x5555555555555555, 0x5555555555555555,
	0x5555555555555555, 0x5555555555555555},
	{0x0000000000000000, 0xFFFFFFFFFFFFFFFF,
	0x0000000000000000, 0xFFFFFFFFFFFFFFFF},
	{0xFFFFFFFFFFFFFFFF, 0x0000000000000000,
	0xFFFFFFFFFFFFFFFF, 0x0000000000000000},
	{0x5555555555555555, 0xAAAAAAAAAAAAAAAA,
	0x5555555555555555, 0xAAAAAAAAAAAAAAAA},
	{0xAAAAAAAAAAAAAAAA, 0x5555555555555555,
	0xAAAAAAAAAAAAAAAA, 0x5555555555555555}
};

/*****************************************************************************/
/**
 * This function measures the total time taken between two points for Memory
 * Test timing calculations
 *
 * @param Current/Start time
 *
 * @return Total Time
 *
 * @note none
 *****************************************************************************/
static double XMt_CalcTime(XTime tCur)
{
	XTime tEnd;
	XTime tDiff;
	double tPerfS;

	XTime_GetTime(&tEnd);
	tDiff = tEnd - tCur;

	/* Convert tPerf into seconds */
	tPerfS = ((double)tDiff / (double)COUNTS_PER_SECOND);

	return tPerfS;
}

/*****************************************************************************/
/**
 * This function creates a reference value which can be used to write to the
 * address or can be used to compare the data read from the address.
 *
 * @param Addr is the Address for which the RefVal is to be created
 * @param Index is the number indicating the byte number
 * @param ModeVal is the Mode number for the test
 * @param Pattern is the Test Pattern to be written
 *
 * @return Reference Value
 *
 * @note none
 *****************************************************************************/
static u64 XMt_GetRefVal(u64 Addr, u64 Index, s32 ModeVal, u64 *Pattern)
{
	u64 RefVal;
	u64 Mod128;
	s64 RandVal;

	/* Create a Random Value */
	RandVal = XMT_RANDOM_VALUE(ModeVal);

	if (ModeVal == 0U) {
		RefVal = ((((Addr + 4) << 32) | Addr) & U64_MASK);
	} else if (ModeVal <= 8U) {
		RefVal = (u64)Pattern[(Index % 32) / 8];
	} else if (ModeVal <= 10U) {
		Mod128 = (Index >> 2) & 0x07f;
		RefVal = (u64)Pattern[Mod128] & U64_MASK;
	} else {
		RandVal = XMT_YLFSR(RandVal);
		RefVal = RandVal & U64_MASK;
	}

	return RefVal;
}

/*****************************************************************************/
/**
 * This function does memory Read/Write test
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 * @param StartVal is the starting Address
 * @param SizeVal is the Size (Bytes) of the memory to be Tested
 * @param ModeVal is the Mode number for the test
 * @param Pattern is the Test Pattern to be written
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
static void XMt_Memtest(XMt_CfgData *XMtPtr, u32 StartVal, u32 SizeVal,
			s32 ModeVal, u64 *Pattern)
{
	u64 Index;
	u64 Start;
	u64 Size;
	u64 Addr;
	u64 RefVal;
	u64 Data;
	u64 UpperDdrOffset;
	s32 MemErr;
	s32 LocalErrCnt[8];
	u8 Cnt;
	XTime tCur1;
	float TestTime;

	MemErr = 0U;

	/* Set the Start Address in Bytes */
	Start = ((u64) StartVal) * XMT_MB2BYTE;
	Size = ((u64) SizeVal) * XMT_MB2BYTE;

	memset(LocalErrCnt, 0U, 8*(sizeof(s32)));

	/* Get the Starting Time value */
	XTime_GetTime(&tCur1);

	UpperDdrOffset = 0U;
	Addr = Start;
	for (Index = 0U; Index < Size; Index += 8U) {
		if (Addr < (XPAR_PSU_DDR_0_S_AXI_HIGHADDR + 1U) - 8U) {
			Addr = Start + Index;
		} else {
			Addr = XMT_DDR_1_BASEADDR + UpperDdrOffset;
			UpperDdrOffset = UpperDdrOffset + 8;
		}

		/* Create a value to be written to the memory */
		RefVal = XMt_GetRefVal(Addr, Index, ModeVal, Pattern);
		Xil_Out64(Addr, RefVal);
	}

	if (XMtPtr->DCacheEnable != 0U) {
		if ((Start + Size) < (XPAR_PSU_DDR_0_S_AXI_HIGHADDR + 1U)) {
			Xil_DCacheInvalidateRange(Start, Size);
		} else {
			Xil_DCacheInvalidateRange(Start, XPAR_PSU_DDR_0_S_AXI_HIGHADDR + 1U - Start);
			Xil_DCacheInvalidateRange(XMT_DDR_1_BASEADDR, Start + Size - XMT_DDR_1_BASEADDR);
		}
	}

	UpperDdrOffset = 0U;
	Addr = Start;
	for (Index = 0U; Index < Size; Index += 8U) {
		if (Addr < (2048*(u64)XMT_MB2BYTE) - 8U) {
			Addr = Start + Index;
		} else {
			Addr = XMT_DDR_1_BASEADDR + UpperDdrOffset;
			UpperDdrOffset = UpperDdrOffset + 8;
		}

		/* Create a value to be compared with read value from memory */
		RefVal = XMt_GetRefVal(Addr, Index, ModeVal, Pattern);
		Data = Xil_In64(Addr);

		/* Compare the data got from the memory */
		if (Data != RefVal) {
			/* Increment the Error Address count */
			MemErr++;
			for (Cnt = 0U; Cnt < XMtPtr->DdrConfigLanes; Cnt++) {
				if ((((Data ^ RefVal) >>
						(Cnt * 8)) & 0xff) != 0U) {
					/* Update per Lane Error count */
					LocalErrCnt[Cnt]++;
				}
			}
			/* Print the Verbose Information */
			if ((Verbose == 1U) && (MemErr <= 10)) {
				xil_printf("Memtest_0 ERROR: "
				"Addr=0x%X rd/RefVal/xor ="
				"0x%016llx 0x%016llx 0x%016llx \r\n",
				Addr, Data, RefVal, Data ^ RefVal);
			}
		}
	}

	/* Get the Ending Time value and calculate the total time*/
	TestTime = XMt_CalcTime(tCur1);

	if (XMtPtr->DdrConfigLanes == XMT_DDR_CONFIG_4_LANE) {
		/* Print the Memory Test Report */
		xil_printf("\rMT0(%2d)  | %6d | %4d, %4d, %4d, %4d  | %d.%06d\r\n",
		       ModeVal, MemErr, LocalErrCnt[0], LocalErrCnt[1], LocalErrCnt[2],
		       LocalErrCnt[3], (int)TestTime,
		       (int)((TestTime - (int)TestTime)*1000000.0f));
	} else if (XMtPtr->DdrConfigLanes == XMT_DDR_CONFIG_8_LANE) {
		/* Print the Memory Test Report */
		xil_printf("\rMT0(%2d)  | %6d | %4d, %4d, %4d, %4d, %4d, %4d, %4d, %4d | %d.%06d\r\n",
		       ModeVal, MemErr, LocalErrCnt[0], LocalErrCnt[1], LocalErrCnt[2],
		       LocalErrCnt[3], LocalErrCnt[4], LocalErrCnt[5], LocalErrCnt[6],
		       LocalErrCnt[7], (int)TestTime,
		       (int)((TestTime - (int)TestTime)*1000000.0f));
	}

	XMt_PrintLine(XMtPtr, 4);

}

/*****************************************************************************/
/**
 * This function does all the memory Read/Write tests
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 * @param TestStart is the starting Address
 * @param TestSize is the Size of the memory to be Tested
 * @param BusWidth is the Width of the DRAM Bus
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
static void XMt_MemtestAll(XMt_CfgData *XMtPtr, u64 TestStart, u64 TestSize,
			   u8 BusWidth)
{
	u8 Mode;
	u32 Index;
	u32 InvMaskInd;
	u64 Pattern[2][128];

	if (BusWidth == XMT_DDR_CONFIG_64BIT_WIDTH) {
		for (Index = 0U; Index < 128U; Index++) {
			InvMaskInd = (Index >> 4) & 0x07;
			Pattern[0][Index] = Pattern64Bit[Index & 15];
			Pattern[1][Index] = Pattern64Bit[Index & 15] ^
					InvertMask64Bit[InvMaskInd];
		}
	} else {
		for (Index = 0U; Index < 128U; Index++) {
			InvMaskInd = (Index >> 4) & 0x07;
			Pattern[0][Index] = Pattern32Bit[Index & 15];
			Pattern[1][Index] = Pattern32Bit[Index & 15] ^
					InvertMask32Bit[InvMaskInd];
		}
	}

	XMt_PrintMemTestHeader(XMtPtr);

	for (Mode = 0U; Mode < XMT_MAX_MODE_NUM; Mode++) {
		if ((Mode == 0U) || (Mode > 10U)) {
			XMt_Memtest(XMtPtr, TestStart, TestSize, Mode, NULL);
		} else if (Mode <= 8U) {
			if (BusWidth == XMT_DDR_CONFIG_64BIT_WIDTH) {
				XMt_Memtest(XMtPtr, TestStart, TestSize, Mode,
					    TestPattern[Mode]);
			} else {
				XMt_Memtest(XMtPtr, TestStart, TestSize, Mode,
					    TestPattern[Mode + 4]);
			}
		} else {
			XMt_Memtest(XMtPtr, TestStart, TestSize,  Mode,
				    &Pattern[Mode - 9][0]);
		}
	}
}

/*****************************************************************************/
/**
 * This function does Read/Write Eye tests and Updated RESULTS registers
 * accordingly. This function is used by Read Eye and Write Eye Tests.
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 * @param StartAddr is the starting Address
 * @param Len is the Size of the memory to be Tested
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XMt_RunEyeMemtest(XMt_CfgData *XMtPtr, u64 StartAddr, u32 Len)
{
	u32 DataPtr;
	u32 RegVal;
	u64 Addr;
	u64 ReadVal;
	u64 ExpectedVal;
	u8 Lane;
	u8 Offset;

	/* For 64Bit increment offset by 8, for 32Bit increment offset by 4 */
	if (XMtPtr->DdrConfigLanes == XMT_DDR_CONFIG_8_LANE) {
		Offset = 8U;
	} else {
		Offset = 4U;
	}

	DataPtr = 0U;

	/* Do the Write operation on memory size specified in argument */
	for (Addr = StartAddr; Addr < (StartAddr+(Len*XMT_KB2BYTE)); Addr += Offset) {
		if (Offset == 8U) {
			Xil_Out64(Addr, AggressorPattern64Bit[DataPtr]);
			DataPtr++;
			DataPtr = (DataPtr) % (sizeof(AggressorPattern64Bit)/sizeof(u64));
		} else {
			Xil_Out32(Addr, AggressorPattern32Bit[DataPtr]);
			DataPtr++;
			DataPtr = (DataPtr) % (sizeof(AggressorPattern32Bit)/sizeof(u32));
		}
	}

	if (XMtPtr->DCacheEnable != 0U) {
		Xil_DCacheInvalidateRange(StartAddr, Len * XMT_KB2BYTE);
	}

	DataPtr = 0U;

	/* Do the Read operation on memory size specified in argument */
	for (Addr = StartAddr; Addr < (StartAddr+(Len*XMT_KB2BYTE)); Addr += Offset) {
		if (Offset == 8U) {
			ReadVal = Xil_In64(Addr);
			ExpectedVal = AggressorPattern64Bit[DataPtr];
			DataPtr++;
			DataPtr = (DataPtr) % (sizeof(AggressorPattern64Bit)/sizeof(u64));
		} else {
			ReadVal = Xil_In32(Addr);
			ExpectedVal = AggressorPattern32Bit[DataPtr];
			DataPtr++;
			DataPtr = (DataPtr) % (sizeof(AggressorPattern32Bit)/sizeof(u32));
		}

		/* Compare the Read Value and the Expected Value */
		if (ReadVal != ExpectedVal) {
			for (Lane = 0U; Lane < XMtPtr->DdrConfigLanes; Lane++) {
				if ((((ReadVal ^ ExpectedVal) >>
						(Lane * 8)) & 0xFF) != 0U) {
					RegVal = Xil_In32(XMT_RESULTS_BASE +
							  (Lane*4));
					Xil_Out32(XMT_RESULTS_BASE + (Lane*4),
						  RegVal + 1);
				}
			}
		}
	}
}

/*****************************************************************************/
/**
 * This is the Memory Test main function.
 *
 * @param none
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
int main(void)
{
	XMt_CfgData XMt;
	s32 Iter;
	s32 RankArg;
	u64 StartAddr;
	u64 TestSize;
	u32 Status;
	u32 Index;
	u32 BusWidth;
	s8 Ch;
	s8 SizeChar;

	/* By Default, the Verbose Mode is Disabled */
	Verbose = 0U;

	/* Set the Default Starting Address */
	StartAddr = 0x0U;

	/* Set the Default number of Iterations */
	Iter = 1U;

	/* Get the DDR configurations */
	Status = XMt_GetDdrConfigParams(&XMt);
	if (Status == XST_FAILURE) {
		xil_printf("Error getting DDR configurations\r\n");
		goto RETURN_PATH;
	}

	BusWidth = XMt.BusWidth;
	/* If the ECC is Enabled, Disable the Caches */
	if (XMt.EccEnabled != 0U) {
		Xil_DCacheDisable();
		XMt.DCacheEnable = 0U;
	} else {
		Xil_DCacheEnable();
		XMt.DCacheEnable = 1U;
	}

	/* Print the Help Menu for different operations */
	XMt_PrintHelp();

	/* Below code will run continuously waiting for a user input */
	while (1) {
		xil_printf("\r\n \tBus Width = %d,  ", XMt.BusWidth);
		xil_printf(" D-cache is %s,  ",
			   (XMt.DCacheEnable) ? "enable" : "disable");
		xil_printf(" Verbose Mode is %s\r\n\r\n",
			   (Verbose) ? "ON" : "OFF");
		xil_printf(" DDR ECC is %s\r\n",
			   XMt.EccEnabled ? "ENABLED" : "DISABLED");
		xil_printf(" Enter 'h' to print help menu\r\n");
		xil_printf(" Enter Test Option:\r\n");

		/* Get Keyboard Input */
		Ch = inbyte();
		if (Ch == '\r') {
			outbyte('\n');
		}
		outbyte(Ch);

		if (((Ch >= '0') && (Ch <= '9')) ||
			((Ch == 'm') || (Ch == 'M')) ||
			((Ch == 'g') || (Ch == 'G'))) {
			if ((Ch >= '0') && (Ch <= '9')) {
				TestSize = 0x10 << (Ch - '0');
			} else {
				xil_printf("\r\n Enter the size in %s : ",
						((Ch == 'm') || (Ch == 'M')) ? "MB" : "GB");
				TestSize = 0;
				do {
					SizeChar = inbyte();
					xil_printf("%c", SizeChar);
					if ((SizeChar >= '0') && (SizeChar <= '9')) {
						TestSize = (TestSize * 10) + (SizeChar - '0');
					} else if ((SizeChar != '\r') && (SizeChar != '\n')) {
						TestSize = 0;
						xil_printf("\r\nPlease enter numeric value : ");
					} else {
						outbyte('\n');
					}
				} while ((SizeChar != '\n') && (SizeChar != '\r'));

				TestSize = ((Ch == 'g') || (Ch == 'G')) ?
							(TestSize * XMT_KB2BYTE) : TestSize;
			}

			if (((StartAddr + TestSize) * XMT_MB2BYTE) <=
					XMT_DDR_MAX_SIZE) {
				xil_printf("\r\nStarting Memory Test...\r\n");
				xil_printf("%dMB length - Address 0x%x...\r\n",
					   TestSize, StartAddr);
				XMt_MemtestAll(&XMt, StartAddr, TestSize,
					       XMt.BusWidth);
			} else {
				xil_printf("\r\nPlease select the address within DDR range\r\n");
			}

		} else if ((Ch == 'r') || (Ch == 'R')) {
			for (Index = 0; Index < Iter; Index++) {
				Status = XMt_MeasureRdEye(&XMt, StartAddr,
							  XMT_DEFAULT_TEST_LEN);
				if (Status != XST_SUCCESS) {
					Status = XST_FAILURE;
					goto RETURN_PATH;
				}
			}

		} else if ((Ch == 'w') || (Ch == 'W')) {
			for (Index = 0; Index < Iter; Index++) {
				Status = XMt_MeasureWrEye(&XMt, StartAddr,
							  XMT_DEFAULT_TEST_LEN);
				if (Status != XST_SUCCESS) {
					Status = XST_FAILURE;
					goto RETURN_PATH;
				}
			}

		} else if ((Ch == 'c') || (Ch == 'C')) {
			for (Index = 0; Index < Iter; Index++) {
				Status = XMt_MeasureRdEye2D(&XMt, StartAddr,
							  XMT_DEFAULT_TEST_LEN);
				if (Status != XST_SUCCESS) {
					Status = XST_FAILURE;
					goto RETURN_PATH;
				}
			}

		} else if ((Ch == 'e') || (Ch == 'E')) {
			for (Index = 0; Index < Iter; Index++) {
				Status = XMt_MeasureWrEye2D(&XMt, StartAddr,
							  XMT_DEFAULT_TEST_LEN);
				if (Status != XST_SUCCESS) {
					Status = XST_FAILURE;
					goto RETURN_PATH;
				}
			}

		} else if ((Ch == 'a') || (Ch == 'A')) {
			xil_printf("Test Start address = 0x%016lx\r\n", StartAddr);

		} else if ((Ch == 'l') || (Ch == 'L')) {
			xil_printf("Enter the number of iterations : ");
			scanf("%d", &Iter);

		} else if ((Ch == 't') || (Ch == 'T')) {
			xil_printf("Please enter the Start address in hex"
				"(without leading 0x and press enter):");
			scanf("%lx", &StartAddr);

		} else if ((Ch == 's') || (Ch == 'S')) {
			xil_printf("\r\nEnter rank to select:");
			Ch = inbyte();
			outbyte(Ch);
			RankArg = Ch - '0';

			if (RankArg > XMt.DdrConfigRanks || RankArg < 1) {
				xil_printf("\r\nInvalid Selection. "
					"Available no. of ranks: %d."
					"Rank selected: %d\r\n",
					XMt.DdrConfigRanks, RankArg);
			} else {
				XMt_SelectRank(RankArg);
			}

		} else if ((Ch == 'i') || (Ch == 'I')) {
			XMt_PrintDdrConfigParams(&XMt);

		} else if ((Ch == 'v') || (Ch == 'V')) {
			Verbose = Verbose ^ 1U;
			xil_printf(" Verbose Mode = %d \r\n", Verbose);

		} else if ((Ch == 'o') || (Ch == 'O')) {
			XMt.DCacheEnable ^= 1;
			if (XMt.DCacheEnable == 0) {
				Xil_DCacheDisable();
				xil_printf("\r\n D-Cache Disabled\r\n");
			} else {
				Xil_DCacheEnable();
				xil_printf("\r\n D-Cache Enabled\r\n");
			}

		} else if ((Ch == 'b') || (Ch == 'B')) {
			if (BusWidth == XMT_DDR_CONFIG_64BIT_WIDTH) {
				if (XMt.BusWidth == XMT_DDR_CONFIG_32BIT_WIDTH) {
					XMt.BusWidth = XMT_DDR_CONFIG_64BIT_WIDTH;
					XMt.DdrConfigLanes = XMT_DDR_CONFIG_8_LANE;
				} else {
					XMt.BusWidth = XMT_DDR_CONFIG_32BIT_WIDTH;
					XMt.DdrConfigLanes = XMT_DDR_CONFIG_4_LANE;
				}
				xil_printf(" Bus Width = %d \r\n", XMt.BusWidth);
			} else {
				xil_printf("Bus Width Switching not supported\r\n");
			}

		} else if ((Ch == 'h') || (Ch == 'H')) {
			XMt_PrintHelp();

		} else if ((Ch == 'q') || (Ch == 'Q')) {
			xil_printf("Exiting the DRAM Test\r\n", StartAddr);
			goto RETURN_PATH;

		}
	}

RETURN_PATH:
	/*
	 * In the normal working condition, it should never reach here,
	 * unless the user wants to.
	 */
	return 0;
}
