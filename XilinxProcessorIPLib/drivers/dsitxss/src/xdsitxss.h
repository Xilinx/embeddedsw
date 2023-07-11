/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdsitxss.h
* @addtogroup dsitxss Overview
* @{
* @details
*
* This is main header file of the Xilinx MIPI DSI Tx Subsystem driver
*
* <b>MIPI DSI Tx Subsystem Overview</b>
*
* MIPI DSI Subsystem is collection of IP cores defines high speed serial
* interface between display peripheral and host processor. DSI Subsystem
* translate data received from a MIPI DSI Transmitter. The MIPI DSI Tx
* Subsystem is a plug-in solution for interfacing with MIPI DSI core.
* It hides all the complexities of programming the underlying cores from
* the end user.
*
* <b>Subsystem Features</b>
*
* MIPI DSI Tx Subsystem supports following features
*	- Support for 1 to 4 Data Lanes.
*	- Line rates ranging from 80 to 1500 Mbps.
*	- Different data type support(RGB888,RGB566,RGB666L,RGB666P).
*	- Filtering of packets based on Virtual channel ID.
*	- Single,Dual,Quad input pixel per beat
*
* <b>Subsystem Configurations</b>
*
* The GUI in IPI allows for the following configurations
* 	- Lanes ( 1 to 4 )
* 	- Pixel Format ( (RGB888,RGB566,RGB666L,RGB666P).
* 	- Number of Input Pixels per beat(1, 2, 4)
* 	- DPHY with/without Register interface
* 	- Line Rate
* 	- CRC Generation Enable
* In order to reduce resource usage, the DPHY can be configured to be without
* register interface with fixed functions. Static configuration parameters
* are stored in xdsitxss_g.c file, that gets generated when compiling the
* board support package (BSP).
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
* <b>Subsystem Driver Description</b>
*
* The subsystem driver provides an abstraction on top of the DSI and DPHY
* drivers.
*
* <b>Interrupt Service</b>
*
* The DSI TX subsytem supports 2 interrupts
* 1. Unsupported Data Type
* 2. Pixel Under flow error
* For Handling these interrupts, The users of this
* driver have to register this handler with the interrupt system and provide
* the callback functions by using XDSiTxSsSetCallback API
*
* <b>Virtual Memory </b>
*
* This driver supports Virtual Memory. The RTOS is responsible for calculating
* the correct device base address in Virtual Memory space.
*
* <b>Threads </b>
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
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date    Changes
* --- --- ------- -------------------------------------------------------
* 1.0 ram 11/02/16 Initial Release for MIPI DSI TX subsystem
* 1.1 sss 08/17/16 Added 64 bit support
*     sss 08/26/16 Add "Command Queue Vacancy" API
*                  API for getting pixel format
*     ms  01/23/17 Modified xil_printf statement in main function for all
*                  examples to ensure that "Successfully ran" and "Failed"
*                  strings are available in all examples. This is a fix
*                  for CR-965028.
*     ms  03/17/17 Added readme.txt file in examples folder for doxygen
*                  generation.
*     ms  04/05/17 Added tabspace for return statements in functions of
*                  xdsitxss_intr_example.c for proper documentation
*                  while generating doxygen.
* </pre>
*
******************************************************************************/

#ifndef XDSITXSS_H_   /* prevent circular inclusions */
#define XDSITXSS_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/


#include "xil_types.h"
#include "xdsi.h"
#include "xparameters.h"

#if (XPAR_XDPHY_NUM_INSTANCES > 0)
#include "xdphy.h"
#endif

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/

/**
 * Sub-Core Configuration Table
 */
typedef struct {
	u32 IsPresent;  /**< Flag to indicate if sub-core is present
			in the design */
	u32 DeviceId;   /**< Device ID of the sub-core */
	u32 AddrOffset;	/**< sub-core offset from subsystem base address */
} DsiTxSsSubCore;

/**
 * Subsystem Enable/Disable
 */
typedef enum {
	XDSITXSS_DISABLE,  /* DSI TX subsystem Disable */
	XDSITXSS_ENABLE	   /* DSI TX subsystem Enable */
} XDsiSS_Selection;

/**
 * Sub-Core Enable/Disable
 */
typedef enum {
	XDSITXSS_DSI,	/* DSI Core */
	XDSITXSS_PHY	/* DPHY */
} XDsiSS_Subcore;

/**
 * MIPI DSI Tx Subsystem configuration structure.
 * Each subsystem device should have a configuration structure associated
 * that defines the MAX supported sub-cores within subsystem
 */
typedef struct {
	u32 DeviceId;	/**< DeviceId is the unique ID
			  *  of the device */
	UINTPTR BaseAddr;	/**< BaseAddress is the physical
			  *  base address of the subsystem
			  *  address range */
	UINTPTR HighAddr;	/**< HighAddress is the physical
			  *  MAX address of the subsystem address range */
	u8 DsiLanes;	/**< DSI supported lanes 1, 2, 3, 4 */
	u8 DataType;	/**< RGB  type */
	u32 DsiByteFifo;	/**< 128, 256, 512, 1024, 2048, 4096,
				              8192, 16384 */
	u8 CrcGen;		/**< CRC Generation enable or not */
	u8 DsiPixel;	/**< Pixels per beat received on input stream */
	u32 DphyLinerate;	/**< DPHY line rate */
	u32 IsDphyRegIntfcPresent; /**< Flag for DPHY register
				 *  interface presence */
	DsiTxSsSubCore DphyInfo;	/**< Sub-core instance configuration */
	DsiTxSsSubCore DsiInfo;	/**< Sub-core instance configuration */
} XDsiTxSs_Config;

/**
*
* Callback type for all interrupts defined.
*
* @param	CallbackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
* @param	Mask is a bit mask indicating the cause of the event. For
*		current core version, this parameter is "OR" of 0 or more
*		XDSITXSS_ISR_*_MASK constants defined in xdsitxss_hw.h.
*
* @return	None.
*
* @note		None.
*
 *****************************************************************************/
typedef void (*XDsiTxSs_Callback) (void *CallbackRef, u32 Mask);

/**
 * The XDsiTxSs driver instance data. The user is required to allocate a variable
 * of this type for every XDsiTxSs device in the system. A pointer to a variable
 * of this type is then passed to the driver API functions.
 */
typedef struct {
	XDsiTxSs_Config Config;	/**< Hardware configuration */
	u32 IsReady;		/**< Device and the driver instance are
				  *  initialized */
	XDsi  *DsiPtr;		/**< handle to sub-core driver instance */
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	XDphy *DphyPtr;		/**< handle to sub-core driver instance */
#endif
	XDsi_ShortPacket SpktData; /**< Short packet strucute to
					send short packet */
	XDsiTx_CmdModePkt CmdPkt; /**< DSI Command mode packet structure */

	XDsi_ConfigParameters ConfigInfo;  /**< Configuration information
						contains GUI parameters,
						Timing parameters */

	XDsiTxSs_Callback ErrorCallback; 	/**< Callback function for
						rest all errors */
	void *ErrRef;				/**< To be passed to the
						Error Call back */
} XDsiTxSs;

/************************** Function Prototypes ******************************/

XDsiTxSs_Config* XDsiTxSs_LookupConfig(u32 DeviceId);
s32 XDsiTxSs_CfgInitialize(XDsiTxSs *InstancePtr, XDsiTxSs_Config *CfgPtr,
							UINTPTR EffectiveAddr);
u32 XDsiTxSs_DefaultConfigure(XDsiTxSs *InstancePtr);
int XDsiTxSs_Activate(XDsiTxSs *InstancePtr, XDsiSS_Subcore core, u8 Flag);
void XDsiTxSs_Reset(XDsiTxSs *InstancePtr);
void XDsiTxSs_ReportCoreInfo(XDsiTxSs *InstancePtr);
u32 XDsiTxSs_SelfTest(XDsiTxSs *InstancePtr);
void XDsiTxSs_SendShortPacket(XDsiTxSs *InstancePtr);
int XDsiTxSs_SetDSIMode(XDsiTxSs *InstancePtr, XDsi_DsiModeType mode);
int XDsiTxSs_SendCmdModePacket(XDsiTxSs *InstancePtr);
void XDsiTxSs_GetConfigParams(XDsiTxSs *InstancePtr);
u32 XDsiTxSs_IsControllerReady(XDsiTxSs *InstancePtr);
u32 XDsiTxSs_GetPixelFormat(XDsiTxSs *InstancePtr);
u32 XDsiTxSs_GetCmdQVacancy(XDsiTxSs *InstancePtr);
s32 XDsiTxSs_SetVideoInterfaceTiming(XDsiTxSs *InstancePtr,
					XDsi_VideoMode VideoMode,
					XVidC_VideoMode Resolution,
					u16 BurstPacketSize);
s32 XDsiTxSs_SetCustomVideoInterfaceTiming(XDsiTxSs *InstancePtr,
		XDsi_VideoMode VideoMode, XDsi_VideoTiming  *Timing);
u32 XDsiTxSs_SetCallback(XDsiTxSs *InstancePtr, u32 HandlerType,
				void *CallbackFunc, void *CallbackRef);
void XDsiTxSs_IntrHandler(void *InstancePtr);

/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
