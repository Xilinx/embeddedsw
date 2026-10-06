/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XQSPIPSU_CONTROL_H_
#define XQSPIPSU_CONTROL_H_

/*****************************************************************************/
/**
 *
 * @file xqspipsu_control.h
 * @brief Declares the intermediate control functions of the QSPIPSU driver.
 *
 * The xqspipsu_control.h file is the header file for the implementation of
 * the QSPIPSU driver.
 * Generic QSPI interface allows for communication to any QSPI slave device.
 * GQSPI contains a GENFIFO into which the bus transfers required are to be
 * pushed with appropriate configuration. The controller provides TX and RX
 * FIFOs and a DMA to be used for RX transfers. The controller executes each
 * GENFIFO entry noting the configuration and places data on the bus as
 * required.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------.
 * 1.11   akm  03/09/20 First release
 * 1.13   akm  01/04/21 Fix MISRA-C violations.
 * 1.15   akm  03/03/22 Enable tapdelay settings for applications on
 * 			 Microblaze platform.
 * 1.18   sb   08/29/23 Added function prototypes for XQspiPsu_PolledMessageTransfer, XQspiPsu_PolledRecvData
 *                      XQspiPsu_PolledSendData, XQspiPsu_IntrDataTransfer, XQspiPsu_IntrSendData,
 *                      XQspiPsu_IntrRecvData and XQspiPsu_IntrDummyDataTransfer.
 *        zm   06/18/26 Queued whole transfer into GENFIFO to fix bus conflict.
 *        zm   06/30/26 Extend the XPS_SYS_CTRL_BASEADDR macro for ARMR5, __aarch64__(non-versal) platforms.
 * 1.24   tb   09/30/26 Align Doxygen comments with AMD standards
 * 1.24   zm   09/29/26 Add MAX_DELAY_CNT macro definition and XQspiPsu_StartDmaMsgTransfer prototype.
 *
 * </pre>
 *
 ******************************************************************************/

/** @addtogroup qspipsu_api QSPIPSU APIs
 * @{
 */

/** @cond INTERNAL */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xqspipsu.h"

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#if defined (ARMR5) || defined (__aarch64__) || defined (__MICROBLAZE__) || defined (__riscv)
/**
 * @name Tap delay settings
 * Defines the tap delay configuration values.
 * @{
 */
#define TAPDLY_BYPASS_VALVE_40MHZ 0x01U /**< Tap delay bypass at 40 MHz. */
#define TAPDLY_BYPASS_VALVE_100MHZ 0x01U /**< Tap delay bypass at 100 MHz. */
#define USE_DLY_LPBK  0x01U /**< Enables the loopback delay. */
#define USE_DATA_DLY_ADJ 0x01U /**< Enables the data delay adjustment. */
#define DATA_DLY_ADJ_DLY 0X02U /**< Data delay adjustment value. */
#define LPBK_DLY_ADJ_DLY0 0X02U /**< Loopback delay adjustment value 0. */
#define LPBK_DLY_ADJ_DLY1 0X02U /**< Loopback delay adjustment value 1. */
/** @} */
#endif

#if (defined (ARMR5) || defined (__aarch64__) || defined (__MICROBLAZE__) || defined (__riscv)) && !defined (versal)
#ifndef XPS_SYS_CTRL_BASEADDR
/** System controller base address. */
#define XPS_SYS_CTRL_BASEADDR   0xFF180000U
#endif
#endif

#define MAX_DELAY_CNT	1000000000U	/**< Maximum delay count. */

/************************** Function Prototypes ******************************/
void XQspiPsu_GenFifoEntryData(XQspiPsu *InstancePtr, XQspiPsu_Msg *Msg);
s32 XQspiPsu_StartDmaMsgTransfer(XQspiPsu *InstancePtr, XQspiPsu_Msg *Msg);
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
s32 XQspiPsu_PolledMessageTransfer(XQspiPsu *InstancePtr, XQspiPsu_Msg *Msg,
				   u32 NumMsg);
s32 XQspiPsu_PolledRecvData(XQspiPsu *InstancePtr, XQspiPsu_Msg *Msg,
			    s32 Index, u32 *IOPending);
s32 XQspiPsu_PolledSendData(XQspiPsu *InstancePtr, XQspiPsu_Msg *Msg,
			    s32 Index);
void XQspiPsu_IntrDataTransfer(XQspiPsu *InstancePtr,
			       u32 *QspiPsuStatusReg);
void XQspiPsu_IntrSendData(XQspiPsu *InstancePtr,
			   u32 QspiPsuStatusReg);
void XQspiPsu_IntrRecvData(XQspiPsu *InstancePtr,
			   u32 QspiPsuStatusReg, u32 DmaIntrStatusReg);
void XQspiPsu_IntrDummyDataTransfer(XQspiPsu *InstancePtr, u32 QspiPsuStatusReg);

#if defined (ARMR5) || defined (__aarch64__) || defined (__MICROBLAZE__) || defined (__riscv)
s32 XQspipsu_Set_TapDelay(const XQspiPsu *InstancePtr, u32 TapdelayBypass,
			  u32 LPBKDelay, u32 Datadelay);
s32 XQspipsu_Calculate_Tapdelay(const XQspiPsu *InstancePtr, u8 Prescaler);
#endif

#ifdef __cplusplus
}
#endif


/** @endcond */
/** @} */
#endif /* XQSPIPSU_CONTROL_H_ */
