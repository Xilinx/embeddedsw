/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsrio.h
* @addtogroup srio Overview
* @{
* @details
*
* This file contains the implementation of the SRIO Gen2 driver.
* User documentation for the driver functions is contained in this file in the
* form of comment blocks at the front of each function.

* The SRIO Gen2 Core supports RapidIO Interconnect Specification rev. 2.2
* The SRIO Gen2 Endpoint comprises of the phy ,logical and transport and buffer
* layers. Using the SRIO Gen2 Endpoint Core we can generate I/O transactions
* Read(NREAD), Write(NWRITE), Read with response (NREAD_R), Stream write(SWRITE)
* atomic operations(atomic set,clear,test and swap etc...). It also supports
* Messaging Transactions Message (MESSAGE), Doorbell(DOORBELL)and
* 8-bit/16-bit device ID's.
*
* <b>Initialization & Configuration</b>
*
* The XSrio_Config structure is used by the driver to configure itself.
* This configuration structure is typically created by the tool-chain based
* on HW build properties.
*
* To support multiple runtime loading and initialization strategies employed
* by various operating systems, the driver instance can be initialized in the
* following way:
*
* - XSrio_LookupConfig(DeviceId) - Use the device identifier to find the
*   static configuration structure defined in xsrio_g.c. This is setup
*   by the tools. For some operating systems the config structure will be
*   initialized by the software and this call is not needed.
*
* - XSrio_CfgInitialize(InstancePtr, CfgPtr, EffectiveAddr) - Uses a
*   configuration structure provided by the caller. If running in a system
*   with address translation, the provided virtual memory base address
*   replaces the physical address present in the configuration structure.
*
* <b>Interrupts</b>
* There are no interrupts available for the SRIO Gen2 Core.
*
* <b> Examples </b>
*
* There is an example provided to show the usage of the APIs
* - SRIO Dma loopback example (xsrio_dma_loopback_example.c)
*
* <b> Asserts </b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining, at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that users leave asserts on during development.
*
* <b>RTOS Independence</b>
*
* This driver is intended to be RTOS and processor independent.  It works with
* physical addresses only.  Any needs for dynamic memory management, threads or
* thread mutual exclusion, virtual memory, or cache control must be satisfied
* by the layer above this driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   adk  16/04/14 Initial release.
* 1.1   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
*                     Changed the prototype of XSrio_CfgInitialize API.
*       ms   01/23/17 Modified xil_printf statement in main function for all
*                     examples to ensure that "Successfully ran" and "Failed"
*                     strings are available in all examples. This is a fix
*                     for CR-965028.
*       ms   03/17/17 Added readme.txt file in examples folder for doxygen
*                     generation.
*       ms   04/05/17 Added tabspace for return statements in functions of
*                     srio examples for proper documentation while
*                     generating doxygen.
* 1.2   adk  30/07/19 Fix portwidth handling in the XSrio_CfgInitialize() API.
* 1.4   mus  09/02/20 Updated makefile to support parallel make and
*                     incremental builds. It would help to reduce compilation
*                     time
* </pre>
******************************************************************************/

#ifndef XSRIO_H          /* prevent circular inclusions */
#define XSRIO_H          /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include <string.h>
#include "xstatus.h"
#include "xil_assert.h"
#include "xsrio_hw.h"

/************************** Constant Definitions *****************************/

/* Processing Element SRIO Gen2 PE Features Flags */
#define XSRIO_IS_MEMORY  	1 /**< Core has physically addressable 
				   * storage space 
				   */
#define XSRIO_IS_PROCESSOR 	2 /**< Core has a local processor that
				   * runs code
				   */
#define XSRIO_IS_BRIDGE    	3 /**< Core can be used as a bridge to 
				   * another interface
				   */

/* Processing Element(SRIO Gen2 Core) Operating Mode Flags */
#define XSRIO_OP_MODE_NREAD		0   /**< Core supports read Operation */
#define XSRIO_OP_MODE_NWRITE  		1   /**< Core supports write Operation */
#define XSRIO_OP_MODE_SWRITE		2   /**< Core supports streaming-write
				             * Operation
				             */
#define XSRIO_OP_MODE_NWRITE_R		3   /**< Core supports write with 
				             * Response operation
				             */
#define XSRIO_OP_MODE_DATA_MESSAGE	4   /**< Core supports data message
					     * Operation
					     */
#define XSRIO_OP_MODE_DOORBELL		5   /**< Core supports doorbell
					     * Operation
					     */
#define XSRIO_OP_MODE_ATOMIC		6   /**< Core supports atomic
					     * Operation
					     */

/* Processing Element(SRIO Gen2 Core) Port State Flags */
#define XSRIO_PORT_OK   	  0  /**< Port is initialized */
#define XSRIO_PORT_UNINITIALIZED  1  /**< Port is uninitialized */
#define XSRIO_PORT_HAS_ERRORS	  2  /**< Port has errors */

/* Processing Element(SRIO Gen2 Core) Direction Flags */
#define XSRIO_DIR_TX		1 /**< Transmit Direction Flag */
#define XSRIO_DIR_RX		2 /**< Receive Direction Flag */

/************************** Type Definitions *****************************/

/**
 * This typedef contains the configuration information for the device.
 */
typedef struct XSrio_Config {
	u16 DeviceId;		/**< Device Id */
	UINTPTR BaseAddress;	/**< Base Address */
} XSrio_Config;

/**
 * The XSrio driver instance data. An instance must be allocated for
 * each SRIO device in use.
 */
typedef struct XSrio {
	XSrio_Config Config;   /**< Config Structure */
	int IsReady;	       /**< Device is initialized and ready */
	int PortWidth;	       /**< Serial lane Port width (1x or 2x or 4x) */
} XSrio;


/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
*
* XSrio_ReadDeviceVendorID returns the Device Vendor Id of the core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	Device Vendor ID of the core.
*
* @note         C-style signature:
*               u16 XSrio_ReadDeviceVendorID(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_ReadDeviceVendorID(InstancePtr)	\
	(XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
		       XSRIO_DEV_ID_CAR_OFFSET) & XSRIO_DEV_ID_VDRID_CAR_MASK)

/****************************************************************************/
/**
*
* XSrio_ReadDeviceID returns the Device Id of the core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	Device ID of the core.
*
* @note         C-style signature:
*               u16 XSrio_ReadDeviceID(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_ReadDeviceID(InstancePtr)	\
	((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_DEV_ID_CAR_OFFSET) & XSRIO_DEV_ID_DEVID_CAR_MASK) >>  \
	 XSRIO_DEV_ID_DEVID_CAR_SHIFT)

/****************************************************************************/
/**
*
* XSrio_ReadAsmVendorID returns the Assembly Vendor Id of the core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	Assembly Vendor ID of the core.
*
* @note         C-style signature:
*               u16 XSrio_ReadAsmVendorID(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_ReadAsmVendorID(InstancePtr)	\
	(XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
		       XSRIO_ASM_ID_CAR_OFFSET) & XSRIO_ASM_ID_CAR_ASMVID_MASK)

/****************************************************************************/
/**
*
* XSrio_ReadAsmID returns the Assembly Id of the SRIO Gen2 core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	Assembly ID of the core.
*
* @note         C-style signature:
*               u16 XSrio_ReadAsmID(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_ReadAsmID(InstancePtr)	\
	((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_ASM_ID_CAR_OFFSET) & XSRIO_ASM_ID_CAR_ASMID_MASK) >> \
	 XSRIO_ASM_ID_CAR_ASMID_SHIFT)

/****************************************************************************/
/**
*
* XSrio_GetExFeaturesPointer gives the pointer to the Phy Register space of
* the SRIO Gen2 core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	Pointer to the Phy Register space of the core.
*
* @note         C-style signature:
*               u16 XSrio_GetExFeaturesPointer(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_GetExFeaturesPointer(InstancePtr)	\
	(XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
		       XSRIO_ASM_INFO_CAR_OFFSET) & XSRIO_ASM_INFO_CAR_EFP_MASK)

/****************************************************************************/
/**
*
* XSrio_ReadAsmRevision returns the Assembly Revision value of the core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	Assembly revision of the core.
*
* @note         C-style signature:
*               u16 XSrio_ReadAsmRevision(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_ReadAsmRevision(InstancePtr)	\
	((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_ASM_INFO_CAR_OFFSET) & XSRIO_ASM_INFO_CAR_ASMREV_MASK) >> \
	 XSRIO_ASM_INFO_CAR_ASMREV_SHIFT)

/****************************************************************************/
/**
*
* XSrio_IsLargeSystem checks whether PE(Processing Element) supports a large
* system (16-bit Device ids)
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return
*		- TRUE If the System Supports 16-bit Devices.
*		- FALSE If the System Supports 8-bit Devices.
*
* @note         C-style signature:
*               u8 XSrio_IsLargeSystem(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_IsLargeSystem(InstancePtr)	\
	((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_PEF_CAR_OFFSET) & XSRIO_PEF_CAR_CTS_MASK) ?  \
	 TRUE : FALSE)

/****************************************************************************/
/**
*
* XSrio_IsCRFSupported checks whether the PE(Processing Element) supports
* CRF(Critical Request Flow indicator).
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return
*		- TRUE If the System Supports CRF.
*		- FALSE If the System Wont Support CRF.
*
* @note         C-style signature:
*               u8 XSrio_IsCRFSupported(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_IsCRFSupported(InstancePtr)	\
	((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_PEF_CAR_OFFSET) & XSRIO_PEF_CAR_CRF_MASK) ?  \
	 TRUE : FALSE)

/****************************************************************************/
/**
*
* XSrio_ReadSrcOps returns the Source Operations CAR  Register contents.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	Contents of the Source Operations CAR Register.
*
* @note         C-style signature:
*               u32 XSrio_ReadSrcOps(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_ReadSrcOps(InstancePtr)	\
	XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
		      XSRIO_SRC_OPS_CAR_OFFSET)

/****************************************************************************/
/**
*
* XSrio_ReadDstOps returns the Destination Operations CAR Register contents.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	Contents of the Destination Operations CAR Register.
*
* @note         C-style signature:
*               u32 XSrio_ReadDstOps(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_ReadDstOps(InstancePtr)	\
	XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
		      XSRIO_DST_OPS_CAR_OFFSET)

/****************************************************************************/
/**
*
* XSrio_GetLCSBA returns the Local Configuration Space Base Address(LCSBA) of
* the SRIO Gen2 core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	Contents of Local Configuration Space Base Address Register.
*
* @note         C-style signature:
*               u32 XSrio_GetLCSBA(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_GetLCSBA(InstancePtr)	\
	((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_LCS1_BASEADDR_CSR_OFFSET) & \
	  XSRIO_LCS1_BASEADDR_LCSBA_CSR_MASK) >>  \
	 XSRIO_LCS1_BASEADDR_LCSBA_CSR_SHIFT)

/****************************************************************************/
/**
*
* XSrio_SetLCSBA Configures the Local Configuration Space Base Address of
* the SRIO Gen2 core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
* @param	Value is the Local Configuration Space Base Address that
*		needs to be configured.
*
* @return	None.
*
* @note         C-style signature:
*               void XSrio_SetLCSBA(XSrio *InstancePtr, u32 Value)
*
*****************************************************************************/
#define XSrio_SetLCSBA(InstancePtr, Value)			\
	(XSrio_WriteReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_LCS1_BASEADDR_CSR_OFFSET,  			\
			((Value << XSRIO_LCS1_BASEADDR_LCSBA_CSR_SHIFT) 	\
			 & XSRIO_LCS1_BASEADDR_LCSBA_CSR_MASK)))

/****************************************************************************/
/**
*
* XSrio_GetLargeBaseDeviceID returns the 16-bit Device Id for an endpoint in a
* Large transport system.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	16-bit Device Id.
*
* @note         C-style signature:
*               u16 XSrio_GetLargeBaseDeviceID(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_GetLargeBaseDeviceID(InstancePtr)		\
	(XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
		       XSRIO_BASE_DID_CSR_OFFSET) & \
	 XSRIO_BASE_DID_CSR_LBDID_MASK)

/****************************************************************************/
/**
*
* XSrio_SetLargeBaseDeviceID configures the 16-bit Device Id for an endpoint in
* a Large transport system.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
* @param 	DeviceId is the Device ID that needs to be configured.
*
* @return	None.
*
* @note         C-style signature:
*               void XSrio_SetLargeBaseDeviceID(XSrio *InstancePtr, u16 DeviceId)
*
*****************************************************************************/
#define XSrio_SetLargeBaseDeviceID(InstancePtr, DeviceId)		\
	(XSrio_WriteReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_BASE_DID_CSR_OFFSET, 	     		\
			((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
					XSRIO_BASE_DID_CSR_OFFSET) & XSRIO_BASE_DID_CSR_BDID_MASK)  | \
			 (DeviceId & XSRIO_BASE_DID_CSR_LBDID_MASK))))

/****************************************************************************/
/**
*
* XSrio_GetBaseDeviceID returns the 8-bit Device Id for an endpoint in a small
* Transport system.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	8-bit Device Id.
*
* @note         C-style signature:
*               u8 XSrio_GetBaseDeviceID(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_GetBaseDeviceID(InstancePtr)	\
	((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_BASE_DID_CSR_OFFSET) & \
	  XSRIO_BASE_DID_CSR_BDID_MASK) >> XSRIO_BASE_DID_CSR_BDID_SHIFT)

/****************************************************************************/
/**
*
* XSrio_SetBaseDeviceID configures the 8-bit Device Id for an endpoint in a
* small transport system.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
* @param 	DeviceId is the Device ID that needs to be configured.
*
* @return	None.
*
* @note         C-style signature:
*               void XSrio_SetBaseDeviceID(XSrio *InstancePtr, u8 DeviceId)
*
*****************************************************************************/
#define XSrio_SetBaseDeviceID(InstancePtr, DeviceId)	\
	(XSrio_WriteReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_BASE_DID_CSR_OFFSET,  \
			((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
					XSRIO_BASE_DID_CSR_OFFSET) & XSRIO_BASE_DID_CSR_LBDID_MASK) \
			 |((DeviceId << XSRIO_BASE_DID_CSR_BDID_SHIFT) \
			   & XSRIO_BASE_DID_CSR_BDID_MASK))))

/****************************************************************************/
/**
*
* XSrio_GetHostBaseDevID_LockCSR returns the Device Id of the system host.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	Device Id of the system host.
*
* @note         C-style signature:
*               u16 XSrio_GetHostBaseDevID_LockCSR(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_GetHostBaseDevID_LockCSR(InstancePtr)	\
	(XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
		       XSRIO_HOST_DID_LOCK_CSR_OFFSET) & \
	 XSRIO_HOST_DID_LOCK_CSR_HBDID_MASK)

/****************************************************************************/
/**
*
* XSrio_SetHostBaseDevID_LockCSR configures the Host Base Device Id of
* the SRIO gen2 Core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
* @param 	DeviceId is the Device ID that needs to be configured.
*
* @return	None.
*
* @note         C-style signature:
*               void XSrio_SetHostBaseDevID_LockCSR(XSrio *InstancePtr,
*						    u16 DeviceId)
*
*****************************************************************************/
#define XSrio_SetHostBaseDevID_LockCSR(InstancePtr, DeviceId)	\
	(XSrio_WriteReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_HOST_DID_LOCK_CSR_OFFSET, 	     \
			(DeviceId & XSRIO_HOST_DID_LOCK_CSR_HBDID_MASK)))

/****************************************************************************/
/**
*
* XSrio_GetComponentTagCSR returns the Component Tag Value set by the software
* during initialization.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	Component Tag Value.
*
* @note         C-style signature:
*               u32 XSrio_GetComponentTagCSR(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_GetComponentTagCSR(InstancePtr)	\
	(XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
		       XSRIO_COMPONENT_TAG_CSR_OFFSET))

/****************************************************************************/
/**
*
* XSrio_SetComponentTagCSR sets the Component Tag Value for SRIO Gen2 core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
* @param 	Value is the Component Tag Value to be set.
*
* @return	None.
*
* @note         C-style signature:
*               void XSrio_SetComponentTagCSR(XSrio *InstancePtr, u32 Value)
*
*****************************************************************************/
#define XSrio_SetComponentTagCSR(InstancePtr, Value)	\
	(XSrio_WriteReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_COMPONENT_TAG_CSR_OFFSET, 	     \
			Value))

/****************************************************************************/
/**
*
* XSrio_GetExtFeaturesID returns the Extended Features Id value.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	Extended Features ID Value.
*
* @note         C-style signature:
*               u16 XSrio_GetExtFeaturesID(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_GetExtFeaturesID(InstancePtr)	\
	(XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
		       XSRIO_EFB_HEADER_OFFSET) & \
	 XSRIO_EFB_HEADER_EFID_MASK)

/****************************************************************************/
/**
*
* XSrio_GetSerialExtFeaturesPointer returns the Extended Features Pointer which
* will point to the next extended features block if one exists.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	Extended Features Pointer Address.
*
* @note         C-style signature:
*               u16 XSrio_GetSerialExtFeaturesPointer(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_GetSerialExtFeaturesPointer(InstancePtr)	\
	((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_EFB_HEADER_OFFSET) & \
	  XSRIO_EFB_HEADER_EFP_MASK) >> XSRIO_EFB_HEADER_EFP_SHIFT)

/****************************************************************************/
/**
*
* XSrio_GetPortLinkTimeOutValue returns the Port Link Timeout value for the
* SRIO Gen2 core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	Port link Timeout Value.
*
* @note         C-style signature:
*               u32 XSrio_GetPortLinkTimeOutValue(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_GetPortLinkTimeOutValue(InstancePtr)	\
	((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_PORT_LINK_TOUT_CSR_OFFSET   ) & \
	  XSRIO_PORT_LINK_TOUT_CSR_TOUTVAL_MASK) >>   \
	 XSRIO_PORT_LINK_TOUT_CSR_TOUTVAL_SHIFT)

/****************************************************************************/
/**
*
* XSrio_SetPortLinkTimeOutValue sets the Port Link Timeout value for the
* SRIO Gen2 core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
* @param 	Value is the Port Link Timeout value to be set.
*
* @return	None.
*
* @note         C-style signature:
*               void  XSrio_SetPortLinkTimeOutValue(XSrio *InstancePtr,
*						     u16 Value)
*
*****************************************************************************/
#define XSrio_SetPortLinkTimeOutValue(InstancePtr, Value)	\
	(XSrio_WriteReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_PORT_LINK_TOUT_CSR_OFFSET   , 	     \
			(Value << XSRIO_PORT_LINK_TOUT_CSR_TOUTVAL_SHIFT) &  \
			XSRIO_PORT_LINK_TOUT_CSR_TOUTVAL_MASK))

/****************************************************************************/
/**
*
* XSrio_GetPortRespTimeOutValue returns the Port Response Timeout value for the
* the SRIO Gen2 core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	Port Response Timeout value.
*
* @note         C-style signature:
*               u32 XSrio_GetPortRespTimeOutValue(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_GetPortRespTimeOutValue(InstancePtr)	\
	((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_PORT_RESP_TOUT_CSR_OFFSET) & \
	  XSRIO_PORT_RESP_TOUT_CSR_TOUTVAL_MASK) >>  \
	 XSRIO_PORT_RESP_TOUT_CSR_TOUTVAL_SHIFT)

/****************************************************************************/
/**
*
* XSrio_SetPortRespTimeOutValue sets the Port Response Timeout value for the
* The SRIO Gen2 core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
* @param 	Value is the Port Response Timeout to be set.
*
* @return	None.
*
* @note         C-style signature:
*               void XSrio_SetPortRespTimeOutValue(XSrio *InstancePtr,
*						    u16 Value)
*
*****************************************************************************/
#define XSrio_SetPortRespTimeOutValue(InstancePtr, Value)	\
	(XSrio_WriteReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_PORT_RESP_TOUT_CSR_OFFSET, 	     \
			(Value << XSRIO_PORT_RESP_TOUT_CSR_TOUTVAL_SHIFT) &  \
			XSRIO_PORT_RESP_TOUT_CSR_TOUTVAL_MASK))

/****************************************************************************/
/**
*
* XSrio_IsPEDiscovered checks whether the PE(Processing Element) is discovered
* or not.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return
*		- TRUE If the PE is Discovered.
*		- FALSE If the PE is not Discovered.
*
* @note         C-style signature:
*               u8 XSrio_IsPEDiscovered(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_IsPEDiscovered(InstancePtr)	\
	((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_PORT_GEN_CTL_CSR_OFFSET) & \
	  XSRIO_PORT_GEN_CTL_CSR_DISCOVERED_MASK) ?  TRUE : FALSE)

/****************************************************************************/
/**
*
* XSrio_SetDiscovered configures the device as Discovered so that it is
* responsible for system exploration.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	None.
*
* @note         C-style signature:
*               void XSrio_SetDiscovered(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_SetDiscovered(InstancePtr)	\
	(XSrio_WriteReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_PORT_GEN_CTL_CSR_OFFSET, 	     \
			(XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
				       XSRIO_PORT_GEN_CTL_CSR_OFFSET) 	\
			 | XSRIO_PORT_GEN_CTL_CSR_DISCOVERED_MASK)))

/****************************************************************************/
/**
*
* XSrio_IsMasterEnabled checks whether PE(Processing Element) is allowed to
* issue request into the system.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return
*		- TRUE If the Master Enable bit is set.
*		- FALSE If the Master Enable bit is not set.
*
* @note         C-style signature:
*               u8 XSrio_IsMasterEnabled(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_IsMasterEnabled(InstancePtr)	\
	((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_PORT_GEN_CTL_CSR_OFFSET) \
	  & XSRIO_PORT_GEN_CTL_CSR_MENABLE_MASK) ?  TRUE : FALSE)

/****************************************************************************/
/**
*
* XSrio_SetMasterEnabled configures the device so that it is allowed to issue
* requests into the system.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	None.
*
* @note         C-style signature:
*               void XSrio_SetMasterEnabled(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_SetMasterEnabled(InstancePtr)	\
	(XSrio_WriteReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_PORT_GEN_CTL_CSR_OFFSET, 	     \
			(XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
				       XSRIO_PORT_GEN_CTL_CSR_OFFSET) 	\
			 | XSRIO_PORT_GEN_CTL_CSR_MENABLE_MASK)))

/****************************************************************************/
/**
*
* XSrio_IsHost checks whether PE(Processing Element) is responsible for
* system exploration.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return
*		- TRUE If the Host bit is set.
*		- FALSE If the Host bit is not set.
*
* @note         C-style signature:
*               u8 XSrio_IsHost(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_IsHost(InstancePtr)	\
	((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_PORT_GEN_CTL_CSR_OFFSET) \
	  & XSRIO_PORT_GEN_CTL_CSR_HOST_MASK) ?  TRUE : FALSE)

/****************************************************************************/
/**
*
* XSrio_SetHostEnabled configures the device to be responsible for system
* exploration.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	None.
*
* @note         C-style signature:
*               void XSrio_SetHostEnabled(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_SetHostEnabled(InstancePtr)	\
	(XSrio_WriteReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_PORT_GEN_CTL_CSR_OFFSET, 	     \
			(XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
				       XSRIO_PORT_GEN_CTL_CSR_OFFSET) 	\
			 | XSRIO_PORT_GEN_CTL_CSR_HOST_MASK)))

/****************************************************************************/
/**
*
* XSrio_GetCommand returns the command value that is sent on the Link-request
* Control symbol of the SRIO Gen2 core. This api is available only if the
* software assisted error recovery option is enabled in the core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	Command information of the link-request control symbol.
*
* @note         C-style signature:
*               u32 XSrio_GetCommand(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_GetCommand(InstancePtr)	\
	(XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
		       XSRIO_PORT_N_MNT_REQ_CSR_OFFSET) & \
	 XSRIO_PORT_N_MNT_REQ_CSR_CMD_MASK)

/****************************************************************************/
/**
*
* XSrio_SendCommand sends the given command in the link-request control symbol
* of the SRIO Gen2 core. This api is available only if the software assisted
* error recovery option is selected in the core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
* @param 	Value is the Command to be send.
*
* @return	None.
*
* @note         C-style signature:
*               void XSrio_SendCommand(XSrio *InstancePtr, u8 Value)
*
*****************************************************************************/
#define XSrio_SendCommand(InstancePtr, Value)	\
	(XSrio_WriteReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_PORT_N_MNT_REQ_CSR_OFFSET, 	     \
			Value & XSRIO_PORT_N_MNT_REQ_CSR_CMD_MASK))

/****************************************************************************/
/**
*
* XSrio_IsResponseValid checks whether the link response is valid or not in
* the SRIO Gen2 Core. This api is available only if the software assisted error
* recovery option is enabled in the core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return
*		- TRUE if the corresponding link request causes a link
*		response.
*		- FALSE if the corresponding link request not causes a link
*		response.
*
* @note         C-style signature:
*               u8 XSrio_IsResponseValid(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_IsResponseValid(InstancePtr)	\
	((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_PORT_N_MNT_RES_CSR_OFFSET) & \
	  XSRIO_PORT_N_MNT_RES_CSR_RVALID_MASK) ? TRUE : FALSE)

/****************************************************************************/
/**
*
* XSrio_GetOutboundAckID returns the value of the next transmitted Ackid of
* the SRIO Gen2 Core. This api is available only if the software assisted error
* recovery option is selected core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	Outbound Ack ID value.
*
* @note         C-style signature:
*               u32 XSrio_GetOutboundAckID(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_GetOutboundAckID(InstancePtr)	\
	(XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
		       XSRIO_PORT_N_ACKID_CSR_OFFSET) & \
	 XSRIO_PORT_N_ACKID_CSR_OBACKID_MASK)

/****************************************************************************/
/**
*
* XSrio_SetOutboundAckID  sets value of the next transmitted Ackid of
* the SRIO Gen2 Core. This api is available only if the software assisted error
* Recovery option is selected in the core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
* @param	Value is the Outbound Ack Id to be set.
*
* @return	None.
*
* @note         C-style signature:
*               void XSrio_SetOutboundAckID(XSrio *InstancePtr, u8 Value)
*
*****************************************************************************/
#define XSrio_SetOutboundAckID(InstancePtr, Value)	\
	(XSrio_WriteReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_PORT_N_ACKID_CSR_OFFSET,  \
			((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
					XSRIO_PORT_N_ACKID_CSR_OFFSET) &	\
			  XSRIO_PORT_N_ACKID_CSR_RESET_OBACKID_MASK) \
			 | (Value & XSRIO_PORT_N_ACKID_CSR_OBACKID_MASK))))

/****************************************************************************/
/**
*
* XSrio_GetInboundAckID returns the expected Ackid of the next received packet
* of the SRIO Gen2 core. This api is available only if the software assisted
* error recovery option is selected in the core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	Inbound Ack ID value.
*
* @note         C-style signature:
*               u32 XSrio_GetInboundAckID(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_GetInboundAckID(InstancePtr)	\
	((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_PORT_N_ACKID_CSR_OFFSET) & \
	  XSRIO_PORT_N_ACKID_CSR_IBACKID_MASK) >>  \
	 XSRIO_PORT_N_ACKID_CSR_IBACKID_SHIFT)

/****************************************************************************/
/**
*
* XSrio_SetInboundAckID sets the value of the next transmitted Ackid of
* the SRIO Gen2 core. This api is available only if the software assisted error
* recovery option is selected in the core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
* @param	Value is the InBound Ack Id to be set.
*
* @return	None.
*
* @note         C-style signature:
*               void XSrio_SetInboundAckID(XSrio *InstancePtr,
*						    u8 Value)
* 	        This api won't work if you call the XSrio_ClrOutStandingAckIDs
*	        before calling this api.
*
*****************************************************************************/
#define XSrio_SetInboundAckID(InstancePtr, Value)	\
	(XSrio_WriteReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_PORT_N_ACKID_CSR_OFFSET, 	     \
			((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
					XSRIO_PORT_N_ACKID_CSR_OFFSET) &  \
			  XSRIO_PORT_N_ACKID_CSR_RESET_IBACKID_MASK)	\
			 | ((Value << XSRIO_PORT_N_ACKID_CSR_IBACKID_SHIFT) &    \
			    XSRIO_PORT_N_ACKID_CSR_IBACKID_MASK))))

/****************************************************************************/
/**
*
* XSrio_ClrOutStandingAckIDs clears all outstanding unacknowledged
* received packets of the SRIO Gen2 core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	None.
*
* @note         C-style signature:
*               void XSrio_ClrOutStandingAckIDs(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_ClrOutStandingAckIDs(InstancePtr)	\
	(XSrio_WriteReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_PORT_N_ACKID_CSR_OFFSET, 	     \
			(XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
				       XSRIO_PORT_N_ACKID_CSR_OFFSET) 	\
			 | XSRIO_PORT_N_ACKID_CSR_CLSACKID_MASK)))

/****************************************************************************/
/**
*
* XSrio_IsEnumerationBoundary checks whether the enumeration boundary is
* available or not for the SRIO Gen2 core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return
*		- TRUE if the EnumerationBoundary Enabled.
*		- FALSE if the EnumerationBoundary is not Enabled.
*
* @note         C-style signature:
*               u8 XSrio_IsEnumerationBoundary(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_IsEnumerationBoundary(InstancePtr)	\
	((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_PORT_N_CTL_CSR_OFFSET) & \
	  XSRIO_PORT_N_CTL_CSR_ENUMB_MASK) ? TRUE : FALSE)

/****************************************************************************/
/**
*
* XSrio_ClrEnumerationBoundary clears the enumeration boundary of the
* SRIO Gen2 core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	None.
*
* @note         C-style signature:
*               void XSrio_ClrEnumerationBoundary(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_ClrEnumerationBoundary(InstancePtr)	\
	(XSrio_WriteReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_PORT_N_CTL_CSR_OFFSET, 	\
			(XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
				       XSRIO_PORT_N_CTL_CSR_OFFSET) \
			 | XSRIO_PORT_N_CTL_CSR_ENUMB_MASK)))

/****************************************************************************/
/**
*
* XSrio_GetPortwidthOverride returns the port width override value of the
* SRIO Gen2 core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	Port Width Override Value.
*
* @note         C-style signature:
*               u8 XSrio_GetPortwidthOverride(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_GetPortwidthOverride(InstancePtr)	\
	((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_PORT_N_CTL_CSR_OFFSET) & \
	  XSRIO_PORT_N_CTL_CSR_PWO_MASK) >> XSRIO_PORT_N_CTL_CSR_PWO_SHIFT)

/****************************************************************************/
/**
*
* XSrio_SetPortwidthOverride configures the port width override value of the
* SRIO Gen2 core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
* @param	Value is the port width override value needs to be set.
*
* @return	None.
*
* @note         C-style signature:
*               void XSrio_SetPortwidthOverride(XSrio *InstancePtr, u8 Value)
*
*****************************************************************************/
#define XSrio_SetPortwidthOverride(InstancePtr, Value)	\
	(XSrio_WriteReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_PORT_N_CTL_CSR_OFFSET, 	\
			((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
					XSRIO_PORT_N_CTL_CSR_OFFSET) &	\
			  XSRIO_PORT_N_CTL_CSR_RESET_PWO_MASK) | \
			 ((Value << XSRIO_PORT_N_CTL_CSR_PWO_SHIFT)  \
			  & XSRIO_PORT_N_CTL_CSR_PWO_MASK))))

/****************************************************************************/
/**
*
* XSrio_GetSerialLaneExtFeaturesPointer returns the extended features pointer
* For the serial lane which will point to the next extended features block
* If one exists.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	Extended Features Pointer Address.
*
* @note         C-style signature:
*               u16 XSrio_GetSerialLaneExtFeaturesPointer(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_GetSerialLaneExtFeaturesPointer(InstancePtr)	\
	((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_EFB_LPSL_OFFSET + XSRIO_SL_HEADER_OFFSET) & \
	  XSRIO_SL_HEADER_EFP_MASK) >> XSRIO_SL_HEADER_EFP_SHIFT)

/****************************************************************************/
/**
*
* XSrio_ClrDecodingErrors clears the 8B/10B decoding errors and return
* Result.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
* @param	Lanenum is the Serial Lane-number(0,1,2,3).
*
* @return	None
*
*
* @note         C-style signature:
*               int XSrio_ClrDecodingErrors(XSrio *InstancePtr, u8 Lanenum)
*
*****************************************************************************/
#define XSrio_ClrDecodingErrors(InstancePtr, Lanenum)	\
	((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_EFB_LPSL_OFFSET + XSRIO_SLS0_CSR_OFFSET(Lanenum)) & \
	  XSRIO_SLS0_CSR_DECODING_ERRORS_MASK)   \
	 >> XSRIO_SLS0_CSR_DECODING_ERRORS_SHIFT)

/****************************************************************************/
/**
*
* XSrio_GetRxSize returns the number of maximum-size packets the rx buffer
* holded.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	Rx buffer size.
*
* @note         C-style signature:
*               u8 XSrio_GetRxSize(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_GetRxSize(InstancePtr)	\
	(XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
		       XSRIO_IMP_BCSR_OFFSET ) & \
	 XSRIO_IMP_BCSR_RXSIZE_MASK)

/****************************************************************************/
/**
*
* XSrio_ForceRxFlowControl forces the Tx flow control enabled core to use
* Rx flow control.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	None.
*
* @note         C-style signature:
*               void XSrio_ForceRxFlowControl(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_ForceRxFlowControl(InstancePtr)	\
	(XSrio_WriteReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_IMP_BCSR_OFFSET, 	\
			(XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
				       XSRIO_IMP_BCSR_OFFSET) | \
			 XSRIO_IMP_BCSR_FRX_FLOW_CNTL_MASK)))

/****************************************************************************/
/**
*
* XSrio_GetTxSize returns the number of maximum-size packets the tx buffer
* holds.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	Tx buffer size.
*
* @note         C-style signature:
*               u8 XSrio_GetTxSize(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_GetTxSize(InstancePtr)	\
	((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_IMP_BCSR_OFFSET ) & \
	  XSRIO_IMP_BCSR_TXSIZE_MASK) >> XSRIO_IMP_BCSR_TXSIZE_SHIFT)

/****************************************************************************/
/**
*
* XSrio_CheckforTxReqreorder checks whether the transmit buffer has been
* configured to allow reordering of requests.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return
* 		- TRUE  If the Tx Request reorder is enabled in the core.
*		- FALSE If the Tx Request reorder is not enabled in the core.
*
* @note         C-style signature:
*               u8 XSrio_CheckforTxReqreorder(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_CheckforTxReqreorder(InstancePtr)	\
	((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_IMP_BCSR_OFFSET) & \
	  XSRIO_IMP_BCSR_TXREQ_REORDER_MASK) ? TRUE : FALSE)

/****************************************************************************/
/**
*
* XSrio_IsTxFlowControl checks whether the BUF is currently operating in
* Tx flow control mode or not.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return
*		- TRUE  If the Tx Flow Control is  enabled in the core.
*		- FALSE If the Tx Flow Control is not enabled in the core.
*
* @note         C-style signature:
*               u8 XSrio_IsTxFlowControl(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_IsTxFlowControl(InstancePtr)	\
	((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_IMP_BCSR_OFFSET) & \
	  XSRIO_IMP_BCSR_TX_FLOW_CNTL_MASK) ? TRUE : FALSE)

/****************************************************************************/
/**
*
* XSrio_GetDestinationID gets the destination id value which will be
* used for outgoing maintenance requests.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	Destination ID value of the outgoing maintenance request.
*
* @note         C-style signature:
*               u8 XSrio_GetDestinationID(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_GetDestinationID(InstancePtr)	\
	(XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
		       XSRIO_IMP_MRIR_OFFSET ) & \
	 XSRIO_IMP_MRIR_REQ_DESTID_MASK)

/****************************************************************************/
/**
*
* XSrio_SetDestinationID sets Device Id which will be used for
* Outgoing maintenance requests.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
* @param	Value is the Device Id value.
*
* @return	None.
*
* @note         C-style signature:
*               void XSrio_SetDestinationID(XSrio *InstancePtr, u8 Value)
*
*****************************************************************************/
#define XSrio_SetDestinationID(InstancePtr, Value)	\
	(XSrio_WriteReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_IMP_MRIR_OFFSET, 	\
			((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
					XSRIO_IMP_MRIR_OFFSET) & ~XSRIO_IMP_MRIR_REQ_DESTID_MASK) | \
			 (Value & XSRIO_IMP_MRIR_REQ_DESTID_MASK))))

/****************************************************************************/
/**
*
* XSrio_GetCRF checks whether the CRF is enabled in the core or not which will
* be used for outgoing maintenance requests.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	CRF Value used for outgoing maintenance requests.
*
* @note         C-style signature:
*               u8 XSrio_GetCRF(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_GetCRF(InstancePtr)	\
	((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_IMP_MRIR_OFFSET) & \
	  XSRIO_IMP_MRIR_REQ_CRF_MASK) >> XSRIO_IMP_MRIR_REQ_CRF_SHIFT)

/****************************************************************************/
/**
*
* XSrio_SetCRF sets CRF value that is used for outgoing maintenance requests.
* This api will work only when the CRF support is enabled in the core
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	None.
*
* @note         C-style signature:
*               void XSrio_SetCRF(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_SetCRF(InstancePtr)	\
	(XSrio_WriteReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_IMP_MRIR_OFFSET, 	\
			(XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
				       XSRIO_IMP_MRIR_OFFSET) | \
			 XSRIO_IMP_MRIR_REQ_CRF_MASK)))

/****************************************************************************/
/**
*
* XSrio_GetPriority priority used for outgoing maintenance requests.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	Priority value.
*
* @note         C-style signature:
*               u8 XSrio_GetPriority(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_GetPriority(InstancePtr)	\
	((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_IMP_MRIR_OFFSET ) & \
	  XSRIO_IMP_MRIR_REQ_PRIO_MASK) >> XSRIO_IMP_MRIR_REQ_PRIO_SHIFT)

/****************************************************************************/
/**
*
* XSrio_SetPriority sets the Priority which will be used for
* outgoing maintenance requests.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
* @param	Value is the Priority value which will used for outgoing
*		maintenance requests.
*
* @return	None.
*
* @note         C-style signature:
*               void XSrio_SetPriority(XSrio *InstancePtr, u8 Value)
*
*****************************************************************************/
#define XSrio_SetPriority(InstancePtr, Value)	\
	(XSrio_WriteReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_IMP_MRIR_OFFSET, 	\
			((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
					XSRIO_IMP_MRIR_OFFSET) & ~XSRIO_IMP_MRIR_REQ_PRIO_MASK) | \
			 ((Value << XSRIO_IMP_MRIR_REQ_PRIO_SHIFT)&  \
			  XSRIO_IMP_MRIR_REQ_PRIO_MASK))))

/****************************************************************************/
/**
*
* XSrio_RequestTID gives the transfer id value which will be used for the
* next outgoing maintenance request. This value will increment after each
* request is sent.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	Transfer ID value.
*
* @note         C-style signature:
*               u8 XSrio_RequestTID(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_RequestTID(InstancePtr)	\
	((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_IMP_MRIR_OFFSET ) & \
	  XSRIO_IMP_MRIR_REQ_TID_MASK) >> XSRIO_IMP_MRIR_REQ_TID_SHIFT)

/****************************************************************************/
/**
*
* XSrio_SetTID sets the transfer id which will be used for the next outgoing
* maintenance request.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
* @param	Value is the transfer id value which of the next outgoing
* 		maintenance request.
*
* @return	None.
*
* @note         C-style signature:
*               void XSrio_SetTID(XSrio *InstancePtr, u8 Value)
*
*****************************************************************************/
#define XSrio_SetTID(InstancePtr, Value)	\
	(XSrio_WriteReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_IMP_MRIR_OFFSET, 	\
			((XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
					XSRIO_IMP_MRIR_OFFSET) & ~XSRIO_IMP_MRIR_REQ_TID_MASK) | \
			 ((Value << XSRIO_IMP_MRIR_REQ_TID_SHIFT)&  \
			  XSRIO_IMP_MRIR_REQ_TID_MASK))))

/****************************************************************************/
/**
*
* XSrio_ClrPortError clears the Port Error specified by the Mask.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
* @param	Mask is the mask for the Port Error to be cleared.
*
* @return	None.
*
* @note         C-style signature:
*               void XSrio_ClrPortError(XSrio *InstancePtrm, u32 Mask)
*
*****************************************************************************/
#define XSrio_ClrPortError(InstancePtr, Mask)	\
	(XSrio_WriteReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_PORT_N_ERR_STS_CSR_OFFSET, 	\
			(XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
				       XSRIO_PORT_N_ERR_STS_CSR_OFFSET) | \
			 (Mask & XSRIO_PORT_N_ERR_STS_CSR_ERR_ALL_MASK))))

/****************************************************************************/
/**
*
* XSrio_GetPortErrorStatus  returns the mask for the port errors currently
* enabled in the SRIO Gen2 core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	The bit mask for the port errors that are currently enabled.
*
* @note         C-style signature:
*               u32 XSrio_GetPortErrorStatus(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_GetPortErrorStatus(InstancePtr)	\
	(XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
		       XSRIO_PORT_N_ERR_STS_CSR_OFFSET) & \
	 XSRIO_PORT_N_ERR_STS_CSR_ERR_ALL_MASK)

/****************************************************************************/
/**
*
* XSrio_SetPortControlStatus Configures specific port specified by the Mask.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
* @param	Mask is the mask for the port that needs to be enabled.
*
* @return	None.
*
* @note         C-style signature:
*               void XSrio_SetPortControlStatus(XSrio *InstancePtrm, u32 Mask)
*
*****************************************************************************/
#define XSrio_SetPortControlStatus(InstancePtr, Mask)	\
	(XSrio_WriteReg((InstancePtr)->Config.BaseAddress,	\
			XSRIO_PORT_N_CTL_CSR_OFFSET, 	\
			(XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
				       XSRIO_PORT_N_CTL_CSR_OFFSET) \
			 | (Mask & XSRIO_PORT_N_CTL_CSR_STATUS_ALL_MASK))))

/****************************************************************************/
/**
*
* XSrio_GetPortControlStatus  returns the status of the port that is currently
* enabled in the SRIO Gen2 core.
*
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
*
* @return	The bit mask for the ports that are currently enabled.
*
* @note         C-style signature:
*               u32 XSrio_GetPortControlStatus(XSrio *InstancePtr)
*
*****************************************************************************/
#define XSrio_GetPortControlStatus(InstancePtr)	\
	(XSrio_ReadReg((InstancePtr)->Config.BaseAddress,	\
		       XSRIO_PORT_N_CTL_CSR_OFFSET) & \
	 XSRIO_PORT_N_CTL_CSR_STATUS_ALL_MASK)

/*************************** Function Prototypes ******************************/
/**
 * Initialization and Control functions in xsrio.c
 */
int XSrio_CfgInitialize(XSrio *InstancePtr,
			XSrio_Config *Config, UINTPTR EffectiveAddress);
XSrio_Config *XSrio_LookupConfig(u32 DeviceId);
int XSrio_GetPortStatus(XSrio *InstancePtr);
int XSrio_GetPEType(XSrio *InstancePtr);
int XSrio_IsOperationSupported(XSrio *InstancePtr, u8 Operation, u8 Direction);
void XSrio_SetWaterMark(XSrio *InstancePtr, u8 WaterMark0, u8 WaterMark1,
			u8 WaterMark2);
void XSrio_GetWaterMark(XSrio *InstancePtr, u8 *WaterMark0, u8 *WaterMark1,
			u8 *WaterMark2);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
