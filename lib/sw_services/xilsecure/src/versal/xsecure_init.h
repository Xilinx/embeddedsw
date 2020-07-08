/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xsecure_init.h
 *
 * Header file for xsecure_init.c
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- ---------- -------------------------------------------------------
 * 1.0   rpo  01/07/2020 Initial release
  * </pre>
 *
 * @note
 *
 ******************************************************************************/
#ifndef XSECURE_INIT_H_
#define XSECURE_INIT_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
u32 XSecure_Init(void);
void XSecure_TamperInterruptHandler(u32 ErrorNodeId, u32 ErrorMask);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_INIT_H_ */