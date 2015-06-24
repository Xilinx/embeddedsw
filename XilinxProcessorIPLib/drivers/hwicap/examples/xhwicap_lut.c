/******************************************************************************
*
* Copyright (C) 2003 - 2014 Xilinx, Inc.  All rights reserved.
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
* @file xhwicap_lut.c
*
* This example tests some of the values in a LUT. This example requires the
* use of a LUT not in use. One can reserve a LUT in the UCF file with the
* constraint: CONFIG PROHIBIT = SLICE_X0Y0
*
*
* @note
*
* This example should run on any Virtex4 or a Virtex5 or a Virtex6 device.
*
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ------------------------------------------------------
* 1.00a bjb  10/30/03 Original version
* 1.00a bjs  01/01/04 First release
* 1.00a bjs  03/08/04 Updated for EDK 6.2.1
* 1.00a sv   07/18/05 Minor changes to comply to Doxygen and coding guidelines
* 2.00a ecm  10/04/07 Changes to support FIFO mode and V5
* 4.00a hvm  11/30/09 Added support for V6
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include <xparameters.h>
#include <xhwicap.h>
#if ((XHI_FAMILY == XHI_DEV_FAMILY_V4) || (XHI_FAMILY == XHI_DEV_FAMILY_V5 ) ||\
	(XHI_FAMILY == XHI_DEV_FAMILY_V6))
#include <xhwicap_clb_lut.h>
#endif
#include <stdio.h>

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define HWICAP_DEVICEID		XPAR_HWICAP_0_DEVICE_ID


/*
 * This is the LUT that is  tested . This LUT must not be used by the design.
 * This can be done in the system.ucf file by putting the following statements:
 *
 * CONFIG PROHIBIT = SLICE_X[COL]Y[ROW];
 *
 * If you wish to reserve the entire column then put the following statement in
 * the ucf file:
 *
 * CONFIG PROHIBIT = SLICE_X[COL]Y*;
 *
 */
#define TEST_COL	0	/* Test Column for LUT */
#define TEST_ROW	0	/* Test Row for LUT    */


#if XHI_FAMILY == XHI_DEV_FAMILY_V4
#define TEST_LUT	XHI_CLB_LUT_F	/* Test LUT */
#define LUT_SIZE	16	/* The number of bits in a LUT */
#elif ((XHI_FAMILY == XHI_DEV_FAMILY_V5) || (XHI_FAMILY == XHI_DEV_FAMILY_V6))
#define TEST_LUT	XHI_CLB_LUT_A	/* Test LUT */
#define LUT_SIZE	64	/* The number of bits in a LUT */
#endif




#define MAX_TEST_COUNT	0xFF

#define printf		xil_printf      /* A smaller footprint printf */


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int HwIcapLutExample(u16 DeviceId);


/************************** Variable Definitions *****************************/

static XHwIcap HwIcap;

u8 LutWriteBuffer[LUT_SIZE];	/* Value written to the LUT     */
u8 LutReadBuffer[LUT_SIZE];	/* Value read back from the LUT */


/*****************************************************************************/
/**
*
* Main function to call the HwIcap LUT example.
*
* @param    None
*
* @return   XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note     None
*
******************************************************************************/
#if ((XHI_FAMILY == XHI_DEV_FAMILY_V4) || (XHI_FAMILY == XHI_DEV_FAMILY_V5 ) ||\
	(XHI_FAMILY == XHI_DEV_FAMILY_V6))
int main(void)
{
	int Status;

	/*
	 * Run the HwIcap LUT example, specify the Device ID generated in
	 * xparameters.h
	 */
	Status = HwIcapLutExample(HWICAP_DEVICEID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}


/*****************************************************************************/
/**
*
* This function does a test on the LUT as an  example.
*
* @param	DeviceId is the XPAR_<HWICAP_INSTANCE>_DEVICE_ID value from
*		xparameters.h
*
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE
*
* @note		None.
*
******************************************************************************/
int HwIcapLutExample(u16 DeviceId)
{
	XHwIcap_Config *CfgPtr; /* Pointer to HwIcap Config */
	u32 Count;		/* Current value to test */
	int Status;		/* Return value          */
	u32 Index;		/* Counter               */
	u32 RowNum;		/* CLB Row location      */
	u32 ColNum;		/* CLB Column location   */
	u32 Slice;		/* CLB Slice location    */

	printf("\r\n Starting HwIcapLutExample. \r\n");
	/*
	 * Initialize the HwIcap instance.
	 */
	CfgPtr = XHwIcap_LookupConfig(DeviceId);
	if (CfgPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XHwIcap_CfgInitialize(&HwIcap, CfgPtr, CfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XHwIcap_SelfTest(&HwIcap);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Identify the LUT to change: LUT in SLICE_X0Y0.
	 */
	ColNum = XHwIcap_SliceX2Col(TEST_COL);
	RowNum = XHwIcap_SliceY2Row(&HwIcap, TEST_ROW);
	Slice  = XHwIcap_SliceXY2Slice(TEST_COL, TEST_ROW);

	/*
	 * Loop through all possible LUT Values
	 */
	for (Count = 0; Count < MAX_TEST_COUNT; Count++) {

		/*
		 * Set the LUT array to be assigned
		 */
		for (Index = 0; Index < LUT_SIZE; Index++) {
			LutWriteBuffer[Index] = (Count >> Index) & 0x01;
			LutReadBuffer[Index] = 0;
		}

		/*
		 * Set LUT
		 */
		Status = XHwIcap_SetClbBits(&HwIcap, RowNum, ColNum,
			 		XHI_CLB_LUT.CONTENTS[Slice][TEST_LUT],
			 		LutWriteBuffer, LUT_SIZE);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*
		 * Read back LUT value
		 */
		Status = XHwIcap_GetClbBits(&HwIcap, RowNum, ColNum,
					XHI_CLB_LUT.CONTENTS[Slice][TEST_LUT],
					LutReadBuffer, LUT_SIZE);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*
		 * Compare the written and read values
		 */
		for (Index = 0; Index < LUT_SIZE; Index++) {
			if (LutWriteBuffer[Index] != LutReadBuffer[Index]) {
			printf("\r\n HwIcapLutExample Failed. Cnt = %d , Index = %d WrBuf = %d RdBuf= %d\
					\r\n", Count, Index, LutWriteBuffer[Index], LutReadBuffer[Index] );
			return XST_FAILURE;
			}
		}


	}

	printf("\r\n HwIcapLutExample Passed Successfully.\r\n");
	return XST_SUCCESS;
}
#else
#error Unsupported FPGA Family
#endif




