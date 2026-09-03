/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

 /*****************************************************************************/
 /**
 *
 * @file xasu_provision_ude_input_example.h
 *
 * This example is supported for versal_2ve_2vm devices. It contains macros which needs
 * to be configured by user for xasu_provision_ude_example.c
 * On the basis of the options selected by user, the required operations will be performed.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who     Date     Changes
 * ----- ------  -------- ------------------------------------------------------
 * 1.0   yog      08/25/26 First release
 *
 * </pre>
 *
 * GUIDE for using example
 *------------------------------------------------------------------------------
 * This example provisions Unique Device Endorsement (UDE) key pairs into the UDE
 * eFUSE slots. For every enabled slot the UDE key pair is generated inside the ASU
 * (the private key never leaves the ASU), the private key is encrypted with the
 * PUF KEK and programmed into the requested eFUSE slot, and a self-signed X.509
 * certificate over the generated public key is returned and printed on the console.
 *
 * Please follow below guide to configure the specific macros as per your requirement:
 *
 * XASU_PROVISION_UDE_KEY_0 must be configured as TRUE to provision UDE key into slot 0.
 *
 * XASU_PROVISION_UDE_KEY_1 must be configured as TRUE to provision UDE key into slot 1.
 *
 * XASU_PROVISION_UDE_KEY_2 must be configured as TRUE to provision UDE key into slot 2.
 *
 * XASU_PROVISION_UDE_KEY_TYPE selects the UDE key type to provision. Currently only
 * XASU_PROVISION_UDE_KEY_TYPE_ECC_P384 is supported.
 *
 ******************************************************************************/
#ifndef XASU_PROVISION_UDE_INPUT_EXAMPLE_H_
#define XASU_PROVISION_UDE_INPUT_EXAMPLE_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xasu_provision.h"

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Constant Definitions *****************************/
#define XASU_PROVISION_UDE_KEY_0		(FALSE)	/**< Enable/disable provisioning of
													 * UDE key into slot 0 */

#define XASU_PROVISION_UDE_KEY_1		(FALSE)	/**< Enable/disable provisioning of
													 * UDE key into slot 1 */

#define XASU_PROVISION_UDE_KEY_2		(FALSE)	/**< Enable/disable provisioning of
													 * UDE key into slot 2 */

#define XASU_PROVISION_UDE_KEY_TYPE		(XASU_PROVISION_UDE_KEY_TYPE_ECC_P384)
/**< UDE key type to provision into the enabled slots */

/**************************** Type Definitions *******************************/

#ifdef __cplusplus
}
#endif

#endif /* XASU_PROVISION_UDE_INPUT_EXAMPLE_H_ */
