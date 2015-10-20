/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file xilskey_eps_zynqmp.h
* Contains the function prototypes, defines and macros for ZynqMP efusePs
* functionality.
*
* @note	None.
*
* </pre>
* MODIFICATION HISTORY:
*
* Ver   Who  	Date     Changes
* ----- ---- 	-------- --------------------------------------------------------
* 4.0   vns     10/01/15 First release
*       vns     10/20/15 Modified secure control bits readback bits.
* </pre>
*
*****************************************************************************/

#ifndef XILSKEY_EPS_ZYNQMP_H
#define XILSKEY_EPS_ZYNQMP_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xilskey_utils.h"

/************************** Constant Definitions *****************************/

/* Key length definitions for ZynqMP eFuse */
#define XSK_ZYNQMP_EFUSEPS_AES_KEY_LEN_IN_BYTES			(32)
#define XSK_ZYNQMP_EFUSEPS_USER_KEY_LEN_IN_BYTES		(32)
#define XSK_ZYNQMP_EFUSEPS_PPK_HASH_LEN_IN_BYTES		(48)
#define XSK_ZYNQMP_EFUSEPS_SPKID_LEN_IN_BYTES			(4)
#define XSK_ZYNQMP_EFUSEPS_JTAG_USER_CODE_LEN_IN_BYTES		(4)
#define XSK_ZYNQMP_EFUSEPS_DNA_LEN_IN_BYTES			(12)

/* ZynqMP eFuse PS keys lengths in bits */
#define XSK_ZYNQMP_EFUSEPS_AES_KEY_LEN_IN_BITS			(256)
#define XSK_ZYNQMP_EFUSEPS_USER_KEY_LEN_IN_BITS			(256)
#define XSK_ZYNQMP_EFUSEPS_PPK_SHA3HASH_LEN_IN_BITS		(384)
#define XSK_ZYNQMP_EFUSEPS_PPK_SHA2HASH_LEN_IN_BITS		(256)
#define XSK_ZYNQMP_EFUSEPS_SPKID_LEN_IN_BITS			(32)
#define XSK_ZYNQMP_EFUSEPS_USR_CODE_LEN_IN_BITS			(32)

/* ZynqMP eFuse maximum bits in a row */
#define XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW			(32)

/* No of Registers allocated for PPK sha3 hash */
#define XSK_ZYNQMP_EFUSEPS_PPK_HASH_REG_NUM			(12)
#define XSK_ZYNQMP_EFUSEPS_USR_KEY_REG_NUM			(8)

/* Row numbers of Efuse PS of Zynq MP */
#define XSK_ZYNQMP_EFUSEPS_JTAG_USERCODE_ROW			(7)
#define XSK_ZYNQMP_EFUSEPS_USR_KEY_START_ROW			(8)
#define XSK_ZYNQMP_EFUSEPS_USR_KEY_END_ROW			(15)
#define XSK_ZYNQMP_EFUSEPS_MISC_USR_CTRL_ROW			(16)
#define XSK_ZYNQMP_EFUSEPS_SEC_CTRL_ROW				(22)
#define XSK_ZYNQMP_EFUSEPS_SPK_ID_ROW				(23)
#define XSK_ZYNQMP_EFUSEPS_AES_KEY_START_ROW			(24)
#define XSK_ZYNQMP_EFUSEPS_AES_KEY_END_ROW			(31)

#define XSK_ZYNQMP_EFUSEPS_PPK0_START_ROW			(40)
#define XSK_ZYNQMP_EFUSEPS_PPK0_SHA2_HASH_END_ROW		(47)
#define XSK_ZYNQMP_EFUSEPS_PPK0_SHA3_HASH_END_ROW		(51)

#define XSK_ZYNQMP_EFUSEPS_PPK1_START_ROW			(52)
#define XSK_ZYNQMP_EFUSEPS_PPK1_SHA2_HASH_END_ROW		(59)
#define XSK_ZYNQMP_EFUSEPS_PPK1_SHA3_HASH_END_ROW		(63)

#define XSK_ZYNQMP_EFUSEPS_TBITS_ROW				(0)
#define XSK_ZYNQMP_EFUSEPS_XILINX_SPECIFIC_CTRL_BITS_ROW	(21)

#define XSK_ZYNQMP_EFUSEPS_TBITS_MASK				(0xF)
#define XSK_ZYNQMP_EFUSEPS_TBITS_SHIFT				(28)

#define XSK_ZYNQMP_EFUSEPS_CRC_AES_ZEROS		(0x6858A3D5)

/* Timer related macros */
#define XilSKey_ZynqMp_EfusePs_Tprgrm(RefClk) \
	XilSKey_Ceil(((float)5 * RefClk) / (float)1000000)
#define XilSKey_ZynqMp_EfusePs_Trd(RefClk) \
	XilSKey_Ceil(((float)15 * RefClk) / (float)100000000)
#define XilSKey_ZynqMp_EfusePs_TsuHPs(RefClk) \
	XilSKey_Ceil(((float)67 * RefClk) / (float)1000000000)
#define XilSKey_ZynqMp_EfusePs_TsuHPsCs(RefClk) \
	XilSKey_Ceil(((float)46 * RefClk) / (float)1000000000)
#define XilSKey_ZynqMp_EfusePs_TsuHCs(RefClk) \
	XilSKey_Ceil(((float)30 * RefClk) / (float)1000000000)

#define BOTH_BITS_SET					0x3
								/**< If both bits are set */
#define XSK_ZYNQMP_EFUSEPS_SECTRL_BIT_SHIFT	0x1
								/**< Shift macro for SEC_CTRL
								 *   if it has 2 bits */

/** @name efuse types
 * @{
 */
typedef enum {
	XSK_ZYNQMP_EFUSEPS_EFUSE_0,
	XSK_ZYNQMP_EFUSEPS_EFUSE_2 = 2,
	XSK_ZYNQMP_EFUSEPS_EFUSE_3 = 3
}XskEfusePs_Type;
/*@}*/

/** @name efuse secure control bits
 * @{
 */
typedef enum {
	XSK_ZYNQMP_EFUSEPS_SEC_AES_RDLK,
	XSK_ZYNQMP_EFUSEPS_SEC_AES_WRLK,
	XSK_ZYNQMP_EFUSEPS_SEC_ENC_ONLY,
	XSK_ZYNQMP_EFUSEPS_SEC_BRAM_DIS,
	XSK_ZYNQMP_EFUSEPS_SEC_ERR_DIS,
	XSK_ZYNQMP_EFUSEPS_SEC_JTAG_DIS,
	XSK_ZYNQMP_EFUSEPS_SEC_DFT_DIS,
	XSK_ZYNQMP_EFUSEPS_SEC_DIS_PROG_GATE0,
	XSK_ZYNQMP_EFUSEPS_SEC_DIS_PROG_GATE1,
	XSK_ZYNQMP_EFUSEPS_SEC_DIS_PROG_GATE2,
	XSK_ZYNQMP_EFUSEPS_SEC_LOCK,
	XSK_ZYNQMP_EFUSEPS_SEC_RSA_EN_BIT1 = 24,
	XSK_ZYNQMP_EFUSEPS_SEC_RSA_EN_BIT2,
	XSK_ZYNQMP_EFUSEPS_SEC_PPK0_WRLK,
	XSK_ZYNQMP_EFUSEPS_SEC_PPK0_INVLD_BIT1,
	XSK_ZYNQMP_EFUSEPS_SEC_PPK0_INVLD_BIT2,
	XSK_ZYNQMP_EFUSEPS_SEC_PPK1_WRLK,
	XSK_ZYNQMP_EFUSEPS_SEC_PPK1_INVLD_BIT1,
	XSK_ZYNQMP_EFUSEPS_SEC_PPK1_INVLD_BIT2
}XskEfusePS_SecCtrlBits;
/*@}*/

/** @name efuse misc user control bits
 * @{
 */
typedef enum {
	XSK_ZYNQMP_EFUSEPS_USR_WRLK_0,
	XSK_ZYNQMP_EFUSEPS_USR_WRLK_1,
	XSK_ZYNQMP_EFUSEPS_USR_WRLK_2,
	XSK_ZYNQMP_EFUSEPS_USR_WRLK_3,
	XSK_ZYNQMP_EFUSEPS_USR_WRLK_4,
	XSK_ZYNQMP_EFUSEPS_USR_WRLK_5,
	XSK_ZYNQMP_EFUSEPS_USR_WRLK_6,
	XSK_ZYNQMP_EFUSEPS_USR_WRLK_7,
}XskEfusePS_MiscUserBits;
/*@}*/

/** @name xilinx specific bits
 * @{
 */
typedef enum {
	XSK_ZYNQMP_EFUSEPS_XILINX_SPECFC_BIT1 = 28,
	XSK_ZYNQMP_EFUSEPS_XILINX_SPECFC_BIT2,
	XSK_ZYNQMP_EFUSEPS_XILINX_SPECFC_BIT3,
	XSK_ZYNQMP_EFUSEPS_XILINX_SPECFC_BIT4
}XskEfusePS_XilinxSpecificBits;
/*@}*/

/**
*
* This typedef contains secure control features of efusePs
*/
typedef struct {
	/* Secure and control bits */
	u8 AesKeyRead;
	u8 AesKeyWrite;
	u8 UseAESOnly;
	u8 BbramDisable;
	u8 PMUError;
	u8 JtagDisable;
	u8 DFTDisable;
	u8 ProgGate0;
	u8 ProgGate1;
	u8 ProgGate2;
	u8 SecureLock;
	u8 RSAEnable;
	u8 PPK0WrLock;
	u8 PPK0Revoke;
	u8 PPK1WrLock;
	u8 PPK1Revoke;

	/* User control bits */
	u8 UserWrLk0;
	u8 UserWrLk1;
	u8 UserWrLk2;
	u8 UserWrLk3;
	u8 UserWrLk4;
	u8 UserWrLk5;
	u8 UserWrLk6;
	u8 UserWrLk7;

	/* Xilinx specific bits */
	u8 XilinxSpecfBit1;
	u8 XilinxSpecfBit2;
	u8 XilinxSpecfBit3;
	u8 XilinxSpecfBit4;

} XilSKey_SecCtrlBits;
/*@}*/

/**
 * XilSKey_ZynqMpEPs is the PS eFUSE driver instance. Using this
 * structure, user can define the eFUSE bits of Zynq MP ultrascale to be
 * blown.
 */
typedef struct {

	XilSKey_SecCtrlBits PrgrmgSecCtrlBits;
	/* For writing into eFuse */
	u8 PrgrmAesKey;
	u8 PrgrmUserKey;
	u8 PrgrmPpk0Hash;
	u8 PrgrmPpk1Hash;
	u8 PrgrmSpkID;
	u8 PrgrmJtagUserCode;

	u8 IsPpk0Sha3Hash;
	u8 IsPpk1Sha3Hash;

	u8 AESKey[XSK_ZYNQMP_EFUSEPS_AES_KEY_LEN_IN_BYTES];
	u8 UserKey[XSK_ZYNQMP_EFUSEPS_USER_KEY_LEN_IN_BYTES];
	u8 Ppk0Hash[XSK_ZYNQMP_EFUSEPS_PPK_HASH_LEN_IN_BYTES];
	u8 Ppk1Hash[XSK_ZYNQMP_EFUSEPS_PPK_HASH_LEN_IN_BYTES];
	u8 SpkId[XSK_ZYNQMP_EFUSEPS_SPKID_LEN_IN_BYTES];
	u8 JtagUserCode[XSK_ZYNQMP_EFUSEPS_JTAG_USER_CODE_LEN_IN_BYTES];

	XilSKey_SecCtrlBits ReadBackSecCtrlBits;

	u8 IntialisedTimer;

}XilSKey_ZynqMpEPs;

/***************** Macros (Inline Functions) Definitions *******************/
/***************************************************************************/
/**
* This macro is used to Unlock the eFuse controller.
*
* @return	None
*
* @note		None
*
****************************************************************************/
#define XilSKey_ZynqMp_EfusePs_CtrlrUnLock() \
	(XilSKey_WriteReg(XSK_ZYNQMP_EFUSEPS_BASEADDR, \
		XSK_ZYNQMP_EFUSEPS_WR_LOCK_OFFSET, \
			XSK_ZYNQMO_EFUSEP_WR_UNLOCK_VALUE))

/***************************************************************************/
/**
* This macro is used to Lock the eFuse controller.
*
* @return	None
*
* @note		None
*
****************************************************************************/
#define XilSKey_ZynqMp_EfusePs_CtrlrLock() \
	XilSKey_WriteReg(XSK_ZYNQMP_EFUSEPS_BASEADDR, \
		XSK_ZYNQMP_EFUSEPS_WR_LOCK_OFFSET, \
			XSK_ZYNQMP_EFUSEPS_WR_LOCK_RSTVAL)

/***************************************************************************/
/**
* This macro is used to tell the lock status of eFuse controller.
*
* @return
*		- TRUE if controller is locked
*		- FALSE if controller is Unlocked
*
* @note		None
*
****************************************************************************/
#define XilSKey_ZynqMp_EfusePs_CtrlrLockStatus() \
	(XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR, \
		XSK_ZYNQMP_EFUSEPS_WR_LOCK_OFFSET) ? 1 : 0)

/***************************************************************************/
/**
* This macro is used to tells the status of eFuse controller.
*
* @return
*		- TRUE if controller is locked
*		- FALSE if controller is Unlocked
*
* @note		None
*
****************************************************************************/
#define XilSKey_ZynqMp_EfusePs_Status() \
	(XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR, \
		XSK_ZYNQMP_EFUSEPS_STS_OFFSET))

/***************************************************************************/
/**
* This macro enables the programming of efuse
*
* @return	None
*
* @note		None
*
****************************************************************************/
#define XilSKey_ZynqMp_EfusePS_PrgrmEn() \
	(XilSKey_WriteReg(XSK_ZYNQMP_EFUSEPS_BASEADDR, \
		XSK_ZYNQMP_EFUSEPS_CFG_OFFSET, \
	(XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR, \
	XSK_ZYNQMP_EFUSEPS_CFG_OFFSET) | XSK_ZYNQMP_EFUSEPS_CFG_PGM_EN_MASK)))

/***************************************************************************/
/**
* This macro disables programming of efuse
*
* @return	None.
*
* @note		None
*
****************************************************************************/
#define XilSKey_ZynqMp_EfusePS_PrgrmDisable() \
	(XilSKey_WriteReg(XSK_ZYNQMP_EFUSEPS_BASEADDR, \
		XSK_ZYNQMP_EFUSEPS_CFG_OFFSET, \
	(XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR, \
		XSK_ZYNQMP_EFUSEPS_CFG_OFFSET) & \
		(~XSK_ZYNQMP_EFUSEPS_CFG_PGM_EN_MASK))))

/****************************Prototypes***************************************/
/* Ps eFuse interface functions of Zynq MP */
u32 XilSKey_ZynqMp_EfusePs_CheckAesKeyCrc(u32 CrcValue);
u32 XilSKey_ZynqMp_EfusePs_ReadUserKey(u32 *UseKeyPtr, u8 ReadOption);
u32 XilSKey_ZynqMp_EfusePs_ReadPpk0Hash(u32 *Ppk0Hash, u8 ReadOption);
u32 XilSKey_ZynqMp_EfusePs_ReadPpk1Hash(u32 *Ppk1Hash, u8 ReadOption);
u32 XilSKey_ZynqMp_EfusePs_ReadSpkId(u32 *SpkId, u8 ReadOption);
u32 XilSKey_ZynqMp_EfusePs_ReadJtagUsrCode(u32 *JtagUsrCode, u8 ReadOption);
void XilSKey_ZynqMp_EfusePs_ReadDna(u32 *DnaRead);

u32 XilSKey_ZynqMp_EfusePs_ReadSecCtrlBits(
		XilSKey_SecCtrlBits *ReadBackSecCtrlBits, u8 ReadOption);
u32 XilSKey_ZynqMp_EfusePs_CacheLoad();
u32 XilSKey_ZynqMp_EfusePs_Write(XilSKey_ZynqMpEPs *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* XILSKEY_EPS_ZYNQMP_H */
