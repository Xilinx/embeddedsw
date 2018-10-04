/******************************************************************************
 *
 * Copyright (C) 2017 - 2018 Xilinx, Inc. All rights reserved.
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
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
 *****************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xi2stx.h
 * @addtogroup i2stx_v1_1
 * @{
 *
 * The Xilinx I2s Transmitter driver. This driver supports the Xilinx I2s
 * Transmitter soft IP core in transmit/source (TX) operation.
 *
 * The Xilinx I2s Transmitter core supports the following features:
 *	- 16 and 24 bit data widths.
 *	- 1,2,3 and 4 channels.
 *	- I2S timer control.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0   kar    11/16/17 Initial release.
 * 1.1   kar    04/02/18 Added debug log function prototypes.
 * 2.0   kar    09/28/18 Added new API to enable justification.
 *                       Added new API to select left/right justification.
 * </pre>
 *
 *****************************************************************************/
#ifndef XI2STX_H
#define XI2STX_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xi2stx_hw.h"
#include "xi2stx_debug.h"
#include "xi2stx_chsts.h"
#include "xil_assert.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/** @name XI2S_Tx_Handlertype
 * @{
 */
/**
 * these constants specify different types of handlers and is used to
 * differentiate interrupt requests from the I2s Transmitter peripheral.
 */
typedef enum {
	XI2S_TX_HANDLER_AES_BLKCMPLT = 0, /**< AES Block Complete Handler */
	XI2S_TX_HANDLER_AES_BLKSYNCERR,   /**< AES Block Sync Error Handler */
	XI2S_TX_HANDLER_AES_CHSTSUPD, /**< AES Channel Status Updated Handler */
	XI2S_TX_HANDLER_AUD_UNDRFLW, /**< Audio Underflow Detected Handler */
		XI2S_TX_NUM_HANDLERS /**< Number of handler types */
} XI2s_Tx_HandlerType;
/*@}*/

/**
 * These constants specify different channel ID's
 */
typedef enum {
	XI2S_TX_CHID0 = 0,  //!< Channel 0
	XI2S_TX_CHID1,      //!< Channel 1
	XI2S_TX_CHID2, //!< Channel 2
	XI2S_TX_CHID3, //!< Channel 3
	XI2S_TX_NUM_CHANNELS //!<Number of Channel ID's
} XI2s_Tx_ChannelId;

/*@}*/

/**
 * @brief This typedef specifies the justification of the the XI2s Transmitter.
 */
typedef enum {
		XI2S_TX_JUSTIFY_LEFT = 0, //!< Left Justification is enabled.
		XI2S_TX_JUSTIFY_RIGHT     //!< Right Justification is enabled.
} XI2s_Tx_Justification;

/*@}*/

/**
 * Callback function data type for handling interrupt requests
 * from the I2s Transmitter peripheral. The application using this driver is
 * expected to define a handler of this type to support interrupt driven mode.
 * The handler is called in an interrupt context such that minimal processing
 * should be performed.
 *
 * @param CallBackRef is a callback reference passed in by the upper
 *        layer when setting the callback functions, and passed back
 *        to the upper layer when the callback is invoked.
 *
 * @return None
 *
 * @note None
 */
typedef void (*XI2s_Tx_Callback)(void *CallbackRef);

/**
 * This typedef contains configuration information for the I2s Transmitter.
 */
typedef struct {
	u16 DeviceId; /**< DeviceId is unique ID of the I2s Tx */
	UINTPTR BaseAddress; /**< BaseAddress is physical address of registers*/
	u8  DWidth;	    /**< Data Width (16/24bit) of I2s Tx core */
	u8  IsMaster;	    /**< IsMaster(TRUE/FALSE) for I2s Tx core */
	u8  MaxNumChannels; /**< Max channels supported by I2s Tx core */
} XI2stx_Config;
/**
 * This typedef implements the I2s Transmitter driver instance data.
 * An instance must be allocated for each I2s Transmitter core in use.
 */
typedef struct {

	u32 IsReady; /**< Core and driver instance are initialized */
	u32 IsStarted; /**< Core and driver instance has started */
	XI2stx_Config Config; /**< Hardware Configuration */

	XI2s_Tx_Log Log; /**< Logging for I2s Transmitter */

	/* Call backs */
	XI2s_Tx_Callback AesBlkCmpltHandler;  /**< AES Block Complete Handler */

	XI2s_Tx_Callback AesBlkSyncErrHandler; /**< AES Block Sync Err Handler*/
	XI2s_Tx_Callback AesChStsUpdHandler; /* AES Channel Status Updated
					      * Handler
					      */

	XI2s_Tx_Callback AudUndrflwHandler; /**< Audio Underflow Handler  */

	void *AesChStsUpdRef;  /* Callback reference for AES Channel Status
				* Updated Handler
				*/

	void *AesBlkCmpltRef; /* Callback reference for AES Block
			       * Complete Handler
			       */

	void *AesBlkSyncErrRef; /* Callback reference for AES Block
				 * Sync Error Handler
				 */
	void *AudUndrflwRef; /* Callback reference for Audio
			      * Underflow Handler
			      */

} XI2s_Tx;

/** @name XI2s_Tx_ChMuxInput
 * @{
 */
/**
 * @brief This typedef specifies the input sources of the the I2s Transmitter.
 */
typedef enum {
	XI2S_TX_CHMUX_DISABLED = 0, /**< Channel disabled */
	XI2S_TX_CHMUX_AXIS_01,      /**< AXI-Stream Audio Channel 0 and 1 */
	XI2S_TX_CHMUX_AXIS_23,      /**< AXI-Stream Audio Channel 2 and 3 */
	XI2S_TX_CHMUX_AXIS_45,      /**< AXI-Stream Audio Channel 4 and 5 */
	XI2S_TX_CHMUX_AXIS_67,      /**< AXI-Stream Audio Channel 6 and 7 */
	XI2S_TX_CHMUX_WAVEGEN,      /**< Wave Generator */
} XI2s_Tx_ChMuxInput;
/*@}*/

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
 *
 * This inline function reads the I2s Transmitter version
 *
 * @param  InstancePtr is a pointer to the XI2s_Tx core instance.
 *
 * @return I2sTx_Version
 *
 *****************************************************************************/
static inline u32 XI2s_Tx_GetVersion(XI2s_Tx *InstancePtr)
{
	u32 I2sTx_Version;

	I2sTx_Version = XI2s_Tx_ReadReg((InstancePtr)->Config.BaseAddress,
			(XI2S_TX_CORE_VER_OFFSET));
	return I2sTx_Version;
}
/*****************************************************************************/
/**
 *
 * This inline function clears the specified interrupt of the XI2s Transmitter.
 *
 * @param InstancePtr is a pointer to the XI2s_Tx core instance.
 * @param Mask is a bit mask of the interrupts to be cleared.
 *
 * @return None.
 * @see XI2stx_hw.h file for the available interrupt masks.
 *
 ******************************************************************************/
static inline void XI2s_Tx_IntrClear(XI2s_Tx *InstancePtr, u32 Mask)

{
	Xil_AssertVoid(InstancePtr != NULL);
	XI2s_Tx_WriteReg((InstancePtr)->Config.BaseAddress,
			(XI2S_TX_IRQSTS_OFFSET), Mask);

}
/*****************************************************************************/
/**
 *
 * This inline function enables the I2s Transmitter logging.
 *
 * @param InstancePtr is a pointer to the XI2s_Tx core instance.
 *
 * @return None.
 *
 *****************************************************************************/
static inline void XI2s_Tx_LogEnable(XI2s_Tx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	(InstancePtr)->Log.IsEnabled = TRUE;
}
/*****************************************************************************/
/**
 *
 * This inline function disables the I2s Transmitter logging.
 *
 * @param InstancePtr is a pointer to the XI2s_Tx core instance.
 *
 * @return None.
 *
 *
 *****************************************************************************/
static inline void XI2s_Tx_LogDisable(XI2s_Tx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	(InstancePtr)->Log.IsEnabled = FALSE;
}

/************************** Function Prototypes ******************************/
/* Self-test function in xi2stx_selftest.c */
int XI2s_Tx_SelfTest(XI2s_Tx *InstancePtr);

/* Initialization function in xi2stx_sinit.c */
XI2stx_Config *XI2s_Tx_LookupConfig(u16 DeviceId);

/* Initialization and control functions in xi2stx.c */
int XI2s_Tx_CfgInitialize(XI2s_Tx *InstancePtr,
		XI2stx_Config *CfgPtr, UINTPTR EffectiveAddr);

void XI2s_Tx_Enable(XI2s_Tx *InstancePtr, u8 Enable);

int XI2s_Tx_SetChMux(XI2s_Tx *InstancePtr, XI2s_Tx_ChannelId ChID,
		XI2s_Tx_ChMuxInput InputSource);
u32 XI2s_Tx_SetSclkOutDiv(XI2s_Tx *InstancePtr, u32 MClk, u32 Fs);

void XI2s_Tx_IntrEnable(XI2s_Tx *InstancePtr, u32 Mask);
void XI2S_Tx_IntrDisable(XI2s_Tx *InstancePtr, u32 Mask);

/* Justification related functions in XI2stx.c */
void XI2s_Tx_JustifyEnable(XI2s_Tx *InstancePtr, u8 Enable);
void XI2s_Tx_Justify(XI2s_Tx *InstancePtr, XI2s_Tx_Justification Justify);

/* Interrupt related functions in xi2stx_intr.c */
void XI2s_Tx_IntrHandler(void *InstancePtr);
int XI2s_Tx_SetHandler(XI2s_Tx *InstancePtr, XI2s_Tx_HandlerType HandlerType,
		XI2s_Tx_Callback FuncPtr, void *CallbackRef);

/* AES related functions */
void XI2s_Tx_GetAesChStatus(XI2s_Tx *InstancePtr, u8 *AesChStatusBuf);
void XI2s_Tx_ClrAesChStatRegs(XI2s_Tx *InstancePtr);

/* Logging */
void XI2s_Tx_LogDisplay(XI2s_Tx *InstancePtr);
void XI2s_Tx_LogWrite(XI2s_Tx *InstancePtr, XI2s_Tx_LogEvt Event, u8 Data);
XI2s_Tx_LogItem* XI2s_Tx_LogRead(XI2s_Tx *InstancePtr);
void XI2s_Tx_LogReset(XI2s_Tx *InstancePtr);
/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* XI2STX_H */
/** @} */
