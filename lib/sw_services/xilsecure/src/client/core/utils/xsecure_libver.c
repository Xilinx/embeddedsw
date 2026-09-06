/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file client/core/utils/xsecure_libver.c
*
* This file contains the client-side implementation of the XilSecure library version
* compatibility check. It queries the server (PLM) library version over the IPI mailbox and
* compares it against the client's compile-time version.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ---------------------------------------------------------------------------
* 5.8   tus     08/11/26 Initial Release
*
* </pre>
*
***************************************************************************************************/

/****************************************** Include Files *****************************************/
#include "xil_types.h"
#include "xil_util.h"
#include "xstatus.h"
#include "xsecure_libver.h"
#include "xsecure_ver.h"
#include "xsecure_mailbox.h"
#include "xsecure_generic.h"
#include "xsecure_defs.h"

/************************************** Constant Definitions **************************************/

/**************************************** Type Definitions ****************************************/

/****************************** Macros (Inline Functions) Definitions *****************************/

/*************************************** Function Prototypes **************************************/

/************************************** Variable Definitions **************************************/

/************************************** Function Definitions **************************************/

/**************************************************************************************************/
/**
*
* @brief	Returns the client XilSecure library version, frozen at compile time.
*
* @param	Version		Pointer to a 32-bit word that receives the client library
*				version packed as XIL_BUILD_VERSION(Major, Minor).
*
* @return
*		- XST_SUCCESS - On success
*		- XST_INVALID_PARAM - If Version pointer is NULL
*
***************************************************************************************************/
int XSecure_GetClientLibVersion(u32 *Version)
{
	int Status = (int)XST_INVALID_PARAM;

	if (Version != NULL) {
		*Version = XIL_BUILD_VERSION(XSECURE_MAJOR_VERSION, XSECURE_MINOR_VERSION);
		Status = (int)XST_SUCCESS;
	}

	return Status;
}

/**************************************************************************************************/
/**
*
* @brief	Queries the XilSecure server (PLM) library information - version and capability
*		bitmaps - by issuing a Features command with an always-present crypto api-id.
*
* @param	InstancePtr	Pointer to the client instance (mailbox handle + SlrIndex).
* @param	LibInfo		Pointer to a Xil_LibInfo structure that receives the server
*				library version (LibVersion) and capability bitmap words.
*
* @return
*		- XST_SUCCESS - On success
*		- XST_INVALID_PARAM - If InstancePtr or LibInfo is NULL
*		- Error - On SMC/mailbox transport failure or if the Features command fails
*
* @note	The Features command is sent with a valid crypto api-id so the server
*		command SUCCEEDS and returns the full response buffer, in which the library
*		version/capability are reported at fixed indices ([3]/[4]). A server too old to
*		report capabilities leaves those words at 0, which the caller reads as "no
*		capability advertised" - the safe default.
*
***************************************************************************************************/
int XSecure_GetServerLibInfo(XSecure_ClientInstance *InstancePtr, Xil_LibInfo *LibInfo)
{
	volatile int Status = (int)XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};
	u32 Response[RESPONSE_ARG_CNT] = {0U};

	/* Validate input parameters */
	if ((InstancePtr == NULL) || (LibInfo == NULL)) {
		Status = (int)XST_INVALID_PARAM;
		goto END;
	}

	/* Query library info via the Features command using a valid crypto api-id
	 * (SHA3) as the selector, so the command succeeds and the server returns the
	 * library version/capability at fixed indices. SlrIndex is folded into the
	 * api-id to reach the correct SLR on SSIT parts. */
#if defined(VERSAL_2VE_2VM)
	XSECURE_PACK_PAYLOAD1(Payload, ((InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT) |
				(u32)XSECURE_API_FEATURES), (u32)XSECURE_API_SHA3_OPERATION);
#else
	XSECURE_PACK_PAYLOAD1(Payload, ((InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT) |
				(u32)XSECURE_API_FEATURES), (u32)XSECURE_API_SHA3_UPDATE);
#endif

	/* Send the request over the generic transport: SMC for EL1 non-secure
	 * AArch64 applications, IPI mailbox otherwise. XSecure_SendRequest returns
	 * the server command status (so a Features failure is not mistaken for
	 * success) and strips the IPI status word, so the data starts at Response[0]. */
	Status = XSecure_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, Response,
				(u32)RESPONSE_ARG_CNT);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/*
	 * The status word is stripped by XSecure_SendRequest, so the data starts at
	 * Response[0] and matches Xil_LibInfo. Copy it in one shot; the server does
	 * not emit Capability[1], so zero it afterwards. For xilsecure all fields
	 * (including Version/NistStatus) are meaningful.
	 */
	Status = Xil_SMemCpy(LibInfo, sizeof(Xil_LibInfo), &Response[0U],
			(u32)(XIL_LIBINFO_RESP_WORDS * sizeof(u32)),
			(u32)(XIL_LIBINFO_RESP_WORDS * sizeof(u32)));
	if (Status != XST_SUCCESS) {
		goto END;
	}
	LibInfo->Capability[1U] = 0U;

END:
	return Status;
}

/**************************************************************************************************/
/**
*
* @brief	Queries the XilSecure server (PLM) library version.
*
* @param	InstancePtr	Pointer to the client instance (mailbox handle + SlrIndex).
* @param	Version		Pointer to a 32-bit word that receives the server library version
*				packed as XIL_BUILD_VERSION(Major, Minor).
*
* @return
*		- XST_SUCCESS - On success
*		- XST_INVALID_PARAM - If InstancePtr or Version pointer is NULL
*		- Error - On SMC/mailbox transport failure
*
***************************************************************************************************/
int XSecure_GetServerLibVersion(XSecure_ClientInstance *InstancePtr, u32 *Version)
{
	int Status = XST_FAILURE;
	Xil_LibInfo LibInfo = {0U};

	/* Validate input parameter */
	if (Version == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/* The version travels in the same response as the capabilities */
	Status = XSecure_GetServerLibInfo(InstancePtr, &LibInfo);
	if (Status == XST_SUCCESS) {
		*Version = LibInfo.LibVersion;
	}

END:
	return Status;
}

/**************************************************************************************************/
/**
*
* @brief	Verifies that the client XilSecure library is compatible with the running server
*		(PLM) XilSecure library.
*
*		The client compile-time version (XSecure_GetLibVersion()) is compared against the
*		version reported by the server. Compatibility requires the major versions to match
*		and the client minor version to be less than or equal to the server minor version
*		(a newer server stays backward compatible with an older client).
*
* @param	InstancePtr	Pointer to the client instance (mailbox handle + SlrIndex).
*
* @return
*		- XST_SUCCESS - Client and server library versions are compatible
*		- XSECURE_LIB_VERSION_MAJOR_MISMATCH - Major versions differ (breaking change)
*		- XSECURE_LIB_VERSION_MINOR_MISMATCH - Client minor is newer than the server
*		- XST_INVALID_PARAM / Error - On transport failure while querying the server
*
***************************************************************************************************/
int XSecure_VerifyLibCompatibility(XSecure_ClientInstance *InstancePtr)
{
	int Status = XST_FAILURE;
	u32 ServerVersion = 0U;
	u32 ClientVersion;
	u32 ClientMajor;
	u32 ClientMinor;
	u32 ServerMajor;
	u32 ServerMinor;

	/* Fetch the server library version over the selected transport */
	Status = XSecure_GetServerLibVersion(InstancePtr, &ServerVersion);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Client version is frozen at compile time */
	ClientVersion = XSecure_GetLibVersion();

	ClientMajor = XSECURE_GET_MAJOR_VERSION(ClientVersion);
	ClientMinor = XSECURE_GET_MINOR_VERSION(ClientVersion);
	ServerMajor = XSECURE_GET_MAJOR_VERSION(ServerVersion);
	ServerMinor = XSECURE_GET_MINOR_VERSION(ServerVersion);

	if (ClientMajor != ServerMajor) {
		/* A major bump signals a breaking change */
		Status = XSECURE_LIB_VERSION_MAJOR_MISMATCH;
	}
	else if (ClientMinor > ServerMinor) {
		/* Client is newer than the server and may expect features an older
		 * server cannot provide. A client older than the server is acceptable
		 * (the server stays backward compatible within a major version). */
		Status = XSECURE_LIB_VERSION_MINOR_MISMATCH;
	}
	else {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/**************************************************************************************************/
/**
*
* @brief	Tests whether a given capability bit is advertised in a library capability word
*		previously fetched with XSecure_GetServerLibInfo().
*
* @param	LibInfo		Pointer to a populated Xil_LibInfo structure.
* @param	CapBit		Capability bit position (one of the XSECURE_CAP_*_BIT values).
*
* @return
*		- TRUE  - The capability is compiled into the running server
*		- FALSE - The capability is absent, or LibInfo is NULL, or CapBit is out of range
*
***************************************************************************************************/
u32 XSecure_IsCapabilitySupported(const Xil_LibInfo *LibInfo, u32 CapBit)
{
	u32 Supported = (u32)FALSE;
	u32 Word = CapBit / XSECURE_LIB_CAP_BITS_PER_WORD;	/* which capability word */
	u32 Bit = CapBit % XSECURE_LIB_CAP_BITS_PER_WORD;	/* which bit within that word */

	if ((LibInfo != NULL) && (Word < XSECURE_LIB_CAP_WORDS)) {
		if ((LibInfo->Capability[Word] & ((u32)1U << Bit)) != 0U) {
			Supported = (u32)TRUE;
		}
	}

	return Supported;
}
