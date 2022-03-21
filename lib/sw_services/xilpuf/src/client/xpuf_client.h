/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpuf_client.h
* @addtogroup xpuf_client_apis XilPuf Versal Client APIs
* @{
* @cond xpuf_internal
* This file Contains the client function prototypes, defines and macros for
* the PUF hardware interface.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kpt  01/04/22 Initial release
*       am   02/28/22 Fixed MISRA C violation rule 8.3
*       kpt  03/16/22 Removed IPI related code and added mailbox support
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPUF_CLIENT_H
#define XPUF_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xpuf_mailbox.h"
#include "xpuf_defs.h"

/************************** Constant Definitions *****************************/
#define XPUF_REGISTRATION			(0x0U)
#define XPUF_REGEN_ON_DEMAND			(0x1U)
#define XPUF_REGEN_ID_ONLY			(0x2U)
#define XPUF_SHUTTER_VALUE			(0x81000100U)
#define XPUF_SYNDROME_MODE_4K			(0x0U)
#define XPUF_SYNDROME_MODE_12K			(0x1U)

/**************************** Type Definitions *******************************/
typedef enum {
	XPUF_READ_FROM_RAM = 0,		/**< Read PUF HD from RAM */
	XPUF_READ_FROM_EFUSE_CACHE	/**< Read PUF HD from cache */
} XPuf_ReadOption;

typedef struct {
	u32 SyndromeData[XPUF_MAX_SYNDROME_DATA_LEN_IN_WORDS];
	u32 Chash;
	u32 Aux;
	u32 PufID[XPUF_ID_LEN_IN_WORDS];
	u32 EfuseSynData[XPUF_EFUSE_TRIM_SYN_DATA_IN_WORDS];
				 /* Trimmed data to be written in efuse */
} XPuf_PufData;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XPuf_ClientInit(XPuf_ClientInstance* const InstancePtr, XMailbox* const MailboxPtr);
int XPuf_Registration(XPuf_ClientInstance *InstancePtr, const u64 DataAddr);
int XPuf_Regeneration(XPuf_ClientInstance *InstancePtr, const u64 DataAddr);
int XPuf_ClearPufID(XPuf_ClientInstance *InstancePtr);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPUF_CLIENT_H */