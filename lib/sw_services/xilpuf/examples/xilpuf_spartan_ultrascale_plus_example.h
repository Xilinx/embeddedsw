/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
*
* @file xilpuf_spartan_ultrascale_plus_example.h.
* This file contains macros which needs to configured by user for
* xilpuf_example.cand based on the options selected by user operations will be
* performed.
*
* This example is supported for spartan ultrascale plus devices.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   kpt     08/23/24 First release
*
* </pre>
*
* User configurable parameters for PUF
*------------------------------------------------------------------------------
* #define XPUF_RED_KEY
* 	      "0000000000000000000000000000000000000000000000000000000000000000"
* Red Key to be encrypted by PUF KEY should be provided in string format.It
* should be either 32 or 64 characters long as only 128 bit and 256 bit key
* are supported.
*
* #define XPUF_RED_KEY_LEN				(XPUF_RED_KEY_SIZE_256)
*							(or)
*						(XPUF_RED_KEY_SIZE_128)
* XPUF_RED_KEY_LEN can be configured as one of the two provided options. This
* configuration should be done on the basis of the length of the red key in bits.
* Only 128 bit and 256 bit key are supported. By default the length of red key is
* configured as 256 bit.
*
* #define XPUF_IV				"000000000000000000000000"
* IV should be provided in string format.It should be 24 characters long, valid
* characters are 0-9,a-f,A-F. Any other character is considered as invalid
* string.The value mentioned here will be converted to hex buffer.It is used
* with the AES-GCM cryptographic hardware in order to encrypt red key.
*
* #define XPUF_GENERATE_KEK_N_ID			(TRUE)
* This macro must be configured as TRUE to generate both PUF KEK and PUF ID. In order
* to generate PUF ID only, it should be configured as FALSE. The default option is set
* as TRUE to enable encryption of Red Key.
*
* #define XPUF_KEY_GENERATE_OPTION		(XPUF_REGISTRATION)
*							(or)
*						(XPUF_REGEN_ON_DEMAND)
* PUF Key can be generated by PUF registration or PUF on-demand
* regeneration.The user can configure XPUF_KEY_GENERATE_OPTION as either
* XPUF_REGISTRATION or XPUF_REGEN_ON_DEMAND to select the mode of PUF operation
* to generate helper data. It should be configured only when XPUF_GENERATE_KEK_N_ID as TRUE.
*
*
 #define XPUF_CHASH				(0x00000000)
* CHASH value should be supplied if XPUF_READ_HD_OPTION is configured as
* XPUF_READ_FROM_RAM.The length of CHASH should be 24 bits This can be obtained
* by performing PUF registration and writing the helper data on the UART.
*
* #define XPUF_AUX				(0x00000000)
* AUX value should be supplied if XPUF_READ_HD_OPTION is configured as
* XPUF_READ_FROM_RAM.The length of AUX should be 32 bits This can be obtained
* by performing PUF registration and writing the helper data on the UART.
*
* #define XPUF_SYN_DATA_ADDRESS		(0x00000000)
* Address of syndrome data should be supplied if XPUF_READ_HD_OPTION is
* configured as XPUF_READ_FROM_RAM.
*
* #define XPUF_WRITE_PUF_HASH_IN_EFUSE		(FALSE)
* If this option is configured as TRUE then hash of PUF helper data is written into the
* eFuse.
*
* #define XPUF_GLBL_VAR_FLTR_OPTION	(TRUE)
* It is recommended to always enable this option to ensure entropy. It can
* be configured as FALSE to disable Global Variation Filter.
*
* #define XPUF_PRGM_HASH_PUF_OR_KEY (FALSE)
* If this option is configured as TRUE then PRGM_HASH_PUF_OR_KEY bit is programmed and will enforce
* PUF hash to be compared with programmed hash in PPK2 during boot
*
******************************************************************************/
#ifndef XILPUF_SPARTAN_ULTRASCALE_PLUS_EXAMPLE_H_
#define XILPUF_SPARTAN_ULTRASCALE_PLUS_EXAMPLE_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"

/************************** Constant Definitions *****************************/
#define XPUF_READ_HASH_PUF_OR_KEY			(FALSE)
/**< For reading Secure control eFUSE bits of PUF */

#define XPUF_RED_KEY	\
	"0000000000000000000000000000000000000000000000000000000000000000"
/**< Red key that is encrypted using PUF KEK to form a black key  */

/*
 * Below macro values should match with enum XSecure_AesKeySize.
 * As preprocessor can't handle enums at preprocessing stage of compilation,
 * these macros are defined.
 */
#define XPUF_RED_KEY_SIZE_128		(0U)
/**< Keysize 128 */
#define XPUF_RED_KEY_SIZE_256		(2U)
/**< Keysize 256 */

#define XPUF_RED_KEY_LEN			(XPUF_RED_KEY_SIZE_256)
/**< Value to indicate red key length that is used during black key generation */

#define XPUF_REGISTRATION 0U
/**< Value to indicate PUF registration */
#define XPUF_REGEN_ON_DEMAND 1U
/**< Value to indicate regeneration on demand */

#define XPUF_RED_KEY_LEN_IN_BYTES 		(32U)
/**< RED key length in bytes */

#define XPUF_IV					"000000000000000000000000"
/**< IV that is used during black key generation */

#define XPUF_GENERATE_KEK_N_ID			(TRUE)
/**< This will enable/disable generating black key and
     it is only applicable during registraion */
#define XPUF_KEY_GENERATE_OPTION		(XPUF_REGISTRATION)
/**< PUF kEK generate option it can be either registration/regeneration on demand */
#define XPUF_GLBL_VAR_FLTR_OPTION	(TRUE)
/**< Enables/disables global variation filter during PUF registraton/regeneration */

#if (XPUF_KEY_GENERATE_OPTION == XPUF_REGEN_ON_DEMAND)
#define XPUF_CHASH				(0x00000000U)
/**< PUF CHASH value */
#define XPUF_AUX				(0x00000000U)
/**< PUF AUX value and it is expected to provide as 0x0FFFFFFF0U */
#define XPUF_SYN_DATA_ADDRESS			(0x00000000U)
/**< PUF syndrome address */
#elif (XPUF_KEY_GENERATE_OPTION == XPUF_REGISTRATION)
#define XPUF_WRITE_PUF_HASH_IN_EFUSE 			(FALSE)
/**< Write PUF hash in efuse */
#define XPUF_WRITE_IN_MEM				(FALSE)
/**< This will enable writing PUFHD,CHASH,AUX and black key into the memory */
#endif

#define XPUF_WRITE_BLACK_KEY_OPTION		(FALSE)

/* For programming Secure control eFUSE bits of PUF */
#if (XPUF_WRITE_PUF_HASH_IN_EFUSE ==  TRUE)
#define XPUF_PRGM_HASH_PUF_OR_KEY				(TRUE)
/**< This will enable programming HASH_PUF_OR_KEY efuse when XPUF_WRITE_PUF_HASH_IN_EFUSE is TRUE */
#else
#define XPUF_PRGM_HASH_PUF_OR_KEY				(FALSE)
/**< This will enable/disable programming HASH_PUF_OR_KEY efuse */
#endif

#define PUF_RO_SWAP				(0x00000000U)
/**< PUF RO swap value */

#if (XPUF_WRITE_IN_MEM == TRUE)
#define XPUF_SYNDROME_DATA_WRITE_ADDR           (0x040BF368U)
/**< PUF syndrome data write address */
#define XPUF_CHASH_DATA_WRITE_ADDR              (0x040BF564U)
/**< PUF CHASH data write address */
#define XPUF_AUX_DATA_WRITE_ADDR                (0x040A00D0U)
/**< PUF AUX data write address */
#define XPUF_AES_BLK_KEY_WRITE_ADDR             (0x040A00A0U)
/**< PUF AES black key write address */
#endif

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#ifdef __cplusplus
}
#endif

#endif /* XILPUF_SPARTAN_ULTRASCALE_PLUS_EXAMPLE_H_ */
