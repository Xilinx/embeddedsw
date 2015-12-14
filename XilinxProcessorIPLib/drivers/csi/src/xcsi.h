/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
*
* @mainpage csi_v1_0
*
* This file contains the implementation of the MIPI CSI2 RX Controller driver.
* User documentation for the driver functions is contained in this file in the
* form of comment blocks at the front of each function.
*
* The CSI2 Rx Controller currently supports the MIPI�Alliance Specification
* for Camera Serial Interface 2 (CSI-2) Version 1.1 � 18 July 2012
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
* <b> Interrupts </b>
*
* There are certain events for which interrupts are received if unmasked like
*	- Frame received
*	- Incorrect Lane Configuration
*	- Short Packet FIFO Full or Not Empty
*	- Stream Line Buffer Full
*	- Stop State
*	- Escape from ULPS
*	- Start of Transmission (Sync) Errors
*	- CRC and ECC errors
*	- Invalid Data ID
*	- Frame Sync and Level Errors for Virtual Channels
*
* <b> Examples </b>
*
* There is currently no example code provided
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
* 1.00a vs   06/16/15 First release
* </pre>
*
******************************************************************************/

#ifndef XCSI_H_   /* prevent circular inclusions */
#define XCSI_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#ifdef __MICROBLAZE__
#include "xenv.h"
#else
#include "xil_types.h"
#include "xil_cache.h"
#endif

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

/**************************** Type Definitions *******************************/

/**
* This typedef contains the Short Packet information from the Generic
* Short Packet Register
*
*/
typedef struct {
	u16 Data; /**< The Data from Camera Sensor */
	u8 DataType; /**< The Data type like RGB, RAW or YUV format */
	u8 VirtualChannel; /**< Virtual Channel on which this
			     *  short packet is sent */
} XCsi_SPktData;

/**
* This typdef contains the structure for getting the information about
* a Virtual Channel
*/
typedef struct {
	u16 LineCount; /**< Line Count from Image Info 1 register */
	u16 ByteCount; /**< Byte Count from Image Info 1 register */
	u8 DataType; /**< Data Type like RGB, YUV, RAW from
			*  Image Info 2 register */
} XCsi_VCInfo;

/**
* This typdef contains the structure for getting the information about
* the Clock Lane
*/
typedef struct {
	u8 StopState; /**< Clock Lane is in Stop State */
	u8 ULPS; /**< Clock Lane in Ultra Low power state */
} XCsi_ClkLaneInfo;

/**
* This typdef contains the structure for getting the information about
* the Data Lane
*/
typedef struct {
	u8 StopState; /**< Data Lane is in Stop State */
	u8 ULPS; /**< Data Lane in Ultra Low power state */
	u8 EscErr; /**< Detected Escape Error */
	u8 CtrlErr; /**< Detected Control Error */
	u8 SoTErr; /**< Detected Start Of Transmission High Speed Error */
	u8 SoTSyncErr; /**< Detected SoT Synchronization Error */
} XCsi_DataLaneInfo;

/**
* The configuration structure for CSI Controller
*
* This structure passes the hardware building information to the driver
*
*/
typedef struct {
	u32 DeviceId;		/**< Device Id */
	u32 BaseAddr;		/**< Base address of CSI2 Rx Controller */

	u32 MaxLanesPresent;	/**< Number of Lanes. Range 0 - 3 */
	u32 HasOffloadNonImageSupport;	/**< Offload non image data
					via separate stream */
	u32 HasVCSupport;	/**< Supports Virtual Channels */
	u32 FixedVC;		/**< Fixed Virtual Channel number filter */

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

	u32 ActiveLanes;    /**< Number of Active Lanes */

	/* Interrupt Callbacks */
	XCsi_CallBack ShortPacketCallBack; /**< Callback for Short Packet
					       *  interrupts */
	void *ShortPacketRef;	/**< To be passed to the Short Packet
				  *  interrupt callback */

	XCsi_CallBack FrameRecvdCallBack; /**< Callback for Frame Received
					       *  interrupt */
	void *FrameRecvdRef;	/**< To be passed to the Frame Received
				  *  interrupt callback */

	XCsi_CallBack DPhyLvlErrCallBack; /**< Callback for Error at the
					    *  DPhy level as per spec */
	void *DPhyLvlErrRef;	/**< To be passed to the DPhy Level Error
				  *  Call back function */

	XCsi_CallBack PktLvlErrCallBack; /**< Callback for Packet Level Error
					   *  as per CSI Spec */
	void *PktLvlErrRef;	/**< To be passed to the Packet Level Error
				  *  Call back function */

	XCsi_CallBack ProtDecodeErrCallBack; /**< Callback for Protocol Decoding
					       *  Level errors as per CSI Spec*/
	void *ProtDecErrRef; /**< To be passed to the Protocol Decoding Level
			       *  Error call back function */

	XCsi_CallBack ErrorCallBack; /**< Call back function for rest all errors
				       *  like Stream Line Buffer Full,
				       *  Stop State, Esc Error and
				       *  ULPM errors */
	void *ErrRef; /**< To be passed to the Error Call back */

	u32 IsReady; /**< Driver is ready */
} XCsi;

/************************** Macros Definitions *******************************/

/************************* Bit field operations ******************************/
/* Setting and resetting bits */
/*****************************************************************************/
/**
*
* XCSI_BIT_SET is used to set bit in a register.
*
* @param        BaseAddress is a base address of IP
*
* @param	RegisterOffset is offset where the register is present
*
* @param	BitName is the part of the bitname before _MASK
*
* @return	None
*
* @note         None
*
****************************************************************************/
#define XCSI_BIT_SET(BaseAddress, RegisterOffset, BitName)	\
			do	\
			{	\
				XCsi_WriteReg((BaseAddress), (RegisterOffset), \
					(XCsi_ReadReg((BaseAddress), \
							(RegisterOffset)) | \
							(BitName##_MASK))); \
			} \
		        while (0);

/****************************************************************************/
/**
*
* XCSI_BIT_RESET is used to reset bit in a register.
*
* @param        BaseAddress is a base address of IP
*
* @param	RegisterOffset is offset where the register is present
*
* @param	BitName is the part of the bitname before _MASK
*
* @return	None
*
* @note         None
*
****************************************************************************/
#define XCSI_BIT_RESET(BaseAddress, RegisterOffset, BitName) \
			do \
			{ \
				XCsi_WriteReg((BaseAddress), (RegisterOffset), \
					(XCsi_ReadReg((BaseAddress), \
						      (RegisterOffset) ) & \
							 ~(BitName##_MASK))); \
			} \
			while (0);

/****************************************************************************/
/**
*
* XCSI_GET_BITFIELD_VALUE is used to get the value of bitfield from register.
*
* @param        BaseAddress is a base address of IP
*
* @param	RegisterOffset is offset where the register is present
*
* @param	BitName is the part of the bitname before _MASK or _SHIFT
*
* @return	Bit Field Value in u32 format
*
* @note         C-style signature:
*			u32 XCSI_GET_BITFIELD_VALUE(u32 BaseAddress,
*						   u32 RegisterOffset,
*						   u32 BitMask,
*						   u32 BitOffset)
*		The bit mask and bit offset are generated from the common
*		part of the bit name by appending _MASK and _SHIFT
*
****************************************************************************/
#define XCSI_GET_BITFIELD_VALUE(BaseAddress, RegisterOffset, BitName) \
		((XCsi_ReadReg((BaseAddress), (RegisterOffset)) \
		  & (BitName##_MASK)) >> (BitName##_SHIFT))

/****************************************************************************/
/**
*
* XCSI_SET_BITFIELD_VALUE is used to set the value of bitfield from register.
*
* @param        BaseAddress is a base address of IP
*
* @param	RegisterOffset is offset where the register is present
*
* @param	BitName is the part of the bitname before _MASK or _SHIFT
*
* @param	Value is to be set. Passed in u32 format.
*
* @return	None
*
* @note         C-style signature:
*		u32 XCSI_SET_BITFIELD_VALUE(u32 BaseAddress,
*					   u32 RegisterOffset,
*					   u32 BitMask,
*					   u32 BitOffset,
*					   u32 Value)
*		The bit mask and bit offset are generated from the common
*		part of the bit name by appending _MASK and _SHIFT
*
****************************************************************************/
#define XCSI_SET_BITFIELD_VALUE(BaseAddress, RegisterOffset, BitName, Value)\
		XCsi_WriteReg((BaseAddress), (RegisterOffset), \
			((XCsi_ReadReg((BaseAddress), (RegisterOffset))	& \
			  ~(BitName##_MASK)) | ((Value) << (BitName##_SHIFT))))

/* Soft Reset Operations */
/****************************************************************************/
/**
*
* XCsi_SetSoftReset is used to start the soft reset process by setting the
* soft reset bit in the Core Configuration Register.
*
* @param        InstancePtr is a pointer to the CSI2 instance to be
*               worked on.
*
*
* @return	None
*
* @note         C-style signature:
*		void XCsi_SetSoftReset(XCsi * InstancePtr)
*
****************************************************************************/
#define XCsi_SetSoftReset(InstancePtr)	\
		XCSI_BIT_SET((InstancePtr)->Config.BaseAddr, \
				(XCSI_CCR_OFFSET), XCSI_CCR_SOFTRESET);

/****************************************************************************/
/**
*
* XCsi_ResetSoftReset is used to stop the soft reset process by resetting the
* soft reset bit in the Core Configuration Register.This is done usually after
* Reset in Progress is 0.
*
* @param        InstancePtr is a pointer to the CSI Instance to be
*               worked on.
*
* @return	None
*
* @note         C-style signature:
*		void XCsi_ResetSoftReset(XCsi * InstancePtr)
*
****************************************************************************/
#define XCsi_ResetSoftReset(InstancePtr) \
		XCSI_BIT_RESET((InstancePtr)->Config.BaseAddr, \
				(XCSI_CCR_OFFSET), XCSI_CCR_SOFTRESET);

/* Core Enable Operations */
/****************************************************************************/
/**
*
* XCsi_DisableCore is used to stop the packet processing by resetting
* Enable core bit in the Core Configuration Register.
*
* @param        InstancePtr is a pointer to the CSI Instance to be
*               worked on.
*
* @return	None
*
* @note         C-style signature:
*		void XCsi_DisableCore(XCsi * InstancePtr)
*
****************************************************************************/
#define XCsi_DisableCore(InstancePtr) \
		XCSI_BIT_RESET((InstancePtr)->Config.BaseAddr, \
				(XCSI_CCR_OFFSET), XCSI_CCR_COREENB);

/****************************************************************************/
/**
*
* XCsi_EnableCore is used to start the packet processing by setting
* Enable core bit in the Core Configuration Register.This is done after the
* configuration of active lanes, interrupt masks, etc.
*
* @param        InstancePtr is a pointer to the CSI Instance to be
*               worked on.
*
* @return	None
*
* @note         C-style signature:
*		void XCsi_EnableCore(XCsi * InstancePtr)
*
****************************************************************************/
#define XCsi_EnableCore(InstancePtr) \
		XCSI_BIT_SET((InstancePtr)->Config.BaseAddr, XCSI_CCR_OFFSET, \
				XCSI_CCR_COREENB);

/****************************************************************************/
/**
*
* XCsi_IsCoreEnabled is used to check if the Core is enabled by checking
* the core enable bit
*
* @param        InstancePtr is a pointer to the CSI Instance to be
*               worked on.
*
* @return	SET or RESET in u32 format
*
* @note         C-style signature:
*		u32 XCsi_IsCoreEnabled(XCsi * InstancePtr)
*
****************************************************************************/
#define XCsi_IsCoreEnabled(InstancePtr) \
		XCSI_GET_BITFIELD_VALUE((InstancePtr)->Config.BaseAddr, \
				XCSI_CCR_OFFSET, XCSI_CCR_COREENB)

/****************************************************************************/
/**
*
* XCsi_GetMaxLaneCount is used to get the number of lanes configured in
* the IP.
*
* @param        InstancePtr is a pointer to the CSI Instance to be
*               worked on.
*
* @return	Max number of lanes available in u32 format
*
* @note         C-style signature:
*		u32 XCsi_GetMaxLaneCount(XCsi * InstancePtr)
*
****************************************************************************/
#define XCsi_GetMaxLaneCount(InstancePtr) \
		XCSI_GET_BITFIELD_VALUE((InstancePtr)->Config.BaseAddr, \
				XCSI_PCR_OFFSET, XCSI_PCR_MAXLANES)

/****************************************************************************/
/**
*
* XCsi_GetActiveLaneCount is used to get the actual number of lanes being
* used by the IP to communicate with a CSI2 Tx.This is lesser than Max lanes.
*
* @param        InstancePtr is a pointer to the CSI Instance to be
*               worked on.
*
* @return	Number of lanes being used in u32 format
*
* @note         C-style signature:
*		u32 XCsi_GetActiveLaneCount(XCsi * InstancePtr)
*
****************************************************************************/
#define XCsi_GetActiveLaneCount(InstancePtr) \
		XCSI_GET_BITFIELD_VALUE((InstancePtr)->Config.BaseAddr, \
				XCSI_PCR_OFFSET, XCSI_PCR_ACTLANES)

/****************************************************************************/
/**
*
* XCsi_SetActiveLaneCount is used to set the actual number of lanes to be
* used to communicate with a CSI2 Tx. This is lesser than Max lanes.
*
* @param        InstancePtr is a pointer to the CSI Instance to be
*               worked on.
*
* @param	Value is number of lanes to be made active
*
* @return	None
*
* @note         C-style signature:
*		void XCsi_SetActiveLaneCount(XCsi *InstancePtr, u32 Value)
*
****************************************************************************/
#define XCsi_SetActiveLaneCount(InstancePtr, Value) \
		XCSI_SET_BITFIELD_VALUE((InstancePtr)->Config.BaseAddr, \
			XCSI_PCR_OFFSET, XCSI_PCR_ACTLANES, Value)

/****************************************************************************/
/**
*
* XCsi_GetCurrentPacketCount is used to get the number of long packets
* received.
*
* @param        InstancePtr is a pointer to the CSI Instance to be
*               worked on.
*
* @return	Number of Packets received
*
* @note         C-style signature:
*		u32 XCsi_GetCurrentPacketCount(XCsi *InstancePtr)
*
****************************************************************************/
#define XCsi_GetCurrentPacketCount(InstancePtr) \
		XCSI_GET_BITFIELD_VALUE((InstancePtr)->Config.BaseAddr, \
				XCSI_CSR_OFFSET, XCSI_CSR_PKTCOUNT)

/****************************************************************************/
/**
*
* XCsi_IsShortPacketFIFOFull is used to check if the Short Packet FIFO is
* full or not
*
* @param        InstancePtr is a pointer to the CSI Instance to be
*               worked on.
*
* @return	TRUE or FALSE
*
* @note         C-style signature:
*		BOOL XCsi_IsShortPacketFIFOFull(XCsi *InstancePtr)
*
****************************************************************************/
#define XCsi_IsShortPacketFIFOFull(InstancePtr) \
		XCSI_GET_BITFIELD_VALUE((InstancePtr)->Config.BaseAddr, \
				XCSI_CSR_OFFSET, XCSI_CSR_SPFIFOFULL)

/****************************************************************************/
/**
*
* XCsi_IsShortPacketFIFONotEmpty is used to check if the Short Packet FIFO
* is not empty
*
* @param        InstancePtr is a pointer to the CSI Instance to be
*               worked on.
*
* @return	TRUE or FALSE
*
* @note         C-style signature:
*		BOOL XCsi_IsShortPacketFIFONotEmpty(XCsi *InstancePtr)
*
****************************************************************************/
#define XCsi_IsShortPacketFIFONotEmpty(InstancePtr) \
		XCSI_GET_BITFIELD_VALUE((InstancePtr)->Config.BaseAddr, \
				XCSI_CSR_OFFSET, XCSI_CSR_SPFIFONE)

/****************************************************************************/
/**
*
* XCsi_IsStreamLineBuffFull is used to check if the Stream Line Buffer is
* full or not.
*
* @param        InstancePtr is a pointer to the CSI Instance to be
*               worked on.
*
* @return	TRUE or FALSE
*
* @note         C-style signature:
*		BOOL XCsi_IsStreamLineBuffFull(XCsi *InstancePtr)
*
****************************************************************************/
#define XCsi_IsStreamLineBuffFull(InstancePtr) \
		XCSI_GET_BITFIELD_VALUE((InstancePtr)->Config.BaseAddr, \
				XCSI_CSR_OFFSET, XCSI_CSR_SLBF)

/****************************************************************************/
/**
*
* XCsi_IsSoftResetInProgress is used to check if a reset is completed or
* is in progress.
*
* @param        InstancePtr is a pointer to the CSI Instance to be
*               worked on.
*
* @return	TRUE or FALSE
*
* @note         C-style signature:
*		BOOL XCsi_IsSoftResetInProgress(XCsi *InstancePtr)
*
****************************************************************************/
#define XCsi_IsSoftResetInProgress(InstancePtr) \
		XCSI_GET_BITFIELD_VALUE((InstancePtr)->Config.BaseAddr, \
				XCSI_CSR_OFFSET, XCSI_CSR_RIPCD)

/****************************************************************************/
/**
*
* XCsi_SetGlobalInterrupt is used to enable the global interrupts. This is
* used after setting the interrupts mask before enabling the core.
*
* @param        InstancePtr is a pointer to the CSI Instance to be
*               worked on.
*
* @return	none
*
* @note         C-style signature:
*		void XCsi_SetGlobalInterrupt(XCsi *InstancePtr)
*
****************************************************************************/
#define XCsi_SetGlobalInterrupt(InstancePtr) \
		XCSI_BIT_SET((InstancePtr)->Config.BaseAddr, XCSI_GIER_OFFSET, \
				XCSI_GIER_GIE)

/****************************************************************************/
/**
*
* XCsi_ResetGlobalInterrupt is used to disable the global interrupts. This is
* done after disabling the core.
*
* @param        InstancePtr is a pointer to the CSI Instance to be
*               worked on.
*
* @return	none
*
* @note         C-style signature:
*		void XCsi_ResetGlobalInterrupt(XCsi *InstancePtr)
*
****************************************************************************/
#define XCsi_ResetGlobalInterrupt(InstancePtr) \
		XCSI_BIT_RESET((InstancePtr)->Config.BaseAddr, \
				XCSI_GIER_OFFSET, XCSI_GIER_GIE)

/****************************************************************************/
/**
*
* XCsi_IntrEnable is used to set interrupt mask to enable interrupts. This is
* done before enabling the core.
*
* @param        InstancePtr is a pointer to the CSI Instance to be
*               worked on.
*
* @param	Mask Interrupts to be enabled.
*
* @return	none
*
* @note         C-style signature:
*		void XCsi_IntrEnable(XCsi *InstancePtr, u32 Mask)
*
****************************************************************************/
#define XCsi_IntrEnable(InstancePtr, Mask) \
		XCsi_WriteReg((InstancePtr)->Config.BaseAddr, \
				(XCSI_IER_OFFSET), \
				(Mask) & (XCSI_IER_ALLINTR_MASK))

/****************************************************************************/
/**
*
* XCsi_GetIntrEnable is used to find out which interrupts are registered for
*
* @param        InstancePtr is a pointer to the CSI Instance to be
*               worked on.
*
* @return	Bit Mask in u32 format
*
* @note         C-style signature:
*		u32 XCsi_GetIntrEnable(XCsi *InstancePtr)
*
****************************************************************************/
#define XCsi_GetIntrEnable(InstancePtr) \
		XCsi_ReadReg((InstancePtr)->Config.BaseAddr, (XCSI_IER_OFFSET))

/****************************************************************************/
/**
*
* XCsi_IntrDisable is used to disable interrupts. This is after disabling
* the core.
*
* @param        InstancePtr is a pointer to the CSI Instance to be
*               worked on.
*
* @param	Mask Interrupts to be disabled.
*
* @return	none
*
* @note         C-style signature:
*		void XCsi_IntrDisable(XCsi *InstancePtr, u32 Mask)
*
****************************************************************************/
#define XCsi_IntrDisable(InstancePtr, Mask) \
		XCsi_WriteReg((InstancePtr)->Config.BaseAddr, \
				(XCSI_IER_OFFSET), \
				~((Mask) & (XCSI_IER_ALLINTR_MASK)))

/****************************************************************************/
/**
*
* XCsi_IntrGetIrq is used to find out which events have triggered the interrupt
*
* @param        InstancePtr is a pointer to the CSI Instance to be
*               worked on.
*
* @return	Bit Mask in u32 format
*
* @note         C-style signature:
*		u32 XCsi_IntrGetIrq(XCsi *InstancePtr)
*
****************************************************************************/
#define XCsi_IntrGetIrq(InstancePtr) \
		XCsi_ReadReg((InstancePtr)->Config.BaseAddr, (XCSI_ISR_OFFSET))

/****************************************************************************/
/**
*
* XCsi_IntrAckIrq is acknowledge the events.
*
* @param        InstancePtr is a pointer to the CSI Instance to be
*               worked on.
*
* @param	Value is Bit Mask for ack of interrupts
*
* @return	None
*
* @note         C-style signature:
*		u32 XCsi_IntrAckIrq(XCsi *InstancePtr, u32 Value)
*
****************************************************************************/
#define XCsi_IntrAckIrq(InstancePtr, Value) \
		XCsi_WriteReg((InstancePtr)->Config.BaseAddr, \
				(XCSI_ISR_OFFSET), \
				((Value) & (XCSI_ISR_ALLINTR_MASK)))

/****************************************************************************/
/**
*
* XCsi_GetGenericShortPacket is a wrapper to read the Short Packet register.
* Generic Short Packet Register will be read once for all fields with
* XCsi_ReadReg() as its a FIFO register and accessing register for each
* bit field will remove a value from the FIFO.
*
* @param        InstancePtr is a pointer to the CSI Instance to be
*               worked on.
*
* @return	u32 format with Short packet register contents
*
* @note         C-style signature:
*		u32 XCsi_GetGenericShortPacket(XCsi *InstancePtr)
*
*
****************************************************************************/
#define XCsi_GetGenericShortPacket(InstancePtr)  \
		XCsi_ReadReg((InstancePtr)->Config.BaseAddr, \
				(XCSI_SPKTR_OFFSET))

/************************** Function Prototypes ******************************/

XCsi_Config *XCsi_LookupConfig(u32 DeviceId);

u32 XCsi_CfgInitialize(XCsi *InstancePtr, XCsi_Config *Config,
			u32 EffectiveAddr);

u32 XCsi_Configure(XCsi *InstancePtr);

void XCsi_Activate(XCsi *InstancePtr, u8 Flag);

u32 XCsi_Reset(XCsi *InstancePtr);

u32 XCsi_SelfTest(XCsi *InstancePtr);

void XCsi_GetShortPacket(XCsi *InstancePtr, XCsi_SPktData *ShortPacketStruct);

void XCsi_GetClkLaneInfo(XCsi *InstancePtr, XCsi_ClkLaneInfo *ClkLane);

void XCsi_GetDataLaneInfo(XCsi *InstancePtr, u8 Lane,
				XCsi_DataLaneInfo *DataLane);

void XCsi_GetVCInfo(XCsi *InstancePtr, u8 Vc, XCsi_VCInfo *VCInfo);

void XCsi_InterruptEnable(XCsi *InstancePtr, u32 Mask);

void XCsi_InterruptDisable(XCsi *InstancePtr, u32 Mask);

u32 XCsi_InterruptGetEnabled(XCsi *InstancePtr);

u32 XCsi_InterruptGetStatus(XCsi *InstancePtr);

void XCsi_InterruptClear(XCsi *InstancePtr, u32 Mask);

void XCsi_IntrHandler(void *InstancePtr);

int XCsi_SetCallBack(XCsi *InstancePtr, u32 HandleType,
			void *CallBackFunc, void *CallBackRef);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
