/******************************************************************************
* Copyright (C) 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
 * @file xilpki_ecdsa.c
 * @addtogroup xilpki Overview
 * This file contains the implementation of the interface functions for the
 * PKI ECDSA hardware module.
 *
 * @{
 * @details
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date      Changes
 * ----- ----  --------  ------------------------------------------------------
 * 1.0   Nava  12/05/22  Initial Release
 *</pre>
 *
 *@note
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xilpki.h"

/************************** Constant Definitions *****************************/
/* Request queue size in bytes */
#define XPKI_MAX_RQ_CFG_BUF_SIZE	0x10000U

/*Completion queue size in bytes */
#define XPKI_MAX_CQ_CFG_BUF_SIZE	0x10000U

#define XPKI_DESC_LEN_BYTES		0x20U
#define XPKI_SIGN_P521_PADD_BYTES	0x2U
#define XPKI_VERIFY_P521_PADD_BYTES	0x6U
#define XPKI_ECDSA_NEW_REQUEST_MASK	0x00000FFFU
#define XPKI_DONE_STATUS_VAL		0x1U
#define XPKI_EXP_CQ_STATUS		0x0U
#define XPKI_EXP_CQ_VAL			0xB04E0001
#define XPKI_DONE_POLL_COUNT		10000U

#define XPKI_RQ_CFG_PERMISSIONS_SAFE	0x0U
#define XPKI_RQ_CFG_PAGE_SIZE_1024	0x10U
#define XPKI_RQ_CFG_CQID		0x0U
#define	XPKI_CQ_CFG_SIZE_4096		0xCU
#define XPKI_CQ_CFG_IRQ_ID_VAL		0x0U
#define XPKI_RQ_CFG_QUEUE_DEPTH_VAL	0x80U
#define XPKI_IRQ_ENABLE_VAL		0xFFFFU
#define XPKI_CQ_CTL_TRIGPOS_VAL		0x201U

/************************** Function Prototypes ******************************/
static int XPki_LoadInputData_EcdsaGenerateSign(XPki_Instance *InstancePtr,
						XPki_EcdsaSignInputData *SignParams,
						u64 *EcdsaReqVal);
static int XPki_LoadInputData_EcdsaVerifySign(XPki_Instance *InstancePtr,
					      XPki_EcdsaVerifyInputData *VerifyParams,
					      u64 *EcdsaReqVal);
static int XPki_LoadInputData_EcdsaPointMulti(XPki_Instance *InstancePtr,
					      XPki_EcdsaPointMultiInputData *PointMultiParams,
					      u64 *EcdsaReqVal);
static int XPki_LoadInputData_ModulusAdd(XPki_Instance *InstancePtr,
					 XPki_ModAddInputData *ModAddParams,
					 u64 *EcdsaReqVal);
static int XPki_Wait_For_Int_Done(u64 ExpVal);
static void XPki_Init_Ecdsa(XPki_Instance *InstancePtr, u64 EcdsaReqVal);
static int XPki_Check_Output_CompletionQueue(UINTPTR CQAddr);
static int XPki_ValidateEcdsaGenerateSignParam(XPki_Instance *InstancePtr,
					       XPki_EcdsaSignInputData *SignParams,
					       XPki_EcdsaSign *Sign);
static int XPki_ValidateEcdsaVerifySignParam(XPki_Instance *InstancePtr,
					     XPki_EcdsaVerifyInputData *VerifyParams);
static int XPki_ValidateEcdsaPointMultiParam(XPki_Instance *InstancePtr,
					     XPki_EcdsaPointMultiInputData *PointMultiParams,
					     XPki_EcdsaGpoint *Gpoint);
static XPki_EcdsaGpoint* XPki_GetEccGpoint(XPki_EcdsaCrvType CrvType);
static u8 XPki_GetEccCurveLen(XPki_EcdsaCrvType CrvType);
static int XPki_GetEccPrivateKey(XPki_Instance *InstancePtr, XPki_EcdsaCrvType CrvType, u8 *Priv);
static int XPki_GetModulusAdd(XPki_Instance *InstancePtr, XPki_ModAddInputData *ModAddParams, u8 *OutBuff);
static const u8* XPki_GetEccCurveOrder(XPki_EcdsaCrvType CrvType);
static int XPki_ValidateModulusAddParam(XPki_Instance *InstancePtr,
					XPki_ModAddInputData *ModAddParams,
					u8 *OutBuff);

/************************** Variable Definitions *****************************/
static const u8 Order_P256[] = { 0x51, 0x25, 0x63, 0xfc, 0xc2, 0xca, 0xb9,
				 0xf3, 0x84, 0x9e, 0x17, 0xa7, 0xad, 0xfa,
				 0xe6, 0xbc, 0xff, 0xff, 0xff, 0xff, 0xff,
				 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
				 0xff, 0xff, 0xff, 0xff, 0x00
			       };


static const u8 Order_P384[] = { 0x73, 0x29, 0xc5, 0xcc, 0x6a, 0x19, 0xec,
				 0xec, 0x7a, 0xa7, 0xb0, 0x48, 0xb2, 0x0d,
				 0x1a, 0x58, 0xdf, 0x2d, 0x37, 0xf4, 0x81,
				 0x4d, 0x63, 0xc7, 0xff, 0xff, 0xff, 0xff,
				 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00
			       };

static const u8 Order_P521[] = { 0x09, 0x64, 0x38, 0x91, 0x1e, 0xb7, 0x6f,
				 0xbb, 0xae, 0x47, 0x9c, 0x89, 0xb8, 0xc9,
				 0xb5, 0x3b, 0xd0, 0xa5, 0x09, 0xf7, 0x48,
				 0x01, 0xcc, 0x7f, 0x6b, 0x96, 0x2f, 0xbf,
				 0x83, 0x87, 0x86, 0x51, 0xfa, 0xff, 0xff,
				 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				 0xff, 0xff, 0x01
			       };

static const u8 EcdsaGpoint_P256_Gx[] = { 0x96, 0xc2, 0x98, 0xd8, 0x45, 0x39,
					  0xa1, 0xf4, 0xa0, 0x33, 0xeb, 0x2d,
					  0x81, 0x7d, 0x03, 0x77, 0xf2, 0x40,
					  0xa4, 0x63, 0xe5, 0xe6, 0xbc, 0xf8,
					  0x47, 0x42, 0x2c, 0xe1, 0xf2, 0xd1,
					  0x17, 0x6b
					};

static const u8 EcdsaGpoint_P256_Gy[] = { 0xf5, 0x51, 0xbf, 0x37, 0x68, 0x40,
					  0xb6, 0xcb, 0xce, 0x5e, 0x31, 0x6b,
					  0x57, 0x33, 0xce, 0x2b, 0x16, 0x9e,
					  0x0f, 0x7c, 0x4a, 0xeb, 0xe7, 0x8e,
					  0x9b, 0x7f, 0x1a, 0xfe, 0xe2, 0x42,
					  0xe3, 0x4f
					};

static const u8 EcdsaGpoint_P384_Gx[] = { 0xb7, 0x0a, 0x76, 0x72, 0x38, 0x5e,
					  0x54, 0x3a, 0x6c, 0x29, 0x55, 0xbf,
					  0x5d, 0xf2, 0x02, 0x55, 0x38, 0x2a,
					  0x54, 0x82, 0xe0, 0x41, 0xf7, 0x59,
					  0x98, 0x9b, 0xa7, 0x8b, 0x62, 0x3b,
					  0x1d, 0x6e, 0x74, 0xad, 0x20, 0xf3,
					  0x1e, 0xc7, 0xb1, 0x8e, 0x37, 0x05,
					  0x8b, 0xbe, 0x22, 0xca, 0x87, 0xaa
					};

static const u8 EcdsaGpoint_P384_Gy[] = { 0x5f, 0x0e, 0xea, 0x90, 0x7c, 0x1d,
					  0x43, 0x7a, 0x9d, 0x81, 0x7e, 0x1d,
					  0xce, 0xb1, 0x60, 0x0a, 0xc0, 0xb8,
					  0xf0, 0xb5, 0x13, 0x31, 0xda, 0xe9,
					  0x7c, 0x14, 0x9a, 0x28, 0xbd, 0x1d,
					  0xf4, 0xf8, 0x29, 0xdc, 0x92, 0x92,
					  0xbf, 0x98, 0x9e, 0x5d, 0x6f, 0x2c,
					  0x26, 0x96, 0x4a, 0xde, 0x17, 0x36
					};

static const u8 EcdsaGpoint_P521_Gx[] = { 0x66, 0xbd, 0xe5, 0xc2, 0x31, 0x7e,
					  0x7e, 0xf9, 0x9b, 0x42, 0x6a, 0x85,
					  0xc1, 0xb3, 0x48, 0x33, 0xde, 0xa8,
					  0xff, 0xa2, 0x27, 0xc1, 0x1d, 0xfe,
					  0x28, 0x59, 0xe7, 0xef, 0x77, 0x5e,
					  0x4b, 0xa1, 0xba, 0x3d, 0x4d, 0x6b,
					  0x60, 0xaf, 0x28, 0xf8, 0x21, 0xb5,
					  0x3f, 0x05, 0x39, 0x81, 0x64, 0x9c,
					  0x42, 0xb4, 0x95, 0x23, 0x66, 0xcb,
					  0x3e, 0x9e, 0xcd, 0xe9, 0x04, 0x04,
					  0xb7, 0x06, 0x8e, 0x85, 0xc6, 0x00
					};

static const u8 EcdsaGpoint_P521_Gy[] = { 0x50, 0x66, 0xd1, 0x9f, 0x76, 0x94,
					  0xbe, 0x88, 0x40, 0xc2, 0x72, 0xa2,
					  0x86, 0x70, 0x3c, 0x35, 0x61, 0x07,
					  0xad, 0x3f, 0x01, 0xb9, 0x50, 0xc5,
					  0x40, 0x26, 0xf4, 0x5e, 0x99, 0x72,
					  0xee, 0x97, 0x2c, 0x66, 0x3e, 0x27,
					  0x17, 0xbd, 0xaf, 0x17, 0x68, 0x44,
					  0x9b, 0x57, 0x49, 0x44, 0xf5, 0x98,
					  0xd9, 0x1b, 0x7d, 0x2c, 0xb4, 0x5f,
					  0x8a, 0x5c, 0x04, 0xc0, 0x3b, 0x9a,
					  0x78, 0x6a, 0x29, 0x39, 0x18, 0x01
					};

static const u32 EcdsaSign_P256_Descriptor[] = { 0x00000002, 0x00101F30,
						 0x00600006, 0x00000000,
						 0x0040000E, 0x00010000,
						 0xB04E0016, 0x00000000
					       };

static const u32 EcdsaSign_P384_Descriptor[] = { 0x00000002, 0x00202F30,
						 0x00900006, 0x00000000,
						 0x0060000E, 0x00010000,
						 0xB04E0016, 0x00000000
					       };

static const u32 EcdsaSign_P521_Descriptor[] = { 0x00000002, 0x00304130,
						 0x00C60006, 0x00000000,
						 0x0084000E, 0x00010000,
						 0xB04E0016, 0x00000000
					       };

static const u32 EcdsaVerify_P256_Descriptor[] = { 0x00000002, 0x00101F31,
						   0x00A00006, 0x00000000,
						   0x0000000E, 0x00010000,
						   0xB04E0016, 0x00000000
						  };

static const u32 EcdsaVerify_P384_Descriptor[] = { 0x00000002, 0x00202F31,
						   0x00F00006, 0x00000000,
						   0x0000000E, 0x00010000,
						   0xB04E0016, 0x00000000
						 };

static const u32 EcdsaVerify_P521_Descriptor[] = { 0x00000002, 0x00304131,
						   0x014A0006, 0x00000000,
						   0x0000000E, 0x00010000,
						   0xB04E0016, 0x00000000
						 };

static const u32 EcdsaPointMul_P256_Descriptor[] = { 0x00000002, 0x00101F22,
						     0x00600006, 0x00000000,
						     0x0040000E, 0x00010000,
						     0xB04E0016, 0x00000000
                                                   };

static const u32 EcdsaPointMul_P384_Descriptor[] = { 0x00000002, 0x00202f22,
						     0x00900006, 0x00000000,
						     0x0060000E, 0x00010000,
						     0xB04E0016, 0x00000000
                                                   };

static const u32 EcdsaPointMul_P521_Descriptor[] = { 0x00000002, 0x00304122,
						     0x00C60006, 0x00000000,
						     0x0084000E, 0x00010000,
						     0xB04E0016, 0x00000000
						   };

static const u32 EccModAdd_P256_Descriptor[] = { 0x00000002, 0x00001F01,
                                                 0x00600006, 0x00000000,
                                                 0x0020000E, 0x00010000,
                                                 0xB04E0016, 0x00000000
                                               };

static const u32 EccModAdd_P384_Descriptor[] = { 0x00000002, 0x00002F02,
                                                 0x00900006, 0x00000000,
                                                 0x0030000E, 0x00010000,
                                                 0xB04E0016, 0x00000000
                                               };

static const u32 EccModAdd_P521_Descriptor[] = { 0x00000002, 0x00004101,
                                                 0x00C60006, 0x00000000,
                                                 0x0042000E, 0x00010000,
                                                 0xB04E0016, 0x00000000
                                               };

/*****************************************************************************/
/*****************************************************************************/
/**
 * @brief	This function is used to generate elliptic signature for a given
 *		hash, key and curve type
 *
 * @param	InstancePtr - Pointer to the XPki instance
 * @param	SignParams - Pointer to the XPki_EcdsaSignInputData
 * @param	XPki_EcdsaSign   - Pointer to the XPki_EcdsaSign
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XPKI_ECDSA_INVALID_PARAM - On invalid argument
 *	-	XPKI_ECDSA_SIGN_GEN_ERR - Fail to sign generate signaure.
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
int XPki_EcdsaGenerateSign(XPki_Instance *InstancePtr,
			   XPki_EcdsaSignInputData *SignParams,
			   XPki_EcdsaSign *Sign)
{
	u8 EcdsaCrvlen = XPki_GetEccCurveLen(SignParams->CrvType);
	volatile int Status = XPKI_ECDSA_NON_SUPPORTED_CRV;
	volatile int SStatus = XST_FAILURE;
	u64 EcdsaReqVal = 0;

	XSECURE_TEMPORAL_CHECK(END, Status, XPki_ValidateEcdsaGenerateSignParam,
			       InstancePtr, SignParams, Sign);

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END_CLR, Status, XPki_LoadInputData_EcdsaGenerateSign,
			       InstancePtr, SignParams, &EcdsaReqVal);

	XPki_Init_Ecdsa(InstancePtr, EcdsaReqVal);

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END_CLR, Status, XPki_Wait_For_Int_Done, XPKI_DONE_STATUS_VAL);

	Status =  XST_FAILURE;
	Status = XPki_Check_Output_CompletionQueue(InstancePtr->CQAddr);
	if (Status != XST_SUCCESS) {
		Status = XPKI_ECDSA_SIGN_GEN_ERR;
		goto END_CLR;
	}

	Xil_DCacheFlushRange((INTPTR)InstancePtr->RQOutputAddr, XPKI_MAX_RQ_CFG_BUF_SIZE);

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END_CLR, Status, Xil_SMemCpy, Sign->SignR, EcdsaCrvlen,
			       (u8 *)(UINTPTR)InstancePtr->RQOutputAddr, EcdsaCrvlen, EcdsaCrvlen);

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END_CLR, Status, Xil_SMemCpy, Sign->SignS, EcdsaCrvlen,
			       (u8 *)(UINTPTR)(InstancePtr->RQOutputAddr + EcdsaCrvlen), EcdsaCrvlen, EcdsaCrvlen);
END_CLR:
	/* Clear the internal Memory */
	SStatus = XST_FAILURE;
	SStatus = Xil_SMemSet((u8 *)InstancePtr->RQInputAddr,
			      XPKI_MAX_RQ_CFG_BUF_SIZE, 0U, XPKI_MAX_RQ_CFG_BUF_SIZE);
	if ((SStatus != XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

	SStatus = XST_FAILURE;
	SStatus = Xil_SMemSet((u8 *)InstancePtr->RQOutputAddr,
			      XPKI_MAX_RQ_CFG_BUF_SIZE, 0U, XPKI_MAX_RQ_CFG_BUF_SIZE);
	if ((SStatus != XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

	SStatus = XST_FAILURE;
	SStatus = Xil_SMemSet((u8 *)InstancePtr->CQAddr, XPKI_MAX_CQ_CFG_BUF_SIZE, 0U,
			      XPKI_MAX_CQ_CFG_BUF_SIZE);
	if ((SStatus != XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to verify the elliptic signature for a
 *		given hash, key and curve type
 *
 * @param	InstancePtr - Pointer to the XPki instance
 * @param	VerifyParams - Pointer to the XPki_EcdsaVerifyInputData
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XPKI_ECDSA_INVALID_PARAM - On invalid argument
 *	-	XPKI_ECDSA_SIGN_VERIFY_ERR - On signaure verifucation failed.
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
int XPki_EcdsaVerifySign(XPki_Instance *InstancePtr, XPki_EcdsaVerifyInputData *VerifyParams)
{
	volatile int Status = XPKI_ECDSA_NON_SUPPORTED_CRV;
	volatile int SStatus = XST_FAILURE;
	u64 EcdsaReqVal = 0;

	XSECURE_TEMPORAL_CHECK(END, Status, XPki_ValidateEcdsaVerifySignParam,
			       InstancePtr, VerifyParams);

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END_CLR, Status, XPki_LoadInputData_EcdsaVerifySign,
			       InstancePtr, VerifyParams, &EcdsaReqVal);

	XPki_Init_Ecdsa(InstancePtr, EcdsaReqVal);

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END_CLR, Status, XPki_Wait_For_Int_Done, XPKI_DONE_STATUS_VAL);

	Status = XST_FAILURE;
	Status = XPki_Check_Output_CompletionQueue(InstancePtr->CQAddr);
	if (Status != XST_SUCCESS) {
		Status = XPKI_ECDSA_SIGN_VERIFY_ERR;
		goto END_CLR;
	}
END_CLR:
	/* Clear the internal Memory */
	SStatus = XST_FAILURE;
	SStatus = Xil_SMemSet((u8 *)InstancePtr->RQInputAddr, XPKI_MAX_RQ_CFG_BUF_SIZE, 0U,
			      XPKI_MAX_RQ_CFG_BUF_SIZE);
	if ((SStatus != XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

	SStatus = XST_FAILURE;
	SStatus = Xil_SMemSet((u8 *)InstancePtr->CQAddr, XPKI_MAX_CQ_CFG_BUF_SIZE, 0U,
			      XPKI_MAX_CQ_CFG_BUF_SIZE);
	if ((SStatus != XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to perform the point multiplication for a
 *		given private key and Generator point on the elliptic curve.
 *
 * @param	InstancePtr - Pointer to the XPki instance
 * @param	PointMultiParams - Pointer to the XPki_EcdsaPointMultiInputData
 * @param	Gpoint - Pointer to the XPki_EcdsaGpoint
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XPKI_ECDSA_INVALID_PARAM - On invalid argument
 *	-	XPKI_ECDSA_POINT_MULT_ERR - On Point multiplication failure.
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
int XPki_EcdsaPointMulti(XPki_Instance *InstancePtr,
			 XPki_EcdsaPointMultiInputData *PointMultiParams,
			 XPki_EcdsaGpoint *Gpoint)
{
	u8 EcdsaCrvlen = XPki_GetEccCurveLen(PointMultiParams->CrvType);
	volatile int Status = XPKI_ECDSA_NON_SUPPORTED_CRV;
	volatile int SStatus = XST_FAILURE;
	u64 EcdsaReqVal = 0;

	XSECURE_TEMPORAL_CHECK(END, Status, XPki_ValidateEcdsaPointMultiParam,
			       InstancePtr, PointMultiParams, Gpoint);

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END_CLR, Status, XPki_LoadInputData_EcdsaPointMulti,
			       InstancePtr, PointMultiParams, &EcdsaReqVal);

	XPki_Init_Ecdsa(InstancePtr, EcdsaReqVal);

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END_CLR, Status, XPki_Wait_For_Int_Done, XPKI_DONE_STATUS_VAL)

	Status = XST_FAILURE;
	Status = XPki_Check_Output_CompletionQueue(InstancePtr->CQAddr);
	if (Status != XST_SUCCESS) {
		Status = XPKI_ECDSA_POINT_MULT_ERR;
		goto END_CLR;
	}

	Xil_DCacheFlushRange((INTPTR)InstancePtr->RQOutputAddr, XPKI_MAX_RQ_CFG_BUF_SIZE);

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END_CLR, Status, Xil_SMemCpy, (u8 *)Gpoint->Gx,
			       EcdsaCrvlen, (u8 *)(UINTPTR)InstancePtr->RQOutputAddr,
			       EcdsaCrvlen, EcdsaCrvlen);

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END_CLR, Status, Xil_SMemCpy, (u8 *)Gpoint->Gy,
			       EcdsaCrvlen, (u8 *)(UINTPTR)(InstancePtr->RQOutputAddr + EcdsaCrvlen),
			       EcdsaCrvlen, EcdsaCrvlen);
END_CLR:
	/* Clear the internal Memory */
	SStatus = XST_FAILURE;
	SStatus = Xil_SMemSet((u8 *)InstancePtr->RQInputAddr, XPKI_MAX_RQ_CFG_BUF_SIZE, 0U,
			      XPKI_MAX_RQ_CFG_BUF_SIZE);
	if ((SStatus != XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

	SStatus = XST_FAILURE;
	SStatus = Xil_SMemSet((u8 *)InstancePtr->RQOutputAddr,
			      XPKI_MAX_RQ_CFG_BUF_SIZE, 0U, XPKI_MAX_RQ_CFG_BUF_SIZE);
        if ((SStatus != XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

	SStatus = XST_FAILURE;
	SStatus = Xil_SMemSet((u8 *)InstancePtr->CQAddr, XPKI_MAX_CQ_CFG_BUF_SIZE, 0U,
			      XPKI_MAX_CQ_CFG_BUF_SIZE);
	if ((SStatus != XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to perform the modular addition
 *
 * @param	InstancePtr - Pointer to the XPki instance
 * @param	ModAddParams - Pointer to the XPki_ModAddInputData
 * @param	OutBuff - Pointer to the Buffer to store the result
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XPKI_ECDSA_INVALID_PARAM - On invalid argument
 *	-	XPKI_MOD_ADD_ERR - On modular addition failure.
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
static int XPki_GetModulusAdd(XPki_Instance *InstancePtr,
			      XPki_ModAddInputData *ModAddParams, u8 *OutBuff)
{
	u8 EcdsaCrvlen = XPki_GetEccCurveLen(ModAddParams->CrvType);
	volatile int Status = XPKI_ECDSA_NON_SUPPORTED_CRV;
	volatile int SStatus = XST_FAILURE;
	u64 EcdsaReqVal = 0;

	XSECURE_TEMPORAL_CHECK(END, Status, XPki_ValidateModulusAddParam,
			       InstancePtr, ModAddParams, OutBuff);

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END_CLR, Status, XPki_LoadInputData_ModulusAdd,
			       InstancePtr, ModAddParams, &EcdsaReqVal);

	XPki_Init_Ecdsa(InstancePtr, EcdsaReqVal);

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END_CLR, Status, XPki_Wait_For_Int_Done, XPKI_DONE_STATUS_VAL);

	Status = XST_FAILURE;
	Status = XPki_Check_Output_CompletionQueue(InstancePtr->CQAddr);
	if (Status != XST_SUCCESS) {
		Status = XPKI_MOD_ADD_ERR;
		goto END_CLR;
	}

	Xil_DCacheFlushRange((INTPTR)InstancePtr->RQOutputAddr, XPKI_MAX_RQ_CFG_BUF_SIZE);

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END_CLR, Status, Xil_SMemCpy, OutBuff, EcdsaCrvlen,
			       (u8 *)(UINTPTR)InstancePtr->RQOutputAddr, EcdsaCrvlen, EcdsaCrvlen);
END_CLR:
	/* Clear the internal Memory */
	SStatus = XST_FAILURE;
	SStatus = Xil_SMemSet((u8 *)InstancePtr->RQInputAddr,
			      XPKI_MAX_RQ_CFG_BUF_SIZE, 0U,
			      XPKI_MAX_RQ_CFG_BUF_SIZE);
	if ((SStatus != XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

	SStatus = XST_FAILURE;
	SStatus = Xil_SMemSet((u8 *)InstancePtr->RQOutputAddr,
			      XPKI_MAX_RQ_CFG_BUF_SIZE, 0U,
			      XPKI_MAX_RQ_CFG_BUF_SIZE);
	if ((SStatus != XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

	SStatus = XST_FAILURE;
	SStatus = Xil_SMemSet((u8 *)InstancePtr->CQAddr, XPKI_MAX_CQ_CFG_BUF_SIZE, 0U,
			      XPKI_MAX_CQ_CFG_BUF_SIZE);
	if ((SStatus != XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function Generate key pairs for the ECC curves NIST-P256,
 *		P384 and P521.
 *
 * @param	InstancePtr - Pointer to the XPki instance
 * @param	CrvType - Type of ECC curve(NIST-P256,P384 and P521).
 * @param	PubKey	- Pointer to the pulic-key buffer
 * @param	PrivKey - Pointer to the Private-key buffer
 *
 * @return
 *	-	XST_SUCCESS - when passes
 *	-	Errorcode       - when fails
 *
 *****************************************************************************/
int XPki_EcdsaGenerateKeyPair(XPki_Instance *InstancePtr, XPki_EcdsaCrvType CrvType,
			      XPki_EcdsaKey *PubKey, u8 *PrivKey)
{
	XPki_EcdsaGpoint *Gpoint =  XPki_GetEccGpoint(CrvType);
	XPki_EcdsaPointMultiInputData PointMulti_Inputs = {0};
	u8 Size = XPki_GetEccCurveLen(CrvType);
	XPki_EcdsaGpoint GpointOutPut = {0};
	volatile int Status = XST_FAILURE;

	if ((Gpoint->Gx == NULL) || (Gpoint->Gy == NULL)) {
		Status = XPKI_ECDSA_NON_SUPPORTED_CRV;
                goto END;
        }

	XSECURE_TEMPORAL_CHECK(END, Status, XPki_GetEccPrivateKey, InstancePtr, CrvType, PrivKey);

	PointMulti_Inputs.CrvType = CrvType;
	PointMulti_Inputs.Gpoint.Gx = Gpoint->Gx;
	PointMulti_Inputs.Gpoint.Gy = Gpoint->Gy;
	PointMulti_Inputs.Gpoint.Gxlen = Gpoint->Gxlen;
	PointMulti_Inputs.Gpoint.Gylen = Gpoint->Gylen;
	PointMulti_Inputs.D = PrivKey;
	PointMulti_Inputs.Dlen = Size;

	GpointOutPut.Gx = PubKey->Qx;
	GpointOutPut.Gy = PubKey->Qy;

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, XPki_EcdsaPointMulti,
			       InstancePtr, &PointMulti_Inputs, &GpointOutPut);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function copy the user-pointed data into the Request queue
 *		buffer in the below order(as per the PKI Spec) to generate the
 *		signature.
 *		-Private key
 *		-Ehimeral key
 *		-Data Hash
 *		-Signature Descriptor Data
 *
 * @param	InstancePtr - Pointer to the XPki instance
 * @param	SignParams - Pointer to the XPki_EcdsaSignInputData
 * @param	EcdsaReqVal - Pointer to the Generate Signature Descriptor Data offset
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XST_FAILURE - On failure
 *	-	XPKI_ECDSA_NON_SUPPORTED_CRV - On Un-supported elliptic curves
 *
 ******************************************************************************/
static int XPki_LoadInputData_EcdsaGenerateSign(XPki_Instance *InstancePtr,
						XPki_EcdsaSignInputData *SignParams,
						u64 *EcdsaReqVal)
{
	u8 EcdsaCrvlen = XPki_GetEccCurveLen(SignParams->CrvType);
	UINTPTR Addr = InstancePtr->RQInputAddr;
	volatile int Status = XST_FAILURE;
	u32 ReqLen;

	/* Copy Private key */
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
			       SignParams->Dlen, SignParams->D,
			       SignParams->Dlen, SignParams->Dlen);

	/* Copy Ehimeral key */
	Addr += SignParams->Dlen;
	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
			       SignParams->Klen, SignParams->K,
			       SignParams->Klen, SignParams->Klen);

	/* Copy Data hash */
	Addr += SignParams->Klen;
	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
			       SignParams->Hashlen, SignParams->Hash,
			       SignParams->Hashlen, SignParams->Hashlen);

	Addr += SignParams->Hashlen;
	ReqLen = EcdsaCrvlen - SignParams->Hashlen;
	if (ReqLen != 0) {
		Status = XST_FAILURE;
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemSet, (u8 *)Addr,
				       ReqLen, 0U, ReqLen);
		Addr += ReqLen;
	}

	/* Copy Write Descriptor */
	Status = XST_FAILURE;
	if (SignParams->CrvType  == ECC_NIST_P256) {
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
				       XPKI_DESC_LEN_BYTES, EcdsaSign_P256_Descriptor,
				       XPKI_DESC_LEN_BYTES, XPKI_DESC_LEN_BYTES);
	} else if (SignParams->CrvType == ECC_NIST_P384) {
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
				       XPKI_DESC_LEN_BYTES, EcdsaSign_P384_Descriptor,
				       XPKI_DESC_LEN_BYTES, XPKI_DESC_LEN_BYTES);
	} else if (SignParams->CrvType == ECC_NIST_P521) {
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemSet, (u8 *)Addr,
				       XPKI_SIGN_P521_PADD_BYTES, 0U,
				       XPKI_SIGN_P521_PADD_BYTES);
		Addr += XPKI_SIGN_P521_PADD_BYTES;
		Status = XST_FAILURE;
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
				       XPKI_DESC_LEN_BYTES, EcdsaSign_P521_Descriptor,
				       XPKI_DESC_LEN_BYTES, XPKI_DESC_LEN_BYTES);
	} else {
		Status = XPKI_ECDSA_NON_SUPPORTED_CRV;
	}

	*EcdsaReqVal = XPKI_ECDSA_NEW_REQUEST_MASK & (Addr + 1);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function  copy the user-pointed data into the Request queue
 *		buffer in the below order(as per the PKI Spec) to validate the
 *		signature.
 *		-Public key(Qx, Qy)
 *		-Signature(SignR, SignS)
 *		-Data Hash
 *		-Verify Signature Descriptor Data
 *
 * @param	InstancePtr - Pointer to the XPki instance
 * @param	VerifyParams - Pointer to the XPki_EcdsaVerifyInputData
 * @param	EcdsaReqVal - Pointer to the Verify Signature Descriptor Data offset
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XST_FAILURE - On failure
 *	-	XPKI_ECDSA_NON_SUPPORTED_CRV - On Un-supported elliptic curves
 *
 ******************************************************************************/
static int XPki_LoadInputData_EcdsaVerifySign(XPki_Instance *InstancePtr,
					      XPki_EcdsaVerifyInputData *VerifyParams,
					      u64 *EcdsaReqVal)
{
	u8 EcdsaCrvlen = XPki_GetEccCurveLen(VerifyParams->CrvType);
	UINTPTR Addr = InstancePtr->RQInputAddr;
	volatile int Status = XST_FAILURE;
	u8 ReqLen;

	/* Copy Public key */
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
			       EcdsaCrvlen, VerifyParams->PubKey.Qx,
			       EcdsaCrvlen, EcdsaCrvlen);

	Addr += EcdsaCrvlen;
	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
			       EcdsaCrvlen, VerifyParams->PubKey.Qy,
			       EcdsaCrvlen, EcdsaCrvlen);

	/* Copy signature */
	Addr += EcdsaCrvlen;
	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
			       EcdsaCrvlen, VerifyParams->Sign.SignR,
			       EcdsaCrvlen, EcdsaCrvlen);

	Addr += EcdsaCrvlen;
	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
			       EcdsaCrvlen, VerifyParams->Sign.SignS,
			       EcdsaCrvlen, EcdsaCrvlen);

	/* Copy Data hash */
	Addr += EcdsaCrvlen;
	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
			       VerifyParams->Hashlen, VerifyParams->Hash,
			       VerifyParams->Hashlen, VerifyParams->Hashlen);

	Addr += VerifyParams->Hashlen;
	ReqLen = EcdsaCrvlen - VerifyParams->Hashlen;
	if (ReqLen != 0) {
		Status = XST_FAILURE;
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemSet, (u8 *)Addr,
				       ReqLen, 0U, ReqLen);
		Addr += ReqLen;
	}

	/* Copy Write Descriptor */
	Status = XST_FAILURE;
	if (VerifyParams->CrvType  == ECC_NIST_P256) {
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
				       XPKI_DESC_LEN_BYTES, EcdsaVerify_P256_Descriptor,
				       XPKI_DESC_LEN_BYTES, XPKI_DESC_LEN_BYTES);
	} else if (VerifyParams->CrvType == ECC_NIST_P384) {
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
				       XPKI_DESC_LEN_BYTES, EcdsaVerify_P384_Descriptor,
				       XPKI_DESC_LEN_BYTES, XPKI_DESC_LEN_BYTES);
	} else if (VerifyParams->CrvType == ECC_NIST_P521) {
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemSet, (u8 *)Addr,
				       XPKI_VERIFY_P521_PADD_BYTES, 0U,
				       XPKI_VERIFY_P521_PADD_BYTES);
		Addr += XPKI_VERIFY_P521_PADD_BYTES;
		Status = XST_FAILURE;
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
				       XPKI_DESC_LEN_BYTES, EcdsaVerify_P521_Descriptor,
				       XPKI_DESC_LEN_BYTES, XPKI_DESC_LEN_BYTES);
	} else {
		Status = XPKI_ECDSA_NON_SUPPORTED_CRV;
		goto END;
	}

	*EcdsaReqVal = XPKI_ECDSA_NEW_REQUEST_MASK & (Addr + 1);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function  copy the user-pointed data into the Request queue
 *		buffer in the below order(as per the PKI Spec) to generate the
 *		Public-key.
 *		-Private key
 *		-Generator points(Gx, Gy)
 *		-Point Multiplication Descriptor Data
 *
 * @param	InstancePtr - Pointer to the XPki instance
 * @param	PointMultiParams - Pointer to the XPki_EcdsaPointMultiInputData
 * @param	InstancePtr->RQInputAddr - Request Queue Address
 * @param	EcdsaReqVal - Pointer to the Point-Multiplication Descriptor Data offset
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XST_FAILURE - On failure
 *	-	XPKI_ECDSA_NON_SUPPORTED_CRV - On Un-supported elliptic curves
 *
 ******************************************************************************/
static int XPki_LoadInputData_EcdsaPointMulti(XPki_Instance *InstancePtr,
					      XPki_EcdsaPointMultiInputData *PointMultiParams,
					      u64 *EcdsaReqVal)
{
	UINTPTR Addr = (UINTPTR)InstancePtr->RQInputAddr;
	volatile int Status = XST_FAILURE;

	/* Copy Private key */
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
			       PointMultiParams->Dlen, PointMultiParams->D,
			       PointMultiParams->Dlen, PointMultiParams->Dlen);

	/* Copy Generator point Gx */
	Addr += PointMultiParams->Dlen;
	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
			       PointMultiParams->Gpoint.Gxlen, PointMultiParams->Gpoint.Gx,
			       PointMultiParams->Gpoint.Gxlen, PointMultiParams->Gpoint.Gxlen);

	/* Copy Generator point Gy */
	Addr += PointMultiParams->Gpoint.Gxlen;
	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
			       PointMultiParams->Gpoint.Gylen, PointMultiParams->Gpoint.Gy,
			       PointMultiParams->Gpoint.Gylen, PointMultiParams->Gpoint.Gylen);

	Addr += PointMultiParams->Gpoint.Gylen;

	 /* Copy Write Descriptor */
	Status = XST_FAILURE;
	if (PointMultiParams->CrvType  == ECC_NIST_P256) {
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
				       XPKI_DESC_LEN_BYTES, EcdsaPointMul_P256_Descriptor,
				       XPKI_DESC_LEN_BYTES, XPKI_DESC_LEN_BYTES);
	} else if (PointMultiParams->CrvType == ECC_NIST_P384) {
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
				       XPKI_DESC_LEN_BYTES, EcdsaPointMul_P384_Descriptor,
				       XPKI_DESC_LEN_BYTES, XPKI_DESC_LEN_BYTES);
	} else if (PointMultiParams->CrvType == ECC_NIST_P521) {
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemSet, (u8 *)Addr,
				       XPKI_SIGN_P521_PADD_BYTES, 0U, XPKI_SIGN_P521_PADD_BYTES);
		Addr += XPKI_SIGN_P521_PADD_BYTES;
		Status = XST_FAILURE;
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
				       XPKI_DESC_LEN_BYTES, EcdsaPointMul_P521_Descriptor,
				       XPKI_DESC_LEN_BYTES, XPKI_DESC_LEN_BYTES);
	} else {
		Status = XPKI_ECDSA_NON_SUPPORTED_CRV;
		goto END;
	}

	*EcdsaReqVal = XPKI_ECDSA_NEW_REQUEST_MASK & (Addr + 1);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function  copy the user-pointed data into the Request queue
 *		buffer in the below order(as per the PKI Spec) to To perform
 *		modulus addition.
 *		- Curve Order
 *		- TRGN Generated Random value(A)
 *		- Value B = 1 -fixed
 *		- Modular addition Descriptor Data
 *
 * @param	InstancePtr - Pointer to the XPki instance
 * @param	ModAddParams - Pointer to the XPki_ModAddInputData
 * @param	EcdsaReqVal - Pointer to the modular addition Descriptor Data offset
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XST_FAILURE - On failure
 *	-	XPKI_ECDSA_NON_SUPPORTED_CRV - On Un-supported elliptic curves
 *
 ******************************************************************************/
static int XPki_LoadInputData_ModulusAdd(XPki_Instance *InstancePtr,
					 XPki_ModAddInputData *ModAddParams,
					 u64 *EcdsaReqVal)
{
	UINTPTR Addr = InstancePtr->RQInputAddr;
	volatile int Status = XST_FAILURE;

	/* Copy Order */
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
			       ModAddParams->Dlen, ModAddParams->Order,
			       ModAddParams->Dlen, ModAddParams->Dlen);

	Addr += ModAddParams->Dlen;
	/* Copy A */
	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
			       ModAddParams->Dlen, ModAddParams->D,
			       ModAddParams->Dlen, ModAddParams->Dlen);

	Addr += ModAddParams->Dlen;
	/* Copy B */
	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemSet, (u8 *)Addr, 1U, 1U, 1U);

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemSet, (u8 *)Addr + 1U,
			       ModAddParams->Dlen - 1U, 0U, ModAddParams->Dlen -1U);

	Addr += ModAddParams->Dlen;
	/* Copy Write Descriptor */
	Status = XST_FAILURE;
	if (ModAddParams->CrvType  == ECC_NIST_P256) {
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
				       XPKI_DESC_LEN_BYTES, EccModAdd_P256_Descriptor,
				       XPKI_DESC_LEN_BYTES, XPKI_DESC_LEN_BYTES);
	} else if (ModAddParams->CrvType == ECC_NIST_P384) {
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
				       XPKI_DESC_LEN_BYTES, EccModAdd_P384_Descriptor,
				       XPKI_DESC_LEN_BYTES, XPKI_DESC_LEN_BYTES);
	} else if (ModAddParams->CrvType == ECC_NIST_P521) {
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemSet, (u8 *)Addr,
				       XPKI_SIGN_P521_PADD_BYTES, 0U, XPKI_SIGN_P521_PADD_BYTES);
		Addr += XPKI_SIGN_P521_PADD_BYTES;
		Status = XST_FAILURE;
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
				       XPKI_DESC_LEN_BYTES, EccModAdd_P521_Descriptor,
				       XPKI_DESC_LEN_BYTES, XPKI_DESC_LEN_BYTES);
	} else {
		Status = XPKI_ECDSA_NON_SUPPORTED_CRV;
		goto END;
	}

	*EcdsaReqVal = XPKI_ECDSA_NEW_REQUEST_MASK & (Addr + 1);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to initialize and trigger the
 *		PKI Operations.
 *
 ******************************************************************************/
static void XPki_Init_Ecdsa(XPki_Instance *InstancePtr, u64 EcdsaReqVal)
{
	XPki_SoftReset();
	Xil_Out32(FPD_PKI_RQ_CFG_PERMISSIONS, XPKI_RQ_CFG_PERMISSIONS_SAFE);
	Xil_Out64(FPD_PKI_RQ_CFG_PAGE_ADDR_INPUT, InstancePtr->RQInputAddr);
	Xil_Out64(FPD_PKI_RQ_CFG_PAGE_ADDR_OUTPUT, InstancePtr->RQOutputAddr);
	Xil_Out64(FPD_PKI_CQ_CFG_ADDR, InstancePtr->CQAddr);
	Xil_Out32(FPD_PKI_RQ_CFG_PAGE_SIZE, XPKI_RQ_CFG_PAGE_SIZE_1024);
	Xil_Out32(FPD_PKI_RQ_CFG_CQID, XPKI_RQ_CFG_CQID);
	Xil_Out32(FPD_PKI_CQ_CFG_SIZE, XPKI_CQ_CFG_SIZE_4096);
	Xil_Out32(FPD_PKI_CQ_CFG_IRQ_IDX, XPKI_CQ_CFG_IRQ_ID_VAL);
	Xil_Out32(FPD_PKI_RQ_CFG_QUEUE_DEPTH, XPKI_RQ_CFG_QUEUE_DEPTH_VAL);
	Xil_Out64(FPD_PKI_IRQ_ENABLE, XPKI_IRQ_ENABLE_VAL);

	/* Flush the cache before transfer */
	Xil_DCacheFlushRange((INTPTR)InstancePtr->RQInputAddr, XPKI_MAX_RQ_CFG_BUF_SIZE);
	Xil_DCacheFlushRange((INTPTR)InstancePtr->CQAddr, XPKI_MAX_CQ_CFG_BUF_SIZE);
	Xil_DCacheFlushRange((INTPTR)InstancePtr->RQOutputAddr, XPKI_MAX_CQ_CFG_BUF_SIZE);

	Xil_Out32(FPD_PKI_CQ_CTL_TRIGPOS, XPKI_CQ_CTL_TRIGPOS_VAL);
	Xil_Out64(FPD_PKI_RQ_CTL_NEW_REQUEST, EcdsaReqVal);
}

/*****************************************************************************/
/**
 * @brief	This function is used to check the PKI Operation status.
 *
 * @param	ExpVal - Expected Status value.
 *
 * @return
 *	- 	XST_SUCCESS - On success
 *	-	XPKI_DONE_STATUS_ERR - On failure
 *
 ******************************************************************************/
static int XPki_Wait_For_Int_Done(u64 ExpVal)
{
	volatile int Status = XPKI_DONE_STATUS_ERR;
	u32 PollCount = XPKI_DONE_POLL_COUNT;
	u64 RegVal;

	while (PollCount--) {
		RegVal = Xil_In64(FPD_PKI_IRQ_STATUS);
		if (RegVal == ExpVal) {
			Status = XST_SUCCESS;
			break;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function is used to check the completion queue status.
 *
 * @param       CQAddr - Completion queue address.
 *
 * @return
 *      -       XST_SUCCESS - On success
 *      -       XST_FAILURE - On failure
 *
 ******************************************************************************/
static int XPki_Check_Output_CompletionQueue(UINTPTR CQAddr)
{
	volatile int Status = XST_FAILURE;

	Xil_DCacheFlushRange((INTPTR)CQAddr, XPKI_MAX_CQ_CFG_BUF_SIZE);

	if ((Xil_In32(CQAddr) != XPKI_EXP_CQ_STATUS) ||
	    (Xil_In32(CQAddr + 4) != XPKI_EXP_CQ_VAL)) {
		Status = XST_FAILURE;
	} else {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to validate the elliptic curve signature
 *		generation input parameters.
 *
 * @param	InstancePtr - Pointer to the XPki instance
 * @param	SignParams - Pointer to the XPki_EcdsaSignInputData
 * @param	Sign   - Pointer to the XPki_EcdsaSign
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XPKI_ECDSA_INVALID_PARAM - On invalid argument
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
static int XPki_ValidateEcdsaGenerateSignParam(XPki_Instance *InstancePtr,
					       XPki_EcdsaSignInputData *SignParams,
					       XPki_EcdsaSign *Sign)
{
	u8 EcdsaCrvlen = XPki_GetEccCurveLen(SignParams->CrvType);
	volatile int Status = XST_FAILURE;

	if ((SignParams->D == NULL) || (SignParams->K == NULL) ||
	    (SignParams->Hash == NULL) || (Sign->SignR == NULL) ||
	    (Sign->SignS == NULL)|| (InstancePtr == NULL)) {
		Status = XPKI_ECDSA_INVALID_PARAM;
		goto END;
	}

	if ((SignParams->Dlen != EcdsaCrvlen) ||
	    (SignParams->Klen != EcdsaCrvlen)) {
		Status = XPKI_ECDSA_INVALID_PARAM;
		goto END;
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to validate the elliptic curve signature
 *		verification input parameters.
 *
 * @param	InstancePtr - Pointer to the XPki instance
 * @param	VerifyParams - Pointer to the XPki_EcdsaVerifyInputData
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XPKI_ECDSA_NON_SUPPORTED_CRV - For Un-supported elliptic curves
 *	-	XPKI_ECDSA_INVALID_PARAM - On invalid argument
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
static int XPki_ValidateEcdsaVerifySignParam(XPki_Instance *InstancePtr,
					     XPki_EcdsaVerifyInputData *VerifyParams)
{
	volatile int Status = XST_FAILURE;

	if (InstancePtr == NULL) {
		Status = XPKI_ECDSA_INVALID_PARAM;
		goto END;
	}

	if ((VerifyParams->CrvType != ECC_NIST_P256) &&
	    (VerifyParams->CrvType != ECC_NIST_P384) &&
	    (VerifyParams->CrvType != ECC_NIST_P521)) {
		Status = XPKI_ECDSA_NON_SUPPORTED_CRV;
		goto END;
	}

	if ((VerifyParams->PubKey.Qx == NULL) ||
	    (VerifyParams->PubKey.Qy == NULL) ||
	    (VerifyParams->Sign.SignR == NULL) ||
	    (VerifyParams->Sign.SignS == NULL) ||
            (VerifyParams->Hash == NULL)) {
		Status = XPKI_ECDSA_INVALID_PARAM;
		goto END;
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to validate the elliptic curve
 *		Point Multiplication input parameters.
 *
 * @param	InstancePtr - Pointer to the XPki instance
 * @param	PointMultiParams - Pointer o the XPki_EcdsaPointMultiInputData
 * @param	Gpoint - Pointer to the XPki_EcdsaGpoint
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XPKI_ECDSA_NON_SUPPORTED_CRV - For unsupported elliptic curves
 *	-	XPKI_ECDSA_INVALID_PARAM - On invalid argument
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
static int XPki_ValidateEcdsaPointMultiParam(XPki_Instance *InstancePtr,
					     XPki_EcdsaPointMultiInputData *PointMultiParams,
					     XPki_EcdsaGpoint *Gpoint)
{
	u8 EcdsaCrvlen = XPki_GetEccCurveLen(PointMultiParams->CrvType);
	volatile int Status = (int)XPKI_ECDSA_NON_SUPPORTED_CRV;

	if ((InstancePtr == NULL) ||
	    (PointMultiParams->D == NULL) ||
	    (PointMultiParams->Gpoint.Gx == NULL) ||
            (PointMultiParams->Gpoint.Gy == NULL) ||
            (Gpoint->Gx == NULL) || (Gpoint->Gy == NULL)) {
		Status = XPKI_ECDSA_INVALID_PARAM;
		goto END;
	}

	if ((PointMultiParams->Dlen != EcdsaCrvlen) ||
	    (PointMultiParams->Gpoint.Gxlen != EcdsaCrvlen) ||
	    (PointMultiParams->Gpoint.Gylen != EcdsaCrvlen)) {
		Status = XPKI_ECDSA_INVALID_PARAM;
		goto END;
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to validate the modular addition input
 *		parameters.
 *
 * @param	InstancePtr - Pointer to the XPki instance
 * @param	ModAddParams - Pointer to the XPki_ModAddInputData
 * @param	OutBuff- Pointer to the Output Buffer to store the result
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XPKI_ECDSA_NON_SUPPORTED_CRV - For unsupported elliptic curves
 *	-	XPKI_ECDSA_INVALID_PARAM - On invalid argument
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
static int XPki_ValidateModulusAddParam(XPki_Instance *InstancePtr,
					XPki_ModAddInputData *ModAddParams,
					u8 *OutBuff)
{
	u8 EcdsaCrvlen = XPki_GetEccCurveLen(ModAddParams->CrvType);
	volatile int Status = XST_FAILURE;

	if ((InstancePtr == NULL) || (ModAddParams->D == NULL) ||
	    (ModAddParams->Order == NULL) || (OutBuff == NULL)) {
		Status = XPKI_ECDSA_INVALID_PARAM;
		goto END;
	}

	if (ModAddParams ->Dlen != EcdsaCrvlen) {
		Status = XPKI_ECDSA_INVALID_PARAM;
		goto END;
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to get the ECC curve generator points.
 *
 * @param	CrvType - Type of ECC curve (NIST-P256,P384 and P521)
 *
 * @return
 *		ECC Generator Points(Gx, Gy)
 *
 *****************************************************************************/
static XPki_EcdsaGpoint* XPki_GetEccGpoint(XPki_EcdsaCrvType CrvType)
{
	static XPki_EcdsaGpoint Gpoint = {0U};

	if (CrvType == ECC_NIST_P256) {
		Gpoint.Gx = EcdsaGpoint_P256_Gx;
		Gpoint.Gy = EcdsaGpoint_P256_Gy;
		Gpoint.Gxlen = NIST_P256_LEN_BYTES;
		Gpoint.Gylen = NIST_P256_LEN_BYTES;
	} else if (CrvType == ECC_NIST_P384) {
		Gpoint.Gx = EcdsaGpoint_P384_Gx;
		Gpoint.Gy = EcdsaGpoint_P384_Gy;
		Gpoint.Gxlen = NIST_P384_LEN_BYTES;
		Gpoint.Gylen = NIST_P384_LEN_BYTES;
	} else if (CrvType == ECC_NIST_P521) {
		Gpoint.Gx = EcdsaGpoint_P521_Gx;
		Gpoint.Gy = EcdsaGpoint_P521_Gy;
		Gpoint.Gxlen = NIST_P521_LEN_BYTES;
		Gpoint.Gylen = NIST_P521_LEN_BYTES;
	} else {
		Gpoint.Gx = NULL;
		Gpoint.Gy = NULL;
	}

	return &Gpoint;
}

/*****************************************************************************/
/**
 * @brief	This function is used the get the ECC curve length.
 *
 * @param	CrvType - Type of ECC curve (NIST-P256,P384 and P521)
 *
 * @return
 *		ECC Curve lenth.
 *
 *****************************************************************************/
static u8 XPki_GetEccCurveLen(XPki_EcdsaCrvType CrvType)
{
	static u8 DLen;

	if (CrvType == ECC_NIST_P256) {
		DLen = NIST_P256_LEN_BYTES;
	} else if (CrvType == ECC_NIST_P384) {
		DLen = NIST_P384_LEN_BYTES;
	} else if (CrvType == ECC_NIST_P521) {
		DLen = NIST_P521_LEN_BYTES;
	} else {
		DLen = 0;
	}

	return DLen;
}

/*****************************************************************************/
/**
 * @brief	This function is used the get the ECC curve Order
 *
 * @param	CrvType - Type of ECC curve (NIST-P256,P384 and P521)
 *
 * @return
 *		ECC Curve Order.
 *
 *****************************************************************************/
static const u8* XPki_GetEccCurveOrder(XPki_EcdsaCrvType CrvType)
{
	static const u8 *Order;

	if (CrvType == ECC_NIST_P256) {
		Order = Order_P256;
	} else if (CrvType == ECC_NIST_P384) {
		Order = Order_P384;
	} else if (CrvType == ECC_NIST_P521) {
		Order = Order_P521;
	} else {
		Order = NULL;
	}

	return Order;
}

/*****************************************************************************/
/**
 * @brief       This function is used the get the ECC curve Private key.
 *
 * @param       InstancePtr     Pointer to the XPki instance
 * @param       CrvType - Type of ECC curve (NIST-P256,P384 and P521)
 * @Param	Priv - Pointer to the private-key buffer
 *
 * @return
 *              ECC Curve Private key.
 *
 *****************************************************************************/
static int XPki_GetEccPrivateKey(XPki_Instance *InstancePtr,
				 XPki_EcdsaCrvType CrvType, u8 *Priv)
{
	volatile int Status = XST_FAILURE;
	XPki_ModAddInputData ModAddParams = {0U};
	const u8 *Order = XPki_GetEccCurveOrder(CrvType);
	u8 Size = XPki_GetEccCurveLen(CrvType);

	if (Size == 0) {
		Status = XPKI_ECDSA_NON_SUPPORTED_CRV;
		goto END;
	}

	Status = XST_FAILURE;
	Status = XPki_TrngGenerateRandomNum(Size, Priv);
        if (Status != XST_SUCCESS) {
		goto END;
	}

	ModAddParams.CrvType = CrvType;
	ModAddParams.Order = Order;
	ModAddParams.Dlen = Size;
	ModAddParams.D = Priv;

	Status = XPki_GetModulusAdd(InstancePtr, &ModAddParams, Priv);

END:
	return Status;
}
