/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XMCDMA_BD_H_
/** @brief Include guard for the MCDMA buffer descriptor header. */
#define XMCDMA_BD_H_

/*****************************************************************************/
/**
*
* @file xmcdma_bd.h
* @addtogroup mcdma Overview
* @{
* @brief Defines buffer descriptor macros and helpers for MCDMA.
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ------------------------------------------------------
* 1.0	adk  18/07/17 Initial version.
* 1.2	mj   05/03/18 Added macro XMcdma_BdSetSwId and XMcdma_BdGetSwId to set
*                     and get Sw ID field from BD.
*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"

/**
 * @brief Buffer descriptor storage as an array of 32-bit words.
 */
typedef u32 XMcdma_Bd[16];

/***************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************
 * Define methods to flush and invalidate cache for BDs should they be
 * located in cached memory.
 *****************************************************************************/
/**
 * @brief Declares cache flush and invalidate helpers for BDs in cached memory.
 */
#ifdef __aarch64__
/**
 * @brief Flushes the data cache for a buffer descriptor.
 *
 * @param[in] BdPtr is the buffer descriptor address.
 */
#define XMCDMA_CACHE_FLUSH(BdPtr) /**< No-op cache flush on AArch64. */
/**
 * @brief Invalidates the data cache for a buffer descriptor.
 *
 * @param[in] BdPtr is the buffer descriptor address.
 */
#define XMCDMA_CACHE_INVALIDATE(BdPtr) /**< No-op cache invalidate on AArch64. */
#else
/**
 * @brief Flushes the data cache for a buffer descriptor.
 *
 * @param[in] BdPtr is the buffer descriptor address.
 */
#define XMCDMA_CACHE_FLUSH(BdPtr) \
	Xil_DCacheFlushRange((UINTPTR)(BdPtr), XMCDMA_BD_HW_NUM_BYTES)

/**
 * @brief Invalidates the data cache for a buffer descriptor.
 *
 * @param[in] BdPtr is the buffer descriptor address.
 */
#define XMCDMA_CACHE_INVALIDATE(BdPtr) \
	Xil_DCacheInvalidateRange((UINTPTR)(BdPtr), XMCDMA_BD_HW_NUM_BYTES)
#endif


/*****************************************************************************/
/**
*
* @brief Read the given Buffer Descriptor word.
*
* @param[in]	BaseAddress is the base address of the BD to read
* @param[in]	Offset is the word offset to be read
*
* @return	The 32-bit value of the field
*
* @note
*		C-style signature:
*		u32 XMcdma_BdRead(u32 BaseAddress, u32 Offset)
*
******************************************************************************/
#define XMcdma_BdRead(BaseAddress, Offset)				\
	(*(u32 *)(((UINTPTR)(void *)(BaseAddress)) + (u32)(Offset)))

/*****************************************************************************/
/**
*
* @brief Read the given Buffer Descriptor word.
*
* @param[in]	BaseAddress is the base address of the BD to read
* @param[in]	Offset is the word offset to be read
*
* @return	The 64-bit value of the field
*
* @note
*		C-style signature:
*		u64 XMcdma_BdRead64(u64 BaseAddress, u32 Offset)
*
******************************************************************************/
#define XMcdma_BdRead64(BaseAddress, Offset)			\
	(*(u64 *)(((UINTPTR)(void *)(BaseAddress)) + (u32)(Offset)))

/*****************************************************************************/
/**
*
* @brief Write the given Buffer Descriptor word.
*
* @param[in]	BaseAddress is the base address of the BD to write
* @param[in]	Offset is the word offset to be written
* @param[in]	Data is the 32-bit value to write to the field
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
* @brief Write the given Buffer Descriptor double word.
*
* @param[in]	BaseAddress is the base address of the BD to write
* @param[in]	Offset is the word offset to be written
* @param[in]	Data is the 64-bit value to write to the field
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
 * @brief Retrieve the status of a Ingress(S2MM) BD
 *
 * @param[in]	BdPtr is the BD to operate on
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
 * @brief Check whether a Ingress(S2MM) BD has completed in hardware.
 * This BD has been submitted to hardware. The application can use this function
 * To poll for the Completion of the BD.
 *
 * May not work if the BD is in cached memory.
 *
 * @param[in]	BdPtr is the BD to check on
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
 * @brief Get the actual transfer length of a Ingress (S2MM)BD.
 * The BD has completed in hw.
 *
 * May not work if the BD is in cached memory.
 *
 * @param[in]	BdPtr is the BD to check on
 * @param[in]	LengthMask is the Maximum Transfer Length.
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
 * @brief Retrieve the status of a Egress(MM2S) BD
 *
 * @param[in]	BdPtr is the BD to operate on
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
 * @brief Check whether a Egress(MM2S) BD has completed in hardware.
 * This BD has been submitted to hardware. The application can use this function
 * To poll for the Completion of the BD.
 *
 * May not work if the BD is in cached memory.
 *
 * @param[in]	BdPtr is the BD to check on
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
 * @brief Get the actual transfer length of a Egress(MM2S) BD.
 * The BD has completed in hw.
 *
 * May not work if the BD is in cached memory.
 *
 * @param[in]	BdPtr is the BD to check on
 * @param[in]	LengthMask is the Maximum Transfer Length.
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
 * @brief Gets the control bits of a BD.
 *
 * @param[in]	BdPtr is the BD to operate on
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
* @brief Return the next BD in the Chain.
*
* @param[in]	Chan is the Channel BD Chain to operate on.
* @param[in]	BdPtr is the current BD.
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
* @brief Return the previous BD in the Chain.
*
* @param[in]	Chan is the MCDMA channel to operate on.
* @param[in]	BdPtr is the current BD.
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
* @brief Check whether a DMA is started, meaning the channel is not halted.
*
* @param[in]	Chan is the channel instance to operate on.
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
* @brief Check whether a DMA channel is started, meaning the channel is not halted.
*
* @param[in]	Chan is the channel instance to operate on.
* @param[in]	Chan_id is the channel number to operate on.
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
 * @brief Set the Tid and Tuser fields of a BD with the user provided values.
 *
 * @param[in]	BdPtr is the BD to operate on.
 * @param[in]	Tid is Tid value to be written in the Bd field.
 * @param[in]	Tuser is Tuser value to be written in the Bd field.
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
 * @brief Retrieve the side band status of a BD
 *
 * @param[in]	BdPtr is the BD to operate on
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
 * @brief Clears the BD Contents.
 *
 * @param[in]	BdPtr is the BD to operate on
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
 * @brief Set the Sw ID field of the given BD. The ID is an arbitrary piece of data the
 * application can associate with a specific BD.
 *
 * @param[in]	BdPtr is the BD to operate on
 * @param[in]	Id is a 32-bit quantity to set in the BD
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
 * @brief Retrieve the Sw ID field of the given BD previously set with
 * XMcdma_BdSetSwId.
 *
 * @param[in]	BdPtr is the BD to operate on
 *
 * @return	BD Sw identifier
 *
 * @note
 *		C-style signature:
 *		u32 XMcdma_BdGetSwId(XMcdma_Bd* BdPtr)
 *
 *****************************************************************************/
#define XMcdma_BdGetSwId(BdPtr) (XMcdma_BdRead((BdPtr), XMCDMA_BD_SW_ID_OFFSET))

#ifdef __cplusplus
}
#endif

/** @} */
#endif
