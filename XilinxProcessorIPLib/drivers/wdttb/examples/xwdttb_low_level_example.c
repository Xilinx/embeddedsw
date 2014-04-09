/* $Id: xwdttb_low_level_example.c,v 1.1.2.1 2009/12/04 05:52:36 svemula Exp $ */
/******************************************************************************
*
* (c) Copyright 2002-2009 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
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
*		      The example is udpated to use XWdtTb_ReadReg and
*		      XWdtTb_WriteReg macros to acheive the functionality of the
*		      macros that were removed.
* 		      Updated this example to check for Watchdog timer reset
*		      condition instead of timer expiry state to avoid a race
* 		      condition
*
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
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
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
		return XST_FAILURE;
	}

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
* @return	XST_SUCCESS if WRS bit is not set in next two subsequent timer
*		expiry state, otherwise XST_FAILURE.
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

