/******************************************************************************
*
* Copyright (C) 2015 - 2016 Xilinx, Inc. All rights reserved.
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
/*****************************************************************************/
/**
*
* @file xcsi.h
* @addtogroup csi_v1_0
* @{
* @details
*
* This file contains the implementation of the MIPI CSI2 RX Controller driver.
* User documentation for the driver functions is contained in this file in the
* form of comment blocks at the front of each function.
*
* <b>Core Features</b>
*
* The CSI2 Rx Controller currently supports the MIPI?Alliance Specification
* for Camera Serial Interface 2 (CSI-2) Version 1.1 ? 18 July 2012
* with D-PHY v1.2.
*
* There is a provision for multi-lane support for upto 4 lanes for receiving
* data upto 1.5 Gbps per lane. It supports both short and long packets.
* Most of the primary and secondary formats of images are supported along with
* interleaving of data at packet or frame level for upto 4 virtual channels
* in the stream. The IP has capability to detect Start of Transmission, CRC,
* ECC, etc type of errors and report them via interrupts.
*
* The IP is programmable when the core is disabled or when the soft reset bit
* has been set and the reset-in-progress is reset.
*
* The programmable parameters are the number of active lane counts, global
* interrupt enable and interrupt masks for variety of events to be detected.
*
* <b>Software Initialization & Configuration</b>
*
* The application needs to do following steps in order for preparing the
* MIPI CSI2 Rx core to be ready to receive stream
*
* - Call XCsi_LookupConfig using a device ID to find the core configuration.
* - Call XCsi_CfgInitialize to initialize the device and the driver
*   instance associated with it.
* - Call XCsi_Configure to set the Active Lanes
* - Register callback functions for interrupts
* - Enable interrupts
* - Call XCsi_Activate to activate the core
*
* <b> Interrupts </b>
*
* There are certain events for which interrupts are received if unmasked like
*	- Frame received
*	- Incorrect Lane Configuration
*	- Short Packet FIFO Full or Not Empty
*	- Stream Line Buffer Full
*	- Stop State
*	- Start of Transmission (Sync) Errors
*	- CRC and ECC errors
*	- Invalid Data ID
*	- Frame Sync and Level Errors for Virtual Channels
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
* 1.0 vsa 06/16/15 Initial release
* 1.1 sss 08/17/16 Added 64 bit support
*     ms  01/23/17 Modified xil_printf statement in main function for all
*                  examples to ensure that "Successfully ran" and "Failed"
*                  strings are available in all examples. This is a fix
*                  for CR-965028.
*     ms  03/17/17 Added readme.txt file in examples folder for doxygen
*                  generation.
*     ms  04/05/17 Modified Comment lines in functions of csi
*                  examples to recognize it as documentation block
*                  for doxygen generation of examples.
* </pre>
*
******************************************************************************/

#ifndef XCSI_H_
#define XCSI_H_			/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xstatus.h"
#include "xcsi_hw.h"

/************************** Constant Definitions *****************************/

/** @name Interrupt Types for setting Callbacks
 * @{
*/
#define XCSI_HANDLER_DPHY		1
#define XCSI_HANDLER_PKTLVL		2
#define XCSI_HANDLER_PROTLVL		3
#define XCSI_HANDLER_SHORTPACKET	4
#define XCSI_HANDLER_FRAMERECVD		5
#define XCSI_HANDLER_OTHERERROR		6

/*@}*/

#define XCSI_ENABLE 	1	/**< Flag denoting enabling of CSI */
#define XCSI_DISABLE	0	/**< Flag denoting disabling of CSI */

#define XCSI_MAX_LANES	4	/**< Max Lanes supported by CSI */
#define XCSI_MAX_VC	4	/**< Max Virtual Channels supported by CSI */

/**************************** Type Definitions *******************************/

/**
* This typedef contains the Short Packet information from the Generic
* Short Packet Register
*
*/
typedef struct {
	u16 Data;	/**< The Data from Camera Sensor */
	u8 DataType;	/**< The Data type like RGB, RAW or YUV format */
	u8 VirtualChannel;	/**< Virtual Channel on which this
				  *  short packet is sent */
} XCsi_SPktData;

/**
* This typdef contains the structure for getting the information about
* a Virtual Channel
*/
typedef struct {
	u16 LineCount;	/**< Line Count from Image Info 1 register */
	u16 ByteCount;	/**< Byte Count from Image Info 1 register */
	u8 DataType;	/**< Data Type like RGB, YUV, RAW from
			  *  Image Info 2 register */
} XCsi_VCInfo;

/**
* This typdef contains the structure for getting the information about
* the Clock Lane
*/
typedef struct {
	u8 StopState;	/**< Clock Lane is in Stop State */
} XCsi_ClkLaneInfo;

/**
* This typdef contains the structure for getting the information about
* the Data Lane
*/
typedef struct {
	u8 StopState;	/**< Data Lane is in Stop State */
	u8 SoTErr;	/**< Detected Start Of Transmission High Speed Error */
	u8 SoTSyncErr;	/**< Detected SoT Synchronization Error */
} XCsi_DataLaneInfo;

/**
* The configuration structure for CSI Controller
*
* This structure passes the hardware building information to the driver
*
*/
typedef struct {
	u32 DeviceId;		/**< Device Id */
	UINTPTR BaseAddr;	/**< Base address of CSI2 Rx Controller */
	u32 MaxLanesPresent;	/**< Number of Lanes. Range 1 - 4 */
	u32 HasOffloadNonImageSupport;	/**< Offload non image data
					  *  via separate stream */
	u32 HasVCSupport;	/**< Supports Virtual Channels */
	u32 FixedVC;		/**< Fixed Virtual Channel number filter */
	u32 FixedLanes;		/**< Fixed Active Lanes */
} XCsi_Config;

/**
*
* Callback type for all interrupts defined.
*
* @param	CallBackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
* @param	Mask is a bit mask indicating the cause of the event. For
*		current core version, this parameter is "OR" of 0 or more
*		XCSI_ISR_*_MASK constants defined in xcsi_hw.h.
*
* @return	None.
*
* @note		None.
*
 *****************************************************************************/
typedef void (*XCsi_CallBack) (void *CallBackRef, u32 Mask);

/**
* The XCsi driver instance data.
* An instance must be allocated for each CSI in use.
*/
typedef struct {
	XCsi_Config Config;	/**< Hardware Configuration */
	u32 ActiveLanes;	/**< Number of Active Lanes */
	/* Interrupt Callbacks */
	XCsi_CallBack ShortPacketCallBack;	/**< Callback for Short Packet
						  *  interrupts */
	void *ShortPacketRef;	/**< To be passed to the Short Packet
				  *  interrupt callback */
	XCsi_CallBack FrameRecvdCallBack;	/**< Callback for Frame Received
						  *  interrupt */
	void *FrameRecvdRef;	/**< To be passed to the Frame Received
				  *  interrupt callback */
	XCsi_CallBack DPhyLvlErrCallBack;	/**< Callback for Error at the
						  *  DPhy level as per spec */
	void *DPhyLvlErrRef;	/**< To be passed to the DPhy Level Error
				  *  Call back function */
	XCsi_CallBack PktLvlErrCallBack;	/**< Callback for Packet Level
						  *  Error as per CSI Spec */
	void *PktLvlErrRef;	/**< To be passed to the Packet Level Error
				  *  Call back function */
	XCsi_CallBack ProtDecodeErrCallBack;	/**< Callback for Protocol
						  *  Decoding Level errors
						  *  as per CSI Spec*/
	void *ProtDecErrRef;	/**< To be passed to the Protocol Decoding Level
				  *  Error call back function */
	XCsi_CallBack ErrorCallBack;	/**< Call back function for rest all errors
					  *  like Stream Line Buffer Full,
					  *  Stop State errors */
	void *ErrRef; /**< To be passed to the Error Call back */
	u32 IsReady; /**< Driver is ready */
} XCsi;

/************************** Macros Definitions *******************************/


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
static inline void XCsi_BitSet(UINTPTR BaseAddress, u32 RegisterOffset,
				u32 BitMask)
{
	XCsi_WriteReg(BaseAddress, RegisterOffset,
	(XCsi_ReadReg(BaseAddress, RegisterOffset) | BitMask));
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
static inline void XCsi_BitReset(UINTPTR BaseAddress, u32 RegisterOffset,
					u32 BitMask)
{
	XCsi_WriteReg(BaseAddress, RegisterOffset,
	(XCsi_ReadReg(BaseAddress, RegisterOffset) & ~(BitMask)));
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
static inline u32 XCsi_GetBitField(UINTPTR BaseAddress, u32 RegisterOffset,
					u32 BitMask, u32 BitShift)
{
	return((XCsi_ReadReg(BaseAddress, RegisterOffset)
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
static inline void XCsi_SetBitField(UINTPTR BaseAddress, u32 RegisterOffset,
					u32 BitMask, u32 BitShift, u32 Value)
{
	XCsi_WriteReg(BaseAddress, RegisterOffset,
		((XCsi_ReadReg(BaseAddress, RegisterOffset) &
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
static inline void XCsi_SetSoftReset(XCsi *InstancePtr)
{
	XCsi_BitSet(InstancePtr->Config.BaseAddr, XCSI_CCR_OFFSET,
	XCSI_CCR_SOFTRESET_MASK);
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
static inline void XCsi_ClearSoftReset(XCsi *InstancePtr)
{
	XCsi_BitReset(InstancePtr->Config.BaseAddr,
			XCSI_CCR_OFFSET, XCSI_CCR_SOFTRESET_MASK);
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
static inline void XCsi_Disable(XCsi *InstancePtr)
{
	XCsi_BitReset(InstancePtr->Config.BaseAddr,
		XCSI_CCR_OFFSET, XCSI_CCR_COREENB_MASK);
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
static inline void XCsi_Enable(XCsi *InstancePtr)
{
	XCsi_BitSet(InstancePtr->Config.BaseAddr, XCSI_CCR_OFFSET,
			XCSI_CCR_COREENB_MASK);
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
static inline u32 XCsi_IsCsiEnabled(XCsi *InstancePtr)
{
	return XCsi_GetBitField(InstancePtr->Config.BaseAddr,
	XCSI_CCR_OFFSET, XCSI_CCR_COREENB_MASK, XCSI_CCR_COREENB_SHIFT);
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
static inline u32 XCsi_GetMaxLaneCount(XCsi *InstancePtr)
{
	return XCsi_GetBitField(InstancePtr->Config.BaseAddr,
	XCSI_PCR_OFFSET, XCSI_PCR_MAXLANES_MASK, XCSI_PCR_MAXLANES_SHIFT);
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
static inline u32 XCsi_GetActiveLaneCount(XCsi *InstancePtr)
{
	return XCsi_GetBitField(InstancePtr->Config.BaseAddr,
	XCSI_PCR_OFFSET, XCSI_PCR_ACTLANES_MASK, XCSI_PCR_ACTLANES_SHIFT);
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
 static inline void XCsi_SetActiveLaneCount(XCsi *InstancePtr, u32 Value)
{
	XCsi_SetBitField(InstancePtr->Config.BaseAddr, XCSI_PCR_OFFSET,
	XCSI_PCR_ACTLANES_MASK, XCSI_PCR_ACTLANES_SHIFT, Value);
}

/****************************************************************************/
/**
*
* This function is used to get the number of long packets received.
*
* @param	InstancePtr is a pointer to the CSI Instance to be
*		worked on.
*
* @return	Number of Packets received
*
* @note		None
*
****************************************************************************/
 static inline u32 XCsi_GetCurrentPacketCount(XCsi *InstancePtr)
{
	return XCsi_GetBitField(InstancePtr->Config.BaseAddr,
	XCSI_CSR_OFFSET, XCSI_CSR_PKTCOUNT_MASK, XCSI_CSR_PKTCOUNT_SHIFT);
}

/****************************************************************************/
/**
*
* This function is used to check if the Short Packet FIFO is full or not
*
* @param	InstancePtr is a pointer to the CSI Instance to be
*		worked on.
*
* @return
*		- 1 - If the Short Packet FIFO Full
*		- 0 - If the Short Packet FIFO avilable
*
* @note		None
*
****************************************************************************/
 static inline u32 XCsi_IsShortPacketFIFOFull(XCsi *InstancePtr)
{
	return XCsi_GetBitField(InstancePtr->Config.BaseAddr,
	XCSI_CSR_OFFSET, XCSI_CSR_SPFIFOFULL_MASK, XCSI_CSR_SPFIFOFULL_SHIFT);
}

/****************************************************************************/
/**
*
* This function is used to check if the Short Packet FIFO is not empty
*
* @param	InstancePtr is a pointer to the CSI Instance to be
*		worked on.
*
* @return
*		- 1 - If the Short Packet FIFO is not Empty
*		- 0 - If the Short Packet FIFO is Empty
*
* @note		None
*
****************************************************************************/
 static inline u32 XCsi_IsShortPacketFIFONotEmpty(XCsi *InstancePtr)
{
	return XCsi_GetBitField(InstancePtr->Config.BaseAddr,
	XCSI_CSR_OFFSET, XCSI_CSR_SPFIFONE_MASK, XCSI_CSR_SPFIFONE_SHIFT);
}

/****************************************************************************/
/**
*
* This function is used to check if the Stream Line Buffer is
* full or not.
*
* @param	InstancePtr is a pointer to the CSI Instance to be
*		worked on.
*
* @return
*		- 1 - If the Stream line buffer is full
*		- 0 - If the Stream line buffer is not full
*
* @note		None
*
****************************************************************************/
 static inline u32 XCsi_IsStreamLineBuffFull(XCsi *InstancePtr)
{
	return XCsi_GetBitField(InstancePtr->Config.BaseAddr,
	XCSI_CSR_OFFSET, XCSI_CSR_SLBF_MASK, XCSI_CSR_SLBF_SHIFT);
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
static inline u32 XCsi_IsSoftResetInProgress(XCsi *InstancePtr)
{
	return XCsi_GetBitField(InstancePtr->Config.BaseAddr,
	XCSI_CSR_OFFSET, XCSI_CSR_RIPCD_MASK, XCSI_CSR_RIPCD_SHIFT);
}

/****************************************************************************/
/**
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
static inline void XCsi_SetGlobalInterrupt(XCsi *InstancePtr)
{
	XCsi_BitSet(InstancePtr->Config.BaseAddr, XCSI_GIER_OFFSET,
			XCSI_GIER_GIE_MASK);
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
static inline void XCsi_ResetGlobalInterrupt(XCsi *InstancePtr)
{
	XCsi_BitReset(InstancePtr->Config.BaseAddr,
	XCSI_GIER_OFFSET, XCSI_GIER_GIE_MASK);
}

/************************** Function Prototypes ******************************/

/* Initialization function in xcsi_sinit.c */
XCsi_Config *XCsi_LookupConfig(u32 DeviceId);

/* Initialization and control functions in xcsi.c */
u32 XCsi_CfgInitialize(XCsi *InstancePtr, XCsi_Config *Config,
			UINTPTR EffectiveAddr);
u32 XCsi_Configure(XCsi *InstancePtr);
u32 XCsi_Activate(XCsi *InstancePtr, u8 Flag);
u32 XCsi_Reset(XCsi *InstancePtr);
void XCsi_GetShortPacket(XCsi *InstancePtr, XCsi_SPktData *ShortPacketStruct);
void XCsi_GetClkLaneInfo(XCsi *InstancePtr, XCsi_ClkLaneInfo *ClkLane);
void XCsi_GetDataLaneInfo(XCsi *InstancePtr, u8 Lane,
				XCsi_DataLaneInfo *DataLane);
void XCsi_GetVCInfo(XCsi *InstancePtr, u8 Vc, XCsi_VCInfo *VCInfo);
u8 XCsi_IsActiveLaneCountValid(XCsi *InstancePtr, u8 ActiveLanesCount);

/* Self test function in xcsi_selftest.c */
u32 XCsi_SelfTest(XCsi *InstancePtr);

/* Interrupt functions in xcsi_intr.c */
void XCsi_IntrHandler(void *InstancePtr);
int XCsi_SetCallBack(XCsi *InstancePtr, u32 HandleType,
			void *Callbackfunc, void *Callbackref);
u32 XCsi_GetIntrEnable(XCsi *InstancePtr);
void XCsi_IntrEnable(XCsi *InstancePtr, u32 Mask);
void XCsi_IntrDisable(XCsi *InstancePtr, u32 Mask);
u32 XCsi_GetIntrStatus(XCsi *InstancePtr);
void XCsi_InterruptClear(XCsi *InstancePtr, u32 Mask);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
