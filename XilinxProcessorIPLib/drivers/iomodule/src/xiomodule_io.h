/******************************************************************************
* Copyright (C) 2011 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xiomodule_io.h
* @addtogroup iomodule_v2_11
* @{
*
* This header file contains identifiers and low-level driver functions (or
* macros) that can be used to access the device.  The user should refer to the
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------
* 1.00a sa   07/15/11 First release
* 2.11  mus  05/07/21 Fixed warnings reported by doxygen tool. It fixes
*                     CR#1088640.
* </pre>
*
******************************************************************************/
/**
 *@cond nocomments
 */
#ifndef XIOMODULE_IO_H		/* prevent circular inclusions */
#define XIOMODULE_IO_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif
 /**
  *@endcond
 */
/***************************** Include Files *********************************/

#include "xil_io.h"

/***************** Macros (Inline Functions) Definitions *********************/
/**
 *@cond nocomments
 */
#define XIomodule_In64 Xil_In64
#define XIomodule_Out64 Xil_Out64

#define XIomodule_In32 Xil_In32
#define XIomodule_Out32 Xil_Out32

#define XIomodule_In16 Xil_In16
#define XIomodule_Out16 Xil_Out16

#define XIomodule_In8 Xil_In8
#define XIomodule_Out8 Xil_Out8
/**
 *@endcond
 */

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
