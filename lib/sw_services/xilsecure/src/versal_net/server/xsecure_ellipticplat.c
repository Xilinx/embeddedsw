/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_ellipticplat.c
*
* This file contains the implementation of the interface functions for ECC
* engine specific to versal net platform.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.0   dc   07/10/22 Initial release
*       am   07/23/22 Removed Ecdsa_ModEccOrder function defination, as it is
*                     declared in Ecdsa.h file
*       dc   09/04/22 set TRNG to HRNG mode after Private key ECC mod order
*
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_ellipticplat.h"
#include "xsecure_plat_kat.h"
#include "xsecure_error.h"
#include "xsecure_elliptic.h"
#include "xsecure_utils.h"
#include "xsecure_ecdsa_rsa_hw.h"
#include "xsecure_error.h"
#include "xil_util.h"

/************************** Constant Definitions *****************************/
#define XSECURE_ECC_TRNG_DF_LENGTH			(2U)
#define XSECURE_ECC_TRNG_RANDOM_NUM_GEN_LEN		(60U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/************************** Function Prototypes ******************************/
extern EcdsaCrvInfo* XSecure_EllipticGetCrvData(XSecure_EllipticCrvTyp CrvTyp);

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
 * This function generates 48-byte key for P-384 curve using TRNG.
 *
 * @param	CrvType specifies the type of the ECC curve.
 * @param	PrivateKeyis the pointer to XSecure_ElliptcPrivateKeyGen
 *
 * @return	XST_SUCCESS on success
 *		Error code otherwise
 *
 *****************************************************************************/
int XSecure_EllipticPrvtKeyGenerate(XSecure_EllipticCrvTyp CrvType,
	XSecure_ElliptcPrivateKeyGen *PrivateKey)
{
	int Status = XSECURE_ECC_PRVT_KEY_GEN_ERR;
	int ClearStatus = XST_FAILURE;
	XSecure_TrngInstance *TrngInstancePtr = XSecure_GetTrngInstance();
	XSecure_TrngUserConfig TrngUserCfg;
	/* The random ephimeral/Private key should be between 1 to ecc order
	 * of the curve. EK can be obtained by using mod operation over
	 * 384-bits random number. However to reduce bias associated with mod
	 * operation, standard recommends of using additional 64-bits i.e.
	 * 384 + 64 = 448-bits. IPCores library has option to perform 448-bit
	 * mod operation however it treats MSB as sign bit. So to overcome the
	 * issue, new function added to IPCore library uses next available
	 * large number operation that hardware can handle is selected,
	 * 480-bits (60 bytes).
	 * Below code takes 448 bits from random number generated remaining
	 * MSB bits are set to 0x00.
	 */
	u8 RandBuf[XSECURE_ECC_TRNG_RANDOM_NUM_GEN_LEN] = {0x00};
	EcdsaCrvInfo *Crv = NULL;

	if ((CrvType == XSECURE_ECC_NIST_P521) || (PrivateKey == NULL)) {
		Status = XSECURE_ELLIPTIC_INVALID_PARAM;
		goto RET;
	}

	Status = XSecure_TrngPreOperationalSelfTests(TrngInstancePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xil_SMemSet(&TrngUserCfg, sizeof(XSecure_TrngUserConfig), 0U,
					sizeof(XSecure_TrngUserConfig));
	if (Status != XST_SUCCESS) {
		goto END;
	}
	TrngUserCfg.Mode = XSECURE_TRNG_DRNG_MODE;
	TrngUserCfg.DFLength = XSECURE_ECC_TRNG_DF_LENGTH;
	TrngUserCfg.AdaptPropTestCutoff = XSECURE_TRNG_USER_CFG_ADAPT_TEST_CUTOFF;
	TrngUserCfg.RepCountTestCutoff = XSECURE_TRNG_USER_CFG_REP_TEST_CUTOFF;
	TrngUserCfg.SeedLife = XSECURE_TRNG_DEFAULT_SEED_LIFE;
	Status = XSecure_TrngInstantiate(TrngInstancePtr,
			(u8 *)(UINTPTR)PrivateKey->SeedAddr, PrivateKey->SeedLength,
			(u8 *)(UINTPTR)PrivateKey->PerStringAddr, &TrngUserCfg);
	if (Status  != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_TrngGenerate(TrngInstancePtr, RandBuf,
			XSECURE_TRNG_SEC_STRENGTH_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XSecure_TrngGenerate(TrngInstancePtr,
			RandBuf + XSECURE_TRNG_SEC_STRENGTH_IN_BYTES,
			XSECURE_ECC_TRNG_RANDOM_NUM_GEN_LEN -
			XSECURE_TRNG_SEC_STRENGTH_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/* Take ECDSA core out if reset */
	XSecure_ReleaseReset(XSECURE_ECDSA_RSA_BASEADDR,
			XSECURE_ECDSA_RSA_RESET_OFFSET);

	Crv = XSecure_EllipticGetCrvData(XSECURE_ECC_NIST_P384);
	if (Crv == NULL) {
		Status = XST_FAILURE;
		goto END;
	}

	/* IPCores library expects MSB bit to be 0 always */
	Status = Ecdsa_ModEccOrder(Crv, RandBuf,
			(u8 *)(UINTPTR)PrivateKey->KeyOutPutAddr);

END:
	ClearStatus = XSecure_TrngUninstantiate(TrngInstancePtr);
	if (Status == XST_SUCCESS) {
		Status = ClearStatus;
	}

	XSecure_SetReset(XSECURE_ECDSA_RSA_BASEADDR,
				XSECURE_ECDSA_RSA_RESET_OFFSET);
	ClearStatus = Xil_SMemSet((void *)RandBuf,
				 XSECURE_ECC_TRNG_RANDOM_NUM_GEN_LEN,
				0U, XSECURE_ECC_TRNG_RANDOM_NUM_GEN_LEN);
	if (Status == XST_SUCCESS) {
		Status = ClearStatus;
		Status |= XSecure_TrngSetHrngMode();
	}
RET:
	return Status;
}