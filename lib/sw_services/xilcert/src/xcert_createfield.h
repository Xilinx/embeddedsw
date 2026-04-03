/******************************************************************************
* Copyright (c) 2023, Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2026, Advanced Micro Devices, Inc.  All rights reserved.
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
* 1.2   am   01/31/2024 Moved entire file under PLM_OCP_KEY_MNGMT macro
*       kpt  02/21/2024 Added support for DME extension
*       kpt  11/19/2024 Add UTF8 encoding support for version field
* 1.6   rmv  01/30/2026 Renamed OCP keymanagement macro
*       tbk  05/03/2026 Add validation for xcert remaining buffer before writing field
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
#include "xplmi_config.h"

#ifdef PLM_OCP_NATIVE_KEY_MGMT
#include "xil_types.h"
#include "xcert_genx509cert.h"

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
#define XCERT_LOWER_NIBBLE_MASK				(0xFU)
						/**< Mask to get lower nibble */
#define XCERT_WORD_LEN							(0x04U)
						/**< Length of word in bytes */
#define XCERT_SHORT_FORM_MAX_LENGTH_IN_BYTES				(127U)
		/**< Max length for which short form encoding of length is used */
#define XCERT_TLVHDRLEN						(3U)
				/**< Length of Tag + Length + Value fields in TLV */
#define XCERT_BYTE_LEN							(1U)
						/**< Length of byte in bytes */
#define XCERT_SERIAL_DATA_HDR_LEN					(3U)
					/**< Length of Serial Data Header */

/**< Calculates pointer difference as u8. */
#define XCERT_PTR_DIFF_U8(End, Start) \
	((u8)((UINTPTR)(End) - (UINTPTR)(Start)))

/**< Calculates pointer difference as u32. */
#define XCERT_PTR_DIFF_U32(End, Start) \
	((u32)((UINTPTR)(End) - (UINTPTR)(Start)))

/**< Checks buffer length; sets Status to XCERT_ERR_X509_INSUFFICIENT_MEMORY
 * and jumps to exit if insufficient. */
#define XCERT_CHECK_BUFFER_LENGTH(BufLen, RequiredLen, Status, Exit) \
	do { \
		if ((BufLen) < (RequiredLen)) { \
			Status = (int)XCERT_ERR_X509_INSUFFICIENT_MEMORY; \
			goto Exit; \
		} \
	} while (0U != 0U)



/**************************** Type Definitions *******************************/
/**
 * This typedef contains enumeration of the fields of X.509 certificate
 * which are configured by user
 */

 /** X.509 Certificate Information Structure */
typedef struct {
	u8 *Buff;			/**< Buffer to store X509 certificate */
	u8 *CurrOffset;			/**< Offset of Current buffer */
	u32 RemainingBytes;		/**< Remaining Byte in buffer */
} XCert_X509CertInfo;


/************************** Helper Functions *********************************/
static inline void XCert_AdvanceOffset(XCert_X509CertInfo *Info, u32 Bytes)
{
	Info->CurrOffset += Bytes;
	Info->RemainingBytes -= Bytes;
}
/************************** Function Prototypes ******************************/
int XCert_CreateInteger(const u8 *IntegerVal, u32 IntegerLen);
int XCert_CreateBitString(const u8 *BitStringVal, u32 BitStringLen, u32 IsLastByteFull);
int XCert_CreateOctetString(const u8 *OctetStringVal, u32 OctetStringLen);
int XCert_CreateRawDataFromByteArray(const u8 *RawData,	const u32 LenOfRawDataVal);

int XCert_CreateBoolean(const u8 BooleanVal);
int XCert_UpdateEncodedLength(u8 *LenIdx, u32 Len, u8 *ValIdx);
int XCert_CreateIntegerFieldFromByteArray(const u8 *IntegerVal, u32 IntegerLen);

int XCert_BuildPlmVersionAndCreateRawField(const u32 IntegerVal);
XCert_X509CertInfo *XCert_GetX509CertInstance(void);
int XCert_UpdateTLVField(u32 Tag, u8 **Len, u8 **Val);
int XCert_UpdateByteField(u8 data);

#ifdef __cplusplus
}
#endif
#endif  /* PLM_OCP_NATIVE_KEY_MGMT */
#endif  /* XCERT_CREATEFIELD_H */
/* @} */
