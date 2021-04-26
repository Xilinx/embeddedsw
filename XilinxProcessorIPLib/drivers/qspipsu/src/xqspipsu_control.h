/******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
 *
 * @file xqspipsu_control.h
 * @addtogroup qspipsu_v1_13
 * @{
 *
 * This is the header file for the implementation of QSPIPSU driver.
 * Generic QSPI interface allows for communication to any QSPI slave device.
 * GQSPI contains a GENFIFO into which the bus transfers required are to be
 * pushed with appropriate configuration. The controller provides TX and RX
 * FIFO's and a DMA to be used for RX transfers. The controller executes each
 * GENFIFO entry noting the configuration and places data on the bus as required
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------.
 * 1.11   akm  03/09/20 First release
 * 1.13   akm  01/04/21 Fix MISRA-C violations.
 *
 * </pre>
 *
 ******************************************************************************/

#ifndef XQSPIPSU_CONTROL_H_		/**< prevent circular inclusions */
#define XQSPIPSU_CONTROL_H_		/**< by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xqspipsu.h"

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#if defined (ARMR5) || (__aarch64__)
#define TAPDLY_BYPASS_VALVE_40MHZ 0x01U
#define TAPDLY_BYPASS_VALVE_100MHZ 0x01U
#define USE_DLY_LPBK  0x01U
#define USE_DATA_DLY_ADJ 0x01U
#define DATA_DLY_ADJ_DLY 0X02U
#define LPBK_DLY_ADJ_DLY0 0X02U
#define LPBK_DLY_ADJ_DLY1 0X02U
#endif

/************************** Function Prototypes ******************************/
void XQspiPsu_GenFifoEntryData(XQspiPsu *InstancePtr, XQspiPsu_Msg *Msg);
u32 XQspiPsu_SetIOMode(XQspiPsu *InstancePtr, XQspiPsu_Msg *Msg);
void XQspiPsu_IORead(XQspiPsu *InstancePtr, XQspiPsu_Msg *Msg,
		u32 StatusReg);
void XQspiPsu_PollDataConfig(XQspiPsu *InstancePtr, XQspiPsu_Msg *FlashMsg);
void XQspiPsu_TXSetup(XQspiPsu *InstancePtr, XQspiPsu_Msg *Msg);
void XQspiPsu_SetupRxDma(const XQspiPsu *InstancePtr,
					XQspiPsu_Msg *Msg);
void XQspiPsu_Setup64BRxDma(const XQspiPsu *InstancePtr,
					XQspiPsu_Msg *Msg);
void XQspiPsu_RXSetup(XQspiPsu *InstancePtr, XQspiPsu_Msg *Msg);
void XQspiPsu_TXRXSetup(XQspiPsu *InstancePtr, XQspiPsu_Msg *Msg,
					u32 *GenFifoEntry);
void XQspiPsu_GenFifoEntryDataLen(XQspiPsu *InstancePtr, XQspiPsu_Msg *Msg,
		u32 *GenFifoEntry);
u32 XQspiPsu_CreatePollDataConfig(const XQspiPsu *InstancePtr,
		const XQspiPsu_Msg *FlashMsg);
void XQspiPsu_PollDataHandler(XQspiPsu *InstancePtr, u32 StatusReg);
u32 XQspiPsu_SelectSpiMode(u8 SpiMode);
void XQspiPsu_SetDefaultConfig(XQspiPsu *InstancePtr);
void XQspiPsu_FillTxFifo(XQspiPsu *InstancePtr, XQspiPsu_Msg *Msg, u32 Size);
void XQspiPsu_ReadRxFifo(XQspiPsu *InstancePtr,	XQspiPsu_Msg *Msg, s32 Size);

#if defined (ARMR5) || (__aarch64__)
s32 XQspipsu_Set_TapDelay(const XQspiPsu *InstancePtr, u32 TapdelayBypass,
						u32 LPBKDelay, u32 Datadelay);
s32 XQspipsu_Calculate_Tapdelay(const XQspiPsu *InstancePtr, u8 Prescaler);
#endif

#ifdef __cplusplus
}
#endif


#endif /* XQSPIPSU_CONTROL_H_ */
/** @} */
