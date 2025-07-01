/**************************************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_ocpinfo.h
 *
 * This file contains the OCP definitions which are common across the
 * client and server
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   rmv  07/16/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasu_common_defs Common Defs
* @{
*/
#ifndef XASU_OCPINFO_H_
#define XASU_OCPINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xil_util.h"
#ifdef SDT
#include "xasu_bsp_config.h"
#endif

/************************************ Constant Definitions ***************************************/
/* OCP module command IDs */
#define XASU_OCP_GET_DEVIK_X509_CERT_CMD_ID	(0U)	/**< Command ID for DevIk x509 certificate
							generation */
#define XASU_OCP_GET_DEVAK_X509_CERT_CMD_ID	(1U)	/**< Command ID for DevAk x509 certificate
							generation */
#define XASU_OCP_GET_DEVIK_CSR_X509_CERT_CMD_ID	(2U)	/**< Command ID for DevIk CSR x509
							certificate generation */

/************************************** Type Definitions *****************************************/
/**
 * This enum contains list of device key types.
 */
enum {
	XOCP_DEVIK = 0,		/**< Device identity key */
	XOCP_DEVAK,		/**< Device attestation key */
	XOCP_MAX_DEVICE_KEY,	/**< Maximum support device key */
};

/**
 * This structure contains OCP params info.
 */
typedef struct {
	u64 CertBufAddr;	/**< Certificate buffer address */
	u32 CertBufLen;		/**< Certificate buffer length */
	u32 CertActualSize;	/**< Certificate actual size */
	u32 DevKeySel;		/**< Device key type */
} XAsu_OcpCertParams;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_OCPINFO_H_ */
/** @} */
