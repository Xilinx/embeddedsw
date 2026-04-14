/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
/* Send a UIC command to the controller. */
u32 XUfsPsxc_SendUICCmd(const XUfsPsxc *InstancePtr, XUfsPsxc_UicCmd *UicCmdPtr);
/* Set Host Controller Enable register. */
u32 XUfsPsxc_SetHce(const XUfsPsxc *InstancePtr, u32 Value);
/* Fill NOP OUT UPIU for link verification. */
void XUfsPsxc_FillNopOutUpiu(XUfsPsxc *InstancePtr,
										XUfsPsxc_Xfer_CmdDesc *CmdDescPtr);
/* Fill Test Unit Ready UPIU. */
void XUfsPsxc_FillTestUnitRdyUpiu(XUfsPsxc *InstancePtr,
										XUfsPsxc_Xfer_CmdDesc *CmdDescPtr);
/* Fill SCSI Read command UPIU with PRDT entries. */
void XUfsPsxc_FillReadCmdUpiu(XUfsPsxc *InstancePtr,
		XUfsPsxc_Xfer_CmdDesc *CmdDescPtr, u64 Address, u32 BlkCnt, const u8 *Buff);
/* Process UPIU transfer and wait for completion. */
u32 XUfsPsxc_ProcessUpiu(const XUfsPsxc *InstancePtr, const XUfsPsxc_Xfer_CmdDesc *CmdDescPtr);
/* Fill Query Flag UPIU for read or set operations. */
void XUfsPsxc_FillFlagUpiu(XUfsPsxc *InstancePtr, XUfsPsxc_Xfer_CmdDesc *CmdDescPtr,
								u32 IsRead, u32 FlagIDn);
/* Fill Query Descriptor UPIU for read or write operations. */
void XUfsPsxc_FillDescUpiu(XUfsPsxc *InstancePtr, XUfsPsxc_Xfer_CmdDesc *CmdDescPtr,
								u32 Tsf_DW0, u32 IsRead, u32 Length);
/* Fill Query Attribute UPIU for read or write operations. */
void XUfsPsxc_FillAttrUpiu(XUfsPsxc *InstancePtr, XUfsPsxc_Xfer_CmdDesc *CmdDescPtr,
								u32 IsRead, u32 AttrIDn, u32 Value);
/* Initialize the UFS host controller. */
u32 XUfsPsxc_HostInitialize(XUfsPsxc *InstancePtr);
/* Partially initialize the UFS device. */
u32 XUfsPsxc_CardInitialize(XUfsPsxc *InstancePtr, XUfsPsxc_Xfer_CmdDesc *CmdDescPtr);
/* Read LUN information from configuration descriptor. */
u32 XUfsPsxc_GetLUNInfo(XUfsPsxc *InstancePtr, XUfsPsxc_Xfer_CmdDesc *CmdDescPtr);
/* Write a PHY register via UIC indirect access. */
u32 XUfsPsxc_WritePhyReg(const XUfsPsxc *InstancePtr, XUfsPsxc_UicCmd *UicCmdPtr, u32 Address, u32 Value);
/* Read a PHY register via UIC indirect access. */
u32 XUfsPsxc_ReadPhyReg(const XUfsPsxc *InstancePtr, XUfsPsxc_UicCmd *UicCmdPtr,
							u32 Address, u32 *Value);
/* Set block size to 4K for boot LUN. */
u32 XUfsPsxc_Set4KBlkSize(XUfsPsxc *InstancePtr, XUfsPsxc_Xfer_CmdDesc *CmdDescPtr);
/* Initialize M-PHY and UniPro layers. */
u32 XUfsPsxc_PhyInit(const XUfsPsxc *InstancePtr);
/* Set up RMMI configuration for M-PHY. */
u32 XUfsPsxc_SetRmmiConfig(const XUfsPsxc *InstancePtr);
/* Enable M-PHY and wait for TX/RX ready. */
u32 XUfsPsxc_EnableMPhy(const XUfsPsxc *InstancePtr);
/* Override PHY RX request for lane initialization. */
u32 XUfsPsxc_OverridePhyRxReq(const XUfsPsxc *InstancePtr, u32 RxReq, u32 NumLanes);
/* Configure TX/RX attributes for power mode change. */
u32 XUfsPsxc_ConfigureTxRxAttributes(const XUfsPsxc *InstancePtr, u32 SpeedGear,
				u32 RxTermCap, u32 TxTermCap);
/* Read device and geometry descriptor information. */
u32 XUfsPsxc_ReadDeviceInfo(XUfsPsxc *InstancePtr, XUfsPsxc_Xfer_CmdDesc *CmdDescPtr);
/* Fill SCSI Write command UPIU with PRDT entries. */
void XUfsPsxc_FillWriteCmdUpiu(XUfsPsxc *InstancePtr,
		XUfsPsxc_Xfer_CmdDesc *CmdDescPtr, u64 Address, u32 BlkCnt, const u8 *Buff);
/* Set device bRefClkFreq attribute based on reference pad clock. */
u32 XUfsPsxc_SetbRefClkFreq(XUfsPsxc *InstancePtr, XUfsPsxc_Xfer_CmdDesc *CmdDescPtr);

#ifdef __cplusplus
}
#endif

#endif /* XUFSPSXC_LOWLEVEL_H_ */
/** @} */
