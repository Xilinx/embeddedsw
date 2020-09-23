/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcsi2txss.h
* @addtogroup csi2txss_v1_4
* @{
* @details
*
* This is main header file of the Xilinx MIPI CSI Tx Subsystem driver.
*
* <b>MIPI CSI2 Tx Subsystem Overview</b>
*
* CSI-2 Tx Controller receives stream of image data via Native / AXI4 Stream
* input interface. It Packs the incoming image data into CSI-2 Packet Structure
* i.e Packs the Synchronization pacckets & performs the pixel-2-Byte Conversions
* for the pixel Data.Packed Byte data is sent over the D-PHY Interface for
* transmission. AXI4-Lite interface will be used to access core registers.
* CSI2-Tx Controller support’s ECC & CRC generation for header & payload
* respectively.
*
* <b>Core Features</b>
* The Xilinx CSI-2 Tx Subsystem has the following features:
•	Compliant with the MIPI CSI-2 Interface Specification, rev. 1.1
•	Standard PPI interface i.e. D-PHY
•	1-4 Lane Support,configurable through GUI
•	Maximum Data Rate per – 1.5 Gigabits per second
•	Multiple data type support :
o	RAW8,RAW10,RAW12,RAW14,RGB888,YUV422-8Bit,User defined  Data types
•	Supports Single,Dual,Quad Pixel Modes, configurable through GUI
•	Virtual channel Support (1 to 4)
•	Low Power State(LPS) insertion between the packets.
•	Ultra Low Power(ULP) mode generation using register access.
•	Interrupt generation & Core Status information can be accessed through
	Register Interface
•	Multilane interoperability.
•	ECC generation for packet header.
•	CRC generation for data bytes(Can be Enabled / Disabled),
	configurable through GUI.
•	Pixel byte conversion based on data format.
•	AXI4-Lite interface to access core registers.
•	Compliant with Xilinx AXI Stream Interface & native
	Interface for input video stream.
•	LS/LE Packet Generation,can be configured through  register interface.
•	Configurable selection of D-PHY Register Interface through GUI options.
•	Support for transmission of Embedded Data packet’s through Input
	Interface.
*
*
* <b>Software Initialization & Configuration</b>
*
* The application needs to do following steps in order for preparing the
* MIPI CSI2 Tx Subsystem core to be ready.
*
* - Call XCsi2TxSs_LookupConfig using a device ID to find the core
*   configuration.
* - Call XCsi2TxSs_CfgInitialize to initialize the device and the driver
*   instance associated with it.
*
* <b>Interrupts</b>
*
* The XCsi2TxSs_SetCallBack() is used to register the call back functions
* for MIPI CSI2 Tx Subsystem driver with the corresponding handles
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
* The MIPI CSI2 Tx Subsystem driver is composed of source files and depends on
* the CSI and DPHY drivers.
* The DPHY driver is pulled in only if the register interface has been enabled
* for it.Otherwise the CSI2TX driver and subsystem files are built.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who  Date     Changes
* --- --- -------- -----------------------------------------------------------
* 1.0 sss 07/14/16 Initial release
*     ms  01/23/17 Modified xil_printf statement in main function for all
*                  examples to ensure that "Successfully ran" and "Failed"
*                  strings are available in all examples. This is a fix
*                  for CR-965028.
*     ms  03/17/17 Added readme.txt file in examples folder for doxygen
*                  generation.
*     vsa 15/12/17 Add support for Clock Mode
* 1.2 vsa 02/28/18 Add Frame End Generation feature
* </pre>
*
******************************************************************************/

#ifndef XCSI2TXSS_H_
#define XCSI2TXSS_H_		/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xcsi2tx.h"
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
#include "xdphy.h"
#endif
#include "xdebug.h"
#include "xcsi2txss_hw.h"

/************************** Constant Definitions *****************************/
/** @name Clock Modes for CSI2 Tx
 *
 * These macros are used to set/get the clock mode in CSI2 Tx.
 * CCM  - continuous clock mode.
 * NCCM - non-continuous clock mode.
 * @{
 */
#define XCSI2TXSS_CCM	0
#define XCSI2TXSS_NCCM	1
/*@}*/

/** @name Interrupt Types for setting Callbacks
 *
 * These handlers are used to determine the type of the interrupt handler being
 * registered with the MIPI CSI2 Tx Subsystem. Since the subsystem is tightly
 * coupled with the CSI2 Tx Controller driver, the handlers from the sub core
 * are promoted to the subsystem level so that the application can use them.
 * @{
 */
#define XCSI2TXSS_HANDLER_WRG_LANE		XCSI2TX_HANDLER_WRG_LANE
#define XCSI2TXSS_HANDLER_GSPFIFO_FULL		XCSI2TX_HANDLER_GSPFIFO_FULL
#define XCSI2TXSS_HANDLER_ULPS			XCSI2TX_HANDLER_ULPS
#define XCSI2TXSS_HANDLER_LINEBUF_FULL		XCSI2TX_HANDLER_LINEBUF_FULL
#define XCSI2TXSS_HANDLER_WRG_DATATYPE		XCSI2TX_HANDLER_WRG_DATATYPE
#define XCSI2TXSS_HANDLER_UNDERRUN_PIXEL	XCSI2TX_HANDLER_UNDERRUN_PIXEL
#define XCSI2TXSS_HANDLER_LCERRVC0		XCSI2TX_HANDLER_LCERRVC0
#define XCSI2TXSS_HANDLER_LCERRVC1		XCSI2TX_HANDLER_LCERRVC1
#define XCSI2TXSS_HANDLER_LCERRVC2		XCSI2TX_HANDLER_LCERRVC2
#define XCSI2TXSS_HANDLER_LCERRVC3		XCSI2TX_HANDLER_LCERRVC3
/*@}*/

#define XCSI2TXSS_MAX_VC			XCSI2TX_MAX_VC

/**
 * This typedef defines the different errors codes for Line Count
 * status for a Virtual Channel when Frame End Generation is enabled
 */
typedef enum {
	XCSI2TXSS_LC_LESS_LINES = XCSI2TX_LC_LESS_LINES,	/**< Less no of lines recvd */
	XCSI2TXSS_LC_MORE_LINES = XCSI2TX_LC_MORE_LINES		/**< More no of lines recvd */
} XCsi2TxSS_LCStatus;

/**
*
* Callback type which acts as a wrapper on top of CSI Callback.
*
* @param	CallbackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
* @param	Mask is a bit mask indicating the cause of the event. For
*		current core version, this parameter is "OR" of 0 or more
*		XCSI2TXSS_ISR_*_MASK constants defined in xcsi2txss_hw.h.
*
* @return	None.
*
* @note		None.
*
 *****************************************************************************/
typedef void (*XCsi2TxSs_Callback)(void *CallbackRef, u32 Mask);

/**
 * Sub-Core Configuration Table
 */
typedef struct {
	u32 IsPresent;	/**< Flag to indicate if sub-core is present in
			  *  design */
	u32 DeviceId;	/**< Device ID of the sub-core */
	u32 AddrOffset;	/**< sub-core offset from subsystem base address */
} SubCoreCsi2Tx;

/**
 * MIPI CSI Tx Subsystem configuration structure.
 * Each subsystem device should have a configuration structure associated
 * that defines the MAX supported sub-cores within subsystem
 */
typedef struct {
	u32 DeviceId;	/**< DeviceId is the unique ID  of the device */
	UINTPTR BaseAddr; /**< BaseAddress is the physical base address of the
			  *  subsystem address range */
	UINTPTR HighAddr; /**< HighAddress is the physical MAX address of the
			  *  subsystem address range */
	u32 LanesPresent;	/**< Active Lanes programming optimization
				  *  enabled */
	u32 DataType;	/*< RAW8,RAW10,RAW12,RAW14,RGB888,RGB565,YUV422 8-bit */
	u32 PixelFormat;	/**< 1 - Single pixel per beat
				2 - Dual pixels per beat
				4 - Quad pixels per beat */
	u32 CsiBuffDepth;	/**< Line buffer Depth set */
	u32 DphyLineRate;	/**< DPHY Line Rate ranging from
					*  80-1500 Mbps */
	u32 IsDphyRegIntfcPresent;	/**< Flag for DPHY register interface
					  *  presence */
	u32 FEGenEnabled;	/**< Frame End Generation enabled flag */
	SubCoreCsi2Tx CsiInfo;	/**< CSI sub-core configuration */
	SubCoreCsi2Tx DphyInfo;	/**< DPHY sub-core configuration */
} XCsi2TxSs_Config;

/**
 * The XCsi2TxSs driver instance data. The user is required to allocate a
 * variable of this type for every XCsi2TxSs device in the system.
 * A pointer to a variable of this type is then passed to the driver API
 * functions.
 */
typedef struct {
	XCsi2TxSs_Config Config;	/**< Hardware configuration */
	u32 IsReady;		/**< Device and the driver instance are
				  *  initialized */
	XCsi2Tx  *CsiPtr;		/* handle to sub-core driver instance */
	XCsi2Tx_SPktData SpktData;		/**< Short packet */
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	XDphy *DphyPtr;		/**< handle to sub-core driver instance */
#endif

} XCsi2TxSs;

/************************** Function Prototypes ******************************/

/* Initialization function in xcsi2txss_sinit.c */
XCsi2TxSs_Config* XCsi2TxSs_LookupConfig(u32 DeviceId);

/* Initialization and control functions xcsi2txss.c */
u32 XCsi2TxSs_CfgInitialize(XCsi2TxSs *InstancePtr, XCsi2TxSs_Config *CfgPtr,
				UINTPTR EffectiveAddr);
u32 XCsi2TxSs_Configure(XCsi2TxSs *InstancePtr, u8 ActiveLanes, u32 IntrMask);
u32 XCsi2TxSs_Activate(XCsi2TxSs *InstancePtr, u8 Flag);
u32 XCsi2TxSs_Reset(XCsi2TxSs *InstancePtr);
void XCsi2TxSs_ReportCoreInfo(XCsi2TxSs *InstancePtr);
void XCsi2TxSs_GetShortPacket(XCsi2TxSs *InstancePtr);
void XCsi2TxSs_LineGen(XCsi2TxSs *InstancePtr, u32 Value);
void XCsi2TxSs_SetGSPEntry(XCsi2TxSs *InstancePtr, u32 Value);
u32 XCsi2TxSs_GetPixelMode(XCsi2TxSs *InstancePtr);
u32 XCsi2TxSs_GetMaxLaneCount(XCsi2TxSs *InstancePtr);
void XCsi2TxSs_SetClkMode(XCsi2TxSs *InstancePtr, u8 Mode);
u32 XCsi2TxSs_GetClkMode(XCsi2TxSs *InstancePtr);
u32 XCsi2TxSs_IsUlps(XCsi2TxSs *InstancePtr);
void XCsi2TxSs_SetUlps(XCsi2TxSs *InstancePtr, u32 Value);
u32 XCsi2TxSs_SetLineCountForVC(XCsi2TxSs *InstancePtr, u8 VC, u16 LineCount);
u32 XCsi2TxSs_GetLineCountForVC(XCsi2TxSs *InstancePtr, u8 VC, u16 *LineCount);
/* Self test function in xcsi2txss_selftest.c */
u32 XCsi2TxSs_SelfTest(XCsi2TxSs *InstancePtr);

/* TBD */
/* Interrupt functions in xcsi2txss_intr.c */
void XCsi2TxSs_IntrHandler(void *InstancePtr);
void XCsi2TxSs_IntrDisable(XCsi2TxSs *InstancePtr, u32 IntrMask);
u32 XCsi2TxSs_SetCallBack(XCsi2TxSs *InstancePtr, u32 HandlerType,
			void *CallbackFunc, void *CallbackRef);

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
