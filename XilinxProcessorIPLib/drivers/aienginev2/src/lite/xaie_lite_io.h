/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_lite_io.h
* @{
*
* This header file defines a lightweight version of AIE driver IO APIs.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Nishad   08/30/2021  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIE_LITE_IO_H
#define XAIE_LITE_IO_H

/***************************** Include Files *********************************/
#include "xaie_lite.h"
#include "xaiegbl.h"

/************************** Constant Definitions *****************************/
/************************** Variable Definitions *****************************/
/************************** Function Prototypes  *****************************/

#ifdef XAIE_FEATURE_LITE

#ifdef XAIE_ENABLE_INPUT_CHECK
#define _XAIEPRINT printf
#define XAIE_ERROR_RETURN(ERRCON, RET, ...) {	\
	if (ERRCON) {				\
		_XAIEPRINT(__VA_ARGS__);	\
		return (RET);			\
	}					\
}
#else
#define XAIE_ERROR_RETURN(...)
#endif

__FORCE_INLINE__
static inline XAie_LocType XAie_LPartGetNextNocTile(XAie_DevInst *DevInst,
		XAie_LocType Loc)
{
	/* TODO: should use start column of the DevInst */
	XAie_LocType lLoc = XAie_TileLoc(Loc.Col, Loc.Row);

	(void)DevInst;
	UPDT_NEXT_NOC_TILE_LOC(lLoc);
	return lLoc;
}

__FORCE_INLINE__
static inline u64 _XAie_LGetTileAddr(u32 Row, u32 Col)
{
	return (Row << XAIE_ROW_SHIFT) | (Col << XAIE_COL_SHIFT);
}

__FORCE_INLINE__
static inline u64 _XAie_LPartGetTileAddr(XAie_DevInst *DevInst,
		XAie_LocType Loc)
{
	return DevInst->BaseAddr + _XAie_LGetTileAddr(Loc.Row, Loc.Col);
}

#if defined(__AIECUSTOMIO__)

extern inline void _XAie_LRawWrite32(u64 RegAddr, u32 Value);
extern inline void _XAie_LRawMaskWrite32(u64 RegAddr, u32 Mask, u32 Value);
extern inline u32 _XAie_LRawRead32(u64 RegAddr);
extern inline u32 _XAie_LRawPoll32(u64 RegAddr, u32 Mask, u32 Value, u32 TimeoutUs);

#elif defined(__AIEDEBUG__)
__FORCE_INLINE__
static inline void _XAie_LRawWrite32(u64 RegAddr, u32 Value)
{
	printf("W: %p, 0x%x\n", (void *) RegAddr, Value);
}

__FORCE_INLINE__
static inline void _XAie_LRawMaskWrite32(u64 RegAddr, u32 Mask, u32 Value)
{
	printf("MW: %p, 0x%x 0x%x\n", (void *) RegAddr, Mask, Value);
}

__FORCE_INLINE__
static inline u32 _XAie_LRawRead32(u64 RegAddr)
{
	printf("R: %p\n", (void *) RegAddr);
	return 0;
}

__FORCE_INLINE__
static inline int _XAie_LRawPoll32(u64 RegAddr, u32 Mask, u32 Value, u32 TimeOutUs)
{
	(void)TimeOutUs;
	printf("P: %p, 0x%x, 0x%x\n", (void *) RegAddr, Mask, Value);
	return 0;
}

#else

extern int usleep(unsigned int usec);

__FORCE_INLINE__
static inline void _XAie_LRawWrite32(u64 RegAddr, u32 Value)
{
	*(volatile u32*) RegAddr = Value;
}

__FORCE_INLINE__
static inline void _XAie_LRawMaskWrite32(u64 RegAddr, u32 Mask, u32 Value)
{
	u32 RegVal = *(volatile u32*) RegAddr;

	RegVal = (RegVal & (~Mask)) | Value;
	*(volatile u32*) RegAddr = RegVal;
}

__FORCE_INLINE__
static inline u32 _XAie_LRawRead32(u64 RegAddr)
{
	return *(volatile u32*) RegAddr;
}

__FORCE_INLINE__
static inline int _XAie_LRawPoll32(u64 RegAddr, u32 Mask, u32 Value, u32 TimeOutUs)
{
	u32 MinTimeOutUs = 20, Count, RegVal;
	Count = TimeOutUs / MinTimeOutUs;

	if(Count == 0) {
		Count++;
	}
	do {
		RegVal = _XAie_LRawRead32(RegAddr);
		if((RegVal & Mask) == Value) {
			return 0;
		}
		usleep(MinTimeOutUs);
	} while(Count--);

	return -1;
}
#endif

__FORCE_INLINE__
static inline void _XAie_LWrite32(u64 RegAddr, u32 Value)
{
	_XAie_LRawWrite32((XAIE_BASE_ADDR + RegAddr), Value);
}

__FORCE_INLINE__
static inline u32 _XAie_LRead32(u64 RegAddr)
{
	return _XAie_LRawRead32(XAIE_BASE_ADDR + RegAddr);
}

__FORCE_INLINE__
static inline void _XAie_LPartWrite32(XAie_DevInst *DevInst, u64 RegAddr,
		u32 Value)
{
	_XAie_LRawWrite32((DevInst->BaseAddr + RegAddr), Value);
}

__FORCE_INLINE__
static inline void _XAie_LPartMaskWrite32(XAie_DevInst *DevInst, u64 RegAddr,
		u32 Mask, u32 Value)
{
	_XAie_LRawMaskWrite32((DevInst->BaseAddr + RegAddr), Mask, Value);
}

__FORCE_INLINE__
static inline void _XAie_LPartBlockSet32(XAie_DevInst *DevInst, u64 RegAddr,
		u32 Value, u32 SizeByte)
{
	for (u32 Count = 0; Count < SizeByte / sizeof(u32); Count++) {
		_XAie_LPartWrite32(DevInst, RegAddr, Value);
		RegAddr += sizeof(u32);
	}
}

__FORCE_INLINE__
static inline u32 _XAie_LPartRead32(XAie_DevInst *DevInst, u64 RegAddr)
{
	return _XAie_LRawRead32(DevInst->BaseAddr + RegAddr);
}

__FORCE_INLINE__
static inline u32 _XAie_LPartPoll32(XAie_DevInst *DevInst, u64 RegAddr,
		u32 Mask, u32 Value, u32 TimeOutUs)
{
	return _XAie_LRawPoll32((DevInst->BaseAddr + RegAddr), Mask, Value,
			TimeOutUs);
}

#endif /* XAIE_FEATURE_LITE */
#endif /* XAIE_LITE_IO_H */

/** @} */
