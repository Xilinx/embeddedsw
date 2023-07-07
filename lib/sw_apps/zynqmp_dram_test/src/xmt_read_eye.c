/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/


/*****************************************************************************/
/**
 *
 * @file xmt_read_eye.c
 *
 * This is the file containing code for DDR Read Eye Tests. This measures
 * Right Eye Edge, Left Eye Edge, Read Eye Width, Taps per Cycle and Read Eye
 * Center.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   mn   08/17/18 Initial release
 *       mn   09/27/18 Modify code to add 2D Read/Write Eye Tests support
 * 1.1   mn   03/10/21 Fixed doxygen warnings
 * 1.2   mn   05/13/21 Fixed issue with mismatching read eye width
 * 1.3   mn   06/10/21 Modify code to sweep VRef from 0 to 127 values
 *       mn   09/08/21 Removed illegal write to DXnGTR0.WDQSL register field
 * 1.4   mn   11/29/21 Updated print information for read/write eye tests
 *       mn   11/29/21 Usability Enhancements for 2D Read/Write Eye
 *       sg   11/11/22 Added 2D Read Eye support for LPDDR4
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/

/***************************** Include Files *********************************/

#include "xmt_common.h"

/************************** Constant Definitions *****************************/

#define XMT_LANE0LCDLR3_OFFSET	XMT_DDR_PHY_DX0LCDLR3
#define XMT_LANE0LCDLR4_OFFSET	XMT_DDR_PHY_DX0LCDLR4

#define XMT_DDR4_READ_VREF_MAX		0x74U
#define XMT_DDR4_READ_VREF_MIN		0x30U
#define XMT_LPDDR4_READ_VREF_MAX		0x7FU
#define XMT_LPDDR4_READ_VREF_MIN		0x0U

#define XMT_PSEC	1000000000000

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * This function is used to get the Read QS Delay
 *
 * @param Addr is the address of LCDL3 register
 *
 * @return RDQSD value
 *
 * @note none
 *****************************************************************************/
static INLINE u32 XMt_GetRdQsd(u32 Addr)
{
	return XMt_GetRegValue(Addr, XMT_DDR_PHY_DX0LCDLR3_RDQSD_MASK,
			       XMT_DDR_PHY_DX0LCDLR3_RDQSD_SHIFT);
}

/*****************************************************************************/
/**
 * This function is used to get the Read DQSN Delay
 *
 * @param Addr is the address of LCDL3 register
 *
 * @return RDQSND value
 *
 * @note none
 *****************************************************************************/
static INLINE u32 XMt_GetRdQsnd(u32 Addr)
{
	return XMt_GetRegValue(Addr, XMT_DDR_PHY_DX0LCDLR4_RDQSND_MASK,
			       XMT_DDR_PHY_DX0LCDLR4_RDQSND_SHIFT);
}

/*****************************************************************************/
/**
 * This function is used to set the Read Eye Center values
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 *
 * @return XST_SUCCESS on success, XST_FAILURE on failure
 *
 * @note none
 *****************************************************************************/
static u32 XMt_GetRdCenter(XMt_CfgData *XMtPtr)
{
	s32 Index;

	if (XMtPtr->ReadCenterFetched == 1U) {
		return XST_SUCCESS;
	}

	for (Index = 0; Index < XMtPtr->DdrConfigLanes; Index++) {
		XMtPtr->RdCenter[Index].Qsd = XMt_GetRdQsd(XMT_LANE0LCDLR3_OFFSET +
				(XMT_LANE_OFFSET * Index));
		XMtPtr->RdCenter[Index].Qsnd = XMt_GetRdQsnd(XMT_LANE0LCDLR4_OFFSET +
				(XMT_LANE_OFFSET * Index));
	}

	XMtPtr->ReadCenterFetched = 1U;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function is used to reset the Read Eye Center values
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 *
 * @return XST_SUCCESS on success, XST_FAILURE on failure
 *
 * @note none
 *****************************************************************************/
static u32 XMt_ResetRdCenter(XMt_CfgData *XMtPtr)
{
	s32 Index;

	/* Disable the Refresh during training */
	XMt_DisableRefresh();

	for (Index = 0; Index < XMtPtr->DdrConfigLanes; Index++) {
		Xil_Out32(XMT_LANE0LCDLR3_OFFSET + (XMT_LANE_OFFSET*Index),
				XMtPtr->RdCenter[Index].Qsd);
		Xil_Out32(XMT_LANE0LCDLR4_OFFSET + (XMT_LANE_OFFSET*Index),
				XMtPtr->RdCenter[Index].Qsnd);
	}

	/* Enable the Refresh after training */
	XMt_EnableRefresh();

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function is used to print the Read Eye Center values
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
static void XMt_PrintRdCenter(XMt_CfgData *XMtPtr)
{
	s32 Index;

	xil_printf(" AUTO CENTER (Delay Line Taps DQS_T, DQS_C):\r\n");
	XMt_PrintLine(XMtPtr, 3);
	for (Index = 0; Index < XMtPtr->DdrConfigLanes; Index++) {
		XMt_CalcPerTapDelay(XMtPtr, Index);
		xil_printf("   %2d,%2d |", XMtPtr->RdCenter[Index].Qsd,
				XMtPtr->RdCenter[Index].Qsnd);
	}
}

#ifdef XMT_DEBUG
/*****************************************************************************/
/**
 * This function is used to print the QSD and QSND values
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
static void XMt_PrintQsdQsnd(XMt_CfgData *XMtPtr)
{
	s32 Index;

	xil_printf("Lane(QSD/QSND): ");
	for (Index = 0; Index < XMtPtr->DdrConfigLanes; Index++) {
		xil_printf("%d(%d,%d), ", Index,
			XMt_GetRdQsd(XMT_LANE0LCDLR3_OFFSET +
			(XMT_LANE_OFFSET*Index)),
			XMt_GetRdQsnd(XMT_LANE0LCDLR4_OFFSET +
			(XMT_LANE_OFFSET*Index)));
	}
	xil_printf("\r\n");
}
#endif

/*****************************************************************************/
/**
 * This function is used to set the QSD and QSND values.
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 * @param Position is the number indicating the Eye Scanning distance from
 *        the Eye Center
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
static void XMt_SetRdDqsDelay(XMt_CfgData *XMtPtr, s32 Position)
{
	s32 Index;

	for (Index = 0; Index < XMtPtr->DdrConfigLanes; Index++) {
		if (((Position > 0) && XMtPtr->EyeEnd[Index] == 0) ||
		    ((Position < 0) && XMtPtr->EyeStart[Index] == 0)) {
			if ((((s32)XMtPtr->RdCenter[Index].Qsd + Position) >= 0) &&
			    (((s32)XMtPtr->RdCenter[Index].Qsd + Position) <= 511))
				Xil_Out32(XMT_LANE0LCDLR3_OFFSET +
						(XMT_LANE_OFFSET*Index),
						(XMtPtr->RdCenter[Index].Qsd) +
						Position);
			if ((((s32)XMtPtr->RdCenter[Index].Qsnd + Position) >= 0) &&
			    (((s32)XMtPtr->RdCenter[Index].Qsnd + Position) <= 511))
				Xil_Out32(XMT_LANE0LCDLR4_OFFSET +
						(XMT_LANE_OFFSET*Index),
						(XMtPtr->RdCenter[Index].Qsnd) +
						Position);
		}
	}

#ifdef XMT_DEBUG
	XMt_PrintQsdQsnd();
#endif
}

/*****************************************************************************/
/**
 * This function is used to measure the Read Eye Edge values of the DDR.
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 * @param TestAddr is the Starting Address
 * @param Len is the length of the memory to be tested
 * @param Mode is the flag indication whether to test right eye or left eye
 *
 * @return XST_SUCCESS on success, XST_FAILURE on failure
 *
 * @note none
 *****************************************************************************/
static u32 XMt_MeasureRdEyeEdge(XMt_CfgData *XMtPtr, u64 TestAddr, u32 Len, u8 Mode)
{
	u32 Index;
	s32 Done;
	s32 Position;

	Done = 0;
	Position = 0;

	while (!Done) {
		if (Mode & XMT_RIGHT_EYE_TEST) {
			/* Move towards right edge */
			Position++;
		} else {
			/* Move towards left edge */
			Position--;
		}

		/* Clear system registers */
		XMt_ClearResults(XMtPtr, XMT_RESULTS_BASE);

		/* Set the QSD and QSND register values based on position */
		XMt_SetRdDqsDelay(XMtPtr, Position);

		/* Do the Write/Read test on Address Range */
		XMt_RunEyeMemtest(XMtPtr, TestAddr, Len);

		/* Update the result to 1 when the position exceeds the left eye edge */
		for (Index = 0; Index < XMtPtr->DdrConfigLanes; Index++) {
			if ((((s32)XMtPtr->RdCenter[Index].Qsd + Position) < 0) &&
					((((s32)XMtPtr->RdCenter[Index].Qsnd + Position) < 0))) {
				if (Xil_In32(XMT_RESULTS_BASE + (Index * 4U)) == 0U) {
					Xil_Out32(XMT_RESULTS_BASE + (Index * 4U), 1U);
				}
			}
		}

		if (!(Mode & XMT_2D_EYE_TEST)) {
			/* Print the lane wise results for this Position */
			xil_printf("%3d    |", Position);
			XMt_PrintResults(XMtPtr);
		}

		/* Calculate the Eye Start/End position values */
		for (Index = 0; Index < XMtPtr->DdrConfigLanes; Index++) {
			if (Mode & XMT_RIGHT_EYE_TEST) {
				if ((Xil_In32(XMT_RESULTS_BASE +
						(Index * 4)) > 0) &&
						XMtPtr->EyeEnd[Index] == 0) {
					XMtPtr->EyeEnd[Index] = Position - 1;
				}
			} else {
				if ((Xil_In32(XMT_RESULTS_BASE +
						(Index * 4)) > 0) &&
						XMtPtr->EyeStart[Index] == 0) {
					XMtPtr->EyeStart[Index] = Position + 1;
				}
			}
		}

		Done = 1;
		/* Once all Eye Start/End values are non-zero, End the test */
		for (Index = 0; Index < XMtPtr->DdrConfigLanes; Index++) {
			if (Mode & XMT_RIGHT_EYE_TEST) {
				if (!XMtPtr->EyeEnd[Index] &&
				    (((XMtPtr->RdCenter[Index].Qsd + Position) < 511) ||
				((XMtPtr->RdCenter[Index].Qsnd + Position) < 511))) {
					Done = 0;
				}
			} else {
				if (!XMtPtr->EyeStart[Index]) {
					if ((abs(Position) <
						XMtPtr->RdCenter[Index].Qsd) ||
					(abs(Position) <
						XMtPtr->RdCenter[Index].Qsnd)) {
						Done = 0;
					} else {
						XMtPtr->EyeStart[Index] = Position + 1;
					}
				}
			}
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function is used to print the Read Eye Test Results
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
static void XMt_PrintReadEyeResults(XMt_CfgData *XMtPtr)
{
	u32 Index;
	float TempVal;

	XMt_PrintLine(XMtPtr, 2);
	xil_printf("\r\nRead Eye Test Results :\r\n");

	/* calculate the average */
	XMt_PrintEyeResultsHeader(XMtPtr);
	XMt_PrintRdCenter(XMtPtr);
	xil_printf("\r\n");
	XMt_PrintLine(XMtPtr, 3);
	xil_printf(" TAP Value (ps):\r\n");
	XMt_PrintLine(XMtPtr, 3);
	for (Index = 0; Index < XMtPtr->DdrConfigLanes; Index++) {
		XMtPtr->TapPs = XMt_CalcPerTapDelay(XMtPtr, Index);
		xil_printf("   %d.%02d  |", (int)XMtPtr->TapPs,
			   (int)((XMtPtr->TapPs - (int)XMtPtr->TapPs)*100.0f));
	}
	xil_printf("\r\n");
	XMt_PrintLine(XMtPtr, 3);
	xil_printf(" TAPS/cycle:\r\n");
	XMt_PrintLine(XMtPtr, 3);
	for (Index = 0; Index < XMtPtr->DdrConfigLanes; Index++) {
		XMtPtr->TapPs = XMt_CalcPerTapDelay(XMtPtr, Index);
		xil_printf("    %d   |", (s32)((1000000 /
				(XMtPtr->DdrFreq * 4)) / XMtPtr->TapPs));
	}
	xil_printf("\r\n");
	XMt_PrintLine(XMtPtr, 3);
	xil_printf(" EYE WIDTH (ps):\r\n");
	XMt_PrintLine(XMtPtr, 3);
	for (Index = 0; Index < XMtPtr->DdrConfigLanes; Index++) {
		XMtPtr->TapPs = XMt_CalcPerTapDelay(XMtPtr, Index);
		TempVal = (float) (XMtPtr->TapPs *
				(XMtPtr->EyeEnd[Index] - XMtPtr->EyeStart[Index]));
		xil_printf("  %d.%02d |", (int)TempVal, (int)((TempVal - (int)TempVal)*100.0f));
	}
	xil_printf("\r\n");
	XMt_PrintLine(XMtPtr, 3);
	xil_printf(" EYE WIDTH (%%):\r\n");
	XMt_PrintLine(XMtPtr, 3);
	for (Index = 0; Index < XMtPtr->DdrConfigLanes; Index++) {
		XMtPtr->TapPs = XMt_CalcPerTapDelay(XMtPtr, Index);
		TempVal = 100 * (XMtPtr->TapPs * (XMtPtr->EyeEnd[Index] -
				XMtPtr->EyeStart[Index])) / (1000000 / (XMtPtr->DdrFreq * 4));
		xil_printf("  %d.%02d  |", (int)TempVal, (int)((TempVal - (int)TempVal)*100.0f));
	}
	xil_printf("\r\n");
	XMt_PrintLine(XMtPtr, 3);
	xil_printf(" EYE CENTER (Delay Line Taps DQS_T, DQS_C):\r\n");
	XMt_PrintLine(XMtPtr, 3);
	for (Index = 0; Index < XMtPtr->DdrConfigLanes; Index++) {
		xil_printf("   %2d,%2d |", ((XMtPtr->RdCenter[Index].Qsd + XMtPtr->EyeEnd[Index]) +
				(XMtPtr->RdCenter[Index].Qsd + XMtPtr->EyeStart[Index])) / 2,
				((XMtPtr->RdCenter[Index].Qsnd + XMtPtr->EyeEnd[Index]) +
				(XMtPtr->RdCenter[Index].Qsnd + XMtPtr->EyeStart[Index]))/2);
	}
	xil_printf("\r\n");
	XMt_PrintLine(XMtPtr, 3);

}

/*****************************************************************************/
/**
 * This function is used for exception handling while running this test
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
static void XMt_RdEyeSyncAbortHandler(void)
{
	u64 RetAddr;

	RetAddr = mfelrel3();
	RetAddr = RetAddr + 4;
	mtelrel3(RetAddr);
}

/*****************************************************************************/
/**
 * This function is used for exception handling while running this test
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
static void XMt_RdEyeSerrorAbortHandler(void)
{
	;
}

/*****************************************************************************/
/**
 * This function is used to measure the Read Eye of the DDR.
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 * @param TestAddr is the Starting Address
 * @param Len is the length of the memory to be tested
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
u32 XMt_MeasureRdEye(XMt_CfgData *XMtPtr, u64 TestAddr, u32 Len)
{
	Xil_ExceptionHandler SyncHandler;
	Xil_ExceptionHandler SerrorHandler;
	void *SyncData;
	void *SerrorData;
	u32 Status;

	xil_printf("\r\nRunning Read Eye Tests\r\n");

	/* Get the system handlers for Sync and SError exceptions */
	Xil_GetExceptionRegisterHandler(XIL_EXCEPTION_ID_SYNC_INT,
			&SyncHandler, &SyncData);
	Xil_GetExceptionRegisterHandler(XIL_EXCEPTION_ID_SERROR_ABORT_INT,
			&SerrorHandler, &SerrorData);

	/* Register the Exception Handlers for Sync and SError exceptions */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_SYNC_INT,
			(Xil_ExceptionHandler)XMt_RdEyeSyncAbortHandler,
			(void *) 0);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_SERROR_ABORT_INT,
			(Xil_ExceptionHandler)XMt_RdEyeSerrorAbortHandler,
			(void *) 0);

	/* Initialize Eye Parameters with zero */
	XMt_ClearEye(XMtPtr, (u32 *)&XMtPtr->EyeStart[0]);
	XMt_ClearEye(XMtPtr, (u32 *)&XMtPtr->EyeEnd[0]);

	/* Disable VT compensation */
	XMt_DisableVtcomp();

	/* Get the Read Eye Center values */
	Status = XMt_GetRdCenter(XMtPtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Print the Eye Test Header */
	XMt_PrintEyeHeader(XMtPtr);

	/* Measure the Right Edge of the Eye */
	Status = XMt_MeasureRdEyeEdge(XMtPtr, TestAddr, Len, XMT_RIGHT_EYE_TEST);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Measure the Left Edge of the Eye */
	Status = XMt_MeasureRdEyeEdge(XMtPtr, TestAddr, Len, XMT_LEFT_EYE_TEST);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Print the Read Eye Test Results */
	XMt_PrintReadEyeResults(XMtPtr);

	/* Reset the Read Eye Center values to Registers */
	Status = XMt_ResetRdCenter(XMtPtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Give back Exception Handling to the system defined handlers */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_SYNC_INT,
			SyncHandler, SyncData);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_SERROR_ABORT_INT,
			SerrorHandler, SerrorData);

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to measure the Read Eye of the DDR.
 *
 * @param XMtPtr is the pointer to the Memtest Data Structure
 * @param TestAddr is the Starting Address
 * @param Len is the length of the memory to be tested
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
u32 XMt_MeasureRdEye2D(XMt_CfgData *XMtPtr, u64 TestAddr, u32 Len)
{
	Xil_ExceptionHandler SyncHandler;
	Xil_ExceptionHandler SerrorHandler;
	void *SyncData;
	void *SerrorData;
	u32 Status;
	u32 VRef;
	u32 VRefMax;
	u32 VRefMin;

	xil_printf("\r\nRunning 2-D Read Eye Tests\r\n");

	if (XMtPtr->DdrType == XMT_DDR_TYPE_DDR4) {
		VRefMax = XMT_DDR4_READ_VREF_MAX;
		VRefMin = XMT_DDR4_READ_VREF_MIN;
	} else {
		VRefMax = XMT_LPDDR4_READ_VREF_MAX;
		VRefMin = XMT_LPDDR4_READ_VREF_MIN;
	}

	/* Get the system handlers for Sync and SError exceptions */
	Xil_GetExceptionRegisterHandler(XIL_EXCEPTION_ID_SYNC_INT,
			&SyncHandler, &SyncData);
	Xil_GetExceptionRegisterHandler(XIL_EXCEPTION_ID_SERROR_ABORT_INT,
			&SerrorHandler, &SerrorData);

	/* Register the Exception Handlers for Sync and SError exceptions */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_SYNC_INT,
			(Xil_ExceptionHandler)XMt_RdEyeSyncAbortHandler,
			(void *) 0);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_SERROR_ABORT_INT,
			(Xil_ExceptionHandler)XMt_RdEyeSerrorAbortHandler,
			(void *) 0);

	XMt_Print2DEyeResultsHeader(XMtPtr);

	/* Disable the DFI */
	XMt_DfiDisable();

	/* Disable VT compensation */
	XMt_DisableVtcomp();

	/* Get the Read Eye Center values */
	Status = XMt_GetRdCenter(XMtPtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	Status = XMt_GetVRefAuto(XMtPtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	for (VRef = VRefMax; VRef >= VRefMin; VRef--) {

		XMt_SetVrefVal(XMtPtr, VRef);

		/* Initialize Eye Parameters with zero */
		XMt_ClearEye(XMtPtr, (u32 *)&XMtPtr->EyeStart[0]);
		XMt_ClearEye(XMtPtr, (u32 *)&XMtPtr->EyeEnd[0]);

		/* Measure the Right Edge of the Eye */
		Status = XMt_MeasureRdEyeEdge(XMtPtr, TestAddr, Len,
				XMT_RIGHT_EYE_TEST | XMT_2D_EYE_TEST);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto RETURN_PATH;
		}

		/* Measure the Left Edge of the Eye */
		Status = XMt_MeasureRdEyeEdge(XMtPtr, TestAddr, Len,
				XMT_LEFT_EYE_TEST | XMT_2D_EYE_TEST);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto RETURN_PATH;
		}

		/* Print the Read Eye Test Results */
		XMt_Print2DReadEyeResults(XMtPtr, VRef);

		/* Reset the Read Eye Center values to Registers */
		Status = XMt_ResetRdCenter(XMtPtr);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto RETURN_PATH;
		}

		if(VRef == 0)
			break;
	}

	XMt_PrintLine(XMtPtr, 5);

	XMt_ResetVrefAuto(XMtPtr);

	/* Enable the DFI */
	XMt_DfiEnable();

	/* Give back Exception Handling to the system defined handlers */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_SYNC_INT,
			SyncHandler, SyncData);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_SERROR_ABORT_INT,
			SerrorHandler, SerrorData);

RETURN_PATH:
	return Status;
}
