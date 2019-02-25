/*******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xhdmiphy1.c
 *
 * Contains a minimal set of functions for the XHdmiphy1 driver that allow
 * access to all of the Video PHY core's functionality. See xhdmiphy1.h for a
 * detailed description of the driver.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 *            dd/mm/yy
 * ----- ---- -------- -----------------------------------------------
 * 1.0   gm   10/12/18 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xhdmiphy1.h"
#include "xhdmiphy1_i.h"

/************************** Constant Definitions *****************************/
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_WHITE   "\x1b[37m"
#define ANSI_COLOR_RESET   "\x1b[0m"


/**************************** Function Prototypes *****************************/

/**************************** Function Definitions ****************************/

/*****************************************************************************/
/**
* This function will reset the driver's logginc mechanism.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_LogReset(XHdmiphy1 *InstancePtr)
{
#ifdef XV_HDMIPHY1_LOG_ENABLE
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->Log.HeadIndex = 0;
	InstancePtr->Log.TailIndex = 0;
#endif
}

#ifdef XV_HDMIPHY1_LOG_ENABLE
/*****************************************************************************/
/**
* This function will insert an event in the driver's logginc mechanism.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	Evt is the event type to log.
* @param	Data is the associated data for the event.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_LogWrite(XHdmiphy1 *InstancePtr, XHdmiphy1_LogEvent Evt,
        u8 Data)
{
	u64 TimeUnit = 0;

	if (InstancePtr->LogWriteCallback) {
		TimeUnit = InstancePtr->LogWriteCallback(InstancePtr->LogWriteRef);
	}
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Evt <= (XHDMIPHY1_LOG_EVT_DUMMY));
	Xil_AssertVoid(Data < 0xFF);

	/* Write data and event into log buffer */
	InstancePtr->Log.DataBuffer[InstancePtr->Log.HeadIndex] =
			(Data << 8) | Evt;
	InstancePtr->Log.TimeRecord[InstancePtr->Log.HeadIndex] = TimeUnit;

	/* Update head pointer if reached to end of the buffer */
	if (InstancePtr->Log.HeadIndex ==
			(u8)((sizeof(InstancePtr->Log.DataBuffer) /
            sizeof(InstancePtr->Log.DataBuffer[0])) - 1)) {
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
			(u8)((sizeof(InstancePtr->Log.DataBuffer) /
            sizeof(InstancePtr->Log.DataBuffer[0])) - 1)) {
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
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
*
* @return	The log data.
*
* @note		None.
*
******************************************************************************/
u16 XHdmiphy1_LogRead(XHdmiphy1 *InstancePtr)
{
#ifdef XV_HDMIPHY1_LOG_ENABLE
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
			(u8)((sizeof(InstancePtr->Log.DataBuffer) /
            sizeof(InstancePtr->Log.DataBuffer[0])) - 1)) {
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
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_LogDisplay(XHdmiphy1 *InstancePtr)
{
#ifdef XV_HDMIPHY1_LOG_ENABLE
	u32 Log;
	u8 Evt;
	u8 Data;
	u64 TimeUnit;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	xil_printf("\r\n\r\n\r\nHDMIPHY log\r\n");
	xil_printf("------\r\n");

	/* Read time record */
	TimeUnit = InstancePtr->Log.TimeRecord[InstancePtr->Log.TailIndex];

	/* Read log data */
	Log = XHdmiphy1_LogRead(InstancePtr);

	while (Log != 0) {
		/* Event */
		Evt = Log & 0xff;

		/* Data */
		Data = (Log >> 8) & 0xFF;

		if (InstancePtr->LogWriteCallback) {
			/* Printing of TimeUnit */
			xil_printf("0x%08X%08X: HDMIPHY - ",
                (u32)((TimeUnit >> 32) & 0xFFFFFFFF),
                (u32)(TimeUnit & 0xFFFFFFFF));
		}

		switch (Evt) {
		case (XHDMIPHY1_LOG_EVT_NONE):
			xil_printf("GT log end\r\n-------\r\n");
			break;
		case (XHDMIPHY1_LOG_EVT_QPLL_EN):
			xil_printf("QPLL enable (%0d)\r\n", Data);
			break;
		case (XHDMIPHY1_LOG_EVT_QPLL_RST):
			xil_printf("QPLL reset (%0d)\r\n", Data);
			break;
		case (XHDMIPHY1_LOG_EVT_CPLL_EN):
			xil_printf("CPLL enable (%0d)\r\n", Data);
			break;
		case (XHDMIPHY1_LOG_EVT_CPLL_RST):
			xil_printf("CPLL reset (%0d)\r\n", Data);
			break;
		case (XHDMIPHY1_LOG_EVT_TXPLL_EN):
			xil_printf("TX MMCM enable (%0d)\r\n", Data);
			break;
		case (XHDMIPHY1_LOG_EVT_TXPLL_RST):
			xil_printf("TX MMCM reset (%0d)\r\n", Data);
			break;
		case (XHDMIPHY1_LOG_EVT_RXPLL_EN):
			xil_printf("RX MMCM enable (%0d)\r\n", Data);
			break;
		case (XHDMIPHY1_LOG_EVT_RXPLL_RST):
			xil_printf("RX MMCM reset (%0d)\r\n", Data);
			break;
		case (XHDMIPHY1_LOG_EVT_GTRX_RST):
			xil_printf("GT RX reset (%0d)\r\n", Data);
			break;
		case (XHDMIPHY1_LOG_EVT_GTTX_RST):
			xil_printf("GT TX reset (%0d)\r\n", Data);
			break;
		case (XHDMIPHY1_LOG_EVT_VID_TX_RST):
			xil_printf("Video TX reset (%0d)\r\n", Data);
			break;
		case (XHDMIPHY1_LOG_EVT_VID_RX_RST):
			xil_printf("Video RX reset (%0d)\r\n", Data);
			break;
		case (XHDMIPHY1_LOG_EVT_TX_ALIGN):
			if (Data == 1) {
				xil_printf("TX alignment done\r\n");
			}
			else {
				xil_printf("TX alignment start.\r\n.");
			}
			break;
		case (XHDMIPHY1_LOG_EVT_TX_ALIGN_TMOUT):
				xil_printf("TX alignment watchdog timed out.\r\n");
			break;
		case (XHDMIPHY1_LOG_EVT_TX_TMR):
			if (Data == 1) {
				xil_printf("TX timer event\r\n");
			}
			else {
				xil_printf("TX timer load\r\n");
			}
			break;
		case (XHDMIPHY1_LOG_EVT_RX_TMR):
			if (Data == 1) {
				xil_printf("RX timer event\r\n");
			}
			else {
				xil_printf("RX timer load\r\n");
			}
			break;
		case (XHDMIPHY1_LOG_EVT_CPLL_RECONFIG):
			if (Data == 1) {
				xil_printf("CPLL reconfig done\r\n");
			}
			else {
				xil_printf("CPLL reconfig start\r\n");
			}
			break;
		case (XHDMIPHY1_LOG_EVT_GT_RECONFIG):
			if (Data == 1) {
				xil_printf("GT reconfig done\r\n");
			}
			else {
				xil_printf("GT reconfig start\r\n");
			}
			break;
		case (XHDMIPHY1_LOG_EVT_GT_TX_RECONFIG):
			if (Data == 1) {
				xil_printf("GT TX reconfig done\r\n");
			}
			else {
				xil_printf("GT TX reconfig start\r\n");
			}
			break;
		case (XHDMIPHY1_LOG_EVT_GT_RX_RECONFIG):
			if (Data == 1) {
				xil_printf("GT RX reconfig done\r\n");
			}
			else {
				xil_printf("GT RX reconfig start\r\n");
			}
			break;
		case (XHDMIPHY1_LOG_EVT_QPLL_RECONFIG):
			if (Data == 1) {
				xil_printf("QPLL reconfig done\r\n");
			}
			else {
				xil_printf("QPLL reconfig start\r\n");
			}
			break;
		case (XHDMIPHY1_LOG_EVT_INIT):
			if (Data == 1) {
				xil_printf("GT init done\r\n");
			}
			else {
				xil_printf("GT init start\r\n");
			}
			break;
		case (XHDMIPHY1_LOG_EVT_TXPLL_RECONFIG):
			if (Data == 1) {
				xil_printf("TX MMCM reconfig done\r\n");
			}
			else {
				xil_printf("TX MMCM reconfig start\r\n");
			}
			break;
		case (XHDMIPHY1_LOG_EVT_RXPLL_RECONFIG):
			if (Data == 1) {
				xil_printf("RX MMCM reconfig done\r\n");
			}
			else {
				xil_printf("RX MMCM reconfig start\r\n");
			}
			break;
		case (XHDMIPHY1_LOG_EVT_QPLL_LOCK):
			if (Data == 1) {
				xil_printf("QPLL lock\r\n");
			}
			else {
				xil_printf("QPLL lost lock\r\n");
			}
			break;
		case (XHDMIPHY1_LOG_EVT_CPLL_LOCK):
			if (Data == 1) {
				xil_printf("CPLL lock\r\n");
			}
			else {
				xil_printf("CPLL lost lock\r\n");
			}
			break;
		case (XHDMIPHY1_LOG_EVT_LCPLL_LOCK):
			if (Data == 1) {
				xil_printf("LCPLL lock\r\n");
			}
			else {
				xil_printf("LCPLL lost lock\r\n");
			}
			break;
		case (XHDMIPHY1_LOG_EVT_RPLL_LOCK):
			if (Data == 1) {
				xil_printf("RPLL lock\r\n");
			}
			else {
				xil_printf("RPLL lost lock\r\n");
			}
			break;
		case (XHDMIPHY1_LOG_EVT_RXPLL_LOCK):
			if (Data == 1) {
				xil_printf("RX MMCM lock\r\n");
			}
			else {
				xil_printf("RX MMCM lost lock\r\n");
			}
			break;
		case (XHDMIPHY1_LOG_EVT_TXPLL_LOCK):
			if (Data == 1) {
				xil_printf("TX MMCM lock\r\n");
			}
			else {
				xil_printf("TX MMCM lost lock\r\n");
			}
			break;
		case (XHDMIPHY1_LOG_EVT_TX_RST_DONE):
			xil_printf("TX reset done\r\n");
			break;
		case (XHDMIPHY1_LOG_EVT_RX_RST_DONE):
			xil_printf("RX reset done\r\n");
			break;
		case (XHDMIPHY1_LOG_EVT_TX_FREQ):
			xil_printf("TX frequency event\r\n");
			break;
		case (XHDMIPHY1_LOG_EVT_RX_FREQ):
			xil_printf("RX frequency event\r\n");
			break;
		case (XHDMIPHY1_LOG_EVT_DRU_EN):
			if (Data == 1) {
				xil_printf("RX DRU enable\r\n");
			}
			else {
				xil_printf("RX DRU disable\r\n");
			}
			break;
		case (XHDMIPHY1_LOG_EVT_TXGPO_RE):
			if (Data == 1) {
				xil_printf("TX GPO Rising Edge Detected\r\n");
			}
			else {
				xil_printf("TX MSTRESET Toggled\r\n");
			}
			break;
		case (XHDMIPHY1_LOG_EVT_RXGPO_RE):
			if (Data == 1) {
				xil_printf("RX GPO Rising Edge Detected\r\n");
			}
			else {
				xil_printf("RX MSTRESET Toggled\r\n");
			}
			break;
		case (XHDMIPHY1_LOG_EVT_FRL_RECONFIG):
			xil_printf(ANSI_COLOR_GREEN);
			if (Data == XHDMIPHY1_DIR_RX) {
				xil_printf("RX FRL Reconfig\r\n");
			}
			else {
				xil_printf("TX FRL Reconfig\r\n");
			}
			xil_printf(ANSI_COLOR_RESET);
			break;
		case (XHDMIPHY1_LOG_EVT_TMDS_RECONFIG):
			xil_printf(ANSI_COLOR_CYAN);
			if (Data == XHDMIPHY1_DIR_RX) {
				xil_printf("RX TMDS Reconfig\r\n");
			}
			else {
				xil_printf("TX TMDS Reconfig\r\n");
			}
			xil_printf(ANSI_COLOR_RESET);
			break;
		case (XHDMIPHY1_LOG_EVT_1PPC_ERR):
				xil_printf(ANSI_COLOR_RED "Error! The HDMIPHY "
						"cannot support this video "
						"format at PPC = 1" ANSI_COLOR_RESET "\r\n");
			break;
		case (XHDMIPHY1_LOG_EVT_PPC_MSMTCH_ERR):
				xil_printf(ANSI_COLOR_RED "Error! HDMI TX SS PPC "
						"value, doesn't match with VHdmiphy PPC value"
						ANSI_COLOR_RESET "\r\n");
			break;
		case (XHDMIPHY1_LOG_EVT_VDCLK_HIGH_ERR):
				xil_printf(ANSI_COLOR_RED "Error! Video PHY "
						"cannot support resolutions with video clock "
						"> 148.5 MHz." ANSI_COLOR_RESET "\r\n");
			break;
		case (XHDMIPHY1_LOG_EVT_NO_DRU):
				xil_printf(ANSI_COLOR_YELLOW "Warning: No DRU instance. "
						"Low resolution video isn't supported in this "
						"design." ANSI_COLOR_RESET "\r\n");
			break;
		case (XHDMIPHY1_LOG_EVT_GT_QPLL_CFG_ERR):
				xil_printf(ANSI_COLOR_RED "Error! QPLL config not found!"
						ANSI_COLOR_RESET "\r\n");
			break;
		case (XHDMIPHY1_LOG_EVT_GT_CPLL_CFG_ERR):
				xil_printf(ANSI_COLOR_RED "Error! CPLL config not found!"
						ANSI_COLOR_RESET "\r\n");
			break;
		case (XHDMIPHY1_LOG_EVT_GT_LCPLL_CFG_ERR):
				xil_printf(ANSI_COLOR_RED "Error! LCPLL config not found!"
						ANSI_COLOR_RESET "\r\n");
			break;
		case (XHDMIPHY1_LOG_EVT_GT_RPLL_CFG_ERR):
				xil_printf(ANSI_COLOR_RED "Error! RPLL config not found!"
						ANSI_COLOR_RESET "\r\n");
			break;
		case (XHDMIPHY1_LOG_EVT_VD_NOT_SPRTD_ERR):
				xil_printf(ANSI_COLOR_YELLOW "Warning: This video format "
						"is not supported by this device. "
						"Change to another format"
						ANSI_COLOR_RESET "\r\n");
			break;
		case (XHDMIPHY1_LOG_EVT_MMCM_ERR):
				xil_printf(ANSI_COLOR_RED "Error! MMCM config not found!"
						ANSI_COLOR_RESET "\r\n");
			break;
		case (XHDMIPHY1_LOG_EVT_HDMI20_ERR):
				xil_printf(ANSI_COLOR_RED "Error!  The Video PHY doesn't "
						"support HDMI 2.0 line rates"
						ANSI_COLOR_RESET "\r\n");
			break;
		case (XHDMIPHY1_LOG_EVT_NO_QPLL_ERR):
				xil_printf(ANSI_COLOR_RED "Error!  There's no QPLL instance "
						"in the design"
						ANSI_COLOR_RESET "\r\n");
			break;
		case (XHDMIPHY1_LOG_EVT_DRU_CLK_ERR):
				xil_printf(ANSI_COLOR_RED "Error!  Wrong DRU REFCLK frequency "
						"detected"
						ANSI_COLOR_RESET "\r\n");
			break;
		case (XHDMIPHY1_LOG_EVT_USRCLK_ERR):
				xil_printf(ANSI_COLOR_RED "Error!  User Clock frequency is "
						"more than 300 MHz"
						ANSI_COLOR_RESET "\r\n");
			break;
		case (XHDMIPHY1_LOG_EVT_SPDGRDE_ERR):
					xil_printf(ANSI_COLOR_RED "Error!  %s: Line rates > 8.0 "
						"Gbps are not supported by -1/-1LV devices"
						ANSI_COLOR_RESET "\r\n",
						(Data == XHDMIPHY1_DIR_RX) ? "RX" : "TX");
			break;
		default:
			xil_printf("Unknown event\r\n");
			break;
		}

		/* Read log data */
		TimeUnit = InstancePtr->Log.TimeRecord[InstancePtr->Log.TailIndex];
		Log = XHdmiphy1_LogRead(InstancePtr);
	}
#else
    xil_printf("\r\nINFO:: HDMIPHY Log Feature is Disabled \r\n");
#endif
}
