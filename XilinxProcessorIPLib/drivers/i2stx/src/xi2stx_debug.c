/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
 *****************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xi2stx_debug.c
 * @addtogroup i2stx_v2_2
 * @{
 *
 * This file contains functions to debug i2s_transmitter driver, and log the
 * I2S Transmitter transactions in a buffer.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0   kar    11/16/17 Initial release.
 * 1.1   kar    04/02/18 Changed log API's to take i2stx instance as argument.
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files *********************************/
#include "xi2stx_debug.h"
#include "xi2stx.h"
#include "xi2stx_hw.h"
#include "xi2stx_chsts.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 *
 * This function writes I2S Transmitter logs into the buffer.
 *
 * @param InstancePtr is a pointer to the XI2s_Tx instance.
 * @param Event is the log event type.
 * @param Data is the log data.
 *
 * @return None.
 *
 * @note Log write can be done only if the log is enabled.
 *
 ******************************************************************************/
void XI2s_Tx_LogWrite(XI2s_Tx *InstancePtr, XI2s_Tx_LogEvt Event, u8 Data)
{
	u16 LogBufSize = 0;
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Event < XI2S_TX_LOG_EVT_INVALID);
	if (!InstancePtr->Log.IsEnabled)
		return;
	/* Write data and event into log buffer */
	InstancePtr->Log.Items[InstancePtr->Log.Head].Data = Data;
	InstancePtr->Log.Items[InstancePtr->Log.Head].Event = Event;

	/* Update head pointer */
	LogBufSize = sizeof(InstancePtr->Log.Items)/sizeof(XI2s_Tx_LogItem);
	InstancePtr->Log.Head++;
	if (InstancePtr->Log.Head == LogBufSize)
		InstancePtr->Log.Head = 0;

	/* Check tail pointer. When the two pointer are equal, then the
	 * buffer is full. In this case then increment the tail pointer
	 * as well to remove the oldest entry from the buffer.
	 */
	if (InstancePtr->Log.Tail == InstancePtr->Log.Head) {
		InstancePtr->Log.Tail++;
		if (InstancePtr->Log.Tail == (LogBufSize))
			InstancePtr->Log.Tail = 0;
	}
}

 /*****************************************************************************/
/**
 *
 * This function returns the next item in the logging buffer.
 *
 * @param InstancePtr is a pointer to the XI2s_Tx instance.
 *
 * @return When the buffer is filled, the next log item is returned.
 *         When the buffer is empty, NULL is returned.
 *
 * @note None.
 *
 *****************************************************************************/
XI2s_Tx_LogItem* XI2s_Tx_LogRead(XI2s_Tx *InstancePtr)
{
	XI2s_Tx_LogItem* LogPtr = NULL;
	u16 LogBufSize = 0;
	/* Verify whether log is enabled */
	if ((InstancePtr)->Log.IsEnabled == FALSE)
		return NULL;
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check if there is any data in the log buffer */
	if (InstancePtr->Log.Tail == InstancePtr->Log.Head)
		return NULL;

	LogPtr = &InstancePtr->Log.Items[InstancePtr->Log.Tail];

	/* Increment tail pointer */
	LogBufSize = sizeof(InstancePtr->Log.Items)/sizeof(XI2s_Tx_LogItem);
	if (InstancePtr->Log.Tail == (LogBufSize - 1))
		InstancePtr->Log.Tail = 0;
	else
		InstancePtr->Log.Tail++;

	return LogPtr;
}

/*****************************************************************************/
/**
 *
 * This function clears the contents of the logging buffer.
 *
 * @param InstancePtr is a pointer to the XI2s_Tx instance.
 *
 * @return None.
 *
 * @note None.
 *
 *****************************************************************************/
void XI2s_Tx_LogReset(XI2s_Tx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	if (InstancePtr->Log.IsEnabled == TRUE) {
		InstancePtr->Log.Head = 0;
		InstancePtr->Log.Tail = 0;
	}
}
/*****************************************************************************/
/**
 *
 * This function prints the contents of the logging buffer.
 *
 * @param InstancePtr is a pointer to the XI2s_Tx instance.
 *
 * @return None.
 *
 * @note None.
 *
 *****************************************************************************/
void XI2s_Tx_LogDisplay(XI2s_Tx *InstancePtr)
{
	XI2s_Tx_LogItem* LogPtr = NULL;

	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((InstancePtr)->Log.IsEnabled == TRUE);

	XI2s_Tx_LogEvt prevEvt = XI2S_TX_LOG_EVT_INVALID;
	int Count = 0;

	xil_printf("--------I2S Transmitter Log Start--------\n\r");
	while (1) {
		LogPtr = XI2s_Tx_LogRead(InstancePtr);
		/* if buffer is empty, NULL is returned */
		if (LogPtr == NULL) {
			if (Count != 0)
				xil_printf("..Repeats %d times..\n\r", Count);

			break;
		}

		if (prevEvt == LogPtr->Event) {
			Count++;
			continue;
		} else {
			if (Count != 0)
				xil_printf("..Repeats %d times..\n\r", Count);

			Count = 0;
		}

		xil_printf("I2S Transmitter %x: ",
				InstancePtr->Config.DeviceId);

		switch (LogPtr->Event) {
		case XI2S_TX_AES_BLKCMPLT_EVT:
			xil_printf("AES Block Complete Detected.\n\r");
			break;

		case XI2S_TX_AES_BLKSYNCERR_EVT:
			xil_printf("AES Block Sync Error Detected.\n\r");
			break;

		case XI2S_TX_AES_CHSTSUPD_EVT:
			xil_printf("AES Channel Status Updated.\n\r");
			break;

		case XI2S_TX_AUD_UNDRFLW_EVT:
			xil_printf("Audio Underflow Detected.\n\r");
			break;

		default:
			xil_printf("Unknown Log Entry.\n\r");
			break;
		}

		prevEvt = LogPtr->Event;
	}
	xil_printf("--------I2S Transmitter Log End--------\n\r");
}
/** @} */
