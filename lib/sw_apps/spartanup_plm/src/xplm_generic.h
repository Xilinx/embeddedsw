/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_generic.h
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  ng   05/31/24 Initial release
 *       ng   02/14/25 Moved the macro to source file to restrict it's scope
 * </pre>
 *
 ******************************************************************************/

/**
 * @addtogroup spartanup_plm_apis SpartanUP PLM APIs
 * @{
 */

#ifndef XPLM_GENERIC_H
#define XPLM_GENERIC_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplm_debug.h"
#include "xplm_cdo.h"
#include "xplm_util.h"

/************************** Constant Definitions *****************************/
/**************************** Type Definitions *******************************/
/**
 * Structure to hold the information to transfer the data to Capture Control Unit.
 */
typedef struct {
	u32 SrcAddr; /**< Address to the keyhole payload */
	u32 DestAddr; /**< Address to CCU write stream */
	u32 BaseAddr; /**< Start address of the CCU write stream */
	u32 Len; /**< Length of the payload to write */
	u32 Keyholesize; /**< Keyhole limit */
	u32 Flags; /**< Keyhole transfer DMA flags */
} XPlm_KeyHoleXfrParams;

/***************** Macros (Inline Functions) Definitions *********************/

/** @cond spartanup_plm_internal */
#define XPLM_READBK_INTF_TYPE_SMAP		(0x0U)
#define XPLM_READBK_INTF_TYPE_JTAG		(0x1U)
#define XPLM_READBACK_SRC_MASK			(0xFFU)

#define XPLM_CFI_DATA_OFFSET			(4U)

/* Mask poll command flag descriptions */
#define XPLM_MASKPOLL_LEN_EXT			(5U)
#define XPLM_MASKPOLL_FLAGS_MASK		(0x3U)
#define XPLM_MASKPOLL_FLAGS_SUCCESS		(0x1U)
#define XPLM_MASKPOLL_FLAGS_DEFERRED_ERR	(0x2U)
#define XPLM_MASKPOLL_FLAGS_BREAK		(0x3U)
#define XPLM_MASKPOLL_FLAGS_BREAK_LEVEL_MASK	(0xFF000000U)
#define XPLM_MASKPOLL_FLAGS_BREAK_LEVEL_SHIFT	(24U)

/* Define related to break */
#define XPLM_BREAK_LEVEL_MASK			(0xFFU)
/** @endcond */

/************************** Function Prototypes ******************************/
void XPlm_GenericInit(void);

/************************** Variable Definitions *****************************/

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* XPLM_GENERIC_H */

/** @} end of spartanup_plm_apis group*/
