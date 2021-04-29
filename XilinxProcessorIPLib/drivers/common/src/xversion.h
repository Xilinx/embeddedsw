/******************************************************************************
* Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xversion.h
* @addtogroup common_v1_2
* @{
*
* This file contains the interface for the XVersion component. This
* component represents a version ID.  It is encapsulated within a component
* so that it's type and implementation can change without affecting users of
* it.
*
* The version is formatted as X.YYZ where X = 0 - 9, Y = 00 - 99, Z = a - z
* X is the major revision, YY is the minor revision, and Z is the
* compatibility revision.
*
* Packed versions are also utilized for the configuration ROM such that
* memory is minimized. A packed version consumes only 16 bits and is
* formatted as follows.
*
* <pre>
* Revision                  Range       Bit Positions
*
* Major Revision            0 - 9       Bits 15 - 12
* Minor Revision            0 - 99      Bits 11 - 5
* Compatibility Revision    a - z       Bits 4 - 0
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date   Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a xd   11/03/04 Improved support for doxygen.
* </pre>
*
******************************************************************************/

#ifndef XVERSION_H		/* prevent circular inclusions */
#define XVERSION_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xbasic_types.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/

/* the following data type is used to hold a null terminated version string
 * consisting of the following format, "X.YYX"
 */
typedef char XVersion[6];

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

void XVersion_UnPack(XVersion *InstancePtr, u16 PackedVersion);

int XVersion_Pack(XVersion *InstancePtr, u16 *PackedVersion);

int XVersion_IsEqual(XVersion *InstancePtr, XVersion *VersionPtr);

void XVersion_ToString(XVersion *InstancePtr, char *StringPtr);

int XVersion_FromString(XVersion *InstancePtr, char *StringPtr);

void XVersion_Copy(XVersion *InstancePtr, XVersion *VersionPtr);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
