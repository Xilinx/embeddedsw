/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
* @file xdsi.h
* @addtogroup dsi_v1_1
* @{
* @details
*
* This file contains the implementation of the MIPI DSI TX Controller driver.
* User documentation for the driver functions is contained in this file in the
* form of comment blocks at the front of each function.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date    Changes
* --- --- ------- -------------------------------------------------------
* 1.0 ram 11/02/16 Initial Release for DSI driver
* 1.1 sss 08/17/16 Added 64 bit support
*     sss 08/26/16 XDSI_VM_NON_BURST_SYNC_PULSES enum changed
*                  Add "Command Queue Vacancy" API
*                  API for getting pixel format
*     ms  01/23/17 Modified xil_printf statement in main function for all
*                  examples to ensure that "Successfully ran" and "Failed"
*                  strings are available in all examples. This is a fix
*                  for CR-965028.
*     ms  03/17/17 Added readme.txt file in examples folder for doxygen
*                  generation.
*     ms  04/05/17 Modified Comment lines in functions of dsi
*                  examples to recognize it as documentation block
*                  for doxygen generation of examples.
* </pre>
*
******************************************************************************/

#ifndef XDSI_H_	/* prevent circular inclusions */
#define XDSI_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xdsi_hw.h"
#include "xvidc.h"

/************************** Constant Definitions *****************************/

/*
 * Interrupt Types for setting Callbacks
*/
#define XDSI_HANDLER_UNSUPPORT_DATATYPE		1
#define XDSI_HANDLER_PIXELDATA_UNDERRUN		2
#define XDSI_HANDLER_OTHERERROR			3
#define XDSI_HANDLER_CMDQ_FIFOFULL		4

typedef enum {
	XDSI_DISABLE, /* DSI Tx controller Disable */
	XDSI_ENABLE   /* DSI Tx controller Disable */
} XDsi_Selection;

/**************************** Type Definitions *******************************/

/**
* This typedef contains the Short Packet information from the Generic
* Short Packet Register
*
*/
typedef struct {
      u8 VcId;		/**< VcId indicates one of four channels */
      u8 DataType;	/**< Short Hand command issue
			from application */
      u8 Data0;		/**< First data byte*/
      u8 Data1;		/**< Second data byte*/
} XDsi_ShortPacket;

/**
 * Video Timing Mode by default Non-burst mode with Sync Events
 */
typedef enum {
	XDSI_VM_NON_BURST_SYNC_PULSES,
	XDSI_VM_NON_BURST_SYNC_EVENT,
	XDSI_VM_BURST_MODE,
	XDSI_VM_NUM_SUPPORTED
} XDsi_VideoMode;

/**
 * Video timing structure.
 */
typedef struct {
	u16 HActive;		/**< Active per video line payload
				  *  size in bytes(WC) */
	u16 HFrontPorch;	/**< Horizontal front porch blanking
				  *  packet payload size in bytes(WC) */
	u16 HSyncWidth;		/**< Horizontal Sync active width
				  *  blanking packet payload size in bytes(WC)*/
	u16 HBackPorch;		/**< Horizontal back porch blanking
				  *  packet payload size in bytes(WC) */
	u16 VActive;		/**< Vertical active region lines */
	u16 VFrontPorch;	/**< Vertical front porch lines */
	u16 VSyncWidth;		/**< Vertical sync active lines */
	u16 VBackPorch;		/**< Vertical back porch lines */
	u16 BLLPBurst;		/**< BLLP duration of VACT region
				  *  packet payload size in bytes(WC).*/
} XDsi_VideoTiming;

/**
 * The configuration structure for DSI Controller
 * This structure passes the hardware building information to the driver
 */
typedef struct {
	u32 DeviceId;		/**< Device Id */
	UINTPTR BaseAddr;	/**< Base address of DSI Controller */
	u8  DsiLanes;		/**< DSI supported lanes1, 2, 3, 4 */
	u8  DataType;		/**< RGB  type */
	u32 DsiByteFifo;	/**< DSI byte FIFO size 128, 256, 512,
				*  1024, 2048, 4096, 8192, 16384 */
	u8  CrcGen;		/**< CRC Generation enable status */
	u8  DsiPixel;		/**< Pixels per beat receive on input stream */
} XDsi_Config;

/**
* The structure to read DSI controller & Configurable Parameters
*/
typedef struct {
	XDsi_Config Config;	/**< GUI Configuration */
	u32  VideoMode;		/**< Specifies VideoMode
				  *  NoNBurst/Burst */
	u8   BlankPacketType;	/**< Blanking packet type
				  *  for BLLP region */
	u8   BLLPMode;		/**< BLLP Selection */
	u8   EoTp;		/**< EoTp Bit Enable/Disable */
	XDsi_VideoTiming Timing;/**< Video Timing structure */
	u32  LineTime;		/**< Total Line Time */
	u32  BLLPTime;		/**< BLLP Time by core */
} XDsi_ConfigParameters;

/*****************************************************************************/
/**
*
* Callback type for all interrupts defined.
*
* @param	CallBackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
* @param	Mask is a bit mask indicating the cause of the event. For
*		current core version, this parameter is "OR" of 0 or more
*		XDSI_ISR_*_MASK constants defined in xdsi_hw.h.
*
* @return	None.
*
* @note		None.
*
 *****************************************************************************/
typedef void (*XDsi_Callback) (void *CallbackRef, u32 Mask);

/**
* The XDsi driver instance data.
* An instance must be allocated for each DSI in use.
*/
typedef struct {
	XDsi_Config Config;		/**< GUI Configuration */

	u32  VideoMode;			/**< Specifies VideoMode
					  *  NoNBurst/Burst */
	u8   BlankPacketType;		/**< Blanking packet type
					  *  for BLLP region */
	u8   BLLPMode;			/**< BLLP Selection */

	u8   EoTp;			/**< EoTp Bit Enable/Disable */

	XDsi_Callback UnSupportedDataTypeCallback;	/**< Callback for
							UnSupportedDataType */
	void *UnsupportDataTypeRef;			/**< To be passed to the
							UnSupportedDataType
							call back*/
	XDsi_Callback PixelDataUnderrunCallback; /**< Callback invoked for
						   *  Byte stream FIFO starves
						   *  for Pixel during
						   *  HACT transmission */
	void *PixelDataUnderrundRef;	/**< To be passed for
					  *  Pixel under run  */
	XDsi_Callback CmdQFIFOFullCallback; /**< Callback invoked for
						   *  Command queue FIFO Full */
	void *CmdQFIFOFullRef;		/**< To be passed for
					  Command queue FIFO Full */
	XDsi_Callback ErrorCallback; /**< Call back function for
				       *  rest all errors */
	void *ErrRef; /**< To be passed to the Error Call back */
	u32 IsReady; /**< Driver is ready */
} XDsi;

/************************** Macros Definitions *******************************/


/************************* Bit field operations ******************************/

/*****************************************************************************/
/**
*
* This function is used to set bit in a register.
*
* @param	BaseAddress is a base address of IP
* @param	RegisterOffset is offset where the register is present
* @param	BitMask is the part of the bitname before _MASK
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XDsi_BitSet(UINTPTR BaseAddress,u32 RegisterOffset,u32 BitMask)
{
	XDsi_WriteReg(BaseAddress, RegisterOffset,
	(XDsi_ReadReg(BaseAddress, RegisterOffset) | BitMask));
}

/****************************************************************************/
/**
*
* This function is used to reset bit in a register.
*
* @param	BaseAddress is a base address of IP
* @param	RegisterOffset is offset where the register is present
* @param	BitMask is the part of the bitname before _MASK
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XDsi_BitReset(UINTPTR BaseAddress, u32 RegisterOffset,
							u32 BitMask)
{
	XDsi_WriteReg(BaseAddress, RegisterOffset,
	(XDsi_ReadReg(BaseAddress, RegisterOffset) & ~(BitMask)));
}

/****************************************************************************/
/**
*
* This function is used to get the value of bitfield from register.
*
* @param	BaseAddress is a base address of IP
* @param	RegisterOffset is offset where the register is present
* @param	BitMask is the part of the bitname before _MASK
* @param	BitShift is the part of the bitname before _SHIFT
* @return	Bit Field Value in u32 format
*
* @note 	None
*
****************************************************************************/
static inline u32 XDsi_GetBitField(UINTPTR BaseAddress, u32 RegisterOffset,
						u32 BitMask, u32 BitShift)
{
	return((XDsi_ReadReg(BaseAddress, RegisterOffset)
		 & BitMask) >> BitShift);
}

/****************************************************************************/
/**
*
* This function is used to set the value of bitfield from register.
*
* @param	BaseAddress is a base address of IP
* @param	RegisterOffset is offset where the register is present
* @param	BitMask is the part of the bitname before _MASK
* @param	BitShift is the part of the bitname before _SHIFT
* @param	Value is to be set. Passed in u32 format.
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XDsi_SetBitField(UINTPTR BaseAddress, u32 RegisterOffset,
				u32 BitMask, u32 BitShift, u32 Value)
{
	XDsi_WriteReg(BaseAddress, RegisterOffset,
		((XDsi_ReadReg(BaseAddress, RegisterOffset) &
		 ~ BitMask) | (Value << BitShift)));
}

/****************************************************************************/
/**
*
* This function is used to start the soft reset process by setting the
* soft reset bit in the Core Configuration Register.
*
* @param	InstancePtr is a pointer to the DSI2 instance to be
*		worked on.
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XDsi_SetSoftReset(XDsi *InstancePtr)
{
	XDsi_BitSet(InstancePtr->Config.BaseAddr,
		XDSI_CCR_OFFSET, XDSI_CCR_SOFTRESET_MASK);
}

/****************************************************************************/
/**
*
* This function is used to check if a controller is ready or
* still in reset stage
*
* @param	InstancePtr is a pointer to the DSI Instance to be
*		worked on.
*
* @return	TRUE or FALSE
*
* @note		None
*
****************************************************************************/
static inline u32 XDsi_IsControllerReady(XDsi *InstancePtr)
{
	return XDsi_GetBitField(InstancePtr->Config.BaseAddr,
	XDSI_CCR_OFFSET, XDSI_CCR_CRREADY_MASK, XDSI_CCR_CRREADY_SHIFT);
}

/****************************************************************************/
/**
*
* This function is used to stop the soft reset process by resetting the
* soft reset bit in the Core Configuration Register.This is done usually after
* Reset in Progress is 0.
*
* @param	InstancePtr is a pointer to the DSI Instance to be
*		worked on.
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XDsi_ClearSoftReset(XDsi *InstancePtr)
{
	XDsi_BitReset(InstancePtr->Config.BaseAddr,
		XDSI_CCR_OFFSET, XDSI_CCR_SOFTRESET_MASK);
}

/****************************************************************************/
/**
*
* This function is used to stop the packet processing by resetting
* Enable core bit in the Core Configuration Register.
*
* @param	InstancePtr is a pointer to the DSI Instance to be
*		worked on.
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XDsi_Disable(XDsi *InstancePtr)
{
	XDsi_BitReset(InstancePtr->Config.BaseAddr,
		XDSI_CCR_OFFSET, XDSI_CCR_COREENB_MASK);
}

/****************************************************************************/
/**
*
* This function is used to start the packet processing by setting
* Enable core bit in the Core Configuration Register.This is done after the
* configuration of active lanes, interrupt masks, etc.
*
* @param	InstancePtr is a pointer to the DSI Instance to be
*		worked on.
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XDsi_Enable(XDsi *InstancePtr)
{
	XDsi_BitSet(InstancePtr->Config.BaseAddr, XDSI_CCR_OFFSET,
			XDSI_CCR_COREENB_MASK);
}

/****************************************************************************/
/**
*
* This function is used to check if the Core is enabled by checking
* the core enable bit
*
* @param	InstancePtr is a pointer to the DSI Instance to be
*		worked on.
*
* @return	SET or RESET in u32 format
*
* @note		None
*
****************************************************************************/
static inline u32 XDsi_IsEnabled(XDsi *InstancePtr)
{
	return XDsi_GetBitField(InstancePtr->Config.BaseAddr,
	XDSI_CCR_OFFSET, XDSI_CCR_COREENB_MASK, XDSI_CCR_COREENB_SHIFT);
}

/****************************************************************************/
/**
*
* This function is used to enable the global interrupts. This is
* used after setting the interrupts mask before enabling the core.
*
* @param	InstancePtr is a pointer to the DSI Instance to be
*		worked on.
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XDsi_SetGlobalInterrupt(XDsi *InstancePtr)
{
	XDsi_BitSet(InstancePtr->Config.BaseAddr, XDSI_GIER_OFFSET,
					XDSI_GIER_GIE_MASK);
}

/****************************************************************************/
/**
*
* This function is used to disable the global interrupts. This is
* done after disabling the core.
*
* @param	InstancePtr is a pointer to the DSI Instance to be
*		worked on.
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XDsi_ResetGlobalInterrupt(XDsi *InstancePtr)
{
	XDsi_BitReset(InstancePtr->Config.BaseAddr,
		XDSI_GIER_OFFSET, XDSI_GIER_GIE_MASK);
}

/****************************************************************************/
/**
*
* This function is used to set interrupt mask to enable interrupts. This is
* done before enabling the core.
*
* @param	InstancePtr is a pointer to the DSI Instance to be
*		worked on.
*
* @param	Mask Interrupts to be enabled.
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XDsi_IntrEnable(XDsi *InstancePtr, u32 Mask)
{
	XDsi_WriteReg(InstancePtr->Config.BaseAddr,
			XDSI_IER_OFFSET, Mask & XDSI_IER_ALLINTR_MASK);
}

/****************************************************************************/
/**
*
* This function is used to find out which interrupts are registered for
*
* @param	InstancePtr is a pointer to the DSI Instance to be
*		worked on.
*
* @return	Bit Mask in u32 format
*
* @note		None
*
****************************************************************************/
static inline u32 XDsi_GetIntrEnableStatus(XDsi *InstancePtr)
{
	return XDsi_ReadReg(InstancePtr->Config.BaseAddr, XDSI_IER_OFFSET);
}

/****************************************************************************/
/**
*
* This function is used to disable interrupts.
*
* @param	InstancePtr is a pointer to the DSI Instance to be
*		worked on.
*
* @param	Mask Interrupts to be disabled.
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XDsi_IntrDisable(XDsi *InstancePtr, u32 Mask)
{
	XDsi_WriteReg(InstancePtr->Config.BaseAddr,
		XDSI_IER_OFFSET, ~  Mask & XDSI_IER_ALLINTR_MASK);
}

/****************************************************************************/
/**
*
* This function is used to find out which events have triggered the interrupt
* source, presently DSI supporting Undefined data type and pixel under flow
* error, the ISR register bits will set when respecive interuupt triggers
*
* @param	InstancePtr is a pointer to the DSI Instance to be
*		worked on.
*
* @return	Bit Mask in u32 format
*
* @note		None
*
****************************************************************************/
static inline u32 XDsi_GetIntrStatus(XDsi *InstancePtr)
{
	 return XDsi_ReadReg(InstancePtr->Config.BaseAddr, XDSI_ISR_OFFSET);
}

/****************************************************************************/
/**
*
* This function is used to acknowledge/clear the events. Once the interrupt
* occur the same value need to write in ISR register
*
* @param	InstancePtr is a pointer to the DSI Instance to be
*		worked on.
*
* @param	Value is Bit Mask for ack of interrupts
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XDsi_IntrClear(XDsi *InstancePtr, u32 Value)
{
	XDsi_WriteReg(InstancePtr->Config.BaseAddr, XDSI_ISR_OFFSET,
			(Value & XDSI_ISR_ALLINTR_MASK));
}

/****************************************************************************/
/**
*
* This function is used to get pixel format
*
* @param	InstancePtr is a pointer to the DSI Instance to be
*		worked on.
*
* @return	0x0E – Packed RGB565
*		0x1E- packed RGB666
*		0x2E – Loosely packed RGB666
*		0x3E- Packed RGB888
*		0x0B- Compressed Pixel Stream
*
* @note		None
*
****************************************************************************/
static inline u32 XDsi_GetPixelFormat(XDsi *InstancePtr)
{
	return XDsi_GetBitField(InstancePtr->Config.BaseAddr,
	XDSI_PCR_OFFSET, XDSI_PCR_PIXELFORMAT_MASK, XDSI_PCR_PIXELFORMAT_SHIFT);
}

/****************************************************************************/
/**
*
* This function is used to get Command queue Vacancy
*
* @param	InstancePtr is a pointer to the DSI Instance to be
*		worked on.
*
* @return	Number of command queue entries can be safely written
* 		to Command queue FIFO, before it goes full.
*
* @note		None
*
****************************************************************************/
static inline u32 XDsi_GetCmdQVacancy(XDsi *InstancePtr)
{
	return XDsi_GetBitField(InstancePtr->Config.BaseAddr,
	XDSI_STATUS_OFFSET, XDSI_CMDQ_MASK, XDSI_CMDQ_SHIFT);
}

/************************** Function Prototypes ******************************/

XDsi_Config *XDsi_LookupConfig(u32 DeviceId);
u32 XDsi_CfgInitialize(XDsi *InstancePtr, XDsi_Config *Config,
			UINTPTR EffectiveAddr);
u32 XDsi_Activate(XDsi *InstancePtr, XDsi_Selection Flag);
void XDsi_Reset(XDsi *InstancePtr);
u32 XDsi_DefaultConfigure(XDsi *InstancePtr);
u32 XDsi_SelfTest(XDsi *InstancePtr);
void XDsi_SendShortPacket(XDsi *InstancePtr, XDsi_ShortPacket *ShortPacket);
void XDsi_GetConfigParams(XDsi *InstancePtr,
		XDsi_ConfigParameters *ConfigInfo);
s32 XDsi_SetVideoInterfaceTiming(XDsi *InstancePtr, XDsi_VideoMode VideoMode,
			XVidC_VideoMode Resolution, u16 BurstPacketSize);
s32 XDsi_SetCustomVideoInterfaceTiming(XDsi *InstancePtr,
			XDsi_VideoMode VideoMode, XDsi_VideoTiming *Timing);
s32 XDsi_SetCallback(XDsi *InstancePtr, u32 HandleType,
			void *CallbackFunc, void *CallbackRef);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
