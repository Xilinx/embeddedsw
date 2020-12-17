/******************************************************************************
* Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xiomodule_io.h
* @addtogroup iomodule_v2_10
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
* </pre>
*
******************************************************************************/

#ifndef XIOMODULE_IO_H		/* prevent circular inclusions */
#define XIOMODULE_IO_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/***************** Macros (Inline Functions) Definitions *********************/

#define XIomodule_In64 Xil_In64
#define XIomodule_Out64 Xil_Out64

#define XIomodule_In32 Xil_In32
#define XIomodule_Out32 Xil_Out32

#define XIomodule_In16 Xil_In16
#define XIomodule_Out16 Xil_Out16

#define XIomodule_In8 Xil_In8
#define XIomodule_Out8 Xil_Out8


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
