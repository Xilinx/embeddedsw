/*******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xv_hdmitxss1_log.c
 *
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.00  EB   22/05/18 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xv_hdmitxss1.h"

/**************************** Function Prototypes *****************************/

/**************************** Function Definitions ****************************/

/*****************************************************************************/
/**
* This function will reset the driver's logging mechanism.
*
* @param	InstancePtr is a pointer to the xv_hdmitxss1 core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
#ifdef XV_HDMITXSS1_LOG_ENABLE
void XV_HdmiTxSs1_LogReset(XV_HdmiTxSs1 *InstancePtr)
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
* @param	InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
* @param	Evt is the event type to log.
* @param	Data is the associated data for the event.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_HdmiTxSs1_LogWrite(XV_HdmiTxSs1 *InstancePtr, XV_HdmiTxSs1_LogEvent Evt, u8 Data)
{
	u64 TimeUnit = 0;

	if (InstancePtr->LogWriteCallback) {
		TimeUnit = InstancePtr->LogWriteCallback(InstancePtr->LogWriteRef);
	}
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Evt <= (XV_HDMITXSS1_LOG_EVT_DUMMY));

	/* Write data and event into log buffer */
	InstancePtr->Log.DataBuffer[InstancePtr->Log.HeadIndex] =
			(Data << 8) | Evt;
	InstancePtr->Log.TimeRecord[InstancePtr->Log.HeadIndex] = TimeUnit;

	/* Update head pointer if reached to end of the buffer */
	if (InstancePtr->Log.HeadIndex ==
			(u8)((sizeof(InstancePtr->Log.DataBuffer) / sizeof(InstancePtr->Log.DataBuffer[0])) - 1)) {
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
			(u8)((sizeof(InstancePtr->Log.DataBuffer) / sizeof(InstancePtr->Log.DataBuffer[0])) - 1)) {
			InstancePtr->Log.TailIndex = 0;
		}
		else {
			InstancePtr->Log.TailIndex++;
		}
	}

/*	xil_printf(">t%d\r\n", Evt);*/
}

/*****************************************************************************/
/**
* This function will read the last event from the log.
*
* @param	InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
*
* @return	The log data.
*
* @note		None.
*
******************************************************************************/
u16 XV_HdmiTxSs1_LogRead(XV_HdmiTxSs1 *InstancePtr)
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
			(u8)((sizeof(InstancePtr->Log.DataBuffer) / sizeof(InstancePtr->Log.DataBuffer[0])) - 1)) {
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
* @param	InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_HdmiTxSs1_LogDisplay(XV_HdmiTxSs1 *InstancePtr)
{
#ifdef XV_HDMITXSS1_LOG_ENABLE
	u32 Log;
	u8 Evt;
	u8 Data;
	u64 TimeUnit;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	xil_printf("\r\n\r\nHDMI TX log\r\n");
	xil_printf("------\r\n");

	/* Read time record */
	TimeUnit = InstancePtr->Log.TimeRecord[InstancePtr->Log.TailIndex];

	/* Read log data */
	Log = XV_HdmiTxSs1_LogRead(InstancePtr);

	while (Log != 0) {
		/* Event */
		Evt = Log & 0xff;

		/* Data */
		Data = (Log >> 8) & 0xFF;

		if (InstancePtr->LogWriteCallback) {
			/* Printing of TimeUnit*/
			xil_printf("0x%08X%08X: TXSS - ", (u32)((TimeUnit >> 32) & 0xFFFFFFFF), (u32)(TimeUnit & 0xFFFFFFFF));
		}

		switch (Evt) {
		case (XV_HDMITXSS1_LOG_EVT_NONE):
			xil_printf("Log end\r\n-------");
			break;
	    case (XV_HDMITXSS1_LOG_EVT_HDMITX1_INIT):
		    xil_printf("Initializing HDMI TX core....");
			break;
	    case (XV_HDMITXSS1_LOG_EVT_VTC_INIT):
		    xil_printf("Initializing VTC core....");
			break;
	    case (XV_HDMITXSS1_LOG_EVT_HDCPTIMER_INIT):
		    xil_printf("Initializing AXI Timer core....");
			break;
	    case (XV_HDMITXSS1_LOG_EVT_HDCP14_INIT):
		    xil_printf("Initializing HDCP 1.4 core....");
			break;
	    case (XV_HDMITXSS1_LOG_EVT_HDCP22_INIT):
		    xil_printf("Initializing HDCP 2.2 core....");
			break;
	    case (XV_HDMITXSS1_LOG_EVT_START):
		    xil_printf("Start HDMI TX Subsystem....");
			break;
	    case (XV_HDMITXSS1_LOG_EVT_STOP):
		    xil_printf("Stop HDMI TX Subsystem....");
			break;
	    case (XV_HDMITXSS1_LOG_EVT_RESET):
		    xil_printf("Reset HDMI TX Subsystem....");
			break;
	    case (XV_HDMITXSS1_LOG_EVT_CONNECT):
		    xil_printf("Cable is connected....");
			break;
	    case (XV_HDMITXSS1_LOG_EVT_DISCONNECT):
		    xil_printf("Cable is disconnected....");
			break;
	    case (XV_HDMITXSS1_LOG_EVT_TOGGLE):
		    xil_printf("Cable is toggled....");
			break;
	    case (XV_HDMITXSS1_LOG_EVT_STREAMUP):
		    xil_printf("Stream is Up");
			break;
	    case (XV_HDMITXSS1_LOG_EVT_STREAMDOWN):
		    xil_printf("Stream is Down");
			break;
	    case (XV_HDMITXSS1_LOG_EVT_STREAMSTART):
		    xil_printf("Stream Start");
			break;
	    case (XV_HDMITXSS1_LOG_EVT_SETAUDIOCHANNELS):
		    xil_printf("Set Audio Channels (%0d)", Data);
			break;
	    case (XV_HDMITXSS1_LOG_EVT_AUDIOMUTE):
		    xil_printf("Audio Muted");
			break;
	    case (XV_HDMITXSS1_LOG_EVT_AUDIOUNMUTE):
		    xil_printf("Audio Unmuted");
			break;
	    case (XV_HDMITXSS1_LOG_EVT_AUDIOINVALIDSAMPRATE):
		    xil_printf("Invalid Audio Sampling Rate");
			break;
	    case (XV_HDMITXSS1_LOG_EVT_SETSTREAM):
		    xil_printf("Set Stream");
			break;
	    case (XV_HDMITXSS1_LOG_EVT_HDCP14_AUTHREQ):
		    xil_printf("HDCP 1.4 authentication request");
			break;
	    case (XV_HDMITXSS1_LOG_EVT_HDCP22_AUTHREQ):
		    xil_printf("HDCP 2.2 authentication request");
			break;
	    case (XV_HDMITXSS1_LOG_EVT_PIX_REPEAT_ERR):
			xil_printf(ANSI_COLOR_RED "Unsupported Pixel Repetition: %d"
					ANSI_COLOR_RESET, Data);
			break;
	    case (XV_HDMITXSS1_LOG_EVT_VTC_RES_ERR):
			xil_printf(ANSI_COLOR_RED "Unsupported Video by VTC"
					ANSI_COLOR_RESET, Data);
			break;
		case (XV_HDMITXSS1_LOG_EVT_BRDG_LOCKED):
			xil_printf("VID Bridge Locked", Data);
			break;
		case (XV_HDMITXSS1_LOG_EVT_BRDG_UNLOCKED):
			xil_printf("VID Bridge Unlocked", Data);
			break;
	    case (XV_HDMITXSS1_LOG_EVT_FRL_START):
			xil_printf("FRL Start Training (MaxFrlRate: %d)",
					Data);
			break;
	    case (XV_HDMITXSS1_LOG_EVT_FRL_LT_PASS):
			xil_printf("FRL LT Pass");
			break;
	    case (XV_HDMITXSS1_LOG_EVT_FRL_CFG):
			xil_printf("FRL Config");
			break;
	    case (XV_HDMITXSS1_LOG_EVT_TMDS_START):
			xil_printf("TMDS Start");
			break;
		case (XV_HDMITXSS1_LOG_EVT_FRL_LTS1):
			xil_printf("FRL LTS:1");
			break;
		case (XV_HDMITXSS1_LOG_EVT_FRL_LTS2):
		xil_printf("FRL LTS:2 (FRL_Rate: %d FFE_Levels: %d)",
							(Data & 0xF), (Data >> 4));
			break;
		case (XV_HDMITXSS1_LOG_EVT_FRL_LTS3):
			xil_printf("FRL LTS:3 (");
			if (Data == 0xF0) {
				xil_printf("LT Pass");
			} else if (Data == 0xFF) {
				xil_printf("Rate Drop Requested");
			} else if (Data == 0xFA) {
				xil_printf("Timeout");
			} else {
				xil_printf("LTP: 0x%X", Data);
			}
			xil_printf(")");
			break;
		case (XV_HDMITXSS1_LOG_EVT_FRL_LTS4):
		xil_printf("FRL LTS:4 (FRL_Rate: %d FFE_Levels: %d)",
							(Data & 0xF), (Data >> 4));
			break;
		case (XV_HDMITXSS1_LOG_EVT_FRL_LTSP):
			xil_printf("FRL LTS:P (LT passed)");
			if (Data > 0) {
				xil_printf(" [DBG Code: %d]", Data);
			}
			break;
		case (XV_HDMITXSS1_LOG_EVT_FRL_LTSL):
			xil_printf("FRL LTS:L");
			break;
		default:
			xil_printf("Unknown event");
			break;
		}

		xil_printf("\r\n");

		/* Read log data */
		TimeUnit = InstancePtr->Log.TimeRecord[InstancePtr->Log.TailIndex];
		Log = XV_HdmiTxSs1_LogRead(InstancePtr);
	}
#else
    xil_printf("\r\n INFO:: HDMITXSS1 Log Feature is Disabled \r\n");
#endif
}
