/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_secure.h
*
* This file contains all security related data.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  vns  04/23/19 First release
* 1.01  vns  07/09/19 Added PPK and SPK integrity checks
*                     Updated chunk size for secure partition
*                     Added encryption + authentication support
*       vns  07/23/19 Added functions to load secure headers
*       vns  08/23/19 Added buffer cleaning on failure
*                     Added different key sources support
* 1.02  vns  02/23/20 Added DPA CM enable/disable functionality
*       har  02/28/20 Added minor error codes for security
*       vns  03/01/20 Added PUF KEK decrypt support
*       ana  04/02/20 Added crypto engine KAT test function calls
*       bsv  04/07/20 Change CSUDMA name to PMCDMA
*       vns  04/13/20 Moved Aes instance to Secure structure
* 1.03  ana  06/04/20 Removed XLOADER_ECDSA_INDEXVAL macro and
*                     updated u32 datatype to u8 datatype
*       tar  07/23/20 Fixed MISRA-C required violations
*       kpt  07/30/20 Added minor error codes for ENC only and macros
*                     related to IV
*       bsv  08/06/20 Added delay load support for secure cases
*       har  08/11/20 Added XLoader_AuthJtagMessage structure  and macros for
*                     Authenticated JTAG
*       td   08/19/20 Fixed MISRA C violations Rule 10.3
*       bsv  08/21/20 Included xil_util.h for XSECURE_TEMPORAL_CHECK macro
*       har  08/24/20 Added macros related to ECDSA P521 support
*       kal  09/14/20 Added new error code to XLoader_SecErrCodes
*       har  09/30/20 Deprecated Family Key support
*       bsv  10/19/20 Parallel DMA changes
*       kpt  10/19/2020 Code clean up
*       har  10/19/20 Replaced ECDSA in header files
* 1.04  har  11/27/20 Added macros related to PDI DpaCm configuration
*       bm   12/16/20 Added PLM_SECURE_EXCLUDE macro. Also moved authentication and
*                     encryption related code to xloader_auth_enc.h file
* 1.05  har  03/17/21 Added API to set the secure state of device
*       har  05/20/21 Updated copyright year
* 1.06  har  07/15/21 Fixed doxygen warnings
*       kpt  09/09/21 Fixed SW-BP-BLIND-WRITE in XLoader_SecureClear
* 1.07  kpt  10/07/21 Removed function prototype XLoader_ProcessSecurePrtn
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XLOADER_SECURE_H
#define XLOADER_SECURE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xsecure_sha.h"
#include "xloader_auth_enc.h"
#include "xloader.h"
#include "xplmi_util.h"
#include "xil_util.h"
#include "xplmi_hw.h"

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Constant Definitions *****************************/
#define XLOADER_WORD_IN_BITS					(32U)
						/**< Word length in bits */

/**
 * @name  Clear buffer error codes
 * @{
 */
/**< In case of failure of any security operation, the buffer must be
 * cleared.In case of success/failure in clearing the buffer,
 * the following error codes shall be updated in the status
 */
#define XLOADER_SEC_CHUNK_CLEAR_ERR		((u32)0x20U << 8U)
#define XLOADER_SEC_BUF_CLEAR_ERR		((u32)0x80U << 8U)
				/**< Error in clearing buffer */
#define XLOADER_SEC_BUF_CLEAR_SUCCESS	((u32)0x40U << 8U)
				/**< Buffer is successfully cleared */
/** @} */

#define XLOADER_EFUSE_PPK0_START_OFFSET			(0xF1250100U)
					/**< PPK0 Start register address */
#define XLOADER_EFUSE_PPK1_START_OFFSET			(0xF1250120U)
					/**< PPK1 Start register address */
#define XLOADER_EFUSE_PPK2_START_OFFSET			(0xF1250140U)
					/**< PPK2 Start register address */
#define XLOADER_EFUSE_PPK2_END_OFFSET			(0xF125015CU)
					/**< PPK2 End register address */
#define XLOADER_EFUSE_SEC_MISC0_OFFSET			(0xF12500E4U)
					/**< Security Misc0 register address */
#define XLOADER_EFUSE_SEC_DEC_MASK			(0x0000FFFFU)
			/**< DEC_ONLY fuse mask in Security Misc0 register */

/**************************** Type Definitions *******************************/

/***************************** Function Prototypes ***************************/
int XLoader_SecureInit(XLoader_SecureParams *SecurePtr, XilPdi *PdiPtr,
	u32 PrtnNum);
int XLoader_SecureCopy(XLoader_SecureParams *SecurePtr, u64 DestAddr, u32 Size);
int XLoader_SecureClear(void);
int XLoader_SecureChunkCopy(XLoader_SecureParams *SecurePtr, u64 SrcAddr,
			u8 Last, u32 BlockSize, u32 TotalSize);
u32 XLoader_GetAHWRoT(const u32* AHWRoTPtr);
u32 XLoader_GetSHWRoT(const u32* SHWRoTPtr);
int XLoader_SetSecureState(void);

#ifdef __cplusplus
}
#endif

#endif /* XLOADER_SECURE_H */
