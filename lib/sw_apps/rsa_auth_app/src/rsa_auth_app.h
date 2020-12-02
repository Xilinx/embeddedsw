/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file rsa_auth_app.h
* 	This file is the header to the SW app used to authenticate a
* 	user application and contains the necessary definitions and prototypes.
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.0   hk  27/01/14 First release
*       kpt 20/08/20 Removed underscore from macro name
*
*</pre>
*
******************************************************************************/
#ifndef RSAEXHEADER_H_
#define RSAEXHEADER_H_

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/*
 * User should provide the following three parameters
 * APPLICATION_START_ADDR - Start address of the application to be validated
 * APP_PARTITION_SIZE - Length(in bytes) of the complete partition
 *                      should be provided.
 * CERTIFICATE_START_ADDR - Authentication certificate start address.
 *                          This should point to the Authentication header.
 * The application and authentication certificate are assumed to be at
 * contiguous memory locations.
 *
 *
 * APPLICATION_START_ADDR>-----------------------------------------------------
 * 			APPLICATION (with padding if any)
 * CERTIFICATE_START_ADDR>-----------------------------------------------------
 * 			AUTHENTICATION HEADER (With padding to 512bit boundary)
 * 			-------------------------------------------------------
 * 			PPK (With padding to 512 bit boundary)
 * 			-------------------------------------------------------
 * 			SPK (With padding to 512 bit boundary)
 * 			-------------------------------------------------------
 * 			SPK SIGNATURE (With padding to 512 bit boundary)
 * 			-------------------------------------------------------
 * 			APPLICATION SIGNATURE (With padding to 512bit boundary)
 * 			-------------------------------------------------------
 *
 * APP_PARTITION_SIZE is the complete size of the above mentioned partition
 * in bytes.
 *
 */

#define APPLICATION_START_ADDR	0x0U

#define PARTITION_SIZE			0x0U

#define CERTIFICATE_START_ADDR	0x0U

/*
 * RSA certificate related definitions
 */
#define RSA_PPK_MODULAR_SIZE			256U
#define RSA_PPK_MODULAR_EXT_SIZE		256U
#define RSA_PPK_EXPO_SIZE				64U
#define RSA_SPK_MODULAR_SIZE			256U
#define RSA_SPK_MODULAR_EXT_SIZE		256U
#define RSA_SPK_EXPO_SIZE				64U
#define RSA_SPK_SIGNATURE_SIZE			256U
#define RSA_PARTITION_SIGNATURE_SIZE	256U
#define RSA_HEADER_SIZE			        4U /* Signature header size in bytes */
#define RSA_MAGIC_WORD_SIZE		        60U	/* Magic word size in bytes */

#define RSA_BYTE_PAD_LENGTH				3U  /**< PKCS Byte Padding */
#define RSA_T_PAD_LENGTH				19U /**< PKCS T Padding */
#define HASHLEN             			32U /**  Hash length */


/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Global Function Prototypes ***********************/

static int AuthenticateApp(void);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif /* RSAEXHEADER_H_ */
