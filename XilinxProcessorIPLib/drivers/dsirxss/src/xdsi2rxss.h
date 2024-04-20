/******************************************************************************
* Copyright 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdsi2rxss.h
* @addtogroup dsirxss Overview
* @{
* @details
*
* This is main header file of the Xilinx MIPI DSI Rx Subsystem driver
*
* <b>MIPI DSI2 Rx Subsystem Overview</b>
*
* MIPI DSI2 Subsystem is collection of IP cores defines high speed serial
* interface between display peripheral and host processor. DSI Subsystem
* translate data received from a MIPI DSI Transmitter. The MIPI DSI Rx
* Subsystem is a plug-in solution for interfacing with MIPI DSI core.
* It hides all the complexities of programming the underlying cores from
* the end user.
*
* <b>Subsystem Features</b>
*
* MIPI DSI2 Rx Subsystem supports following features
*	- Ultrascale+ family support.
*	- Support for 1 to 4 Data Lanes.
*	- Line rates ranging from 80 to 2500 Mbps.
*	- Different data type support(RGB888,RGB565,RGB666L,RGB666P).
*	- Continuous and non-continuous clock.
*	- Unidirectional.
*	- 3 video modes and command mode(Unidirectional supported command).
*	- 1-bit error correction and 2-bit error detection and CRC.
*	- Inturrpt generation to indicate subsystem status information.
*	- Single Virtual Channel.
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
* are stored in xdsirxss_g.c file, that gets generated when compiling the
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
* The DSI RX subsytem supports 2 interrupts
* 1. Unsupported Data Type
* 2. Pixel Under flow error
* For Handling these interrupts, The users of this
* driver have to register this handler with the interrupt system and provide
* the callback functions by using XDSiRxSsSetCallback API
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
* 1.0 Kunal 12/02/24 Initial Release for MIPI DSI RX subsystem
* </pre>
*
******************************************************************************/

#ifndef XDSI2RXSS_H_   /* prevent circular inclusions */
#define XDSI2RXSS_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/


#include "xil_types.h"
#include "xdsi2rx.h"
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
	UINTPTR AddrOffset; /* Address offset of the IP */

} Dsi2RxSsSubCores;

/**
 * Subsystem Enable/Disable
 */
typedef enum {
	XDSI2RXSS_DISABLE,  /* DSI RX subsystem Disable */
	XDSI2RXSS_ENABLE	   /* DSI RX subsystem Enable */
} XDsi2RxSs_Selection;

/**
 * Sub-Core Enable/Disable
 */
typedef enum {
	XDSI2RXSS_DSI,	/* DSI Core */
	XDSI2RXSS_PHY	/* DPHY */
} XDsi2RxSs_SubCore;

/**
 * MIPI DSI Rx Subsystem configuration structure.
 * Each subsystem device should have a configuration structure associated
 * that defines the MAX supported sub-cores within subsystem
 */
typedef struct {
	char *Name;		/* Name of the Device */
	UINTPTR BaseAddr;	/**< BaseAddress is the physical
			  *  base address of the subsystem
			  *  address range */
	UINTPTR HighAddr;	/**< HighAddress is the physical
			  *  MAX address of the subsystem address range */
	u8 DataType;	/**< RGB  type */
	u8 DsiPixel;	/**< Pixels per beat received on input stream */
	u32 DphyLinerate;	/**< DPHY line rate */
	u32 IsDphyRegIntfcPresent; /**< Flag for DPHY register
				 *  interface presence */
	Dsi2RxSsSubCores DphyInfo;	/**< Sub-core instance configuration */
	Dsi2RxSsSubCores Dsi2RxInfo;	/**< Sub-core instance configuration */
	u16 IntrId;		/* Interrupt ID */
	UINTPTR IntrParent; 	/* Bit[0] Interrupt Parent */
} XDsi2RxSs_Config;

/**
*
* Callback type for all interrupts defined.
*
* @param	CallbackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
* @param	Mask is a bit mask indicating the cause of the event. For
*		current core version, this parameter is "OR" of 0 or more
*		XDSIRXSS_ISR_*_MASK constants defined in xdsirxss_hw.h.
*
* @return	None.
*
* @note		None.
*
 *****************************************************************************/
typedef void (*XDsi2RxSs_Callback) (void *CallbackRef, u32 Mask);

/**
 * The XDsi2RxSs driver instance data. The user is required to allocate a variable
 * of this type for every XDsi2RxSs device in the system. A pointer to a variable
 * of this type is then passed to the driver API functions.
 */
typedef struct {
	XDsi2RxSs_Config Config;	/**< Hardware configuration */
	u32 IsReady;		/**< Device and the driver instance are
				  *  initialized */
	XDsi2Rx  *Dsi2RxPtr;		/**< handle to sub-core driver instance */
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	XDphy *DphyPtr;		/**< handle to sub-core driver instance */
#endif

	XDsi2Rx_ConfigParameters ConfigInfo;  /**< Configuration information
						contains GUI parameters,
						Timing parameters */

	XDsi2RxSs_Callback ErrorCallback; 	/**< Callback function for
						rest all errors */
	void *ErrRef;				/**< To be passed to the
						Error Call back */
} XDsi2RxSs;

/************************** Function Prototypes ******************************/
XDsi2RxSs_Config* XDsi2RxSs_LookupConfig(UINTPTR BaseAddress);
u32 XDsi2RxSs_GetDrvIndex(XDsi2RxSs *InstancePtr, UINTPTR BaseAddress);

s32 XDsi2RxSs_CfgInitialize(XDsi2RxSs *InstancePtr, XDsi2RxSs_Config *CfgPtr,
							UINTPTR EffectiveAddr);
u32 XDsi2RxSs_DefaultConfigure(XDsi2RxSs *InstancePtr);
int XDsi2RxSs_Activate(XDsi2RxSs *InstancePtr, XDsi2RxSs_SubCore core, u8 Flag);
void XDsi2RxSs_Reset(XDsi2RxSs *InstancePtr);
void XDsi2RxSs_ReportCoreInfo(XDsi2RxSs *InstancePtr);
u32 XDsi2RxSs_SelfTest(XDsi2RxSs *InstancePtr);
void XDsi2RxSs_GetConfigParams(XDsi2RxSs *InstancePtr);
u32 XDsi2RxSs_IsControllerReady(XDsi2RxSs *InstancePtr);
u32 XDsi2RxSs_GetPixelFormat(XDsi2RxSs *InstancePtr);
u32 XDsi2RxSs_SetCallback(XDsi2RxSs *InstancePtr, u32 HandlerType,
				void *CallbackFunc, void *CallbackRef);
void XDsi2RxSs_IntrHandler(void *InstancePtr);

/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
