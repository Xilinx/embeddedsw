/******************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

 /*****************************************************************************/
 /**
 *
 * @file xasu_dme_keys_enc_input_example.h.
 *
 * This example is supported for versal_2ve_2vm devices.It contains macros which needs
 * to be configured by user for xasu_dme_keys_enc_example.c
 * On the basis of the options selected by user, the required operations will be performed.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who     Date     Changes
 * ----- ------  -------- ------------------------------------------------------
 * 1.0   yog     08/20/25 First release
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
 * #define XOCP_ENCRYPT_DME_PRIV_KEY_0			(FALSE)
 * This option must be configured as TRUE to encrypt DME Private key 0.
 *
  #define XOCP_DME_PRIV_KEY_0
 * 		"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
 * DME Private Key 0 to be encrypted by DME KEK should be provided in string format. It
 * should be 96 characters long.
 *
 * #define XOCP_ENCRYPT_DME_PRIV_KEY_1			(FALSE)
 * This option must be configured as TRUE to encrypt DME Private key 1.
 *
  #define XOCP_DME_PRIV_KEY_1
 * 		"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
 * DME Private Key 1 to be encrypted by DME KEK should be provided in string format. It
 * should be 96 characters long.
 * #define XOCP_ENCRYPT_DME_PRIV_KEY_2			(FALSE)
 * This option must be configured as TRUE to encrypt DME Private key 2.
 *
 * #define XOCP_DME_PRIV_KEY_2
 * 		"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
 * DME Private Key 2 to be encrypted by DME KEK should be provided in string format. It
 * should be 96 characters long.
 *
 * #define XOCP_ENV_MONITOR_DISABLE		(FALSE)
 *
 * TRUE will disable the temparature and voltage checks before eFuse programming.
 * FALSE will not disable the temparature and voltage checks before eFuse programming.
 * By default the value will be FALSE.
 *
 * #define XOCP_PRGM_ENC_DME_PRIV_KEY_0		(FALSE)
 * If set as TRUE, the encrypted DME private key 0 will be programmed into eFuses.
 * By default the value is set to FALSE
 *
 * #define XOCP_PRGM_ENC_DME_PRIV_KEY_1		(FALSE)
 * If set as TRUE, the encrypted DME private key 1 will be programmed into eFuses.
 * By default the value is set to FALSE
 *
 * #define XOCP_PRGM_ENC_DME_PRIV_KEY_2		(FALSE)
 * If set as TRUE, the encrypted DME private key 2 will be programmed into eFuses.
 * By default the value is set to FALSE
 *
 ******************************************************************************/
#ifndef XASU_DME_KEYS_ENC_INPUT_EXAMPLE_H_
#define XASU_DME_KEYS_ENC_INPUT_EXAMPLE_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Constant Definitions *****************************/
#define XOCP_ENCRYPT_DME_PRIV_KEY_0		(FALSE)
#define XOCP_DME_PRIV_KEY_0	\
	"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"

#define XOCP_ENCRYPT_DME_PRIV_KEY_1		(FALSE)
#define XOCP_DME_PRIV_KEY_1	\
	"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"

#define XOCP_ENCRYPT_DME_PRIV_KEY_2		(FALSE)
#define XOCP_DME_PRIV_KEY_2	\
	"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"

#define XOCP_ENV_MONITOR_DISABLE		(FALSE)

#define XOCP_PRGM_ENC_DME_PRIV_KEY_0		(FALSE)
#define XOCP_PRGM_ENC_DME_PRIV_KEY_1		(FALSE)
#define XOCP_PRGM_ENC_DME_PRIV_KEY_2		(FALSE)

/**************************** Type Definitions *******************************/

#ifdef __cplusplus
}
#endif

#endif /* XASU_DME_KEYS_ENC_INPUT_EXAMPLE_H_ */
