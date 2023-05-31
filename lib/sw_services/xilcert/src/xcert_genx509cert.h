/******************************************************************************
* Copyright (c) 2023, Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcert_genx509cert.h
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
#include "xplmi_debug.h"

/**************************** Type Definitions *******************************/
#define XCERT_USERCFG_MAX_SIZE						(32U)

/**************************** Constant Definitions *******************************/
#define XCERT_ISSUER_MAX_SIZE					(600U)
#define XCERT_SUBJECT_MAX_SIZE					(600U)
#define XCERT_VALIDITY_MAX_SIZE					(40U)
#define XCert_Printf						XPlmi_Printf

/**************************** Type Definitions *******************************/
typedef enum {
	XCERT_ISSUER = 0U,	/**< 0U */
	XCERT_SUBJECT,		/**< 1U */
	XCERT_VALIDITY		/**< 2U */
} XCert_UserCfgFields;

typedef struct {
	u32 SubsystemId;
	u8 Issuer[XCERT_ISSUER_MAX_SIZE];
	u32 IssuerLen;
	u8 Subject[XCERT_SUBJECT_MAX_SIZE];
	u32 SubjectLen;
	u8 Validity[XCERT_VALIDITY_MAX_SIZE];
	u32 ValidityLen;
} XCert_UserCfg;

typedef struct {
	u8 IsSelfSigned;
	u8* SubjectPublicKey;
	u8* PrvtKey;
}XCert_AppCfg;

typedef struct {
	u32 SubSystemId;
	XCert_UserCfg *UserCfg;
	XCert_AppCfg AppCfg;
}XCert_Config;

/************************** Function Prototypes ******************************/
int XCert_GenerateX509Cert(u64 X509CertAddr, u32 MaxCertSize, u32* X509CertSize, XCert_Config *Cfg);
XCert_UserCfg *XCert_GetCertUserInput(void);
int XCert_StoreCertUserInput(u32 SubSystemId, XCert_UserCfgFields FieldType, u8* Val, u32 Len);
u32* XCert_GetCertUsrCfgStoreIdx(void);

#endif
#ifdef __cplusplus
}
#endif  /* XCERT_GENX509CERT_H */
/* @} */
