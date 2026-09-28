/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcsi2tx.h
* @addtogroup csi2tx Overview
* @{
* @details
*
* This file contains the implementation of the MIPI CSI2 TX Controller driver.
* User documentation for the driver functions is contained in this file in the
* form of comment blocks at the front of each function.
*
* <b>MIPI CSI2 Tx Overview</b>
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
* The Xilinx CSI-2 Tx has the following features:
* • Compliant with the MIPI CSI-2 Interface Specification, rev. 1.1
* • Standard PPI interface i.e. D-PHY
* • 1-4 Lane Support,configurable through GUI
* • Maximum Data Rate per – 1.5 Gigabits per second
* • Multiple data type support :
* • RAW8,RAW10,RAW12,RAW14,RGB888,YUV422-8Bit,User defined  Data types
* • Supports Single,Dual,Quad Pixel Modes, configurable through GUI
* • Virtual channel Support (1 to 4)
* • Low Power State(LPS) insertion between the packets.
* • Ultra Low Power(ULP) mode generation using register access.
* • Interrupt generation & Core Status information can be accessed through
	Register Interface
* • Multilane interoperability.
* • ECC generation for packet header.
* • CRC generation for data bytes(Can be Enabled / Disabled),
	configurable through GUI.
* • Pixel byte conversion based on data format.
* • AXI4-Lite interface to access core registers.
* • Compliant with Xilinx AXI Stream Interface & native
	Interface for input video stream.
* • LS/LE Packet Generation,can be configured through  register interface.
* • Configurable selection of D-PHY Register Interface through GUI options.
* • Support for transmission of Embedded Data packet’s through Input
	Interface.
*
*
* <b>Interrupts</b>
*
* The XCsi2Tx_SetCallBack() is used to register the call back functions
* for MIPI CSI2 Tx driver with the corresponding handles
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
* compile time, the NDEBUG identifier. By default, asserts are turned on and
* it is recommended that application developers leave asserts on during
* development.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 sss 07/15/16 Initial release
*     ms  01/23/17 Modified xil_printf statement in main function for all
*                  examples to ensure that "Successfully ran" and "Failed"
*                  strings are available in all examples. This is a fix
*                  for CR-965028.
*     ms  03/17/17 Added readme.txt file in examples folder for doxygen
*                  generation.
*     ms  04/05/17 Modified Comment lines in functions of csi2tx
*                  examples to recognize it as documentation block
*                  for doxygen generation of examples.
*     vsa 15/12/17 Add support for Clock Mode
* 1.1 vsa 02/28/18 Added Frame End Generation feature
* 1.2 ai  09/17/26 Added multi-VC (NUM_VC 4/8/16) support with per-VC line
*                  count handlers, bank-select helpers and 8 pixels-per-clock
*                  mode for CSI2 TX subsystem v3.0
* </pre>
*
******************************************************************************/

#ifndef XCSI2TX_H_
#define XCSI2TX_H_			/**< Prevent circular inclusions
					  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xstatus.h"
#include "xcsi2tx_hw.h"

/************************** Constant Definitions *****************************/

/** @name Interrupt Types for setting Callbacks
 * @{
*/
#define XCSI2TX_HANDLER_WRG_LANE		1
#define XCSI2TX_HANDLER_GSPFIFO_FULL		2
#define XCSI2TX_HANDLER_ULPS			3
#define XCSI2TX_HANDLER_LINEBUF_FULL		4
#define XCSI2TX_HANDLER_WRG_DATATYPE		5
#define XCSI2TX_HANDLER_UNDERRUN_PIXEL		6
#define XCSI2TX_HANDLER_LCERRVC0		7
#define XCSI2TX_HANDLER_LCERRVC1		8
#define XCSI2TX_HANDLER_LCERRVC2		9
#define XCSI2TX_HANDLER_LCERRVC3		10
#define XCSI2TX_HANDLER_LCERRVC4		11	/**< Line count error VC4 handler */
#define XCSI2TX_HANDLER_LCERRVC5		12	/**< Line count error VC5 handler */
#define XCSI2TX_HANDLER_LCERRVC6		13	/**< Line count error VC6 handler */
#define XCSI2TX_HANDLER_LCERRVC7		14	/**< Line count error VC7 handler */
#define XCSI2TX_HANDLER_LCERRVC8		15	/**< Line count error VC8 handler */
#define XCSI2TX_HANDLER_LCERRVC9		16	/**< Line count error VC9 handler */
#define XCSI2TX_HANDLER_LCERRVC10		17	/**< Line count error VC10 handler */
#define XCSI2TX_HANDLER_LCERRVC11		18	/**< Line count error VC11 handler */
#define XCSI2TX_HANDLER_LCERRVC12		19	/**< Line count error VC12 handler */
#define XCSI2TX_HANDLER_LCERRVC13		20	/**< Line count error VC13 handler */
#define XCSI2TX_HANDLER_LCERRVC14		21	/**< Line count error VC14 handler */
#define XCSI2TX_HANDLER_LCERRVC15		22	/**< Line count error VC15 handler */

/** @} */

#define XCSI2TX_ENABLE 	1	/**< Flag denoting enabling of CSI */
#define XCSI2TX_DISABLE	0	/**< Flag denoting disabling of CSI */

#define XCSI2TX_MAX_LANES	4	/**< Max Lanes supported by CSI */
#define XCSI2TX_INACTIVE	1
#define XCSI2TX_READY		0

#define XCSI2TX_MAX_VC		16 /**< Max number of Virtual Channels
				     *  supported (NUM_VC up to 16) */
#define XCSI2TX_DEFAULT_NUM_VC	4 /**< Default number of Virtual Channels
				    *  when not specified (older IP) */
#define XCSI2TX_VC_PER_BANK	8 /**< Virtual Channels mapped per bank */

/** @name Pixel per clock modes
 * @{
 */
#define XCSI2TX_PIXEL_MODE_8PPC	4 /**< 8 pixels-per-clock mode (PCR[16]) */
/** @} */
/**************************** Type Definitions *******************************/

/**
* This typedef contains the Short Packet information from the Generic
* Short Packet Register
*
*/
typedef struct {
	u16 Data;	/**< The Data from Camera Sensor */
	u8 DataType;	/**< The Data type like RGB, RAW or YUV format */
	u8 VirtualChannel;	/**< Virtual Channel (0-15) on which this
				  *  short packet is sent */
} XCsi2Tx_SPktData;

/**
* The configuration structure for CSI Controller
*
* This structure passes the hardware building information to the driver
*
*/
typedef struct {
	char *Name;
	UINTPTR BaseAddr;	/**< Base address of CSI2 Rx Controller */
	u32 MaxLanesPresent;	/**< Max value of Lanes. Range 0 - 3 */
	u32 ActiveLanes;	/**< Number of Lanes configured. Range 0 - 3 */
	u32 FEGenEnabled;	/**< Frame End generation enabled */
} XCsi2Tx_Config;

/**
 * This typedef defines the different errors codes for Line Count
 * status for a Virtual Channel when Frame End Generation is enabled
 */
typedef enum {
	XCSI2TX_LC_LESS_LINES = 1,	/**< Less no of lines recvd */
	XCSI2TX_LC_MORE_LINES	/**< More no of lines recvd */
} XCsi2Tx_LCStatus;

/**
*
* Callback type for all interrupts defined.
*
* @param	CallBackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
* @param	Mask is a bit mask indicating the cause of the event. For
*		current core version, this parameter is "OR" of 0 or more
*		XCSI2TX_ISR_*_MASK constants defined in xcsi_hw.h.
*
* @return	None
*
* @note		None
*
 *****************************************************************************/
typedef void (*XCsi2Tx_CallBack) (void *CallBackRef, u32 Mask);

/**
* The XCsi2Tx driver instance data.
* An instance must be allocated for each CSI in use.
*/
typedef struct {
	XCsi2Tx_Config Config;	/**< Hardware Configuration */
	u32 ActiveLanes;	/**< Number of Active Lanes */

	/* Interrupt Callbacks */
	XCsi2Tx_CallBack IncorrectLaneCallBack;	/**< Callback for incorrect
						  *  lane configuration */
	void *IncorrectLaneRef;	/**< To be passed to the incorrect
				  *  lane configuration */
	XCsi2Tx_CallBack GSPFIFOCallBack;	/**< Callback for Generic Short
						Packet FIFO full interrupts */
	void *GSPFIFORef;	/*To be passed to the Callback for Generic Short
				  *  interrupt callback */

	XCsi2Tx_CallBack DPhyUlpsCallBack;	/**< Callback for DPhy ULPS
						  *  interrupts */
	void *DPhyUlpsRef;	/**< DPhy ULPS interrupt callback */

	XCsi2Tx_CallBack LineBufferCallBack;	/* Callback for Line buffer full
						  *  interrupts */
	void *LineBufferRef;	/**< Line buffer full interrupt callback */
	XCsi2Tx_CallBack WrgDataTypeCallBack;	/*Callback for unsopported
						data types interrupts */
	void *WrgDataTypeRef;	/* unsopported data types interrupt callback */
	XCsi2Tx_CallBack UnderrunPixelCallBack;	/*Callback for Pixel data
						  underrun interrupts */
	void *UnderrunPixelRef;	/**< Pixel data underrun interrupt callback */
	/* Per-Virtual-Channel Line Count Status error callbacks, indexed by
	 * physical VC (0 to XCSI2TX_MAX_VC-1). VC0-VC3 are used by all IP
	 * versions; VC4-VC15 are used only on the multi-VC (NUM_VC 8/16) IP.
	 */
	XCsi2Tx_CallBack LineCountErrVC[XCSI2TX_MAX_VC]; /**< Line Count Status
						  *  error callback per VC */
	void *LCErrVCRef[XCSI2TX_MAX_VC];	/**< Reference passed to the
						  *  Line Count Status error
						  *  callback per VC */

	u32 IsReady; /**< Driver is ready */
} XCsi2Tx;

/************************* Bit field operations ******************************/

/*****************************************************************************/
/**
*
* This function is used to set bit in a register.
*
* @param	BaseAddress is a base address of IP
* @param	RegisterOffset is offset where the register is present
* @param	BitMask of bit to be set
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XCsi2Tx_BitSet(UINTPTR BaseAddress, u32 RegisterOffset,
				u32 BitMask)
{
	XCsi2Tx_WriteReg(BaseAddress, RegisterOffset,
	(XCsi2Tx_ReadReg(BaseAddress, RegisterOffset) | BitMask));
}

/****************************************************************************/
/**
*
* This function is used to reset bit in a register.
*
* @param	BaseAddress is a base address of IP
* @param	RegisterOffset is offset where the register is present
* @param	BitMask of bit to be reset
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XCsi2Tx_BitReset(UINTPTR BaseAddress, u32 RegisterOffset,
					u32 BitMask)
{
	XCsi2Tx_WriteReg(BaseAddress, RegisterOffset,
	(XCsi2Tx_ReadReg(BaseAddress, RegisterOffset) & ~(BitMask)));
}

/****************************************************************************/
/**
*
* This function is used to get the value of bitfield from register.
*
* @param	BaseAddress is a base address of IP
* @param	RegisterOffset is offset where the register is present
* @param	BitMask of bit fields to be retrieved
* @param	BitShift is offset of the field in register
*
* @return 	Bit Field Value in u32 format
*
* @note 	None
*
****************************************************************************/
static inline u32 XCsi2Tx_GetBitField(UINTPTR BaseAddress, u32 RegisterOffset,
					u32 BitMask, u32 BitShift)
{
	return((XCsi2Tx_ReadReg(BaseAddress, RegisterOffset)
		 & BitMask) >> BitShift);
}

/****************************************************************************/
/**
*
* This function is used to set the value of bitfield from register.
*
* @param	BaseAddress is a base address of IP
* @param	RegisterOffset is offset where the register is present
* @param	BitMask of bit fields to be retrieved
* @param	BitShift is offset of the field in register
* @param	Value is to be set. Passed in u32 format.
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XCsi2Tx_SetBitField(UINTPTR BaseAddress, u32 RegisterOffset,
					u32 BitMask, u32 BitShift, u32 Value)
{
	XCsi2Tx_WriteReg(BaseAddress, RegisterOffset,
		((XCsi2Tx_ReadReg(BaseAddress, RegisterOffset) &
		 ~ BitMask) | (Value << BitShift)));
}

/****************************************************************************/
/**
*
* This function is used to start the soft reset process by setting the
* soft reset bit in the Core Configuration Register.
*
* @param	InstancePtr is a pointer to the CSI2 instance to be
*		worked on.
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XCsi2Tx_SetSoftReset(XCsi2Tx *InstancePtr)
{
	XCsi2Tx_BitSet(InstancePtr->Config.BaseAddr, XCSI2TX_CCR_OFFSET,
	XCSI2TX_CCR_SOFTRESET_MASK);
}

/****************************************************************************/
/**
*
* This function is used to stop the soft reset process by resetting the
* soft reset bit in the Core Configuration Register.This is done usually after
* Reset in Progress is 0.
*
* @param	InstancePtr is a pointer to the CSI Instance to be
*		worked on.
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XCsi2Tx_ClearSoftReset(XCsi2Tx *InstancePtr)
{
	XCsi2Tx_BitReset(InstancePtr->Config.BaseAddr,
			XCSI2TX_CCR_OFFSET, XCSI2TX_CCR_SOFTRESET_MASK);
}

/****************************************************************************/
/**
*
* This function is used to stop the packet processing by resetting
* Enable core bit in the Core Configuration Register.
*
* @param	InstancePtr is a pointer to the CSI Instance to be
*		worked on.
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XCsi2Tx_Disable(XCsi2Tx *InstancePtr)
{
	XCsi2Tx_BitReset(InstancePtr->Config.BaseAddr,
		XCSI2TX_CCR_OFFSET, XCSI2TX_CCR_COREENB_MASK);
}

/****************************************************************************/
/**
*
* This function is used to start the packet processing by setting
* Enable core bit in the Core Configuration Register.This is done after the
* configuration of active lanes, interrupt masks, etc.
*
* @param	InstancePtr is a pointer to the CSI Instance to be
*		worked on.
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XCsi2Tx_Enable(XCsi2Tx *InstancePtr)
{
	XCsi2Tx_BitSet(InstancePtr->Config.BaseAddr, XCSI2TX_CCR_OFFSET,
			XCSI2TX_CCR_COREENB_MASK);
}

/****************************************************************************/
/**
*
* This function is used to check if the Core is enabled by checking
* the core enable bit
*
* @param	InstancePtr is a pointer to the CSI Instance to be
*		worked on.
*
* @return
* 		- 1 - If the CSI core is enabled
*		- 0 - If the CSI core is disabled
*
* @note		None
*
****************************************************************************/
static inline u32 XCsi2Tx_IsCsiEnabled(XCsi2Tx *InstancePtr)
{
	return XCsi2Tx_GetBitField(InstancePtr->Config.BaseAddr,
					XCSI2TX_CCR_OFFSET,
					XCSI2TX_CCR_COREENB_MASK,
					XCSI2TX_CCR_COREENB_SHIFT);
}

/****************************************************************************/
/**
*
* This function is used to check if lanes are in ulps mode
*
* @param	InstancePtr is a pointer to the CSI Instance to be
*		worked on.
*
* @return
* 		- 1 - ulps mode enter
*		- 0 - ulps mode exit
*
* @note		None
*
****************************************************************************/
static inline u32 XCsi2Tx_IsUlps(XCsi2Tx *InstancePtr)
{
	return XCsi2Tx_GetBitField(InstancePtr->Config.BaseAddr,
	XCSI2TX_CCR_OFFSET, XCSI2TX_CCR_ULPS_MASK, XCSI2TX_CCR_ULPS_SHIFT);
}

/****************************************************************************/
/**
*
* This function is used to set lanes in ulps mode
*
* @param	InstancePtr is a pointer to the CSI Instance to be
*		worked on.
*
* @param	Value
* 		- 1 - ulps mode enter
*		- 0 - ulps mode exit
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XCsi2Tx_SetUlps(XCsi2Tx *InstancePtr, u32 Value)
{
	XCsi2Tx_SetBitField(InstancePtr->Config.BaseAddr,
	XCSI2TX_CCR_OFFSET, XCSI2TX_CCR_ULPS_MASK, XCSI2TX_CCR_ULPS_SHIFT,
								Value);
}

/****************************************************************************/
/**
*
* This function is used to get the number of lanes configured in
* the IP.
*
* @param	InstancePtr is a pointer to the CSI Instance to be
*		worked on.
*
* @return	Max number of lanes available in u32 format
*
* @note		None
*
****************************************************************************/
static inline u32 XCsi2Tx_GetMaxLaneCount(XCsi2Tx *InstancePtr)
{
	return XCsi2Tx_GetBitField(InstancePtr->Config.BaseAddr,
	XCSI2TX_PCR_OFFSET, XCSI2TX_PCR_MAXLANES_MASK,
					XCSI2TX_PCR_MAXLANES_SHIFT);
}

/****************************************************************************/
/**
*
* This function is used to get the Pixel Mode
*
* @param	InstancePtr is a pointer to the CSI Instance to be
*		worked on.
*
* @return	0x0  - Single Pixel Mode
* 		0x1  - Dual Pixel Mode
* 		0x3  - Quad Pixel Mode
* 		0x4  - 8 Pixels per clock Mode (XCSI2TX_PIXEL_MODE_8PPC)
*
* @note		The 8 pixels-per-clock mode is reported via PCR[16] on the
*		CSI2 TX subsystem v3.0 IP.
*
****************************************************************************/
static inline u32 XCsi2Tx_GetPixelMode(XCsi2Tx *InstancePtr)
{
	if (XCsi2Tx_GetBitField(InstancePtr->Config.BaseAddr,
			XCSI2TX_PCR_OFFSET, XCSI2TX_PCR_PIXEL8_MASK,
			XCSI2TX_PCR_PIXEL8_SHIFT)) {
		return XCSI2TX_PIXEL_MODE_8PPC;
	}

	return XCsi2Tx_GetBitField(InstancePtr->Config.BaseAddr,
	XCSI2TX_PCR_OFFSET, XCSI2TX_PCR_PIXEL_MASK, XCSI2TX_PCR_PIXEL_SHIFT);
}

/****************************************************************************/
/**
*
* This function is to get the Line Synchronization packet Generation status
*
* @param	InstancePtr is a pointer to the CSI Instance to be
*		worked on.
*
* @return	0 : Do not generate
* 		1 : Generate
*
* @note		None
*
****************************************************************************/
static inline u32 XCsi2Tx_IsLineGen(XCsi2Tx *InstancePtr)
{
	return XCsi2Tx_GetBitField(InstancePtr->Config.BaseAddr,
	XCSI2TX_PCR_OFFSET, XCSI2TX_PCR_LINEGEN_MASK,
					XCSI2TX_PCR_LINEGEN_SHIFT);
}

/****************************************************************************/
/**
*
* This function is to set the Line Synchronization packet Generation status
*
* @param	InstancePtr is a pointer to the CSI Instance to be
*		worked on.
*
* @param	Value
* 		0 : Do not generate
* 		1 : Generate
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XCsi2Tx_SetLineGen(XCsi2Tx *InstancePtr, u32 Value)
{
	XCsi2Tx_SetBitField(InstancePtr->Config.BaseAddr,
	XCSI2TX_PCR_OFFSET, XCSI2TX_PCR_LINEGEN_MASK,
			XCSI2TX_PCR_LINEGEN_SHIFT, Value);
}

/****************************************************************************/
/**
*
* This function is used to get the actual number of lanes being
* used by the IP to communicate with a CSI2 Tx.
* This is lesser or equal to Max lanes.
*
* @param	InstancePtr is a pointer to the CSI Instance to be
*		worked on.
*
* @return	Number of lanes being used in u32 format
*
* @note		None
*
****************************************************************************/
static inline u32 XCsi2Tx_GetActiveLaneCount(XCsi2Tx *InstancePtr)
{
	return XCsi2Tx_GetBitField(InstancePtr->Config.BaseAddr,
	XCSI2TX_PCR_OFFSET, XCSI2TX_PCR_ACTLANES_MASK,
					XCSI2TX_PCR_ACTLANES_SHIFT);
}

/****************************************************************************/
/**
*
* This function is used to set the actual number of lanes to be
* used to communicate with a CSI2 Tx. This is lesser than or equal to
* Max lanes.
*
* @param	InstancePtr is a pointer to the CSI Instance to be
*		worked on.
* @param	Value is number of lanes to be made active
*
* @return	None
*
* @note		None
*
****************************************************************************/
 static inline void XCsi2Tx_SetActiveLaneCount(XCsi2Tx *InstancePtr, u32 Value)
{
	XCsi2Tx_SetBitField(InstancePtr->Config.BaseAddr, XCSI2TX_PCR_OFFSET,
	XCSI2TX_PCR_ACTLANES_MASK, XCSI2TX_PCR_ACTLANES_SHIFT, Value);
}

/****************************************************************************/
/**
*
* This function is used to check if a reset is completed or is in progress.
*
* @param	InstancePtr is a pointer to the CSI Instance to be
*		worked on.
*
* @return
*		- 1 - If the Soft reset operation is in progress
*		- 0 - If the Soft reset operation is completed
*
* @note		None
*
****************************************************************************/
static inline u32 XCsi2Tx_IsSoftResetInProgress(XCsi2Tx *InstancePtr)
{
	u32 value;
	value = XCsi2Tx_GetBitField(InstancePtr->Config.BaseAddr,
	XCSI2TX_CCR_OFFSET, XCSI2TX_CSR_RIPCD_MASK, XCSI2TX_CSR_RIPCD_SHIFT);
	if (value)
		return XCSI2TX_READY;
	else
		return XCSI2TX_INACTIVE;
}

/****************************************************************************
*
* This function is used to enable the global interrupts. This is
* used after setting the interrupts mask before enabling the core.
*
* @param	InstancePtr is a pointer to the CSI Instance to be
*		worked on.
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XCsi2Tx_SetGlobalInterrupt(XCsi2Tx *InstancePtr)
{
	XCsi2Tx_BitSet(InstancePtr->Config.BaseAddr, XCSI2TX_GIER_OFFSET,
			XCSI2TX_GIER_GIE_MASK);
}

/****************************************************************************/
/**
*
* This function is used to disable the global interrupts. This is
* done after disabling the core.
*
* @param	InstancePtr is a pointer to the CSI Instance to be
*		worked on.
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XCsi2Tx_ResetGlobalInterrupt(XCsi2Tx *InstancePtr)
{
	XCsi2Tx_BitReset(InstancePtr->Config.BaseAddr,
	XCSI2TX_GIER_OFFSET, XCSI2TX_GIER_GIE_MASK);
}

/****************************************************************************/
/**
*
* This function is used to get the Short Packet Status
*
* @param	InstancePtr is a pointer to the CSI Instance to be
*		worked on.
*
* @return	Number of Generic Short Packets can
* 		be safely written to Generic Short packet
* 		FIFO, before it goes full.
*
* @note		None
*
****************************************************************************/
static inline u32 XCsi2Tx_GetGSPCount(XCsi2Tx *InstancePtr)
{
	return XCsi2Tx_GetBitField(InstancePtr->Config.BaseAddr,
	XCSI2TX_GSP_OFFSET, XCSI2TX_GSP_MASK, XCSI2TX_GSP_SHIFT);
}

/****************************************************************************/
/**
*
* This function is used to set value to Generic Short Packet Entry
*
* @param	InstancePtr is a pointer to the CSI Instance to be
*		worked on.
*
* @param	Value
* 		31:26 Reserved
*		25:24 VC extension - upper 2 bits of VC for 16 VC support
*		23:16 Byte 1 of short packet
*		15:8  Byte 0 of short packet
*		7:6   VC value of short packet (lower 2 bits)
*		5:0   Short packet data type
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XCsi2Tx_SetGSPEntry(XCsi2Tx *InstancePtr, u32 Value)
{
	XCsi2Tx_WriteReg(InstancePtr->Config.BaseAddr, XCSI2TX_SPKTR_OFFSET,
									Value);
}

/****************************************************************************/
/**
*
* This function is used to set the clock mode
*
* @param	InstancePtr is a pointer to the CSI Instance to be
*		worked on.
*
* @param	Value
* 		- 0 - continuous clock mode
*		- 1 - non-continuous clock mode
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XCsi2Tx_SetClkMode(XCsi2Tx *InstancePtr, u8 Value)
{
	XCsi2Tx_SetBitField(InstancePtr->Config.BaseAddr,
		XCSI2TX_CCR_OFFSET, XCSI2TX_CCR_CLKMODE_MASK,
		XCSI2TX_CCR_CLKMODE_SHIFT, Value);
}

/****************************************************************************/
/**
*
* This function is used to get the clock mode
*
* @param	InstancePtr is a pointer to the CSI Instance to be
*		worked on.
*
* @return
* 		- 0 - Continuous Clock Mode
* 		- 1 - Non-continuous Clock Mode
*
* @note		None
*
****************************************************************************/
static inline u32 XCsi2Tx_GetClkMode(XCsi2Tx *InstancePtr)
{
	return XCsi2Tx_GetBitField(InstancePtr->Config.BaseAddr,
			XCSI2TX_CCR_OFFSET, XCSI2TX_CCR_CLKMODE_MASK,
			XCSI2TX_CCR_CLKMODE_SHIFT);
}

/****************************************************************************/
/**
*
* This function selects the Line Count register bank in the Core Configuration
* Register. Bank 0 maps VC0-VC7 and bank 1 maps VC8-VC15 onto the Line Count
* registers. This bit is only honoured when the core is built with NUM_VC=16;
* it reads back as 0 and ignores writes otherwise.
*
* @param	InstancePtr is a pointer to the CSI Instance to be worked on.
* @param	Bank is 0 (VC0-VC7) or 1 (VC8-VC15).
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XCsi2Tx_SetLineCountBank(XCsi2Tx *InstancePtr, u8 Bank)
{
	XCsi2Tx_SetBitField(InstancePtr->Config.BaseAddr, XCSI2TX_CCR_OFFSET,
		XCSI2TX_CCR_LCBANKSEL_MASK, XCSI2TX_CCR_LCBANKSEL_SHIFT, Bank);
}

/****************************************************************************/
/**
*
* This function selects the Virtual Channel interrupt bank in the Interrupt
* Status Register. Bank 0 maps VC0-VC7 and bank 1 maps VC8-VC15 onto the Line
* Count status/enable fields. This bit is only honoured when the core is built
* with NUM_VC=16; it reads back as 0 and ignores writes otherwise.
*
* @param	InstancePtr is a pointer to the CSI Instance to be worked on.
* @param	Bank is 0 (VC0-VC7) or 1 (VC8-VC15).
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XCsi2Tx_SetVcIsrBank(XCsi2Tx *InstancePtr, u8 Bank)
{
	XCsi2Tx_SetBitField(InstancePtr->Config.BaseAddr, XCSI2TX_ISR_OFFSET,
		XCSI2TX_ISR_VCBANKSEL_MASK, XCSI2TX_ISR_VCBANKSEL_SHIFT, Bank);
}

/****************************************************************************/
/**
*
* This function returns the Virtual Channel interrupt bank select bit read
* back from the Interrupt Status Register. It reads back as 1 only on cores
* built with NUM_VC=16, and is used to gate bank-1 (VC8-VC15) processing.
*
* @param	InstancePtr is a pointer to the CSI Instance to be worked on.
*
* @return	1 if bank 1 is selected, 0 otherwise.
*
* @note		None
*
****************************************************************************/
static inline u32 XCsi2Tx_GetVcIsrBank(XCsi2Tx *InstancePtr)
{
	return XCsi2Tx_GetBitField(InstancePtr->Config.BaseAddr,
		XCSI2TX_ISR_OFFSET, XCSI2TX_ISR_VCBANKSEL_MASK,
		XCSI2TX_ISR_VCBANKSEL_SHIFT);
}

/************************** Function Prototypes ******************************/

/* Initialization function in xcsi2tx_sinit.c */
XCsi2Tx_Config *XCsi2Tx_LookupConfig(UINTPTR BaseAddress);

/* Initialization and control functions in xcsi2tx.c */
u32 XCsi2Tx_CfgInitialize(XCsi2Tx *InstancePtr, XCsi2Tx_Config *Config,
			UINTPTR EffectiveAddr);
u32 XCsi2Tx_Configure(XCsi2Tx *InstancePtr);
u32 XCsi2Tx_Activate(XCsi2Tx *InstancePtr, u8 Flag);
u32 XCsi2Tx_Reset(XCsi2Tx *InstancePtr);
void XCsi2Tx_GetShortPacket(XCsi2Tx *InstancePtr,
		XCsi2Tx_SPktData *ShortPacketStruct);
u8 XCsi2Tx_IsActiveLaneCountValid(XCsi2Tx *InstancePtr, u8 ActiveLanesCount);
u32 XCsi2Tx_SetLineCountForVC(XCsi2Tx *InstancePtr, u8 VC, u16 LineCount);
u32 XCsi2Tx_GetLineCountForVC(XCsi2Tx *InstancePtr, u8 VC, u16 *LineCount);

/* Self test function in xcsi2tx_selftest.c */
u32 XCsi2Tx_SelfTest(XCsi2Tx *InstancePtr);

/* Interrupt functions in xcsi2tx_intr.c */
void XCsi2Tx_IntrHandler(void *InstancePtr);
int XCsi2Tx_SetCallBack(XCsi2Tx *InstancePtr, u32 HandleType,
			void *Callbackfunc, void *Callbackref);
u32 XCsi2Tx_GetIntrEnable(XCsi2Tx *InstancePtr);
void XCsi2Tx_IntrEnable(XCsi2Tx *InstancePtr, u32 Mask);
void XCsi2Tx_IntrDisable(XCsi2Tx *InstancePtr, u32 Mask);
u32 XCsi2Tx_GetIntrStatus(XCsi2Tx *InstancePtr);
void XCsi2Tx_InterruptClear(XCsi2Tx *InstancePtr, u32 Mask);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
