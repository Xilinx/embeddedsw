/******************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

 /*****************************************************************************/
 /**
 *
 * @file xasu_ude_keys_enc_input_example.h
 *
 * This example is supported for versal_2ve_2vm devices.It contains macros which needs
 * to be configured by user for xasu_ude_keys_enc_example.c
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
 * This example is provided to generate black key, UDS prime and encrypted UDE
 * private keys by using either PUF registration or PUF on demand regeneration.
 * It can also be used to generate only the PUF ID.
 *
 * Please follow below guide to configure the specific macros as per your requirement:
 *
 * XOCP_ENCRYPT_UDE_PRIV_KEY_0 must be configured as TRUE to encrypt UDE Private key 0.
 *
 * XOCP_ENCRYPT_UDE_PRIV_KEY_1 must be configured as TRUE to encrypt UDE Private key 1.
 *
 * XOCP_ENCRYPT_UDE_PRIV_KEY_2 must be configured as TRUE to encrypt UDE Private key 2.
 *
 * XOCP_ENV_MONITOR_DISABLE - configure as TRUE to disable the temperature
 * and voltage checks before eFuse programming. FALSE will not disable the temperature
 * and voltage checks before eFuse programming. By default the value will be FALSE.
 *
 * XOCP_PRGM_ENC_UDE_PRIV_KEY_0 must be configured as TRUE to program the
 * encrypted UDE Private key 0 into eFuses.
 *
 * XOCP_PRGM_ENC_UDE_PRIV_KEY_1 must be configured as TRUE to program the
 * encrypted UDE Private key 1 into eFuses.
 *
 * XOCP_PRGM_ENC_UDE_PRIV_KEY_2 must be configured as TRUE to program the
 * encrypted UDE Private key 2 into eFuses.
 *
 ******************************************************************************/
#ifndef XASU_UDE_KEYS_ENC_INPUT_EXAMPLE_H_
#define XASU_UDE_KEYS_ENC_INPUT_EXAMPLE_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Constant Definitions *****************************/
#define XOCP_ENCRYPT_UDE_PRIV_KEY_0		(TRUE)	/**< Enable/disable encryption of
													 * UDE Private Key 0 */

#define XOCP_ENCRYPT_UDE_PRIV_KEY_1		(TRUE)	/**< Enable/disable encryption of
													 * UDE Private Key 1 */

#define XOCP_ENCRYPT_UDE_PRIV_KEY_2		(TRUE)	/**< Enable/disable encryption of
													 * UDE Private Key 2 */

#define XOCP_ENV_MONITOR_DISABLE		(FALSE)
/**< Enable/disable environment monitoring during eFuse programming */

#define XOCP_PRGM_ENC_UDE_PRIV_KEY_0		(FALSE)
/**< Enable/disable programming of encrypted UDE Private Key 0 into eFuses */
#define XOCP_PRGM_ENC_UDE_PRIV_KEY_1		(FALSE)
/**< Enable/disable programming of encrypted UDE Private Key 1 into eFuses */
#define XOCP_PRGM_ENC_UDE_PRIV_KEY_2		(FALSE)
/**< Enable/disable programming of encrypted UDE Private Key 2 into eFuses */

/**************************** Type Definitions *******************************/

#ifdef __cplusplus
}
#endif

#endif /* XASU_UDE_KEYS_ENC_INPUT_EXAMPLE_H_ */
