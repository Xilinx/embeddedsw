/******************************************************************************
* Copyright (c) 2023, Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcert_genX509cert.h
*
* This file contains the functions to create different fields of the X.509
* certificate.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   har  01/09/2023 Initial release
*
* </pre>
*
*
******************************************************************************/

#ifndef XCERT_GENX509CERT_H
#define XCERT_GENX509CERT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xparameters.h"

/**************************** Type Definitions *******************************/
#define XCERT_USERCFG_MAX_SIZE						(32U)

typedef enum {
	XCERT_ISSUER = 0U,	/**< 0U */
	XCERT_SUBJECT,		/**< 1U */
	XCERT_VALIDITY		/**< 2U */
} XCert_UserCfgFields;

typedef struct {
	u8 Issuer[XCERT_USERCFG_MAX_SIZE];
	u32 IssuerLen;
	u8 Subject[XCERT_USERCFG_MAX_SIZE];
	u32 SubjectLen;
	u8 Validity[XCERT_USERCFG_MAX_SIZE];
	u32 ValidityLen;
} XCert_UserCfg;

typedef struct {
	u8 IsSelfSigned;
	u8* SubjectPublicKey;
	u8* PrvtKey;
}XCert_AppCfg;

typedef struct {
	XCert_UserCfg *UserCfg;
	XCert_AppCfg AppCfg;
}XCert_Config;

/************************** Function Prototypes ******************************/
int XCert_GenerateX509Cert(u8* X509CertBuf, u32 MaxCertSize, u32* X509CertSize, XCert_Config Cfg);
XCert_UserCfg *XCert_GetCertUserInput(void);
int XCert_StoreCertUserInput(XCert_UserCfgFields FieldType, u8* Val, u32 Len);

#endif
#ifdef __cplusplus
}
#endif  /* XCERT_GENX509CERT_H */
/* @} */
