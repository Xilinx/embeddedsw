/******************************************************************************
* Copyright (C) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
*
* @file xdmapcie.h
*
* This file contains the software API definition of the Xilinx XDMA PCIe IP
* (XDma_0). This driver provides "C" function interface to application/upper
* layer to access the hardware.
*
* <b>Features</b>
* The driver provides its user with entry points
*   - To initialize and configure itself and the hardware
*   - To access PCIe configuration space locally
*
* <b>Driver Initialization & Configuration</b>
*
* The XDmaPcie_Config structure is used by the driver to configure itself. This
* configuration structure is typically created by the tool-chain based on HW
* build properties.
*
* To support multiple runtime loading and initialization strategies employed
* by various operating systems, the driver instance can be initialized in the
* following way:
*
*   - XDmaPcie_LookupConfig(DeviceId) - Use the device identifier to find the
*   static configuration structure defined in xdmapcie_g.c. This is setup by
*   the tools.
*
*   - XDmaPcie_CfgInitialize(InstancePtr, CfgPtr, EffectiveAddr) - Uses a
*   configuration structure provided by the caller. If running in a system with
*   address translation, the provided virtual memory base address replaces the
*   physical address present in the configuration structure.
*
* <b>Interrupt Management</b>
*
* The XDmaPcie driver provides interrupt management functions. It allows
* the caller to enable/disable each individual interrupt as well as get/clear
* pending interrupts. Implementation of callback handlers is left to the user.
*
* @note
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.0	tk	01/30/2019	First release
* </pre>
*
*****************************************************************************/
#ifndef XDMAPCIE_H			/* prevent circular inclusions */
#define XDMAPCIE_H			/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/
#include "xil_assert.h"
#include "xstatus.h"
#include "xdmapcie_hw.h"
#include "xil_types.h"

#include <string.h>
#include "xil_cache.h"


/************************** Constant Definitions ****************************/

/*
 * To figure out if PCIe IP
 * is configured as an end
 * point or as a root complex
 */
#define XDMAPCIE_IS_RC		0x01

/*
 * 4KB alignment
 */
#define ALIGN_4KB		0xFFFFF000

/*
 * Version Specific Enhanced Capability register numbers.
 */
#define XDMAPCIE_VSEC1		0x00 /**< First VSEC Register */
#define XDMAPCIE_VSEC2		0x01 /**< Second VSEC Register */

/**************************** Type Definitions ******************************/

/**
 * This typedef contains IP hardware configuration information.
 */

typedef  struct {
#ifndef SDT
	u16 DeviceId;			/**< Unique ID of PCIe IP */
#else
	char *Name;			/* Compatible string */
#endif
	UINTPTR BaseAddress;		/**< Register base address */
	u8  LocalBarsNum;		/* The number of local bus (AXI) BARs
					 * in hardware
					 */
#if !defined(versal) || defined(QDMA_PCIE_BRIDGE) || defined(XDMA_PCIE_BRIDGE) || defined(SDT)
	u8  IncludeBarOffsetReg;	/**<Are BAR Offset registers built in
					 * hardware
					 */
#endif
	u8  IncludeRootComplex;		/**< Is IP built as root complex */
#if defined(__aarch64__) || defined(__arch64__)
#if defined(SDT)
	u64 Ecam;
	u32     NpMemBaseAddr;          /**< non prefetchable memory base address */
	u32     NpMemMaxAddr;   /**< non prefetchable memory max base address*/
	u64     PMemBaseAddr;           /**< prefetchable memory base address */
	u64     PMemMaxAddr;    /**< prefetchable memory max base address */
#else
	u64 Ecam;
	u32	NpMemBaseAddr;		/**< non prefetchable memory base address */
	u64	PMemBaseAddr;		/**< prefetchable memory base address */
	u32	NpMemMaxAddr;	/**< non prefetchable memory max base address*/
	u64	PMemMaxAddr;	/**< prefetchable memory max base address */
#endif
#else
	u32 Ecam;
	u32	NpMemBaseAddr;		/**< non prefetchable memory base address */
	u32	NpMemMaxAddr;	/**< non prefetchable memory max base address*/
#endif

} XDmaPcie_Config;

/**
 * The XDmaPcie driver instance data. The user is required to allocate a
 * variable of this type for every PCIe device in the system that will be
 * using this API. A pointer to a variable of this type is passed to the driver
 * API functions defined here.
 */

typedef struct {
	XDmaPcie_Config Config;		/**< Configuration data */
	u32 IsReady;			/**< Is IP been initialized and ready */
	u32 MaxNumOfBuses;		/**< If this is RC IP, Max Number of
					 * Buses */

} XDmaPcie;


/**
 * The user is required to use this strucuture when reading or writing
 * translation vector between local bus BARs and XDMA PCIe BARs. It is used
 * when calling "XDmaPcie_GetLocalBusBar2PcieBar" and
 * "XDmaPcie_SetLocalBusBar2PcieBar" functions. The translation vectors are
 * 64 bits wide even though they might only use the lower 32 bits
 */
typedef struct {
	u32 LowerAddr;		/**< Lower 32 bits of translation value */
	u32 UpperAddr;		/**< Upper 32 bits of translation value */
} XDmaPcie_BarAddr;

/***************** Macros (Inline Functions) Definitions ********************/

#ifndef XDmaPcie_GetRequestId
#define XDmaPcie_GetRequestId XDmaPcie_GetRequesterId
#endif

/****************************************************************************/
/**
* Check whether link is up or not.
*
* @param	InstancePtr is the XDmaPcie instance to operate on.
*
* @return
*		- TRUE if link is up
*		- FALSE if link is down
*
* @note		None
*
*****************************************************************************/
#define	 XDmaPcie_IsLinkUp(InstancePtr) 	\
	(XDmaPcie_ReadReg((InstancePtr)->Config.BaseAddress, 	\
	XDMAPCIE_PHYSC_OFFSET) & XDMAPCIE_PHYSC_LINK_UP_MASK) ? TRUE : FALSE


/****************************************************************************/
/**
* Check whether ECAM is busy or not.
*
* @param	InstancePtr is the XDmaPcie instance to operate on.
*
* @return
*		- TRUE if ECAM is busy
*		- FALSE if ECAM is idel
*
* @note		This function is valid only when IP is configured as a
*		root complex
*
*****************************************************************************/
#define	 XDmaPcie_IsEcamBusy(InstancePtr) 	\
	(XDmaPcie_ReadReg((InstancePtr)->Config.BaseAddress, 	\
	XDMAPCIE_BSC_OFFSET) & XDMAPCIE_BSC_ECAM_BUSY_MASK) ? TRUE : FALSE

/************************** Function Prototypes *****************************/

/*
 * API exported by the driver to the upper layer
 *
 * The following are the driver interface entry points.
 *
 */

/*
 * Config Look Up Function.
 * This API is implemented in xdmapcie_sinit.c
 */

#ifndef SDT
XDmaPcie_Config * XDmaPcie_LookupConfig(u16 DeviceId);
#else
XDmaPcie_Config * XDmaPcie_LookupConfig(UINTPTR BaseAddress);
#endif

/*
 * PCIe Setup and Configuration Functions.
 * This API is implemented in xdmapcie.c
 */

/* Config Initialization */
int XDmaPcie_CfgInitialize(XDmaPcie * InstancePtr, XDmaPcie_Config * CfgPtr,
							UINTPTR EffectiveAddress);
void XDmaPcie_GetVsecCapability(XDmaPcie *InstancePtr, u8 VsecNum,
			u16 *VsecIdPtr, u8 *VersionPtr, u16 *NextCapPtr);
void XDmaPcie_GetVsecHeader(XDmaPcie *InstancePtr, u8 VsecNum, u16 *VsecIdPtr,
				u8 *RevisionPtr, u16 *LengthPtr);
void XDmaPcie_GetBridgeInfo(XDmaPcie *InstancePtr, u8 *Gen2Ptr,
					u8 *RootPortPtr, u8 *ECAMSizePtr);
void XDmaPcie_GetRequesterId(XDmaPcie *InstancePtr, u8 *BusNumPtr,
				u8 *DevNumPtr, u8 *FunNumPtr, u8 *PortNumPtr);
void XDmaPcie_GetPhyStatusCtrl(XDmaPcie *InstancePtr, u32 *PhyState);
void XDmaPcie_GetRootPortStatusCtrl(XDmaPcie *InstancePtr, u32 *StatusPtr);
void XDmaPcie_SetRootPortStatusCtrl(XDmaPcie *InstancePtr, u32 StatusData);
int XDmaPcie_SetRootPortMSIBase(XDmaPcie *InstancePtr,
						unsigned long long MsiBase);
void XDmaPcie_GetRootPortErrFIFOMsg(XDmaPcie *InstancePtr, u16 *ReqIdPtr,
					u8 *ErrType, u8 *ErrValid);
void XDmaPcie_ClearRootPortErrFIFOMsg(XDmaPcie *InstancePtr);
int XDmaPcie_GetRootPortIntFIFOReg(XDmaPcie *InstancePtr, u16 *ReqIdPtr,
	u16 *MsiAddr, u8 *MsiInt, u8 *IntValid, u16 *MsiMsgData);
void XDmaPcie_ClearRootPortIntFIFOReg(XDmaPcie *InstancePtr);
void XDmaPcie_GetLocalBusBar2PcieBar(XDmaPcie *InstancePtr, u8 BarNumber,
						XDmaPcie_BarAddr *BarAddrPtr);
void XDmaPcie_SetLocalBusBar2PcieBar(XDmaPcie *InstancePtr, u8 BarNumber,
						XDmaPcie_BarAddr *BarAddrPtr);
void XDmaPcie_ReadLocalConfigSpace(XDmaPcie *InstancePtr, u16 Offset,
								u32 *DataPtr);
void XDmaPcie_WriteLocalConfigSpace(XDmaPcie *InstancePtr, u16 Offset,
								u32 Data);
void XDmaPcie_ReadRemoteConfigSpace(XDmaPcie *InstancePtr, u8 Bus, u8 Device,
		u8 Function, u16 Offset, u32 *DataPtr);
void XDmaPcie_WriteRemoteConfigSpace(XDmaPcie *InstancePtr, u8 Bus, u8 Device,
					u8 Function, u16 Offset, u32 Data);
u32 XDmaPcie_ComposeExternalConfigAddress(u8 Bus, u8 Device, u8 Function,
								 u16 Offset);
void XDmaPcie_EnumerateFabric(XDmaPcie *XdmaPciePtr);

/*
 * Interrupt Functions.
 * This API is implemented in xdmapcie_intr.c
 */
void XDmaPcie_EnableGlobalInterrupt(XDmaPcie *InstancePtr);
void XDmaPcie_DisableGlobalInterrupt(XDmaPcie *InstancePtr);
void XDmaPcie_EnableInterrupts(XDmaPcie *InstancePtr, u32 EnableMask);
void XDmaPcie_DisableInterrupts(XDmaPcie *InstancePtr, u32 DisableMask);
void XDmaPcie_GetEnabledInterrupts(XDmaPcie *InstancePtr, u32 *EnabledMaskPtr);
void XDmaPcie_GetPendingInterrupts(XDmaPcie *InstancePtr, u32 *PendingMaskPtr);
void XDmaPcie_ClearPendingInterrupts(XDmaPcie *InstancePtr, u32 ClearMask);

/*
 * Capabilites Functions.
 * This API is implemented in xdmapcie_caps.c
 */
u8 XDmaPcie_HasCapability(XDmaPcie *InstancePtr, u8 Bus, u8 Device,
		u8 Function, u8 CapId);
u64 XDmaPcie_GetCapability(XDmaPcie *InstancePtr, u8 Bus, u8 Device,
		u8 Function, u8 CapId);
u8 XDmaPcie_PrintAllCapabilites(XDmaPcie *InstancePtr, u8 Bus, u8 Device,
		u8 Function);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
