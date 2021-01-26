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

#endif /* XAIE_IO_COMMON_H */

/** @} */
