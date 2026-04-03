/******************************************************************************
* Copyright (c) 2023, Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2026, Advanced Micro Devices, Inc.  All rights reserved.
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
*       kpt  11/19/2024 Add UTF8 encoding support for version field
* 1.6   rmv  01/30/2026 Renamed OCP keymanagement macro
*       tbk  05/03/2026 Add validation for xcert remaining buffer before writing field
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

#ifdef PLM_OCP_NATIVE_KEY_MGMT
#include "xcert_createfield.h"
#include "xil_mem.h"
#include "xil_util.h"
#include "xsecure_utils.h"

/************************** Constant Definitions *****************************/
#define XCERT_BIT7_MASK 						(0x80U)
					/**< Mask to get bit 7*/

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
#define XCERT_SHIFT_24							(24U)
					/**< Shift by 24 places */
#define XCERT_SHIFT_16							(16U)
					/**< Shift by 16 places */
#define XCERT_SHIFT_8							(8U)
					/**< Shift by 8 places */

#define XCERT_DOT_UTF8_ENCODING                                          (0x2EU)
					/**< dot utf8 encoding */

#define XCERT_SMALLEST_3_DIGIT_VAL                                       (100U)
					/**< Smallest three digit value */

#define XCERT_SMALLEST_2_DIGIT_VAL                                       (10U)
					/**< Smallest two digit value */

#define XCERT_MAX_VER_LEN                                                (15U)
					/**< Maximum version length */

#define XCERT_PLM_VER_START_IDX                                          (2U)
					/**< PLM version start index */

#define XCERT_PLM_VER_DEF_MSB_VAL                                        "20"
					/**< PLM version default MSB value */

#define XCERT_PLM_VER_LEN_INIT_VAL                                       (2U)
					/**< PLM version initial value */

#define XCERT_ZERO_ASCII_VAL                                             (0x30U)
					/**< Zero ascii value in hex */

/************************** Function Prototypes ******************************/
static u32 XCert_GetTrailingZeroesCount(u8 Data);
static u32 XCert_ConvertNumToString(u32 Num, u8 *Out);
static u32 XCert_BuildPlmVerNumber(u32 Num, u8 *Out);
static u32 XCert_GetFirstNonZeroByteOffset(const u8 *Data, u32 Cnt);

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
 * @brief	This function return X509 Certificate info instance
 *
 * @return
 *		 Pointer to XCert_X509CertInfo instance
 *
 ******************************************************************************/
XCert_X509CertInfo *XCert_GetX509CertInstance(void)
{
	static XCert_X509CertInfo X509CertInstance; /**< X.509 certificate generation instance*/

	return &X509CertInstance;
}

/*****************************************************************************/
/**
 * @brief	This function creates DER encoded ASN.1 Integer
 *
 * @param	IntegerVal	Value of the ASN.1 Integer
 * @param	IntegerLen	Length of the value of the ASN.1 Integer
 *
 * @return
 *		 - XCERT_ERR_X509_INSUFFICIENT_MEMORY  If provided buffer is not sufficient
 *		 - XST_SUCCESS  Successfully created DER encoded ASN.1 Integer
 *		 - XST_FAILURE  In case of failure
 *
 * @note	ASN.1 tag for Integer is 0x02.
 *
 ******************************************************************************/
int XCert_CreateInteger(const u8 *IntegerVal, u32 IntegerLen)
{
	int Status = XST_FAILURE;
	u8 *IntegerLenIdx;
	u8 *IntegerValIdx;

	XCert_X509CertInfo *X509CertInfo =  XCert_GetX509CertInstance();

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_INTEGER,
					&IntegerLenIdx, &IntegerValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_CreateIntegerFieldFromByteArray(IntegerVal, IntegerLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	*IntegerLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset, IntegerValIdx);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function creates DER encoded ASN.1 BitString
 *
 * @param	BitStringVal	Value of the ASN.1 BitString
 * @param	BitStringLen	Length of the value of the ASN.1 BitString in bytes
 * @param	IsLastByteFull	Flag to check if the last byte is full or not
 * 			- FALSE for Key Usage
 * 			- TRUE for Public Key
 *
 * @return
 *		 - XST_SUCCESS  if successfully created DER encoded ASN.1 BitString
 *		 - Error  In case of failure
 *
 * @note	ASN.1 tag for BitString is 0x03
 *
 ******************************************************************************/
int XCert_CreateBitString(const u8 *BitStringVal, u32 BitStringLen, u32 IsLastByteFull)
{
	int Status = XST_FAILURE;
	u32 NumofTrailingZeroes = 0U;
	u8 *BitStringLenIdx;
	u8 *BitStringValIdx;

	XCert_X509CertInfo *X509CertInfo =  XCert_GetX509CertInstance();

	Status  = XCert_UpdateTLVField(XCERT_ASN1_TAG_BITSTRING,
					&BitStringLenIdx, &BitStringValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/**
	 * The first byte of the value of the BITSTRING is used to show the number
	 * of unused bits in the last byte of the BITSTRING.
	 */
	if (IsLastByteFull == FALSE) {
		NumofTrailingZeroes = XCert_GetTrailingZeroesCount(BitStringVal[BitStringLen - 1U]);
	}

	*(X509CertInfo->CurrOffset) = (u8)NumofTrailingZeroes;

	XCert_AdvanceOffset(X509CertInfo, XCERT_BYTE_LEN);

	Status  = Xil_SMemCpy(X509CertInfo->CurrOffset, X509CertInfo->RemainingBytes, BitStringVal,
					BitStringLen, BitStringLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XCert_AdvanceOffset(X509CertInfo, BitStringLen);
	*BitStringLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset, BitStringValIdx);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function creates DER encoded ASN.1 OctetString
 *
 * @param	OctetStringVal	Value of the ASN.1 OctetString
 * @param	OctetStringLen	Length of the value of the ASN.1 OctetString
 *
 * @return
 *		 - XST_SUCCESS  if successfully created DER encoded ASN.1 OctetString
 *		 - XST_FAILURE  In case of failure
 *
 * @note	ASN.1 tag for OctetString is 0x04
 *
 ******************************************************************************/
int XCert_CreateOctetString(const u8 *OctetStringVal, u32 OctetStringLen)
{
	int Status = XST_FAILURE;
	u8 *OctetStringLenIdx;
	u8 *OctetStringValIdx;

	XCert_X509CertInfo *X509CertInfo =  XCert_GetX509CertInstance();

	Status  = XCert_UpdateTLVField(XCERT_ASN1_TAG_OCTETSTRING,
					&OctetStringLenIdx, &OctetStringValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status  = Xil_SMemCpy(X509CertInfo->CurrOffset, X509CertInfo->RemainingBytes, OctetStringVal,
					OctetStringLen, OctetStringLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XCert_AdvanceOffset(X509CertInfo, OctetStringLen);

	Status = XCert_UpdateEncodedLength(OctetStringLenIdx,
				XCERT_PTR_DIFF_U32(X509CertInfo->CurrOffset, OctetStringValIdx),
				OctetStringValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((*OctetStringLenIdx & (u8)(~XCERT_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		XCert_AdvanceOffset(X509CertInfo,
				((u32)(*OctetStringLenIdx) & XCERT_LOWER_NIBBLE_MASK));
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function takes DER encoded data as input in form of byte array
 *		and updates it in the provided buffer.
 *
 * @param	RawData		DER encoded value as byte array to be updated in buffer
 * @param	LenOfRawDataVal	Length of DER encoded value
 *
 * @return
 *		 - XCERT_ERR_X509_INSUFFICIENT_MEMORY  If provided buffer is not sufficient
 *		 - XST_SUCCESS  If update is successful
 *		 - XST_FAILURE  In case of failure
 *
 ******************************************************************************/
int XCert_CreateRawDataFromByteArray(const u8 *RawData, const u32 LenOfRawDataVal)
{
	int Status = XST_FAILURE;
	XCert_X509CertInfo *X509CertInfo =  XCert_GetX509CertInstance();

	Status = Xil_SMemCpy(X509CertInfo->CurrOffset, X509CertInfo->RemainingBytes, RawData,
							LenOfRawDataVal, LenOfRawDataVal);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	XCert_AdvanceOffset(X509CertInfo, LenOfRawDataVal);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function creates DER encoded ASN.1 Boolean
 *
 * @param	BooleanVal	Can be TRUE or FALSE
 *
 * @return
 *		 - XCERT_ERR_X509_INSUFFICIENT_MEMORY  If provided buffer is not sufficient
 *		 - XST_SUCCESS  if successfully created DER encoded ASN.1 Boolean
 *		 - XST_FAILURE  In case of failure
 *
 * @note	ASN.1 tag for Boolean is 0x01
 *
 ******************************************************************************/
int XCert_CreateBoolean(const u8 BooleanVal)
{
	int Status = XST_FAILURE;

	Status = XCert_UpdateByteField(XCERT_ASN1_TAG_BOOLEAN);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status  = XCert_UpdateByteField(XCERT_LEN_OF_VALUE_OF_BOOLEAN);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (BooleanVal == TRUE) {
		Status  = XCert_UpdateByteField(XCERT_BOOLEAN_TRUE);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	else {
		Status  = XCert_UpdateByteField(XCERT_BOOLEAN_FALSE);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	Status = XST_SUCCESS;

END:
	return Status;
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
int XCert_UpdateEncodedLength(u8 *LenIdx, u32 Len, u8 *ValIdx)
{
	int Status = XST_FAILURE;
	XCert_X509CertInfo *X509CertInfo = XCert_GetX509CertInstance();

	if (Len <= XCERT_SHORT_FORM_MAX_LENGTH_IN_BYTES) {
		*LenIdx = (u8)Len;
		Status = XST_SUCCESS;
	}
	else if ((Len > XCERT_SHORT_FORM_MAX_LENGTH_IN_BYTES) && (Len <=
		XCERT_LONG_FORM_2_BYTES_MAX_LENGTH_IN_BYTES)) {
		XCERT_CHECK_BUFFER_LENGTH(X509CertInfo->RemainingBytes,
						XCERT_LONG_FORM_LENGTH_1BYTE, Status, END);
		*LenIdx = XCERT_LONG_FORM_LENGTH_1BYTE;
		Status = Xil_SMemMove(ValIdx + 1U, Len, ValIdx, Len, Len);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		*(LenIdx + 1U) = (u8)Len;
	}
	else {
		XCERT_CHECK_BUFFER_LENGTH(X509CertInfo->RemainingBytes,
				XCERT_LONG_FORM_LENGTH_2BYTES, Status, END);
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

/*****************************************************************************/
/**
 * @brief	This function takes a byte array as input and returns offset of an
 *              starting non-zero byte
 *
 *
 * @param	Data	Input byte array
 * @param	Cnt	Number of bytes in a given array
 *
 * @return
 *		Offset of first non-zero byte in a given array
 *
 ******************************************************************************/
static u32 XCert_GetFirstNonZeroByteOffset(const u8 *Data, u32 Cnt)
{
	u32 Offset = 0;

	while (Offset < Cnt) {
		if (Data[Offset] != 0U) {
			break;
		}
		Offset++;
	}

	return Offset;
}

/*****************************************************************************/
/**
 * @brief	This function extracts each byte from the integer value and create
 *		DER encoded ASN.1 Integer.
 *
 * @param	IntegerVal	Pointer to the Value of the ASN.1 Integer
 * @param       IntegerLen      Length of the integer
 *
 * @return
 *		- XCERT_ERR_X509_INSUFFICIENT_MEMORY  If provided buffer is not sufficient
 *		- XST_SUCCESS  Successfully created DER encoded ASN.1 integer
 *		- XST_FAILURE  In case of failure
 *
 ******************************************************************************/
int XCert_CreateIntegerFieldFromByteArray(const u8 *IntegerVal, u32 IntegerLen)
{
	int Status = XST_FAILURE;
	u32 Offset = 0U;
	XCert_X509CertInfo *X509CertInfo = XCert_GetX509CertInstance();
	Offset = XCert_GetFirstNonZeroByteOffset(IntegerVal, IntegerLen - 1U);
	/**
	 * If the most significant bit in the first byte of IntegerVal is set,
	 * then the value must be prepended with 0x00.
	 */
	if ((IntegerVal[Offset] & XCERT_BIT7_MASK) == XCERT_BIT7_MASK) {
		Status  = XCert_UpdateByteField(0x0);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	Status  = Xil_SMemCpy(X509CertInfo->CurrOffset, X509CertInfo->RemainingBytes,
						&IntegerVal[Offset], IntegerLen - Offset,
						IntegerLen - Offset);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	XCert_AdvanceOffset(X509CertInfo, (IntegerLen - Offset));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function converts a maximum three digit number to string
 *
 * @param	Num	Number that needs to be converted
 * @param	Out	Value of the string
 *
 * @return
 *              Length of converted output string
 *
 ******************************************************************************/
static u32 XCert_ConvertNumToString(u32 Num, u8 *Out)
{
	u32 InputData = Num;
	u32 Divisor = 0U;
	u32 Len = 0U;

	if (InputData >= XCERT_SMALLEST_3_DIGIT_VAL) {
		Divisor = XCERT_SMALLEST_3_DIGIT_VAL;
	}
	else if (InputData < XCERT_SMALLEST_2_DIGIT_VAL) {
		Out[Len++] = (u8)(InputData + XCERT_ZERO_ASCII_VAL);
		Divisor = 0U;
	}
	else {
		Divisor = XCERT_SMALLEST_2_DIGIT_VAL;
	}

	while (Divisor > 0U) {
		Out[Len++] = (u8)((InputData / Divisor) + XCERT_ZERO_ASCII_VAL);
		InputData = InputData % Divisor;
		Divisor = Divisor / 10U;
	}

	return Len;
}

/*****************************************************************************/
/**
 * @brief	This function builds PLM version string in below format
 *              MajorVersion.MinorVersion.RCVersion.UserDefinedVersion
 *              i.e. 2024.2.0.0
 *
 * @param	Num	Number that needs to be converted
 * @param	Out	Pointer to memory where PLM version number
 *			will be stored in string format.
 *
 * @return
 *              Length of converted output string
 *
 ******************************************************************************/
static u32 XCert_BuildPlmVerNumber(u32 Num, u8 *Out)
{
	u32 Len = 0U;
	u32 Data;
	u8 *OutStr = Out;

	Data = (Num >> XCERT_SHIFT_24) & XCERT_BYTE0_MASK;
	Len += XCert_ConvertNumToString(Data, OutStr);

	OutStr[Len++] = XCERT_DOT_UTF8_ENCODING;

	Data = (Num >> XCERT_SHIFT_16) & XCERT_BYTE0_MASK;
	Len += XCert_ConvertNumToString(Data, &OutStr[Len]);

	OutStr[Len++] = XCERT_DOT_UTF8_ENCODING;

	Data = (Num >> XCERT_SHIFT_8) & XCERT_BYTE0_MASK;
	Len += XCert_ConvertNumToString(Data, &OutStr[Len]);

	OutStr[Len++] = XCERT_DOT_UTF8_ENCODING;

	Data = Num & XCERT_BYTE0_MASK;
	Len += XCert_ConvertNumToString(Data, &OutStr[Len]);

	return Len;
}

/*****************************************************************************/
/**
 * @brief	This function builds PLM version number and creates raw data from byte array.
 *
 * @param	IntegerVal	Value of the ASN.1 Integer
 *
 * @return
 *		 - XST_SUCCESS  Successfully created PLM version field
 *		 - XST_FAILURE  In case of failure
 *
 ******************************************************************************/
int XCert_BuildPlmVersionAndCreateRawField(const u32 IntegerVal)
{
	u8 VersionData[XCERT_MAX_VER_LEN] = XCERT_PLM_VER_DEF_MSB_VAL;
	u32 VersionLength = XCERT_PLM_VER_LEN_INIT_VAL;

	VersionLength += XCert_BuildPlmVerNumber(IntegerVal, &VersionData[XCERT_PLM_VER_START_IDX]);

	return XCert_CreateRawDataFromByteArray(VersionData, VersionLength);
}

/*****************************************************************************/
/**
 * @brief	This function create the TLV field in the X.509 certificate
 *
 * @param	Tag		Tag value of the TLV field
 * @param	Len		Pointer to store the Length field address
 * @param	Val		Pointer to store the Value field address
 *
 * @return
 *		 - XST_SUCCESS  If successfully generated TLV field
 *		 - XST_FAILURE  In case of failure
 * @note
 *		The TLV field is created by writing Tag, Length and Value fields
 *		sequentially in the buffer.
 *		Tag : 1 byte
 *		Length : 1 byte
 ******************************************************************************/
int XCert_UpdateTLVField(u32 Tag, u8 **Len, u8 **Val)
{

	int Status = XST_FAILURE;
	XCert_X509CertInfo *X509CertInfo = XCert_GetX509CertInstance();

	XCERT_CHECK_BUFFER_LENGTH(X509CertInfo->RemainingBytes, XCERT_TLVHDRLEN, Status, END);

	*(X509CertInfo->CurrOffset) = (u8)Tag;
	XCert_AdvanceOffset(X509CertInfo, XCERT_BYTE_LEN);
	*Len = X509CertInfo->CurrOffset;
	XCert_AdvanceOffset(X509CertInfo, XCERT_BYTE_LEN);
	*Val = X509CertInfo->CurrOffset;
	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function create the Byte field in the X.509 certificate
 *
 * @param	Data		Data byte to be written in the buffer
 *
 * @return
 *		 - XST_SUCCESS  if successfully generated Byte field
 *		 - XST_FAILURE  In case of failure
 * @note
 *		The Byte field is created by writing a single byte in the
 *		buffer.
 ******************************************************************************/
int XCert_UpdateByteField(u8 data)
{
	int Status = XST_FAILURE;
	XCert_X509CertInfo *X509CertInfo = XCert_GetX509CertInstance();

	XCERT_CHECK_BUFFER_LENGTH(X509CertInfo->RemainingBytes, XCERT_BYTE_LEN, Status, END);

	*(X509CertInfo->CurrOffset) = data;
	XCert_AdvanceOffset(X509CertInfo, XCERT_BYTE_LEN);
	Status = XST_SUCCESS;
END:
	return Status;
}
#endif  /* PLM_OCP_KEY_MNGMT */

/**
 * @}
 * @endcond
 */
