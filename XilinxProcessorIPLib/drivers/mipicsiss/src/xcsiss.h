/******************************************************************************
* Copyright (C) 2015 - 2022 Xilinx, Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcsiss.h
* @addtogroup csiss Overview
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
#if (XPAR_XMIPI_RX_PHY_NUM_INSTANCES > 0)
#include "xmipi_rx_phy.h"
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
#define XCSISS_HANDLER_DPHY		XCSI_HANDLER_DPHY	/**< D-PHY handler */
#define XCSISS_HANDLER_PKTLVL		XCSI_HANDLER_PKTLVL	/**< Packet level handler */
#define XCSISS_HANDLER_PROTLVL		XCSI_HANDLER_PROTLVL	/**< Protocol level handler */
#define XCSISS_HANDLER_SHORTPACKET	XCSI_HANDLER_SHORTPACKET /**< Short packet handler */
#define XCSISS_HANDLER_FRAMERECVD	XCSI_HANDLER_FRAMERECVD	/**< Frame received handler */
#define XCSISS_HANDLER_OTHERERROR	XCSI_HANDLER_OTHERERROR	/**< Other error handler */
#define XCSISS_HANDLER_VCX		XCSI_HANDLER_VCXFRAMEERROR /**< VCX frame error handler */
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
 * @brief Sub-Core Configuration Table
 *
 * This structure contains the configuration information for sub-cores
 * present in the MIPI CSI Rx Subsystem.
 */
typedef struct {
	u32 IsPresent;	/**< Flag to indicate if sub-core is present in
			  *  design */
#ifndef SDT
	u32 DeviceId;	/**< Device ID of the sub-core */
	u32 AddrOffset;	/**< Sub-core offset from subsystem base address */
#else
	UINTPTR AddrOffset;	/**< Sub-core offset from subsystem base address */
#endif
} CsiRxSsSubCore;

/**
 * @brief MIPI CSI Rx Subsystem configuration structure
 *
 * This structure contains the configuration information for the MIPI CSI Rx
 * Subsystem. Each subsystem device should have a configuration structure
 * associated that defines the MAX supported sub-cores within subsystem.
 */
typedef struct {
#ifndef SDT
	u32 DeviceId;	/**< DeviceId is the unique ID  of the device */
#else
	char *Name;
#endif
	UINTPTR BaseAddr;	/**< BaseAddress is the physical base address
				of the subsystem address range */
	UINTPTR HighAddr;	/**< HighAddress is the physical MAX address
				of the  subsystem address range */
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
	u8 EnableCSIv20;	/**< CSI v2.0 support enabled */
	u8 EnableVCx;		/**< VCx feature support enabled */
	CsiRxSsSubCore CsiInfo;	/**< CSI sub-core configuration */
	CsiRxSsSubCore DphyInfo;	/**< DPHY sub-core configuration */
	CsiRxSsSubCore MipiRxPhyInfo;	/**< MIPI RX PHY sub-core configuration */
#ifdef SDT
	u16 IntrId;		/**< Interrupt ID */
	UINTPTR IntrParent;	/**< Bit[0] Interrupt Parent */
#endif
} XCsiSs_Config;

/**
 * @brief The XCsiSs driver instance data
 *
 * The user is required to allocate a variable of this type for every XCsiSs
 * device in the system. A pointer to a variable of this type is then passed
 * to the driver API functions.
 */
typedef struct {
	XCsiSs_Config Config;	/**< Hardware configuration */
	u32 IsReady;		/**< Device and the driver instance are
				  *  initialized */
	XCsi  *CsiPtr;		/**< Handle to CSI sub-core driver instance */
#if (XPAR_XMIPI_RX_PHY_NUM_INSTANCES > 0)
	XMipi_Rx_Phy *MipiRxPhyPtr;	/**< Handle to MIPI RX PHY sub-core driver instance */
#endif
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	XDphy *DphyPtr;		/**< Handle to DPHY sub-core driver instance */
#endif
	XCsi_ClkLaneInfo ClkInfo;	/**< Clock Lane information */
	XCsi_DataLaneInfo DLInfo[XCSI_MAX_LANES];	/**< Data Lane information array */
	XCsi_SPktData SpktData;		/**< Short packet data */
	XCsi_VCInfo VCInfo[XCSI_MAX_VC];	/**< Virtual Channel information array */
} XCsiSs;

/************************** Function Prototypes ******************************/

/* Initialization function in xcsiss_sinit.c */
#ifndef SDT
/**
 * @brief Lookup the device configuration based on the unique device ID
 *
 * This function returns a reference to an XCsiSs_Config structure based on
 * the unique device ID. This function will return a NULL pointer if the
 * device ID is not found.
 *
 * @param	DeviceId is the unique device ID of the device for the lookup
 *		operation.
 *
 * @return	XCsiSs_Config reference if DeviceId is found, NULL otherwise.
 *
 * @note	None.
 *
 ******************************************************************************/
XCsiSs_Config* XCsiSs_LookupConfig(u32 DeviceId);
#else
/**
 * @brief Lookup the device configuration based on the base address
 *
 * This function returns a reference to an XCsiSs_Config structure based on
 * the base address. This function will return a NULL pointer if the base
 * address is not found.
 *
 * @param	BaseAddress is the base address of the device to lookup for.
 *
 * @return	XCsiSs_Config reference if BaseAddress is found, NULL otherwise.
 *
 * @note	None.
 *
 ******************************************************************************/
XCsiSs_Config* XCsiSs_LookupConfig(UINTPTR BaseAddress);

/**
 * @brief Get the driver index based on the base address
 *
 * This function returns the driver index based on the base address.
 *
 * @param	BaseAddress is the base address of the device.
 *
 * @return	Driver index.
 *
 * @note	None.
 *
 ******************************************************************************/
u32 XCsiSs_GetDrvIndex(UINTPTR BaseAddress);
#endif

/* Initialization and control functions xcsiss.c */
/**
 * @brief Initialize the MIPI CSI Rx Subsystem driver
 *
 * This function initializes the MIPI CSI Rx Subsystem driver. This function
 * must be called prior to using the driver. Initialization includes setting up
 * the instance data and ensuring the hardware is in a quiescent state.
 *
 * @param	InstancePtr is a pointer to the XCsiSs instance.
 * @param	CfgPtr is a reference to a structure containing information
 *		about a specific XCsiSs instance.
 * @param	EffectiveAddr is the base address of the device. If address
 *		translation is being used, then this parameter must reflect the
 *		virtual base address. Otherwise, the physical address should be
 *		used.
 *
 * @return
 *		- XST_SUCCESS if initialization was successful.
 *		- XST_FAILURE if initialization failed.
 *
 * @note	None.
 *
 ******************************************************************************/
u32 XCsiSs_CfgInitialize(XCsiSs *InstancePtr, XCsiSs_Config *CfgPtr,
				UINTPTR EffectiveAddr);

/**
 * @brief Configure the MIPI CSI Rx Subsystem
 *
 * This function configures the MIPI CSI Rx Subsystem with the specified
 * active lanes and interrupt mask.
 *
 * @param	InstancePtr is a pointer to the XCsiSs instance.
 * @param	ActiveLanes is the number of active lanes to be used.
 * @param	IntrMask is the interrupt mask to enable specific interrupts.
 *
 * @return
 *		- XST_SUCCESS if configuration was successful.
 *		- XST_FAILURE if configuration failed.
 *
 * @note	None.
 *
 ******************************************************************************/
u32 XCsiSs_Configure(XCsiSs *InstancePtr, u8 ActiveLanes, u32 IntrMask);

/**
 * @brief Activate or deactivate the MIPI CSI Rx Subsystem
 *
 * This function activates or deactivates the MIPI CSI Rx Subsystem based on
 * the Flag parameter.
 *
 * @param	InstancePtr is a pointer to the XCsiSs instance.
 * @param	Flag is the enable/disable flag. TRUE to enable, FALSE to disable.
 *
 * @return
 *		- XST_SUCCESS if operation was successful.
 *		- XST_FAILURE if operation failed.
 *
 * @note	None.
 *
 ******************************************************************************/
u32 XCsiSs_Activate(XCsiSs *InstancePtr, u8 Flag);

/**
 * @brief Reset the MIPI CSI Rx Subsystem
 *
 * This function resets the MIPI CSI Rx Subsystem.
 *
 * @param	InstancePtr is a pointer to the XCsiSs instance.
 *
 * @return
 *		- XST_SUCCESS if reset was successful.
 *		- XST_FAILURE if reset failed.
 *
 * @note	None.
 *
 ******************************************************************************/
u32 XCsiSs_Reset(XCsiSs *InstancePtr);

/**
 * @brief Report the core information
 *
 * This function reports the core information of the MIPI CSI Rx Subsystem.
 *
 * @param	InstancePtr is a pointer to the XCsiSs instance.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void XCsiSs_ReportCoreInfo(XCsiSs *InstancePtr);

/**
 * @brief Get lane information
 *
 * This function retrieves the lane information from the MIPI CSI Rx Subsystem.
 *
 * @param	InstancePtr is a pointer to the XCsiSs instance.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void XCsiSs_GetLaneInfo(XCsiSs *InstancePtr);

/**
 * @brief Get short packet information
 *
 * This function retrieves the short packet information from the MIPI CSI Rx
 * Subsystem.
 *
 * @param	InstancePtr is a pointer to the XCsiSs instance.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void XCsiSs_GetShortPacket(XCsiSs *InstancePtr);

/**
 * @brief Get virtual channel information
 *
 * This function retrieves the virtual channel information from the MIPI CSI Rx
 * Subsystem.
 *
 * @param	InstancePtr is a pointer to the XCsiSs instance.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void XCsiSs_GetVCInfo(XCsiSs *InstancePtr);

/**
 * @brief Get the virtual channel selection
 *
 * This function returns the current virtual channel selection.
 *
 * @param	InstancePtr is a pointer to the XCsiSs instance.
 *
 * @return	Virtual channel selection value.
 *
 * @note	None.
 *
 ******************************************************************************/
u32 XCsiSs_GetVCSelection(XCsiSs *InstancePtr);

/**
 * @brief Set the virtual channel selection
 *
 * This function sets the virtual channel selection.
 *
 * @param	InstancePtr is a pointer to the XCsiSs instance.
 * @param	Value is the virtual channel selection value to be set.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void XCsiSs_SetVCSelection(XCsiSs *InstancePtr, u16 Value);

/* Self test function in xcsiss_selftest.c */
/**
 * @brief Run a self-test on the MIPI CSI Rx Subsystem
 *
 * This function runs a self-test on the MIPI CSI Rx Subsystem driver/device.
 * The self-test checks for proper functionality of the subsystem.
 *
 * @param	InstancePtr is a pointer to the XCsiSs instance.
 *
 * @return
 *		- XST_SUCCESS if self-test was successful.
 *		- XST_FAILURE if self-test failed.
 *
 * @note	None.
 *
 ******************************************************************************/
u32 XCsiSs_SelfTest(XCsiSs *InstancePtr);

/* Interrupt functions in xcsiss_intr.c */
/**
 * @brief Interrupt handler for the MIPI CSI Rx Subsystem
 *
 * This function is the interrupt handler for the MIPI CSI Rx Subsystem driver.
 * It processes all pending interrupts and routes them to the appropriate
 * callback handlers.
 *
 * @param	InstancePtr is a pointer to the XCsiSs instance (passed as void *).
 *
 * @return	None.
 *
 * @note	This function should be connected to the interrupt system.
 *
 ******************************************************************************/
void XCsiSs_IntrHandler(void *InstancePtr);

/**
 * @brief Disable interrupts in the MIPI CSI Rx Subsystem
 *
 * This function disables the specified interrupts in the MIPI CSI Rx Subsystem.
 *
 * @param	InstancePtr is a pointer to the XCsiSs instance.
 * @param	IntrMask is the bit-mask of the interrupts to be disabled.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void XCsiSs_IntrDisable(XCsiSs *InstancePtr, u32 IntrMask);

/**
 * @brief Set callback function for interrupt handling
 *
 * This function sets a callback function for a specific interrupt handler type.
 * The registered callback function will be called when the corresponding
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XCsiSs instance.
 * @param	HandlerType is the type of handler to register. It can be one of:
 *		- XCSISS_HANDLER_DPHY
 *		- XCSISS_HANDLER_PKTLVL
 *		- XCSISS_HANDLER_PROTLVL
 *		- XCSISS_HANDLER_SHORTPACKET
 *		- XCSISS_HANDLER_FRAMERECVD
 *		- XCSISS_HANDLER_OTHERERROR
 *		- XCSISS_HANDLER_VCX
 * @param	CallbackFunc is the address of the callback function.
 * @param	CallbackRef is a user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return
 *		- XST_SUCCESS if callback was successfully registered.
 *		- XST_FAILURE if callback registration failed.
 *
 * @note	None.
 *
 ******************************************************************************/
u32 XCsiSs_SetCallBack(XCsiSs *InstancePtr, u32 HandlerType,
			void *CallbackFunc, void *CallbackRef);

#if (XPAR_XMIPI_RX_PHY_NUM_INSTANCES > 0)
/**
 * @brief Configure the MIPI RX PHY PLL for a desired line rate at runtime
 *
 * This function configures the MIPI RX PHY PLL for a desired line rate at
 * runtime. It delegates to XMipi_Rx_Phy_DynamicLineRateConfig which handles
 * validation of CPHY mode and dynamic line rate hardware enablement.
 *
 * @param	InstancePtr is a pointer to the XCsiSs instance.
 * @param	PllBaseAddr is the base address of the PLL to be configured.
 * @param	LineRate is the desired line rate in Mbps (400-4500).
 *
 * @return
 *		- XST_SUCCESS if PLL configuration is successful.
 *		- XST_FAILURE if dynamic line rate configuration fails.
 *
 * @note	None.
 *
 ******************************************************************************/
u32 XCsiSs_DynamicLineRateConfig(XCsiSs *InstancePtr, UINTPTR PllBaseAddr,
				u32 LineRate);
#endif

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
