/*******************************************************************************
# Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xv_hdmirxss1_log.c
 *
 *
 * @note    None.
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

#include "xv_hdmirxss1.h"

/**************************** Function Prototypes *****************************/

/**************************** Function Definitions ****************************/
#ifdef XV_HDMIRXSS1_LOG_ENABLE
/*****************************************************************************/
/**
* This function will reset the driver's logging mechanism.
*
* @param    InstancePtr is a pointer to the xv_hdmirxss1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiRxSs1_LogReset(XV_HdmiRxSs1 *InstancePtr)
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
* @param    InstancePtr is a pointer to the XV_HdmiRxSs1 core instance.
* @param    Evt is the event type to log.
* @param    Data is the associated data for the event.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiRxSs1_LogWrite(XV_HdmiRxSs1 *InstancePtr, XV_HdmiRxSs1_LogEvent Evt, u8 Data)
{
	u64 TimeUnit = 0;

	if (InstancePtr->LogWriteCallback) {
		TimeUnit = InstancePtr->LogWriteCallback(InstancePtr->LogWriteRef);
	}
    /* Verify arguments. */
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(Evt <= (XV_HDMIRXSS1_LOG_EVT_DUMMY));

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

/*	xil_printf(">r%d\r\n", Evt);*/
}

/*****************************************************************************/
/**
* This function will read the last event from the log.
*
* @param    InstancePtr is a pointer to the XV_HdmiRxSs1 core instance.
*
* @return   The log data.
*
* @note     None.
*
******************************************************************************/
u16 XV_HdmiRxSs1_LogRead(XV_HdmiRxSs1 *InstancePtr)
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
            (u8)((sizeof(InstancePtr->Log.DataBuffer) /
					sizeof(InstancePtr->Log.DataBuffer[0])) - 1)) {
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
* @param    InstancePtr is a pointer to the XV_HdmiRxSs1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiRxSs1_LogDisplay(XV_HdmiRxSs1 *InstancePtr)
{
#ifdef XV_HDMIRXSS1_LOG_ENABLE
	u32 Log;
	u8 Evt;
	u8 Data;
	u64 TimeUnit;

    /* Verify argument. */
    Xil_AssertVoid(InstancePtr != NULL);

    xil_printf("\r\n\r\n\nHDMI RX log\r\n");
    xil_printf("------\r\n");

	/* Read time record */
	TimeUnit = InstancePtr->Log.TimeRecord[InstancePtr->Log.TailIndex];

    /* Read log data */
    Log = XV_HdmiRxSs1_LogRead(InstancePtr);

    while (Log != 0) {
        /* Event */
        Evt = Log & 0xff;

		/* Data */
		Data = (Log >> 8) & 0xFF;

		if (InstancePtr->LogWriteCallback) {
			/* Printing of TimeUnit*/
			xil_printf("0x%08X%08X: RXSS - ", (u32)((TimeUnit >> 32) & 0xFFFFFFFF), (u32)(TimeUnit & 0xFFFFFFFF));
		}

        switch (Evt) {
        case (XV_HDMIRXSS1_LOG_EVT_NONE):
            xil_printf("HDMI RXSS log end\r\n-------");
            break;
        case (XV_HDMIRXSS1_LOG_EVT_HDMIRX1_INIT):
            xil_printf("Initializing HDMI RX core....");
            break;
        case (XV_HDMIRXSS1_LOG_EVT_VTC_INIT):
            xil_printf("Initializing VTC core....");
            break;
        case (XV_HDMIRXSS1_LOG_EVT_HDCPTIMER_INIT):
            xil_printf("Initializing AXI Timer core....");
            break;
        case (XV_HDMIRXSS1_LOG_EVT_HDCP14_INIT):
            xil_printf("Initializing HDCP 1.4 core....");
            break;
        case (XV_HDMIRXSS1_LOG_EVT_HDCP22_INIT):
            xil_printf("Initializing HDCP 2.2 core....");
            break;
        case (XV_HDMIRXSS1_LOG_EVT_START):
            xil_printf("Start HDMI RX Subsystem....");
            break;
        case (XV_HDMIRXSS1_LOG_EVT_STOP):
            xil_printf("Stop HDMI RX Subsystem....");
            break;
        case (XV_HDMIRXSS1_LOG_EVT_RESET):
            xil_printf("Reset HDMI RX Subsystem....");
            break;
        case (XV_HDMIRXSS1_LOG_EVT_CONNECT):
            xil_printf("Cable is connected....");
            break;
        case (XV_HDMIRXSS1_LOG_EVT_DISCONNECT):
            xil_printf("Cable is disconnected....");
            break;
        case (XV_HDMIRXSS1_LOG_EVT_LINKSTATUS):
            xil_printf("Link Status Error....");
            break;
        case (XV_HDMIRXSS1_LOG_EVT_STREAMUP):
            xil_printf("Stream is Up");
            break;
        case (XV_HDMIRXSS1_LOG_EVT_STREAMDOWN):
            xil_printf("Stream is Down");
            break;
        case (XV_HDMIRXSS1_LOG_EVT_STREAMINIT):
            xil_printf("Stream Start");
            break;
        case (XV_HDMIRXSS1_LOG_EVT_SETSTREAM):
            xil_printf("Stream Init");
            break;
        case (XV_HDMIRXSS1_LOG_EVT_SETSTREAM_ERR):
            xil_printf(ANSI_COLOR_RED "Error: Stream Reference Clock = 0"
			           ANSI_COLOR_RESET "");
            break;
        case (XV_HDMIRXSS1_LOG_EVT_REFCLOCKCHANGE):
            xil_printf("TMDS reference clock change");
            break;
         case (XV_HDMIRXSS1_LOG_EVT_HDCP14):
              if (Data) {
                xil_printf("HDCP 1.4 Enabled");
              } else {
                xil_printf("HDCP 1.4 Disabled");
              }
            break;
         case (XV_HDMIRXSS1_LOG_EVT_HDCP22):
              if (Data) {
                xil_printf("RX HDCP 2.2 Enabled");
              } else {
                xil_printf("RX HDCP 2.2 Disabled");
              }
            break;
        case (XV_HDMIRXSS1_LOG_EVT_DVIMODE):
            xil_printf("Mode changed to DVI");
            break;
        case (XV_HDMIRXSS1_LOG_EVT_HDMIMODE):
            xil_printf("Mode changed to HDMI");
            break;
        case (XV_HDMIRXSS1_LOG_EVT_SYNCLOSS):
            xil_printf("Sync Loss detected");
            break;
        case (XV_HDMIRXSS1_LOG_EVT_PIX_REPEAT_ERR):
		xil_printf(ANSI_COLOR_RED "Unsupported Pixel Repetition: %d"
				ANSI_COLOR_RESET, Data);
		break;
	case (XV_HDMIRXSS1_LOG_EVT_SYNCEST):
		xil_printf("Sync Loss recovered");
		break;
	case (XV_HDMIRXSS1_LOG_EVT_VICERROR):
		xil_printf(ANSI_COLOR_YELLOW "Vic and video timing mismatch"
				ANSI_COLOR_RESET);
		break;
	case (XV_HDMIRXSS1_LOG_EVT_LNKRDYERROR):
		xil_printf(ANSI_COLOR_YELLOW "Link Clk Error [DBG Code: %d]"
				ANSI_COLOR_RESET, Data);
		break;
	case (XV_HDMIRXSS1_LOG_EVT_VIDRDYERROR):
		xil_printf(ANSI_COLOR_YELLOW "Video Clk Error [DBG Code: %d]"
				ANSI_COLOR_RESET, Data);
		break;
	case (XV_HDMIRXSS1_LOG_EVT_SKEWLOCKERROR):
		xil_printf(ANSI_COLOR_YELLOW "Skew Lock Error [DBG Code: %d]"
				ANSI_COLOR_RESET, Data);
		break;
	case (XV_HDMIRXSS1_LOG_EVT_FRL_LTS1):
		xil_printf("FRL LTS:1");
		break;
	case (XV_HDMIRXSS1_LOG_EVT_FRL_LTS2):
		xil_printf("FRL LTS:2");
		break;
	case (XV_HDMIRXSS1_LOG_EVT_FRL_LTS3):
		xil_printf("FRL LTS:3 (FRL_Rate: %d FFE_Levels: %d)",
				(Data & 0xF), (Data >> 4));
		break;
	case (XV_HDMIRXSS1_LOG_EVT_FRL_LTS4):
		xil_printf("FRL LTS:3 (Rate Drop Request)");
		break;
	case (XV_HDMIRXSS1_LOG_EVT_FRL_LTSP):
		xil_printf("FRL LTS:3 (LTP Detected)");
		break;
	case (XV_HDMIRXSS1_LOG_EVT_FRL_LTSL):
		xil_printf("FRL LTS:L");
		break;
	case (XV_HDMIRXSS1_LOG_EVT_FRL_START):
		xil_printf("FRL LTS:P (FRL Start)");
		break;
        default:
            xil_printf("Unknown event");
            break;
        }

        xil_printf("\r\n");

        /* Read log data */
		TimeUnit = InstancePtr->Log.TimeRecord[InstancePtr->Log.TailIndex];
        Log = XV_HdmiRxSs1_LogRead(InstancePtr);
    }
#else
    xil_printf("\r\n INFO:: HDMIRXSS Log Feature is Disabled \r\n");
#endif
}
