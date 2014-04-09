/****************************************************************************
*
* (c) Copyright 2011-13 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
*****************************************************************************/
/****************************************************************************/
/**
*
* @file xaxipcie.h
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
* The AXI PCIE IP supports only the endpoint for Virtex®-6 and Spartan®-6
* families.
*
* The AXI PCIE IP supports both the endpoint and Root Port for the Kintex® 7
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
	u32 BaseAddress;		/**< Register base address */
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
							u32 EffectiveAddress);
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

