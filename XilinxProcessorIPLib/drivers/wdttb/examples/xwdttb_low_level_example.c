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
* MODIFICATION HISTORY:
*
* <pre>
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00b rpm  04/26/02 First release
* 1.00b sv   04/26/05 Minor changes to comply to Doxygen and coding guidelines
* 2.00a ktn  22/10/09 Updated the example to use the HAL APIs/macros.
*                     The following macros defined in xwdttb_l.h file have been
*                     removed - XWdtTb_mEnableWdt, XWdtTb_mDisbleWdt,
*                     XWdtTb_mRestartWdt, XWdtTb_mGetTimebaseReg and
*                     XWdtTb_mHasReset.
*                     The example is updated to use XWdtTb_ReadReg and
*                     XWdtTb_WriteReg macros to achieve the functionality of
*                     the macros that were removed.
*                     Updated this example to check for Watchdog timer reset
*                     condition instead of timer expiry state to avoid a race
*                     condition
* 3.00  sha  12/29/15 Updated example with Window WDT feature.
*                     Added debug messages.
* </pre>
*
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
#define WDTTB_BASEADDR		XPAR_WDTTB_0_BASEADDR

/*
 * Defines legacy or window WDT feature.
 */
#define WIN_WDT_EN		XPAR_WDTTB_0_ENABLE_WINDOW_WDT

#if (WIN_WDT_EN)
#define WIN_WDT_SW_COUNT	0xF00000	/**< Number of clock cycles for
						  *  second window */
#define WIN_WDT_SBC_COUNT	16		/**< Selected byte count */
#define WIN_WDT_BSS_COUNT	2		/**< Byte segment selected */
#endif

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

#if (WIN_WDT_EN)
int XWdtTb_WWdtLowLevelExample(u32 WdtTbBaseAddress);
#else
int XWdtTb_LowLevelExample(u32 WdtTbBaseAddress);
#endif

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
#if (WIN_WDT_EN)
	Status = XWdtTb_WWdtLowLevelExample(WDTTB_BASEADDR);
#else
	Status = XWdtTb_LowLevelExample(WDTTB_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("WDTTB low level example failed.\n\r");
		return XST_FAILURE;
	}

	xil_printf("WDTTB low level example ran successfully.\n\r");
	return XST_SUCCESS;
}

#if (WIN_WDT_EN)
/*****************************************************************************/
/**
* This function assumes that the reset output of the watchdog timer
* timebase device is not connected to the reset of the processor. The function
* allows the watchdog timer to restart when it is reached to interrupt
* programmed point.
*
* This function checks for Watchdog timer to restart twice.
*
* @param	WdtTbBaseAddress is the base address of the device.
*
* @return
*		- XST_SUCCESS if window watchdog timer restarted twice.
		- XST_FAILURE.if window watchdog timer failed to restart twice.
*
* @note		None.
*
****************************************************************************/
int XWdtTb_WWdtLowLevelExample(u32 WdtTbBaseAddress)
{
	u32 RegValue;
	u32 Count;
	u32 Counter = 0;

	/* Set register space to writable */
	XWdtTb_WriteReg(WdtTbBaseAddress, XWT_MWR_OFFSET, 1);

	/* Write first and second window count value */
	XWdtTb_WriteReg(WdtTbBaseAddress, XWT_FWR_OFFSET, 0);
	XWdtTb_WriteReg(WdtTbBaseAddress, XWT_SWR_OFFSET, WIN_WDT_SW_COUNT);

	/* Set selection byte count */
	RegValue = XWdtTb_ReadReg(WdtTbBaseAddress, XWT_FCR_OFFSET);
	Count = ((u32)WIN_WDT_SBC_COUNT << XWT_FCR_SBC_SHIFT) &
		(XWT_FCR_SBC_MASK);
	RegValue |= Count;
	XWdtTb_WriteReg(WdtTbBaseAddress, XWT_FCR_OFFSET, RegValue);

	/* Set byte segment selection */
	RegValue = XWdtTb_ReadReg(WdtTbBaseAddress, XWT_FCR_OFFSET);
	Count = (WIN_WDT_BSS_COUNT << XWT_FCR_BSS_SHIFT) & (XWT_FCR_BSS_MASK);
	RegValue |= Count;
	XWdtTb_WriteReg(WdtTbBaseAddress, XWT_FCR_OFFSET, RegValue);

	/* Enable Window WDT */
	RegValue = XWdtTb_ReadReg(WdtTbBaseAddress, XWT_ESR_OFFSET) |
		(XWT_ESR_WEN_MASK);
	XWdtTb_WriteReg(WdtTbBaseAddress, XWT_ESR_OFFSET, RegValue);

	/* Set register space to writable */
	XWdtTb_WriteReg(WdtTbBaseAddress, XWT_MWR_OFFSET, 1);

	while(1) {
		RegValue = (XWdtTb_ReadReg(WdtTbBaseAddress, XWT_ESR_OFFSET) &
			(XWT_ESR_WINT_MASK)) >> (XWT_ESR_WINT_SHIFT);

		/* Check for interrupt programmed point */
		if (RegValue) {
			/* Clear interrupt */
			RegValue = XWdtTb_ReadReg(WdtTbBaseAddress,
				XWT_ESR_OFFSET) | (XWT_ESR_WINT_MASK);
			Count = (RegValue & XWT_ESR_WSW_MASK) >>
				XWT_ESR_WSW_SHIFT;
			if (Count) {
				/* Set WSW bit to zero. It is RW1C bit */
				RegValue &= (~XWT_ESR_WSW_MASK);
			}
			XWdtTb_WriteReg(WdtTbBaseAddress, XWT_ESR_OFFSET,
				RegValue);

			/* Restart */
			RegValue = XWdtTb_ReadReg(WdtTbBaseAddress,
				XWT_ESR_OFFSET) | (XWT_ESR_WSW_MASK);
			XWdtTb_WriteReg(WdtTbBaseAddress, XWT_ESR_OFFSET,
				RegValue);

			Counter++;
			xil_printf("\n\rRestart kick %d\n\r", Counter);
		}

		/* Get event */
		RegValue = (XWdtTb_ReadReg(WdtTbBaseAddress, XWT_ESR_OFFSET) &
			(XWT_ESR_LBE_MASK)) >> (XWT_ESR_LBE_SHIFT);

		/* Check for last event */
		if (RegValue != 0) {
			/* Stop Window WDT */
			Count = (XWdtTb_ReadReg(WdtTbBaseAddress,
				XWT_ESR_OFFSET) & (XWT_ESR_WSW_MASK)) >>
					(XWT_ESR_WSW_SHIFT);
			if (Count) {
				RegValue = XWdtTb_ReadReg(WdtTbBaseAddress,
					XWT_ESR_OFFSET) & (~XWT_ESR_WEN_MASK);
				RegValue &= (~XWT_ESR_WSW_MASK);
				XWdtTb_WriteReg(WdtTbBaseAddress, XWT_ESR_OFFSET,
					RegValue);
			}
			return XST_FAILURE;
		}

		if(Counter == 2) {
			/* Stop Window WDT */
			Count = (XWdtTb_ReadReg(WdtTbBaseAddress,
				XWT_ESR_OFFSET) & (XWT_ESR_WSW_MASK)) >>
					(XWT_ESR_WSW_SHIFT);
			if (Count) {
				RegValue = XWdtTb_ReadReg(WdtTbBaseAddress,
					XWT_ESR_OFFSET) & (~XWT_ESR_WEN_MASK);
				RegValue &= (~XWT_ESR_WSW_MASK);
				XWdtTb_WriteReg(WdtTbBaseAddress, XWT_ESR_OFFSET,
					RegValue);
			}
			break;
		}
	}

	return XST_SUCCESS;
}
#else

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
	 * Verify Whether the WatchDog Reset Status has been set in the next
	 * two expiry state.
	 */
	while (1) {

		/* If the watchdog timer expired, then restart it */
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
#endif
