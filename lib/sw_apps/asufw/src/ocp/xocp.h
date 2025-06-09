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
#include "xasu_eccinfo.h"
#include "xasufw_dma.h"
#include "xil_types.h"

#ifdef XASU_OCP_ENABLE
/************************************ Constant Definitions ***************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XOCP_PERSONAL_STRING_LEN		(48U)	/**< Personalised string length */

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

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif /* XASU_OCP_ENABLE */
#endif /* XOCP_H_ */
/** @} */
