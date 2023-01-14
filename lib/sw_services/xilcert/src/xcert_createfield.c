/******************************************************************************
* Copyright (c) 2023, Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023, Advanced Micro Devices, Inc.  All rights reserved.
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
*
* </pre>
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xcert_createfield.h"
#include "xil_mem.h"
#include "xil_util.h"
#include "xsecure_utils.h"

/************************** Constant Definitions *****************************/
#define XCERT_MAX_FIELD_VAL_LENGTH					(200U)
#define XCERT_BIT7_MASK 						(0x80)

#define XCERT_SHORT_FORM_MAX_LENGTH_IN_BYTES				(127U)
#define XCERT_LONG_FORM_2_BYTES_MAX_LENGTH_IN_BYTES			(255U)
#define XCERT_LONG_FORM_LENGTH_1BYTE					(0x81U)
#define XCERT_LONG_FORM_LENGTH_2BYTES					(0x82U)
#define XCERT_BYTE0_MASK						(0xFFU)
#define XCERT_BYTE1_MASK						(0xFF00U)
#define XCERT_LENGTH_OF_BYTE_IN_BITS					(8)

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
 * @brief	This function creates DER encoded ASN.1 Integer
 *
 * @param	DataBuf is the pointer to the buffer where the encoded data
		needs to be updated
 * @param	IntegerVal is the value of the ASN.1 Integer
 * @param	IntegerLen is the length of the value of the ASN.1 Integer
 * @param	FieldLen is the total length of the encoded ASN.1 Integer
 *
 * @note	ASN.1 tag for Integer is 0x02.
 *
 ******************************************************************************/
void XCert_CreateInteger(u8* DataBuf, const u8* IntegerVal, u32 IntegerLen, u32* FieldLen)
{
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

	XSecure_MemCpy64((u64)(UINTPTR)Curr, (u64)(UINTPTR)IntegerVal, IntegerLen);
	Curr = Curr + IntegerLen;
	*IntegerLenIdx = Curr - IntegerValIdx;
	*FieldLen = Curr - DataBuf;
}

/*****************************************************************************/
/**
 * @brief	This function creates DER encoded ASN.1 BitString
 *
 * @param	DataBuf is the pointer to the buffer where the encoded data
		needs to be updated
 * @param	BitStringVal is the value of the ASN.1 BitString
 * @param	BitStringLen is the length of the value of the ASN.1 BitString
 * @param	FieldLen is the total length of the encoded ASN.1 BitString
 *
 * @note	ASN.1 tag for BitString is 0x03
 *
 ******************************************************************************/
void XCert_CreateBitString(u8* DataBuf, const u8* BitStringVal, u32 BitStringLen, u32* FieldLen)
{
	u8* Curr = DataBuf;

	*(Curr++) = XCERT_ASN1_TAG_BITSTRING;
	*(Curr++) = BitStringLen;

	XSecure_MemCpy64((u64)(UINTPTR)Curr, (u64)(UINTPTR)BitStringVal, BitStringLen);

	Curr = Curr + BitStringLen;
	*FieldLen = Curr - DataBuf;
}

/*****************************************************************************/
/**
 * @brief	This function creates DER encoded ASN.1 OctetString
 *
 * @param	DataBuf is the pointer to the buffer where the encoded data
		needs to be updated
 * @param	OctetStringVal is the value of the ASN.1 OctetString
 * @param	OctetStringLen is the length of the value of the ASN.1 OctetString
 * @param	FieldLen is the total length of the encoded ASN.1 OctetString
 *
 * @note	ASN.1 tag for OctetString is 0x04
 *
 ******************************************************************************/
void XCert_CreateOctetString(u8* DataBuf, const u8* OctetStringVal, u32 OctetStringLen, u32* FieldLen)
{
	u8* Curr = DataBuf;

	*(Curr++) = XCERT_ASN1_TAG_OCTETSTRING;
	*(Curr++) = OctetStringLen;
	XSecure_MemCpy64((u64)(UINTPTR)Curr, (u64)(UINTPTR)OctetStringVal, OctetStringLen);
	Curr = Curr + OctetStringLen;
	*FieldLen = Curr - DataBuf;
}

/*****************************************************************************/
/**
 * @brief	This function takes DER encoded data as input in form of string and
 *			updates it in the provided buffer.
 *
 * @param	DataBuf is the pointer to the buffer where the encoded data
			needs to be updated
 * @param	RawData is the DER encoded value as string to be updated in buffer
 * @param	RawDataLen is the length of the DER encoded value
 *
 * @return
 *			- XST_SUCCESS - If update is successful
 *			- XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XCert_CreateRawDataFromStr(u8* DataBuf, const char* RawData, u32* RawDataLen)
{
	int Status = XST_FAILURE;
	u8* Curr = DataBuf;
	u8 FieldVal[XCERT_MAX_FIELD_VAL_LENGTH] = {0U};
	u32 Len = Xil_Strnlen(RawData, XCERT_MAX_FIELD_VAL_LENGTH);

	Status =  Xil_ConvertStringToHexBE(RawData, FieldVal,
		Xil_Strnlen(RawData, XCERT_MAX_FIELD_VAL_LENGTH) * 4);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XSecure_MemCpy64((u64)(UINTPTR)Curr, (u64)(UINTPTR)FieldVal, Len/2);
	Curr = Curr + Len/2;
	*RawDataLen = Curr - DataBuf;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function takes DER encoded data as input in form of byte array
 *			and updates it in the provided buffer.
 *
 * @param	DataBuf is the pointer to the buffer where the encoded data
			needs to be updated
 * @param	RawData is the DER encoded value as byte array to be updated in buffer
 * @param	RawDataLen is the length of the DER encoded value
 *
 * @return
 *		- XST_SUCCESS - If update is successful
 *		- XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
void XCert_CreateRawDataFromByteArray(u8* DataBuf, const u8* RawData, u32* RawDataLen)
{
	u8* Curr = DataBuf;
	u32 Len = Xil_Strnlen((char* )RawData, XCERT_MAX_FIELD_VAL_LENGTH);

	XSecure_MemCpy64((u64)(UINTPTR)Curr, (u64)(UINTPTR)RawData, Len);
	Curr = Curr + Len;
	*RawDataLen = Curr - DataBuf;
}

/*****************************************************************************/
/**
 * @brief	This function encodes the Length field in the DER encoded value
 *		and updates in the provided pointer. In case the Length field
 *		requires more than one byte, it also shifts the value accordingly.
 *
 * @param	LenIdx is the pointer to the Length field of the encoded value
 * @param	Len is the length of the Value field in bytes
 * @param	ValIdx is the pointer to the Value field of the encoded value
 *
 * @return
 *		- XST_SUCCESS - If updating encoded length is success
 *		- XST_FAILURE - Upon any failure
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
		*LenIdx = Len;
		Status = XST_SUCCESS;
	}
	else if ((Len > XCERT_SHORT_FORM_MAX_LENGTH_IN_BYTES) && (Len <=
		XCERT_LONG_FORM_2_BYTES_MAX_LENGTH_IN_BYTES)) {
		*LenIdx = XCERT_LONG_FORM_LENGTH_1BYTE;
		Status = Xil_SMemMove(ValIdx + 1, Len, ValIdx, Len, Len);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		*(LenIdx + 1) = Len;
	}
	else {
		*LenIdx = XCERT_LONG_FORM_LENGTH_2BYTES;
		Status = Xil_SMemMove(ValIdx + 2, Len, ValIdx, Len, Len);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		*(LenIdx + 1) = (Len & XCERT_BYTE1_MASK) >> XCERT_LENGTH_OF_BYTE_IN_BITS;
		*(LenIdx + 2) = Len & XCERT_BYTE0_MASK;
	}

END:
	return Status;
}