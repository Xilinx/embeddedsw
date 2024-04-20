/******************************************************************************
* Copyright 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdsi2rx.h
* @addtogroup dsi Overview
* @{
* @details
*
* This file contains the implementation of the MIPI DSI2 RX Controller driver.
* User documentation for the driver functions is contained in this file in the
* form of comment blocks at the front of each function.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who   Date     Changes
* --- ---   -------  -------------------------------------------------------
* 1.0 Kunal 18/04/24 Initial Release for DSI2RX driver
* </pre>
*
******************************************************************************/

#ifndef XDSI2RX_H_	/* prevent circular inclusions */
#define XDSI2RX_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xdsi2rx_hw.h"
#include "xvidc.h"

/************************** Constant Definitions *****************************/

/*
 * Interrupt Types for setting Callbacks
*/
#define XDSI2RX_HANDLER_UNSUPPORT_DATATYPE		1
#define XDSI2RX_HANDLER_CRC_ERROR			2
#define XDSI2RX_HANDLER_ECC1_BIT_ERROR			3
#define XDSI2RX_HANDLER_ECC2_BIT_ERROR			4
#define XDSI2RX_HANDLER_SOT_SYNC_ERR_LANE1		5
#define XDSI2RX_HANDLER_SOT_ERR_LANE1			6
#define XDSI2RX_HANDLER_SOT_SYNC_ERR_LANE2		7
#define XDSI2RX_HANDLER_SOT_ERR_LANE2			8
#define XDSI2RX_HANDLER_SOT_SYNC_ERR_LANE3		9
#define XDSI2RX_HANDLER_SOT_ERR_LANE3			10
#define XDSI2RX_HANDLER_SOT_SYNC_ERR_LANE4		11
#define XDSI2RX_HANDLER_SOT_ERR_LANE4			12
#define XDSI2RX_HANDLER_STOP_STATE			13
#define XDSI2RX_HANDLER_LM_ASYNC_FIFO_FULL		14
#define XDSI2RX_HANDLER_STREAM_ASYNC_FIFO_FULL		15
#define XDSI2RX_HANDLER_GSP_FIFO_NE			16
#define XDSI2RX_HANDLER_GSP_FIFO_FULL			17
#define XDSI2RX_HANDLER_FRAME_STARTED			18

typedef enum {
	XDSI2RX_DISABLE, /* DSI Tx controller Disable */
	XDSI2RX_ENABLE   /* DSI Tx controller Disable */
} XDsi2Rx_Selection;

/**************************** Type Definitions *******************************/

/**
 * The configuration structure for DSI Controller
 * This structure passes the hardware building information to the driver
 */
typedef struct {
	char *Name;
	UINTPTR BaseAddr;	/**< Base address of DSI Controller */
	u8  DataType;		/**< RGB  type */
	u8  DsiPixel;		/**< Pixels per beat receive on input stream */
} XDsi2Rx_Config;

/**
* The structure to read DSI controller & Configurable Parameters
*/
typedef struct {
	XDsi2Rx_Config Config;	/**< GUI Configuration */
	u32  PixelMode;		/**< Specifies VideoMode */
} XDsi2Rx_ConfigParameters;

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
*		XDSI2RX_ISR_*_MASK constants defined in xdsi_hw.h.
*
* @return	None.
*
* @note		None.
*
 *****************************************************************************/
typedef void (*XDsi2Rx_Callback) (void *CallbackRef, u32 Mask);

/**
* The XDsi2Rx driver instance data.
* An instance must be allocated for each DSI in use.
*/
typedef struct {
	XDsi2Rx_Config Config;		/**< GUI Configuration */

	u32  PixelMode;			/**< Specifies VideoMode
					  *  NoNBurst/Burst */

	XDsi2Rx_Callback UnSupportedDataTypeCallback;	/**< Callback for
							UnSupportedDataType */
	void *UnsupportDataTypeRef;			/**< To be passed to the
							UnSupportedDataType
							call back*/
	XDsi2Rx_Callback CRCCallback;	/*CRC Callback */
	void *CRCRef;
	XDsi2Rx_Callback ECC1Callback; /* ECC1 Callback */
	void *ECC1Ref;
	XDsi2Rx_Callback ECC2Callback; /* ECC2 Callback */
	void *ECC2Ref;
	XDsi2Rx_Callback SOTSyncErrLane1Callback; /* SOT Sync
							Error Lane 1 */
	void *SOTSyncErrLane1Ref;
	XDsi2Rx_Callback SOTErrLane1Callback;  /* SOT Error Lane 1 */
	void *SOTErrLane1Ref;
	XDsi2Rx_Callback SOTSyncErrLane2Callback; /* SOT Sync
					y		Error Lane 2 */
	void *SOTSyncErrLane2Ref;
	XDsi2Rx_Callback SOTErrLane2Callback; /* SOT Error
							Lane 2 */
	void *SOTErrLane2Ref;
	XDsi2Rx_Callback SOTSyncErrLane3Callback; /* SOT Sync Error
							Lane 3 */
	void *SOTSyncErrLane3Ref;
	XDsi2Rx_Callback SOTErrLane3Callback;/* SOT Error Lane 3 */
	void *SOTErrLane3Ref;
	XDsi2Rx_Callback SOTSyncErrLane4Callback;/* SOT Sync Error
							Lane 4 */
	void *SOTSyncErrLane4Ref;
	XDsi2Rx_Callback SOTErrLane4Callback;/* SOT Error Lane 4 */
	void *SOTErrLane4Ref;
	XDsi2Rx_Callback StopStateCallback; /* Stop State */
	void *StopStateRef;
	XDsi2Rx_Callback LmAsyncFifoFullCallback; /* LM Async FIFO Full */
	void *LmAsyncFifoFullRef;
	XDsi2Rx_Callback StreamAsyncFifoFullCallback; /* Stream
						AsyncFIFO Full */
	void *StreamAsyncFifoFullRef;
	XDsi2Rx_Callback GSPFifoNECallback; /* GSP FIFO Not empty */
	void *GSPFFifoNERef;
	XDsi2Rx_Callback GSPFifoFullCallback; /* Generic Short Packet
						FIFO full*/
	void *GSPFifoFullRef;
	XDsi2Rx_Callback FrmStartDetCallback; /* FRAME Start Detect*/
	void *FrmStartDetRef;

u32 IsReady; /**< Driver is ready */
} XDsi2Rx;

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
static inline void XDsi2Rx_BitSet(UINTPTR BaseAddress,u32 RegisterOffset,u32 BitMask)
{
	XDsi2Rx_WriteReg(BaseAddress, RegisterOffset,
	(XDsi2Rx_ReadReg(BaseAddress, RegisterOffset) | BitMask));
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
static inline void XDsi2Rx_BitReset(UINTPTR BaseAddress, u32 RegisterOffset,
							u32 BitMask)
{
	XDsi2Rx_WriteReg(BaseAddress, RegisterOffset,
	(XDsi2Rx_ReadReg(BaseAddress, RegisterOffset) & ~(BitMask)));
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
static inline u32 XDsi2Rx_GetBitField(UINTPTR BaseAddress, u32 RegisterOffset,
						u32 BitMask, u32 BitShift)
{
	return((XDsi2Rx_ReadReg(BaseAddress, RegisterOffset)
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
static inline void XDsi2Rx_SetBitField(UINTPTR BaseAddress, u32 RegisterOffset,
				u32 BitMask, u32 BitShift, u32 Value)
{
	XDsi2Rx_WriteReg(BaseAddress, RegisterOffset,
		((XDsi2Rx_ReadReg(BaseAddress, RegisterOffset) &
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
static inline void XDsi2Rx_SetSoftReset(XDsi2Rx *InstancePtr)
{
	XDsi2Rx_BitSet(InstancePtr->Config.BaseAddr,
		XDSI2RX_CCR_OFFSET, XDSI2RX_CCR_SOFTRESET_MASK);
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
static inline u32 XDsi2Rx_IsControllerReady(XDsi2Rx *InstancePtr)
{
	return XDsi2Rx_GetBitField(InstancePtr->Config.BaseAddr,
	XDSI2RX_CCR_OFFSET, XDSI2RX_CCR_CRREADY_MASK, XDSI2RX_CCR_CRREADY_SHIFT);
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
static inline void XDsi2Rx_ClearSoftReset(XDsi2Rx *InstancePtr)
{
	XDsi2Rx_BitReset(InstancePtr->Config.BaseAddr,
		XDSI2RX_CCR_OFFSET, XDSI2RX_CCR_SOFTRESET_MASK);
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
static inline void XDsi2Rx_Disable(XDsi2Rx *InstancePtr)
{
	XDsi2Rx_BitReset(InstancePtr->Config.BaseAddr,
		XDSI2RX_CCR_OFFSET, XDSI2RX_CCR_COREENB_MASK);
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
static inline void XDsi2Rx_Enable(XDsi2Rx *InstancePtr)
{
	XDsi2Rx_BitSet(InstancePtr->Config.BaseAddr, XDSI2RX_CCR_OFFSET,
			XDSI2RX_CCR_COREENB_MASK);
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
static inline u32 XDsi2Rx_IsEnabled(XDsi2Rx *InstancePtr)
{
	return XDsi2Rx_GetBitField(InstancePtr->Config.BaseAddr,
	XDSI2RX_CCR_OFFSET, XDSI2RX_CCR_COREENB_MASK, XDSI2RX_CCR_COREENB_SHIFT);
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
static inline void XDsi2Rx_SetGlobalInterrupt(XDsi2Rx *InstancePtr)
{
	XDsi2Rx_BitSet(InstancePtr->Config.BaseAddr, XDSI2RX_GIER_OFFSET,
					XDSI2RX_GIER_GIE_MASK);
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
static inline void XDsi2Rx_ResetGlobalInterrupt(XDsi2Rx *InstancePtr)
{
	XDsi2Rx_BitReset(InstancePtr->Config.BaseAddr,
		XDSI2RX_GIER_OFFSET, XDSI2RX_GIER_GIE_MASK);
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
static inline u32 XDsi2Rx_GetIntrEnableStatus(XDsi2Rx *InstancePtr)
{
	return XDsi2Rx_ReadReg(InstancePtr->Config.BaseAddr, XDSI2RX_IER_OFFSET);
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
static inline u32 XDsi2Rx_GetIntrStatus(XDsi2Rx *InstancePtr)
{
	 return XDsi2Rx_ReadReg(InstancePtr->Config.BaseAddr, XDSI2RX_ISR_OFFSET);
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
static inline void XDsi2Rx_IntrClear(XDsi2Rx *InstancePtr, u32 Value)
{
	XDsi2Rx_WriteReg(InstancePtr->Config.BaseAddr, XDSI2RX_ISR_OFFSET,
			(Value & XDSI2RX_ISR_ALLINTR_MASK));
}

/****************************************************************************/
/****************************************************************************/
/**
*
* This function is used to get pixel format
*
* @param	InstancePtr is a pointer to the DSI Instance to be
*		worked on.
*
* @return	0x0E â?" Packed RGB565
*		0x1E- packed RGB666
*		0x2E â?" Loosely packed RGB666
*		0x3E- Packed RGB888
*		0x0B- Compressed Pixel Stream
*
* @note		None
*
****************************************************************************/
static inline u32 XDsi2Rx_GetPixelFormat(XDsi2Rx *InstancePtr)
{
	return XDsi2Rx_GetBitField(InstancePtr->Config.BaseAddr,
	XDSI2RX_PCR_OFFSET, XDSI2RX_PCR_PIXELFORMAT_MASK, XDSI2RX_PCR_PIXELFORMAT_SHIFT);
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
static inline void XDsi2Rx_IntrEnable(XDsi2Rx *InstancePtr, u32 Mask)
{
	XDsi2Rx_WriteReg(InstancePtr->Config.BaseAddr,
			XDSI2RX_IER_OFFSET, Mask & XDSI2RX_IER_ALLINTR_MASK);
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
static inline void XDsi2Rx_IntrDisable(XDsi2Rx *InstancePtr, u32 Mask)
{
	XDsi2Rx_WriteReg(InstancePtr->Config.BaseAddr,
		XDSI2RX_IER_OFFSET, ~  Mask & XDSI2RX_IER_ALLINTR_MASK);
}

/************************** Function Prototypes ******************************/

XDsi2Rx_Config *XDsi2Rx_LookupConfig(UINTPTR BaseAddress);

u32 XDsi2Rx_CfgInitialize(XDsi2Rx *InstancePtr, XDsi2Rx_Config *Config,
			UINTPTR EffectiveAddr);
u32 XDsi2Rx_Activate(XDsi2Rx *InstancePtr, XDsi2Rx_Selection Flag);
void XDsi2Rx_Reset(XDsi2Rx *InstancePtr);
u32 XDsi2Rx_DefaultConfigure(XDsi2Rx *InstancePtr);
u32 XDsi2Rx_SelfTest(XDsi2Rx *InstancePtr);
void XDsi2Rx_GetConfigParams(XDsi2Rx *InstancePtr,
		XDsi2Rx_ConfigParameters *ConfigInfo);
s32 XDsi2Rx_SetCallback(XDsi2Rx *InstancePtr, u32 HandleType,
			void *CallbackFunc, void *CallbackRef);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
