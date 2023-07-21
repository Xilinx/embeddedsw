/******************************************************************************
* Copyright (c) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xilskey_puf_registration.h
*
* This file contains header interface related information for PUF device
* and macros used in the driver
*
* @note
*
*               User configurable parameters for PUF
*------------------------------------------------------------------------------
*
*	#define	XSK_PUF_INFO_ON_UART			FALSE
*	TRUE will display syndrome data on the UART com port
*	FALSE will display any data on UART com port.
*
*	#define XSK_PUF_PROGRAM_EFUSE			FALSE
*	TRUE will program the generated syndrome data, CHash ,Auxiliary values,
*	and the Black key.
*	FALSE will not program data into eFUSE.
*
*	#define XSK_PUF_IF_CONTRACT_MANUFACTURER	FALSE
*	This should be enabled when application is hand over to contract
*	manufacturer.
*	TRUE will allow only authenticated application.
*	FALSE authentication is not mandatory.
*
*	#define XSK_PUF_REG_MODE			XSK_PUF_MODE4K
*	PUF registration is performed in 4K mode. Only 4K mode is supported and
*	user should not modify this value.
*
*	#define XSK_PUF_READ_SECUREBITS			FALSE
*	TRUE will read the status of the PUF secure bits from eFUSEs and will be
*	displayed on UART.
*	FALSE will not read the secure bits.
*
*	#define XSK_PUF_PROGRAM_SECUREBITS		FALSE
*	TRUE will program PUF secure bits based on the user input provided
*	at XSK_PUF_SYN_INVALID, XSK_PUF_SYN_WRLK and XSK_PUF_REGISTER_DISABLE
*	FALSE will not program any PUF secure bits.
*
*	#define XSK_PUF_SYN_INVALID			FALSE
*	TRUE will permanently invalidates the already programmed syndrome data.
*	FALSE will not modify anything
*
*	#define XSK_PUF_SYN_WRLK			FALSE
*	TRUE will permanently disable programming syndrome data into eFUSEs.
*	FALSE will not modify anything.
*
*	#define XSK_PUF_REGISTER_DISABLE		FALSE
*	TRUE will permanently disable PUF syndrome data registration.
*	FALSE will not modify anything.
*
*	#define XSK_PUF_RESERVED				FALSE
*	TRUE programs this reserved eFUSE bit.
*	FALSE will not modify anything.
*
*	#define		XSK_PUF_AES_KEY
*	"0000000000000000000000000000000000000000000000000000000000000000"
*	The value will be converted to hex buffer and encrypts
*	this with PUF in order to generate black key ,the black key will get
*	written to the PS eFUSE array when XSK_PUF_PROGRAM_EFUSE macro is set
*	to TRUE.
*	This value should be given in string format. It should be 64 characters
*	long, valid characters are 0-9,a-f,A-F. Any other character is
*	considered as invalid  and will not burn Black key.
*	Note: Provided here should be red key and application calculates the
*	black key and programs into eFUSE if XSK_PUF_PROGRAM_EFUSE macro is
*	TRUE.
*	To avoid programming eFUSE results can be displayed on UART com port
*	by making XSK_PUF_INFO_ON_UART to TRUE.
*
*	#define		XSK_PUF_BLACK_KEY_IV	"000000000000000000000000"
*	The value mentioned here will be converted to hex buffer.
*	This is Initialization vector(IV) used with the AES-GCM cryptographic
*	hardware in order to generate encrypted red key, which is black key.
*	This value should be given in string format. It should be 24 characters
*	long, valid characters are 0-9,a-f,A-F. Any other character is
*	considered as invalid string.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 6.1   rp   17/10/16 First release.
* 6.2   vns  03/10/17 Added support for programming and reading one reserved
*                     bit
* 6.3   vns  10/05/18 Corrected the following macro
*                     XSK_PUF_IV  to XSK_PUF_BLACK_KEY_IV and
*                     XSK_PUF_IF_CONTRACT_MANUFATURER to
*                     XSK_PUF_IF_CONTRACT_MANUFACTURER
* 6.8   psl  06/07/19 Added doxygen tags
* 7.5   ng   07/13/23 added SDT support
* </pre>
*
*
******************************************************************************/
#ifndef XILSKEY_PUF_REGISTRATION_H_
#define XILSKEY_PUF_REGISTRATION_H_

#ifdef __cplusplus
extern "C" {
#endif

/************************** Include Files ************************************/

#include "xsecure_aes.h"
#include "xilskey_eps_zynqmp_puf.h"

/************************** Constant Definitions ****************************/

#ifdef	XSK_ZYNQ_ULTRA_MP_PLATFORM
#ifndef SDT
#define XSK_CSUDMA_DEVICE_ID				XPAR_XCSUDMA_0_DEVICE_ID
#else
#define XSK_CSUDMA_DEVICE_ID				XPAR_XCSUDMA_0_BASEADDR
#endif
#endif

#define 	XSK_PUF_MODE4K		(0U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/* Following parameters should be configured by user */

#define XSK_PUF_INFO_ON_UART			FALSE
#define XSK_PUF_PROGRAM_EFUSE			FALSE
#define XSK_PUF_IF_CONTRACT_MANUFACTURER	FALSE

/* For programming/reading secure bits of PUF */
#define XSK_PUF_READ_SECUREBITS			FALSE
#define XSK_PUF_PROGRAM_SECUREBITS		FALSE

#if (XSK_PUF_PROGRAM_SECUREBITS == TRUE)
#define	XSK_PUF_SYN_INVALID			FALSE
#define	XSK_PUF_SYN_WRLK			FALSE
#define	XSK_PUF_REGISTER_DISABLE		FALSE
#define	XSK_PUF_RESERVED			FALSE
#endif

#define	XSK_PUF_AES_KEY		"0000000000000000000000000000000000000000000000000000000000000000"
#define	XSK_PUF_BLACK_KEY_IV	"000000000000000000000000"

#define XSK_PUF_REG_MODE			XSK_PUF_MODE4K
						/**< Registration Mode
						  *  XPUF_MODE4K */

/***************************End of configurable parameters********************/

#if (XSK_PUF_INFO_ON_UART == TRUE)
#define	XPUF_INFO_ON_UART		/**< If defined, sends  information
					  *  on UART */
#define XPUF_DEBUG_GENERAL		1
#else
#define XPUF_DEBUG_GENERAL		0
#endif


#if (XSK_PUF_PROGRAM_EFUSE == TRUE)
#define	XPUF_FUSE_SYN_DATA		/**< If defined, writes syndrome data,
					  *  black key, Aux and Chash
					  *  values into eFUSE */
#endif

#if (XSK_PUF_IF_CONTRACT_MANUFACTURER == TRUE)
#define	XPUF_CONTRACT_MANUFACTURER	/**< If defined, additional checks
					  *  will be made to verify that app
					  *  is authenticated before running*/
#endif

/************************** Type Definitions **********************************/

/* All the instances used for this application */
XilSKey_Puf PufInstance;
XSecure_Aes AesInstance;
XCsuDma CsuDma;
XilSKey_ZynqMpEPs EfuseInstance;

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* XILSKEY_PUF_REGISTRATION_H_ */
