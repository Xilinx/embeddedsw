/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
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
*   - To enable/disable and to report errors (interrupts).
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
*   the tools. For some operating systems the config structure will be
*   initialized by the software and this call is not needed.
*
*   - XPciePsu_CfgInitialize(InstancePtr, CfgPtr, EffectiveAddr) - Uses a
*   configuration structure provided by the caller. If running in a system with
*   address translation, the provided virtual memory base address replaces the
*   physical address present in the configuration structure.
*
* <b>Interrupt Management</b>
*
* The PciePsu driver provides interrupt management functions. It allows
* the caller to enable/disable each individual interrupt as well as get/clear
* pending interrupts. Implementation of callback handlers is left to the user.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 0.1	bs	08/21/2018	First release
* </pre>
*
*******************************************************************************/
#ifndef SRC_XPCIEPSU_H_
#define SRC_XPCIEPSU_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************** Include Files *******************************/
#include "xil_types.h"
#include "xstatus.h"
#include "xil_io.h"
#include "xpciepsu_hw.h"
#include "xpciepsu_ht.h"

/**************************** Constant Definitions ****************************/

/******************** Macros (Inline Functions) Definitions *******************/

/****************************** Type Definitions ******************************/

typedef struct {
	u16 DeviceId; /**< Unique ID of PCIe IP */
	u64 BrigReg;  /**< Bridge Register base address */
	u64 PciReg;		/**< pcie Register base address */
	u64 Ecam;		/**< Ecam space base address */
	u32 NP_mem;		/**< non prefetchable memory base address */
	u64 P_mem;		/**< prefetchable memory base address */
	u32 NP_mem_max;	/**< non prefetchable memory max base address*/
	u64 P_mem_max;	/**< prefetchable memory max base address */
} XPciePsu_Config;

typedef struct {
	XPciePsu_Config Config; /**< Configuration data */
	u32 IsReady;		/**< Is IP been initialized and ready */
	u32 last_busno;		/**< If this is RC IP, Max Number of  Buses */
} XPciePsu;

/***************************** Function Prototypes ****************************/

XPciePsu_Config *XPciePsu_LookupConfig(u16 DeviceId);

u32 XPciePsu_CfgInitialize(XPciePsu *InstancePtr, XPciePsu_Config *CfgPtr,
			   UINTPTR EffectiveBrgAddress);
void XPciePsuEnumerate_Fabric(XPciePsu *InstancePtr);
void XPciePsu_ReadRemoteConfigSpace(XPciePsu *InstancePtr, u8 Bus, u8 Device,
				    u8 Function, u16 Offset, u32 *DataPtr);
void XPciePsu_WriteRemoteConfigSpace(XPciePsu *InstancePtr, u8 Bus, u8 Device,
				     u8 Function, u16 Offset, u32 Data);
u32 XPciePsu_ComposeExternalConfigAddress(u8 Bus, u8 Device, u8 Function,
					  u16 Offset);

u32 XPciePsu_getBaseCapabilityPtr(XPciePsu *InstancePtr, u8 Bus, u8 Device,
		u8 Function);
u32 XPciePsu_hasCapability(XPciePsu *InstancePtr, u8 Bus, u8 Device,
		u8 Function, u8 capId);
u32 XPciePsu_getCapabilityFor(XPciePsu *InstancePtr, u8 Bus, u8 Device,
		u8 Function, u8 capId);
void XPciePsu_listAllCapabilites(XPciePsu *InstancePtr, u8 Bus, u8 Device,
		u8 Function);

#ifdef __cplusplus
}
#endif

#endif /* SRC_XPCIEPSU_H_ */
