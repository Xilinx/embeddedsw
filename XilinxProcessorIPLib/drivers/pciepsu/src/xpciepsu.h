/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
* @file xpciepsu.h
*
* This file contains the software API definition of the Xilinx PSU PCI IP
* (psu_pcie). This driver provides "C" function interface to application/upper
* layer to access the hardware.
*
* <b>Features</b>
* The driver provides its user with entry points
*   - To initialize and configure itself and the hardware
*   - To access PCIe configuration space locally
*
* <b>Driver Initialization & Configuration</b>
*
* The XPciePsu_Config structure is used by the driver to configure itself. This
* configuration structure is typically created by the tool-chain based on HW
* build properties.
*
* To support multiple runtime loading and initialization strategies employed
* by various operating systems, the driver instance can be initialized in the
* following way:
*
*   - XPciePsu_LookupConfig(DeviceId) - Use the device identifier to find the
*   static configuration structure defined in xpciepsu_g.c. This is setup by
*   the tools.
*
*   - XPciePsu_CfgInitialize(InstancePtr, CfgPtr, EffectiveAddress) - Uses a
*   configuration structure provided by the caller. If running in a system with
*   address translation, the provided virtual memory base address replaces the
*   physical address present in the configuration structure.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.0	bs	08/21/2018	First release
* </pre>
*
*******************************************************************************/
#ifndef XPCIEPSU_H_
#define XPCIEPSU_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************** Include Files *******************************/
#include "xil_types.h"
#include "xstatus.h"
#include "xil_io.h"
#include "xpciepsu_hw.h"

/**************************** Constant Definitions ****************************/

/******************** Macros (Inline Functions) Definitions *******************/
#define XPciePsu_ReadReg(BaseAddr, RegOffset) Xil_In32((BaseAddr) + (RegOffset))

#define XPciePsu_WriteReg(BaseAddr, RegOffset, Val)                            \
	Xil_Out32((BaseAddr) + (RegOffset), (Val))

#define ARRAY_SIZE(x)	sizeof(x) / sizeof(x[0])

/****************************** Type Definitions ******************************/

typedef struct {
	u16 DeviceId; /**< Unique ID of PCIe IP */
	u64 BrigReg;  /**< Bridge Register base address */
	u64 PciReg;		/**< pcie Register base address */
	u64 Ecam;		/**< Ecam space base address */
	u32	NpMemBaseAddr;		/**< non prefetchable memory base address */
	u64	PMemBaseAddr;		/**< prefetchable memory base address */
	u32	NpMemMaxAddr;	/**< non prefetchable memory max base address*/
	u64	PMemMaxAddr;	/**< prefetchable memory max base address */
    u32 DmaBaseAddr;	/**< DMA base address */
	u8	PcieMode;		/**< pcie mode rc or endpoint */
} XPciePsu_Config;

typedef struct {
	XPciePsu_Config Config; /**< Configuration data */
	u32 IsReady;		/**< Is IP been initialized and ready */
	u32 MaxSupportedBusNo;		/**< If this is RC IP, Max Number of  Buses */
} XPciePsu;

/***************************** Function Prototypes ****************************/

XPciePsu_Config *XPciePsu_LookupConfig(u16 DeviceId);

u32 XPciePsu_CfgInitialize(XPciePsu *InstancePtr, XPciePsu_Config *CfgPtr,
			   UINTPTR EffectiveBrgAddress);
u8 XPciePsu_EnumerateBus(XPciePsu *InstancePtr);
u8 XPciePsu_ReadConfigSpace(XPciePsu *InstancePtr, u8 Bus, u8 Device,
				    u8 Function, u16 Offset, u32 *DataPtr);
u8 XPciePsu_WriteConfigSpace(XPciePsu *InstancePtr, u8 Bus, u8 Device,
				     u8 Function, u16 Offset, u32 Data);
u32 XPciePsu_ComposeExternalConfigAddress(u8 Bus, u8 Device, u8 Function,
					  u16 Offset);

u8 XPciePsu_HasCapability(XPciePsu *InstancePtr, u8 Bus, u8 Device,
		u8 Function, u8 CapId);
u64 XPciePsu_GetCapability(XPciePsu *InstancePtr, u8 Bus, u8 Device,
		u8 Function, u8 CapId);
u8 XPciePsu_PrintAllCapabilites(XPciePsu *InstancePtr, u8 Bus, u8 Device,
		u8 Function);

#ifdef __cplusplus
}
#endif

#endif /* XPCIEPSU_H_ */
