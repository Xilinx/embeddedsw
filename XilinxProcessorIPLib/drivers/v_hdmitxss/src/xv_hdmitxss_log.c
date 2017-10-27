/*******************************************************************************
 *
 * Copyright (C) 2015 - 2017 Xilinx, Inc.  All rights reserved.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
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
 * @file xv_hdmitxss_log.c
 *
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   YH   17/08/16 Initial release.
 * 1.01  MMO  03/01/17 Add compiler option(XV_HDMITXSS_LOG_ENABLE) to enable Log
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xv_hdmitxss.h"

/**************************** Function Prototypes *****************************/

/**************************** Function Definitions ****************************/

/*****************************************************************************/
/**
* This function will reset the driver's logging mechanism.
*
* @param	InstancePtr is a pointer to the xv_hdmitxss core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
#ifdef XV_HDMITXSS_LOG_ENABLE
void XV_HdmiTxSs_LogReset(XV_HdmiTxSs *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->Log.HeadIndex = 0;
	InstancePtr->Log.TailIndex = 0;
}

/*****************************************************************************/
/**
* This function will insert an event in the driver's logginc mechanism.
*
* @param	InstancePtr is a pointer to the XV_HdmiTxSs core instance.
* @param	Evt is the event type to log.
* @param	Data is the associated data for the event.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_HdmiTxSs_LogWrite(XV_HdmiTxSs *InstancePtr, XV_HdmiTxSs_LogEvent Evt, u8 Data)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Evt <= (XV_HDMITXSS_LOG_EVT_DUMMY));
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

/*****************************************************************************/
/**
* This function will read the last event from the log.
*
* @param	InstancePtr is a pointer to the XV_HdmiTxSs core instance.
*
* @return	The log data.
*
* @note		None.
*
******************************************************************************/
u16 XV_HdmiTxSs_LogRead(XV_HdmiTxSs *InstancePtr)
{
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
}
#endif
/*****************************************************************************/
/**
* This function will print the entire log.
*
* @param	InstancePtr is a pointer to the XV_HdmiTxSs core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_HdmiTxSs_LogDisplay(XV_HdmiTxSs *InstancePtr)
{
#ifdef XV_HDMITXSS_LOG_ENABLE
	u16 Log;
	u8 Evt;
	u8 Data;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	xil_printf("\r\n\r\nHDMI TX log\r\n");
	xil_printf("------\r\n");

	/* Read log data */
	Log = XV_HdmiTxSs_LogRead(InstancePtr);

	while (Log != 0) {
		/* Event */
		Evt = Log & 0xff;

		/* Data */
		Data = (Log >> 8) & 0xFF;
		Data = Data;

		switch (Evt) {
		case (XV_HDMITXSS_LOG_EVT_NONE):
			xil_printf("HDMI TXSS log end\r\n-------\r\n");
			break;
	    case (XV_HDMITXSS_LOG_EVT_HDMITX_INIT):
		    xil_printf("Initializing HDMI TX core....\r\n");
			break;
	    case (XV_HDMITXSS_LOG_EVT_VTC_INIT):
		    xil_printf("Initializing VTC core....\r\n");
			break;
	    case (XV_HDMITXSS_LOG_EVT_HDCPTIMER_INIT):
		    xil_printf("Initializing AXI Timer core....\r\n");
			break;
	    case (XV_HDMITXSS_LOG_EVT_HDCP14_INIT):
		    xil_printf("Initializing HDCP 1.4 core....\r\n");
			break;
	    case (XV_HDMITXSS_LOG_EVT_HDCP22_INIT):
		    xil_printf("Initializing HDCP 2.2 core....\r\n");
			break;
	    case (XV_HDMITXSS_LOG_EVT_START):
		    xil_printf("Start HDMI TX Subsystem....\r\n");
			break;
	    case (XV_HDMITXSS_LOG_EVT_STOP):
		    xil_printf("Stop HDMI TX Subsystem....\r\n");
			break;
	    case (XV_HDMITXSS_LOG_EVT_RESET):
		    xil_printf("Reset HDMI TX Subsystem....\r\n");
			break;
	    case (XV_HDMITXSS_LOG_EVT_CONNECT):
		    xil_printf("TX cable is connected....\r\n");
			break;
	    case (XV_HDMITXSS_LOG_EVT_DISCONNECT):
		    xil_printf("TX cable is disconnected....\r\n");
			break;
	    case (XV_HDMITXSS_LOG_EVT_TOGGLE):
		    xil_printf("TX cable is toggled....\r\n");
			break;
	    case (XV_HDMITXSS_LOG_EVT_STREAMUP):
		    xil_printf("TX Stream is Up\r\n");
			break;
	    case (XV_HDMITXSS_LOG_EVT_STREAMDOWN):
		    xil_printf("TX Stream is Down\r\n");
			break;
	    case (XV_HDMITXSS_LOG_EVT_STREAMSTART):
		    xil_printf("TX Stream Start\r\n");
			break;
	    case (XV_HDMITXSS_LOG_EVT_SETAUDIOCHANNELS):
		    xil_printf("TX Set Audio Channels (%0d)\r\n", Data);
			break;
	    case (XV_HDMITXSS_LOG_EVT_AUDIOMUTE):
		    xil_printf("TX Audio Muted\r\n");
			break;
	    case (XV_HDMITXSS_LOG_EVT_AUDIOUNMUTE):
		    xil_printf("TX Audio Unmuted\r\n");
			break;
	    case (XV_HDMITXSS_LOG_EVT_SETSTREAM):
		    xil_printf("TX Set Stream, with TMDS (%0d)\r\n", Data);
			break;
	    case (XV_HDMITXSS_LOG_EVT_HDCP14_AUTHREQ):
		    xil_printf("TX HDCP 1.4 authentication request\r\n");
			break;
	    case (XV_HDMITXSS_LOG_EVT_HDCP22_AUTHREQ):
		    xil_printf("TX HDCP 2.2 authentication request\r\n");
			break;
		default:
			xil_printf("Unknown event\r\n");
			break;
		}

		/* Read log data */
		Log = XV_HdmiTxSs_LogRead(InstancePtr);
	}
#else
    xil_printf("\r\n INFO:: HDMITXSS Log Feature is Disabled \r\n");
#endif
}
