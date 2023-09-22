/******************************************************************************
* Copyright (c) 2023, Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcert_createfield.h
*
* This file contains the implementation of the interface functions for creating
* DER encoded ASN.1 structures.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   har  01/09/2023 Initial release
*
* </pre>
* @note
*
******************************************************************************/

#ifndef XCERT_CREATEFIELD_H
#define XCERT_CREATEFIELD_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"

/************************** Constant Definitions *****************************/
/** @name ASN.1 tags
 * @{
 */
 /**< ASN.1 tags */
#define XCERT_ASN1_TAG_BOOLEAN						(0x01U)
#define XCERT_ASN1_TAG_INTEGER						(0x02U)
#define XCERT_ASN1_TAG_BITSTRING					(0x03U)
#define XCERT_ASN1_TAG_OCTETSTRING					(0x04U)
#define XCERT_ASN1_TAG_NULL						(0x05U)
#define XCERT_ASN1_TAG_SEQUENCE 					(0x30U)
#define XCERT_ASN1_TAG_SET						(0x31U)
/** @} */

#define XCERT_LEN_OF_VALUE_OF_VERSION					(0x01U)
						/**< Length of Version field */
#define XCERT_LEN_OF_VALUE_OF_SERIAL					(0x14U)
						/**< Length of Serial field */
#define XCERT_LEN_OF_VALUE_OF_BOOLEAN					(0x1U)
						/**< Length of ASN.1 Boolean */
#define XCERT_LEN_OF_VALUE_OF_PATH_LEN_CONSTRAINT			(0x1U)
				/**< Length of Path Length Constraint field */

#define XCERT_VERSION_VALUE_V1						(0x0U)
						/**< Value of Version 0 */
#define XCERT_VERSION_VALUE_V3						(0x02U)
						/**< Value of Version 3 */
#define XCERT_NULL_VALUE						(0x00U)
							/**< Value of NULL */
#define XCERT_PATH_LEN_CONSTRAINT_VALUE_0x0				(0x0U)
				/**< Value of Path Length Constraint - 0 */

#define XCERT_BOOLEAN_TRUE						(0xFFU)
						/**< Value of Boolean TRUE */
#define XCERT_BOOLEAN_FALSE						(0x0U)
						/**< Value of Boolean FALSE */
#define XCERT_LEN_OF_VERSION_FIELD					(0x3U)
						/**< Length of Version field */

/************************** Function Prototypes ******************************/
int XCert_CreateInteger(u8* DataBuf, const u8* IntegerVal, u32 IntegerLen, u32 *FieldLen);
int XCert_CreateBitString(u8* DataBuf, const u8* BitStringVal, u32 BitStringLen, u32 IsLastByteFull, u32* FieldLen);
int XCert_CreateOctetString(u8* DataBuf, const u8* OctetStringVal, u32 OctetStringLen,
	u32* FieldLen);
int XCert_CreateRawDataFromByteArray(u8* DataBuf, const u8* RawData, const u32 LenOfRawDataVal, u32* RawDataFieldLen);
void XCert_CreateBoolean(u8* DataBuf, const u8 BooleanVal, u32* FieldLen);
int XCert_UpdateEncodedLength(u8* LenIdx, u32 Len, u8* ValIdx);

#ifdef __cplusplus
}
#endif
#endif  /* XCERT_CREATEFIELD_H */
/* @} */
