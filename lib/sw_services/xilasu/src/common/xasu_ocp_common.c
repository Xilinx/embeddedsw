/**************************************************************************************************
* Copyright (c) 2025 - 2026, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_ocp_common.c
 *
 * This file contains the OCP function definitions which are common across the client and server.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   rmv  08/12/25 Initial release
 *       rmv  09/08/25 Add validation for signature buffer length in XAsu_OcpValidateAttestParams
 *		       function
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_ocp_common_apis OCP Common APIs
 * @{
*/

/************************************ Include Files **********************************************/
#include "xasu_ocp_common.h"
#include "xasu_def.h"
#include "xasu_eccinfo.h"

/********************************* Constant Definitions ******************************************/

/*********************************** Macros Definitions ******************************************/
/**
 * Bit positions at which the fields of a point in time are packed into a single value.
 */
#define XASU_X509_TIME_YEAR_SHIFT	(40U)	/**< Bit position of the year */
#define XASU_X509_TIME_MONTH_SHIFT	(32U)	/**< Bit position of the month */
#define XASU_X509_TIME_DAY_SHIFT	(24U)	/**< Bit position of the day */
#define XASU_X509_TIME_HOUR_SHIFT	(16U)	/**< Bit position of the hour */
#define XASU_X509_TIME_MINUTE_SHIFT	(8U)	/**< Bit position of the minute */

/*********************************** Type Definitions ********************************************/

/********************************* Variable Definitions ******************************************/

/****************************** Inline Function Definitions **************************************/

/********************************** Function Prototypes ******************************************/
static u64 XAsu_OcpGetX509TimeValue(const XAsu_X509Time *X509Time);

/*************************************************************************************************/
/**
 * @brief	This function validates the OCP certificate parameters.
 *
 * @param	OcpCertParam	Pointer to XAsu_OcpCertParams structure which holds the parameters
 *				of OCP input argument.
 *
 * @return
 *	- XST_SUCCESS, if OCP certificate parameters are validated successfully.
 *	- XST_FAILURE, if OCP certificate parameters validation is failed.
 *
 *************************************************************************************************/
s32 XAsu_OcpValidateCertParams(const XAsu_OcpCertParams *OcpCertParam)
{
	s32 Status = XST_FAILURE;

	if ((OcpCertParam->CertBufAddr == 0U) || (OcpCertParam->CertBufLen == 0U) ||
	    (OcpCertParam->CertActualSizeAddr == 0U) ||
	    (OcpCertParam->DevKeySel >= (u32)XOCP_MAX_DEVICE_KEY)) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the OCP DevAk attestation parameters.
 *
 * @param	OcpAttestParam	Pointer to XAsu_OcpDevAkAttest Params structure which holds the
 *				parameters of OCP attestation input argument.
 *
 * @return
 *	- XST_SUCCESS, if OCP attestation parameters are validated successfully.
 *	- XST_FAILURE, if OCP attestation parameters validation is failed.
 *
 *************************************************************************************************/
s32 XAsu_OcpValidateAttestParams(const XAsu_OcpDevAkAttest *OcpAttestParam)
{
	s32 Status = XST_FAILURE;

	if ((OcpAttestParam->DataAddr == 0U) || (OcpAttestParam->DataLen == 0U) ||
	    (OcpAttestParam->SignatureAddr == 0U) ||
	    (OcpAttestParam->SignatureBufLen <
	    XAsu_DoubleCurveLength(XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES))) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the user X.509 certificate request parameters.
 *
 * @param	OcpUserCertParam	Pointer to XAsu_OcpUserCertParams structure which holds the
 *					user certificate request parameters.
 *
 * @return
 *	- XST_SUCCESS, if user certificate request parameters are validated successfully.
 *	- XST_FAILURE, if user certificate request parameters validation is failed.
 *
 *************************************************************************************************/
s32 XAsu_OcpValidateUserCertParams(const XAsu_OcpUserCertParams *OcpUserCertParam)
{
	s32 Status = XST_FAILURE;

	if (OcpUserCertParam == NULL) {
		goto END;
	}

	if ((OcpUserCertParam->UserCertCfgAddr == 0U) || (OcpUserCertParam->CertBufAddr == 0U) ||
	    (OcpUserCertParam->CertActualSizeAddr == 0U) || (OcpUserCertParam->CertBufLen == 0U)) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the user supplied configuration of a user X.509
 *		certificate.
 *
 * @param	UserCertCfg	Pointer to XAsu_X509UserCertCfg structure which holds the
 *				certificate fields, the key usage and the key identifiers.
 *
 * @return
 *	- XST_SUCCESS, if user certificate configuration is validated successfully.
 *	- XST_FAILURE, if user certificate configuration validation is failed.
 *
 *************************************************************************************************/
s32 XAsu_OcpValidateUserCertCfg(const XAsu_X509UserCertCfg *UserCertCfg)
{
	s32 Status = XST_FAILURE;
	const XAsu_X509CertFields *CertCfg = NULL;

	/** Validate the user certificate configuration pointer. */
	if (UserCertCfg == NULL) {
		goto END;
	}

	/** Get the certificate configuration. */
	CertCfg = &UserCertCfg->CertCfg;

	/** Validate the subject name lengths. */
	if ((CertCfg->Subject.OrgNameLen == 0U) ||
	    (CertCfg->Subject.OrgNameLen > XASU_X509_ORG_NAME_MAX_LEN) ||
	    (CertCfg->Subject.CommonNameLen == 0U) ||
	    (CertCfg->Subject.CommonNameLen > XASU_X509_COMMON_NAME_MAX_LEN)) {
		goto END;
	}

	/** Validate the year of the validity period. */
	if ((CertCfg->Validity.NotBefore.Year < XASU_X509_MIN_YEAR) ||
	    (CertCfg->Validity.NotBefore.Year > XASU_X509_MAX_YEAR) ||
	    (CertCfg->Validity.NotAfter.Year < XASU_X509_MIN_YEAR) ||
	    (CertCfg->Validity.NotAfter.Year > XASU_X509_MAX_YEAR)) {
		goto END;
	}

	/**
	 * Validate the ordering of the validity period, as a certificate whose validity period
	 * ends before it starts is expired the moment it is issued.
	 */
	if (XAsu_OcpGetX509TimeValue(&CertCfg->Validity.NotBefore) >
	    XAsu_OcpGetX509TimeValue(&CertCfg->Validity.NotAfter)) {
		goto END;
	}

	/** Validate the issuer name lengths. */
	if ((CertCfg->Issuer.OrgNameLen == 0U) ||
	    (CertCfg->Issuer.OrgNameLen > XASU_X509_ORG_NAME_MAX_LEN) ||
	    (CertCfg->Issuer.CommonNameLen == 0U) ||
	    (CertCfg->Issuer.CommonNameLen > XASU_X509_COMMON_NAME_MAX_LEN)) {
		goto END;
	}

	/** Validate the key usage. */
	if ((CertCfg->KeyUsage == 0U) ||
	    ((CertCfg->KeyUsage & ~((u32)XASU_X509_ECC_KEY_USAGE_MASK)) != 0U)) {
		goto END;
	}

	/** Validate the key identifiers of the subject public key and of the issuer key pair. */
	if ((UserCertCfg->SubjectPubKeyId == 0U) || (UserCertCfg->IssuerPubKeyId == 0U) ||
	    (UserCertCfg->IssuerPvtKeyId == 0U)) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function returns a packed date/time value (Year/Month/Day/Hour/Minute/Second)
 *		used for ordering comparisons (monotonic by fields).
 *
 * @param	X509Time	Pointer to XAsu_X509Time structure which holds the point in time.
 *
 * @return
 *	- The point in time as a value which increases monotonically with time.
 *
 *************************************************************************************************/
static u64 XAsu_OcpGetX509TimeValue(const XAsu_X509Time *X509Time)
{
	return (((u64)X509Time->Year << XASU_X509_TIME_YEAR_SHIFT) |
		((u64)X509Time->Month << XASU_X509_TIME_MONTH_SHIFT) |
		((u64)X509Time->Day << XASU_X509_TIME_DAY_SHIFT) |
		((u64)X509Time->Hour << XASU_X509_TIME_HOUR_SHIFT) |
		((u64)X509Time->Minute << XASU_X509_TIME_MINUTE_SHIFT) |
		(u64)X509Time->Second);
}
/** @} */
