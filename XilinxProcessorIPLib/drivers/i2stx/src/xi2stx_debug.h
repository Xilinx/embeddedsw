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
 *****************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xi2stx_debug.h
 * @addtogroup i2stx_v1_1
 * @{
 *
 * This file contains defintions of data structures used in debugging and
 * logging the I2S Transmitter transactions.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0   kar    11/16/17 Initial release.
 * 1.1   kar    04/02/18 Moved debug function prototypes to xi2stx.h file.
 * </pre>
 *
 *****************************************************************************/

#ifndef XI2STX_DEBUG_H
#define XI2STX_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_assert.h"
#include "xil_types.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/
/** @ name Log Item Buffer Size
 * @{
 */
#define XI2S_TX_LOG_ITEM_BUFFER_SIZE (256)
/** @} */

/**************************** Type Definitions *******************************/

/** @name XI2s_Tx_LogEvt
 * @{
 */
/**
 * These constants specify different types of handlers and is used to
 * differentiate interrupt requests from the I2S Transmitter peripheral.
 */
typedef enum {
		XI2S_TX_AES_BLKCMPLT_EVT,    /**< AES Block Complete Event */
		XI2S_TX_AES_BLKSYNCERR_EVT,  /**< AES Block Sync Error Event */
		XI2S_TX_AES_CHSTSUPD_EVT,/**< AES Channel Status Updated Event*/
		XI2S_TX_AUD_UNDRFLW_EVT,   /**< Audio Underflow Detected Event*/
		XI2S_TX_LOG_EVT_INVALID      /**< Invalid Log Event */
} XI2s_Tx_LogEvt;
/*@}*/

/**
 * This structure is used to store log events
 */
typedef struct {
	XI2s_Tx_LogEvt Event;   /**< Log Event */
	u32            Data;     /**< Optional Data */
} XI2s_Tx_LogItem;

/**
 * The I2s Transmitter Log buffer.
 */
typedef struct {
		XI2s_Tx_LogItem Items[XI2S_TX_LOG_ITEM_BUFFER_SIZE];
		/**< Log item buffer */
		u16            Head;        /**< Head pointer */
		u16            Tail;        /**< Tail pointer */
		u8             IsEnabled;   /**< Logging Enabled/Disabled */
} XI2s_Tx_Log;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* XI2STX_DEBUG_H */
/** @} */
