/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_elliptic_ipihandler.h
* @addtogroup xsecure_apis XilSecure Versal APIs
* @{
* @cond xsecure_internal
* This file contains the xilsecure elliptic IPI handler declaration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kal   03/04/2021 Initial release
* 4.6   har   07/14/2021 Fixed doxygen warnings
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

#ifndef XSECURE_ELLIPTIC_IPIHANDLER_H_
#define XSECURE_ELLIPTIC_IPIHANDLER_H_

#ifdef __cplusplus
extern "c" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_cmd.h"

/************************** Constant Definitions *****************************/
int XSecure_EllipticIpiHandler(XPlmi_Cmd *Cmd);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_ELLIPTIC_IPIHANDLER_H_ */
