/******************************************************************************
*
* Copyright (C) 2010 - 2019 Xilinx, Inc.  All rights reserved.
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
* @file  xttcps_low_level_example.c
*
* This file contains a design example using the Triple Timer Counter hardware
* and driver in  polled mode.
*
* The example generates a square wave output on the waveform out pin.
*
* @note
*
* None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who    Date     Changes
* ---- ------ -------- --------------------------------------------------------
* 1.00 drg/jz 01/23/10 First release
* 3.0  pkp	  12/09/14 Change TTC_NUM_DEVICES for Zynq Ultrascale MP support
* 3.9  mus    04/09/19 Updated SettingsTable values as per TmrCntrSetup
*                      template
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xttcps.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define PCLK_FREQ_HZ		XPAR_XTTCPS_0_CLOCK_HZ /* Input freq */
#define TTC_NUM_DEVICES		2

#define TABLE_OFFSET		6
#define MAX_LOOP_COUNT		0xFF

/**************************** Type Definitions *******************************/

typedef struct {
	u32 OutputHz;		/* The frequency the timer should output on the
				   waveout pin */
	u8 OutputDutyCycle;	/* The duty cycle of the output wave as a
				   percentage */
	u8 PrescalerValue;	/* Value of the prescaler in the Count Control
				   register */
} TmrCntrSetup;

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

static int TmrCtrLowLevelExample(u8 SettingsTableOffset);

/************************** Variable Definitions *****************************/

/*
 * Convert from a 0-2 index to the correct timer counter number as defined in
 * xttcps_hw.h
 */
static u32 TimerCounterBaseAddr[] = {
	XPAR_XTTCPS_0_BASEADDR,
	XPAR_XTTCPS_1_BASEADDR
};

/*
 * A table of the actual prescaler setting based on the prescaler value from
 * 0-15. The setting is 2^(prescaler value + 1). Use a table to avoid doing
 * powers at run time.
 * A Prescaler value of 16, means use no prescaler, or a prescaler value of 1.
 */
static u32 PrescalerSettings[] = {
	2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384,
	32768, 65536, 1
};

static TmrCntrSetup SettingsTable[] = {
	/* Table offset of 0 */
	{10, 50, 6},
	{10, 25, 6},
	{10, 75, 6},

	/* Table offset of 3 */
	{100, 50, 3},
	{200, 25, 2},
	{400, 12, 1},

	/* Table offset of 6 */
	{500, 50, 1},
	{1000, 50, 0},
	{5000, 50, 16},

	/* Table offset of 9 */
	{10000, 50, 16},
	{50000, 50, 16},
	{100000, 50, 16},

	/* Table offset of 12 */
	{500000, 50, 16},
	{1000000, 50, 16},
	{5000000, 50, 16},
	/* Note: at greater than 1 MHz the timer reload is noticeable. */

};

#define SETTINGS_TABLE_SIZE  (sizeof(SettingsTable)/sizeof(TmrCntrSetup))

/*****************************************************************************/
/**
*
* This function is the main function of the Timer/Counter example.
*
* @param	None
*
* @return	XST_SUCCESS to indicate success, else XST_FAILURE to indicate
*		a Failure.
*
* @note		None.
*
*****************************************************************************/
int main(void)
{
	int Status;

	xil_printf("TTC Lowlevel Example Test \r\n");

	/*
	 * Run the Timer Counter - Low Level example.
	 */
	Status = TmrCtrLowLevelExample(TABLE_OFFSET);
	if (Status != XST_SUCCESS) {
		xil_printf("TTC Lowlevel Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran TTC Lowlevel Example Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function does a minimal test on the timer counter device and
* the low level driver as a design example.  The purpose of this function is
* to illustrate how to use the XTtcPs low level driver.
*
*
* @param	SettingsTableOffset is an offset into the settings table. This
*		allows multiple counter setups to be kept and swapped easily.
*
* @return	XST_SUCCESS to indicate success, else XST_FAILURE to indicate
*		a Failure.
*
* @note
*
* This function contains a loop which waits for the value of a timer counter
* to change.  If the hardware is not working correctly, this function may not
* return.
*
****************************************************************************/
int TmrCtrLowLevelExample(u8 SettingsTableOffset)
{
	u32 RegValue;
	u32 LoopCount;
	u32 TmrCtrBaseAddress;
	u32 IntervalValue, MatchValue;
	TmrCntrSetup *CurrSetup;

	/*
	 * Assert on the value of constants calculated above
	 * Because the counters are 16 bits, no value can be greater than 65535
	 */
	if ((SettingsTableOffset + 2) > SETTINGS_TABLE_SIZE) {
		return XST_FAILURE;
	}

	for (LoopCount = 0; LoopCount < TTC_NUM_DEVICES; LoopCount++) {
		/*
		 * Set the timer counter number to use
		 */
		TmrCtrBaseAddress = TimerCounterBaseAddr[LoopCount];

		/* And get the setup for that counter
		 */
		CurrSetup = &SettingsTable[SettingsTableOffset + LoopCount];

		/*
		 * Set the Clock Control Register
		 */
		if (16 > CurrSetup->PrescalerValue) {
			/* Use the clock prescaler */
			RegValue =
				(CurrSetup->
				 PrescalerValue <<
				 XTTCPS_CLK_CNTRL_PS_VAL_SHIFT) &
				XTTCPS_CLK_CNTRL_PS_VAL_MASK;
			RegValue |= XTTCPS_CLK_CNTRL_PS_EN_MASK;
		}
		else {
			/* Do not use the clock prescaler */
			RegValue = 0;
		}
		XTtcPs_WriteReg(TmrCtrBaseAddress, XTTCPS_CLK_CNTRL_OFFSET,
				  RegValue);


		/*
		 * Set the Interval register. This determines the frequency of
		 * the waveform. The counter will be reset to 0 each time this
		 * value is reached.
		 */
		IntervalValue = PCLK_FREQ_HZ /
			(u32) (PrescalerSettings[CurrSetup->PrescalerValue] *
			       CurrSetup->OutputHz);

		/*
		 * Make sure the value is not to large or too small
		 */
		if ((65535 < IntervalValue) || (4 > IntervalValue)) {
			return XST_FAILURE;
		}

		XTtcPs_WriteReg(TmrCtrBaseAddress,
				  XTTCPS_INTERVAL_VAL_OFFSET, IntervalValue);

		/*
		 * Set the Match register. This determines the duty cycle of the
		 * waveform. The waveform output will be toggle each time this
		 * value is reached.
		 */
		MatchValue = (IntervalValue * CurrSetup->OutputDutyCycle) / 100;

		/*
		 * Make sure the value is not to large or too small
		 */
		if ((65535 < MatchValue) || (4 > MatchValue)) {
			return XST_FAILURE;
		}
		XTtcPs_WriteReg(TmrCtrBaseAddress, XTTCPS_MATCH_0_OFFSET,
				  MatchValue);

		/*
		 * Set the Counter Control Register
		 */
		RegValue =
			~(XTTCPS_CNT_CNTRL_DIS_MASK |
			  XTTCPS_CNT_CNTRL_EN_WAVE_MASK) &
			(XTTCPS_CNT_CNTRL_INT_MASK |
			 XTTCPS_CNT_CNTRL_MATCH_MASK |
			 XTTCPS_CNT_CNTRL_RST_MASK);
		XTtcPs_WriteReg(TmrCtrBaseAddress, XTTCPS_CNT_CNTRL_OFFSET,
				  RegValue);

		/*
		 * Write to the Interrupt enable register. The status flags are
		 * not active if this is not done.
		 */
		XTtcPs_WriteReg(TmrCtrBaseAddress, XTTCPS_IER_OFFSET,
				  XTTCPS_IXR_INTERVAL_MASK);
	}

	LoopCount = 0;
	while (LoopCount < MAX_LOOP_COUNT) {

		/*
		 * Read the status register for debugging
		 */
		RegValue =
			XTtcPs_ReadReg(TmrCtrBaseAddress, XTTCPS_ISR_OFFSET);

		/*
		 * Write the status register to clear the flags
		 */
		XTtcPs_WriteReg(TmrCtrBaseAddress, XTTCPS_ISR_OFFSET,
				  RegValue);

		if (0 != (XTTCPS_IXR_INTERVAL_MASK & RegValue)) {
			LoopCount++;
			/*
			 * Count the number of output cycles so the program will
			 * eventually exit. Otherwise it would stay in this loop
			 * indefinitely.
			 */
		}
	}

	return XST_SUCCESS;
}
