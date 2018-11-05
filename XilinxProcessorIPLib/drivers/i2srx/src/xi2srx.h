/*******************************************************************************
 *
 * Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
 ******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xi2srx.h
 * @addtogroup i2srx_v1_0
 * @{
 *
 * <pre>
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who     Date     Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0   kar    01/25/18  Initial release.
 * 1.1   kar    04/02/18  Added debug log function prototypes.
 * </pre>
 *
 *****************************************************************************/

#ifndef XI2SRX_H
#define XI2SRX_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xi2srx_hw.h"
#include "xi2srx_debug.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/** @name Handler Types
 * @{
 */
/**
 * These constants specify different types of handlers and is used to
 * differentiate interrupt requests from the XI2s Receiver peripheral.
 */
typedef enum {
		XI2S_RX_HANDLER_AES_BLKCMPLT = 0,//!< AES Block Complete Handler
		XI2S_RX_HANDLER_AUD_OVRFLW, //!< Audio Overflow Detected Handler
		XI2S_RX_NUM_HANDLERS        //!< Number of handler types
} XI2s_Rx_HandlerType;

/** @name Handler Types
 * @{
 */
/**
 * These constants specify different channel ID's
 */
typedef enum {
		XI2S_RX_CHID0 = 0,  //!< Channel 0
		XI2S_RX_CHID1,      //!< Channel 1
		XI2S_RX_CHID2, //!< Channel 2
		XI2S_RX_CHID3, //!< Channel 3
		XI2S_RX_NUM_CHANNELS //!<Number of Channel ID's
} XI2s_Rx_ChannelId;

/*@}*/

/**
 * Callback function data type for handling interrupt requests
 * from the XI2s Receiver peripheral. The application using this driver is
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
	typedef void (*XI2s_Rx_Callback)(void *CallbackRef);

/**
 * @brief This typedef contains configuration information for the XI2s Receiver.
 */
	typedef struct {
		u32 DeviceId;//!< DeviceId is the unique ID of the XI2s Receiver
		UINTPTR BaseAddress; /* BaseAddress is the physical base address
				      * of the core's registers
				      */
		u8 DWidth; //!<Indicates the I2S data width of the core
		u8 IsMaster; /* Indicates if the core has been generated as an
			      * I2S Master/Slave
			      */
		u8 MaxNumChannels; /* Indicates the maximum number of channels
				    * supported by the core
				    */
	} XI2srx_Config;

/**
 * @brief The XI2s Receiver driver instance data.
 *
 * An instance must be allocated for each XI2s Receiver core in use.
 */
	typedef struct {

		u32 IsReady; //!< Core and the driver instance are initialized
		u32 IsStarted; //!< Core and the driver instance has started
		XI2srx_Config Config; //!< Hardware Configuration

		XI2s_Rx_Log Log; //!< Logging for XI2s Receiver

		XI2s_Rx_Callback AesBlkCmpltHandler; /* AES Block Complete
						      * Handler
						      */

		XI2s_Rx_Callback AudOverflowHandler;//!< Audio Overflow Handler

		/* Call backs */
		void *AesBlkCmpltRef; /* Callback reference for AES
				       * Block Complete Handler
				       */

		void *AudOverflowRef; /* Callback reference for Audio
				       * Overflow Handler
				       */
	} XI2s_Rx;

/**
 * @brief This typedef specifies the input sources of the the XI2s Receiver.
 */
typedef enum {
		XI2S_RX_CHMUX_DISABLED = 0, //!< Channel disabled
		XI2S_RX_CHMUX_XI2S_01,       //!< XI2S Audio Channel 0 and 1
		XI2S_RX_CHMUX_XI2S_23,       //!< XI2S Audio Channel 2 and 3
		XI2S_RX_CHMUX_XI2S_45,       //!< XI2S Audio Channel 4 and 5
		XI2S_RX_CHMUX_XI2S_67,       //!< XI2S Audio Channel 6 and 7
		XI2S_RX_CHMUX_WAVEGEN,      //!< Wave Generator
} XI2s_Rx_ChMuxInput;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
 *
 * This inline function reads the XI2s Receiver version
 *
 * @param  InstancePtr is a pointer to the XI2s_Rx core instance.
 *
 * @return Version.
 *
 *****************************************************************************/
static inline u32 XI2s_Rx_GetVersion(XI2s_Rx *InstancePtr)

{
		u32 I2sRx_Version;

		I2sRx_Version =
			XI2s_Rx_ReadReg((InstancePtr)->Config.BaseAddress,
					(XI2S_RX_CORE_VER_OFFSET));
		return I2sRx_Version;
}
/*****************************************************************************/
/**
 *
 * This inline function clears the specified interrupt of the XI2s Receiver.
 *
 * @param InstancePtr is a pointer to the XI2s_Rx core instance.
 * @param Mask is a bit mask of the interrupts to be cleared.
 *
 * @return None.
 * @see XI2srx_HW for the available interrupt masks.
 *
 *****************************************************************************/
static inline void XI2s_Rx_IntrClear(XI2s_Rx *InstancePtr, u32 Mask)

{
		Xil_AssertVoid(InstancePtr != NULL);
		XI2s_Rx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XI2S_RX_IRQSTS_OFFSET), Mask);

}
/*****************************************************************************/
/**
 *
 * This inline function enables the XI2s Receiver logging.
 *
 * @param InstancePtr is a pointer to the XI2s_Rx core instance.
 *
 * @return None.
 *
 *****************************************************************************/
static inline void XI2s_Rx_LogEnable(XI2s_Rx *InstancePtr)
{
		Xil_AssertVoid(InstancePtr != NULL);
		(InstancePtr)->Log.IsEnabled = TRUE;
}
/*****************************************************************************/
/**
 *
 * This inline function disables the XI2s Receiver logging.
 *
 * @param InstancePtr is a pointer to the XI2s_Rx core instance.
 *
 * @return None.
 *
 *****************************************************************************/
static inline void XI2s_Rx_LogDisable(XI2s_Rx *InstancePtr)
{
		Xil_AssertVoid(InstancePtr != NULL);
		(InstancePtr)->Log.IsEnabled = FALSE;
}

/************************** Function Prototypes ******************************/

/* Self-test function in xi2srx_selftest.c */
int XI2s_Rx_SelfTest(XI2s_Rx *InstancePtr);
/* Initialization function in XI2srx_sinit.c */
XI2srx_Config *XI2s_Rx_LookupConfig(u16 DeviceId);

int XI2s_Rx_Initialize(XI2s_Rx *InstancePtr, u16 DeviceId);

/* Initialization and control functions in XI2srx.c */
int XI2s_Rx_CfgInitialize(XI2s_Rx *InstancePtr,
		XI2srx_Config *CfgPtr, UINTPTR EffectiveAddr);

void XI2s_Rx_Enable(XI2s_Rx *InstancePtr, u8 Enable);
void XI2s_Rx_LatchAesChannelStatus(XI2s_Rx *InstancePtr);

int XI2s_Rx_SetChMux(XI2s_Rx *InstancePtr, XI2s_Rx_ChannelId ChID,
		XI2s_Rx_ChMuxInput InputSource);

u32 XI2s_Rx_SetSclkOutDiv(XI2s_Rx *InstancePtr, u32 MClk, u32 Fs);

void XI2s_Rx_IntrEnable(XI2s_Rx *InstancePtr, u32 Mask);
void XI2s_Rx_IntrDisable(XI2s_Rx *InstancePtr, u32 Mask);

/* Interrupt related functions in XI2srx_intr.c */
void XI2s_Rx_IntrHandler(void *InstancePtr);
int XI2s_Rx_SetHandler(XI2s_Rx *InstancePtr,
	XI2s_Rx_HandlerType HandlerType,
	XI2s_Rx_Callback FuncPtr, void *CallbackRef);
/* Logging */
void XI2s_Rx_LogDisplay(XI2s_Rx *InstancePtr);
void XI2s_Rx_LogReset(XI2s_Rx *InstancePtr);
void XI2s_Rx_LogWrite(XI2s_Rx *InstancePtr, XI2s_Rx_LogEvt Event, u8 Data);
XI2s_Rx_LogItem* XI2s_Rx_LogRead(XI2s_Rx *InstancePtr);

/* AES related functions */
void XI2s_Rx_SetAesChStatus(XI2s_Rx *InstancePtr, u8 *AesChStatusBuf);
void XI2s_Rx_ClrAesChStatRegs(XI2s_Rx *InstancePtr);

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* XI2SRX_H */
/** @} */
