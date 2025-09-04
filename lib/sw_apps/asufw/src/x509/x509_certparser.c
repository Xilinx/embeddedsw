/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file x509_certparser.c
 *
 * This file contains the code for parsing extension from X.509 certificate.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   rmv  05/08/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup x509_apis X.509 APIs
* @{
*/
/*************************************** Include Files *******************************************/
#include "x509_cert.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xil_sutil.h"
#include "xstatus.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/
/**
 * This structure holds parsing information for X.509 certificate.
 */
typedef struct {
	u8 *Buf;	/**< Pointer to X.509 certificate */
	u32 Offset;	/**< Current offset of buffer */
	u32 Size;	/**< Remaining size of the buffer */
	u32 FieldLen;	/**< Intermediate length of the field being processed */
} X509_CertParserInfo;

/*************************** Macros (Inline Functions) Definitions *******************************/
#define X509_LOWER7_BITS_MASK		(0x7FU)	/**< Mask to get lower 7 bits */
#define X509_MAX_DAYS_IN_MONTH		(31U)	/**< Maximum days in month(except February) */
#define X509_MIN_DAYS_IN_MONTH		(30U)	/**< Minimum days in month(except February) */
#define X509_MAX_DAYS_IN_FEB		(29U)	/**< February month days in leap year */
#define X509_MIN_DAYS_IN_FEB		(28U)	/**< February month days in regular year */
#define X509_MAX_SEC_IN_MINUTE		(59U)	/**< Maximum seconds in a minute */
#define X509_MAX_MINUTES_IN_HOUR	(59U)	/**< Maximum minute in an hour */
#define X509_MAX_HOURS_IN_DAY		(23U)	/**< Maximum hours in a day */
#define X509_MAX_VALID_YEAR		(9999U)	/**< Maximum year value */

/************************************ Function Prototypes ****************************************/
static inline s32 X509_UpdateOffsetToNextField(u32 Len);
static s32 X509_ValidateTag(u8 Tag);
static s32 X509_ValidateTimeAndDate(const X509_DateTime *DateTime);
static s32 X509_ParseTimeAndDate(u8 YearLen, X509_DateTime *DateTime);
static s32 X509_GetValidity(void);
static s32 X509_GetAlgorithm(X509_PublicKeyType *KeyType);
static s32 X509_GetEccPubKey(X509_CertInfo *CertInfo);
static s32 X509_GetPublicKeyInfo(X509_CertInfo *CertInfo);
static s32 X509_GetExtKeyUsage(X509_CertInfo *CertInfo);
static s32 X509_GetExtTypeFromOid(u32 Len, X509_ExtensionType *ExtType);
static s32 X509_GetBool(u8 *Value);
static s32 X509_GetExtensions(u32 Len, X509_CertInfo *CertInfo);
static s32 X509_GetExtensionsInfo(X509_CertInfo *CertInfo);
static s32 X509_GetVersion(u8 *Version);
static s32 X509_ParseTbs(X509_CertInfo *CertInfo);
static s32 X509_GetFieldLen(u32 *Len);
static s32 X509_GetTimeAndDate(u8 *YearLen);

/************************************ Variable Definitions ***************************************/
static X509_CertParserInfo X509_CertInstance;	/**< X.509 certificate parser instance used to hold
						parsing information */

static const u8 Oid_EcPublicKey[] = {0X2AU, 0X86U, 0X48U, 0XCEU, 0X3DU, 0X02U,
				     0X01U};	/**< ECC public key OID */
static const u8 ExtnKeyUsage[] = {0X55U, 0X1DU, 0X0FU};	/**< Key usage extension OID */

/**< X.509 public key OID list */
static X509_OidPublicKeyDescriptor PubKeyOidList[] = {
	{
		.PubKeyType = X509_PUB_KEY_ECC,
		.Oid = Oid_EcPublicKey,
		.OidLen = (u8)XASUFW_ARRAY_SIZE(Oid_EcPublicKey),
	},
};

/** X.509 extensions OID list */
static X509_ExtensionIdDescriptor ExtnList[] = {
	{
		.ExtnType = X509_EXTN_KEY_USAGE,
		.ExtnId = ExtnKeyUsage,
		.ExtnLen = (u8)XASUFW_ARRAY_SIZE(ExtnKeyUsage),
	},
};

/*************************************************************************************************/
/**
 * @brief	This function parses DER encoded X.509 certificate.
 *
 * @param	X509CertAddr	X.509 Certificate address.
 * @param	Size		Size of X.509 certificate in bytes.
 * @param	CertInfo	Pointer to the structure containing addresses to store X.509 parsed
 *				fields information.
 *
 * @return
 *	- XASUFW_SUCCESS, if X.509 certificate parsed successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_INVALID_BUFFER_SIZE, if buffer size is invalid.
 *	- XASUFW_X509_INVALID_PARAM, if parameter is invalid.
 *	- XASUFW_X509_PARSER_TAG_VALIDATION_FAILED, if tag validation is failed while parsing X.509.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 * @note	Currently this API supports only 32-bit addresses. Data type for X509CertAddr is
 *		64-bit to ensure future compatibility with system that may require 64-bit
 *		addressing.
 *
 *************************************************************************************************/
s32 X509_ParseCertificate(u64 X509CertAddr, u32 Size, X509_CertInfo *CertInfo)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Validate certificate address and CertInfo pointer. */
	if ((X509CertAddr == 0U) || (CertInfo == NULL)) {
		Status = XASUFW_X509_INVALID_PARAM;
		goto END;
	}

	/** Validate minimum TLV(Tag, Len, Value) size. */
	if (Size < X509_ASN1_TAG_LEN_VAL_MIN_SIZE) {
		Status = XASUFW_X509_INVALID_BUFFER_SIZE;
		goto END;
	}

	/** Initialize X.509 certificate parser instance. */
	X509_CertInstance.Buf = (u8 *)(UINTPTR)X509CertAddr;
	X509_CertInstance.Offset = 0U;
	X509_CertInstance.Size = Size;

	/** Validate tag for X.509 certificate. */
	Status = X509_ValidateTag(X509_ASN1_TAG_SEQUENCE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_TAG_VALIDATION_FAILED);
		goto END;
	}

	/** Parse TBS certificate. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ParseTbs(CertInfo);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function updates the offset value for buffer to point next field.
 *
 * @param	Len	Number of bytes to update the offset.
 *
 * @return
 *	- XASUFW_SUCCESS, if offset is updated successfully to next field.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_INVALID_BUFFER_SIZE, if buffer size is invalid.
 *
 *************************************************************************************************/
static inline s32 X509_UpdateOffsetToNextField(u32 Len)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	if (X509_CertInstance.Size <= Len) {
		Status = (s32)XASUFW_X509_INVALID_BUFFER_SIZE;
		goto END;
	}

	X509_CertInstance.Offset += Len;
	X509_CertInstance.Size -= Len;

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates an ASN1 tag.
 *
 * @param	Tag	Expected Tag value.
 *
 * @return
 *	- XASUFW_SUCCESS, if tag validation is successful.
 *	- XASUFW_FAILURE, if tag validation is failed.
 *	- XASUFW_X509_INVALID_BUFFER_SIZE, if buffer size is invalid.
 *	- XASUFW_X509_UNEXPECTED_TAG, if unexpected is found.
 *	- XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL, if update offset is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 *************************************************************************************************/
static s32 X509_ValidateTag(u8 Tag)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Len = 0U;

	/** Buffer shall contain minimum one byte for the tag. */
	if (X509_CertInstance.Size < X509_ASN1_TAG_LEN_VAL_MIN_SIZE) {
		Status = XASUFW_X509_INVALID_BUFFER_SIZE;
		goto END;
	}

	/** Return an error if expected tag is not found. */
	if (X509_CertInstance.Buf[X509_CertInstance.Offset] != Tag) {
		Status = XASUFW_X509_UNEXPECTED_TAG;
		goto END;
	}

	/** Move buffer to the field of current tag. */
	Status = X509_UpdateOffsetToNextField(X509_ASN1_TAG_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL);
		goto END;
	}

	/** Get the length of the field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GetFieldLen(&Len);

	X509_CertInstance.FieldLen = Len;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the date and time.
 *
 * @param	DateTime	Pointer to the structure containing time and date.
 *
 * @return
 *	- XASUFW_SUCCESS, if date and time is valid.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_INVALID_DATA, if date or time is invalid.
 *
 *************************************************************************************************/
static s32 X509_ValidateTimeAndDate(const X509_DateTime *DateTime)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u8 Days;
	u16 Year;

	/** Validate month and get number of days based on month. */
	switch (DateTime->Month) {
	case JANUARY:
	case MARCH:
	case MAY:
	case JULY:
	case AUGUST:
	case OCTOBER:
	case DECEMBER:
		Days = X509_MAX_DAYS_IN_MONTH;
		break;
	case APRIL:
	case JUNE:
	case SEPTEMBER:
	case NOVEMBER:
		Days = X509_MIN_DAYS_IN_MONTH;
		break;
	case FEBRUARY:
		Year = DateTime->Year;
		/* Leap year check. */
		if ((((Year % 4U) == 0U) && ((Year % 100U) != 0U)) || ((Year % 400U) == 0U)) {
			Days = X509_MAX_DAYS_IN_FEB;
		} else {
			Days = X509_MIN_DAYS_IN_FEB;
		}
		break;
	default:
		Status = XASUFW_X509_INVALID_DATA;
		break;
	}

	/** Validate day, year, hour, min, sec. */
	if ((Status == XASUFW_X509_INVALID_DATA) || (DateTime->Day > Days) ||
	    (DateTime->Year > X509_MAX_VALID_YEAR) ||
	    (DateTime->Hour > X509_MAX_HOURS_IN_DAY) ||
	    (DateTime->Min > X509_MAX_MINUTES_IN_HOUR) ||
	    (DateTime->Sec > X509_MAX_SEC_IN_MINUTE)) {
		Status = XASUFW_X509_INVALID_DATA;
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function parses the date and time from DER encoded data buffer.
 *
 * @param	YearLen		Year length.
 * @param	DateTime	Pointer to the structure containing date and time field to be
 *				updated.
 *
 * @return
 *	- XASUFW_SUCCESS, if date and time are parsed successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL, if update offset is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 *************************************************************************************************/
static s32 X509_ParseTimeAndDate(u8 YearLen, X509_DateTime *DateTime)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u16 Tmp;
	u8 ZeroTimeOffset;

	/**
	 * Convert year from ASCII to integer. Year length can be 2 according to UTC time format or
	 * 4 according GENERALIZED time format in digits.
	 */
	DateTime->Year = (u16)XAsufw_AsciiToInt(
			  &X509_CertInstance.Buf[X509_CertInstance.Offset], YearLen);

	if (YearLen == X509_ASN1_YEAR_LEN_UTC_BYTES) {
		Tmp = (DateTime->Year < X509_ASN1_THRESHOLD_YEARS) ?
		      X509_ASN1_YEAR_MAX_OFFSET : X509_ASN1_YEAR_MIN_OFFSET;
		DateTime->Year += Tmp;
	}

	Status = X509_UpdateOffsetToNextField(YearLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL);
		goto END;
	}

	/** Convert month, day, hour, min and sec from ASCII to integer. */
	DateTime->Month = (u8)XAsufw_AsciiToInt(&X509_CertInstance.Buf[X509_CertInstance.Offset],
						X509_ASN1_TIME_SEGMENT_LEN);
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateOffsetToNextField(X509_ASN1_TIME_SEGMENT_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL);
		goto END;
	}

	DateTime->Day = (u8)XAsufw_AsciiToInt(&X509_CertInstance.Buf[X509_CertInstance.Offset],
					      X509_ASN1_TIME_SEGMENT_LEN);
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateOffsetToNextField(X509_ASN1_TIME_SEGMENT_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL);
		goto END;
	}

	DateTime->Hour = (u8)XAsufw_AsciiToInt(&X509_CertInstance.Buf[X509_CertInstance.Offset],
					       X509_ASN1_TIME_SEGMENT_LEN);
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateOffsetToNextField(X509_ASN1_TIME_SEGMENT_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL);
		goto END;
	}

	DateTime->Min = (u8)XAsufw_AsciiToInt(&X509_CertInstance.Buf[X509_CertInstance.Offset],
					      X509_ASN1_TIME_SEGMENT_LEN);
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateOffsetToNextField(X509_ASN1_TIME_SEGMENT_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL);
		goto END;
	}

	DateTime->Sec = (u8)XAsufw_AsciiToInt(&X509_CertInstance.Buf[X509_CertInstance.Offset],
					      X509_ASN1_TIME_SEGMENT_LEN);
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateOffsetToNextField(X509_ASN1_TIME_SEGMENT_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL);
		goto END;
	}

	/** Check if the validity string ends with 'Z'(zero time offset). */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	ZeroTimeOffset = X509_CertInstance.Buf[X509_CertInstance.Offset];
	if (ZeroTimeOffset == (u8)X509_ASN1_ZERO_TIME_OFFSET_VAL) {
		Status = X509_UpdateOffsetToNextField(X509_ASN1_ZERO_TIME_OFFSET_LEN);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
							  XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL);
			goto END;
		}
	}

	/** Validate date and time. */
	Status = X509_ValidateTimeAndDate(DateTime);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function parses validity field from DER encoded data buffer.
 *
 * @return
 *	- XASUFW_SUCCESS, if validity is parsed successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_PARSER_TAG_VALIDATION_FAILED, if tag validation is failed.
 *	- XASUFW_X509_PARSER_VALIDITY_INVALID_INFO, if validity information is invalid.
 *	- XASUFW_X509_PARSER_VALIDITY_FROM_FAIL, if validity "From" field is invalid.
 *	- XASUFW_X509_PARSER_VALIDITY_TO_FAIL, if validity "To" field is invalid.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 *************************************************************************************************/
static s32 X509_GetValidity(void)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	X509_DateTime DateTime;
	u8 YearLen = 0U;

	/** Validate tag for X.509 validity. */
	Status = X509_ValidateTag(X509_ASN1_TAG_SEQUENCE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_TAG_VALIDATION_FAILED);
		goto END;
	}

	/** Get and parses time & date field for 'Not Before' field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GetTimeAndDate(&YearLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_VALIDITY_INVALID_INFO);
		goto END;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ParseTimeAndDate(YearLen, &DateTime);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_VALIDITY_FROM_FAIL);
		goto END;
	}

	/** Get and parses time & date field for 'Not After' field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GetTimeAndDate(&YearLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_VALIDITY_INVALID_INFO);
		goto END;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ParseTimeAndDate(YearLen, &DateTime);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_VALIDITY_TO_FAIL);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function parses public key algorithm from DER encoded buffer.
 *
 * @param	KeyType	Pointer to the variable to store the public key type.
 *
 * @return
 *	- XASUFW_SUCCESS, if public key algorithm is parsed successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_PARSER_TAG_VALIDATION_FAILED, if tag validation is failed.
 *	- XASUFW_X509_UNSUPPORTED_ALGORITHM, if algorithm is not supported.
 *	- XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL, if update offset is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 *************************************************************************************************/
static s32 X509_GetAlgorithm(X509_PublicKeyType *KeyType)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Idx;

	/** Initialise key type to unsupported public key. */
	*KeyType = X509_PUB_KEY_UNSUPPORTED;

	/** Validate tag for algorithm OID. */
	Status = X509_ValidateTag(X509_ASN1_TAG_OID);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_TAG_VALIDATION_FAILED);
		goto END;
	}

	/** Check supported public key OIDs and set key type if matched. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	for (Idx = 0U; Idx < XASUFW_ARRAY_SIZE(PubKeyOidList); Idx++) {
		if ((X509_CertInstance.FieldLen == PubKeyOidList[Idx].OidLen) &&
		    (Xil_SMemCmp(&X509_CertInstance.Buf[X509_CertInstance.Offset],
				 X509_CertInstance.FieldLen,
				 PubKeyOidList[Idx].Oid, PubKeyOidList[Idx].OidLen,
				 PubKeyOidList[Idx].OidLen) == XASUFW_SUCCESS)) {

			/** Move buffer to next tag */
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = X509_UpdateOffsetToNextField(X509_CertInstance.FieldLen);
			if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status,
						XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL);
				goto END;
			}

			*KeyType = PubKeyOidList[Idx].PubKeyType;

			Status = XASUFW_SUCCESS;
			goto END;
		}
	}

	Status = XASUFW_X509_UNSUPPORTED_ALGORITHM;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function parses the ECC public key from DER encoded buffer.
 *
 * @param	CertInfo	Pointer to the structure containing addresses to store X.509 parsed
 *				fields information.
 *
 * @return
 *	- XASUFW_SUCCESS, if ECC public key is parsed successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_PARSER_TAG_VALIDATION_FAILED, if tag validation is failed.
 *	- XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL, if update offset is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 *************************************************************************************************/
static s32 X509_GetEccPubKey(X509_CertInfo *CertInfo)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 PublicKeyLen;

	/** Validate tag for algorithm Parameter and move buffer to next tag. */
	Status = X509_ValidateTag(X509_ASN1_TAG_OID);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_TAG_VALIDATION_FAILED);
		goto END;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateOffsetToNextField(X509_CertInstance.FieldLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL);
		goto END;
	}

	/** Validate tag for subject public key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateTag(X509_ASN1_TAG_BIT_STRING);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_TAG_VALIDATION_FAILED);
		goto END;
	}

	/** Skip a byte indicating unused bits. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateOffsetToNextField(X509_ASN1_UNUSED_BITS_SIZE_IN_BYTE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL);
		goto END;
	}
	PublicKeyLen = X509_CertInstance.FieldLen - XASUFW_VALUE_ONE;

	/** Stores public key. */
	CertInfo->PublicKey.PubKey = &X509_CertInstance.Buf[X509_CertInstance.Offset];
	CertInfo->PublicKey.PubKeyLen = PublicKeyLen;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateOffsetToNextField(PublicKeyLen);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function parses information related to public key.
 *
 * @param	CertInfo	Pointer to the structure containing addresses to store X.509 parsed
 *				fields information.
 *
 * @return
 *	- XASUFW_SUCCESS, if public key information is parsed successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_PARSER_TAG_VALIDATION_FAILED, if tag validation is failed.
 *	- XASUFW_X509_PARSER_PUB_KEY_ALGO_FAIL, if public key algorithm parsing is failed.
 *	- XASUFW_X509_INVALID_DATA, if data is invalid.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 *************************************************************************************************/
static s32 X509_GetPublicKeyInfo(X509_CertInfo *CertInfo)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	X509_PublicKeyType KeyType;

	/** Validate tag for public key info. */
	Status = X509_ValidateTag(X509_ASN1_TAG_SEQUENCE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_TAG_VALIDATION_FAILED);
		goto END;
	}

	/** Validate tag for algorithm identifier. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateTag(X509_ASN1_TAG_SEQUENCE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_TAG_VALIDATION_FAILED);
		goto END;
	}

	/** Get algorithm OID. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GetAlgorithm(&KeyType);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_PUB_KEY_ALGO_FAIL);
		goto END;
	}

	/** Select public key extraction function based on key type. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	switch (KeyType) {
	case X509_PUB_KEY_ECC:
		Status = X509_GetEccPubKey(CertInfo);
		break;
	default:
		Status = XASUFW_X509_INVALID_DATA;
		break;
	}

	if (Status == XASUFW_SUCCESS) {
		CertInfo->PublicKey.PubKeyType = KeyType;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function parses key usage extension from DER encoded buffer.
 *
 * @param	CertInfo	Pointer to the structure containing addresses to store X.509 parsed
 *				fields information.
 *
 * @return
 *	- XASUFW_SUCCESS, if key usage extension is parsed successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_PARSER_TAG_VALIDATION_FAILED, if tag validation is failed.
 *	- XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL, if update offset is failed.
 *	- XASUFW_X509_INVALID_FIELD_LEN, if ASN.1 field length is invalid.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 *************************************************************************************************/
static s32 X509_GetExtKeyUsage(X509_CertInfo *CertInfo)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u16 KeyUsage = 0U;
	u16 UnusedBit;
	u32 Idx;
	u32 KeyUsageLen = 0U;

	/** Validate octet string tag. */
	Status = X509_ValidateTag(X509_ASN1_TAG_OCTET_STRING);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_TAG_VALIDATION_FAILED);
		goto END;
	}

	/** Validate bit string tag. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateTag(X509_ASN1_TAG_BIT_STRING);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_TAG_VALIDATION_FAILED);
		goto END;
	}

	/** Get number of unused bits. */
	UnusedBit = X509_CertInstance.Buf[X509_CertInstance.Offset];
	if (UnusedBit > X509_ASN1_MAX_UNUSED_BITS) {
		Status = XASUFW_X509_INVALID_FIELD_LEN;
		goto END;
	}

	/** Skip a byte indicating unused bits. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateOffsetToNextField(X509_ASN1_UNUSED_BITS_SIZE_IN_BYTE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL);
		goto END;
	}
	KeyUsageLen = X509_CertInstance.FieldLen - XASUFW_VALUE_ONE;

	/** Extract key usage bits. */
	for (Idx = 0U; Idx < KeyUsageLen; Idx++) {
		KeyUsage = ((KeyUsage << X509_ASN1_NO_OF_BITS_IN_BYTE) |
			    X509_CertInstance.Buf[X509_CertInstance.Offset + Idx]);
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateOffsetToNextField(KeyUsageLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL);
		goto END;
	}

	/** Discard unused bit(s). */
	KeyUsage = KeyUsage >> UnusedBit;

	/** Stores keyusage value */
	CertInfo->KeyUsage = KeyUsage;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function parses extension type using object identifier.
 *
 * @param	Len	Extension length.
 * @param	ExtType Pointer to the variable to store extension type.
 *
 * @return
 *	- XASUFW_SUCCESS, if extension type is parsed successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *
 *************************************************************************************************/
static s32 X509_GetExtTypeFromOid(u32 Len, X509_ExtensionType *ExtType)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Idx;

	/** Assign unsupported extension value. */
	*ExtType = X509_EXTN_UNSUPPORTED;

	/** Check listed extensions in ExtnList to match extension OID and set ExtType. */
	for (Idx = 0U; Idx < (u32)XASUFW_ARRAY_SIZE(ExtnList); Idx++) {
		if ((Len == ExtnList[Idx].ExtnLen) &&
		    (Xil_SMemCmp(&X509_CertInstance.Buf[X509_CertInstance.Offset], Len,
				 ExtnList[Idx].ExtnId, ExtnList[Idx].ExtnLen,
				 ExtnList[Idx].ExtnLen) == XASUFW_SUCCESS)) {
			*ExtType = ExtnList[Idx].ExtnType;
			goto END;
		}
	}

END:
	/** Skip extension OID and move buffer to next tag. */
	Status = X509_UpdateOffsetToNextField(Len);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function check whether data is boolean or not in DER encoded data buffer
 *
 * @param	Value	Pointer to the variable to store boolean value.
 *
 * @return
 *	- XASUFW_SUCCESS, if boolean value parsed successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_PARSER_TAG_VALIDATION_FAILED, if tag validation is failed.
 *	- XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL, if update offset is failed.
 *	- XASUFW_X509_BOOLEAN_TAG_NOT_FOUND, if boolean tag is not found.
 *	- XASUFW_X509_INVALID_FIELD_LEN, if ASN.1 field length is invalid.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 *************************************************************************************************/
static s32 X509_GetBool(u8 *Value)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Validate boolean tag. */
	Status = X509_ValidateTag(X509_ASN1_TAG_BOOLEAN);
	if (Status == XASUFW_X509_UNEXPECTED_TAG) {
		Status = XASUFW_X509_BOOLEAN_TAG_NOT_FOUND;
		goto END;
	} else if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_TAG_VALIDATION_FAILED);
		goto END;
	} else {
		/* Do nothing. */
	}

	/** Boolean length must be 1. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if (X509_CertInstance.FieldLen != XASUFW_VALUE_ONE) {
		Status = XASUFW_X509_INVALID_FIELD_LEN;
		goto END;
	}

	*Value = (X509_CertInstance.Buf[X509_CertInstance.Offset] == 0U) ?
		 (u8)XASU_FALSE : (u8)XASU_TRUE;

	/** Move buffer to next tag. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateOffsetToNextField(X509_CertInstance.FieldLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function parses extensions from DER encoded buffer.
 *
 * @param	Len		Length extension field.
 * @param	CertInfo	Pointer to the structure containing addresses to store X.509 parsed
 *				fields information.
 *
 * @return
 *	- XASUFW_SUCCESS, if extensions are parsed successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_BOOLEAN_TAG_NOT_FOUND, if boolean tag is not found.
 *	- XASUFW_X509_PARSER_TAG_VALIDATION_FAILED, if tag validation is failed.
 *	- XASUFW_X509_PARSER_EXT_KEY_USAGE_FAIL, if key usage extension parsing is failed.
 *	- XASUFW_X509_UNSUPPORTED_EXTN, if extension is unsupported.
 *	- XASUFW_X509_PARSER_GET_EXTN_OID_FAIL, if get extension OID is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 *************************************************************************************************/
static s32 X509_GetExtensions(u32 Len, X509_CertInfo *CertInfo)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	X509_ExtensionType ExtType;
	const u8 *Curr = &X509_CertInstance.Buf[X509_CertInstance.Offset];
	const u8 *BufEnd = Curr + Len;
	u8 IsCritical;
	u32 ExtLen;
	u32 ExtnOffset = 0U;

	while (&X509_CertInstance.Buf[X509_CertInstance.Offset] < BufEnd) {
		/** Validate sequence tag. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = X509_ValidateTag(X509_ASN1_TAG_SEQUENCE);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
							  XASUFW_X509_PARSER_TAG_VALIDATION_FAILED);
			goto END;
		}

		ExtnOffset = X509_CertInstance.Offset;
		ExtLen = X509_CertInstance.FieldLen;

		/** Validate tag for extension OID. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = X509_ValidateTag(X509_ASN1_TAG_OID);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
							  XASUFW_X509_PARSER_TAG_VALIDATION_FAILED);
			goto END;
		}

		/** Get extension OID. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = X509_GetExtTypeFromOid(X509_CertInstance.FieldLen, &ExtType);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
							  XASUFW_X509_PARSER_GET_EXTN_OID_FAIL);
			goto END;
		}

		/** Check whether extension is critical or not. */
		IsCritical = 0U;
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = X509_GetBool(&IsCritical);
		if ((Status != XASUFW_SUCCESS) && (Status != XASUFW_X509_BOOLEAN_TAG_NOT_FOUND)) {
			Status = XASUFW_X509_UNSUPPORTED_EXTN;
			goto END;
		}

		/** Report error if critical extension is not supported, else skip it. */
		if ((IsCritical != 0U) && (ExtType == X509_EXTN_UNSUPPORTED)) {
			Status = XASUFW_X509_UNSUPPORTED_EXTN;
			goto END;
		}

		/** Get supported X.509 extension types. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		switch (ExtType) {
		case X509_EXTN_KEY_USAGE:
			Status = X509_GetExtKeyUsage(CertInfo);
			if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status,
						XASUFW_X509_PARSER_EXT_KEY_USAGE_FAIL);
				goto END;
			}
			break;
		case X509_EXTN_AUTH_KEY_IDENTIFIER:
		case X509_EXTN_SUB_KEY_IDENTIFIER:
		case X509_EXTN_SUB_ALT_NAME:
		case X509_EXTN_BASIC_CONSTRAINTS:
		case X509_EXTN_EXTD_KEY_USAGE:
		default:
			Status = XASUFW_X509_UNSUPPORTED_EXTN;
			break;
		}

		/** Move buffer to next extension. */
		X509_CertInstance.Offset = ExtLen + ExtnOffset;
		X509_CertInstance.Size -= ExtLen;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function parses an information related to extensions.
 *
 * @param	CertInfo	Pointer to the structure containing addresses to store X.509 parsed
 *				fields information.
 *
 * @return
 *	- XASUFW_SUCCESS, if extensions information is parsed successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_PARSER_TAG_VALIDATION_FAILED, if tag validation is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 *************************************************************************************************/
static s32 X509_GetExtensionsInfo(X509_CertInfo *CertInfo)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Validate tag for extensions. */
	Status = X509_ValidateTag(X509_ASN1_TAG_OPTIONAL_PARAM_3_CONSTRUCTED_TAG);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_TAG_VALIDATION_FAILED);
		goto END;
	}

	/** Validate sequence tag. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateTag(X509_ASN1_TAG_SEQUENCE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_TAG_VALIDATION_FAILED);
		goto END;
	}

	/** Parse extensions. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GetExtensions(X509_CertInstance.FieldLen, CertInfo);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function parses version field from DER encoded data buffer.
 *
 * @param	Version	Pointer to the variable to store the version.
 *
 * @return
 *	- XASUFW_SUCCESS, if version is parsed successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_UNEXPECTED_TAG, if unexpected is found.
 *	- XASUFW_X509_PARSER_TAG_VALIDATION_FAILED, if tag validation is failed.
 *	- XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL, if update offset is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 *************************************************************************************************/
static s32 X509_GetVersion(u8 *Version)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Validate and extract the version field from the data buffer. */
	Status = X509_ValidateTag(X509_ASN1_TAG_OPTIONAL_PARAM_0_CONSTRUCTED_TAG);
	if (Status == XASUFW_SUCCESS) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = X509_ValidateTag(X509_ASN1_TAG_INTEGER);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
							  XASUFW_X509_PARSER_TAG_VALIDATION_FAILED);
			goto END;
		}
		*Version = X509_CertInstance.Buf[X509_CertInstance.Offset];
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = X509_UpdateOffsetToNextField(X509_CertInstance.FieldLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
							  XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL);
			goto END;
		}
	} else if (Status == XASUFW_X509_UNEXPECTED_TAG) {
		*Version = 0U;
		Status = XASUFW_SUCCESS;
	} else {
		/** Do nothing */
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function parses TBS certificate from DER encoded data buffer.
 *
 * @param	CertInfo	Pointer to the structure containing addresses to store X.509 parsed
 *				fields information.
 *
 * @return
 *	- XASUFW_SUCCESS, if TBS certificate is parsed successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL, if update offset is failed.
 *	- XASUFW_X509_PARSER_TBS_INVALID_TAG, if invalid tag found during TBS parsing.
 *	- XASUFW_X509_PARSER_GET_VERSION_FAIL, if version field parsing is failed.
 *	- XASUFW_X509_PARSER_SERIAL_NO_INVALID_TAG, if invalid tag found during serial number
 *	  parsing.
 *	- XASUFW_X509_PARSER_SIGN_ALGO_INVALID_TAG, if invalid tag found during signature algorithm
 *	  parsing.
 *	- XASUFW_X509_PARSER_ISSUER_INVALID_TAG, if invalid tag found while parsing issuer field .
 *	- XASUFW_X509_PARSER_VALIDITY_FAIL, if validity parsing is failed.
 *	- XASUFW_X509_PARSER_SUB_INVALID_TAG, if invalid tag found while parsing subject field.
 *	- XASUFW_X509_PARSER_PUBLIC_KEY_INFO_FAIL, if public key info parsing is failed.
 *	- XASUFW_X509_PARSER_EXTENSION_INFO_FAIL, if extension parsing is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 *************************************************************************************************/
static s32 X509_ParseTbs(X509_CertInfo *CertInfo)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u8 Version = 0U;

	/** Validate TBS tag. */
	Status = X509_ValidateTag(X509_ASN1_TAG_SEQUENCE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_TBS_INVALID_TAG);
		goto END;
	}

	/** Get version. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GetVersion(&Version);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_GET_VERSION_FAIL);
		goto END;
	}

	/** Validate tag for certificate serial number field and move buffer to next tag. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateTag(X509_ASN1_TAG_INTEGER);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_SERIAL_NO_INVALID_TAG);
		goto END;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateOffsetToNextField(X509_CertInstance.FieldLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL);
		goto END;
	}

	/** Validate tag for Signature algorithm field and move buffer to next tag. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateTag(X509_ASN1_TAG_SEQUENCE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_SIGN_ALGO_INVALID_TAG);
		goto END;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateOffsetToNextField(X509_CertInstance.FieldLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL);
		goto END;
	}

	/** Validate tag for issuer field and move buffer to next tag. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateTag(X509_ASN1_TAG_SEQUENCE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_ISSUER_INVALID_TAG);
		goto END;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateOffsetToNextField(X509_CertInstance.FieldLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL);
		goto END;
	}

	/** Get validity. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GetValidity();
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_VALIDITY_FAIL);
		goto END;
	}

	/** Validate tag for subject field and move buffer to next tag. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateTag(X509_ASN1_TAG_SEQUENCE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_SUB_INVALID_TAG);
		goto END;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateOffsetToNextField(X509_CertInstance.FieldLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL);
		goto END;
	}

	/** Get public key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GetPublicKeyInfo(CertInfo);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_PUBLIC_KEY_INFO_FAIL);
		goto END;
	}

	/** Parse extensions for v3 certificates. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if (Version == X509_VERSION_VALUE_V3) {
		Status = X509_GetExtensionsInfo(CertInfo);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
							  XASUFW_X509_PARSER_EXTENSION_INFO_FAIL);
		}
	}

END:
	return Status;

}

/*************************************************************************************************/
/**
 * @brief	This function parses the length field of ASN1 encoded data.
 *
 * @param	Len	Pointer to the variable to store the length value.
 *
 * @return
 *	- XASUFW_SUCCESS, if ASN1 length is valid.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_INVALID_BUFFER_SIZE, if certificate size is invalid.
 *	- XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL, if update offset is failed.
 *	- XASUFW_X509_INVALID_FIELD_LEN, if field length is invalid.
 *	- Error code received from called functions in case of other failure from the called
 *	function.
 *
 *************************************************************************************************/
static s32 X509_GetFieldLen(u32 *Len)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Idx;

	/** Buffer shall contain minimum two byte for the length. */
	if (X509_CertInstance.Size < X509_ASN1_LEN_VAL_MIN_SIZE) {
		Status = XASUFW_X509_INVALID_BUFFER_SIZE;
		goto END;
	}

	/**
	 * If MSB bit is 1, that is indicate lower 7 bits indicate the number of bytes used to
	 * encode the length else byte itself is the length.
	 */
	if ((X509_CertInstance.Buf[X509_CertInstance.Offset] & X509_ASN1_BIT7_MASK) == 0U) {
		*Len = X509_CertInstance.Buf[X509_CertInstance.Offset];
		Status = X509_UpdateOffsetToNextField(X509_SINGLE_BYTE);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
							  XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL);
			goto END;
		}
	} else {
		Idx = (u32)(X509_CertInstance.Buf[X509_CertInstance.Offset] &
			    X509_LOWER7_BITS_MASK);
		if ((Idx < X509_ASN1_MIN_LEN_BYTES) || (Idx > X509_ASN1_MAX_LEN_BYTES)) {
			Status = XASUFW_X509_INVALID_FIELD_LEN;
			goto END;
		}
		if (X509_CertInstance.Size <= Idx) {
			Status = XASUFW_X509_INVALID_BUFFER_SIZE;
			goto END;
		}
		*Len = 0U;
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = X509_UpdateOffsetToNextField(X509_SINGLE_BYTE);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
							  XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL);
			goto END;
		}
		while ((Idx--) > 0U) {
			*Len = ((*Len << X509_ASN1_NO_OF_BITS_IN_BYTE) |
				X509_CertInstance.Buf[X509_CertInstance.Offset]);
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = X509_UpdateOffsetToNextField(X509_SINGLE_BYTE);
			if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status,
						XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL);
				goto END;
			}
		}
	}

	/** Remaining certificate buffer size must be more than the length. */
	if (X509_CertInstance.Size < *Len) {
		Status = XASUFW_X509_INVALID_BUFFER_SIZE;
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function extracts time and date from DER encoded data buffer.
 *
 * @param	YearLen		Pointer to the variable to store the year length.
 *
 * @return
 *	- XASUFW_SUCCESS, if get time successful.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL, if update offset is failed.
 *	- XASUFW_X509_UNEXPECTED_TAG, if unexpected tag is found.
 *	- XASUFW_X509_INVALID_DATA, if data is invalid.
 *	- XASUFW_X509_PARSER_GET_FIELD_LEN_FAIL, if get field length is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	function.
 *
 *************************************************************************************************/
static s32 X509_GetTimeAndDate(u8 *YearLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Tag;
	u32 Len = 0U;

	/** Check ASN.1 tag for time type and set year length; error if tag is unexpected. */
	Tag = X509_CertInstance.Buf[X509_CertInstance.Offset];
	if (Tag == X509_ASN1_TAG_UTC_TIME) {
		*YearLen = X509_ASN1_YEAR_LEN_UTC_BYTES;
	} else if (Tag == X509_ASN1_TAG_GENERALIZED_TIME) {
		*YearLen = X509_ASN1_YEAR_LEN_GEN_BYTES;
	} else {
		Status = XASUFW_X509_UNEXPECTED_TAG;
		goto END;
	}

	/** Move buffer to length field. */
	Status = X509_UpdateOffsetToNextField(X509_SINGLE_BYTE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL);
		goto END;
	}

	/** Get length of time and date field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GetFieldLen(&Len);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_PARSER_GET_FIELD_LEN_FAIL);
		goto END;
	}

	/** Validate ASN.1 time format length and check for optional 'Z' suffix. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if ((Len != (u32)X509_ASN1_TIME_WITH_OUT_ZERO_TIME_OFFSET(*YearLen)) &&
	    !((Len == (u32)X509_ASN1_TIME_WITH_ZERO_TIME_OFFSET(*YearLen)) &&
	      (X509_CertInstance.Buf[X509_CertInstance.Offset + Len - 1U] ==
	       (u8)'Z'))) {
		Status = XASUFW_X509_INVALID_DATA;
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/** @} */
