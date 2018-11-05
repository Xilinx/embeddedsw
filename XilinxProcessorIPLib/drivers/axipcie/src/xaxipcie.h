/******************************************************************************
*
* Copyright (C) 2011 - 2014 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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
******************************************************************************/
/****************************************************************************/
/**
*
* @file xaxipcie.h
* @addtogroup axipcie_v3_1
* @{
* @details
*
* This file contains the software API definition of the Xilinx AXI PCIe IP
* (XAxiPcie). This driver provides "C" function interface to application/upper
* layer to access the hardware.
*
* <b>Features</b>
* The driver provides its user with entry points
*   - To initialize and configure itself and the hardware
*   - To access PCIe configuration space locally
*   - To enable/disable and to report errors (interrupts).
*
* <b>IP Hardware Configuration</b>
* The AXI PCIE IP supports only the endpoint for Virtex�-6 and Spartan�-6
* families.
*
* The AXI PCIE IP supports both the endpoint and Root Port for the Kintex� 7
* devices.
*
*
* <b>Driver Initialization & Configuration</b>
*
* The XAxiPcie_Config structure is used by the driver to configure itself. This
* configuration structure is typically created by the tool-chain based on HW
* build properties.
*
* To support multiple runtime loading and initialization strategies employed
* by various operating systems, the driver instance can be initialized in the
* following way:
*
*   - XAxiPcie_LookupConfig(DeviceId) - Use the device identifier to find the
*   static configuration structure defined in xaxipcie_g.c. This is setup by
*   the tools. For some operating systems the config structure will be
*   initialized by the software and this call is not needed.
*
*   - XAxiPcie_CfgInitialize(InstancePtr, CfgPtr, EffectiveAddr) - Uses a
*   configuration structure provided by the caller. If running in a system with
*   address translation, the provided virtual memory base address replaces the
*   physical address present in the configuration structure.
*
* <b>Interrupt Management</b>
*
* The XAxiPcie driver provides interrupt management functions. It allows
* the caller to enable/disable each individual interrupt as well as get/clear
* pending interrupts. Implementation of callback handlers is left to the user.
*
*
* @note
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ------------------------------------------------------
* 1.00a rkv  03/03/11 Original code.
* 2.00a nm   10/19/11  Added support of pcie root complex functionality.
*		       Changed these functions
* 		       	-renamed function XAxiPcie_GetRequestId to
*		        XAxiPcie_GetRequesterId
*		       	-added two functions arguments RootPortPtr &
*			ECAMSizePtr to XAxiPcie_GetBridgeInfo API
*		       Added these new API for root complex support
*			- XAxiPcie_GetRootPortStatusCtrl
*			- XAxiPcie_SetRootPortStatusCtrl
*			- XAxiPcie_SetRootPortMSIBase
*			- XAxiPcie_GetRootPortErrFIFOMsg
*			- XAxiPcie_ClearRootPortErrFIFOMsg
*			- XAxiPcie_GetRootPortIntFIFOReg
*			- XAxiPcie_ClearRootPortIntFIFOReg
*			- XAxiPcie_WriteLocalConfigSpace
*			- XAxiPcie_ComposeExternalConfigAddress
*			- XAxiPcie_ReadRemoteConfigSpace
*			- XAxiPcie_WriteRemoteConfigSpace
*
* 2.01a nm   04/01/12  Removed XAxiPcie_SetRequesterId and
*		       XAxiPcie_SetBlPortNumber APIs as these are writing
*		       to Read Only bits for CR638299.
* 2.02a nm   08/01/12  Updated for removing compilation errors with C++,
*		       changed XCOMPONENT_IS_READY to XIL_COMPONENT_IS_READY
*		       Removed the Endian Swap in
*		       XAxiPcie_ReadRemoteConfigSpace and
*		       XAxiPcie_WriteRemoteConfigSpace APIs as the HW
*		       has been fixed and the swapping is not required
*		       in  the driver (CR 657412)
* 2.03a srt  04/13/13  Removed Warnings (CR 705004).
* 2.04a srt  09/06/13  Fixed CR 734175:
*		       C_BASEADDR and C_HIGHADDR configuration parameters are
* 		       renamed to BASEADDR and HIGHADDR in Vivado builds.
*	               Modified the tcl for this change.
* 3.0   adk  19/12/13 Updated as per the New Tcl API's
* 3.1   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
*                     Changed the prototype of XAxiPcie_CfgInitialize API.
*       ms   01/23/17 Added xil_printf statement in main function for all
*                    examples to ensure that "Successfully ran" and "Failed"
*                    strings are available in all examples. This is a fix
*                    for CR-965028.
*       ms   03/17/17 Added readme.txt file in examples folder for doxygen
*                     generation.
*       ms   04/05/17 Added tabspace for return statements in functions
*                     of axipcie examples for proper documentation while
*                     generating doxygen.
*
* </pre>
*
*****************************************************************************/
#ifndef XAXIPCIE_H			/* prevent circular inclusions */
#define XAXIPCIE_H			/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/
#include "xil_assert.h"
#include "xstatus.h"
#include "xaxipcie_hw.h"
#include "xil_types.h"

#include <string.h>
#include "xil_cache.h"


/************************** Constant Definitions ****************************/

/*
 * To figure out if PCIe IP
 * is configured as an end
 * point or as a root complex
 */
#define XAXIPCIE_IS_RC		0x01

/*
 * 4KB alignment
 */
#define ALIGN_4KB		0xFFFFF000

/*
 * Version Specific Enhanced Capability register numbers.
 */
#define XAXIPCIE_VSEC1		0x00 /**< First VSEC Register */
#define XAXIPCIE_VSEC2		0x01 /**< Second VSEC Register */

/**************************** Type Definitions ******************************/

/**
 * This typedef contains IP hardware configuration information.
 */

typedef  struct {
	u16 DeviceId;			/**< Unique ID of PCIe IP */
	UINTPTR BaseAddress;		/**< Register base address */
	u8  LocalBarsNum;		/* The number of local bus (AXI) BARs
					 * in hardware
					 */
	u8  IncludeBarOffsetReg;	/**<Are BAR Offset registers built in
					 * hardware
					 */
	u8  IncludeRootComplex;		/**< Is IP built as root complex */
} XAxiPcie_Config;

/**
 * The XAxiPcie driver instance data. The user is required to allocate a
 * variable of this type for every PCIe device in the system that will be
 * using this API. A pointer to a variable of this type is passed to the driver
 * API functions defined here.
 */

typedef struct {
	XAxiPcie_Config Config;		/**< Configuration data */
	u32 IsReady;			/**< Is IP been initialized and ready */
	u32 MaxNumOfBuses;		/**< If this is RC IP, Max Number of
					 * Buses */
} XAxiPcie;


/**
 * The user is required to use this strucuture when reading or writing
 * translation vector between local bus BARs and AXI PCIe BARs. It is used
 * when calling "XAxiPcie_GetLocalBusBar2PcieBar" and
 * "XAxiPcie_SetLocalBusBar2PcieBar" functions. The translation vectors are
 * 64 bits wide even though they might only use the lower 32 bits
 */
typedef struct {
	u32 LowerAddr;		/**< Lower 32 bits of translation value */
	u32 UpperAddr;		/**< Upper 32 bits of translation value */
} XAxiPcie_BarAddr;

/***************** Macros (Inline Functions) Definitions ********************/

#ifndef XAxiPcie_GetRequestId
#define XAxiPcie_GetRequestId XAxiPcie_GetRequesterId
#endif

/****************************************************************************/
/**
* Check whether link is up or not.
*
* @param	InstancePtr is the XAxiPcie instance to operate on.
*
* @return
*		- TRUE if link is up
*		- FALSE if link is down
*
* @note		None
*
*****************************************************************************/
#define	 XAxiPcie_IsLinkUp(InstancePtr) 	\
	(XAxiPcie_ReadReg((InstancePtr)->Config.BaseAddress, 	\
	XAXIPCIE_PHYSC_OFFSET) & XAXIPCIE_PHYSC_LINK_UP_MASK) ? TRUE : FALSE


/****************************************************************************/
/**
* Check whether ECAM is busy or not.
*
* @param	InstancePtr is the XAxiPcie instance to operate on.
*
* @return
*		- TRUE if ECAM is busy
*		- FALSE if ECAM is idel
*
* @note		This function is valid only when IP is configured as a
*		root complex
*
*****************************************************************************/
#define	 XAxiPcie_IsEcamBusy(InstancePtr) 	\
	(XAxiPcie_ReadReg((InstancePtr)->Config.BaseAddress, 	\
	XAXIPCIE_BSC_OFFSET) & XAXIPCIE_BSC_ECAM_BUSY_MASK) ? TRUE : FALSE

/************************** Function Prototypes *****************************/

/*
 * API exported by the driver to the upper layer
 *
 * The following are the driver interface entry points.
 *
 */

/*
 * Config Look Up Function.
 * This API is implemented in xaxipcie_sinit.c
 */

XAxiPcie_Config * XAxiPcie_LookupConfig(u16 DeviceId);

/*
 * PCIe Setup and Configuration Functions.
 * This API is implemented in xaxipcie.c
 */

/* Config Initialization */
int XAxiPcie_CfgInitialize(XAxiPcie * InstancePtr, XAxiPcie_Config * CfgPtr,
							UINTPTR EffectiveAddress);
void XAxiPcie_GetVsecCapability(XAxiPcie *InstancePtr, u8 VsecNum,
			u16 *VsecIdPtr, u8 *VersionPtr, u16 *NextCapPtr);
void XAxiPcie_GetVsecHeader(XAxiPcie *InstancePtr, u8 VsecNum, u16 *VsecIdPtr,
				u8 *RevisionPtr, u16 *LengthPtr);
void XAxiPcie_GetBridgeInfo(XAxiPcie *InstancePtr, u8 *Gen2Ptr,
					u8 *RootPortPtr, u8 *ECAMSizePtr);
void XAxiPcie_GetRequesterId(XAxiPcie *InstancePtr, u8 *BusNumPtr,
				u8 *DevNumPtr, u8 *FunNumPtr, u8 *PortNumPtr);
void XAxiPcie_GetPhyStatusCtrl(XAxiPcie *InstancePtr, u32 *PhyState);
void XAxiPcie_GetRootPortStatusCtrl(XAxiPcie *InstancePtr, u32 *StatusPtr);
void XAxiPcie_SetRootPortStatusCtrl(XAxiPcie *InstancePtr, u32 StatusData);
int XAxiPcie_SetRootPortMSIBase(XAxiPcie *InstancePtr,
						unsigned long long MsiBase);
void XAxiPcie_GetRootPortErrFIFOMsg(XAxiPcie *InstancePtr, u16 *ReqIdPtr,
					u8 *ErrType, u8 *ErrValid);
void XAxiPcie_ClearRootPortErrFIFOMsg(XAxiPcie *InstancePtr);
int XAxiPcie_GetRootPortIntFIFOReg(XAxiPcie *InstancePtr, u16 *ReqIdPtr,
	u16 *MsiAddr, u8 *MsiInt, u8 *IntValid, u16 *MsiMsgData);
void XAxiPcie_ClearRootPortIntFIFOReg(XAxiPcie *InstancePtr);
void XAxiPcie_GetLocalBusBar2PcieBar(XAxiPcie *InstancePtr, u8 BarNumber,
						XAxiPcie_BarAddr *BarAddrPtr);
void XAxiPcie_SetLocalBusBar2PcieBar(XAxiPcie *InstancePtr, u8 BarNumber,
						XAxiPcie_BarAddr *BarAddrPtr);
void XAxiPcie_ReadLocalConfigSpace(XAxiPcie *InstancePtr, u16 Offset,
								u32 *DataPtr);
void XAxiPcie_WriteLocalConfigSpace(XAxiPcie *InstancePtr, u16 Offset,
								u32 Data);
void XAxiPcie_ReadRemoteConfigSpace(XAxiPcie *InstancePtr, u8 Bus, u8 Device,
		u8 Function, u16 Offset, u32 *DataPtr);
void XAxiPcie_WriteRemoteConfigSpace(XAxiPcie *InstancePtr, u8 Bus, u8 Device,
					u8 Function, u16 Offset, u32 Data);
/*
 * Interrupt Functions.
 * This API is implemented in xaxipcie_intr.c
 */
void XAxiPcie_EnableGlobalInterrupt(XAxiPcie *InstancePtr);
void XAxiPcie_DisableGlobalInterrupt(XAxiPcie *InstancePtr);
void XAxiPcie_EnableInterrupts(XAxiPcie *InstancePtr, u32 EnableMask);
void XAxiPcie_DisableInterrupts(XAxiPcie *InstancePtr, u32 DisableMask);
void XAxiPcie_GetEnabledInterrupts(XAxiPcie *InstancePtr, u32 *EnabledMaskPtr);
void XAxiPcie_GetPendingInterrupts(XAxiPcie *InstancePtr, u32 *PendingMaskPtr);
void XAxiPcie_ClearPendingInterrupts(XAxiPcie *InstancePtr, u32 ClearMask);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */

/** @} */
