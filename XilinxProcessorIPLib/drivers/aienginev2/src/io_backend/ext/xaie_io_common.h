/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_io_common.h
* @{
*
* This file contains the data structures and routines for low level IO
* operations that are common accross multiple backends.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   01/20/2021 Initial creation.
* </pre>
*
******************************************************************************/
#ifndef XAIE_IO_COMMON_H
#define XAIE_IO_COMMON_H

static inline u64 XAie_IODummyGetTid(void)
{
	return 0;
}

void _XAie_IOCommon_MarkTilesInUse(XAie_DevInst *DevInst,
		XAie_BackendTilesArray *Args);

#ifndef XAIE_FEATURE_RSC_ENABLE
static inline AieRC _XAie_RequestRscCommon(XAie_DevInst *DevInst,
		XAie_BackendTilesRsc *Arg) {
	(void)DevInst;
	(void)Arg;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
static inline AieRC _XAie_ReleaseRscCommon(XAie_BackendTilesRsc *Arg) {
	(void)Arg;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
static inline AieRC _XAie_FreeRscCommon(XAie_BackendTilesRsc *Arg) {
	(void)Arg;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
static inline AieRC _XAie_RequestAllocatedRscCommon(XAie_DevInst *DevInst,
		XAie_BackendTilesRsc *Arg) {
	(void)DevInst;
	(void)Arg;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
static inline AieRC _XAie_GetRscStatCommon(XAie_DevInst *DevInst,
		XAie_BackendRscStat *Arg) {
	(void)DevInst;
	(void)Arg;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
#else /* XAIE_FEATURE_RSC_ENABLE */
AieRC _XAie_RequestRscCommon(XAie_DevInst *DevInst, XAie_BackendTilesRsc *Arg);
AieRC _XAie_ReleaseRscCommon(XAie_BackendTilesRsc *Arg);
AieRC _XAie_FreeRscCommon(XAie_BackendTilesRsc *Arg);
AieRC _XAie_RequestAllocatedRscCommon(XAie_DevInst *DevInst,
		XAie_BackendTilesRsc *Arg);
AieRC _XAie_GetRscStatCommon(XAie_DevInst *DevInst, XAie_BackendRscStat *Arg);
#endif /* XAIE_FEATURE_RSC_ENABLE */

#endif /* XAIE_IO_COMMON_H */

/** @} */
