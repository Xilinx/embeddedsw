/*******************************************************************************
 *
 * Copyright (C) 2015 - 2016 Xilinx, Inc.  All rights reserved.
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
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xvphy.c
 *
 * Contains a minimal set of functions for the XVphy driver that allow access
 * to all of the Video PHY core's functionality. See xvphy.h for a detailed
 * description of the driver.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  10/19/15 Initial release.
 * 1.1   gm   02/01/16 Additional events for event log printout
 * 1.2   gm            Added log events for debugging
 * 1.4   gm   11/24/16 Made debug log optional (can be disabled via makefile)
 *                     Added XVPHY_LOG_EVT_TX_ALIGN_TMOUT log event
 * 1.6   gm   06/08/17 Added XVPHY_LOG_EVT_HDMI20_ERR log event
 *                     Added Red & Yellow printing for errors and warnings
 *                     Added XVPHY_LOG_EVT_NO_QPLL_ERR log event
 *                     Changed xil_printf new lines to \r\n
 *                     Added XVPHY_LOG_EVT_DRU_CLK_ERR log event
 * 1.7   gm   13/09/17 Added XVPHY_LOG_EVT_USRCLK_ERR event
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xvphy.h"
#include "xvphy_i.h"

/************************** Constant Definitions *****************************/
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"

/**************************** Function Prototypes *****************************/

/**************************** Function Definitions ****************************/

/*****************************************************************************/
/**
* This function will reset the driver's logginc mechanism.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XVphy_LogReset(XVphy *InstancePtr)
{
#ifdef XV_VPHY_LOG_ENABLE
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->Log.HeadIndex = 0;
	InstancePtr->Log.TailIndex = 0;
#endif
}

#ifdef XV_VPHY_LOG_ENABLE
/*****************************************************************************/
/**
* This function will insert an event in the driver's logginc mechanism.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	Evt is the event type to log.
* @param	Data is the associated data for the event.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XVphy_LogWrite(XVphy *InstancePtr, XVphy_LogEvent Evt, u8 Data)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Evt <= (XVPHY_LOG_EVT_DUMMY));
	Xil_AssertVoid(Data < 0xFF);

	/* Write data and event into log buffer */
	InstancePtr->Log.DataBuffer[InstancePtr->Log.HeadIndex] =
			(Data << 8) | Evt;

	/* Update head pointer if reached to end of the buffer */
	if (InstancePtr->Log.HeadIndex ==
			(u8)((sizeof(InstancePtr->Log.DataBuffer) / 2) - 1)) {
		/* Clear pointer */
		InstancePtr->Log.HeadIndex = 0;
	}
	else {
		/* Increment pointer */
		InstancePtr->Log.HeadIndex++;
	}

	/* Check tail pointer. When the two pointer are equal, then the buffer
	 * is full. In this case then increment the tail pointer as well to
	 * remove the oldest entry from the buffer. */
	if (InstancePtr->Log.TailIndex == InstancePtr->Log.HeadIndex) {
		if (InstancePtr->Log.TailIndex ==
			(u8)((sizeof(InstancePtr->Log.DataBuffer) / 2) - 1)) {
			InstancePtr->Log.TailIndex = 0;
		}
		else {
			InstancePtr->Log.TailIndex++;
		}
	}
}
#endif

/*****************************************************************************/
/**
* This function will read the last event from the log.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
*
* @return	The log data.
*
* @note		None.
*
******************************************************************************/
u16 XVphy_LogRead(XVphy *InstancePtr)
{
#ifdef XV_VPHY_LOG_ENABLE
	u16 Log;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check if there is any data in the log */
	if (InstancePtr->Log.TailIndex == InstancePtr->Log.HeadIndex) {
		Log = 0;
	}
	else {
		Log = InstancePtr->Log.DataBuffer[InstancePtr->Log.TailIndex];

		/* Increment tail pointer */
		if (InstancePtr->Log.TailIndex ==
			(u8)((sizeof(InstancePtr->Log.DataBuffer) / 2) - 1)) {
			InstancePtr->Log.TailIndex = 0;
		}
		else {
			InstancePtr->Log.TailIndex++;
		}
	}

	return Log;
#endif
}

/*****************************************************************************/
/**
* This function will print the entire log.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XVphy_LogDisplay(XVphy *InstancePtr)
{
#ifdef XV_VPHY_LOG_ENABLE
	u16 Log;
	u8 Evt;
	u8 Data;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	xil_printf("\r\n\r\n\r\nVPHY log\r\n");
	xil_printf("------\r\n");

	/* Read log data */
	Log = XVphy_LogRead(InstancePtr);

	while (Log != 0) {
		/* Event */
		Evt = Log & 0xff;

		/* Data */
		Data = (Log >> 8) & 0xFF;

		switch (Evt) {
		case (XVPHY_LOG_EVT_NONE):
			xil_printf("GT log end\r\n-------\r\n");
			break;
		case (XVPHY_LOG_EVT_QPLL_EN):
			xil_printf("QPLL enable (%0d)\r\n", Data);
			break;
		case (XVPHY_LOG_EVT_QPLL_RST):
			xil_printf("QPLL reset (%0d)\r\n", Data);
			break;
		case (XVPHY_LOG_EVT_CPLL_EN):
			xil_printf("CPLL enable (%0d)\r\n", Data);
			break;
		case (XVPHY_LOG_EVT_CPLL_RST):
			xil_printf("CPLL reset (%0d)\r\n", Data);
			break;
		case (XVPHY_LOG_EVT_TXPLL_EN):
			xil_printf("TX MMCM enable (%0d)\r\n", Data);
			break;
		case (XVPHY_LOG_EVT_TXPLL_RST):
			xil_printf("TX MMCM reset (%0d)\r\n", Data);
			break;
		case (XVPHY_LOG_EVT_RXPLL_EN):
			xil_printf("RX MMCM enable (%0d)\r\n", Data);
			break;
		case (XVPHY_LOG_EVT_RXPLL_RST):
			xil_printf("RX MMCM reset (%0d)\r\n", Data);
			break;
		case (XVPHY_LOG_EVT_GTRX_RST):
			xil_printf("GT RX reset (%0d)\r\n", Data);
			break;
		case (XVPHY_LOG_EVT_GTTX_RST):
			xil_printf("GT TX reset (%0d)\r\n", Data);
			break;
		case (XVPHY_LOG_EVT_VID_TX_RST):
			xil_printf("Video TX reset (%0d)\r\n", Data);
			break;
		case (XVPHY_LOG_EVT_VID_RX_RST):
			xil_printf("Video RX reset (%0d)\r\n", Data);
			break;
		case (XVPHY_LOG_EVT_TX_ALIGN):
			if (Data == 1) {
				xil_printf("TX alignment done\r\n");
			}
			else {
				xil_printf("TX alignment start.\r\n.");
			}
			break;
		case (XVPHY_LOG_EVT_TX_ALIGN_TMOUT):
				xil_printf("TX alignment watchdog timed out.\r\n");
			break;
		case (XVPHY_LOG_EVT_TX_TMR):
			if (Data == 1) {
				xil_printf("TX timer event\r\n");
			}
			else {
				xil_printf("TX timer load\r\n");
			}
			break;
		case (XVPHY_LOG_EVT_RX_TMR):
			if (Data == 1) {
				xil_printf("RX timer event\r\n");
			}
			else {
				xil_printf("RX timer load\r\n");
			}
			break;
		case (XVPHY_LOG_EVT_CPLL_RECONFIG):
			if (Data == 1) {
				xil_printf("CPLL reconfig done\r\n");
			}
			else {
				xil_printf("CPLL reconfig start\r\n");
			}
			break;
		case (XVPHY_LOG_EVT_GT_RECONFIG):
			if (Data == 1) {
				xil_printf("GT reconfig done\r\n");
			}
			else {
				xil_printf("GT reconfig start\r\n");
			}
			break;
		case (XVPHY_LOG_EVT_GT_TX_RECONFIG):
			if (Data == 1) {
				xil_printf("GT TX reconfig done\r\n");
			}
			else {
				xil_printf("GT TX reconfig start\r\n");
			}
			break;
		case (XVPHY_LOG_EVT_GT_RX_RECONFIG):
			if (Data == 1) {
				xil_printf("GT RX reconfig done\r\n");
			}
			else {
				xil_printf("GT RX reconfig start\r\n");
			}
			break;
		case (XVPHY_LOG_EVT_QPLL_RECONFIG):
			if (Data == 1) {
				xil_printf("QPLL reconfig done\r\n");
			}
			else {
				xil_printf("QPLL reconfig start\r\n");
			}
			break;
		case (XVPHY_LOG_EVT_PLL0_RECONFIG):
			if (Data == 1) {
				xil_printf("PLL0 reconfig done\r\n");
			}
			else {
				xil_printf("PLL0 reconfig start\r\n");
			}
			break;
		case (XVPHY_LOG_EVT_PLL1_RECONFIG):
			if (Data == 1) {
				xil_printf("PLL1 reconfig done\r\n");
			}
			else {
				xil_printf("PLL1 reconfig start\r\n");
			}
			break;
		case (XVPHY_LOG_EVT_INIT):
			if (Data == 1) {
				xil_printf("GT init done\r\n");
			}
			else {
				xil_printf("GT init start\r\n");
			}
			break;
		case (XVPHY_LOG_EVT_TXPLL_RECONFIG):
			if (Data == 1) {
				xil_printf("TX MMCM reconfig done\r\n");
			}
			else {
				xil_printf("TX MMCM reconfig start\r\n");
			}
			break;
		case (XVPHY_LOG_EVT_RXPLL_RECONFIG):
			if (Data == 1) {
				xil_printf("RX MMCM reconfig done\r\n");
			}
			else {
				xil_printf("RX MMCM reconfig start\r\n");
			}
			break;
		case (XVPHY_LOG_EVT_QPLL_LOCK):
			if (Data == 1) {
				xil_printf("QPLL lock\r\n");
			}
			else {
				xil_printf("QPLL lost lock\r\n");
			}
			break;
		case (XVPHY_LOG_EVT_PLL0_LOCK):
			if (Data == 1) {
				xil_printf("PLL0 lock\r\n");
			}
			else {
				xil_printf("PLL0 lost lock\r\n");
			}
			break;
		case (XVPHY_LOG_EVT_PLL1_LOCK):
			if (Data == 1) {
				xil_printf("PLL1 lock\r\n");
			}
			else {
				xil_printf("PLL1 lost lock\r\n");
			}
			break;
		case (XVPHY_LOG_EVT_CPLL_LOCK):
			if (Data == 1) {
				xil_printf("CPLL lock\r\n");
			}
			else {
				xil_printf("CPLL lost lock\r\n");
			}
			break;
		case (XVPHY_LOG_EVT_RXPLL_LOCK):
			if (Data == 1) {
				xil_printf("RX MMCM lock\r\n");
			}
			else {
				xil_printf("RX MMCM lost lock\r\n");
			}
			break;
		case (XVPHY_LOG_EVT_TXPLL_LOCK):
			if (Data == 1) {
				xil_printf("TX MMCM lock\r\n");
			}
			else {
				xil_printf("TX MMCM lost lock\r\n");
			}
			break;
		case (XVPHY_LOG_EVT_TX_RST_DONE):
			xil_printf("TX reset done\r\n");
			break;
		case (XVPHY_LOG_EVT_RX_RST_DONE):
			xil_printf("RX reset done\r\n");
			break;
		case (XVPHY_LOG_EVT_TX_FREQ):
			xil_printf("TX frequency event\r\n");
			break;
		case (XVPHY_LOG_EVT_RX_FREQ):
			xil_printf("RX frequency event\r\n");
			break;
		case (XVPHY_LOG_EVT_DRU_EN):
			if (Data == 1) {
				xil_printf("RX DRU enable\r\n");
			}
			else {
				xil_printf("RX DRU disable\r\n");
			}
			break;
		case (XVPHY_LOG_EVT_GT_PLL_LAYOUT):
				xil_printf(ANSI_COLOR_RED "Error! "
						"Couldn't find the correct GT "
						"parameters for this video resolution. "
						"Try another GT PLL layout."
						ANSI_COLOR_RESET "\r\n");
			break;
		case (XVPHY_LOG_EVT_GT_UNBONDED):
				xil_printf(ANSI_COLOR_RED "Error! "
						"Transmitter cannot be used on "
						"bonded mode when DRU is enabled. "
						"Switch to unbonded PLL layout"
						ANSI_COLOR_RESET "\r\n");
			break;
		case (XVPHY_LOG_EVT_1PPC_ERR):
				xil_printf(ANSI_COLOR_RED "Error! The VPHY "
						"cannot support this video "
						"format at PPC = 1" ANSI_COLOR_RESET "\r\n");
			break;
		case (XVPHY_LOG_EVT_PPC_MSMTCH_ERR):
				xil_printf(ANSI_COLOR_RED "Error! HDMI TX SS PPC "
						"value, doesn't match with VPhy PPC value"
						ANSI_COLOR_RESET "\r\n");
			break;
		case (XVPHY_LOG_EVT_VDCLK_HIGH_ERR):
				xil_printf(ANSI_COLOR_RED "Error! GTPE2 Video PHY "
						"cannot support resolutions with video clock "
						"> 148.5 MHz." ANSI_COLOR_RESET "\r\n");
			break;
		case (XVPHY_LOG_EVT_NO_DRU):
				xil_printf(ANSI_COLOR_YELLOW "Warning: No DRU instance. "
						"Low resolution video isn't supported in this "
						"design." ANSI_COLOR_RESET "\r\n");
			break;
		case (XVPHY_LOG_EVT_GT_QPLL_CFG_ERR):
				xil_printf(ANSI_COLOR_RED "Error! QPLL config not found!"
						ANSI_COLOR_RESET "\r\n");
			break;
		case (XVPHY_LOG_EVT_GT_CPLL_CFG_ERR):
				xil_printf(ANSI_COLOR_RED "Error! CPLL config not found!"
						ANSI_COLOR_RESET "\r\n");
			break;
		case (XVPHY_LOG_EVT_VD_NOT_SPRTD_ERR):
				xil_printf(ANSI_COLOR_YELLOW "Warning: This video format "
						"is not supported by this device. "
						"Change to another format"
						ANSI_COLOR_RESET "\r\n");
			break;
		case (XVPHY_LOG_EVT_MMCM_ERR):
				xil_printf(ANSI_COLOR_RED "Error! MMCM config not found!"
						ANSI_COLOR_RESET "\r\n");
			break;
		case (XVPHY_LOG_EVT_HDMI20_ERR):
				xil_printf(ANSI_COLOR_RED "Error!  The Video PHY doesn't "
						"support HDMI 2.0 line rates"
						ANSI_COLOR_RESET "\r\n");
			break;
		case (XVPHY_LOG_EVT_NO_QPLL_ERR):
				xil_printf(ANSI_COLOR_RED "Error!  There's no QPLL instance "
						"in the design"
						ANSI_COLOR_RESET "\r\n");
			break;
		case (XVPHY_LOG_EVT_DRU_CLK_ERR):
				xil_printf(ANSI_COLOR_RED "Error!  Wrong DRU REFCLK frequency "
						"detected"
						ANSI_COLOR_RESET "\r\n");
			break;
		case (XVPHY_LOG_EVT_USRCLK_ERR):
				xil_printf(ANSI_COLOR_RED "Error!  User Clock frequency is "
						"more than 297 MHz"
						ANSI_COLOR_RESET "\r\n");
			break;
		default:
			xil_printf("Unknown event\r\n");
			break;
		}

		/* Read log data */
		Log = XVphy_LogRead(InstancePtr);
	}
#else
    xil_printf("\r\nINFO:: VPHY Log Feature is Disabled \r\n");
#endif
}
