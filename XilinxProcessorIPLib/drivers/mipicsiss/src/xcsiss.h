/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcsiss.h
* @addtogroup csiss_v1_6
* @{
* @details
*
* This is main header file of the Xilinx MIPI CSI Rx Subsystem driver.
*
* <b>MIPI CSI Rx Subsystem Overview</b>
*
* MIPI CSI Subsystem is collection of IP cores to control, receive and
* translate data received from a MIPI CSI Transmitter. The MIPI CSI2 Rx
* Subsystem is a plug-in solution for interfacing with MIPI CSI based image
* sensors and rest of the video pipeline. It hides all the complexities of
* programming the underlying cores from end user.
*
* <b>Core Features</b>
*
* MIPI CSI Rx Subsystem supports following features
*	- Support for 1 to 4 PPI Lanes.
*	- Line rates ranging from 80 to 1500 Mbps.
*	- Different data type support(RAW,RGB,YUV).
*	- AXI IIC support for CCI Interface.
*	- Using existing AXI IIC for CCI interface support for better
*	  understanding & compatibility with other IIC’s (if any) used
*	  in the system
*	- Filtering of packets based on Virtual channel ID.
*	- Single,Dual,Quad pixel support at output interface compliant
*	  to UG934 format.
*
* The GUI in IPI allows for the following configurations
*	- Lanes ( 1 to 4 )
*	- Pixel Format (All RAW and RGB, only YUV422 8bit)
*	- Virtual Channel (to filter or allow all from interlaced streams)
*	- Number of Pixels per clock (1, 2, 4)
*	- DPHY with/without Register interface
*	- Line Rate
*	- Buffer Depth
*	- Embedded Non Image data (if needed)
*	- Add IIC to subsystem (if required)
*
* The IIC can be added if the system doesn't contain an IIC or if a dedicated
* IIC is to be used for MIPI CSI Rx Subsystem. In order to reduce resource
* usage, the DPHY can be configured to be without register interface with
* fixed functions. Static configuration parameters are stored in xcsiss_g.c
* file that gets generated when compiling the board support package (BSP).
* A table is defined where each entry contains configuration information
* for the instances of the subsystem in the design. This information includes
* the elected configuration, sub-cores used and their device ID, base addresses
* of memory mapped devices and address range available for subsystem
* frame/field buffers.
*
* The subsystem driver itself always includes the full software stack
* irrespective of the configuration selected. Generic API's are provided to
* interact with the subsystem and/or with the included sub -cores.
* At run-time the subsystem will query the static configuration and configures
* itself for supported use cases
*
* The subsystem driver provides an abstraction on top of the CSI and DPHY
* drivers.
*
* The IIC instance (if present) is shared with application
* and can be controlled using the AXI IIC driver.
*
* <b>Software Initialization & Configuration</b>
*
* The application needs to do following steps in order for preparing the
* MIPI CSI2 Rx Subsystem core to be ready.
*
* - Call XCsiSs_LookupConfig using a device ID to find the core
*   configuration.
* - Call XCsiSs_CfgInitialize to initialize the device and the driver
*   instance associated with it.
*
* <b>Interrupts</b>
*
* The XCsiSs_SetCallBack() is used to register the call back functions
* for MIPI CSI2 Rx Subsystem driver with the corresponding handles
*
* <b> Virtual Memory </b>
*
* This driver supports Virtual Memory. The RTOS is responsible for calculating
* the correct device base address in Virtual Memory space.
*
* <b> Threads </b>
*
* This driver is not thread safe. Any needs for threads or thread mutual
* exclusion must be satisfied by the layer above this driver.
*

* <b>Asserts</b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining, at
* compile time, the NDEBUG identifier.  By default, asserts are turned on and
* it is recommended that application developers leave asserts on during
* development.
*
* <b>Building the driver</b>
*
* The MIPI CSI2 Rx Subsystem driver is composed of source files and depends on
* the CSI and DPHY drivers. The IIC driver is pulled in if the the IIC instance
* is enabled. The DPHY driver is pulled in only if the register interface has
* been enabled for it.Otherwise the CSI driver and subsystem files are built.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 vsa 07/25/15 Initial release
* 1.1 sss 08/17/16 Added 64 bit support
*     sss 08/29/16 Renamed SubCore to CsiRxSsSubCore
*     ms  01/23/17 Modified xil_printf statement in main function for all
*                  examples to ensure that "Successfully ran" and "Failed"
*                  strings are available in all examples. This is a fix
*                  for CR-965028.
*     ms  03/17/17 Added readme.txt file in examples folder for doxygen
*                  generation.
* </pre>
*
******************************************************************************/

#ifndef XCSISS_H_
#define XCSISS_H_		/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xcsi.h"
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
#include "xdphy.h"
#endif
#if (XPAR_XIIC_NUM_INSTANCES > 0)
#include "xiic.h"
#endif
#include "xdebug.h"
#include "xcsiss_hw.h"

/************************** Constant Definitions *****************************/

/** @name Interrupt Types for setting Callbacks
 *
 * These handlers are used to determine the type of the interrupt handler being
 * registered with the MIPI CSI Rx Subsystem. Since the subsystem is tightly
 * coupled with the CSI Rx Controller driver, the handlers from the sub core
 * are promoted to the subsystem level so that the application can use them.
 * @{
 */
#define XCSISS_HANDLER_DPHY		XCSI_HANDLER_DPHY
#define XCSISS_HANDLER_PKTLVL		XCSI_HANDLER_PKTLVL
#define XCSISS_HANDLER_PROTLVL		XCSI_HANDLER_PROTLVL
#define XCSISS_HANDLER_SHORTPACKET	XCSI_HANDLER_SHORTPACKET
#define XCSISS_HANDLER_FRAMERECVD	XCSI_HANDLER_FRAMERECVD
#define XCSISS_HANDLER_OTHERERROR	XCSI_HANDLER_OTHERERROR
#define XCSISS_HANDLER_VCX		XCSI_HANDLER_VCXFRAMEERROR
/*@}*/

/**
*
* Callback type which acts as a wrapper on top of CSI Callback.
*
* @param	CallbackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
* @param	Mask is a bit mask indicating the cause of the event. For
*		current core version, this parameter is "OR" of 0 or more
*		XCSISS_ISR_*_MASK constants defined in xcsiss_hw.h.
*
* @return	None.
*
* @note		None.
*
 *****************************************************************************/
typedef void (*XCsiSs_Callback)(void *CallbackRef, u32 Mask);

/**
 * Sub-Core Configuration Table
 */
typedef struct {
	u32 IsPresent;	/**< Flag to indicate if sub-core is present in
			  *  design */
	u32 DeviceId;	/**< Device ID of the sub-core */
	u32 AddrOffset;	/**< sub-core offset from subsystem base address */
} CsiRxSsSubCore;

/**
 * MIPI CSI Rx Subsystem configuration structure.
 * Each subsystem device should have a configuration structure associated
 * that defines the MAX supported sub-cores within subsystem
 */
typedef struct {
	u32 DeviceId;	/**< DeviceId is the unique ID  of the device */
	UINTPTR BaseAddr;	/**< BaseAddress is the physical base address
				of the subsystem address range */
	UINTPTR HighAddr;	/**< HighAddress is the physical MAX address
				of the  subsystem address range */
	u32 IsIicPresent;	/**< Flag for IIC presence in subsystem */
	u32 LanesPresent;	/**< Number of PPI Lanes in the design */
	u32 PixelCount;	/**< Number of Pixels per clock 1,2,4 */
	u32 PixelFormat;	/**< The pixel format selected from all RGB,
				  *  RAW and YUV422 8bit options */
	u32 VcNo;	/**< Number of Virtual Channels supported by system.
			  *  This can range from 1 - 4 to ALL */
	u32 CsiBuffDepth;	/**< Line buffer Depth set */
	u32 IsEmbNonImgPresent;	/**< Flag for presence of Embedded Non Image
				  *  data */
	u32 IsDphyRegIntfcPresent;	/**< Flag for DPHY register interface
					  *  presence */
	u32 DphyLineRate;	/**< DPHY Line Rate ranging from
				  *  80-1500 Mbps */
	u32 EnableCrc;		/**< CRC Calculation optimization enabled */
	u32 EnableActiveLanes;	/**< Active Lanes programming optimization
				  *  enabled */
	u8 EnableCSIv20; /* csiv2.0 support*/
	u8 EnableVCx;	/* vcx feature support*/
	CsiRxSsSubCore IicInfo;	/**< IIC sub-core configuration */
	CsiRxSsSubCore CsiInfo;	/**< CSI sub-core configuration */
	CsiRxSsSubCore DphyInfo;	/**< DPHY sub-core configuration */
} XCsiSs_Config;

/**
 * The XCsiSs driver instance data. The user is required to allocate a variable
 * of this type for every XCsiSs device in the system. A pointer to a variable
 * of this type is then passed to the driver API functions.
 */
typedef struct {
	XCsiSs_Config Config;	/**< Hardware configuration */
	u32 IsReady;		/**< Device and the driver instance are
				  *  initialized */
	XCsi  *CsiPtr;		/**< handle to sub-core driver instance */
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	XDphy *DphyPtr;		/**< handle to sub-core driver instance */
#endif
#if (XPAR_XIIC_NUM_INSTANCES > 0)
	XIic *IicPtr;		/**< handle to sub-core driver instance */
#endif
	XCsi_ClkLaneInfo ClkInfo;	/**< Clock Lane information */
	XCsi_DataLaneInfo DLInfo[XCSI_MAX_LANES];	/**< Data Lane
							  *  information */
	XCsi_SPktData SpktData;		/**< Short packet */
	XCsi_VCInfo VCInfo[XCSI_MAX_VC];/**< Virtual Channel information */
} XCsiSs;

/************************** Function Prototypes ******************************/

/* Initialization function in xcsiss_sinit.c */
XCsiSs_Config* XCsiSs_LookupConfig(u32 DeviceId);

/* Initialization and control functions xcsiss.c */
u32 XCsiSs_CfgInitialize(XCsiSs *InstancePtr, XCsiSs_Config *CfgPtr,
				UINTPTR EffectiveAddr);
#if (XPAR_XIIC_NUM_INSTANCES > 0)
XIic* XCsiSs_GetIicInstance(XCsiSs *InstancePtr);
#endif
u32 XCsiSs_Configure(XCsiSs *InstancePtr, u8 ActiveLanes, u32 IntrMask);
u32 XCsiSs_Activate(XCsiSs *InstancePtr, u8 Flag);
u32 XCsiSs_Reset(XCsiSs *InstancePtr);
void XCsiSs_ReportCoreInfo(XCsiSs *InstancePtr);
void XCsiSs_GetLaneInfo(XCsiSs *InstancePtr);
void XCsiSs_GetShortPacket(XCsiSs *InstancePtr);
void XCsiSs_GetVCInfo(XCsiSs *InstancePtr);

/* Self test function in xcsiss_selftest.c */
u32 XCsiSs_SelfTest(XCsiSs *InstancePtr);

/* Interrupt functions in xcsiss_intr.c */
void XCsiSs_IntrHandler(void *InstancePtr);
void XCsiSs_IntrDisable(XCsiSs *InstancePtr, u32 IntrMask);
u32 XCsiSs_SetCallBack(XCsiSs *InstancePtr, u32 HandlerType,
			void *CallbackFunc, void *CallbackRef);

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
