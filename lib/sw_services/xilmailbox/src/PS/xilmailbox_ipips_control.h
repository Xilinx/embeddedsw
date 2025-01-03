/******************************************************************************
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilmailbox_ipips_control.h
 * @addtogroup xilmailbox Overview
 * @{
 * @details
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.8   ht   07/24/23    Restructure the code for more modularity.
 * 1.11  ht   01/02/25    Fix GCC warnings
 *
 *  *</pre>
 *
 *@note
 *****************************************************************************/


#ifndef XILMAILBOX_IPIPS_CONTROL_H
#define XILMAILBOX_IPIPS_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif


/***************************** Include Files *********************************/
#include "xilmailbox.h"
#include "xil_util.h"
#if !defined (__MICROBLAZE__) && !defined (__riscv)
#include "xscugic.h"
#endif
#include "sleep.h"
#ifdef SDT
#include "xinterrupt_wrap.h"
#endif

/**************************** Type Definitions *******************************/

/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/
#ifndef SDT
u32 XIpiPs_Init(XMailbox *InstancePtr, u8 DeviceId);
#else
u32 XIpiPs_Init(XMailbox *InstancePtr, UINTPTR BaseAddress);
#endif
u32 XIpiPs_Send(XMailbox *InstancePtr, u8 Is_Blocking);
u32 XIpiPs_SendData(XMailbox *InstancePtr, void *MsgBufferPtr,
		    u32 MsgLen, u8 BufferType, u8 Is_Blocking);
u32 XIpiPs_RecvData(XMailbox *InstancePtr, void *MsgBufferPtr,
		    u32 MsgLen, u8 BufferType);
u32 XIpiPs_PollforDone(XMailbox *InstancePtr);
#if !defined (__MICROBLAZE__) && !defined (__riscv)
void XIpiPs_ErrorIntrHandler(void *XMailboxPtr);
void XIpiPs_IntrHandler(void *XMailboxPtr);
#ifndef SDT
XStatus XIpiPs_RegisterIrq(XScuGic *IntcInstancePtr,
			   XMailbox *InstancePtr,
			   u32 IpiIntrId);
#else
XStatus XIpiPs_RegisterIrq(XMailbox *InstancePtr, u32 IpiIntrId);
#endif
#endif


#ifdef __cplusplus
}
#endif

#endif /* XILMAILBOX_IPIPS_CONTROL_H */
