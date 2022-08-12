/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *  @file xaxicdma_bdring.c
* @addtogroup axicdma_v4_10
* @{
 *
 * Implementation for support on Scatter Gather (SG) transfers.
 * It includes the implementation of the BD ring API. There is only one BD ring
 * per DMA engine.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a jz   04/18/10 First release
 * 2.01a rkv  01/25/11 Replaced with "\r\n" in place on "\n\r" in printf statements
 * 4.3   mi   09/21/16 Fixed compilation warnings
 * 4.5   rsp  07/04/18 Fixed cppcheck warning
 * </pre>
 *
 *****************************************************************************/
#include "xaxicdma.h"
#include "xaxicdma_i.h"

/***************** Macros (Inline Functions) Definitions *********************/
/* The following macros are helper functions inside this file.
 */

/******************************************************************************
 * Move the BdPtr argument ahead an arbitrary number of BDs. Wrapping around
 * to the beginning of the ring if needed.
 *
 * We know a wraparound should occur if the new BdPtr is greater than
 * the high address in the ring.
 *
 * @param	InstancePtr is the ring BdPtr appears in
 * @param	BdPtr on input is the starting BD position and on output is the
 *			final BD position
 * @param	NumBd is the number of BD spaces to increment
 *
 * @note	The BdPtr will be changed if NumBd not zero.
 *
 *****************************************************************************/
#define XAXICDMA_RING_SEEKAHEAD(InstancePtr, BdPtr, NumBd)         \
{                                                                  \
    UINTPTR Addr = (UINTPTR)(void *)(BdPtr);                       \
                                                                   \
    Addr += ((InstancePtr)->BdSeparation * (NumBd));               \
    if (Addr > (InstancePtr)->LastBdAddr) {                        \
        Addr -= (InstancePtr)->BdRingTotalLen;                     \
    }                                                              \
                                                                   \
    (BdPtr) = (XAxiCdma_Bd *)(void *)Addr;                                  \
}

/******************************************************************************
 * Move the BdPtr argument backwards an arbitrary number of BDs. Wrapping
 * around to the end of the ring if needed.
 *
 * We know a wraparound should occur if the new BdPtr is less than
 * the base address in the ring.
 *
 * @param	InstancePtr is the ring BdPtr appears in
 * @param	BdPtr on input is the starting BD position and on output is the
 *			final BD position
 * @param	NumBd is the number of BD spaces to increment
 *
 * @note	The BdPtr will be changed if NumBd not zero.
 *
 *****************************************************************************/
#define XAXICDMA_RING_SEEKBACK(InstancePtr, BdPtr, NumBd)        \
{                                                                \
    UINTPTR Addr = (UINTPTR)(void *)(BdPtr);                                     \
                                                                 \
    Addr -= ((InstancePtr)->BdSeparation * (NumBd));             \
    if (Addr < (InstancePtr)->FirstBdAddr) {                     \
        Addr += (InstancePtr)->BdRingTotalLen;                   \
    }                                                            \
                                                                 \
    (BdPtr) = (XAxiCdma_Bd*)(void *)Addr;                                \
}

/*****************************************************************************/
/**
 * This function calculates how many BDs can be built using given number of
 * bytes of memory, according to alignment provided.
 *
 * @param	Alignment is the preferred alignment for the BDs
 * @param	Bytes is the number of bytes of memory to build BDs with
 * @param	BdBuffAddr is the buffer address allocated for the BDs. This
 *			is to check the alignment of the buffer to make sure the the
 *			buffer is aligned to the BD alignment. An invalid buffer
 *			address results in 0.
 *
 * @return	The number of BDs can be built. 0 means buffer address
 *			is not valid.
 *
 * @note	The application is responsible to align the buffer before pass
 *			it to this function.
 *
 *****************************************************************************/
u32 XAxiCdma_BdRingCntCalc(u32 Alignment, u32 Bytes, UINTPTR BdBuffAddr)
{

	/* The buffer alignment has to be taken account of. An unaligned buffer
	 * is invalid.
	 */
	if (BdBuffAddr & Alignment) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Invalid buffer addr %x\r\n",
			(unsigned int)BdBuffAddr);
		return 0;
	}

	return ((uint32_t)(Bytes) / ((sizeof(XAxiCdma_Bd) + (Alignment - 1)) &
               ~(Alignment - 1)));
}

/*****************************************************************************/
/**
 * This function calculates how much memory is needed to build requested number
 * of BDs.
 *
 * @param	Alignment is the preferred alignment for the BDs
 * @param	NumBd is the number of BDs to be built
 *
 * @return	The number of bytes of memory needed to build the BDs
 *
 * @note	None.
 *
 *****************************************************************************/
u32 XAxiCdma_BdRingMemCalc(u32 Alignment, int NumBd)
{
	return (uint32_t)(((sizeof(XAxiCdma_Bd) + (Alignment - 1)) &
               ~(Alignment - 1)) * NumBd);
}

/*****************************************************************************/
/**
 * This function set the Current BD ptr of a BD
 *
 * @param       BdPtr is the pointer to the BD to set the Next BD ptr
 * @param       CurBdPtr is the valud of Current BD ptr
 *
 * @return      None
 *
 * @note        None.
 *
 *****************************************************************************/
void XAxiCdma_BdSetCurBdPtr(XAxiCdma *InstancePtr, UINTPTR CurBdPtr)
{
	XAxiCdma_WriteReg(InstancePtr->BaseAddr, XAXICDMA_CDESC_OFFSET,
                          (CurBdPtr & XAXICDMA_DESC_LSB_MASK));
    if (InstancePtr->AddrWidth > 32)
         XAxiCdma_WriteReg(InstancePtr->BaseAddr, XAXICDMA_CDESC_MSB_OFFSET,
			   UPPER_32_BITS(CurBdPtr));
        return;
}

/*****************************************************************************/
/**
 * This function grabs a copy of the current BD pointer from the hardware.
 *
 * @param	InstancePtr is the driver instance we are working on
 *
 * @return	The BD pointer in CDESC register
 *
 * @note	None.
 *
 *****************************************************************************/
XAxiCdma_Bd *XAxiCdma_BdRingGetCurrBd(XAxiCdma *InstancePtr)
{

	if (InstancePtr->AddrWidth > 32) {
		return (XAxiCdma_Bd *)(UINTPTR)((XAxiCdma_BdRead(InstancePtr->BaseAddr, XAXICDMA_CDESC_OFFSET)) |
		((uint64_t)(XAxiCdma_BdRead(InstancePtr->BaseAddr, XAXICDMA_CDESC_MSB_OFFSET)) << 32U));
	} else {
		return (XAxiCdma_Bd *)(UINTPTR)(XAxiCdma_BdRead(InstancePtr->BaseAddr, XAXICDMA_CDESC_OFFSET));
	}
}

/*****************************************************************************/
/**
 * This function gets the total number of BDs in the BD ring.
 *
 * @param	InstancePtr is the driver instance we are working on
 *
 * @return	The total number of BDs for this instance
 *
 * @note	None.
 *
 *****************************************************************************/
u32 XAxiCdma_BdRingGetCnt(XAxiCdma *InstancePtr)
{
	return (uint32_t)InstancePtr->AllBdCnt;
}

/*****************************************************************************/
/**
 * This function gets the number of free BDs.
 *
 * @param	InstancePtr is the driver instance we are working on
 *
 * @return	The total number of free BDs for this instance
 *
 * @note	None.
 *
 *****************************************************************************/
u32 XAxiCdma_BdRingGetFreeCnt(XAxiCdma *InstancePtr)
{
	return InstancePtr->FreeBdCnt;
}

/*****************************************************************************/
/**
 * This function grabs a copy of the current BD pointer from the hardware.
 * It is normally used to prepare for the hardware reset. The snapshot of the
 * current BD pointer should be reloaded once the reset is done.
 *
 * @param	InstancePtr is the driver instance we are working on
 *
 * @return	None
 *
 * @note	None.
 *
 *****************************************************************************/
void XAxiCdma_BdRingSnapShotCurrBd(XAxiCdma *InstancePtr)
{
	XAxiCdma_Bd *BdPtr;

	BdPtr =	XAxiCdma_BdRingGetCurrBd(InstancePtr);

	InstancePtr->BdaRestart = XAxiCdma_BdRingNext(InstancePtr, BdPtr);

	return;
}

/*****************************************************************************/
/**
 * This function set the Tail BD ptr of a BD
 *
 * @param       BdPtr is the pointer to the BD to set the Next BD ptr
 * @param       Tail Bd is the value of Tail BD ptr
 *
 * @return      None
 *
 * @note        None.
 *
 *****************************************************************************/
void XAxiCdma_BdSetTailBdPtr(XAxiCdma *InstancePtr, UINTPTR TailBdPtr)
{
    XAxiCdma_WriteReg(InstancePtr->BaseAddr, XAXICDMA_TDESC_OFFSET,
                      TailBdPtr & XAXICDMA_DESC_LSB_MASK);
    if (InstancePtr->AddrWidth > 32)
            XAxiCdma_WriteReg(InstancePtr->BaseAddr, XAXICDMA_TDESC_MSB_OFFSET,
                              UPPER_32_BITS(TailBdPtr));
        return;
}


/*****************************************************************************/
/**
 * This function gets the next BD of the current BD on the BD ring.
 *
 * @param	InstancePtr is the driver instance we are working on
 * @param	BdPtr is the current BD
 *
 * @return	The next BD on the ring from the current BD, NULL if passed
 *			in BdPtr not valid.
 *
 * @note	None.
 *
 *****************************************************************************/
XAxiCdma_Bd *XAxiCdma_BdRingNext(XAxiCdma *InstancePtr, XAxiCdma_Bd *BdPtr)
{
	UINTPTR ReturnBd;

	/* Check whether the BD ptr is valid
	 * A BD ptr is not valid if:
	 * - It is outside of the BD memory range
	 * - It has invalid alignment
	 */
	if (((UINTPTR)BdPtr > InstancePtr->LastBdAddr)  ||
	    ((UINTPTR)BdPtr < InstancePtr->FirstBdAddr) ||
		((UINTPTR)BdPtr & (InstancePtr->BdSeparation - 1))) {

		xdbg_printf(XDBG_DEBUG_ERROR, "Invalid BdPtr %x: %x/%x/%x\r\n",
		(UINTPTR)BdPtr, (UINTPTR)InstancePtr->FirstBdAddr,
			(UINTPTR)InstancePtr->LastBdAddr,
			(unsigned int)XAXICDMA_BD_MINIMUM_ALIGNMENT);
		ReturnBd = 0x0;
	}

	/* If the current BD is the last BD in the ring, return the first BD
	 */
	else if ((UINTPTR)BdPtr == InstancePtr->LastBdAddr) {
		ReturnBd = InstancePtr->FirstBdAddr;
	}
	else {
		ReturnBd = (UINTPTR)BdPtr + InstancePtr->BdSeparation;
	}

	 return (XAxiCdma_Bd *)ReturnBd;
}

/*****************************************************************************/
/**
 * This function gets the previous BD of the current BD on the BD ring.
 *
 * @param	InstancePtr is the driver instance we are working on
 * @param	BdPtr is the current BD
 *
 * @return	The previous BD on the ring from the current BD
 *
 * @note	None.
 *
 *****************************************************************************/
XAxiCdma_Bd *XAxiCdma_BdRingPrev(XAxiCdma *InstancePtr, XAxiCdma_Bd *BdPtr)
{
	UINTPTR ReturnBd;

	/* Check whether the BD ptr is valid
	 * A BD ptr is not valid if:
	 * - It is outside of the BD memory range
	 * - It has invalid alignment
	 */
	if (((UINTPTR)BdPtr > InstancePtr->LastBdAddr)  ||
	    ((UINTPTR)BdPtr < InstancePtr->FirstBdAddr) ||
		((UINTPTR)BdPtr & (InstancePtr->BdSeparation - 1))) {

		xdbg_printf(XDBG_DEBUG_ERROR, "Invalid BdPtr %x: %x/%x/%x\r\n",
			(UINTPTR)BdPtr, (UINTPTR)InstancePtr->FirstBdAddr,
			(UINTPTR)InstancePtr->LastBdAddr,
		    (unsigned int)XAXICDMA_BD_MINIMUM_ALIGNMENT);
		ReturnBd = 0x0;
	}

	/* If the current BD is the first BD in the ring, return the last BD
	 */
	else if ((UINTPTR)BdPtr == InstancePtr->FirstBdAddr) {
		ReturnBd = InstancePtr->LastBdAddr;
	}
	else {
		ReturnBd = (UINTPTR)BdPtr - InstancePtr->BdSeparation;
	}

	 return (XAxiCdma_Bd *)ReturnBd;
}

/*****************************************************************************/
/**
 * This function creates the BD ring for the driver instance. If a BD ring
 * pre-exist of this ring, the previous ring is lost.
 *
 * @param	InstancePtr is the driver instance we are working on
 * @param	PhysAddr is the physical address of the memory for the BD ring
 * @param	VirtAddr is the virtual address of the memory for the BD ring
 * @param	Alignment is the alignment for the BDs
 * @param	BdCount is the number of BDs to create in the ring
 *
 * @return
 *		- XST_SUCCESS for success
 *		- XST_INVALID_PARAM for invalid parameter
 *		- XST_DMA_SG_LIST_ERROR for invalid memory region
 *		- XST_FAILURE if the hardware build is simple mode only
 *
 * @note	For a system that has flat memory layout, then the PhysAddr
 *		and the VirtAddr are the same.
 *
 *****************************************************************************/
LONG XAxiCdma_BdRingCreate(XAxiCdma *InstancePtr, UINTPTR PhysAddr,
			UINTPTR VirtAddr, u32 Alignment, int BdCount)
{
	UINTPTR BdVirtAddr;
	UINTPTR BdNxtPhysAddr;
	UINTPTR BdPhysAddr;
	int Index;

	if (InstancePtr->SimpleOnlyBuild) {
		xdbg_printf(XDBG_DEBUG_ERROR, "BdRingCreate: hardware simple "
			"mode only\r\n");
		return XST_FAILURE;
	}

	if (BdCount <= 0) {
		xdbg_printf(XDBG_DEBUG_ERROR, "BdRingCreate: non-positive "
			"BD number %d\r\n", BdCount);

		return XST_INVALID_PARAM;
	}

	/* In case there is a failure prior to creating list, make sure the
	 * following attributes are 0 to prevent calls to other SG functions
	 */
	InstancePtr->AllBdCnt = 0;
	InstancePtr->FreeBdCnt = 0;
	InstancePtr->HwBdCnt = 0;
	InstancePtr->PreBdCnt = 0;
	InstancePtr->PostBdCnt = 0;

	/* Make sure Alignment parameter meets minimum requirements */
	if (Alignment < XAXICDMA_BD_MINIMUM_ALIGNMENT) {

		xdbg_printf(XDBG_DEBUG_ERROR, "BdRingCreate: alignment too small "
			"%d, need to be at least %d\r\n", (int)Alignment,
			(int)XAXICDMA_BD_MINIMUM_ALIGNMENT);

		return XST_INVALID_PARAM;
	}

	/* Make sure Alignment is a power of 2 */
	if ((Alignment - 1) & Alignment) {

		xdbg_printf(XDBG_DEBUG_ERROR, "BdRingCreate: alignment not valid "
			"%d\r\n", (int)Alignment);

		return XST_INVALID_PARAM;
	}

	/* Make sure PhysAddr and VirtAddr have valid Alignment */
	if ((PhysAddr & (Alignment - 1)) || (VirtAddr & (Alignment - 1))) {

		xdbg_printf(XDBG_DEBUG_ERROR, "BdRingCreate: Physical address %x "
			"and/or virtual address %x have invalid alignment\r\n",
			(unsigned int)PhysAddr, (unsigned int)VirtAddr);

		return XST_INVALID_PARAM;
	}

	/* Compute how many bytes will be between the start of adjacent BDs */
	InstancePtr->BdSeparation =
		(sizeof(XAxiCdma_Bd) + (Alignment - 1)) & ~(Alignment - 1);

	/* Must make sure the ring doesn't span address 0x00000000. If it does,
	 * then the next/prev BD traversal macros will fail.
	 */
	if (VirtAddr > (VirtAddr + (InstancePtr->BdSeparation * BdCount) - 1)) {

		xdbg_printf(XDBG_DEBUG_ERROR, "BdRingCreate: BD space cross "
			"0x0\r\n");

		return XST_DMA_SG_LIST_ERROR;
	}

	/* Initial ring setup:
	 *  - Clear the entire space
	 *  - Setup each BD's next pointer with the physical address of the
	 *    next BD
	 */
	memset((void *) VirtAddr, 0, (InstancePtr->BdSeparation * BdCount));

	/* The virtual address is used to reference/flush cache for the BD
	 * The physical address is to be put in the BD for hardware reference
	 */
	BdVirtAddr = VirtAddr;
	BdNxtPhysAddr = PhysAddr + InstancePtr->BdSeparation;
	BdPhysAddr = PhysAddr;

	for (Index = 1; Index < BdCount; Index++) {
		XAxiCdma_BdSetAddrLen((XAxiCdma_Bd *)BdVirtAddr,
		    InstancePtr->AddrWidth);
		XAxiCdma_BdSetNextPtr((XAxiCdma_Bd *)BdVirtAddr, BdNxtPhysAddr);
		XAxiCdma_BdSetPhysAddr((XAxiCdma_Bd *)BdVirtAddr, BdPhysAddr);
		XAxiCdma_BdSetIsLite((XAxiCdma_Bd *)BdVirtAddr,
		    InstancePtr->IsLite);
		XAxiCdma_BdSetHasDRE((XAxiCdma_Bd *)BdVirtAddr,
		    InstancePtr->HasDRE);
		XAxiCdma_BdSetWordLen((XAxiCdma_Bd *)BdVirtAddr,
		    InstancePtr->WordLength);
		XAxiCdma_BdSetMaxLen((XAxiCdma_Bd *)BdVirtAddr,
		    InstancePtr->MaxTransLen);

		XAXICDMA_CACHE_FLUSH(BdVirtAddr);
		BdVirtAddr += InstancePtr->BdSeparation;
		BdNxtPhysAddr += InstancePtr->BdSeparation;
		BdPhysAddr += InstancePtr->BdSeparation;
	}

	/* At the end of the ring, link the last BD back to the top,
	 * and set its fields
	 */
	XAxiCdma_BdSetAddrLen((XAxiCdma_Bd *)BdVirtAddr,
                    InstancePtr->AddrWidth);
	XAxiCdma_BdSetNextPtr((XAxiCdma_Bd *)BdVirtAddr, PhysAddr);
	XAxiCdma_BdSetPhysAddr((XAxiCdma_Bd *)BdVirtAddr, BdPhysAddr);
	XAxiCdma_BdSetIsLite((XAxiCdma_Bd *)BdVirtAddr,
	    InstancePtr->IsLite);
	XAxiCdma_BdSetHasDRE((XAxiCdma_Bd *)BdVirtAddr,
	    InstancePtr->HasDRE);
	XAxiCdma_BdSetWordLen((XAxiCdma_Bd *)BdVirtAddr,
	    InstancePtr->WordLength);
	XAxiCdma_BdSetMaxLen((XAxiCdma_Bd *)BdVirtAddr,
	    InstancePtr->MaxTransLen);

	/* Setup and initialize pointers and counters */
	InstancePtr->FirstBdAddr = VirtAddr;
	InstancePtr->FirstBdPhysAddr = PhysAddr;
	InstancePtr->LastBdAddr = BdVirtAddr;
	InstancePtr->BdRingTotalLen = InstancePtr->LastBdAddr -
		InstancePtr->FirstBdAddr + InstancePtr->BdSeparation;
	InstancePtr->AllBdCnt = BdCount;
	InstancePtr->FreeBdCnt = BdCount;
	InstancePtr->FreeBdHead = (XAxiCdma_Bd *) (void *)VirtAddr;
	InstancePtr->PreBdHead = (XAxiCdma_Bd *) VirtAddr;
	InstancePtr->HwBdHead = (XAxiCdma_Bd *) VirtAddr;
	InstancePtr->HwBdTail = (XAxiCdma_Bd *) VirtAddr;
	InstancePtr->PostBdHead = (XAxiCdma_Bd *) VirtAddr;
	InstancePtr->BdaRestart = (XAxiCdma_Bd *) (void *)PhysAddr;

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
 * This function clones all BDs in the BD ring to be the same as the given
 * BD.
 *
 * @param	InstancePtr is the driver instance we are working on
 * @param	TemplateBdPtr is the BD to be copied from
 *
 * @return
 *			- XST_SUCCESS for success
 *			- XST_DMA_SG_NO_LIST if there is no BD ring
 *			- XST_DEVICE_IS_STARTED if the hardware is running
 *			- XST_DMA_SG_LIST_ERROR is the BD ring is still in use
 *
 * @note	None.
 *
 *****************************************************************************/
LONG XAxiCdma_BdRingClone(XAxiCdma *InstancePtr, XAxiCdma_Bd * TemplateBdPtr)
{
	int Index;
	UINTPTR CurBd;
	XAxiCdma_Bd TmpBd;

	/* Can't do this function if there isn't a ring */
	if (InstancePtr->AllBdCnt == 0) {

		xdbg_printf(XDBG_DEBUG_ERROR, "BdRingClone: no bds\r\n");

		return XST_DMA_SG_NO_LIST;
	}

	/* Can't do this function with the channel running */
	if (XAxiCdma_IsBusy(InstancePtr)) {

		xdbg_printf(XDBG_DEBUG_ERROR, "BdRingClone: engine is running, "
			"cannot do\r\n");

		return XST_DEVICE_IS_STARTED;
	}

	/* Can't do this function with some of the BDs in use */
	if (InstancePtr->FreeBdCnt != InstancePtr->AllBdCnt) {

		xdbg_printf(XDBG_DEBUG_ERROR,
		    "BdRingClone: some bds already in use %d/%d\r\n",
		    InstancePtr->FreeBdCnt, InstancePtr->AllBdCnt);

		return XST_DMA_SG_LIST_ERROR;
	}

	/* Make a copy of the template then clear the status bits
	 */
	memcpy(&TmpBd, TemplateBdPtr, sizeof(XAxiCdma_Bd));

	XAxiCdma_BdClearSts(&TmpBd);

	/* Starting from the top of the ring, save BD.Next, BD.PhysAddr
	 * overwrite the entire BD with the template, then restore BD.Next
	 * and BD.PhysAddr
	 */
	for (Index = 0, CurBd = InstancePtr->FirstBdAddr;
		Index < InstancePtr->AllBdCnt;
		Index++, CurBd += InstancePtr->BdSeparation) {

		XAxiCdma_BdClone((XAxiCdma_Bd *)CurBd, &TmpBd);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function requests number of BDs from the BD ring.
 *
 * @param	InstancePtr is the driver instance we are working on
 * @param	NumBd is the number of BDs to request
 * @param	BdSetPtr is the pointer to the set of BDs returned
 *
 * @return
 *			- XST_SUCCESS for success
 *			- XST_INVALID_PARAM if requests non-positive number of BDs
 *			- XST_FAILURE if not enough free BDs available
 *
 * @note	None.
 *
 *****************************************************************************/
LONG XAxiCdma_BdRingAlloc(XAxiCdma *InstancePtr, int NumBd,
   XAxiCdma_Bd ** BdSetPtr)
{
	if (NumBd <= 0) {

		xdbg_printf(XDBG_DEBUG_ERROR,
		    "BdRingAlloc: negative BD number %d\r\n", NumBd);

		return XST_INVALID_PARAM;
	}

	/* Enough free BDs available for the request? */
	if (InstancePtr->FreeBdCnt < NumBd) {
		return XST_FAILURE;
	}

	/* Set the return argument and move FreeBdHead forward */
	*BdSetPtr = InstancePtr->FreeBdHead;
	XAXICDMA_RING_SEEKAHEAD(InstancePtr, InstancePtr->FreeBdHead, NumBd);
	InstancePtr->FreeBdCnt -= NumBd;
	InstancePtr->PreBdCnt += NumBd;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function tries to free the number of BDs back to the ring.
 *
 * @param	InstancePtr is the driver instance we are working on
 * @param	NumBd is the number of BDs to return
 * @param	BdSetPtr is the set of BDs to be returned
 *
 * @return
 *			- XST_SUCCESS for success
 *			- XST_INVALID_PARAM if to free non-positive number of BDs
 *			- XST_FAILURE if BD ring management shows an error
 *
 * @note	None.
 *
 *****************************************************************************/
LONG XAxiCdma_BdRingUnAlloc(XAxiCdma *InstancePtr, int NumBd,
			 XAxiCdma_Bd * BdSetPtr)
{
	XAxiCdma_Bd *TmpBd;

	if (NumBd <= 0) {

		xdbg_printf(XDBG_DEBUG_ERROR, "BdRingUnAlloc: negative BD number "
			"%d\r\n", NumBd);

		return XST_INVALID_PARAM;
	}

	/* Enough BDs in the preprocessing state for the request? */
	if (InstancePtr->PreBdCnt < NumBd) {
		return XST_FAILURE;
	}

	/* The last BD in the BD set must has the FreeBdHead as its next BD.
	 * Otherwise, this is not a valid operation.
	 */
	TmpBd = BdSetPtr;
	XAXICDMA_RING_SEEKAHEAD(InstancePtr, TmpBd, NumBd);

	if (TmpBd != InstancePtr->FreeBdHead) {
		return XST_FAILURE;
	}

	/* Set the return argument and move FreeBdHead backward */
	XAXICDMA_RING_SEEKBACK(InstancePtr, InstancePtr->FreeBdHead, NumBd);
	InstancePtr->FreeBdCnt += NumBd;
	InstancePtr->PreBdCnt -= NumBd;

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
 * Stub call back function.
 *
 * This is used in case the user does not provide a callback function.
 * This function only releases the resources.
 *
 * @param	CallBackRef is the call back reference passed in by
 *			application,A NULL pointer is acceptable.
 * @param	IrqMask is the interrupt mask regarding this completion
 * @param	NumBdPtr is the pointer to number of BDs this handler should
 *			handle
 *
 * @return	None
 *
 * @note	None.
 *
 *****************************************************************************/
static void StubCallBackFn(void *CallBackRef, u32 IrqMask, int *NumBdPtr)
{
	XAxiCdma *InstancePtr;
	XAxiCdma_Bd *BdPtr;
	int TargetNum;
	int BdCount;

	InstancePtr = (XAxiCdma *)CallBackRef;
	TargetNum = *NumBdPtr;
	(void) (IrqMask);

	BdCount = XAxiCdma_BdRingFromHw(InstancePtr, TargetNum, &BdPtr);

	if (BdCount > 0) {
		int Status;
		Status = XAxiCdma_BdRingFree(InstancePtr, BdCount, BdPtr);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_ERROR, "CallBack: BdRingFree()"
					"failed with %08x", Status);
		}
	}

	*NumBdPtr = TargetNum - BdCount;

	return;
}

/*****************************************************************************/
/**
 * This function tries to enqueue the number of BDs to the hardware.
 *
 * @param	InstancePtr is the driver instance we are working on
 * @param	NumBd is the number of BDs to enqueue
 * @param	BdSetPtr is the set of BDs to be enqueued
 * @param	CallBackFn is the callback function for this transfer, NULL is fine
 * @param	CallBackRef is the callback reference pointer
 *
 * @return
 *			- XST_SUCCESS for success
 *			- XST_INVALID_PARAM if enqueues negative number of BDs or zero 
 *			transfer len
 *			- XST_DMA_SG_LIST_ERROR if BD ring management has a problem
 *			- XST_FIFO_NO_ROOM if the interrupt handler array is full
 *			- XST_FAILURE for:
 *			Hardware is in invalid state, for example, reset failed
 *			Or, the hardware build is simple mode only
 *
 * @note	None.
 *
 *****************************************************************************/
LONG XAxiCdma_BdRingToHw(XAxiCdma *InstancePtr, int NumBd,
        XAxiCdma_Bd *BdSetPtr, XAxiCdma_CallBackFn CallBackFn,
        void *CallBackRef)
{
	XAxiCdma_Bd *CurBdPtr;
	int Index;

	if ((!InstancePtr->Initialized) ||
	    (InstancePtr->SimpleOnlyBuild)) {
		xdbg_printf(XDBG_DEBUG_ERROR, "BdRingToHw: driver instance not "
			"in valid state or simple only build");
		return XST_FAILURE;
	}

	if (NumBd < 0) {
		xdbg_printf(XDBG_DEBUG_ERROR, "BdRingToHw: enqueue negative "
			"number of BDs %d", NumBd);
		return XST_INVALID_PARAM;
	}

	/* If the commit set is empty, do nothing */
	if (NumBd == 0) {
		return XST_SUCCESS;
	}

	/* Make sure we are in sync with XAxiCdma_BdRingAlloc() */
	if ((InstancePtr->PreBdCnt < NumBd) ||
	    (InstancePtr->PreBdHead != BdSetPtr)) {

		xdbg_printf(XDBG_DEBUG_ERROR, "Bd ring has problems\r\n");
		return XST_DMA_SG_LIST_ERROR;
	}

	/* Check whether the driver can manage another interrupt handler
	 */
	Index = InstancePtr->SgHandlerTail + 1;

	if (Index == XAXICDMA_MAXIMUM_MAX_HANDLER) {
		Index = 0;
	}

	/* If the interrupt handler array is full, we cannot submit this
	 * transfer
	 */
	if (InstancePtr->SgHandlerHead == Index) {
		return XST_FIFO_NO_ROOM;
	}

	CurBdPtr = BdSetPtr;

	/* Clear the status bits, also check BD length is positive
	 *
	 * Checking BD length can be removed if user check return status for
	 * BD set length call
	 */
	for (Index = 0; Index < NumBd; Index++) {

		/* Make sure the length value in the BD is non-zero. */
		if (XAxiCdma_BdGetLength(CurBdPtr) == 0) {

			xdbg_printf(XDBG_DEBUG_ERROR,
			    "0 length bd %x\r\n", (UINTPTR)CurBdPtr);

			return XST_INVALID_PARAM;
		}

		XAxiCdma_BdClearSts(CurBdPtr);

		/* Flush the current BD so DMA core could see the updates */
		XAXICDMA_CACHE_FLUSH((UINTPTR)CurBdPtr);

		CurBdPtr = XAxiCdma_BdRingNext(InstancePtr, CurBdPtr);
	}

	/* Rewind the current BD ptr, because the loop over steps one
	 */
	CurBdPtr = XAxiCdma_BdRingPrev(InstancePtr, CurBdPtr);

	/* This set has completed pre-processing, adjust ring pointers and
	 * counters
	 */
	XAXICDMA_RING_SEEKAHEAD(InstancePtr, InstancePtr->PreBdHead, NumBd);
	InstancePtr->PreBdCnt -= NumBd;
	InstancePtr->HwBdTail = CurBdPtr;
	InstancePtr->HwBdCnt += NumBd;

	/* Put the interrupt handler into the interrupt handler array
	 */
	Index = InstancePtr->SgHandlerTail;

	if (CallBackFn != NULL) {
		InstancePtr->Handlers[Index].CallBackFn = CallBackFn;
		InstancePtr->Handlers[Index].CallBackRef = CallBackRef;
	}
	else {   /* Use the default handler */
		InstancePtr->Handlers[Index].CallBackFn = StubCallBackFn;
		InstancePtr->Handlers[Index].CallBackRef = InstancePtr;
	}

	InstancePtr->Handlers[Index].NumBds = NumBd;

	Index += 1;

	if (Index == XAXICDMA_MAXIMUM_MAX_HANDLER) {
		Index = 0;
	}

	InstancePtr->SgHandlerTail = Index;

	/* Now check whether hardware is in SG mode
	 * If a simple transfer is still going on or cannot switch to SG
	 * mode, mark that there are SG transfers waiting
	 */
	if (XAxiCdma_IsSimpleMode(InstancePtr)) {

		if ((InstancePtr->SimpleNotDone) ||
			(XAxiCdma_SwitchMode(InstancePtr, XAXICDMA_SG_MODE)
				 != XST_SUCCESS)) {

			xdbg_printf(XDBG_DEBUG_ERROR, "BdRingToHw: Cannot "
			    "switch to SG or simple in progress\r\n");

			InstancePtr->SGWaiting = 1;
			return XST_SUCCESS;
		}
	}

	/* Signal that SG transfer has been satisfied
	 */
	InstancePtr->SGWaiting = 0;

	/* The hardware is in SG mode, update the tail BD pointer register
	 * so that hardware will process the transfer
	 */
	XAxiCdma_BdSetTailBdPtr(InstancePtr,
				XAxiCdma_BdGetPhysAddr((XAxiCdma_Bd *)InstancePtr->HwBdTail));


	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function tries to retrieve completed BDs from the hardware.
 *
 * @param	InstancePtr is the driver instance we are working on
 * @param	BdLimit is the maximum number of completed BDs to retrieve
 * @param	BdSetPtr is the set of completed BDs
 *
 * @return	The number of completed BDs.
 *
 * @note	None.
 *
 *****************************************************************************/
u32 XAxiCdma_BdRingFromHw(XAxiCdma *InstancePtr, int BdLimit,
			     XAxiCdma_Bd ** BdSetPtr)
{
	XAxiCdma_Bd *CurBdPtr;
	int BdCount;
	u32 BdSts;

	CurBdPtr = InstancePtr->HwBdHead;
	BdCount = 0;
	BdSts = 0;

	/* If no BDs in work group, then there's nothing to search */
	if (InstancePtr->HwBdCnt == 0) {
		*BdSetPtr = (XAxiCdma_Bd *)NULL;

		return 0;
	}

	if (BdLimit > InstancePtr->HwBdCnt) {
		BdLimit = InstancePtr->HwBdCnt;
	}

	/* Starting at HwBdHead, keep moving forward in the list until:
	 *  - A BD is encountered with its completed bit clear in the status
	 *    word which means hardware has not completed processing of that
	 *    BD.
	 *  - InstancePtr->HwBdTail is reached
	 *  - The number of requested BDs has been processed
	 */
	while (BdCount < BdLimit) {
		/* Read the status */
		XAXICDMA_CACHE_INVALIDATE((UINTPTR)CurBdPtr);
		BdSts = XAxiCdma_BdGetSts(CurBdPtr);

		/* If the hardware still hasn't processed this BD then we are
		 * done
		 */
		if (!(BdSts & XAXICDMA_BD_STS_COMPLETE_MASK)) {
			break;
		}

		BdCount++;

		/* Reached the end of the work group */
		if (CurBdPtr == InstancePtr->HwBdTail) {
			break;
		}

		/* Move on to the next BD in work group */
		CurBdPtr = XAxiCdma_BdRingNext(InstancePtr, CurBdPtr);
	}

	/* If BdCount is non-zero then BDs were found to return. Set return
	 * parameters, update pointers and counters, return success
	 */
	if (BdCount) {
		*BdSetPtr = InstancePtr->HwBdHead;
		InstancePtr->HwBdCnt -= BdCount;
		InstancePtr->PostBdCnt += BdCount;
		XAXICDMA_RING_SEEKAHEAD(InstancePtr,
		    InstancePtr->HwBdHead, BdCount);

		return BdCount;
	}
	else {
		*BdSetPtr = (XAxiCdma_Bd *)NULL;

		return 0;
	}

	return 0;
}

/*****************************************************************************/
/**
 * This function returns the BDs back to the free pool of the BD ring.
 *
 * @param	InstancePtr is the driver instance we are working on
 * @param	NumBd is the number of BDs to free
 * @param	BdSetPtr is the set of BDs to be freed
 *
 * @return
 *			- XST_SUCCESS for success
 *			- XST_INVALID_PARAM if number of BDs is negative
 *			- XST_DMA_SG_LIST_ERROR if the BD ring management has a problem
 *
 * @note	None.
 *
 *****************************************************************************/
u32 XAxiCdma_BdRingFree(XAxiCdma *InstancePtr, int NumBd,
		      XAxiCdma_Bd * BdSetPtr)
{
	if (NumBd < 0) {

		xdbg_printf(XDBG_DEBUG_ERROR, "BdRingFree: negative BDs%d\r\n",
			NumBd);

		return XST_INVALID_PARAM;
	}

	/* If the BD Set to free is empty, do nothing
	 */
	if (NumBd == 0) {
		return XST_SUCCESS;
	}

	/* Make sure we are in sync with XAxiCdma_BdRingFromHw() */
	if ((InstancePtr->PostBdCnt < NumBd) ||
	    (InstancePtr->PostBdHead != BdSetPtr)) {

		xdbg_printf(XDBG_DEBUG_ERROR, "BdRingFree: Error free BDs: "
		    "post count %d to free %d, PostBdHead %x to free ptr %x\r\n",
		    InstancePtr->PostBdCnt, NumBd,
		    (UINTPTR)InstancePtr->PostBdHead,
		    (UINTPTR)BdSetPtr);

		return XST_DMA_SG_LIST_ERROR;
	}

	/* Update pointers and counters */
	InstancePtr->FreeBdCnt += NumBd;
	InstancePtr->PostBdCnt -= NumBd;
	XAXICDMA_RING_SEEKAHEAD(InstancePtr, InstancePtr->PostBdHead, NumBd);

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
 * This function tries to start the SG transfer that has been submitted to
 * the driver, however, not to the hardware yet, because the hardware was
 * doing a simple transfer at the time of submit.
 *
 * This function should be called when the simple transfer is done.
 *
 * @param	InstancePtr is the driver instance we are working on
 *
 * @return
 *		- XST_SUCCESS if BD ring has been successfully started,
 *		SGWaiting will be 0
 *		- XST_FAILURE if BD ring cannot be be started, either because
 *		the hardware is simple only build or cannot switch to SG mode.
 *
 * @note	None.
 *
 *****************************************************************************/
int XAxiCdma_BdRingStartTransfer(XAxiCdma *InstancePtr)
{

	if (InstancePtr->SimpleOnlyBuild) {
		return XST_FAILURE;
	}

	/* If no BDs need to be transferred, we are done
	 */
	if (InstancePtr->HwBdCnt == 0) {
		return XST_SUCCESS;
	}

	/* See whether hardware is in simple mode
	 */
	if (XAxiCdma_IsSimpleMode(InstancePtr)) {
		/* Cannot switch to SG mode
		 */
		if ((InstancePtr->SimpleNotDone) ||
		    (XAxiCdma_SwitchMode(InstancePtr, XAXICDMA_SG_MODE)
			!= XST_SUCCESS)) {
			return XST_FAILURE;
		}
	}

	/* Now it is in SG mode, update the tail pointer to start the
	 * SG transfer
	 */
	XAxiCdma_BdSetTailBdPtr(InstancePtr,
                XAxiCdma_BdGetPhysAddr((XAxiCdma_Bd *)InstancePtr->HwBdTail));


	InstancePtr->SGWaiting = 0;

	return XST_SUCCESS;
}

/** @} */
