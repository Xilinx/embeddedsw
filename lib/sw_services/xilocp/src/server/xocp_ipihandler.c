/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xocp_ipihandler.c
*
* This file contains the XilOcp IPI handlers implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.1   am   12/21/22 Initial release
*       am   01/10/23 Added handler API for dme
* 1.2   kpt  06/02/23 Updated XOcp_GetPcrLogIpi to XOcp_GetHwPcrLogIpi
*       kal  06/02/23 Added handler API for SW PCR
*       am   09/04/23 Cleared SharedSecretTmp array
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_config.h"
#ifdef PLM_OCP
#include "xocp.h"
#include "xocp_ipihandler.h"
#include "xocp_keymgmt.h"
#include "xocp_common.h"
#include "xocp_def.h"
#include "xocp_init.h"
#include "xplmi_dma.h"
#include "xplmi_hw.h"
#include "xsecure_defs.h"
#include "xsecure_ellipticplat.h"
#include "xsecure_elliptic.h"

/************************** Function Prototypes *****************************/
static int XOcp_ExtendHwPcrIpi(u32 PcrNum, u32 ExtHashAddrLow, u32 ExtHashAddrHigh, u32 Size);
static int XOcp_GetHwPcrIpi(u32 PcrMask, u32 PcrBuffAddrLow, u32 PcrBuffAddrHigh, u32 PurBufSize);
static int XOcp_GetHwPcrLogIpi(u32 HwPcrEvntAddrLow, u32 HwPcrEvntAddrHigh,
			u32 HwPcrLogInfoAddrLow, u32 HwPcrLogInfoAddrHigh,u32 NumOfLogEntries);
static int XOcp_GenDmeRespIpi(u32 NonceAddrLow, u32 NonceAddrHigh, u32 DmeStructResAddrLow, u32 DmeStructResAddrHigh);
static int XOcp_GetX509CertificateIpi(u32 GetX509CertAddrLow,
				u32 GetX509CertAddrHigh, u32 SubSystemID);
static int XOcp_AttestWithDevAkIpi(u32 AttestWithDevAkLow,
			u32 AttestWithDevAkHigh, u32 SubSystemID);
static int XOcp_SetSwPcrConfig(u32 *Pload, u32 Len);
static int XOcp_ExtendSwPcrIpi(u32 ExtParamsAddrLow, u32 ExtParamsAddrHigh);
static int XOcp_GetSwPcrIpi(u32 PcrMask, u32 PcrBuffAddrLow, u32 PcrBuffAddrHigh, u32 PcrBufSize);
static int XOcp_GetSwPcrLogIpi(u32 AddrLow, u32 AddrHigh);
static int XOcp_GetSwPcrDataIpi(u32 AddrLow, u32 AddrHigh);
#ifndef PLM_ECDSA_EXCLUDE
static int XOcp_GenSharedSecretwithDevAkIpi(u32 SubSystemId, u32 PubKeyAddrLow, u32 PubKeyAddrHigh,
	u32 SharedSecretAddrLow, u32 SharedSecretAddrHigh);
#endif
/*************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief   This function calls respective IPI handler based on the API_ID.
 *
 * @param   Cmd - Pointer to command structure
 *
 * @return
 *          - XST_SUCCESS - If the handler execution is successful
 *	    - ErrorCode - If there is a failure
 *
 ******************************************************************************/
int XOcp_IpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	u32 *Pload = Cmd->Payload;

	if (Pload == NULL) {
		goto END;
	}

	switch (Cmd->CmdId & XOCP_API_ID_MASK) {
		case XOCP_API(XOCP_API_EXTEND_HWPCR):
			Status = XOcp_ExtendHwPcrIpi(Pload[0], Pload[1], Pload[2], Pload[3]);
			break;
		case XOCP_API(XOCP_API_GET_HWPCR):
			Status = XOcp_GetHwPcrIpi(Pload[0], Pload[1], Pload[2], Pload[3]);
			break;
		case XOCP_API(XOCP_API_GET_HWPCRLOG):
			Status = XOcp_GetHwPcrLogIpi(Pload[0], Pload[1], Pload[2], Pload[3], Pload[4]);
			break;
		case XOCP_API(XOCP_API_GENDMERESP):
			Status = XOcp_GenDmeRespIpi(Pload[0], Pload[1], Pload[2], Pload[3]);
			break;
		case XOCP_API(XOCP_API_GETX509CERT):
			Status = XOcp_GetX509CertificateIpi(Pload[0], Pload[1],
					Cmd->SubsystemId);
			break;
		case XOCP_API(XOCP_API_ATTESTWITHDEVAK):
			Status = XOcp_AttestWithDevAkIpi(Pload[0], Pload[1],
					Cmd->SubsystemId);
			break;
		case XOCP_API(XOCP_API_SET_SWPCRCONFIG):
			Status = XOcp_SetSwPcrConfig(Cmd->Payload, Cmd->Len);
			break;
		case XOCP_API(XOCP_API_EXTEND_SWPCR):
			Status = XOcp_ExtendSwPcrIpi(Pload[0], Pload[1]);
			break;
		case XOCP_API(XOCP_API_GET_SWPCR):
			Status = XOcp_GetSwPcrIpi(Pload[0], Pload[1], Pload[2], Pload[3]);
			break;
		case XOCP_API(XOCP_API_GET_SWPCRLOG):
			Status = XOcp_GetSwPcrLogIpi(Pload[0], Pload[1]);
			break;
		case XOCP_API(XOCP_API_GET_SWPCRDATA):
			Status = XOcp_GetSwPcrDataIpi(Pload[0], Pload[1]);
			break;
#ifndef PLM_ECDSA_EXCLUDE
		case XOCP_API(XOCP_API_GEN_SHARED_SECRET):
			Status = XOcp_GenSharedSecretwithDevAkIpi(Cmd->SubsystemId, Pload[0], Pload[1],
				Pload[2], Pload[3]);
			break;
#endif
		default:
			XOcp_Printf(DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
			Status = XST_INVALID_PARAM;
			break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function handler calls XOcp_ExtendHwPcr server API to extend
 *          PCR with provided hash by requesting ROM service.
 *
 * @param   ExtHashAddrLow - Lower 32 bit address of the ExtendedHash
 *          buffer address
 * @param   ExtHashAddrHigh - Higher 32 bit address of the ExtendedHash
 *          buffer address
 * @param   PcrNum - Variable of enum XOcp_RomPcr to select the PCR to be
 *          extended
 *
 * @return
 *          - XST_SUCCESS - If PCR extend is success
 *          - ErrorCode - Upon any failure
 *
 ******************************************************************************/
static int XOcp_ExtendHwPcrIpi(u32 PcrNum, u32 ExtHashAddrLow, u32 ExtHashAddrHigh, u32 Size)
{
	volatile int Status = XST_FAILURE;
	u64 ExtendedHashAddr = ((u64)ExtHashAddrHigh << 32U) | (u64)ExtHashAddrLow;

	Status = XOcp_ExtendHwPcr(PcrNum, (u64)(UINTPTR)ExtendedHashAddr, Size);

	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function handler calls XOcp_GetHwPcr server API to get
 *          the PCR value from requested PCR.
 *
 * @param   PcrMask - Mask to tell what PCRs to read
 * @param   PcrBuffAddrLow - Lower 32 bit address of the PCR buffer address
 * @param   PcrBuffAddrHigh - Higher 32 bit address of the PCR buffer address
 * @param   PcrBufSize - Size of the Pcr Buffer
 *
 * @return
 *          - XST_SUCCESS - If PCR contents are copied
 *          - ErrorCode - Upon any failure
 *
 ******************************************************************************/
static int XOcp_GetHwPcrIpi(u32 PcrMask, u32 PcrBuffAddrLow, u32 PcrBuffAddrHigh, u32 PcrBufSize)
{
	volatile int Status = XST_FAILURE;
	u64 PcrBuffAddr = ((u64)PcrBuffAddrHigh << 32U) | (u64)PcrBuffAddrLow;

	Status = XOcp_GetHwPcr(PcrMask, PcrBuffAddr, PcrBufSize);

	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function handler calls XOcp_GetHwPcrLog server API to get log
 *
 * @param   HwPcrEvntAddrLow     - Lower 32 bit address of the XOcp_HwPcrEvent buffer address
 * @param   HwPcrEvntAddrHigh    - Higher 32 bit address of the XOcp_HwPcrEvent buffer address
 * @param   HwPcrLogInfoAddrLow  - Lower 32 bit address of the XOcp_HwPcrLogInfo buffer address
 * @param   HwPcrLogInfoAddrHigh - Higher 32 bit address of the XOcp_HwPcrLogInfo buffer address
 * @param   NumOfLogEntries      - Number of log entries to read
 *
 * @return
 *          - XST_SUCCESS - If log contents are copied
 *          - ErrorCode - Upon any failure
 *
 ******************************************************************************/
static int XOcp_GetHwPcrLogIpi(u32 HwPcrEvntAddrLow, u32 HwPcrEvntAddrHigh,
			u32 HwPcrLogInfoAddrLow, u32 HwPcrLogInfoAddrHigh,u32 NumOfLogEntries)
{
	volatile int Status = XST_FAILURE;
	u64 HwPcrEventAddr = ((u64)HwPcrEvntAddrHigh << 32U) | (u64)HwPcrEvntAddrLow;
	u64 HwPcrLogInfoAddr = ((u64)HwPcrLogInfoAddrHigh << 32U) | (u64)HwPcrLogInfoAddrLow;

	Status = XOcp_GetHwPcrLog(HwPcrEventAddr, HwPcrLogInfoAddr, NumOfLogEntries);

	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function handler calls XOcp_GenerateDmeResponse server API to
 *          generate response with the DME signature
 *
 * @param   NonceAddrLow - Lower 32 bit address of the nonce buffer address
 * @param   NonceAddrHigh - Higher 32 bit address of the nonce buffer address
 * @param   DmeStructResAddrLow - Lower 32 bit address of the
 *          XOcp_DmeResponseAddr structure
 * @param   DmeStructResAddrHigh - Higher 32 bit address of the
 *          XOcp_DmeResponseAddr structure
 *
 * @return
 *          - XST_SUCCESS - Upon success
 *          - ErrorCode - Upon any failure
 *
 ******************************************************************************/
static int XOcp_GenDmeRespIpi(u32 NonceAddrLow, u32 NonceAddrHigh,
	u32 DmeStructResAddrLow, u32 DmeStructResAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 NonceBufAddr = ((u64)NonceAddrHigh << 32U) | (u64)NonceAddrLow;
	u64 DmeStructResAddr = ((u64)DmeStructResAddrHigh << 32U) | (u64)DmeStructResAddrLow;

	Status = XOcp_GenerateDmeResponse(NonceBufAddr, DmeStructResAddr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function handler calls XOcp_GetX509Certificate server API to
 *          generate the 509 certificate.
 *
 * @param   GetX509CertAddrLow - Lower 32 bit address of XOcp_GetX509Cert
 * @param   GetX509CertAddrHigh - Higher 32 bit address of XOcp_GetX509Cert
 *
 * @return
 *          - XST_SUCCESS - Upon success
 *          - ErrorCode - Upon any failure
 ******************************************************************************/
static int XOcp_GetX509CertificateIpi(u32 GetX509CertAddrLow,
				u32 GetX509CertAddrHigh, u32 SubSystemID)
{
	volatile int Status = XST_FAILURE;
	u64 GetX509CertAddr = ((u64)GetX509CertAddrHigh << 32U) |
				(u64)GetX509CertAddrLow;
	XOcp_X509Cert X509Cert __attribute__ ((aligned (32U)));

	XPlmi_MemCpy64((u64)(UINTPTR)&X509Cert, GetX509CertAddr,
				sizeof(XOcp_X509Cert));

	Status = XOcp_GetX509Certificate(&X509Cert, SubSystemID);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function handler calls XOcp_GenerateDmeResponse server API to
 *          generate response with the DME signature
 *
 * @param   AttestWithDevAkLow - Lower 32 bit address of
 *			XOcp_AttestWithDevAk strucure
 * @param   AttestWithDevAkHigh - Higher 32 bit address of
 *			XOcp_AttestWithDevAk strucure
 *
 * @return
 *          - XST_SUCCESS - Upon success
 *          - ErrorCode - Upon any failure
 ******************************************************************************/
static int XOcp_AttestWithDevAkIpi(u32 AttestWithDevAkLow,
			u32 AttestWithDevAkHigh, u32 SubSystemID)
{
	volatile int Status = XST_FAILURE;
	u64 AttestWithDevAkAddr = ((u64)AttestWithDevAkHigh << 32U) |
			(u64)AttestWithDevAkLow;
	XOcp_Attest AttestInstance __attribute__ ((aligned (32U)));

	XPlmi_MemCpy64((u64)(UINTPTR)&AttestInstance, AttestWithDevAkAddr,
				sizeof(XOcp_Attest));

	Status = XOcp_AttestWithDevAk(&AttestInstance, SubSystemID);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function handler calls the XOcp_StoreSwPcrConfig server
 * 	    API to store SwPcr configuration sent over IPI.
 *
 * @param   Pload - Pointer to command payload
 * @param   Len   - Length of the payload
 *
 * @return
 *          - XST_SUCCESS - Upon success
 *          - ErrorCode - Upon any failure
 ******************************************************************************/
static int XOcp_SetSwPcrConfig(u32 *Pload, u32 Len)
{
	volatile int Status = XST_FAILURE;

	Status = XOcp_StoreSwPcrConfig(Pload, Len);

	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function handler calls XOcp_ExtendSwPcr to extend data to
 * 	    specified SWPCR
 *
 * @param   ExtParamsAddrLow - Lower 32 bit address of the XOcp_SwPcrExtendParams
 * @param   ExtParamsAddrHigh - Higher 32 bit address of the XOcp_SwPcrExtendParams
 *
 * @return
 *          - XST_SUCCESS - Upon success
 *          - ErrorCode - Upon any failure
 ******************************************************************************/
static int XOcp_ExtendSwPcrIpi(u32 ExtParamsAddrLow, u32 ExtParamsAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)ExtParamsAddrHigh << 32U) | (u64)ExtParamsAddrLow;
	XOcp_SwPcrExtendParams ExtendParams;

	Status = XPlmi_MemCpy64((u64)(UINTPTR)&ExtendParams, Addr, sizeof(ExtendParams));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XOcp_ExtendSwPcr(ExtendParams.PcrNum, ExtendParams.MeasurementIdx,
		ExtendParams.DataAddr, ExtendParams.DataSize, ExtendParams.OverWrite);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function handler calls XOcp_GetSwPcr server API to get
 * 	    specified SW PCR value
 *
 * @param   PcrMask - Mask to tell what PCRs to read
 * @param   PcrBuffAddrLow - Lower 32 bit address of the PCR buffer address
 * @param   PcrBuffAddrHigh - Higher 32 bit address of the PCR buffer address
 * @param   PcrBufSize - Size of the PCR Buffer
 *
 * @return
 *          - XST_SUCCESS - Upon success
 *          - ErrorCode - Upon any failure
 ******************************************************************************/
static int XOcp_GetSwPcrIpi(u32 PcrMask, u32 PcrBuffAddrLow,
	u32 PcrBuffAddrHigh, u32 PcrBufSize)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)PcrBuffAddrHigh << 32U) | (u64)PcrBuffAddrLow;

	Status = XOcp_GetSwPcr(PcrMask, Addr, PcrBufSize);

	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function handler calls XOcp_GetSwPcrLog server API for
 * 		SWPCR extend.
 *
 * @param   AddrLow - Lower 32 bit address of the XOcp_SwPcrLogReadData
 * @param   AddrHigh - Higher 32 bit address of the XOcp_SwPcrLogReadData
 *
 * @return
 *          - XST_SUCCESS - Upon success
 *          - ErrorCode - Upon any failure
 ******************************************************************************/
static int XOcp_GetSwPcrLogIpi(u32 AddrLow, u32 AddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)AddrHigh << 32U) | (u64)AddrLow;

	Status = XOcp_GetSwPcrLog(Addr);

	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function handler calls XOcp_GetSwPcrData server API to
 * 		read the specified SWPCR data.
 *
 * @param   AddrLow - Lower 32 bit address of the XOcp_SwPcrReadData
 * @param   AddrHigh - Higher 32 bit address of the XOcp_SwPcrReadData
 *
 * @return
 *          - XST_SUCCESS - Upon success
 *          - ErrorCode - Upon any failure
 ******************************************************************************/
static int XOcp_GetSwPcrDataIpi(u32 AddrLow, u32 AddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)AddrHigh << 32U) | (u64)AddrLow;

	Status = XOcp_GetSwPcrData(Addr);

	return Status;
}

#ifndef PLM_ECDSA_EXCLUDE
/*****************************************************************************/
/**
 * @brief	This function handler calls XSecure_EcdhGetSecret server API to
 * 		generate the shared secret using ECDH.
 *
 * @param	SubSystemId - ID of the subsystem from where the command is
 * 			originating
 * @param	PubKeyAddrLow - Lower 32 bit address of the Public Key buffer
 * @param	PubKeyAddrHigh - Upper 32 bit address of the Public Key buffer
 * @param	SharedSecretAddrLow - Lower 32 bit address of the Shared Secret buffer
 * @param	SharedSecretAddrHigh - Upper 32 bit address of the Shared Secret buffer
 *
 * @return
 *		 - XST_SUCCESS  On Success
 *		 - Errorcode  On failure

 ******************************************************************************/
static int XOcp_GenSharedSecretwithDevAkIpi(u32 SubSystemId, u32 PubKeyAddrLow, u32 PubKeyAddrHigh,
	u32 SharedSecretAddrLow, u32 SharedSecretAddrHigh)
{
	volatile int Status = XST_FAILURE;
	volatile int ClrStatus = XST_FAILURE;
	u64 PubKeyAddr = ((u64)PubKeyAddrHigh << XOCP_ADDR_HIGH_SHIFT) | (u64)PubKeyAddrLow;
	u64 SharedSecretAddr = ((u64)SharedSecretAddrHigh << XOCP_ADDR_HIGH_SHIFT) | (u64)SharedSecretAddrLow;
	XOcp_DevAkData *DevAkData = XOcp_GetDevAkData();
	u32 DevAkIndex = XOCP_INVALID_DEVAK_INDEX;
	u64 PrvtKeyAddr;
	u8 PubKeyTmp[XSECURE_ECC_P384_SIZE_IN_BYTES * 2U];
	u8 SharedSecretTmp[XSECURE_ECC_P384_SIZE_IN_BYTES * 2U];

	DevAkIndex = XOcp_GetSubSysReqDevAkIndex(SubSystemId);
	if (DevAkIndex == XOCP_INVALID_DEVAK_INDEX) {
		Status = XOCP_ERR_INVALID_DEVAK_REQ;
		goto END;
	}

	DevAkData = DevAkData + DevAkIndex;

	if (DevAkData->IsDevAkKeyReady == TRUE) {
		PrvtKeyAddr = (u64)(UINTPTR)DevAkData->EccPrvtKey;

		XSecure_FixEndiannessNCopy(XSECURE_ECC_P384_SIZE_IN_BYTES,
			(u64)(UINTPTR)PubKeyTmp, PubKeyAddr);
		XSecure_FixEndiannessNCopy(XSECURE_ECC_P384_SIZE_IN_BYTES,
			(u64)(UINTPTR)(PubKeyTmp + XSECURE_ECC_P384_SIZE_IN_BYTES),
			PubKeyAddr + XSECURE_ECC_P384_SIZE_IN_BYTES);

		Status = XSecure_EcdhGetSecret(XSECURE_ECC_NIST_P384, PrvtKeyAddr,
			(u64)(UINTPTR)PubKeyTmp, (u64)(UINTPTR)SharedSecretTmp);

		XSecure_FixEndiannessNCopy(XSECURE_ECC_P384_SIZE_IN_BYTES,
			SharedSecretAddr, (u64)(UINTPTR)SharedSecretTmp);
		XSecure_FixEndiannessNCopy(XSECURE_ECC_P384_SIZE_IN_BYTES,
			SharedSecretAddr + XSECURE_ECC_P384_SIZE_IN_BYTES,
			(u64)(UINTPTR)(SharedSecretTmp + XSECURE_ECC_P384_SIZE_IN_BYTES));
	}
	else {
		Status = XOCP_ERR_DEVAK_NOT_READY;
	}

END:
	ClrStatus = Xil_SecureZeroize(SharedSecretTmp, XSECURE_ECC_P384_SIZE_IN_BYTES * 2U);
	if (ClrStatus != XST_SUCCESS) {
		Status |= ClrStatus;
	}

	return Status;
}
#endif

#endif /* PLM_OCP */
