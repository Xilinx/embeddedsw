/******************************************************************************
*
* (c) Copyright 2014 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
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
*
*</pre>
*
******************************************************************************/
#ifndef ___RSAEXHEADER_H___
#define ___RSAEXHEADER_H___

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"
#include "xstatus.h"
#include "xil_printf.h"
#include "xilrsa.h"

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

#define APPLICATION_START_ADDR	0x0

#define PARTITION_SIZE	0x0

#define CERTIFICATE_START_ADDR	0x0

/*
 * RSA certificate related definitions
 */
#define RSA_PPK_MODULAR_SIZE		256
#define RSA_PPK_MODULAR_EXT_SIZE	256
#define RSA_PPK_EXPO_SIZE		64
#define RSA_SPK_MODULAR_SIZE		256
#define RSA_SPK_MODULAR_EXT_SIZE	256
#define RSA_SPK_EXPO_SIZE		64
#define RSA_SPK_SIGNATURE_SIZE		256
#define RSA_PARTITION_SIGNATURE_SIZE	256
#define RSA_SIGNATURE_SIZE		0x6C0 	/* Signature size in bytes */
#define RSA_HEADER_SIZE			4 /* Signature header size in bytes */
#define RSA_MAGIC_WORD_SIZE		60	/* Magic word size in bytes */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Global Function Prototypes ***********************/

int AuthenticateApp(void);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif /* ___RSAEXHEADER_H___ */
