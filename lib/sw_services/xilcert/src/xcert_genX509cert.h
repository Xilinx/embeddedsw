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

#ifndef PLM_ECDSA_EXCLUDE

/**************************** Type Definitions *******************************/
typedef struct {
	u8* Issuer;
	u8* Subject;
	u8* Validity;
} XCert_UserCfg;

typedef struct {
	u8 IsSelfSigned;
	u8* SubjectPublicKey;
	u8* PrvtKey;
}XCert_AppCfg;

typedef struct {
	XCert_UserCfg UserCfg;
	XCert_AppCfg AppCfg;
}XCert_Config;

/************************** Function Prototypes ******************************/
int XCert_GenerateX509Cert(u8* X509CertBuf, u32 MaxCertSize, u32* X509CertSize, XCert_Config Cfg);

#endif
#ifdef __cplusplus
}
#endif
#endif  /* XCERT_GENX509CERT_H */
/* @} */
