/******************************************************************************
* Copyright (c) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xilskey_eps_zynqmp.h
* @addtogroup xilskey_zynqmp_efuse ZynqMP EFUSE PS
* @{
* @cond xilskey_internal
* @{
*
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
* 6.0   vns     07/18/16 Added separate User FUSEs programming feasibility
*                        Modified XilSKey_ZynqMp_EfusePs_ReadUserFuse
*                        prototype. Removed JTAG user code programming
*                        feature.Added XSK_ZYNQMP_SEC_PPK_INVLD_BITS_SET
*                        to check both PPK invalid bits are set or not,
*                        To check RSA authentication enable, defined
*                        XSK_ZYNQMP_SEC_RSA_15BITS_SET and
*                        XSK_ZYNQMP_SEC_RSA_2BITS_SET macros. Added all RSA
*                        enable bits to enum. Modified RSAenable variable type
*                        to u16.
* 6.2   vns     03/10/17 Added support for LBIST, LPD and FPD sc enable,
*                        PBR_BOOT_ERROR. Modified names of secure control
*                        bits UseAESOnly -> EncOnly, PMUError->ErrorDisable,
*                        PPK0Revoke->PPK0InVld and PPK1Revoke->PPK1InVld
* 6.6   vns     06/06/18 Added doxygen tags
*       vns     09/18/18 Added APIs to support eFUSE programming from linux
* 6.7   arc     01/05/19 Fixed MISRA-C violations.
* 6.8   psl     07/30/19 Fixed MISRA-C violations.
* 6.9   kal     03/16/20 Added macro for AES key offset for IPI calls.
*       ana     04/07/20 Removed IsPpk0Sha3Hash and IsPpk1Sha3Hash variabes,
*                        as these are not required with only sha3 support.
* 7.0	am	    10/04/20 Resolved MISRA C violations
* 7.1	kal	    02/28/21 Added new macro for number of eFuse rows per page
*       kpt     05/11/21 Added support for programming PUF Fuses as
*                        general purpose data

*
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

/* Efuse read selection */
#define XSK_EFUSEPS_READ_FROM_CACHE	0U
#define XSK_EFUSEPS_READ_FROM_EFUSE	1U

/* Key length definitions for ZynqMP eFuse */
#define XSK_ZYNQMP_EFUSEPS_AES_KEY_LEN_IN_BYTES			(32U)
#define XSK_ZYNQMP_EFUSEPS_USER_FUSE_ROW_LEN_IN_BYTES		(4U)
#define XSK_ZYNQMP_EFUSEPS_PPK_HASH_LEN_IN_BYTES		(48U)
#define XSK_ZYNQMP_EFUSEPS_SPKID_LEN_IN_BYTES			(4U)
#define XSK_ZYNQMP_EFUSEPS_DNA_LEN_IN_BYTES			(12U)

/* ZynqMP eFuse PS keys lengths in bits */
#define XSK_ZYNQMP_EFUSEPS_AES_KEY_LEN_IN_BITS			(256U)
#define XSK_ZYNQMP_EFUSEPS_USER_FUSE_ROW_LEN_IN_BITS		(32U)
#define XSK_ZYNQMP_EFUSEPS_PPK_SHA3HASH_LEN_IN_BITS		(384U)
#define XSK_ZYNQMP_EFUSEPS_SPKID_LEN_IN_BITS			(32U)

/* ZynqMP eFuse maximum bits in a row */
#define XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW			(32U)

/* No of Registers allocated for PPK sha3 hash */
#define XSK_ZYNQMP_EFUSEPS_PPK_HASH_REG_NUM			(12U)
#define XSK_ZYNQMP_EFUSEPS_USR_FUSE_REG_NUM			(8U)

/* Row numbers of Efuse PS of Zynq MP */
#define XSK_ZYNQMP_EFUSEPS_USR_KEY_END_ROW			(15U)
#define XSK_ZYNQMP_EFUSEPS_MISC_USR_CTRL_ROW			(16U)
#define XSK_ZYNQMP_EFUSEPS_PBR_BOOT_ERR_ROW			(17U)
#define XSK_ZYNQMP_EFUSEPS_RESERVED_ROW				(19U)
#define XSK_ZYNQMP_EFUSEPS_SEC_CTRL_ROW				(22U)
#define XSK_ZYNQMP_EFUSEPS_SPK_ID_ROW				(23U)
#define XSK_ZYNQMP_EFUSEPS_AES_KEY_START_ROW			(24U)
#define XSK_ZYNQMP_EFUSEPS_AES_KEY_END_ROW			(31U)
#define XSK_ZYNQMP_EFUSEPS_PUF_ROW_START		(0U)
#define XSK_ZYNQMP_EFUSEPS_PUF_ROW_END			(63U)
#define XSK_ZYNQMP_EFUSEPS_PUF_CHASH_ROW		(20U)
#define XSK_ZYNQMP_EFUSEPS_PUF_AUX_ROW			(21U)

/* User Fuses Row numbers */
#define XSK_ZYNQMP_EFUSEPS_USR0_FUSE_ROW			(8U)
#define XSK_ZYNQMP_EFUSEPS_USR1_FUSE_ROW			(9U)
#define XSK_ZYNQMP_EFUSEPS_USR2_FUSE_ROW			(10U)
#define XSK_ZYNQMP_EFUSEPS_USR3_FUSE_ROW			(11U)
#define XSK_ZYNQMP_EFUSEPS_USR4_FUSE_ROW			(12U)
#define XSK_ZYNQMP_EFUSEPS_USR5_FUSE_ROW			(13U)
#define XSK_ZYNQMP_EFUSEPS_USR6_FUSE_ROW			(14U)
#define XSK_ZYNQMP_EFUSEPS_USR7_FUSE_ROW			(15U)

#define XSK_ZYNQMP_EFUSEPS_PPK0_START_ROW			(40U)
#define XSK_ZYNQMP_EFUSEPS_PPK0_SHA3_HASH_END_ROW		(51U)

#define XSK_ZYNQMP_EFUSEPS_PPK1_START_ROW			(52U)
#define XSK_ZYNQMP_EFUSEPS_PPK1_SHA3_HASH_END_ROW		(63U)

#define XSK_ZYNQMP_EFUSEPS_TBITS_ROW				(0U)

#define XSK_ZYNQMP_EFUSEPS_TBITS_MASK				(0xFU)
#define XSK_ZYNQMP_EFUSEPS_TBITS_SHIFT				(28U)

#define XSK_ZYNQMP_EFUSEPS_CRC_AES_ZEROS		(0x6858A3D5U)

/* eFuse Offset = efuse row number * 4(that is sizeof(row))
 */
#define XSK_ZYNQMP_EFUSEPS_AES_KEY_OFFSET 	\
			(XSK_ZYNQMP_EFUSEPS_AES_KEY_START_ROW << 2U)

/* User fuses*/
#define XSK_ZYNQMP_EFUSEPS_USR0_FUSE	(0U)
#define XSK_ZYNQMP_EFUSEPS_USR1_FUSE	(1U)
#define XSK_ZYNQMP_EFUSEPS_USR2_FUSE	(2U)
#define XSK_ZYNQMP_EFUSEPS_USR3_FUSE	(3U)
#define XSK_ZYNQMP_EFUSEPS_USR4_FUSE	(4U)
#define XSK_ZYNQMP_EFUSEPS_USR5_FUSE	(5U)
#define XSK_ZYNQMP_EFUSEPS_USR6_FUSE	(6U)
#define XSK_ZYNQMP_EFUSEPS_USR7_FUSE	(7U)

#define XSK_ZYNQMP_EFUSEPS_NUM_OF_ROWS_PER_PAGE		(64U)

#define XSK_EFUSEPS_TPRGM_VALUE \
	(((5.0f) * (XSK_ZYNQMP_EFUSEPS_PS_REF_CLK_FREQ)) / (1000000.0f))
#define XSK_EFUSEPS_TRD_VALUE	\
 (((15.0f) * (XSK_ZYNQMP_EFUSEPS_PS_REF_CLK_FREQ)) / (100000000.0f))
#define XSK_EFUSEPS_TSUHPS_VALUE \
 (((67.0f) * (XSK_ZYNQMP_EFUSEPS_PS_REF_CLK_FREQ)) / (1000000000.0f))
#define XSK_EFUSEPS_TSUHPSCS_VALUE \
 (((46.0f) * (XSK_ZYNQMP_EFUSEPS_PS_REF_CLK_FREQ)) / (1000000000.0f))
#define XSK_EFUSEPS_TSUHCS_VALUE \
 (((30.0f) * (XSK_ZYNQMP_EFUSEPS_PS_REF_CLK_FREQ)) / (1000000000.0f))

/* Timer related macros */
#define XilSKey_ZynqMp_EfusePs_Tprgrm() \
	Xil_Ceil(XSK_EFUSEPS_TPRGM_VALUE)
#define XilSKey_ZynqMp_EfusePs_Trd() \
	Xil_Ceil(XSK_EFUSEPS_TRD_VALUE)
#define XilSKey_ZynqMp_EfusePs_TsuHPs() \
	Xil_Ceil(XSK_EFUSEPS_TSUHPS_VALUE)
#define XilSKey_ZynqMp_EfusePs_TsuHPsCs() \
	Xil_Ceil(XSK_EFUSEPS_TSUHPSCS_VALUE)
#define XilSKey_ZynqMp_EfusePs_TsuHCs() \
	Xil_Ceil(XSK_EFUSEPS_TSUHCS_VALUE)


#define XSK_ZYNQMP_SEC_PPK_INVLD_BITS_SET	(0x3U)
			/**< If PPK invalid bits are set */
/* For Silicon from 3.0 version */
#define XSK_ZYNQMP_SEC_RSA_15BITS_SET		(0x7FFF)
			/**< If RSA authentication bits are set */
/* For Silicon before 3.0 version */
#define XSK_ZYNQMP_SEC_RSA_2BITS_SET		(0x3U)

/* For any of secure control bits which has 3 bits */
#define XSK_ZYNQMP_SEC_ALL_3BITS_SET		(0x7U)

/* For any of secure control bits which has 3 bits */
#define XSK_ZYNQMP_SEC_ALL_16BITS_SET   (0xFFFFU)

#define XSK_ZYNQMP_EFUSEPS_SECTRL_BIT_SHIFT	0x1U
								/**< Shift macro for SEC_CTRL
								 *   if it has 2 bits */
#define XSK_EFUSEPS_OFFSET_MASK			(0xFFU)
#define XSK_EFUSEPS_PUF_ROW_OFFSET_MASK		(0x7FU)
#define XSK_EFUSEPS_ONE_WORD			(1U)
#define XSK_EFUSEPS_BYTES_IN_WORD		(4U)

/** @name efuse types
 * @{
 */
typedef enum {
	XSK_ZYNQMP_EFUSEPS_EFUSE_0,
	XSK_ZYNQMP_EFUSEPS_EFUSE_2 = 2U,
	XSK_ZYNQMP_EFUSEPS_EFUSE_3 = 3U
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
	XSK_ZYNQMP_EFUSEPS_SEC_RSA_EN_BIT1,
	XSK_ZYNQMP_EFUSEPS_SEC_RSA_EN_BIT2,
	XSK_ZYNQMP_EFUSEPS_SEC_RSA_EN_BIT3,
	XSK_ZYNQMP_EFUSEPS_SEC_RSA_EN_BIT4,
	XSK_ZYNQMP_EFUSEPS_SEC_RSA_EN_BIT5,
	XSK_ZYNQMP_EFUSEPS_SEC_RSA_EN_BIT6,
	XSK_ZYNQMP_EFUSEPS_SEC_RSA_EN_BIT7,
	XSK_ZYNQMP_EFUSEPS_SEC_RSA_EN_BIT8,
	XSK_ZYNQMP_EFUSEPS_SEC_RSA_EN_BIT9,
	XSK_ZYNQMP_EFUSEPS_SEC_RSA_EN_BIT10,
	XSK_ZYNQMP_EFUSEPS_SEC_RSA_EN_BIT11,
	XSK_ZYNQMP_EFUSEPS_SEC_RSA_EN_BIT12,
	XSK_ZYNQMP_EFUSEPS_SEC_RSA_EN_BIT13,
	XSK_ZYNQMP_EFUSEPS_SEC_RSA_EN_BIT14,
	XSK_ZYNQMP_EFUSEPS_SEC_RSA_EN_BIT15,
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
	XSK_ZYNQMP_EFUSEPS_LBIST_EN = 10,
	XSK_ZYNQMP_EFUSEPS_LPD_SC_EN_0,
	XSK_ZYNQMP_EFUSEPS_LPD_SC_EN_1,
	XSK_ZYNQMP_EFUSEPS_LPD_SC_EN_2,
	XSK_ZYNQMP_EFUSEPS_FPD_SC_EN_0,
	XSK_ZYNQMP_EFUSEPS_FPD_SC_EN_1,
	XSK_ZYNQMP_EFUSEPS_FPD_SC_EN_2,
}XskEfusePS_MiscUserBits;
/*@}*/

/** @name efuse PBR boot error bits
 * @{
 */
typedef enum {
	XSK_ZYNQMP_EFUSEPS_PBR_BOOT_ERR_0,
	XSK_ZYNQMP_EFUSEPS_PBR_BOOT_ERR_1,
	XSK_ZYNQMP_EFUSEPS_PBR_BOOT_ERR_2
}XskEfusePS_PbrBootErrBits;
/*@}*/

/**
*
* This typedef contains secure control features of efusePs
*/
typedef struct {
	/* Secure and control bits */
	u8 AesKeyRead;
	u8 AesKeyWrite;
	u8 EncOnly;
	u8 BbramDisable;
	u8 ErrorDisable;
	u8 JtagDisable;
	u8 DFTDisable;
	u8 ProgGate;
	u8 SecureLock;
	u16 RSAEnable;
	u8 PPK0WrLock;
	u8 PPK0InVld;
	u8 PPK1WrLock;
	u8 PPK1InVld;
	u8 PbrBootErr;

	/* User control bits */
	u8 UserWrLk0;
	u8 UserWrLk1;
	u8 UserWrLk2;
	u8 UserWrLk3;
	u8 UserWrLk4;
	u8 UserWrLk5;
	u8 UserWrLk6;
	u8 UserWrLk7;

	u8 LBistEn;
	u8 FpdScEn;
	u8 LpdScEn;

	/* Reserved for Xilinx internal use */
	u16 Reserved1;
	u16 Reserved2;

} XilSKey_SecCtrlBits;
/*@}*/

#if defined (XSK_ACCESS_PUF_USER_EFUSE)
typedef struct {
	u8 PrgrmPufFuse;
	u8 ReadPufFuse;
	u8 PufFuseStartRow;
	u8 PufNumOfFuses;
	u32 *PufFuseData;
} XilSKey_PufEfuse;
#endif

/**
 * XilSKey_ZynqMpEPs is the PS eFUSE driver instance. Using this
 * structure, user can define the eFUSE bits of Zynq MP ultrascale to be
 * blown.
 */
typedef struct {

	XilSKey_SecCtrlBits PrgrmgSecCtrlBits;
	/* For writing into eFuse */
	u8 PrgrmAesKey;
	u8 PrgrmPpk0Hash;
	u8 PrgrmPpk1Hash;
	u8 PrgrmSpkID;

	u8 PrgrmUser0Fuse;
	u8 PrgrmUser1Fuse;
	u8 PrgrmUser2Fuse;
	u8 PrgrmUser3Fuse;
	u8 PrgrmUser4Fuse;
	u8 PrgrmUser5Fuse;
	u8 PrgrmUser6Fuse;
	u8 PrgrmUser7Fuse;

	u8 AESKey[XSK_ZYNQMP_EFUSEPS_AES_KEY_LEN_IN_BYTES];

	u8 User0Fuses[XSK_ZYNQMP_EFUSEPS_USER_FUSE_ROW_LEN_IN_BYTES];
	u8 User1Fuses[XSK_ZYNQMP_EFUSEPS_USER_FUSE_ROW_LEN_IN_BYTES];
	u8 User2Fuses[XSK_ZYNQMP_EFUSEPS_USER_FUSE_ROW_LEN_IN_BYTES];
	u8 User3Fuses[XSK_ZYNQMP_EFUSEPS_USER_FUSE_ROW_LEN_IN_BYTES];
	u8 User4Fuses[XSK_ZYNQMP_EFUSEPS_USER_FUSE_ROW_LEN_IN_BYTES];
	u8 User5Fuses[XSK_ZYNQMP_EFUSEPS_USER_FUSE_ROW_LEN_IN_BYTES];
	u8 User6Fuses[XSK_ZYNQMP_EFUSEPS_USER_FUSE_ROW_LEN_IN_BYTES];
	u8 User7Fuses[XSK_ZYNQMP_EFUSEPS_USER_FUSE_ROW_LEN_IN_BYTES];

	u8 Ppk0Hash[XSK_ZYNQMP_EFUSEPS_PPK_HASH_LEN_IN_BYTES];
	u8 Ppk1Hash[XSK_ZYNQMP_EFUSEPS_PPK_HASH_LEN_IN_BYTES];
	u8 SpkId[XSK_ZYNQMP_EFUSEPS_SPKID_LEN_IN_BYTES];

	XilSKey_SecCtrlBits ReadBackSecCtrlBits;

	u8 IntialisedTimer;

}XilSKey_ZynqMpEPs;

/**
 * XilSKey_Efuse is the eFUSE access structure. Using this
 * structure, user can request the eFUSE access.
 */
typedef struct {
	u64 Src;	/**< Address of Data buffer */
	u32 Size;	/**< Size in words */
	u32 Offset;	/**< offset */
	u32 Flag;	/**< Flag - 0 : to read efuse and Flag - 1 : to write efuse */
	u32 PufUserFuse;/**< PufUserFuse - 0 : Puf HD eFuses for only PUF HD
				PufUserFuse - 1 : Puf HD eFuses for User Data */
}XilSKey_Efuse;

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
	(((XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR, \
		XSK_ZYNQMP_EFUSEPS_WR_LOCK_OFFSET)) != 0U) ? 1U : 0U)

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
	XSK_ZYNQMP_EFUSEPS_CFG_OFFSET) | (u32)XSK_ZYNQMP_EFUSEPS_CFG_PGM_EN_MASK)))

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

/** @}
@endcond */
/****************************Prototypes***************************************/
/* Ps eFuse interface functions of Zynq MP */
u32 XilSKey_ZynqMp_EfusePs_CheckAesKeyCrc(u32 CrcValue);
u32 XilSKey_ZynqMp_EfusePs_ReadUserFuse(u32 *UseFusePtr, u8 UserFuse_Num,
							u8 ReadOption);
u32 XilSKey_ZynqMp_EfusePs_ReadPpk0Hash(u32 *Ppk0Hash, u8 ReadOption);
u32 XilSKey_ZynqMp_EfusePs_ReadPpk1Hash(u32 *Ppk1Hash, u8 ReadOption);
u32 XilSKey_ZynqMp_EfusePs_ReadSpkId(u32 *SpkId, u8 ReadOption);
void XilSKey_ZynqMp_EfusePs_ReadDna(u32 *DnaRead);

u32 XilSKey_ZynqMp_EfusePs_ReadSecCtrlBits(
		XilSKey_SecCtrlBits *ReadBackSecCtrlBits, u8 ReadOption);
u32 XilSKey_ZynqMp_EfusePs_CacheLoad(void);
u32 XilSKey_ZynqMp_EfusePs_Write(XilSKey_ZynqMpEPs *InstancePtr);
u32 XilSkey_ZynqMpEfuseAccess(const u32 AddrHigh, const u32 AddrLow);
void XilSKey_ZynqMp_EfusePs_SetTimerValues(void);
u32 XilSKey_ZynqMp_EfusePs_ReadRow(u8 Row, XskEfusePs_Type EfuseType,
							u32 *RowData);
u32 XilSKey_ZynqMp_EfusePs_SetWriteConditions(void);
u32 XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(u8 Row, u8 Column,
						XskEfusePs_Type EfuseType);
u32 XilSKey_ZynqMp_EfusePs_Init(void);
u32 XilSKey_ZynqMp_EfusePs_CheckForZeros(u8 RowStart, u8 RowEnd,
						XskEfusePs_Type EfuseType);

#if defined (XSK_ACCESS_PUF_USER_EFUSE)
u32 XilSKey_ZynqMp_EfusePs_ProgramPufAsUserFuses(
						const XilSKey_PufEfuse *PufFuse);
u32 XilSKey_ZynqMp_EfusePs_ReadPufAsUserFuses(
					    const XilSKey_PufEfuse *PufFuse);
#endif

#ifdef __cplusplus
}
#endif

#endif /* XILSKEY_EPS_ZYNQMP_H */
/**@}*/
