/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

 /*****************************************************************************/
 /**
 *
 * @file xilpuf_example.h.
 * This file contains macros which needs to configured by user for
 * xilpuf_example.cand based on the options selected by user operations will be
 * performed.
 *
 * This example is supported for Versal and Versal Net devices.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who     Date     Changes
 * ----- ------  -------- ------------------------------------------------------
 * 1.0   har     01/31/20 First release
 * 1.1   har     01/31/20 Updated file version to 1.1 to sync with library version
 * 1.2   am      08/14/20 Changed unsigned to signed enum object.
 * 1.3   har     05/20/21 Added option to program Black IV
 * 2.1   skg     12/14/22 Added enum for slr indexs
 * 2.5   hj      02/17/25 Added IV and red key macros for SSITs.
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
 * #define XPUF_RED_KEY0   \
 * 	"0000000000000000000000000000000000000000000000000000000000000000"
 * SLR 0 Red Key to be encrypted by PUF KEY should be provided in string format.It
 * should be either 32 or 64 characters long as only 128 bit and 256 bit key
 * are supported.
 *
 * #define XPUF_RED_KEY1   \
 *	"0000000000000000000000000000000000000000000000000000000000000000"
 * SLR 1 Red Key to be encrypted by PUF KEY should be provided in string format.It
 * should be either 32 or 64 characters long as only 128 bit and 256 bit key
 * are supported.
 *
 * #define XPUF_RED_KEY2   \
 *	"0000000000000000000000000000000000000000000000000000000000000000"
 * SLR 2 Red Key to be encrypted by PUF KEY should be provided in string format.It
 * should be either 32 or 64 characters long as only 128 bit and 256 bit key
 * are supported.
 *
 * #define XPUF_RED_KEY3   \
 *      "0000000000000000000000000000000000000000000000000000000000000000"
 * SLR 3 Red Key to be encrypted by PUF KEY should be provided in string format.It
 * should be either 32 or 64 characters long as only 128 bit and 256 bit key
 * are supported.
 *
 * #define XPUF_IV				"000000000000000000000000"
 * IV should be provided in string format.It should be 24 characters long, valid
 * characters are 0-9,a-f,A-F. Any other character is considered as invalid
 * string.The value mentioned here will be converted to hex buffer.It is used
 * with the AES-GCM cryptographic hardware in order to encrypt red key.
 *
 * #define XPUF_IV0                                "000000000000000000000000"
 * IV should be provided in string format.It should be 24 characters long, valid
 * characters are 0-9,a-f,A-F. Any other character is considered as invalid
 * string.The value mentioned here will be converted to hex buffer.It is used
 * with the AES-GCM cryptographic hardware in order to encrypt red key of SLR 0.
 *
 * #define XPUF_IV1                                "000000000000000000000000"
 * IV should be provided in string format.It should be 24 characters long, valid
 * characters are 0-9,a-f,A-F. Any other character is considered as invalid
 * string.The value mentioned here will be converted to hex buffer.It is used
 * with the AES-GCM cryptographic hardware in order to encrypt red key of SLR 1.
 *
 * #define XPUF_IV2                                "000000000000000000000000"
 * IV should be provided in string format.It should be 24 characters long, valid
 * characters are 0-9,a-f,A-F. Any other character is considered as invalid
 * string.The value mentioned here will be converted to hex buffer.It is used
 * with the AES-GCM cryptographic hardware in order to encrypt red key of SLR 2.
 *
 * #define XPUF_IV3                                "000000000000000000000000"
 * IV should be provided in string format.It should be 24 characters long, valid
 * characters are 0-9,a-f,A-F. Any other character is considered as invalid
 * string.The value mentioned here will be converted to hex buffer.It is used
 * with the AES-GCM cryptographic hardware in order to encrypt red key of SLR 3.
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
 * #define XPUF_READ_HD_OPTION			(XPUF_READ_FROM_RAM)
 *							(or)
 *						(XPUF_READ_FROM_EFUSE_CACHE)
 * This selects the location from where the helper data must be read by the
 * application. This option must be configured if XPUF_KEY_GENERATE_OPTION
 * is configured as XPUF_REGEN_ON_DEMAND.
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
 * #define XPUF_WRITE_HD_IN_EFUSE		(FALSE)
 * If this option is configured as TRUE then PUF helper data is written into the
 * eFUSE.
 *
 * #define XPUF_WRITE_BLACK_KEY_OPTION		(FALSE)
 *							(or)
 *						(XPUF_EFUSE_AES_KEY_N_IV)
 *							(or)
 *						(XPUF_BBRAM_AES_KEY)
 *							(or)
 *						(XPUF_EFUSE_USER_0_KEY)
 *							(or)
 *						(XPUF_EFUSE_USER_1_KEY)
 * This selects the location where the Black key must be programmed. Please note that
 * If this option is configured as XPUF_EFUSE_AES_KEY_N_IV then Black key will
 * be programmed in eFUSE AES key and Black IV is programmed in efuses.
 * If it is configured as XPUF_BBRAM_AES_KEY, XPUF_EFUSE_USER_0_KEY or
 * XPUF_EFUSE_USER_1_KEY then the Black key is programmed in the corresponding location
 * but Black IV is not programmed.

 * #define XPUF_READ_SEC_CTRL_BITS		(FALSE)
 * This option should be configured as TRUE to read secure control eFUSE bits
 * of PUF
 *
 * #define XPUF__SEC_CTRL_BITS		(FALSE)
 * This option should be configured as TRUE to program secure control eFUSE bits
 * of PUF
 *
 * #define PUF_DIS				(FALSE)
 * This option should be configured as TRUE to program the PUF_DIS eFUSE bit
 *
 * #define PUF_REGEN_DIS			(FALSE)
 * This option should be configured as TRUE to program the PUF_REGEN_DIS eFUSE bit
 *
 * #define PUF_HD_INVLD				(FALSE)
 * This option should be configured as TRUE to program the PUF_HD_INVLD eFUSE bit
 *
 * #define PUF_SYN_LK				(FALSE)
 * This option should be configured as TRUE to program the PUF_SYN_LK eFUSE bit
 *
 * #define XPUF_GLBL_VAR_FLTR_OPTION	(TRUE)
 * It is recommended to always enable this option to ensure entropy. It can
 * be configured as FALSE to disable Global Variation Filter.
 *
 * #define XPUF_ENV_MONITOR_DISABLE		(FALSE)
 * This option should be configured as TRUE to disable the temparature and voltage
 * checks before eFuses programming.
 *
 * #define SLR_INDEX	   (XPUF_SLR_INDEX_0)
 *							(or)
 *						(XPUF_SLR_INDEX_1)
 *							(or)
 *						(XPUF_SLR_INDEX_2)
 *							(or)
 *						(XPUF_SLR_INDEX_3)
 * This option selects the Slave Slr for provisioning.
 *
 ******************************************************************************/
#ifndef XILPUF_EXAMPLE_H
#define XILPUF_EXAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"

/************************** Constant Definitions *****************************/
/* For reading Secure control eFUSE bits of PUF */
#define XPUF_READ_SEC_CTRL_BITS			(FALSE)

#define XPUF_RED_KEY	\
	"0000000000000000000000000000000000000000000000000000000000000000"

/*
 * Below macro values should match with enum XSecure_AesKeySize.
 * As preprocessor can't handle enums at preprocessing stage of compilation,
 * these macros are defined.
 */
#define XPUF_RED_KEY_SIZE_128		(0U)
#define XPUF_RED_KEY_SIZE_256		(2U)

#define XPUF_RED_KEY_LEN			(XPUF_RED_KEY_SIZE_256)

#if (XPUF_RED_KEY_LEN == XPUF_RED_KEY_SIZE_256)
#define XPUF_RED_KEY_LEN_IN_BYTES 		(32U)
#elif (XPUF_RED_KEY_LEN == XPUF_RED_KEY_SIZE_128)
#define XPUF_RED_KEY_LEN_IN_BYTES 		(16U)
#endif

#define XPUF_IV					"000000000000000000000000"

#define XPUF_GENERATE_KEK_N_ID			(TRUE)
#define XPUF_KEY_GENERATE_OPTION		(XPUF_REGISTRATION)
#define XPUF_GLBL_VAR_FLTR_OPTION	(TRUE)

#if (XPUF_KEY_GENERATE_OPTION == XPUF_REGEN_ON_DEMAND)
#define XPUF_READ_HD_OPTION			(XPUF_READ_FROM_RAM)
#define XPUF_CHASH				(0x00000000U)
#define XPUF_AUX				(0x00000000U)
#define XPUF_SYN_DATA_ADDRESS			(0x00000000U)
#elif (XPUF_KEY_GENERATE_OPTION == XPUF_REGISTRATION)
#define XPUF_WRITE_HD_IN_EFUSE 			(FALSE)
#endif

#define XPUF_WRITE_BLACK_KEY_OPTION		(FALSE)

/**< Input SlrIndex*/
#define SLR_INDEX     XPUF_SLR_INDEX_0

/* Iv and Keys for all ssit devices */
#define XPUF_IV0                                "000000000000000000000000"
#define XPUF_IV1                                "000000000000000000000000"
#define XPUF_IV2                                "000000000000000000000000"
#define XPUF_IV3                                "000000000000000000000000"
#define XPUF_RED_KEY0   \
	"0000000000000000000000000000000000000000000000000000000000000000"
#define XPUF_RED_KEY1   \
	"0000000000000000000000000000000000000000000000000000000000000000"
#define XPUF_RED_KEY2   \
	"0000000000000000000000000000000000000000000000000000000000000000"
#define XPUF_RED_KEY3   \
        "0000000000000000000000000000000000000000000000000000000000000000"

/* For programming Secure control eFUSE bits of PUF */
#define XPUF_WRITE_SEC_CTRL_BITS			(FALSE)
#if (XPUF_WRITE_SEC_CTRL_BITS == TRUE)
#define PUF_DIS					(FALSE)
#define PUF_REGEN_DIS				(FALSE)
#define PUF_HD_INVLD				(FALSE)
#define PUF_SYN_LK				(FALSE)
#if defined (VERSAL_NET)
#define PUF_REGIS_DIS				(FALSE)
#endif
#endif

#if defined (VERSAL_NET)
#define PUF_RO_SWAP				(0x00000000)
#endif

#define XPUF_ENV_MONITOR_DISABLE          	(FALSE)

/**************************** Type Definitions *******************************/
typedef enum {
	XPUF_EFUSE_AES_KEY_N_IV = 1,
	XPUF_BBRAM_AES_KEY,
	XPUF_EFUSE_USER_0_KEY,
	XPUF_EFUSE_USER_1_KEY
} XPuf_WriteBlackKeyOption;

/***************** Macros (Inline Functions) Definitions *********************/

#ifdef __cplusplus
}
#endif

#endif /* XILPUF_EXAMPLE_H_ */
