/******************************************************************************
* Copyright (c) 2023, Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2024, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcert_createfield.c
*
* This file contains the implementation of the interface functions for creating
* DER encoded ASN.1 structures.
* DER is a type-length-value encoding. As we parse the encoded value,
* the first byte represents the ASN.1 tag. This is a byte, or series of bytes,
* that tells you what type of thing is encoded.
* Next byte represents length. This tells the length of the value in bytes.
* Followed by length, is the value itself.
* As an example, the hex bytes 02 03 01 00 01 would represent an INTEGER
* (tag 02 corresponds to the INTEGER type), with length 03,
* and a three-byte value consisting of 01 00 01.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   har  01/09/2023 Initial release
* 1.2   pre  26/12/2023 Avoids infinite loop in XCert_GetTrailingZeroesCount
*                       function
*       am   01/31/2024 Moved entire file under PLM_OCP_KEY_MNGMT macro
*       kpt  02/21/2024 Added support for DME extension
*       har  05/03/2024 Fixed size when it is of long form
* 1.3   har  05/07/2024 Added doxygen grouping and tags
*			Fixed doxygen warnings
*       har  09/17/2024 Fixed doxygen warnings
*
* </pre>
* @note
*
******************************************************************************/
/**
 * @addtogroup xcert_apis XilCert APIs
 * @{
 */


/**
 * @cond xcert_internal
 * @{
 */
/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_OCP_KEY_MNGMT
#include "xcert_createfield.h"
#include "xil_mem.h"
#include "xil_util.h"
#include "xsecure_utils.h"

/************************** Constant Definitions *****************************/
#define XCERT_MAX_FIELD_VAL_LENGTH					(200U)
					/**< Max length of value of field */
#define XCERT_BIT7_MASK 						(0x80U)
					/**< Mask to get bit 7*/

#define XCERT_SHORT_FORM_MAX_LENGTH_IN_BYTES				(127U)
	/**< Max length for which short form encoding of length is used */
#define XCERT_LONG_FORM_2_BYTES_MAX_LENGTH_IN_BYTES			(255U)
	/**< Max length for which long form encoding of length is used */
#define XCERT_LONG_FORM_LENGTH_1BYTE					(0x81U)
	/**< To indicate that length is 1 byte long*/
#define XCERT_LONG_FORM_LENGTH_2BYTES					(0x82U)
	/**< To indicate that length is 2 bytes long*/
#define XCERT_BYTE0_MASK						(0xFFU)
					/**< Mask to get byte 0 */
#define XCERT_BYTE1_MASK						(0xFF00U)
					/**< Mask to get byte 1 */
#define XCERT_LENGTH_OF_BYTE_IN_BITS					(8U)
					/**< Length of byte in bits */

/************************** Function Prototypes ******************************/
static u32 XCert_GetTrailingZeroesCount(u8 Data);

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
 * @brief	This function creates DER encoded ASN.1 Integer
 *
 * @param	DataBuf		Pointer to the buffer where the encoded data needs to be updated
 * @param	IntegerVal	Value of the ASN.1 Integer
 * @param	IntegerLen	Length of the value of the ASN.1 Integer
 * @param	FieldLen	Total length of the encoded ASN.1 Integer
 *
 * @return
 *		 - XST_SUCCESS  Successfully created DER encoded ASN.1 Integer
 *		 - XST_FAILURE  In case of failure
 *
 * @note	ASN.1 tag for Integer is 0x02.
 *
 ******************************************************************************/
int XCert_CreateInteger(u8* DataBuf, const u8* IntegerVal, u32 IntegerLen, u32* FieldLen)
{
	int Status = XST_FAILURE;
	u8* Curr = DataBuf;
	u8* IntegerLenIdx;
	u8* IntegerValIdx;

	*(Curr++) = XCERT_ASN1_TAG_INTEGER;
	IntegerLenIdx = Curr++;
	IntegerValIdx = Curr;

	/**
	 * If the most significant bit in the first byte of IntegerVal is set,
	 * then the value must be prepended with 0x00.
	 */
	if ((*IntegerVal & XCERT_BIT7_MASK) == XCERT_BIT7_MASK) {
		*(Curr++) = 0x0;
	}

	Status  = Xil_SMemCpy(Curr, IntegerLen, IntegerVal, IntegerLen, IntegerLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Curr = Curr + IntegerLen;
	*IntegerLenIdx = (u8)(Curr - IntegerValIdx);
	*FieldLen = (u8)(Curr - DataBuf);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function creates DER encoded ASN.1 BitString
 *
 * @param	DataBuf		Pointer to the buffer where the encoded data needs to be updated
 * @param	BitStringVal	Value of the ASN.1 BitString
 * @param	BitStringLen	Length of the value of the ASN.1 BitString in bytes
 * @param	IsLastByteFull	Flag to check if the last byte is full or not
 * 			- FALSE for Key Usage
 * 			- TRUE for Public Key
 * @param	FieldLen	Total length of the encoded ASN.1 BitString
 *
 * @return
 *		 - XST_SUCCESS  Successfully created DER encoded ASN.1 BitString
 *		 - XST_FAILURE  In case of failure
 *
 * @note	ASN.1 tag for BitString is 0x03
 *
 ******************************************************************************/
int XCert_CreateBitString(u8* DataBuf, const u8* BitStringVal, u32 BitStringLen, u32 IsLastByteFull, u32* FieldLen)
{
	int Status = XST_FAILURE;
	u32 NumofTrailingZeroes = 0U;
	u8* Curr = DataBuf;
	u8* BitStringLenIdx;
	u8* BitStringValIdx;

	*(Curr++) = XCERT_ASN1_TAG_BITSTRING;
	BitStringLenIdx = Curr++;
	BitStringValIdx = Curr;

	/**
	 * The first byte of the value of the BITSTRING is used to show the number
	 * of unused bits in the last byte of the BITSTRING.
	 */
	if (IsLastByteFull == FALSE) {
		NumofTrailingZeroes = XCert_GetTrailingZeroesCount(*(BitStringVal + BitStringLen - 1U));
	}
	*(Curr++) = NumofTrailingZeroes;

	Status  = Xil_SMemCpy(Curr, BitStringLen, BitStringVal, BitStringLen, BitStringLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Curr = Curr + BitStringLen;
	*BitStringLenIdx = (u8)(Curr - BitStringValIdx);
	*FieldLen = (u8)(Curr - DataBuf);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function creates DER encoded ASN.1 OctetString
 *
 * @param	DataBuf		Pointer to the buffer where the encoded data needs to be updated
 * @param	OctetStringVal	Value of the ASN.1 OctetString
 * @param	OctetStringLen	Length of the value of the ASN.1 OctetString
 * @param	FieldLen	Total length of the encoded ASN.1 OctetString
 *
 * @return
 *		 - XST_SUCCESS  Successfully created DER encoded ASN.1 OctetString
 *		 - XST_FAILURE  In case of failure
 *
 * @note	ASN.1 tag for OctetString is 0x04
 *
 ******************************************************************************/
int XCert_CreateOctetString(u8* DataBuf, const u8* OctetStringVal, u32 OctetStringLen, u32* FieldLen)
{
	int Status = XST_FAILURE;
	u8* Curr = DataBuf;
	u8* OctetStringLenIdx;
	u8* OctetStringValIdx;

	*(Curr++) = XCERT_ASN1_TAG_OCTETSTRING;
	OctetStringLenIdx = Curr++;
	OctetStringValIdx = Curr;

	Status  = Xil_SMemCpy(Curr, OctetStringLen, OctetStringVal, OctetStringLen, OctetStringLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + OctetStringLen;

	Status = XCert_UpdateEncodedLength(OctetStringLenIdx, (u32)(Curr - OctetStringValIdx), OctetStringValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((*OctetStringLenIdx & (u8)(~XCERT_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		Curr = Curr + ((*OctetStringLenIdx) & XCERT_LOWER_NIBBLE_MASK);
	}
	*FieldLen = (u8)(Curr - DataBuf);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function takes DER encoded data as input in form of byte array
 *		and updates it in the provided buffer.
 *
 * @param	DataBuf		Pointer to the buffer where the encoded data needs to be updated
 * @param	RawData		DER encoded value as byte array to be updated in buffer
 * @param	LenOfRawDataVal	Length of DER encoded value
 * @param	RawDataFieldLen	Total length of the field
 *
 * @return
 *		 - XST_SUCCESS  If update is successful
 *		 - XST_FAILURE  In case of failure
 *
 ******************************************************************************/
int XCert_CreateRawDataFromByteArray(u8* DataBuf, const u8* RawData, const u32 LenOfRawDataVal, u32* RawDataFieldLen)
{
	int Status = XST_FAILURE;
	u8* Curr = DataBuf;

	Status = Xil_SMemCpy(Curr, LenOfRawDataVal, RawData, LenOfRawDataVal, LenOfRawDataVal);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Curr = Curr + LenOfRawDataVal;
	*RawDataFieldLen = (u8)(Curr - DataBuf);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function creates DER encoded ASN.1 Boolean
 *
 * @param	DataBuf		Pointer to the buffer where the encoded data needs to be updated
 * @param	BooleanVal	Can be TRUE or FALSE
 * @param	FieldLen	Total length of the encoded ASN.1 Boolean
 *
 * @note	ASN.1 tag for Boolean is 0x01
 *
 ******************************************************************************/
void XCert_CreateBoolean(u8* DataBuf, const u8 BooleanVal, u32* FieldLen)
{
	u8* Curr = DataBuf;

	*(Curr++) = XCERT_ASN1_TAG_BOOLEAN;
	*(Curr++) = XCERT_LEN_OF_VALUE_OF_BOOLEAN;

	if (BooleanVal == (u8)TRUE) {
		*(Curr++) = (u8)XCERT_BOOLEAN_TRUE;
	}
	else {
		*(Curr++) = (u8)XCERT_BOOLEAN_FALSE;
	}

	*FieldLen = (u8)(Curr - DataBuf);
}

/*****************************************************************************/
/**
 * @brief	This function encodes the Length field in the DER encoded value
 *		and updates in the provided pointer. In case the Length field
 *		requires more than one byte, it also shifts the value accordingly.
 *
 * @param	LenIdx	Pointer to the Length field of the encoded value
 * @param	Len	Length of the Value field in bytes
 * @param	ValIdx	Pointer to the Value field of the encoded value
 *
 * @return
 *		- XST_SUCCESS  If updating encoded length is success
 *		- XST_FAILURE  Upon any failure
 *
 * @note	The Length field in DER encoded value identifies the number of
 *		bytes encoded in the Value field.
 *		The encoding of length can take two forms: short or long.
 *		The short form is a single byte, between 0 and 127.
 *		The long form is at least two bytes long, and has bit 7 of the
 *		first byte set to 1. Bits 0-6 of the first byte indicate
 *		how many more bytes are in the length field itself.
 *		Then the remaining bytes specify the length itself,
 *		as a multi-byte integer.
 *
 ******************************************************************************/
int XCert_UpdateEncodedLength(u8* LenIdx, u32 Len, u8* ValIdx)
{
	int Status = XST_FAILURE;

	if (Len <= XCERT_SHORT_FORM_MAX_LENGTH_IN_BYTES) {
		*LenIdx = (u8)Len;
		Status = XST_SUCCESS;
	}
	else if ((Len > XCERT_SHORT_FORM_MAX_LENGTH_IN_BYTES) && (Len <=
		XCERT_LONG_FORM_2_BYTES_MAX_LENGTH_IN_BYTES)) {
		*LenIdx = XCERT_LONG_FORM_LENGTH_1BYTE;
		Status = Xil_SMemMove(ValIdx + 1U, Len, ValIdx, Len, Len);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		*(LenIdx + 1U) = (u8)Len;
	}
	else {
		*LenIdx = XCERT_LONG_FORM_LENGTH_2BYTES;
		Status = Xil_SMemMove(ValIdx + 2U, Len, ValIdx, Len, Len);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		*(LenIdx + 1U) = (u8)((Len & XCERT_BYTE1_MASK) >> XCERT_LENGTH_OF_BYTE_IN_BITS);
		*(LenIdx + 2U) = (u8)(Len & XCERT_BYTE0_MASK);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function takes a byte of data as input and returns number of
 * 		trailing zeroes in that byte.
 *
 * @param	Data	Input byte for which number of trailing zeroes need to be counted
 *
 * @return
 *		Number of trailing zeroes. In case the Data is 0 then number of
 *		trailing zeroes is 8
 *
 ******************************************************************************/
static u32 XCert_GetTrailingZeroesCount(u8 Data)
{
	u32 Count = 0;

	if (Data != 0x0U) {
		while ((Data & 0x1U) == 0x0U) {
			Data = Data >> 0x1U;
			Count++;
		}
	}
	else {
		Count = XCERT_LENGTH_OF_BYTE_IN_BITS;
	}

	return Count;
}
#endif  /* PLM_OCP_KEY_MNGMT */

/**
 * @}
 * @endcond
 */
