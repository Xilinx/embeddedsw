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
#define XCERT_ASN1_TAG_INTEGER						(0x02U)
#define XCERT_ASN1_TAG_BITSTRING					(0x03U)
#define XCERT_ASN1_TAG_OCTETSTRING					(0x04U)
#define XCERT_ASN1_TAG_NULL						(0x05U)
#define XCERT_ASN1_TAG_SEQUENCE 					(0x30U)

#define XCERT_LEN_OF_VALUE_OF_VERSION					(0x01U)
#define XCERT_LEN_OF_VALUE_OF_SERIAL					(0x14U)

#define XCERT_VERSION_VALUE_V3						(0x02U)
#define XCERT_NULL_VALUE						(0x00U)

/************************** Function Prototypes ******************************/
void XCert_CreateInteger(u8* DataBuf, const u8* IntegerVal, u32 IntegerLen, u32 *Len);
int XCert_CreateBitString(u8* DataBuf, const u8* BitStringVal, u32 BitStringLen, u32* FieldLen);
void XCert_CreateOctetString(u8* DataBuf, const u8* OctetStringVal, u32 OctetStringLen,
	u32* FieldLen);
int XCert_CreateRawData(u8* DataBuf, const u8* RawData, u32* RawDataLen);
int XCert_UpdateEncodedLength(u8* LenIdx, u32 Len, u8* ValIndex);

#ifdef __cplusplus
}
#endif
#endif  /* XCERT_CREATEFIELD_H */
/* @} */