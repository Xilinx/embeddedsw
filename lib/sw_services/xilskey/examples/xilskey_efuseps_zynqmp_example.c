/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xilskey_efuseps_zynqmp_example.c
* This file illustrates how to program ZynqMp efuse and read back the keys from
* efuse.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 4.0   vns     10/01/15 First release
*       vns     10/20/15 Modified XilSKey_EfusePs_Example_ReadSecCtrlBits API
*                        while reading RSA authentication and PPK revokes bits
*                        status it may return 0 or BOTH_BITS_SET. So in place
*                        of TRUE added BOTH_BITS_SET.
* 6.0   vns     07/18/16 Removed JTAG user code programming and reading feature
*                        as it is not the part of the eFUSE 3.0 silicon.
*                        Modified XilSKey_ZynqMp_EfusePs_ReadUserKey function
*                        to XilSKey_ZynqMp_EfusePs_ReadUserFuse.
*                        Provided single bit programming facility for User
*                        FUSES. Modified RSA authentication bit set macro
*                        BOTH_BITS_SET to XSK_ZYNQMP_SEC_RSA_15BITS_SET and
*                        XSK_ZYNQMP_SEC_RSA_3BITS_SET, from silicon version
*                        3.0 RSA authentication is possible only if all 15
*                        bits of RSA authentication bits are set, for 1.0 and
*                        2.0 versions only 2 bits are needed, for PPK0 REVOKE
*                        check added new macro XSK_ZYNQMP_SEC_PPK_INVLD_BITS_SET
* 6.2   vns      03/10/17 Added support for programming and reading
*                         LDP SC EN, FPD SC EN, LBIST, reading some of
*                         reserved bits, modified names of secure control bits
*                         Provided DNA read API call in example.
* 6.4   vns      02/19/18 Removed XilSKey_ZynqMp_EfusePs_CacheLoad() call as
*                         now library is been updated to reload cache after
*                         successful programming of the requested efuse bits.
* 6.7	psl      03/13/19 Added XSK_EFUSEPS_CHECK_AES_KEY_CRC, to check for
* 						  AES key CRC if TRUE.
* 	    psl      03/28/19 Corrected typos
*       psl      04/10/19 Fixed IAR warnings.
* 6.8   psl      07/17/19 Added print to display CRC of AES key for CRC
*                         verification.
* 7.0   kpt      09/02/20 Added successfully ran print to the example in
*                         case of success
* 7.1   kpt      05/11/21 Added BareMetal support for programming PUF Fuses as
*                         general purpose fuses
*       kpt      05/21/21 Added print before programming PPK hash into non-zero
*                         PPK efuses
* 7.5   ng       07/13/23 Added support for system device tree flow
*       ng       08/31/23 removed redundant header file inclusion
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xilskey_efuseps_zynqmp_input.h"
#include "xil_printf.h"

/***************** Macros (Inline Functions) Definitions *********************/
#define XSK_EFUSEPS_AES_KEY_STRING_LEN			(64)
#define XSK_EFUSEPS_USER_FUSE_ROW_STRING_LEN		(8)
#define XSK_EFUSEPS_PPK_SHA3_HASH_STRING_LEN_96		(96)
#define XSK_EFUSEPS_SPK_ID_STRING_LEN			(8)

#define XSK_EFUSEPS_AES_KEY_LEN_IN_BITS			(256)
#define XSK_EFUSEPS_USER_FUSE_ROW_LEN_IN_BITS		(32)
#define XSK_EFUSEPS_PPK_SHA3HASH_LEN_IN_BITS_384	(384)
#define XSK_EFUSEPS_SPKID_LEN_IN_BITS			(32)

#define XSK_EFUSEPS_RD_FROM_CACHE				(0)
#define XSK_EFUSEPS_RD_FROM_EFUSE				(1)

/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

static inline u32 XilSKey_EfusePs_ZynqMp_InitData(
				XilSKey_ZynqMpEPs *PsInstancePtr);
static inline u32 XilSKey_EfusePs_Example_ReadSecCtrlBits();
#if defined(XSK_ACCESS_PUF_USER_EFUSE)
static inline u32 XilSKey_EfusePs_ZynqMp_ProgramPufFuses(void);
static inline u32 XilSKey_EfusePs_ZynqMp_ReadPufFuses(void);
#endif

/*****************************************************************************/

int main()
{

	XilSKey_ZynqMpEPs PsInstance = {{0}};
	u32 PsStatus;
	u32 UserFuses[8];
	u32 Ppk0[12];
	u32 Ppk1[12];
	u32 SpkId;
	s8 Row;
	u32 AesCrc;
	u32 Dna[3];

#if defined (XSK_ZYNQ_PLATFORM) || defined (XSK_MICROBLAZE_PLATFORM)
	xil_printf("This example will not work for this platform\n\r");
#endif
	/* Initiate the Efuse PS instance */
	PsStatus = XilSKey_EfusePs_ZynqMp_InitData(&PsInstance);
	if (PsStatus != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

	/* Programming the keys */
	PsStatus = XilSKey_ZynqMp_EfusePs_Write(&PsInstance);
	if (PsStatus != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}
	/* Read DNA */
	XilSKey_ZynqMp_EfusePs_ReadDna(Dna);
	xil_printf("DNA:%08x%08x%08x", Dna[2], Dna[1], Dna[0]);

	/* Read keys from cache */
	xil_printf("keys read from cache \n\r");
	for (Row = 0; Row < 8; Row++) {
		PsStatus = XilSKey_ZynqMp_EfusePs_ReadUserFuse(
					&UserFuses[Row], Row,
					XSK_EFUSEPS_RD_FROM_CACHE);
		if (PsStatus != XST_SUCCESS) {
			goto EFUSE_ERROR;
		}
	}
	for (Row = 0; Row < 8; Row++) {
		xil_printf("User%d Fuse:%08x\n\r", Row,UserFuses[Row]);
	}
	xil_printf("\n\r");

	PsStatus = XilSKey_ZynqMp_EfusePs_ReadPpk0Hash(Ppk0,
							XSK_EFUSEPS_RD_FROM_CACHE);
	if (PsStatus != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}
	else {
		xil_printf("\n\rPPK0:");
		for (Row = 11; Row >= 0; Row--)
			xil_printf("%08x", Ppk0[Row]);
	}
	xil_printf("\n\r");

	PsStatus = XilSKey_ZynqMp_EfusePs_ReadPpk1Hash(Ppk1,
							XSK_EFUSEPS_RD_FROM_CACHE);
	if (PsStatus != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}
	else {
		xil_printf("\n\rPPK1:");
		for (Row = 11; Row >= 0; Row--)
			xil_printf("%08x", Ppk1[Row]);
	}
	xil_printf("\n\r");

	PsStatus = XilSKey_ZynqMp_EfusePs_ReadSpkId(&SpkId,
							XSK_EFUSEPS_RD_FROM_CACHE);
	if (PsStatus != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}
	else {
		xil_printf("\r\nSpkid: %08x\n\r", SpkId);

	}

	/* CRC check for programmed AES key */
	if (XSK_EFUSEPS_CHECK_AES_KEY_CRC == TRUE) {
		AesCrc = XilSKey_CrcCalculation((u8 *)XSK_EFUSEPS_AES_KEY);
		xil_printf("AES Key's CRC provided for verification: %08x\n\r",
								AesCrc);
		PsStatus = XilSKey_ZynqMp_EfusePs_CheckAesKeyCrc(AesCrc);
		if (PsStatus != XST_SUCCESS) {
			xil_printf("\r\nAES CRC check is failed\n\r");
			goto EFUSE_ERROR;
		}
		else {
			xil_printf("\r\nAES CRC check is passed\n\r");
		}
	}

	/* Reading control and secure bits of eFuse */
	PsStatus = XilSKey_EfusePs_Example_ReadSecCtrlBits();
	if (PsStatus != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

#if defined(XSK_ACCESS_PUF_USER_EFUSE)
	PsStatus = XilSKey_EfusePs_ZynqMp_ProgramPufFuses();
	if (PsStatus != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}
	PsStatus = XilSKey_EfusePs_ZynqMp_ReadPufFuses();
	if (PsStatus != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}
#endif

EFUSE_ERROR:
	if (PsStatus != XST_SUCCESS) {
		xil_printf("\r\nnZynqMP eFuse example is failed with Status = %08x\n\r",
								PsStatus);
	}
	else {
		xil_printf("\r\nSuccessfully ran ZynqMP eFuse example....");
	}

	return PsStatus;
}

/****************************************************************************/
/**
*
*
* Helper functions to properly initialize the Ps eFUSE structure instance
*
*
* @param	PsInstancePtr - Structure Address to update the
*		structure elements
*
* @return
*		- XST_SUCCESS - In case of Success
*		- XST_FAILURE - If initialization fails
*
* @note
*
*****************************************************************************/

static inline u32 XilSKey_EfusePs_ZynqMp_InitData(
				XilSKey_ZynqMpEPs *PsInstancePtr)
{

	u32 PsStatus;
	u32 PpkRd[XSK_ZYNQMP_EFUSEPS_PPK_HASH_REG_NUM];
	u8 Row;

	PsStatus = XST_SUCCESS;

	/*
	 * Copy the xilskey_efuseps_zynqmp_input.h values into
	 * PS eFUSE structure elements
	 */

	/* Secure and control bits for programming */
	PsInstancePtr->PrgrmgSecCtrlBits.AesKeyRead = XSK_EFUSEPS_AES_RD_LOCK;
	PsInstancePtr->PrgrmgSecCtrlBits.AesKeyWrite = XSK_EFUSEPS_AES_WR_LOCK;
	PsInstancePtr->PrgrmgSecCtrlBits.EncOnly = XSK_EFUSEPS_ENC_ONLY;
	PsInstancePtr->PrgrmgSecCtrlBits.BbramDisable =
						XSK_EFUSEPS_BBRAM_DISABLE;
	PsInstancePtr->PrgrmgSecCtrlBits.ErrorDisable =	XSK_EFUSEPS_ERR_DISABLE;
	PsInstancePtr->PrgrmgSecCtrlBits.JtagDisable = XSK_EFUSEPS_JTAG_DISABLE;
	PsInstancePtr->PrgrmgSecCtrlBits.DFTDisable = XSK_EFUSEPS_DFT_DISABLE;
	PsInstancePtr->PrgrmgSecCtrlBits.ProgGate =
								XSK_EFUSEPS_PROG_GATE_DISABLE;
	PsInstancePtr->PrgrmgSecCtrlBits.SecureLock = XSK_EFUSEPS_SECURE_LOCK;
	PsInstancePtr->PrgrmgSecCtrlBits.RSAEnable = XSK_EFUSEPS_RSA_ENABLE;
	PsInstancePtr->PrgrmgSecCtrlBits.PPK0WrLock = XSK_EFUSEPS_PPK0_WR_LOCK;
	PsInstancePtr->PrgrmgSecCtrlBits.PPK0InVld = XSK_EFUSEPS_PPK0_INVLD;
	PsInstancePtr->PrgrmgSecCtrlBits.PPK1WrLock = XSK_EFUSEPS_PPK1_WR_LOCK;
	PsInstancePtr->PrgrmgSecCtrlBits.PPK1InVld = XSK_EFUSEPS_PPK1_INVLD;
	PsInstancePtr->PrgrmgSecCtrlBits.LBistEn = XSK_EFUSEPS_LBIST_EN;
	PsInstancePtr->PrgrmgSecCtrlBits.LpdScEn = XSK_EFUSEPS_LPD_SC_EN;
	PsInstancePtr->PrgrmgSecCtrlBits.FpdScEn = XSK_EFUSEPS_FPD_SC_EN;
	PsInstancePtr->PrgrmgSecCtrlBits.PbrBootErr = XSK_EFUSEPS_PBR_BOOT_ERR;

	/* User control bits */
	PsInstancePtr->PrgrmgSecCtrlBits.UserWrLk0 = XSK_EFUSEPS_USER_WRLK_0;
	PsInstancePtr->PrgrmgSecCtrlBits.UserWrLk1 = XSK_EFUSEPS_USER_WRLK_1;
	PsInstancePtr->PrgrmgSecCtrlBits.UserWrLk2 = XSK_EFUSEPS_USER_WRLK_2;
	PsInstancePtr->PrgrmgSecCtrlBits.UserWrLk3 = XSK_EFUSEPS_USER_WRLK_3;
	PsInstancePtr->PrgrmgSecCtrlBits.UserWrLk4 = XSK_EFUSEPS_USER_WRLK_4;
	PsInstancePtr->PrgrmgSecCtrlBits.UserWrLk5 = XSK_EFUSEPS_USER_WRLK_5;
	PsInstancePtr->PrgrmgSecCtrlBits.UserWrLk6 = XSK_EFUSEPS_USER_WRLK_6;
	PsInstancePtr->PrgrmgSecCtrlBits.UserWrLk7 = XSK_EFUSEPS_USER_WRLK_7;

	/* For writing into eFuse */
	PsInstancePtr->PrgrmAesKey = XSK_EFUSEPS_WRITE_AES_KEY;
	PsInstancePtr->PrgrmPpk0Hash = XSK_EFUSEPS_WRITE_PPK0_HASH;
	PsInstancePtr->PrgrmPpk1Hash = XSK_EFUSEPS_WRITE_PPK1_HASH;
	PsInstancePtr->PrgrmSpkID = XSK_EFUSEPS_WRITE_SPKID;

	PsInstancePtr->PrgrmUser0Fuse = XSK_EFUSEPS_WRITE_USER0_FUSE;
	PsInstancePtr->PrgrmUser1Fuse = XSK_EFUSEPS_WRITE_USER1_FUSE;
	PsInstancePtr->PrgrmUser2Fuse = XSK_EFUSEPS_WRITE_USER2_FUSE;
	PsInstancePtr->PrgrmUser3Fuse = XSK_EFUSEPS_WRITE_USER3_FUSE;
	PsInstancePtr->PrgrmUser4Fuse = XSK_EFUSEPS_WRITE_USER4_FUSE;
	PsInstancePtr->PrgrmUser5Fuse = XSK_EFUSEPS_WRITE_USER5_FUSE;
	PsInstancePtr->PrgrmUser6Fuse = XSK_EFUSEPS_WRITE_USER6_FUSE;
	PsInstancePtr->PrgrmUser7Fuse = XSK_EFUSEPS_WRITE_USER7_FUSE;

	/* Variable for Timer Initialization */
	PsInstancePtr->IntialisedTimer = 0;

	/* Copy the fuses to be programmed */

	/* Copy all the user fuses to be programmed */
	if (PsInstancePtr->PrgrmUser0Fuse == TRUE) {
		/* Validation of User 0 fuse */
		PsStatus = XilSKey_Efuse_ValidateKey(
				(char *)XSK_EFUSEPS_USER0_FUSES,
				XSK_EFUSEPS_USER_FUSE_ROW_STRING_LEN);
		if(PsStatus != XST_SUCCESS) {
			goto ERROR;
		}
		/* Assign the User key [32:0]bits of User fuse 0 */
		XilSKey_Efuse_ConvertStringToHexLE(
			(char *)XSK_EFUSEPS_USER0_FUSES,
			&PsInstancePtr->User0Fuses[0],
			XSK_EFUSEPS_USER_FUSE_ROW_LEN_IN_BITS);
	}
	if (PsInstancePtr->PrgrmUser1Fuse == TRUE) {
		/* Validation of User 1 fuse */
		PsStatus = XilSKey_Efuse_ValidateKey(
				(char *)XSK_EFUSEPS_USER1_FUSES,
				XSK_EFUSEPS_USER_FUSE_ROW_STRING_LEN);
		if(PsStatus != XST_SUCCESS) {
			goto ERROR;
		}
		/* Assign the User key [32:0]bits of User fuse  1 */
		XilSKey_Efuse_ConvertStringToHexLE(
			(char *)XSK_EFUSEPS_USER1_FUSES,
			&PsInstancePtr->User1Fuses[0],
			XSK_EFUSEPS_USER_FUSE_ROW_LEN_IN_BITS);
	}
	if (PsInstancePtr->PrgrmUser2Fuse == TRUE) {
		/* Validation of User 2 fuse */
		PsStatus = XilSKey_Efuse_ValidateKey(
				(char *)XSK_EFUSEPS_USER2_FUSES,
				XSK_EFUSEPS_USER_FUSE_ROW_STRING_LEN);
		if(PsStatus != XST_SUCCESS) {
			goto ERROR;
		}
		/* Assign the User key [32:0]bits of User fuse  2 */
		XilSKey_Efuse_ConvertStringToHexLE(
			(char *)XSK_EFUSEPS_USER2_FUSES,
			&PsInstancePtr->User2Fuses[0],
			XSK_EFUSEPS_USER_FUSE_ROW_LEN_IN_BITS);
	}
	if (PsInstancePtr->PrgrmUser3Fuse == TRUE) {
		/* Validation of User 3 fuse */
		PsStatus = XilSKey_Efuse_ValidateKey(
				(char *)XSK_EFUSEPS_USER3_FUSES,
				XSK_EFUSEPS_USER_FUSE_ROW_STRING_LEN);
		if(PsStatus != XST_SUCCESS) {
			goto ERROR;
		}
		/* Assign the User key [32:0]bits of User fuse 3 */
		XilSKey_Efuse_ConvertStringToHexLE(
			(char *)XSK_EFUSEPS_USER3_FUSES,
			&PsInstancePtr->User3Fuses[0],
			XSK_EFUSEPS_USER_FUSE_ROW_LEN_IN_BITS);
	}
	if (PsInstancePtr->PrgrmUser4Fuse == TRUE) {
		/* Validation of User 4 fuse */
		PsStatus = XilSKey_Efuse_ValidateKey(
				(char *)XSK_EFUSEPS_USER4_FUSES,
				XSK_EFUSEPS_USER_FUSE_ROW_STRING_LEN);
		if(PsStatus != XST_SUCCESS) {
			goto ERROR;
		}
		/* Assign the User key [32:0]bits of User fuse  4 */
		XilSKey_Efuse_ConvertStringToHexLE(
			(char *)XSK_EFUSEPS_USER4_FUSES,
			&PsInstancePtr->User4Fuses[0],
			XSK_EFUSEPS_USER_FUSE_ROW_LEN_IN_BITS);
	}
	if (PsInstancePtr->PrgrmUser5Fuse == TRUE) {
		/* Validation of User 5 fuse */
		PsStatus = XilSKey_Efuse_ValidateKey(
				(char *)XSK_EFUSEPS_USER5_FUSES,
				XSK_EFUSEPS_USER_FUSE_ROW_STRING_LEN);
		if(PsStatus != XST_SUCCESS) {
			goto ERROR;
		}
		/* Assign the User key [32:0]bits of User fuse 5 */
		XilSKey_Efuse_ConvertStringToHexLE(
			(char *)XSK_EFUSEPS_USER5_FUSES,
			&PsInstancePtr->User5Fuses[0],
			XSK_EFUSEPS_USER_FUSE_ROW_LEN_IN_BITS);
	}
	if (PsInstancePtr->PrgrmUser6Fuse == TRUE) {
		/* Validation of User 6 fuse */
		PsStatus = XilSKey_Efuse_ValidateKey(
				(char *)XSK_EFUSEPS_USER6_FUSES,
				XSK_EFUSEPS_USER_FUSE_ROW_STRING_LEN);
		if(PsStatus != XST_SUCCESS) {
			goto ERROR;
		}
		/* Assign the User key [32:0]bits of User fuse 6 */
		XilSKey_Efuse_ConvertStringToHexLE(
			(char *)XSK_EFUSEPS_USER6_FUSES,
			&PsInstancePtr->User6Fuses[0],
			XSK_EFUSEPS_USER_FUSE_ROW_LEN_IN_BITS);
	}
	if (PsInstancePtr->PrgrmUser7Fuse == TRUE) {
		/* Validation of User 7 fuse */
		PsStatus = XilSKey_Efuse_ValidateKey(
				(char *)XSK_EFUSEPS_USER7_FUSES,
				XSK_EFUSEPS_USER_FUSE_ROW_STRING_LEN);
		if(PsStatus != XST_SUCCESS) {
			goto ERROR;
		}
		/* Assign the User key [32:0]bits of User fuse 7 */
		XilSKey_Efuse_ConvertStringToHexLE(
			(char *)XSK_EFUSEPS_USER7_FUSES,
			&PsInstancePtr->User7Fuses[0],
			XSK_EFUSEPS_USER_FUSE_ROW_LEN_IN_BITS);
	}

	/* Is AES key programming is enabled */
	if (PsInstancePtr->PrgrmAesKey == TRUE) {
		/* Validation of AES Key */
		PsStatus = XilSKey_Efuse_ValidateKey(
			(char *)XSK_EFUSEPS_AES_KEY,
			XSK_EFUSEPS_AES_KEY_STRING_LEN);
		if(PsStatus != XST_SUCCESS) {
			goto ERROR;
		}
		/* Assign the AES Key Value */
		XilSKey_Efuse_ConvertStringToHexLE(
			(char *)XSK_EFUSEPS_AES_KEY,
			&PsInstancePtr->AESKey[0],
			XSK_EFUSEPS_AES_KEY_LEN_IN_BITS);
	}

	/* Is PPK0 hash programming is enabled */
	if (PsInstancePtr->PrgrmPpk0Hash == TRUE) {
		/* Validation of PPK0 sha3 hash */
		PsStatus = XilSKey_Efuse_ValidateKey(
			(char *)XSK_EFUSEPS_PPK0_HASH,
			XSK_EFUSEPS_PPK_SHA3_HASH_STRING_LEN_96);
		if(PsStatus != XST_SUCCESS) {
			goto ERROR;
		}
		/* Assign the PPK0 sha3 hash */
		XilSKey_Efuse_ConvertStringToHexBE(
			(char *)XSK_EFUSEPS_PPK0_HASH,
			&PsInstancePtr->Ppk0Hash[0],
			XSK_EFUSEPS_PPK_SHA3HASH_LEN_IN_BITS_384);

		/* Warn user before programming if PPK Hash is non-zero */
		PsStatus = XilSKey_ZynqMp_EfusePs_ReadPpk0Hash(PpkRd,
									XSK_EFUSEPS_RD_FROM_CACHE);
		if (PsStatus != XST_SUCCESS) {
			goto ERROR;
		}

		for (Row = 0U; Row < XSK_ZYNQMP_EFUSEPS_PPK_HASH_REG_NUM; Row++) {
			if (PpkRd[Row] != 0U) {
				xil_printf(
					"PPK0 Hash in efuse is non-zero before programming \r\n");
				break;
			}
		}
	}

	/* Is PPK1 hash programming is enabled */
	if (PsInstancePtr->PrgrmPpk1Hash == TRUE) {
		/* Validation of PPK1 sha3 hash */
		PsStatus = XilSKey_Efuse_ValidateKey(
			(char *)XSK_EFUSEPS_PPK1_HASH,
			XSK_EFUSEPS_PPK_SHA3_HASH_STRING_LEN_96);
		if(PsStatus != XST_SUCCESS) {
			goto ERROR;
		}
		/* Assign the PPK1 sha3 hash */
		XilSKey_Efuse_ConvertStringToHexBE(
			(char *)XSK_EFUSEPS_PPK1_HASH,
			&PsInstancePtr->Ppk1Hash[0],
			XSK_EFUSEPS_PPK_SHA3HASH_LEN_IN_BITS_384);

		/* Warn user before programming if PPK Hash is non-zero */
		PsStatus = XilSKey_ZynqMp_EfusePs_ReadPpk1Hash(PpkRd,
									XSK_EFUSEPS_RD_FROM_CACHE);
		if (PsStatus != XST_SUCCESS) {
			goto ERROR;
		}

		for (Row = 0U; Row < XSK_ZYNQMP_EFUSEPS_PPK_HASH_REG_NUM; Row++) {
			if (PpkRd[Row] != 0U) {
				xil_printf(
					"PPK1 Hash in efuse is non-zero before programming \r\n");
				break;
			}
		}
	}

	if (PsInstancePtr->PrgrmSpkID == TRUE) {
		/* Validation of SPK ID */
		PsStatus = XilSKey_Efuse_ValidateKey(
				(char *)XSK_EFUSEPS_SPK_ID,
				XSK_EFUSEPS_SPK_ID_STRING_LEN);
		if (PsStatus != XST_SUCCESS) {
			goto ERROR;
		}
		/* Assign SPK ID */
		XilSKey_Efuse_ConvertStringToHexLE(
			(char *)XSK_EFUSEPS_SPK_ID,
			&PsInstancePtr->SpkId[0],
			XSK_EFUSEPS_SPKID_LEN_IN_BITS);
	}



ERROR:
	return PsStatus;

}

/****************************************************************************/
/**
* This API reads secure control bits from efuse and prints the status bits
*
*
* @param	None
*
* @return
*		- XST_SUCCESS - In case of Success
*		- ErrorCode - If fails
*
* @note
*
*****************************************************************************/
static inline u32 XilSKey_EfusePs_Example_ReadSecCtrlBits()
{
	u32 PsStatus;
	XilSKey_SecCtrlBits ReadSecCtrlBits;
	u32 Silicon_ver = XGetPSVersion_Info();

	PsStatus = XilSKey_ZynqMp_EfusePs_ReadSecCtrlBits(&ReadSecCtrlBits,
								XSK_EFUSEPS_RD_FROM_CACHE);
	if (PsStatus != XST_SUCCESS) {
		return PsStatus;
	}

	xil_printf("\r\nSecure and Control bits of eFuse:\n\r");

	if (ReadSecCtrlBits.AesKeyRead == TRUE) {
		xil_printf("\r\nAES key CRC check is disabled\n\r");
	}
	else {
		xil_printf("\r\nAES key CRC check is enabled\n\r");
	}

	if (ReadSecCtrlBits.AesKeyWrite == TRUE) {
		xil_printf("Programming AES key is disabled\n\r");
	}
	else {
		xil_printf("Programming AES key is enabled\n\r");
	}
	if (ReadSecCtrlBits.EncOnly == TRUE) {
		xil_printf("All boots must be encrypted with eFuse"
					"AES key is enabled\n\r");
	}
	else {
		xil_printf("All boots must be encrypted with eFuse"
					"AES key is disabled\n\r");
	}
	if (ReadSecCtrlBits.BbramDisable == TRUE) {
		xil_printf("Disables BBRAM key\n\r");
	}
	else {
		xil_printf("BBRAM key is not disabled\n\r");
	}
	if (ReadSecCtrlBits.ErrorDisable == TRUE) {
		xil_printf("Error output from PMU is disabled\n\r");
	}
	else {
		xil_printf("Error output from PMU is enabled\n\r");
	}
	if (ReadSecCtrlBits.JtagDisable == TRUE) {
		xil_printf("Jtag is disabled\n\r");
	}
	else {
		xil_printf("Jtag is enabled\n\r");
	}
	if (ReadSecCtrlBits.DFTDisable == TRUE) {
		xil_printf("DFT is disabled\n\r");
	}
	else {
		xil_printf("DFT is enabled\n\r");
	}
	if (ReadSecCtrlBits.ProgGate == XSK_ZYNQMP_SEC_ALL_3BITS_SET) {
		xil_printf("PROG_GATE feature is disabled\n\r");
	}
	else {
		xil_printf("PROG_GATE feature is enabled\n\r");
	}
	if (ReadSecCtrlBits.SecureLock == TRUE) {
		xil_printf("Reboot from JTAG mode is disabled when"
				"doing secure lock down\n\r");
	}
	else {
		xil_printf("Reboot from JTAG mode is enabled\n\r");
	}
	if (((Silicon_ver > XPS_VERSION_2) &&
	 (ReadSecCtrlBits.RSAEnable == XSK_ZYNQMP_SEC_RSA_15BITS_SET)) ||
		((Silicon_ver <= XPS_VERSION_2) &&
	 (ReadSecCtrlBits.RSAEnable == XSK_ZYNQMP_SEC_RSA_2BITS_SET))) {
		xil_printf("RSA authentication is enabled\n\r");
	}
	else {
		xil_printf("RSA authentication is disabled\n\r");
	}
	if (ReadSecCtrlBits.PPK0WrLock == TRUE) {
		xil_printf("Locks writing to PPK0 efuse \n\r");
	}
	else {
		xil_printf("writing to PPK0 efuse is not locked\n\r");
	}

	if (ReadSecCtrlBits.PPK0InVld == XSK_ZYNQMP_SEC_PPK_INVLD_BITS_SET) {
		xil_printf("Revoking PPK0 is enabled \n\r");
	}
	else {
		xil_printf("Revoking PPK0 is disabled\n\r");
	}

	if (ReadSecCtrlBits.PPK1WrLock == TRUE) {
		xil_printf("Locks writing to PPK1 efuses\n\r");
	}
	else {
		xil_printf("writing to PPK1 efuses is not locked\n\r");
	}

	if (ReadSecCtrlBits.PPK1InVld == XSK_ZYNQMP_SEC_PPK_INVLD_BITS_SET) {
		xil_printf("Revoking PPK1 is enabled \n\r");
	}
	else {
		xil_printf("Revoking PPK1 is disabled\n\r");
	}
	if (ReadSecCtrlBits.LBistEn == TRUE) {
		xil_printf("LBIST is been enabled\n\r");
	}
	else {
		xil_printf("LBIST is in disabled state\n\r");
	}
	if (ReadSecCtrlBits.PbrBootErr == XSK_ZYNQMP_SEC_ALL_3BITS_SET) {
		xil_printf("PBR boot error is programmed and boot is"
				" halted on any PMU error\n\r");
	}
	else {
		xil_printf("PBR boot error halt is disabled \n\r");
	}
	if (ReadSecCtrlBits.LpdScEn == XSK_ZYNQMP_SEC_ALL_3BITS_SET) {
		xil_printf("Zeroization of registers in Low Power Domain (LPD)"
				" during boot is enabled\n\r");
	}
	else {
		xil_printf("Zeroization of registers in Low Power Domain (LPD)"
				" during boot is disabled\n\r");
	}
	if (ReadSecCtrlBits.FpdScEn == XSK_ZYNQMP_SEC_ALL_3BITS_SET) {
		xil_printf("Zeroization of registers in Full Power Domain (FPD)"
				" during boot is enabled\n\r");
	}
	else {
		xil_printf("Zeroization of registers in Full Power Domain (FPD)"
				" during boot is disabled\n\r");
	}

	xil_printf("\r\nUser control bits of eFuse:\n\r");

	if (ReadSecCtrlBits.UserWrLk0 == TRUE) {
		xil_printf("Programming USER_0 fuses is disabled\n\r");
	}
	else {
		xil_printf("Programming USER_0 fuses is enabled\n\r");
	}
	if (ReadSecCtrlBits.UserWrLk1 == TRUE) {
		xil_printf("Programming USER_1 fuses is disabled\n\r");
	}
	else {
		xil_printf("Programming USER_1 fuses is enabled\n\r");
	}
	if (ReadSecCtrlBits.UserWrLk2 == TRUE) {
		xil_printf("Programming USER_2 fuses is disabled\n\r");
	}
	else {
		xil_printf("Programming USER_2 fuses is enabled\n\r");
	}
	if (ReadSecCtrlBits.UserWrLk3 == TRUE) {
		xil_printf("Programming USER_3 fuses is disabled\n\r");
	}
	else {
		xil_printf("Programming USER_3 fuses is enabled\n\r");
	}
	if (ReadSecCtrlBits.UserWrLk4 == TRUE) {
		xil_printf("Programming USER_4 fuses is disabled\n\r");
	}
	else {
		xil_printf("Programming USER_4 fuses is enabled\n\r");
	}
	if (ReadSecCtrlBits.UserWrLk5 == TRUE) {
		xil_printf("Programming USER_5 fuses is disabled\n\r");
	}
	else {
		xil_printf("Programming USER_5 fuses is enabled\n\r");
	}
	if (ReadSecCtrlBits.UserWrLk6 == TRUE) {
		xil_printf("Programming USER_6 fuses is disabled\n\r");
	}
	else {
		xil_printf("Programming USER_6 fuses is enabled\n\r");
	}
	if (ReadSecCtrlBits.UserWrLk7 == TRUE) {
		xil_printf("Programming USER_7 fuses is disabled\n\r");
	}
	else {
		xil_printf("Programming USER_7 fuses is enabled\n\r");
	}
	if (ReadSecCtrlBits.Reserved1 == XSK_ZYNQMP_SEC_ALL_16BITS_SET) {
		xil_printf("Reserved 1 bits are programmed on eFUSE\n\r");
	}
	else {
		xil_printf("Reserved 1 bits are not programmed on eFUSE\n\r");
	}
	if (ReadSecCtrlBits.Reserved2 == XSK_ZYNQMP_SEC_ALL_16BITS_SET) {
		xil_printf("Reserved 2 bits are programmed on eFUSE\n\r");
	}
	else {
		xil_printf("Reserved 2 bits are not programmed on eFUSE\n\r");
	}

	return XST_SUCCESS;

}
#if defined (XSK_ACCESS_PUF_USER_EFUSE)

/****************************************************************************/
/**
*
* This function programs the puf fuses as general purpose data
*
* @return
*		- XST_SUCCESS - In case of Program or Read Success
*		- XST_FAILURE - If program fails
*
*****************************************************************************/
static inline u32 XilSKey_EfusePs_ZynqMp_ProgramPufFuses() {
	XilSKey_PufEfuse PufFuse = {0};
	u32 Status = XST_FAILURE;
	u32 XSK_EFUSEPS_PufData[XSK_EFUSEPS_PUF_NUM_OF_ROWS];

	/* Program PUF Fuses */
	PufFuse.PrgrmPufFuse = XSK_EFUSEPS_WRITE_PUF_FUSE;
	if (PufFuse.PrgrmPufFuse == TRUE) {
		Status = Xil_ConvertStringToHex((char*)XSK_EFUSEPS_PUF_FUSE_DATA,
						XSK_EFUSEPS_PufData,
						XSK_EFUSEPS_PUF_NUM_OF_ROWS * 8U);
		if (Status != XST_SUCCESS) {
			xil_printf("String conversion failed with err:%02x",Status);
			goto END;
		}
		PufFuse.PufFuseData= XSK_EFUSEPS_PufData;
		PufFuse.PufFuseStartRow= XSK_EFUSEPS_PUF_START_ROW;
		PufFuse.PufNumOfFuses= XSK_EFUSEPS_PUF_NUM_OF_ROWS;

		Status = XilSKey_ZynqMp_EfusePs_ProgramPufAsUserFuses(&PufFuse);
		if (Status != XST_SUCCESS) {
			xil_printf("Programming user efuses to PUF Helper Data failed\n\r");
			goto END;
		}
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/****************************************************************************/
/**
*
* This function reads the puf fuses as general purpose data
*
* @return
*		- XST_SUCCESS - In case of Read Success
*		- XST_FAILURE - If read fails
*
*****************************************************************************/
static inline u32 XilSKey_EfusePs_ZynqMp_ReadPufFuses() {
	XilSKey_PufEfuse PufFuse = {0};
	u32 Status = XST_FAILURE;
	u32 Row = 0U;
	u32 XSK_EFUSEPS_PufReadData[XSK_EFUSEPS_PUF_READ_NUM_OF_ROWS];

	/* Read PUF Fuses */
	PufFuse.ReadPufFuse = XSK_EFUSEPS_READ_PUF_FUSE;
	if (PufFuse.ReadPufFuse == TRUE) {
		PufFuse.PufFuseData = XSK_EFUSEPS_PufReadData;
		PufFuse.PufFuseStartRow = XSK_EFUSEPS_PUF_READ_START_ROW;
		PufFuse.PufNumOfFuses = XSK_EFUSEPS_PUF_READ_NUM_OF_ROWS;
		Status = XilSKey_ZynqMp_EfusePs_ReadPufAsUserFuses(&PufFuse);
		if (Status != XST_SUCCESS) {
			xil_printf("Read user fuses from PUF Helper Data failed\n\r");
			goto END;
		}

		for (Row = XSK_EFUSEPS_PUF_READ_START_ROW;
			Row < (XSK_EFUSEPS_PUF_READ_START_ROW +
					XSK_EFUSEPS_PUF_READ_NUM_OF_ROWS); Row++) {
			xil_printf("User_eFuse(PufHd) %d:%08x \n\r",
					Row, XSK_EFUSEPS_PufReadData[Row - XSK_EFUSEPS_PUF_READ_START_ROW]);
		}
	}

	Status = XST_SUCCESS;
END:
	return Status;
}
#endif
