/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xmcdma_bd.h
* @addtogroup mcdma_v1_4
* @{
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ------------------------------------------------------
* 1.0	adk  18/07/17 Initial version.
* 1.2	mj   05/03/18 Added macro XMcdma_BdSetSwId and XMcdma_BdGetSwId to set
*                     and get Sw ID field from BD.
*****************************************************************************/

#ifndef XMCDMA_BD_H_
#define XMCDMA_BD_H

#include "xil_types.h"

typedef u32 XMcdma_Bd[16];

/***************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************
 * Define methods to flush and invalidate cache for BDs should they be
 * located in cached memory.
 *****************************************************************************/
#ifdef __aarch64__
#define XMCDMA_CACHE_FLUSH(BdPtr)
#define XMCDMA_CACHE_INVALIDATE(BdPtr)
#else
#define XMCDMA_CACHE_FLUSH(BdPtr) \
        Xil_DCacheFlushRange((UINTPTR)(BdPtr), XMCDMA_BD_HW_NUM_BYTES)

#define XMCDMA_CACHE_INVALIDATE(BdPtr) \
        Xil_DCacheInvalidateRange((UINTPTR)(BdPtr), XMCDMA_BD_HW_NUM_BYTES)
#endif


/*****************************************************************************/
/**
*
* Read the given Buffer Descriptor word.
*
* @param	BaseAddress is the base address of the BD to read
* @param	Offset is the word offset to be read
*
* @return	The 32-bit value of the field
*
* @note
*		C-style signature:
*		u32 XMcdma_BdRead(u32 BaseAddress, u32 Offset)
*
******************************************************************************/
#define XMcdma_BdRead(BaseAddress, Offset)				\
	(*(u32 *)(((void *)(UINTPTR)(BaseAddress)) + (u32)(Offset)))

/*****************************************************************************/
/**
*
* Read the given Buffer Descriptor word.
*
* @param	BaseAddress is the base address of the BD to read
* @param	Offset is the word offset to be read
*
* @return	The 64-bit value of the field
*
* @note
*		C-style signature:
*		u64 XMcdma_BdRead64(u64 BaseAddress, u32 Offset)
*
******************************************************************************/
#define XMcdma_BdRead64(BaseAddress, Offset)			\
	(*(u64 *)(((void *)(UINTPTR)(BaseAddress)) + (u32)(Offset)))

/*****************************************************************************/
/**
*
* Write the given Buffer Descriptor word.
*
* @param	BaseAddress is the base address of the BD to write
* @param	Offset is the word offset to be written
* @param	Data is the 32-bit value to write to the field
*
* @return	None.
*
* @note
* 		C-style signature:
*		void XMcdma_BdWrite(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XMcdma_BdWrite(BaseAddress, Offset, Data)			\
	(*(u32 *)((UINTPTR)(void *)(BaseAddress) + (u32)(Offset))) = (u32)(Data)

/*****************************************************************************/
/**
*
* Write the given Buffer Descriptor double word.
*
* @param	BaseAddress is the base address of the BD to write
* @param	Offset is the word offset to be written
* @param	Data is the 64-bit value to write to the field
*
* @return	None.
*
* @note
* 		C-style signature:
*		void XMcdma_BdWrite(u64 BaseAddress, u32 RegOffset, u64 Data)
*
******************************************************************************/
#define XMcdma_BdWrite64(BaseAddress, Offset, Data)			\
	(*(u64 *)((UINTPTR)(void *)(BaseAddress) + (u32)(Offset))) = (u64)(Data)

/*****************************************************************************/
/**
 * Retrieve the status of a Ingress(S2MM) BD
 *
 * @param	BdPtr is the BD to operate on
 *
 * @return	Use XMDMA_BD_STS_*** values defined in xmdma_hw.h to
 *		interpret the returned value
 *
 * @note
 *		C-style signature:
 *		u32 XMcDma_BdGetSts(XMcdma_Bd* BdPtr)
 *
 *****************************************************************************/
#define XMcDma_BdGetSts(BdPtr)	\
	XMcdma_BdRead((BdPtr), XMCDMA_BD_STS_OFFSET)

/*****************************************************************************/
/**
 * Check whether a Ingress(S2MM) BD has completed in hardware.
 * This BD has been submitted to hardware. The application can use this function
 * To poll for the Completion of the BD.
 *
 * This function may not work if the BD is in cached memory.
 *
 * @param	BdPtr is the BD to check on
 *
 * @return
 *		- 0 if not complete
 *		- XMCDMA_BD_STS_COMPLETE_MASK if completed, may contain
 * 		  XMDMA_BD_STS_*_ERR_MASK bits.
 *
 * @note
 * 		C-style signature:
 *		int XMcdma_BdHwCompleted(XMcdma_Bd* BdPtr)
 *
 *****************************************************************************/
#define XMcdma_BdHwCompleted(BdPtr)                     \
	(XMcdma_BdRead((BdPtr), XMCDMA_BD_STS_OFFSET) & \
		XMCDMA_BD_STS_COMPLETE_MASK)

/*****************************************************************************/
/**
 * Get the actual transfer length of a Ingress (S2MM)BD.
 * The BD has completed in hw.
 *
 * This function may not work if the BD is in cached memory.
 *
 * @param	BdPtr is the BD to check on
 * @param	LengthMask is the Maximum Transfer Length.
 *
 * @return	None
 *
 * @note
 *		C-style signature:
 *		int XMcDma_BdGetActualLength(XMcdma_Bd* BdPtr, u32 LengthMask)
 *
 *****************************************************************************/
#define XMcDma_BdGetActualLength(BdPtr, LengthMask)      \
	(XMcdma_BdRead((BdPtr), XMCDMA_BD_STS_OFFSET) & \
		LengthMask)

/*****************************************************************************/
/**
 * Retrieve the status of a Egress(MM2S) BD
 *
 * @param	BdPtr is the BD to operate on
 *
 * @return	Use XMDMA_BD_STS_*** values defined in xmdma_hw.h to
 *		interpret the returned value
 *
 * @note
 *		C-style signature:
 *		u32 XMcDma_TxBdGetSts(XMcdma_Bd* BdPtr)
 *
 *****************************************************************************/
#define XMcDma_TxBdGetSts(BdPtr)	\
	XMcdma_BdRead((BdPtr), XMCDMA_BD_SIDEBAND_STS_OFFSET)

/*****************************************************************************/
/**
 * Check whether a Egress(MM2S) BD has completed in hardware.
 * This BD has been submitted to hardware. The application can use this function
 * To poll for the Completion of the BD.
 *
 * This function may not work if the BD is in cached memory.
 *
 * @param	BdPtr is the BD to check on
 *
 * @return
 *		- 0 if not complete
 *		- XMCDMA_BD_STS_COMPLETE_MASK if completed, may contain
 * 		  XMDMA_BD_STS_*_ERR_MASK bits.
 *
 * @note
 * 		C-style signature:
 *		int XMcdma_TxBdHwCompleted(XMcdma_Bd* BdPtr)
 *
 *****************************************************************************/
#define XMcdma_TxBdHwCompleted(BdPtr)                     \
	(XMcdma_BdRead((BdPtr), XMCDMA_BD_SIDEBAND_STS_OFFSET) & \
		XMCDMA_BD_STS_COMPLETE_MASK)

/*****************************************************************************/
/**
 * Get the actual transfer length of a Egress(MM2S) BD.
 * The BD has completed in hw.
 *
 * This function may not work if the BD is in cached memory.
 *
 * @param	BdPtr is the BD to check on
 * @param	LengthMask is the Maximum Transfer Length.
 *
 * @return	None
 *
 * @note
 *		C-style signature:
 *		int XMcDma_TxBdGetActualLength(XMcdma_Bd* BdPtr, u32 LengthMask)
 *
 *****************************************************************************/
#define XMcDma_TxBdGetActualLength(BdPtr, LengthMask)      \
	(XMcdma_BdRead((BdPtr), XMCDMA_BD_SIDEBAND_STS_OFFSET) & \
		LengthMask)

/*****************************************************************************/
/**
 * Gets the control bits of a BD.
 *
 * @param	BdPtr is the BD to operate on
 *
 * @return	control bits of a BD.
 *
 * @note
 *		C-style signature:
 *		u32 XMcDma_BdGetCtrl(XMCDMA_Bd* BdPtr)
 *
 *****************************************************************************/
#define XMcDma_BdGetCtrl(BdPtr)				\
		(XMcdma_BdRead((BdPtr), XMCDMA_BD_CTRL_OFFSET)	\
		& XMCDMA_BD_CTRL_ALL_MASK)

/****************************************************************************/
/**
* Return the next BD in the Chain.
*
* @param	Chan is the Channel BD Chain to operate on.
* @param	BdPtr is the current BD.
*
* @return	The next BD in the Chain relative to the BdPtr parameter.
*
* @note
*		C-style signature:
*		XMcdma_Bd *XMcdma_BdChainNextBd(XMcdma_ChanCtrl* Chan,
*						 XMcdma_Bd *BdPtr)
*****************************************************************************/
#define XMcdma_BdChainNextBd(Chan, BdPtr)			\
	(((UINTPTR)(BdPtr) >= (Chan)->LastBdAddr) ?	\
		(UINTPTR)(Chan)->FirstBdAddr :	\
		(UINTPTR)((UINTPTR)(BdPtr) + (Chan)->Separation))

/****************************************************************************/
/**
* Return the previous BD in the Chain.
*
* @param	Chan is the MCDMA channel to operate on.
* @param	BdPtr is the current BD.
*
* @return	The previous BD in the Chain relative to the BdPtr parameter.
*
* @note
*		C-style signature:
*		XMcdma_Bd *XMcdma_BdChainPrevBd(XMcdma_ChanCtrl* Chan,
*						XMcdma_Bd *BdPtr)
*****************************************************************************/
#define XMcdma_BdChainPrevBd(Chan, BdPtr)				\
	(((UINTPTR)(BdPtr) <= (Chan)->FirstBdAddr) ?			\
		(XMcdma_Bd *)(Chan)->LastBdAddr :			\
		(XMcdma_Bd *)((UINTPTR)(BdPtr) - (Chan)->Separation))

/****************************************************************************/
/**
* Check whether a DMA is started, meaning the channel is not halted.
*
* @param        Chan is the channel instance to operate on.
*
* @return
*               - 1 if channel is started
*               - 0 otherwise
*
* @note
*               C-style signature:
*               int XMcdma_ChanHwIsStarted(XMcdma_ChanCtrl *Chan)
*
*****************************************************************************/
#define XMcdma_HwIsStarted(Chan)                             	\
        ((XMcdma_ReadReg((Chan)->ChanBase, XMCDMA_CSR_OFFSET) 	\
          & XMCDMA_CSR_HALTED_MASK) ? FALSE : TRUE)

/****************************************************************************/
/**
* Check whether a DMA channel is started, meaning the channel is not halted.
*
* @param        Chan is the channel instance to operate on.
* @param	Chan_id is the channel number to operate on.
*
* @return
*               - 1 if channel is started
*               - 0 otherwise
*
* @note
*               C-style signature:
*               int XMcdma_ChanHwIsStarted(XMcdma_ChanCtrl *Chan, u32 Chan_id)
*
*****************************************************************************/
#define XMcdma_ChanHwIsStarted(Chan, Chan_id) \
	((XMcdma_ReadReg((Chan)->ChanBase, (XMCDMA_CR_OFFSET + \
	 (Chan_id - 1) * XMCDMA_NXTCHAN_OFFSET)) \
	    & XMCDMA_CCR_RUNSTOP_MASK) ? TRUE : FALSE)

/*****************************************************************************/
/**
 * Set the Tid and Tuser fields of a BD with the user provided values.
 *
 * @param	BdPtr is the BD to operate on.
 * @param	Tid is Tid value to be written in the Bd field.
 * @param	Tuser is Tuser value to be written in the Bd field.
 *
 * @note
 *		C-style signature:
 *		u32 XMcDma_BdSetCtrlSideBand(XMcdma_Bd* BdPtr, u8 Tid, u8 Tuser)
 *
 *****************************************************************************/
#define XMcDma_BdSetCtrlSideBand(BdPtr, Tid, Tuser) \
	XMcdma_BdWrite((BdPtr), XMCDMA_BD_CTRL_SBAND_OFFSET, \
			Tid << XMCDMA_BD_CTRL_SBAND_SHIFT | Tuser)

/*****************************************************************************/
/**
 * Retrieve the side band status of a BD
 *
 * @param	BdPtr is the BD to operate on
 *
 * @return	Use XMDMA_BD_STS_*** values defined in xmdma_hw.h to
 *		interpret the returned value
 *
 * @note
 *		C-style signature:
 *		u32 XMcDma_GetBdSbandStats(XMcdma_Bd* BdPtr)
 *
 *****************************************************************************/
#define XMcDma_GetBdSbandStats(BdPtr) \
	XMcdma_BdRead((BdPtr), XMCDMA_BD_SIDEBAND_STS_OFFSET)

/*****************************************************************************/
/**
 * Clears the BD Contents.
 *
 * @param	BdPtr is the BD to operate on
 *
 * @note
 *		C-style signature:
 *		void XMcdma_BdClear(XMcdma_Bd* BdPtr)
 *
 *****************************************************************************/
#define XMcdma_BdClear(BdPtr)                    \
	memset((void *)(((UINTPTR)(BdPtr)) + XMCDMA_BD_START_CLEAR), 0, \
			XMCDMA_BD_BYTES_TO_CLEAR)

/*****************************************************************************/
/**
 * Set the Sw ID field of the given BD. The ID is an arbitrary piece of data the
 * application can associate with a specific BD.
 *
 * @param	BdPtr is the BD to operate on
 * @param	Id is a 32 bit quantity to set in the BD
 *
 * @return	None
 *
 * @note
 *		C-style signature:
 *		void XMcdma_BdSetSwId(XMcdma_Bd* BdPtr, void Id)
 *
 *****************************************************************************/
#define XMcdma_BdSetSwId(BdPtr, Id) \
	(XMcdma_BdWrite((BdPtr), XMCDMA_BD_SW_ID_OFFSET, (UINTPTR)(Id)))


/*****************************************************************************/
/**
 * Retrieve the Sw ID field of the given BD previously set with
 * XMcdma_BdSetSwId.
 *
 * @param	BdPtr is the BD to operate on
 *
 * @return	BD Sw identifier
 *
 * @note
 *		C-style signature:
 *		u32 XMcdma_BdGetSwId(XMcdma_Bd* BdPtr)
 *
 *****************************************************************************/
#define XMcdma_BdGetSwId(BdPtr) (XMcdma_BdRead((BdPtr), XMCDMA_BD_SW_ID_OFFSET))

#endif
