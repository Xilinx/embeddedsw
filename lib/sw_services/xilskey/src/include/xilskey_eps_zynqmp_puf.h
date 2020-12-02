/******************************************************************************
* Copyright (c) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xilskey_eps_zynqmp_puf.h
* @addtogroup xilskey_zynqmp_efuse ZynqMP EFUSE PS
* @{
* @cond xilskey_internal
* @{
* Contains the function prototypes, defines and macros for ZynqMP efusePs puf
* functionality.
*
* @note	None.
*
* </pre>
* MODIFICATION HISTORY:
*
* Ver   Who   Date     Changes
* ----- ---- -------- --------------------------------------------------------
* 6.1   vns  17/10/16 First release.
* 6.6   vns  06/06/18 Added doxygen tags
* 6.7	arc  01/05/19 Fixed MISRA-C violations.
*       mmd  03/17/19 Added PUF syndrome data length in bytes for 4K mode
* 6.9   kpt  02/27/20 Removed prototype XilSKey_Puf_Debug2
* 7.0 	am	 10/04/20 Resolved MISRA C violations
*
* </pre>
*
*****************************************************************************/
#ifndef XSK_EPS_ZYNQMP_PUF_H
#define XSK_EPS_ZYNQMP_PUF_H


#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xilskey_eps_zynqmp.h"
#include "xilskey_utils.h"

/************************** Constant Definitions *****************************/
#if defined XSK_PUF_DEBUG
#define XSK_PUF_DEBUG_GENERAL (1U)
#else
#define XSK_PUF_DEBUG_GENERAL (0U)
#endif

#define		xPuf_printf(type,...) if ((type) == (1U)) \
				{xil_printf (__VA_ARGS__);}

#define		XSK_ZYNQMP_PUF_MODE4K			(0U)
#define		XSK_ZYNQMP_PUF_SYN_LEN			(386U)

#define		XSK_ZYNQMP_MAX_RAW_4K_PUF_SYN_LEN	(140U)
						/* In bytes */

#define		XSK_ZYNQMP_PUF_REGISTRATION		(1U)
#define 	XSK_ZYNQMP_PUF_REGENERATION		(4U)

#define 	XSK_ZYNQMP_PUF_CFG0_INIT_VAL		(2U)
#define 	XSK_ZYNQMP_PUF_CFG1_INIT_VAL_4K		(0x0c230090U)

#define		XSK_ZYNQMP_EFUSEPS_PUF_TOTAL_ROWS	(128U)
#define		XSK_ZYNQMP_PUF_SYN_DATA_LEN_IN_BYTES	(386U)
#define		XSK_ZYNQMP_PUF_FORMATTED_SYN_DATA_LEN_IN_BYTES	(140U)
#define		XSK_ZYNQMP_PUF_DBG2_DATA_LEN_IN_BYTES	(36U)
#define		XSK_ZYNQMP_PUF_KEY_IV_LEN_IN_BYTES	(12U)
#define		XSK_ZYNQMP_PUF_AUX_LEN_IN_BITS		(24U)
#define		XSK_ZYNQMP_PUF_SHUTTER_VALUE		(0x0100005eU)
#define 	XSK_ZYNQMP_GCM_TAG_SIZE				(16U)

/************************** Type Definitions ********************************/


/** @name xilinx eFUSE PUF secure bits
 * @{
 */
typedef enum {
	XSK_ZYNQMP_EFUSEPS_PUF_RESERVED = 28,
	XSK_ZYNQMP_EFUSEPS_PUF_SYN_INVALID,
	XSK_ZYNQMP_EFUSEPS_PUF_SYN_LOCK,
	XSK_ZYNQMP_EFUSEPS_PUF_REG_DIS
}XskEfusePS_Puf_SecureBits;
/*@}*/

/** @name contains secure bits of efuse PUF
 * @{
 */
typedef struct {
	u8 SynInvalid;
	u8 SynWrLk;
	u8 RegisterDis;
	u8 Reserved;
}XilSKey_Puf_Secure;
/*@}*/

/** @name PUF instance
 * @{
 */
typedef struct {
	u8 RegistrationMode;	/**< PUF Registration Mode: Always 4K Mode */
	u32 ShutterValue;	/**< PUF Shutter value */
	u32 SyndromeData[XSK_ZYNQMP_PUF_SYN_DATA_LEN_IN_BYTES];
				/**< Helper data */
	u32 EfuseSynData[XSK_ZYNQMP_PUF_FORMATTED_SYN_DATA_LEN_IN_BYTES];
				/**< Formatted Syndrome data */
	u32 Debug2Data[XSK_ZYNQMP_PUF_DBG2_DATA_LEN_IN_BYTES];
				/**< Debug 2 Data */
	u32 Chash;		/**< CHASH Value */
	u32 Aux;		/**< AUX Value */
	u8 RedKey[XSK_ZYNQMP_EFUSEPS_AES_KEY_LEN_IN_BYTES];
				/**< Red Key */
	u8 BlackKeyIV[XSK_ZYNQMP_PUF_KEY_IV_LEN_IN_BYTES];
				/**< Black key IV (IV used to encrypt the
				  *  red key using PUF key) */
	u8 BlackKey[XSK_ZYNQMP_EFUSEPS_AES_KEY_LEN_IN_BYTES +
				XSK_ZYNQMP_GCM_TAG_SIZE];
				/**< Black Key generated */
} XilSKey_Puf;
/** @}
@endcond */
/****************************Prototypes***************************************/
u32 XilSKey_ZynqMp_EfusePs_WritePufHelprData(const XilSKey_Puf *InstancePtr);
u32 XilSKey_ZynqMp_EfusePs_ReadPufHelprData(u32 *Address);

u32 XilSKey_ZynqMp_EfusePs_WritePufChash(const XilSKey_Puf *InstancePtr);
u32 XilSKey_ZynqMp_EfusePs_ReadPufChash(u32 *Address, u8 ReadOption);

u32 XilSKey_ZynqMp_EfusePs_WritePufAux(const XilSKey_Puf *InstancePtr);
u32 XilSKey_ZynqMp_EfusePs_ReadPufAux(u32 *Address, u8 ReadOption);

u32 XilSKey_Write_Puf_EfusePs_SecureBits(
		const XilSKey_Puf_Secure *WriteSecureBits);
u32 XilSKey_Read_Puf_EfusePs_SecureBits(
		XilSKey_Puf_Secure *SecureBitsRead, u8 ReadOption);

u32 XilSKey_Puf_Registration(XilSKey_Puf *InstancePtr);

u32 XilSKey_Puf_Regeneration(const XilSKey_Puf *InstancePtr);
#ifdef __cplusplus
}
#endif

#endif /* XSK_EPS_ZYNQMP_PUF_H */
/*@}*/