/*******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xi2srx_debug.h
 * @addtogroup i2srx Overview
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
