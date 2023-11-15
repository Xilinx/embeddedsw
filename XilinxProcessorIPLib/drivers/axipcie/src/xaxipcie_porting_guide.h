/******************************************************************************
* Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xaxipcie_porting_guide.h
* @addtogroup axipcie Overview
* @{
*
* This is a guide on how to move from using the plbv46pcie driver to use
* xaxipcie driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a rkv  03/21/11 First release
* 2.00a nm   10/19/11 Added support of PCIe root complex functionality. The
*                     Root Port/Root Complex is available only in the 7 series
*                     families.
*
* </pre>
*
* <b>Overview</b>
*
* The API for xaxipcie driver is similar to plbv46pcie driver. The prefix for the
* API functions and structures is XAxiPcie_ for the xaxipcie driver.
*
* Due to hardware feature changes, some new API's are added and some API names
* are changed.
*
* We present AXI PCIE API functions:
* - That Map with PLBv46PCIE
* - That are new API functions
* - That have been removed
*
* <b>AXI PCIE API Functions that maps with PLBv46PCIE API</b>
*
* <pre>
*         plbv46pcie driver	     |         xaxipcie driver
* -----------------------------------------------------------------------
*   XPcie_LookupConfig(   )	     |  XAxiPcie_LookupConfig(...)
*   XPcie_CfgInitialize(...)	     |  XAxiPcie_CfgInitialize(...)
*   XPcie_GetRequesterID(...)        |  XAxiPcie_GetRequesterID(...)
*   XPcie_SetRequesterID(...)	     |  ............................
*   XPcie_GetLinkStatus(...)         |  XAxiPcie_IsLinkUp(...)
*   XPcie_GetLocalBusBar2PcieBar(...)|  XAxiPcie_GetLocalBusBar2PcieBar(...)
*   XPcie_SetLocalBusBar2PcieBar(...)|  XAxiPcie_SetLocalBusBar2PcieBar(...)
*   XPcie_ReadLocalConfigSpace(...)  |  XAxiPcie_ReadLocalConfigSpace(...)
*   XPcie_WriteLocalConfigSpace(...) |  XAxiPcie_WriteLocalConfigSpace(...)
*   XPcie_EnableInterrupts(...)      |  XAxiPcie_EnableInterrupts(...)
*   XPcie_DisableInterrupts(...)     |  XAxiPcie_DisableInterrupts(...)
*   XPcie_GetEnabledInterrupts(...)  |  XAxiPcie_GetEnabledInterrupts(...)
*   XPcie_GetPendingInterrupts(...)  |  XAxiPcie_GetPendingInterrupts(...)
*   XPcie_ClearPendingInterrupts(...)|  XAxiPcie_ClearPendingInterrupts(...)
*   XPcie_SetRequesterID(...)	     |  XAxiPcie_SetRequesterID(...)
*   XPcie_EnablePCIeConnection(...)  |  No enable feature in hardware.
*   XPcie_DisablePCIeConnection(..)  |  No Disable feature in hardware.
*   XPcie_ReadRemoteConfigSpace(..)  |  XAxiPcie_ReadRemoteConfigSpace(..)
*   XPcie_WriteRemoteConfigSpace(..) |  XAxiPcie_WriteRemoteConfigSpace(..)
*
*</pre>
*
* <b>API Functions that are new</b>
*
* - void XAxiPcie_GetVsecCapability(XAxiPcie *InstancePtr, u8 VsecNum,
*			u16 *VsecIdPtr, u8 *VersionPtr, u16 *NextCapPtr)
*
* - void XAxiPcie_GetVsecHeader(XAxiPcie *InstancePtr, u8 VsecNum,
*			u16 *VsecIdPtr, u8 *RevisionPtr, u16 *LengthPtr)
*
* - void XAxiPcie_GetBridgeInfo(XAxiPcie *InstancePtr, u8 *Gen2Ptr,
*					u8 *RootPortPtr, u8 *ECAMSizePtr);
*
*
* - void XAxiPcie_GetPhyStatusCtrl(XAxiPcie *InstancePtr, u32 *PhyState)
*
* - void XAxiPcie_EnableGlobalInterrupt(XAxiPcie *InstancePtr)
*
* - void XAxiPcie_DisableGlobalInterrupt(XAxiPcie *InstancePtr)
*
* - void XAxiPcie_GetRootPortStatusCtrl(XAxiPcie *InstancePtr, u32 *StatusPtr)
*
* - int XAxiPcie_SetRootPortMSIBase(XAxiPcie *InstancePtr,
*						unsigned long long MsiBase)
*
* - void XAxiPcie_GetRootPortErrFIFOMsg(XAxiPcie *InstancePtr, u16 *ReqIdPtr,
*					u8 *ErrType, u8 *ErrValid)
*
* - void XAxiPcie_ClearRootPortErrFIFOMsg(XAxiPcie *InstancePtr)
*
* - int XAxiPcie_GetRootPortIntFIFOReg(XAxiPcie *InstancePtr, u16 *ReqIdPtr,
*	u16 *MsiAddr, u8 *MsiInt, u8 *IntValid, u16 *MsiMsgData)
*
* - void XAxiPcie_ClearRootPortIntFIFOReg(XAxiPcie *InstancePtr)
*
* <b>API Functions That Have Been Removed</b>
*
* - void XPcie_GetRequestControl(XPcie *InstancePtr, u8 *MaxPayLoadPtr,
*								u8 *MaxReadPtr)
*
* - void XPcie_EnablePCIeConnection(XPcie *InstancePtr, u8 NumOfBars)
*
* - void XPcie_DisablePCIeConnection(XPcie *InstancePtr)
*
******************************************************************************/
/** @} */
