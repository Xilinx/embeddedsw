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
 * @file xi2srx_debug.h
 * @addtogroup i2srx_v1_0
 * @{
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0   kar    01/25/18 Initial release.
 * 1.1   kar    04/02/18 Moved debug Function prototypes to xi2srx.h file
 * </pre>
 *
 *****************************************************************************/

#ifndef XI2SRX_DEBUG_H
#define XI2SRX_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_assert.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xi2srx.h"
#include "xi2srx_hw.h"
#include "xi2srx_chsts.h"

/************************** Constant Definitions *****************************/
/** @ name Log Item Buffer Size
 * @{
 */
#define XI2S_RX_LOG_ITEM_BUFFER_SIZE (256)
/** @} */
/**************************** Type Definitions *******************************/

/** @name Handler Types
 * @{
 */
/**
 * These constants specify different types of handlers and is used to
 * differentiate interrupt requests from the XI2s Receiver peripheral.
 */
typedef enum {
		XI2S_RX_AES_BLKCMPLT_EVT,    //!< AES Block Complete Event
		XI2S_RX_AUD_OVERFLOW_EVT,    //!< Audio Overflow Detected Event
		XI2S_RX_LOG_EVT_INVALID      //!< Invalid Log Event
} XI2s_Rx_LogEvt;
/*@}*/

/**
 * @brief This structure is used to store log events
 */
typedef struct {
		XI2s_Rx_LogEvt Event; //!< Log Event
		u32            Data;  //!< Optional Data
} XI2s_Rx_LogItem;

/**
 * @brief The XI2s Receiver Log buffer.
 */
typedef struct {
		XI2s_Rx_LogItem Items[XI2S_RX_LOG_ITEM_BUFFER_SIZE];
		//!< Log item buffer
		u16            Head;       //!< Head pointer
		u16            Tail;       //!< Tail pointer
		u8             IsEnabled;  //!< Logging Enabled/Disabled
} XI2s_Rx_Log;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* XI2SRX_DEBUG_H */
/** @} */
