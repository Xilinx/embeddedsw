/**************************************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xocp.h
 *
 * This file contains declarations for xocp.c file.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date       Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   rmv  07/16/25 Initial release
 *       yog  08/21/25 Added OCP-DME support
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xocp_server_apis OCP server APIs
* @{
*/
#ifndef XOCP_H_
#define XOCP_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "x509_cert.h"
#include "xasu_eccinfo.h"
#include "xasu_ocpinfo.h"
#include "xasufw_dma.h"
#include "xil_types.h"

#ifdef XASU_OCP_ENABLE
/************************************ Constant Definitions ***************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
/** Starting from this memory address, 8K of memory is reserved for OCP CDO in the linker script. */
#define XOCP_CDO_DATA_ADDR		(0xEBE5BC00U)	/**< User data address */

#define XOCP_MAX_OCP_SUBSYSTEMS		(4U)	/**< Maximum number of OCP subsystems */
#define XOCP_USER_SUBSYS_START_INDEX	(1U)	/**< Start index for OCP subsystem */

#define XOCP_PERSONAL_STRING_LEN		(48U)	/**< Personalised string length */

#define XASUFW_SUBSYTEM_ID		(0x1C000002U)	/**< ASUFW subsystem ID */

/************************************** Type Definitions *****************************************/
/**
 * This structure is used to store generated device keys.
 */
typedef struct {
	u8 EccPvtKey[XASU_ECC_P384_SIZE_IN_BYTES];	/**< ECC private key */
	u32 PublicKeyLen;				/**< Public key length */
	u8 EccX[XASU_ECC_P384_SIZE_IN_BYTES];		/**< ECC public key x coordinate */
	u8 EccY[XASU_ECC_P384_SIZE_IN_BYTES];		/**< ECC public key y coordinate */
	u8 IsDevIkKeyReady;				/**< Indicates if DevIk is ready or not */
	u8 IsDevAkKeyReady;				/**< Indicates if DevAk is ready or not */
} XOcp_DeviceKeys;

/**
 * This structure contains information about seed, personalized string and ECC private key.
 */
typedef struct {
	u32 CdiAddr;		/**< Address to the CDI buffer. */
	u32 CdiLength;		/**< CDI length. */
	u32 PerStringAddr;	/**< Personalized string. */
	u32 PvtKeyAddr;		/**< Key output address. */
} XOcp_PrivateKeyGen;

/**
 * This structure contains information about Subsystems data.
 */
typedef struct {
	u32 SubSystemID;				/**< Subsystem Id */
	u8 PersonalString[XOCP_PERSONAL_STRING_LEN];	/**< Subsystem personalized string */
	u32 IsPersonalStringAvailable;			/**< Flag to indicate if personalized string
							is given by user */
	X509_UserCfg UserCfg;				/**< User configurations */
} XOcp_SubsystemData;

/**
 * This structure contains information related to CDO data.
 */
typedef struct {
	XOcp_SubsystemData AsuSubsysData;			/**< ASU Subsystem data */
	XOcp_SubsystemData SubsysData[XOCP_MAX_OCP_SUBSYSTEMS];	/**< Subsystem data */
	u32 NumSubsys;						/**< Total number of subsystems */
} XOcp_CdoData;

/**
 * This structure contains X.509 certificate related data.
 */
typedef struct {
	u32 DevKeyType;			/**< Device key type */
	u64 CertAddr;			/**< X.509 certificate address */
	u32 CertMaxSize;		/**< Maximum size of X.509 certificate */
	u32 *CertActualSize;		/**< Pointer to stores the actual size of X.509
					certificate */
} XOcp_CertData;

/************************************ Function Prototypes ****************************************/
s32 XOcp_GenerateDeviceKeys(XAsufw_Dma *DmaPtr, u32 EventMask);
s32 XOcp_GetX509Cert(u32 SubsystemId, const XOcp_CertData *CertPtr, void *PlatData, u8 IsCsr);
s32 XOcp_AttestWithDevAk(XAsufw_Dma *DmaPtr, const XAsu_OcpDevAkAttest *OcpAttestParam,
			 u32 SubsystemId);
XOcp_DeviceKeys* XOcp_GetDevIk(void);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif /* XASU_OCP_ENABLE */
#endif /* XOCP_H_ */
/** @} */
