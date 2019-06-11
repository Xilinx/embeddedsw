/******************************************************************************
*
* Copyright (C) 2006 Vreelin Engineering, Inc.  All Rights Reserved.
* (c) Copyright 2007-2019 Xilinx, Inc. All rights reserved.
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
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xusb.h
* @addtogroup usb_v5_2
* @{
* @details
*
* This file contains the implementation of the XUsb component. It is the
* driver for the USB device controller.
*
* The USB device controller supports the following features:
*	- USB 2.0 Specification supporting High/Full/Low Speed
*	- 8 Endpoints
*		- 1 Control Endpoint
*		- 7 Configurable Endpoints, which can be configured
*			as IN or OUT , and configurable as Interrupt or Bulk or
* 			Isochronous
*	- 2 Ping Pong Buffers for all the endpoints except the Control Endpoint
*
* <b>Initialization & Configuration</b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate to the USB device. Apart from transmission and reception of
* USB frames the driver also handles the configuration of the device. A single
* device driver can support multiple USB devices.
*
* XUsb_CfgInitialize() API is used to initialize the USB device.
* The user needs to first call the XUsb_LookupConfig() API which returns
* the Configuration structure pointer which is passed as a parameter to the
* XUsb_CfgInitialize() API.
*
* - Configuration of the DEVICE endpoints:
*   The endpoints of the device need to be configured using the
*  	XUsb_EpConfigure() function call.
*   After configuration is complete, the endpoints need to be initialized
*   using the XUsb_EpInitializeAll() function call.
*
* <b> PHY Communication </b>
*
* As the H/W doesn't give any provision for the driver to configure the PHY, the
* driver doesn't provide any mechanism for directly programming the PHY.
*
* <b> DMA </b>
*
* The USB device has an inbuilt DMA. It's a simple DMA for data transfer
* between endpoint buffer memory and the external memory.
* The driver has two APIs for DMA operation. one API is used for resetting the
* DMA module of the USB device. The other API is for initiating the DMA
* transfer. The DMA transfer API is internal to the driver and is used by the
* USB endpoint data send and data receive functions. Upon completion of DMA
* transfer the USB device sets the buffer ready bit of the endpoint for which
* the DMA transfer is initiated. Setting of the buffer ready bit enables
* transmission/reception of an endpoint data. To enable the USB device to know
* to which endpoint the current DMA transfer is initiated, the driver writes the
* buffer ready mask to the DMA control register.
*
* The DMA in the device can be enabled or disabled only during the system build
* time.
*
* <b> Interrupts </b>
*
* The driver provides an interrupt handler XUsb_IntrHandler for handling
* the interrupt from the USB device. The users of this driver have to
* register this handler with the interrupt system and provide the callback
* functions.
* The interrupt handlers and associated callback functions for the USB device
* have to be registered by the user using the XUsb_IntrSetHandler() function
* and/or XUsb_EpSetHandler() function.
*
* XUsb_IntrSetHandler() function installs an asynchronous callback function
* for the general interrupts (interrupts other than the endpoint interrupts).
*
* XUsb_EpSetHandler() function installs the callback functions for the
* interrupts related to the endpoint events. A separate callback function has to
* be installed for each of the endpoints.
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
* <b> Asserts </b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining, at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that users leave asserts on during development.
*
* <b> Building the driver </b>
*
* The XUsb driver is composed of several source files. This allows the user
* to build and link only those parts of the driver that are necessary.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00a hvm  02/22/07 First release.
* 1.01a hvm  05/30/07 The chapter 9 files are moved to examples directory.
* 1.01a sdm  08/22/08 Removed support for static interrupt handlers from the MDD
*			file
* 1.01a hvm  10/02/08 The Buffer0Ready, Buffer1Ready are declared as volatile.
*			In function XUsb_EpDataRecv, the initialization of
*			Buffer0Ready, Buffer1Ready and CurBufNum variables is
*			moved before the buffer ready bit is set in the buffer
*			ready register.
*			Added the initialization of Buffer0Ready, Buffer1Ready
*			and CurBufNum variables in the XUsb_EpDataSecd function.
* 2.00a	hvm  10/22/08 Added Support for the new XPS USB device. The new
*			device has support for DMA. Apart from the
*			DMA, remote wakeup feature is also added the USB device.
* 			However, there is no additional code needed to be added
*			in the driver to support this feature.
* 3.00a hvm  12/03/09 Added the modifications related to the new USB error
*			reporting register in the XPS USB device. Updated to use
*			HAL processor APIs. Removed _m from the	name of the
*			macros.
*			XUsb_mReadReg is renamed to XUsb_ReadReg and
*			XUsb_mWriteReg is renamed to XUsb_WriteReg.
* 3.01a hvm  5/20/10  Updated with fix for CR561171.The interrupt handler is
*			updated to call the error handler callback function
* 			during error interrupts.
* 3.02a hvm  7/15/10  Added Device ID initialization in XUsb_CfgInitialize
*			function (CR555996).
* 3.02a hvm  8/5/10   Updated the XUsb_EpDataRecv function to ensure that the
*			buffer ready bit setup is now made only during non-DMA
*			case. CR570776.
* 3.02a hvm  8/16/10  Updated the examples with the little endian support.
* 4.00a hvm  10/21/10 Added new API XUsb_DmaIntrSetHandler for setting up DMA
*			handler. Updated the XUsb_IntrHandler function to call
*			the DMA handler to handle DMA events. Removed DmaDone
*			and DmaError variables from the XUsb structure.
*			Added two new APIs to provide access to the new ULPI PHY
*			register.
* 4.01a hvm  8/23/11  Added new bit definitions for isochronous transfer bits
*			in endpoint configuration register. Added a new API
*			for setting these bits for a given endpoint. These bits
*			are available only in the newer versions of the AXI USB
*			IP. Check the IP datasheet for more details.
* 4.02a bss  3/04/12  Modified XCOMPONENT_IS_READY to XIL_COMPONENT_IS_READY
*			CR 650877
* 4.03a bss  06/20/10 Added SIE Reset API to reset (XUsb_SieReset) the SIE
*		      state machine in xusb.c and SIE Reset Mask in xusb_l.h
*		      for CR 660602
* 4.04a bss  10/22/13 Added macros for HSIC PHY registers in xusb_l.h.
* 5.0   adk  19/12/13 Updated as per the New Tcl API's
* 5.1   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
*                     Changed the prototype of XUsb_CfgInitialize API.
* 5.2  MNK   29/03/15 Added 64bit changes for ZYNQMP.
*      ms    03/17/17 Modified text file in examples folder for doxygen
*                     generation.
* 5.3   asa  02/05/19 Added dependencies.props in data folder for
*                     importing examples in SDK.
*
* </pre>
*
 ******************************************************************************/
#ifndef XUSB_H
#define XUSB_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xusb_l.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/**
 * @name 	Endpoint Direction
 * Definitions to be used with Endpoint related function that require a
 * 'Direction' parameter.
 *
 * NOTE:
 *   The direction is always defined from the perspective of the HOST!  This
 *   means that an IN endpoint on the device is used for sending data while the
 *   OUT endpoint on the device is used for receiving data.
 * @{
 */
#define XUSB_EP_DIRECTION_IN		1	/**< Endpoint direction IN. */
#define XUSB_EP_DIRECTION_OUT		0	/**< Endpoint direction OUT. */

/* @} */

/**
 * @name Speed
 * Definitions to be used with speed.
 * @{
 */
#define XUSB_EP_HIGH_SPEED		1	/**< High Speed */
#define XUSB_EP_FULL_SPEED		0	/**< Full Speed */

/* @} */

/** @name  USB device specific global configuration constants.
 *  @{
 */
#define XUSB_MAX_ENDPOINTS		8	/**< Maximum End Points */
#define XUSB_EP_NUMBER_ZERO		0	/**< End point Zero */
#define XUSB_DEVICEADDR_MAX		127	/**< Max device address */

/* @} */

/** @name USB device disconnect state
 * @{
 */
#define XUSB_DISCONNECTED		0x1	/**< Disconnected state */
#define XUSB_RESET			0x0	/**< Reset State */
/* @} */

/**  @name Test Modes (Set Feature)
 * @{
 */
#define TEST_J				1	/**< Chirp J Test */
#define TEST_K				2	/**< Chirp K Test */
#define TEST_SE0_NAK			3	/**< Chirp SE0 Test */
#define TEST_PKT			4	/**< Packet Test */

/* @} */

/**************************** Type Definitions *******************************/

/*****************************************************************************/
/**
 * This data type defines the callback function to be used for Endpoint
 * handlers.
 *
 * @param	CallBackRef is the Callback reference passed in by the upper
 * 		layer when setting the handler, and is passed back to the upper
 * 		layer when the handler is called.
 * @param	EpNum is the endpoint that caused the event.
 * @param	EventType is the type of the event that occurred on that
 * 		endpoint.
 */
typedef void (*XUsb_EpHandlerFunc) (void *CallBackRef, u8 EpNum, u32 EventType);

/*****************************************************************************/
/**
 * This data type defines the callback function to be used for the general
 * interrupt handler.
 *
 * @param	CallBackRef is the Callback reference passed in by the upper
 * 		layer when setting the handler, and is passed back to the upper
 *		layer when the handler is called.
 * @param	InterruptType is the type of the event that caused the
 * 		interrupt.
 */
typedef void (*XUsb_IntrHandlerFunc) (void *CallBackRef, u32 InterruptType);

/**
 * The XUsb_EpConfig structure is used to configure endpoints.
 */
typedef struct {
	int OutIn;		/**< The end point direction */
	int EpType;		/**< Bulk/interrupt/Isochronous */
	int Buffer0Count;	/**< Pkt Size for the first ping-pong
					buffer */
	volatile int Buffer0Ready;	/**< Status flag for first ping-pong
					buffer */
	int Buffer1Count;	/**< Pkt Size for the second ping-pong
					buffer */
	volatile int Buffer1Ready;	/**< Status flag for second ping-pong
					buffer */
	u32 Size;		/**< Maximum buffer size for this end
					point */
	u32 RamBase;		/**< The rambase offset value in the
					end point buffer space */
	volatile int CurBufNum;		/**< The current ping-pong buffer to be
					used */

	XUsb_EpHandlerFunc HandlerFunc;	/**< Call back function for this end
						point */
	void *HandlerRef;		/**< Callback reference */

} XUsb_EpConfig;

/**
 * The XUsb_DeviceConfig structure contains the configuration information to
 * configure the USB controller for DEVICE mode. This data structure is used
 * with the XUsb_ConfigureDevice() function call.
 */
typedef struct {
	u8 NumEndpoints;	/**< Number of Endpoints */
	XUsb_EpConfig Ep[XUSB_MAX_ENDPOINTS];	/**< An array of end points */
	u8 Status;		/**< USB device Status */
	u8 CurrentConfiguration;     /**< Current state of enumeration
				enumerated (1)/Not enumerated (0)*/
	u32 CurrentSpeed;	/**< Current Speed */

} XUsb_DeviceConfig;

/**
 * The XUsb_Config structure contains configuration information for the USB
 * controller.
 *
 * This structure only contains the basic configuration for the device. The
 * caller also needs to initialize the USB device controller with the
 * XUsb_ConfigureDevice() function call.
 */
typedef struct {
	u16 DeviceId;		/**< Unique ID of device. */
	UINTPTR BaseAddress;	/**< Core register base address. */
	u8 DmaEnabled;		/**< DMA support Enabled */
	u8 AddrWidth;		/**< DMA Address Width */
} XUsb_Config;


/**
 * The XUsb driver instance data. The user is required to allocate a
 * variable of this type for every USB device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
	XUsb_Config Config;	/**< Configuration structure */

	u32 IsReady;		/**< Device is initialized and ready */

	u32 USBAddress;		/**< The USB address of the device. */

	u32 EndPointOffset[XUSB_MAX_ENDPOINTS];	    /**< End point Offsets */

	/**
	 * The following structure holds the configuration for the controller.
	 * They are initialized using the XUsb_ConfigureDevice() function
	 * call.
	 */
	XUsb_DeviceConfig DeviceConfig;

	/**
	 * Callbacks and callback references
	 */
	XUsb_IntrHandlerFunc HandlerFunc;
	void *HandlerRef;

	XUsb_IntrHandlerFunc ErrHandlerFunc;
	void * ErrHandlerRef;

	XUsb_IntrHandlerFunc DmaHandlerFunc;
	void * DmaHandlerRef;

	XUsb_IntrHandlerFunc UlpiHandlerFunc;
	void * UlpiHandlerRef;

} XUsb;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions ******************************/

/************************** Function Prototypes ******************************/

/**
 * Setup / Initialize and DMA functions
 *
 * Implemented in the file xusb.c
 */
int XUsb_CfgInitialize(XUsb *InstancePtr, XUsb_Config *ConfigPtr,
			UINTPTR EffectiveAddr);

int XUsb_ConfigureDevice(XUsb *InstancePtr, XUsb_DeviceConfig *CfgPtr);

void XUsb_Start(XUsb *InstancePtr);
void XUsb_Stop(XUsb *InstancePtr);

u32 XUsb_GetFrameNum(const XUsb *InstancePtr);

int XUsb_SetDeviceAddress(XUsb *InstancePtr, u8 Address);

void XUsb_SetTestMode(XUsb *InstancePtr, u8 TestMode, u8 *BufPtr);

void XUsb_DmaReset(XUsb *InstancePtr);

void XUsb_DmaTransfer(XUsb *InstancePtr, UINTPTR *SrcAddr, UINTPTR *DstAddr,
				u16 Length);

void XUsb_ReadErrorCounters(XUsb *InstancePtr, u8 *BitStuffErrors,
				u8 *PidErrors, u8 *CrcErrors);

u8 XUsb_UlpiPhyReadRegister(XUsb *InstancePtr, u8 RegAddr);

int XUsb_UlpiPhyWriteRegister(XUsb *InstancePtr, u8 RegAddr,
				u8 UlpiPhyRegData);

void XUsb_SieReset(XUsb *InstancePtr);

/*
 * Functions for managing Endpoints / Data Transfers
 *
 * Implemented in the file xusb_endpoint.c
 */

void XUsb_EpEnable(const XUsb *InstancePtr, u8 EpNum);
void XUsb_EpDisable(const XUsb *InstancePtr, u8 EpNum);
void XUsb_EpConfigure(XUsb *InstancePtr, u8 EpNum, XUsb_EpConfig *EpCfgPtr);

int XUsb_EpDataSend(XUsb *InstancePtr, u8 EpNum, u8 *BufferPtr, u32 BufferLen);
int XUsb_EpDataRecv(XUsb *InstancePtr, u8 EpNum, u8 *BufferPtr, u32 BufferLen);

void XUsb_EpStall(const XUsb *InstancePtr, u8 EpNum);
void XUsb_EpUnstall(const XUsb *InstancePtr, u8 EpNum);

void XUsb_EpIsoTransferConfigure(XUsb *InstancePtr, u8 EpNum, u8 NoOfTransfers);

/*
 * Interrupt handling functions
 *
 * Implemented in the file xusb_intr.c
 */

void XUsb_IntrEnable(XUsb *InstancePtr, u32 IntrMask);

void XUsb_IntrDisable(XUsb *InstancePtr, u32 IntrMask);

void XUsb_IntrHandler(void *InstancePtr);

void XUsb_IntrSetHandler(XUsb *InstancePtr, void *CallBackFunc,
			 void *CallBackRef);
void XUsb_EpSetHandler(XUsb *InstancePtr, u8 EpNum,
			XUsb_EpHandlerFunc *CallBackFunc, void *CallBackRef);

void XUsb_ErrIntrSetHandler(XUsb *InstancePtr, void *CallBackFunc,
			 void *CallBackRef);

void XUsb_DmaIntrSetHandler(XUsb *InstancePtr, void *CallBackFunc,
			 void *CallBackRef);

void XUsb_UlpiIntrSetHandler(XUsb *InstancePtr, void *CallBackFunc,
			 void *CallBackRef);

/*
 * Static configuration helper function.
 *
 * The following function is provided for non-Linux drivers such as drivers for
 * VxWorks and Xilinx standalone systems.
 *
 * Implemented in xusb_sinit.c
 */

XUsb_Config *XUsb_LookupConfig(u16 DeviceId);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_H */
/** @} */
