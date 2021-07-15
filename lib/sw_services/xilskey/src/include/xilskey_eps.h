/******************************************************************************
* Copyright (c) 2013 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xilskey_eps.h
 *
 * @addtogroup xilskey_zynq_efuse Zynq EFUSE PS
 * @{
 * @cond xilskey_internal
 * @{
 *
 * @note	None.
 *
 *
 * MODIFICATION HISTORY:
 *
* Ver   Who  	Date     Changes
* ----- ---- 	-------- --------------------------------------------------------
* 1.00a rpoolla 04/26/13 First release
* 1.02a hk      10/28/13 Added API to read status register:
*			 u32 XilSKey_EfusePs_ReadStatus(
*				XilSKey_EPs *InstancePtr, u32 *StatusBits)
*                        RSA key read back is stored in RsaKeyReadback in
*                        Instance structure instead of RsaKeyHashValue -
*			 Change in API:
*			 u32 XilSKey_EfusePs_Read(XilSKey_EPs *PsInstancePtr)
* 2.00  hk      23/01/14 Changed PS efuse error codes for voltage out of range
* 2.1   sk      04/03/15 Initialized RSAKeyReadback with Zeros CR# 829723.
* 4.0   vns     10/20/15 Added cplusplus boundary blocks.
* 6.6   vns     06/06/18 Added doxygen tags
* 6.7   arc     01/05/19 Fixed MISRA-C changes.
* 7.2   am      07/13/21 Fixed doxygen warnings
*
* </pre>
*
*****************************************************************************/

#ifndef XILSKEY_EPS_H
#define XILSKEY_EPS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Key length definition for RSA KEY Hash
 */
#define XSK_EFUSEPS_RSA_KEY_HASH_LEN_IN_BYTES			(32U)

/**
 * XSKEfusePs is the PS eFUSE driver instance. Using this
 * structure, user can define the eFUSE bits to be
 * blown.
 */
typedef struct {
	/**
	 * EnableWriteProtect:Enable the eFUSE Array write protection
	 */
	u32 EnableWriteProtect;
	/**
	 * EnableRsaAuth: Enable the RSA Authentication eFUSE Bit
	 */
	u32 EnableRsaAuth;
	/**
	 * Enable the ROM code 128K crc  eFUSE Bit
	 */
	u32 EnableRom128Crc;
		/**
	 * Disable DFT JTAG
	 */
	u32 DisableDftJtag;
	/**
	 * Disable DFT Mode
	 */
	u32 DisableDftMode;
	/**
	 * EnableRsaKeyHash: Enabling this RsaKeyHashValue[32] is written to
	 * eFUSE array
	 */
	u32 EnableRsaKeyHash;
	/**
	 * RsaKeyHashValue: RSA key Hash
	 */
	u8 RsaKeyHashValue[XSK_EFUSEPS_RSA_KEY_HASH_LEN_IN_BYTES];
	/**
	 * Rsa key read
	 */
	u8 RsaKeyReadback[XSK_EFUSEPS_RSA_KEY_HASH_LEN_IN_BYTES];
} XilSKey_EPs;
/** @}
@endcond */

/**
 * PS eFUSE interface functions
 */
u32 XilSKey_EfusePs_Write(XilSKey_EPs *InstancePtr);
u32 XilSKey_EfusePs_Read (XilSKey_EPs *InstancePtr);
u32 XilSKey_EfusePs_ReadStatus(XilSKey_EPs *InstancePtr, u32 *StatusBits);

#ifdef __cplusplus
}
#endif

#endif /* XILSKEY_EPS_H */
/**@}*/
