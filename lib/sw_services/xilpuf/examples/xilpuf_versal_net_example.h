/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

 /*****************************************************************************/
 /**
 *
 * @file xilpuf_versal_net_example.h.
 *
 * This example is supported for versal_net devices.It contains macros which needs
 * to be configured by user for xilpuf_versal_net_example.c
 * On the basis of the options selected by user, the required operations will be performed.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who     Date     Changes
 * ----- ------  -------- ------------------------------------------------------
 * 1.0   har     07/05/22 First release
 *
 * </pre>
 *
 * GUIDE for using example
 *------------------------------------------------------------------------------
 * This example is provided to generate black key, UDS prime and encrypted DME
 * private keys by using either PUF registration or PUF on demand regeneration.
 * It can also be used to generate only the PUF ID.
 *
 * Please follow below guide to configure the specific macros as per your requirement:
 *
 * (I) GENERATE PUF KEK AND PUF ID
 *
 * #define XPUF_GENERATE_KEK_N_ID			(TRUE)
 * This macro must be configured as TRUE to generate both PUF KEK and PUF ID. In order
 * to generate PUF ID only, it should be configured as FALSE. The default option is set
 * as TRUE to enable encryption of keys and UDS.
 *
 * #define XPUF_KEY_GENERATE_OPTION			(XPUF_REGISTRATION)
 * 								OR
 * 							(XPUF_REGEN_ON_DEMAND)
 * This macro must be configured to select an option to generate PUF KEK. It can be configured
 * as XPUF_REGISTRATION or XPUF_REGEN_ON_DEMAND. By default it is configured as
 * XPUF_REGISTRATION. This configuration is mandatory if XPUF_GENERATE_KEK_N_ID is selected
 * as TRUE.
 *
 * #define XPUF_READ_HD_OPTION				(XPUF_READ_FROM_RAM)
 * This selects the location from where the helper data must be read by the
 * application. This option must be configured if XPUF_KEY_GENERATE_OPTION
 * is configured as XPUF_REGEN_ON_DEMAND or if XPUF_GENERATE_KEK_N_ID is configured as
 * FALSE.
 *
 * #define XPUF_CHASH					(0x00000000)
 * CHASH value should be supplied if XPUF_READ_HD_OPTION is configured as
 * XPUF_READ_FROM_RAM.The length of CHASH should be 24 bits This can be obtained
 * by performing PUF registration and writing the helper data on the UART.
 *
 * #define XPUF_AUX					(0x00000000)
 * AUX value should be supplied if XPUF_READ_HD_OPTION is configured as
 * XPUF_READ_FROM_RAM.The length of AUX should be 32 bits This can be obtained
 * by performing PUF registration and writing the helper data on the UART.
 *
 * #define XPUF_SYN_DATA_ADDRESS			(0x00000000)
 * Address of syndrome data should be supplied if XPUF_READ_HD_OPTION is
 * configured as XPUF_READ_FROM_RAM.
 *
 * #define XPUF_GLBL_VAR_FLTR_OPTION			(TRUE)
 * It is recommended to always enable this option to ensure entropy. It can
 * be configured as FALSE to disable Global Variation Filter.
 *
 * #define XPUF_RO_SWAP_VAL				(0x0U)
 * RO Swap Value should be supplied which will be used at time of PUF operation.
 *
 * (II) GENERATE BLACK KEY
 *
 * #define XPUF_ENCRYPT_RED_KEY				(FALSE)
 * This option must be configured as TRUE to encrypt red key to get
 * the black key.
 *
 * #define XPUF_RED_KEY
 * 		"0000000000000000000000000000000000000000000000000000000000000000"
 * Red Key to be encrypted by PUF KEK should be provided in string format. It
 * should be either 32 or 64 characters long as only 128 bit and 256 bit key
 * are supported.
 *
 * #define XPUF_RED_KEY_LEN				(XPUF_RED_KEY_SIZE_256)
 *								(or)
 *							(XPUF_RED_KEY_SIZE_128)
 * XPUF_RED_KEY_LEN can be configured as one of the two provided options. This
 * configuration should be done on the basis of the length of the red key in bits.
 * Only 128 bit and 256 bit key are supported. By default the length of red key is
 * configured as 256 bit.
 *
 * (III) GENERATE UDS PRIME
 *
 * #define XPUF_ENCRYPT_UDS				(FALSE)
 * This option must be configured as TRUE to encrypt UDS.
 *
 * #define XPUF_UDS
 * 		"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
 * UDS to be encrypted by PUF KEK should be provided in string format. It
 * should be 96 characters long.
 *
 * (IV) GENERATE ENCRYPTED DME PRIVATE KEYS
 *
 * #define XPUF_ENCRYPT_DME_PRIV_KEY_0			(FALSE)
 * This option must be configured as TRUE to encrypt DME Private key 0.
 *
 * #define XPUF_DME_PRIV_KEY_0
 * 		"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
 * DME Private Key 0 to be encrypted by PUF KEK should be provided in string format. It
 * should be 96 characters long.

 * #define XPUF_ENCRYPT_DME_PRIV_KEY_1			(FALSE)
 * This option must be configured as TRUE to encrypt DME Private key 1.
 *
  #define XPUF_DME_PRIV_KEY_1
 * 		"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
 * DME Private Key 1 to be encrypted by PUF KEK should be provided in string format. It
 * should be 96 characters long.
 *
 * #define XPUF_ENCRYPT_DME_PRIV_KEY_2			(FALSE)
 * This option must be configured as TRUE to encrypt DME Private key 2.
 *
  #define XPUF_DME_PRIV_KEY_2
 * 		"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
 * DME Private Key 2 to be encrypted by PUF KEK should be provided in string format. It
 * should be 96 characters long.
 * #define XPUF_ENCRYPT_DME_PRIV_KEY_3			(FALSE)
 * This option must be configured as TRUE to encrypt DME Private key 3.
  *
 * #define XPUF_DME_PRIV_KEY_3
 * 		"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
 * DME Private Key 3 to be encrypted by PUF KEK should be provided in string format. It
 * should be 96 characters long.
 *
 * #define XPUF_IV				"000000000000000000000000"
 * IV should be provided in string format.It should be 24 characters long, valid
 * characters are 0-9,a-f,A-F. Any other character is considered as invalid
 * string.The value mentioned here will be converted to hex buffer.It is used
 * with the AES-GCM cryptographic hardware in order to encrypt red key. Please note
 * that same IV will be incremented by one and used for AES encryption of UDS
 * and DME private keys.
 * Encryption of red key	 	=	IV
 * Encryption of UDS 			=	IV + 0x1
 * Encryption of DME private key 0	=	IV + 0x2
 * Encryption of DME private key 1 	=	IV + 0x3
 * Encryption of DME private key 2 	=	IV + 0x4
 * Encryption of DME private key 3 	=	IV + 0x5
 *
 ******************************************************************************/
#ifndef XILPUF_VERSAL_NET_EXAMPLE_H
#define XILPUF_VERSAL_NET_EXAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/***************** Macros (Inline Functions) Definitions *********************/
/*
 * Below macro values should match with enum XSecure_AesKeySize.
 * As preprocessor can't handle enums at preprocessing stage of compilation,
 * these macros are defined.
 */
#define XPUF_RED_KEY_SIZE_128			(0U)
#define XPUF_RED_KEY_SIZE_256			(2U)

/************************** Constant Definitions *****************************/
#define XPUF_GENERATE_KEK_N_ID			(TRUE)

#define XPUF_KEY_GENERATE_OPTION		(XPUF_REGISTRATION)

#define XPUF_READ_HD_OPTION			(XPUF_READ_FROM_RAM)
#define XPUF_CHASH				(0x00000000U)
#define XPUF_AUX				(0x0000000U)
#define XPUF_SYN_DATA_ADDRESS			(0x00000000U)

#define XPUF_GLBL_VAR_FLTR_OPTION		(TRUE)
#define XPUF_RO_SWAP_VAL			(0x0U)

#define XPUF_ENCRYPT_RED_KEY			(FALSE)
#define XPUF_RED_KEY	\
	"0000000000000000000000000000000000000000000000000000000000000000"
#define XPUF_RED_KEY_LEN			(XPUF_RED_KEY_SIZE_256)
#if (XPUF_RED_KEY_LEN == XPUF_RED_KEY_SIZE_256)
#define XPUF_RED_KEY_LEN_IN_BYTES 		(32U)
#elif (XPUF_RED_KEY_LEN == XPUF_RED_KEY_SIZE_128)
#define XPUF_RED_KEY_LEN_IN_BYTES 		(16U)
#endif

#define XPUF_ENCRYPT_UDS			(FALSE)
#define XPUF_UDS	\
	"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"

#define XPUF_ENCRYPT_DME_PRIV_KEY_0		(FALSE)
#define XPUF_DME_PRIV_KEY_0	\
	"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"

#define XPUF_ENCRYPT_DME_PRIV_KEY_1		(FALSE)
#define XPUF_DME_PRIV_KEY_1	\
	"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"

#define XPUF_ENCRYPT_DME_PRIV_KEY_2		(FALSE)
#define XPUF_DME_PRIV_KEY_2	\
	"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"

#define XPUF_ENCRYPT_DME_PRIV_KEY_3		(FALSE)
#define XPUF_DME_PRIV_KEY_3	\
	"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"

#define XPUF_IV					"000000000000000000000000"
/**************************** Type Definitions *******************************/

#ifdef __cplusplus
}
#endif

#endif /* XILPUF_VERSAL_NET_EXAMPLE_H_ */
