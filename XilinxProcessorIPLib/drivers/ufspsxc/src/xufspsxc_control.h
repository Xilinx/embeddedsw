/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xufspsxc_control.h
* @addtogroup ufspsxc Overview
* @{
*
* This is the header file for the low-level functions of UFSPSXC driver.
* These functions will be used internally by the user API's.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------.
* 1.0   sk  01/16/24 First release
*
* </pre>
*
******************************************************************************/
#ifndef XUFSPSXC_LOWLEVEL_H_		/**< prevent circular inclusions */
#define XUFSPSXC_LOWLEVEL_H_		/**< by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xufspsxc.h"

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/

/************************** Function Prototypes ******************************/
u32 XUfsPsxc_SendUICCmd(const XUfsPsxc *InstancePtr, XUfsPsxc_UicCmd *UicCmdPtr);
u32 XUfsPsxc_SetHce(const XUfsPsxc *InstancePtr, u32 Value);
void XUfsPsxc_FillNopOutUpiu(XUfsPsxc *InstancePtr,
										XUfsPsxc_Xfer_CmdDesc *CmdDescPtr);
void XUfsPsxc_FillTestUnitRdyUpiu(XUfsPsxc *InstancePtr,
										XUfsPsxc_Xfer_CmdDesc *CmdDescPtr);
void XUfsPsxc_FillReadCmdUpiu(XUfsPsxc *InstancePtr,
		XUfsPsxc_Xfer_CmdDesc *CmdDescPtr, u64 Address, u32 BlkCnt, const u8 *Buff);
u32 XUfsPsxc_ProcessUpiu(const XUfsPsxc *InstancePtr, const XUfsPsxc_Xfer_CmdDesc *CmdDescPtr);
void XUfsPsxc_FillFlagUpiu(XUfsPsxc *InstancePtr, XUfsPsxc_Xfer_CmdDesc *CmdDescPtr,
								u32 IsRead, u32 FlagIDn);
void XUfsPsxc_FillDescUpiu(XUfsPsxc *InstancePtr, XUfsPsxc_Xfer_CmdDesc *CmdDescPtr,
								u32 Tsf_DW0, u32 IsRead, u32 Length);
void XUfsPsxc_FillAttrUpiu(XUfsPsxc *InstancePtr, XUfsPsxc_Xfer_CmdDesc *CmdDescPtr,
								u32 IsRead, u32 AttrIDn, u32 Value);
u32 XUfsPsxc_HostInitialize(XUfsPsxc *InstancePtr);
u32 XUfsPsxc_CardInitialize(XUfsPsxc *InstancePtr, XUfsPsxc_Xfer_CmdDesc *CmdDescPtr);
u32 XUfsPsxc_GetLUNInfo(XUfsPsxc *InstancePtr, XUfsPsxc_Xfer_CmdDesc *CmdDescPtr);
u32 XUfsPsxc_WritePhyReg(const XUfsPsxc *InstancePtr, XUfsPsxc_UicCmd *UicCmdPtr, u32 Address, u32 Value);
u32 XUfsPsxc_ReadPhyReg(const XUfsPsxc *InstancePtr, XUfsPsxc_UicCmd *UicCmdPtr,
							u32 Address, u32 *Value);
u32 XUfsPsxc_Set4KBlkSize(XUfsPsxc *InstancePtr, XUfsPsxc_Xfer_CmdDesc *CmdDescPtr);
u32 XUfsPsxc_PhyInit(const XUfsPsxc *InstancePtr);
u32 XUfsPsxc_SetRmmiConfig(const XUfsPsxc *InstancePtr);
u32 XUfsPsxc_EnableMPhy(const XUfsPsxc *InstancePtr);
u32 XUfsPsxc_ConfigureTxRxAttributes(const XUfsPsxc *InstancePtr, u32 SpeedGear,
				u32 RxTermCap, u32 TxTermCap);
u32 XUfsPsxc_ReadDeviceInfo(XUfsPsxc *InstancePtr, XUfsPsxc_Xfer_CmdDesc *CmdDescPtr);
void XUfsPsxc_FillWriteCmdUpiu(XUfsPsxc *InstancePtr,
		XUfsPsxc_Xfer_CmdDesc *CmdDescPtr, u64 Address, u32 BlkCnt, const u8 *Buff);

#ifdef __cplusplus
}
#endif

#endif /* XUFSPSXC_LOWLEVEL_H_ */
/** @} */
