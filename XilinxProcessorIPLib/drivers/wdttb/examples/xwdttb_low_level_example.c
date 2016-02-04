/******************************************************************************
*
* Copyright (C) 2002 - 2016 Xilinx, Inc. All rights reserved.
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xwdttb_low_level_example.c
*
* This file contains a design example using the low-level driver macros of
* the Watchdog Timer Timebase driver. These macros are found in xwdttb_l.h.
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00b rpm  04/26/02 First release
* 1.00b sv   04/26/05 Minor changes to comply to Doxygen and coding guidelines
* 2.00a ktn  22/10/09 Updated the example to use the HAL APIs/macros.
*		      The following macros defined in xwdttb_l.h file have been
*		      removed - XWdtTb_mEnableWdt, XWdtTb_mDisbleWdt,
*		      XWdtTb_mRestartWdt, XWdtTb_mGetTimebaseReg and
*		      XWdtTb_mHasReset.
*		      The example is updated to use XWdtTb_ReadReg and
*		      XWdtTb_WriteReg macros to achieve the functionality of the
*		      macros that were removed.
* 		      Updated this example to check for Watchdog timer reset
*		      condition instead of timer expiry state to avoid a race
* 		      condition
* 4.0   sha  02/04/16 Added debug messages.
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xwdttb_l.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define WDTTB_BASEADDR  XPAR_WDTTB_0_BASEADDR


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int XWdtTb_LowLevelExample(u32 WdtTbBaseAddress);

/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
* Main function to call the Wdttb low level example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if unsuccessful.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the low level example, specify the Base Address that is
	 * generated in xparameters.h
	 */
	Status = XWdtTb_LowLevelExample(WDTTB_BASEADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("WDTTB low level example failed.\n\r");
		return XST_FAILURE;
	}

	xil_printf("WDTTB low level example ran successfully.\n\r");
	return XST_SUCCESS;
}


/*****************************************************************************/
/**
* This function assumes that the reset output of the watchdog timer
* timebase device is not connected to the reset of the processor. The function
* allows the watchdog timer to timeout such that a reset will occur if it is
* connected. This also assumes the interrupt output is not connected to an
* interrupt input.
*
* This function checks for Watchdog timer reset condition in two timer expiry
* state.
* This function may require some time (seconds or even minutes) to execute
* because it waits for the watchdog timer to expire.
*
* @param	WdtTbBaseAddress is the base address of the device.
*
* @return
*		- XST_SUCCESS if WRS bit is not set in next two subsequent
*		timer expiry state.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
****************************************************************************/
int XWdtTb_LowLevelExample(u32 WdtTbBaseAddress)
{
	int Count = 0;

	/*
	 * Set the registers to enable the watchdog timer, both enable bits
	 * in TCSR0 and TCSR1 need to be set to enable it.
	 * Clear the bit that indicates the reason for the last
	 * system reset, WRS and the WDS bit, if set, by writing
	 * 1's to TCSR0
	 */
	XWdtTb_WriteReg(WdtTbBaseAddress, XWT_TWCSR0_OFFSET,
			(XWT_CSR0_WRS_MASK | XWT_CSR0_WDS_MASK |
			XWT_CSR0_EWDT1_MASK));

	XWdtTb_WriteReg(WdtTbBaseAddress, XWT_TWCSR1_OFFSET,
			XWT_CSRX_EWDT2_MASK);

	/*
	 * Verify Whether the WatchDog Reset Status has been set in the next two
	 * expiry state.
	 */
	while (1) {

		/*
		 * If the watchdog timer expired, then restart it.
		 */
		if (XWdtTb_ReadReg(WdtTbBaseAddress, XWT_TWCSR0_OFFSET) &
				XWT_CSR0_WDS_MASK) {

			/*
			 * Restart the watchdog timer as a normal application
			 * would
			 */
			XWdtTb_WriteReg(WdtTbBaseAddress, XWT_TWCSR0_OFFSET,
				(XWT_CSR0_WRS_MASK | XWT_CSR0_WDS_MASK |
				XWT_CSR0_EWDT1_MASK));
			Count++;
		}

		/*
		 * Check whether the WatchDog Reset Status has been set.
		 * If this is set means then the test has failed
		 */
		if (XWdtTb_ReadReg(WdtTbBaseAddress, XWT_TWCSR0_OFFSET) &
				XWT_CSR0_WRS_MASK) {
			return XST_FAILURE;
		}

		/*
		 * Check whether the WatchDog timer expires two times.
		 * If the timer expires two times then the test is passed.
		 */
		if(Count == 2) {
			break;
		}
	}

	return XST_SUCCESS;

}

