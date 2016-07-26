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
* @file xilskey_eps_zynqmp.c
* This file contains the PS eFUSE API's of ZynqMp to program/read the
* eFUSE array.
*
* @note	None.
*
* </pre>
* MODIFICATION HISTORY:
*
* Ver   Who   Date     Changes
* ----- ----  -------- ------------------------------------------------------
* 4.0   vns   10/01/15 First release
*     vns     10/20/15 Modified XilSKey_ZynqMp_EfusePs_ReadSecCtrlBits API
*                      when reading from efuse memory to return both bits
*                      of secure control feature for RSA enable, PPK hash
*                      bits invalid bits.
* 6.0   vns   07/18/16 PR #1968, Provided User FUSEs single bit programming
*                      Removed JTAG User code programming and reading
*                      feature.
* </pre>
*
*****************************************************************************/

/***************************** Include Files *********************************/

#include "xilskey_eps_zynqmp.h"
#include "xilskey_eps_zynqmp_hw.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions ******************************/

/*
 * XilSKey_UsrFuses holds the User FUSES which needs to be
 * actually programmed
 */
typedef struct {
	u8 UserFuse[XSK_ZYNQMP_EFUSEPS_USER_FUSE_ROW_LEN_IN_BITS];
}XilSKey_UsrFuses;

/***************** Macros (Inline Functions) Definitions ********************/


/************************** Variable Definitions ****************************/


/************************** Function Prototypes *****************************/

static inline u32 XilSKey_ZynqMp_EfusePsWrite_Checks(
				XilSKey_ZynqMpEPs *InstancePtr);
static inline u32 XilSKey_ZynqMp_EfusePs_WriteAndVerify_RowRange(u8 *Data,
		u8 RowStart, u8 RowEnd, XskEfusePs_Type EfuseType);
static inline u32 XilSKey_ZynqMp_EfusePs_PrgrmTbits();
static inline u32 XilSKey_ZynqMp_EfusePs_WriteBit(u8 Row, u8 Column,
						XskEfusePs_Type EfuseType);
static inline void XilSKey_ZynqMp_EfusePs_SetTimerValues(
				XilSKey_ZynqMpEPs *InstancePtr);
static inline u32 XilSKey_ZynqMp_EfusePs_Write_SecCtrl(
				XilSKey_ZynqMpEPs *InstancePtr);
static inline u32 XilSKey_ZynqMp_EfusePs_Write_SecCtrlBits(
				XilSKey_ZynqMpEPs *InstancePtr);
static inline u32 XilSKey_ZynqMp_EfusePs_Write_UsrCtrlBits(
				XilSKey_ZynqMpEPs *InstancePtr);
static inline u32 XilSKey_ZynqMp_EfusePs_Write_XilinxSpecific_CntlBits(
				XilSKey_ZynqMpEPs *InstancePtr);
static inline void XilSKey_ZynqMp_EfusePs_ReadSecCtrlBits_Regs(
				XilSKey_SecCtrlBits *ReadBackSecCtrlBits);
static inline u32 XilSKey_ZynqMp_EfusePs_CheckZeros_BfrPrgrmg(
				XilSKey_ZynqMpEPs *InstancePtr);
static inline u32 XilSKey_ZynqMp_EfusePs_UserFuses_WriteChecks(
	XilSKey_ZynqMpEPs *InstancePtr, XilSKey_UsrFuses *ToBePrgrmd);
static inline u32 XilSKey_ZynqMp_EfusePs_UserFuses_TobeProgrammed(
			u8 *UserFuses_Write, u8 *UserFuses_Read,
			XilSKey_UsrFuses *UserFuses_ToBePrgrmd);
u32 XilSKey_ZynqMp_EfusePs_SetWriteConditions();
u32 XilSKey_ZynqMp_EfusePs_ReadRow(u8 Row, XskEfusePs_Type EfuseType,
							u32 *RowData);
u32 XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(u8 Row, u8 Column,
						XskEfusePs_Type EfuseType);
u32 XilSKey_ZynqMp_EfusePs_CheckForZeros(u8 RowStart, u8 RowEnd,
						XskEfusePs_Type EfuseType);

/************************** Function Definitions *****************************/

/***************************************************************************/
/**
* This function is used to program the PS efuse of ZynqMP.
*
* @param	InstancePtr is the pointer to the XilSKey_ZynqMpEPs.
*
* @return
* 		- XST_SUCCESS if programs successfully.
* 		- Errorcode on failure
*
* @note		Cache reload is required for obtaining updated values.
*
****************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_Write(XilSKey_ZynqMpEPs *InstancePtr)
{
	u32 Status = XST_SUCCESS;
	u8 AesKeyInBits[XSK_ZYNQMP_EFUSEPS_AES_KEY_LEN_IN_BITS] = {0};
	u8 Ppk0InBits[XSK_ZYNQMP_EFUSEPS_PPK_SHA3HASH_LEN_IN_BITS] = {0};
	u8 Ppk1InBits[XSK_ZYNQMP_EFUSEPS_PPK_SHA3HASH_LEN_IN_BITS] = {0};
	u8 SpkIdInBits[XSK_ZYNQMP_EFUSEPS_SPKID_LEN_IN_BITS] = {0};
	XilSKey_UsrFuses UsrFuses_ToPrgm[8] = {{0}};

	/* Conditions to check programming is possible or not */
	Status = XilSKey_ZynqMp_EfusePsWrite_Checks(InstancePtr);
	if (Status != XST_SUCCESS) {
		return (Status + XSK_EFUSEPS_ERROR_BEFORE_PROGRAMMING);
	}

	/* Validation of requested User FUSES bits */
	Status = XilSKey_ZynqMp_EfusePs_UserFuses_WriteChecks(
			InstancePtr, UsrFuses_ToPrgm);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Unlock the controller */
	XilSKey_ZynqMp_EfusePs_CtrlrUnLock();

	/* Check the unlock status */
	if (XilSKey_ZynqMp_EfusePs_CtrlrLockStatus()) {
		return (XSK_EFUSEPS_ERROR_CONTROLLER_LOCK);
	}

	/* Setting all the conditions for writing into eFuse */
	Status = XilSKey_ZynqMp_EfusePs_SetWriteConditions(InstancePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Check for Zeros for Programming eFuse */
	Status = XilSKey_ZynqMp_EfusePs_CheckZeros_BfrPrgrmg(InstancePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (InstancePtr->PrgrmAesKey == TRUE) {
		XilSKey_Efuse_ConvertBitsToBytes(InstancePtr->AESKey,
			AesKeyInBits, XSK_ZYNQMP_EFUSEPS_AES_KEY_LEN_IN_BITS);
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerify_RowRange(
			AesKeyInBits,XSK_ZYNQMP_EFUSEPS_AES_KEY_START_ROW,
				XSK_ZYNQMP_EFUSEPS_AES_KEY_END_ROW,
				XSK_ZYNQMP_EFUSEPS_EFUSE_0);
		if (Status != XST_SUCCESS) {
			Status = (Status + XSK_EFUSEPS_ERROR_WRITE_AES_KEY);
			goto END;
		}
	}

	if (InstancePtr->PrgrmUser0Fuse == TRUE) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerify_RowRange(
			UsrFuses_ToPrgm[XSK_ZYNQMP_EFUSEPS_USR0_FUSE].UserFuse,
			XSK_ZYNQMP_EFUSEPS_USR0_FUSE_ROW,
			XSK_ZYNQMP_EFUSEPS_USR0_FUSE_ROW,
				XSK_ZYNQMP_EFUSEPS_EFUSE_0);
		if (Status != XST_SUCCESS) {
			Status = (Status + XSK_EFUSEPS_ERROR_WRITE_USER0_FUSE);
			goto END;
		}
	}
	if (InstancePtr->PrgrmUser1Fuse == TRUE) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerify_RowRange(
			UsrFuses_ToPrgm[XSK_ZYNQMP_EFUSEPS_USR1_FUSE].UserFuse,
			XSK_ZYNQMP_EFUSEPS_USR1_FUSE_ROW,
			XSK_ZYNQMP_EFUSEPS_USR1_FUSE_ROW,
				XSK_ZYNQMP_EFUSEPS_EFUSE_0);
		if (Status != XST_SUCCESS) {
			Status = (Status + XSK_EFUSEPS_ERROR_WRITE_USER1_FUSE);
			goto END;
		}
	}
	if (InstancePtr->PrgrmUser2Fuse == TRUE) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerify_RowRange(
			UsrFuses_ToPrgm[XSK_ZYNQMP_EFUSEPS_USR2_FUSE].UserFuse,
			XSK_ZYNQMP_EFUSEPS_USR2_FUSE_ROW,
			XSK_ZYNQMP_EFUSEPS_USR2_FUSE_ROW,
				XSK_ZYNQMP_EFUSEPS_EFUSE_0);
		if (Status != XST_SUCCESS) {
			Status = (Status + XSK_EFUSEPS_ERROR_WRITE_USER2_FUSE);
			goto END;
		}
	}
	if (InstancePtr->PrgrmUser3Fuse == TRUE) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerify_RowRange(
			UsrFuses_ToPrgm[XSK_ZYNQMP_EFUSEPS_USR3_FUSE].UserFuse,
			XSK_ZYNQMP_EFUSEPS_USR3_FUSE_ROW,
			XSK_ZYNQMP_EFUSEPS_USR3_FUSE_ROW,
					XSK_ZYNQMP_EFUSEPS_EFUSE_0);
		if (Status != XST_SUCCESS) {
			Status = (Status + XSK_EFUSEPS_ERROR_WRITE_USER3_FUSE);
			goto END;
		}
	}
	if (InstancePtr->PrgrmUser4Fuse == TRUE) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerify_RowRange(
			UsrFuses_ToPrgm[XSK_ZYNQMP_EFUSEPS_USR4_FUSE].UserFuse,
			XSK_ZYNQMP_EFUSEPS_USR4_FUSE_ROW,
			XSK_ZYNQMP_EFUSEPS_USR4_FUSE_ROW,
					XSK_ZYNQMP_EFUSEPS_EFUSE_0);
		if (Status != XST_SUCCESS) {
			Status = (Status + XSK_EFUSEPS_ERROR_WRITE_USER4_FUSE);
			goto END;
		}
	}
	if (InstancePtr->PrgrmUser5Fuse == TRUE) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerify_RowRange(
			UsrFuses_ToPrgm[XSK_ZYNQMP_EFUSEPS_USR5_FUSE].UserFuse,
			XSK_ZYNQMP_EFUSEPS_USR5_FUSE_ROW,
			XSK_ZYNQMP_EFUSEPS_USR5_FUSE_ROW,
				XSK_ZYNQMP_EFUSEPS_EFUSE_0);
		if (Status != XST_SUCCESS) {
			Status = (Status + XSK_EFUSEPS_ERROR_WRITE_USER5_FUSE);
			goto END;
		}
	}
	if (InstancePtr->PrgrmUser6Fuse == TRUE) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerify_RowRange(
			UsrFuses_ToPrgm[XSK_ZYNQMP_EFUSEPS_USR6_FUSE].UserFuse,
			XSK_ZYNQMP_EFUSEPS_USR6_FUSE_ROW,
			XSK_ZYNQMP_EFUSEPS_USR6_FUSE_ROW,
				XSK_ZYNQMP_EFUSEPS_EFUSE_0);
		if (Status != XST_SUCCESS) {
			Status = (Status + XSK_EFUSEPS_ERROR_WRITE_USER6_FUSE);
			goto END;
		}
	}
	if (InstancePtr->PrgrmUser7Fuse == TRUE) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerify_RowRange(
			UsrFuses_ToPrgm[XSK_ZYNQMP_EFUSEPS_USR7_FUSE].UserFuse,
			XSK_ZYNQMP_EFUSEPS_USR7_FUSE_ROW,
			XSK_ZYNQMP_EFUSEPS_USR7_FUSE_ROW,
					XSK_ZYNQMP_EFUSEPS_EFUSE_0);
		if (Status != XST_SUCCESS) {
			Status = (Status + XSK_EFUSEPS_ERROR_WRITE_USER7_FUSE);
			goto END;
		}
	}
	if (InstancePtr->PrgrmSpkID == TRUE) {
		XilSKey_Efuse_ConvertBitsToBytes(InstancePtr->SpkId,
			SpkIdInBits, XSK_ZYNQMP_EFUSEPS_SPKID_LEN_IN_BITS);
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerify_RowRange(
			SpkIdInBits, XSK_ZYNQMP_EFUSEPS_SPK_ID_ROW,
				XSK_ZYNQMP_EFUSEPS_SPK_ID_ROW,
					XSK_ZYNQMP_EFUSEPS_EFUSE_0);
		if (Status != XST_SUCCESS) {
			Status = (Status + XSK_EFUSEPS_ERROR_WRITE_SPK_ID);
			goto END;
		}
	}
	if (InstancePtr->PrgrmPpk0Hash == TRUE) {
		/* Programming SHA3 hash(384 bit) into Efuse PPK0 */
		if (InstancePtr->IsPpk0Sha3Hash == TRUE) {
			XilSKey_Efuse_ConvertBitsToBytes(
				InstancePtr->Ppk0Hash, Ppk0InBits,
				XSK_ZYNQMP_EFUSEPS_PPK_SHA3HASH_LEN_IN_BITS);
			Status =
				XilSKey_ZynqMp_EfusePs_WriteAndVerify_RowRange(
				Ppk0InBits, XSK_ZYNQMP_EFUSEPS_PPK0_START_ROW,
				XSK_ZYNQMP_EFUSEPS_PPK0_SHA3_HASH_END_ROW,
						XSK_ZYNQMP_EFUSEPS_EFUSE_0);
			if (Status != XST_SUCCESS) {
				Status = (Status +
					XSK_EFUSEPS_ERROR_WRITE_PPK0_HASH);
				goto END;
			}
		}
		/* Programming SHA2 hash(256 bit) into Efuse PPK0 */
		else {
			XilSKey_Efuse_ConvertBitsToBytes(
				InstancePtr->Ppk0Hash, Ppk0InBits,
				XSK_ZYNQMP_EFUSEPS_PPK_SHA2HASH_LEN_IN_BITS);
			Status =
				XilSKey_ZynqMp_EfusePs_WriteAndVerify_RowRange(
				Ppk0InBits, XSK_ZYNQMP_EFUSEPS_PPK0_START_ROW,
				XSK_ZYNQMP_EFUSEPS_PPK0_SHA2_HASH_END_ROW,
						XSK_ZYNQMP_EFUSEPS_EFUSE_0);
			if (Status != XST_SUCCESS) {
				Status = (Status +
					XSK_EFUSEPS_ERROR_WRITE_PPK0_HASH);
				goto END;
			}
		}
	}
	if (InstancePtr->PrgrmPpk1Hash == TRUE) {
	/* Programming SHA3 hash(384 bit) into Efuse PPK1 */
		if (InstancePtr->IsPpk1Sha3Hash == TRUE) {
			XilSKey_Efuse_ConvertBitsToBytes(
				InstancePtr->Ppk1Hash, Ppk1InBits,
				XSK_ZYNQMP_EFUSEPS_PPK_SHA3HASH_LEN_IN_BITS);
			Status = XilSKey_ZynqMp_EfusePs_WriteAndVerify_RowRange(
				Ppk1InBits, XSK_ZYNQMP_EFUSEPS_PPK1_START_ROW,
				XSK_ZYNQMP_EFUSEPS_PPK1_SHA3_HASH_END_ROW,
						XSK_ZYNQMP_EFUSEPS_EFUSE_0);
			if (Status != XST_SUCCESS) {
				Status = (Status +
					XSK_EFUSEPS_ERROR_WRITE_PPK1_HASH);
				goto END;
			}
		}
		/* Programming SHA2 hash(256 bit) into Efuse PPK1 */
		else {
			XilSKey_Efuse_ConvertBitsToBytes(
				InstancePtr->Ppk1Hash, Ppk1InBits,
				XSK_ZYNQMP_EFUSEPS_PPK_SHA2HASH_LEN_IN_BITS);
			Status = XilSKey_ZynqMp_EfusePs_WriteAndVerify_RowRange(
				Ppk1InBits, XSK_ZYNQMP_EFUSEPS_PPK1_START_ROW,
				XSK_ZYNQMP_EFUSEPS_PPK1_SHA2_HASH_END_ROW,
						XSK_ZYNQMP_EFUSEPS_EFUSE_0);
			if (Status != XST_SUCCESS) {
				Status = (Status +
					XSK_EFUSEPS_ERROR_WRITE_PPK1_HASH);
				goto END;
			}
		}
	}

	/* Programming Secure and control bits */
	Status = XilSKey_ZynqMp_EfusePs_Write_SecCtrl(InstancePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	/* Lock the controller back */
	XilSKey_ZynqMp_EfusePs_CtrlrLock();
	XilSKey_ZynqMp_EfusePS_PrgrmDisable();

	return Status;

}

/*****************************************************************************/
/**
* This function is used to read the PS efuse secure control bits.
*
* @param	ReadBackSecCtrlBits is the pointer to the XilSKey_SecCtrlBits
*		which holds the read secure control bits.
* @param	ReadOption is a u8 variable which has to be provided by user
*		based on this input reading is happend from cache or from efuse
*		array.
*		- 0	Reads from cache
*		- 1	Reads from efuse array
*
* @return
* 		- XST_SUCCESS if reads successfully
* 		- XST_FAILURE if reading is failed
*
* @note		Cache reload is required for obtaining updated values for
*		ReadOption 0.
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_ReadSecCtrlBits(
		XilSKey_SecCtrlBits *ReadBackSecCtrlBits, u8 ReadOption)
{
	u32 RegData;
	u32 Status;
	u8 DataInBits[32];

	if (ReadOption == 0) {
		XilSKey_ZynqMp_EfusePs_ReadSecCtrlBits_Regs(
					ReadBackSecCtrlBits);
	}
	else {
		/* Unlock the controller */
		XilSKey_ZynqMp_EfusePs_CtrlrUnLock();
		/* Check the unlock status */
		if (XilSKey_ZynqMp_EfusePs_CtrlrLockStatus()) {
			return (XSK_EFUSEPS_ERROR_CONTROLLER_LOCK);
		}

		Status = XilSKey_ZynqMp_EfusePs_ReadRow(
				XSK_ZYNQMP_EFUSEPS_MISC_USR_CTRL_ROW,
				XSK_ZYNQMP_EFUSEPS_EFUSE_0, &RegData);
		if (Status != XST_SUCCESS) {
			goto UNLOCK;
		}
		XilSKey_Efuse_ConvertBitsToBytes((u8 *)&RegData, DataInBits,
					XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);
		ReadBackSecCtrlBits->UserWrLk0 =
				DataInBits[XSK_ZYNQMP_EFUSEPS_USR_WRLK_0];
		ReadBackSecCtrlBits->UserWrLk1 =
				DataInBits[XSK_ZYNQMP_EFUSEPS_USR_WRLK_1];
		ReadBackSecCtrlBits->UserWrLk2 =
				DataInBits[XSK_ZYNQMP_EFUSEPS_USR_WRLK_2];
		ReadBackSecCtrlBits->UserWrLk3 =
				DataInBits[XSK_ZYNQMP_EFUSEPS_USR_WRLK_3];
		ReadBackSecCtrlBits->UserWrLk4 =
				DataInBits[XSK_ZYNQMP_EFUSEPS_USR_WRLK_4];
		ReadBackSecCtrlBits->UserWrLk5 =
				DataInBits[XSK_ZYNQMP_EFUSEPS_USR_WRLK_5];
		ReadBackSecCtrlBits->UserWrLk6 =
				DataInBits[XSK_ZYNQMP_EFUSEPS_USR_WRLK_6];
		ReadBackSecCtrlBits->UserWrLk7 =
				DataInBits[XSK_ZYNQMP_EFUSEPS_USR_WRLK_7];

		Status = XilSKey_ZynqMp_EfusePs_ReadRow(
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_ROW,
			XSK_ZYNQMP_EFUSEPS_EFUSE_0, &RegData);
		if (Status != XST_SUCCESS) {
			goto UNLOCK;
		}
		XilSKey_Efuse_ConvertBitsToBytes((u8 *)&RegData, DataInBits,
				XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);
		ReadBackSecCtrlBits->AesKeyRead =
			DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_AES_RDLK];
		ReadBackSecCtrlBits->AesKeyWrite =
			DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_AES_WRLK];
		ReadBackSecCtrlBits->UseAESOnly =
			DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_ENC_ONLY];
		ReadBackSecCtrlBits->BbramDisable =
			DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_BRAM_DIS];
		ReadBackSecCtrlBits->PMUError =
			DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_ERR_DIS];
		ReadBackSecCtrlBits->JtagDisable =
			DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_JTAG_DIS];
		ReadBackSecCtrlBits->DFTDisable =
			DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_DFT_DIS];
		ReadBackSecCtrlBits->ProgGate0 =
			DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_DIS_PROG_GATE0];
		ReadBackSecCtrlBits->ProgGate1 =
			DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_DIS_PROG_GATE1];
		ReadBackSecCtrlBits->ProgGate2 =
			DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_DIS_PROG_GATE2];
		ReadBackSecCtrlBits->SecureLock =
			DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_LOCK];
		ReadBackSecCtrlBits->RSAEnable =
			(DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_RSA_EN_BIT2] <<
					XSK_ZYNQMP_EFUSEPS_SECTRL_BIT_SHIFT) |
			DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_RSA_EN_BIT1];
		ReadBackSecCtrlBits->PPK0WrLock =
			DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_PPK0_WRLK];
		ReadBackSecCtrlBits->PPK0Revoke =
			(DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_PPK0_INVLD_BIT2] <<
					XSK_ZYNQMP_EFUSEPS_SECTRL_BIT_SHIFT) |
			DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_PPK0_INVLD_BIT1];
		ReadBackSecCtrlBits->PPK1WrLock =
			DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_PPK1_WRLK];
		ReadBackSecCtrlBits->PPK1Revoke =
			(DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_PPK1_INVLD_BIT2] <<
			XSK_ZYNQMP_EFUSEPS_SECTRL_BIT_SHIFT) |
			DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_PPK1_INVLD_BIT1];

		Status = XilSKey_ZynqMp_EfusePs_ReadRow(
			XSK_ZYNQMP_EFUSEPS_XILINX_SPECIFIC_CTRL_BITS_ROW,
					XSK_ZYNQMP_EFUSEPS_EFUSE_0, &RegData);
		if (Status != XST_SUCCESS) {
			goto UNLOCK;
		}
		XilSKey_Efuse_ConvertBitsToBytes((u8 *)&RegData, DataInBits,
					XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);
		ReadBackSecCtrlBits->XilinxSpecfBit1 =
			DataInBits[XSK_ZYNQMP_EFUSEPS_XILINX_SPECFC_BIT1];
		ReadBackSecCtrlBits->XilinxSpecfBit2 =
			DataInBits[XSK_ZYNQMP_EFUSEPS_XILINX_SPECFC_BIT2];
		ReadBackSecCtrlBits->XilinxSpecfBit3 =
			DataInBits[XSK_ZYNQMP_EFUSEPS_XILINX_SPECFC_BIT3];
		ReadBackSecCtrlBits->XilinxSpecfBit4 =
			DataInBits[XSK_ZYNQMP_EFUSEPS_XILINX_SPECFC_BIT4];

UNLOCK:
		XilSKey_ZynqMp_EfusePs_CtrlrUnLock();
	}

	return Status;

}

/*****************************************************************************/
/**
* This function is used to read the PS efuse secure control bits from cache.
*
* @param	ReadBackSecCtrlBits is the pointer to the XilSKey_SecCtrlBits
*		which holds the read secure control bits.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline void XilSKey_ZynqMp_EfusePs_ReadSecCtrlBits_Regs(
				XilSKey_SecCtrlBits *ReadBackSecCtrlBits)
{

	u32 RegData;

	RegData = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
			XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_OFFSET);
	ReadBackSecCtrlBits->UserWrLk0 =
		RegData & XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_0_MASK;
	ReadBackSecCtrlBits->UserWrLk1 =
		(RegData & XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_1_MASK) >>
			XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_1_SHIFT;
	ReadBackSecCtrlBits->UserWrLk2 =
		(RegData & XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_2_MASK) >>
			XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_2_SHIFT;
	ReadBackSecCtrlBits->UserWrLk3 =
		(RegData & XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_3_MASK) >>
			XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_3_SHIFT;
	ReadBackSecCtrlBits->UserWrLk4 =
		(RegData & XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_4_MASK) >>
			XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_4_SHIFT;
	ReadBackSecCtrlBits->UserWrLk5 =
		(RegData & XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_5_MASK) >>
			XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_5_SHIFT;
	ReadBackSecCtrlBits->UserWrLk6 =
		(RegData & XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_6_MASK) >>
			XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_6_SHIFT;
	ReadBackSecCtrlBits->UserWrLk7 =
		(RegData & XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_7_MASK) >>
			XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_7_SHIFT;

	RegData = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_SEC_CTRL_OFFSET);
	ReadBackSecCtrlBits->AesKeyRead =
		(RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_AES_RDLK_MASK);
	ReadBackSecCtrlBits->AesKeyWrite =
		(RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_AES_WRLK_MASK) >>
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_AES_WRLK_SHIFT;
	ReadBackSecCtrlBits->UseAESOnly =
		(RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_ENC_ONLY_MASK) >>
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_ENC_ONLY_SHIFT;
	ReadBackSecCtrlBits->BbramDisable =
		(RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_BBRAM_DIS_MASK) >>
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_BBRAM_DIS_SHIFT;
	ReadBackSecCtrlBits->PMUError =
		(RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_ERR_DIS_MASK) >>
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_ERR_DIS_SHIFT;
	ReadBackSecCtrlBits->JtagDisable =
		(RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_JTAG_DIS_MASK) >>
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_JTAG_DIS_SHIFT;
	ReadBackSecCtrlBits->DFTDisable =
		(RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_DFT_DIS_MASK) >>
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_DFT_DIS_SHIFT;
	ReadBackSecCtrlBits->ProgGate0 =
		(RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PROG_GATE_0_MASK) >>
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PROG_GATE_0_SHIFT;
	ReadBackSecCtrlBits->ProgGate1 =
		(RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PROG_GATE_1_MASK) >>
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PROG_GATE_1_SHIFT;
	ReadBackSecCtrlBits->ProgGate2 =
		(RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PROG_GATE_2_MASK) >>
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PROG_GATE_2_SHIFT;
	ReadBackSecCtrlBits->SecureLock =
		(RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_LOCK_MASK) >>
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_LOCK_SHIFT;
	ReadBackSecCtrlBits->RSAEnable =
		(RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_RSA_EN_MASK) >>
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_RSA_EN_SHIFT;
	ReadBackSecCtrlBits->PPK0WrLock =
		(RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PPK0_WRLK_MASK) >>
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PPK0_WRLK_SHIFT;
	ReadBackSecCtrlBits->PPK0Revoke =
		(RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PPK0_INVLD_MASK) >>
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PPK0_INVLD_SHIFT;
	ReadBackSecCtrlBits->PPK1WrLock =
		(RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PPK1_WRLK_MASK) >>
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PPK1_WRLK_SHIFT;
	ReadBackSecCtrlBits->PPK1Revoke =
		(RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PPK1_INVLD_MASK) >>
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PPK1_INVLD_SHIFT;

	RegData = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_XILINX_SPECIFIC_CTRL_ROW);
	ReadBackSecCtrlBits->XilinxSpecfBit1 =
		(RegData & XSK_ZYNQMP_EFUSEPS_XILINX_SPECFC_CTRLBIT1_MASK) >>
			XSK_ZYNQMP_EFUSEPS_XILINX_SPECFC_CTRLBIT1_SHIFT;
	ReadBackSecCtrlBits->XilinxSpecfBit2 =
		(RegData & XSK_ZYNQMP_EFUSEPS_XILINX_SPECFC_CTRLBIT2_MASK) >>
			XSK_ZYNQMP_EFUSEPS_XILINX_SPECFC_CTRLBIT2_SHIFT;
	ReadBackSecCtrlBits->XilinxSpecfBit3 =
		(RegData & XSK_ZYNQMP_EFUSEPS_XILINX_SPECFC_CTRLBIT3_MASK) >>
			XSK_ZYNQMP_EFUSEPS_XILINX_SPECFC_CTRLBIT3_SHIFT;
	ReadBackSecCtrlBits->XilinxSpecfBit4 =
		(RegData & XSK_ZYNQMP_EFUSEPS_XILINX_SPECFC_CTRLBIT4_MASK) >>
			XSK_ZYNQMP_EFUSEPS_XILINX_SPECFC_CTRLBIT4_SHIFT;

}

/*****************************************************************************/
/**
* This function performs pre checks for programming all the specified bits.
*
* @param	InstancePtr is the pointer to the XilSKey_ZynqMpEPs.
*
* @return
*		XST_SUCCESS - if all the conditions for programming is satisfied
*		Errorcode - if any of the conditions are not met
*
* @note		None.
*
******************************************************************************/
static inline u32 XilSKey_ZynqMp_EfusePsWrite_Checks(
					XilSKey_ZynqMpEPs *InstancePtr)
{
	u32 RowOffset;
	/* Read secure and control bits */
	XilSKey_ZynqMp_EfusePs_ReadSecCtrlBits(
			&(InstancePtr->ReadBackSecCtrlBits), 0);
		if (InstancePtr->PrgrmAesKey == TRUE) {
			if (InstancePtr->ReadBackSecCtrlBits.AesKeyWrite ==
									TRUE) {
				return (XSK_EFUSEPS_ERROR_WRITE_AES_KEY);
			}
		}
		if (InstancePtr->PrgrmSpkID == TRUE) {
			if (XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_PGM_LOCK_OFFSET) != 0x00) {
				return (XSK_EFUSEPS_ERROR_WRITE_SPK_ID);
			}
			/* Check for Zeros */
			if (XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_SPK_ID_OFFSET) != 0x00) {
				return (
				XSK_EFUSEPS_ERROR_SPKID_ALREADY_PROGRAMMED);
			}
		}
		if (((InstancePtr->PrgrmUser0Fuse == TRUE) &&
		(InstancePtr->ReadBackSecCtrlBits.UserWrLk0 == TRUE))) {
			return (XSK_EFUSEPS_ERROR_FUSE_PROTECTED |
					XSK_EFUSEPS_ERROR_WRITE_USER0_FUSE);
		}

		if (((InstancePtr->PrgrmUser1Fuse == TRUE) &&
		(InstancePtr->ReadBackSecCtrlBits.UserWrLk1 == TRUE))) {
			return (XSK_EFUSEPS_ERROR_FUSE_PROTECTED |
					XSK_EFUSEPS_ERROR_WRITE_USER1_FUSE);
		}
		if (((InstancePtr->PrgrmUser2Fuse == TRUE) &&
		(InstancePtr->ReadBackSecCtrlBits.UserWrLk2 == TRUE))) {
			return (XSK_EFUSEPS_ERROR_FUSE_PROTECTED |
					XSK_EFUSEPS_ERROR_WRITE_USER2_FUSE);
		}
		if (((InstancePtr->PrgrmUser3Fuse == TRUE) &&
		(InstancePtr->ReadBackSecCtrlBits.UserWrLk3 == TRUE))) {
			return (XSK_EFUSEPS_ERROR_FUSE_PROTECTED |
					XSK_EFUSEPS_ERROR_WRITE_USER3_FUSE);
		}
		if (((InstancePtr->PrgrmUser4Fuse == TRUE) &&
		(InstancePtr->ReadBackSecCtrlBits.UserWrLk4 == TRUE))) {
			return (XSK_EFUSEPS_ERROR_FUSE_PROTECTED |
					XSK_EFUSEPS_ERROR_WRITE_USER4_FUSE);
		}
		if (((InstancePtr->PrgrmUser5Fuse == TRUE) &&
		(InstancePtr->ReadBackSecCtrlBits.UserWrLk5 == TRUE))) {
			return (XSK_EFUSEPS_ERROR_FUSE_PROTECTED |
				XSK_EFUSEPS_ERROR_WRITE_USER5_FUSE);
		}
		if (((InstancePtr->PrgrmUser6Fuse == TRUE) &&
		(InstancePtr->ReadBackSecCtrlBits.UserWrLk6 == TRUE))) {
			return (XSK_EFUSEPS_ERROR_FUSE_PROTECTED |
					XSK_EFUSEPS_ERROR_WRITE_USER6_FUSE);
		}
		if (((InstancePtr->PrgrmUser7Fuse == TRUE) &&
		(InstancePtr->ReadBackSecCtrlBits.UserWrLk7 == TRUE))) {
			return (XSK_EFUSEPS_ERROR_FUSE_PROTECTED |
					XSK_EFUSEPS_ERROR_WRITE_USER7_FUSE);
		}
		if (InstancePtr->PrgrmPpk0Hash == TRUE) {
			if (InstancePtr->ReadBackSecCtrlBits.PPK0WrLock ==
								TRUE) {
				return (XSK_EFUSEPS_ERROR_WRITE_PPK0_HASH);
			}
			/* Check for Zeros */
			for (RowOffset = XSK_ZYNQMP_EFUSEPS_PPK0_0_OFFSET;
				RowOffset < XSK_ZYNQMP_EFUSEPS_PPK0_11_OFFSET;
				RowOffset = RowOffset + 4) {
				if (XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
							RowOffset) != 0x00) {
					return (
				XSK_EFUSEPS_ERROR_RSA_HASH_ALREADY_PROGRAMMED);
				}
			}
		}
		if (InstancePtr->PrgrmPpk1Hash == TRUE) {
			if (InstancePtr->ReadBackSecCtrlBits.PPK1WrLock ==
								TRUE) {
				return (XSK_EFUSEPS_ERROR_WRITE_PPK1_HASH);
			}
			/* Check for Zeros */
			for (RowOffset = XSK_ZYNQMP_EFUSEPS_PPK1_0_OFFSET;
				RowOffset < XSK_ZYNQMP_EFUSEPS_PPK1_11_OFFSET;
				RowOffset = RowOffset + 4) {
				if (XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
							RowOffset) != 0x00) {
					return (
				XSK_EFUSEPS_ERROR_RSA_HASH_ALREADY_PROGRAMMED);
				}
			}
		}

	return XST_SUCCESS;

}

/*****************************************************************************/
/* This function programs and verifys the Row range provided with provided data.
*
* @param	Data is a pointer to an array which contains data to be
*		programmed.
* @param	RowStart holds the row number from which data programming has to
*		be started.
* @param	RowEnd holds the row number till which data programming has to
*		be performed.
* @param	EfuseType holds the type of the efuse in which programming rows
*		resides in.
*
* @return
*		XST_SUCCESS - On success
*		XST_FAILURE - on Failure
*
* @note		None.
*
******************************************************************************/
static inline u32 XilSKey_ZynqMp_EfusePs_WriteAndVerify_RowRange(u8 *Data,
			u8 RowStart, u8 RowEnd, XskEfusePs_Type EfuseType)
{
	u8 Row;
	u8 Column;
	u32 Status;
	u32 Bit;

	for (Row = RowStart; Row <= RowEnd; Row++) {
		for (Column = 0; Column < 32; Column++) {
			Bit = (Row - RowStart) * XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW;
			Bit += Column;
			if (Data[Bit]) {
				Status =
				XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
							Column, EfuseType);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
			}
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/* This function programs Tbits
*
* @param	None.
*
* @return
*		XST_SUCCESS - On success
*		ErrorCode - on Failure
*
* @note		None.
*
******************************************************************************/
static inline u32 XilSKey_ZynqMp_EfusePs_PrgrmTbits()
{
	u32 RowData;
	u32 Status;
	u8 Column;
	u32 TbitsPrgrmReg;

	/* Enable TBITS programming bit */
	TbitsPrgrmReg = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
			XSK_ZYNQMP_EFUSEPS_TBITS_PRGRMG_EN_OFFSET);

	XilSKey_WriteReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_TBITS_PRGRMG_EN_OFFSET,
		(TbitsPrgrmReg & (~XSK_ZYNQMP_EFUSEPS_TBITS_PRGRMG_EN_MASK)));

	Status = XilSKey_ZynqMp_EfusePs_ReadRow(XSK_ZYNQMP_EFUSEPS_TBITS_ROW,
					XSK_ZYNQMP_EFUSEPS_EFUSE_0, &RowData);
	if (Status != XST_SUCCESS) {
		return Status;
	}
	if (((RowData >> XSK_ZYNQMP_EFUSEPS_TBITS_SHIFT) &
			XSK_ZYNQMP_EFUSEPS_TBITS_MASK) != 0x00) {
		return XSK_EFUSEPS_ERROR_PROGRAMMING_TBIT_PATTERN;
	}

	Status = XilSKey_ZynqMp_EfusePs_ReadRow(XSK_ZYNQMP_EFUSEPS_TBITS_ROW,
			XSK_ZYNQMP_EFUSEPS_EFUSE_2, &RowData);
	if (Status != XST_SUCCESS) {
		return Status;
	}
	if (((RowData >> XSK_ZYNQMP_EFUSEPS_TBITS_SHIFT) &
				XSK_ZYNQMP_EFUSEPS_TBITS_MASK) != 0x00) {
		return XSK_EFUSEPS_ERROR_PROGRAMMING_TBIT_PATTERN;
	}

	Status = XilSKey_ZynqMp_EfusePs_ReadRow(XSK_ZYNQMP_EFUSEPS_TBITS_ROW,
					XSK_ZYNQMP_EFUSEPS_EFUSE_3, &RowData);
	if (Status != XST_SUCCESS) {
		return Status;
	}
	if (((RowData >> XSK_ZYNQMP_EFUSEPS_TBITS_SHIFT) &
				XSK_ZYNQMP_EFUSEPS_TBITS_MASK) != 0x00) {
		return XSK_EFUSEPS_ERROR_PROGRAMMING_TBIT_PATTERN;
	}

	/* Programming Tbits */
	for (Column = 28; Column <= 31; Column++) {
		if ((Column == 28) || (Column == 30)) {
			continue;
		}
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(
				XSK_ZYNQMP_EFUSEPS_TBITS_ROW, Column,
				XSK_ZYNQMP_EFUSEPS_EFUSE_0);
		if (Status != XST_SUCCESS) {
			return Status;
		}
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(
				XSK_ZYNQMP_EFUSEPS_TBITS_ROW, Column,
					XSK_ZYNQMP_EFUSEPS_EFUSE_2);
		if (Status != XST_SUCCESS) {
			return Status;
		}
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(
				XSK_ZYNQMP_EFUSEPS_TBITS_ROW, Column,
					XSK_ZYNQMP_EFUSEPS_EFUSE_3);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}

	XilSKey_WriteReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
		XSK_ZYNQMP_EFUSEPS_TBITS_PRGRMG_EN_OFFSET, TbitsPrgrmReg);

	return XST_SUCCESS;

}

/*****************************************************************************/
/* This function programs and verifys the particular bit of efuse array
*
* @param	Row specifies the row number.
* @param	Column specifies the column number.
* @param	EfuseType specifies the efuse type.
*
* @return
*		XST_SUCCESS - On success
*		ErrorCode - on Failure
*
* @note		None.
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(u8 Row, u8 Column,
						XskEfusePs_Type EfuseType)
{
	u32 RowData;
	u32 Status;

	/*
	 * TBD temperature and voltage check once the sysmon driver is ready
	 * it will be added here
	 */

	/* Programming bit */
	Status = XilSKey_ZynqMp_EfusePs_WriteBit(Row, Column, EfuseType);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	XilSKey_WriteReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
			XSK_ZYNQMP_EFUSEPS_ISR_OFFSET,
			XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_ISR_OFFSET));

	/*
	 * If Row Belongs to AES key can't verify the bit
	 * as AES key can be checked only CRC
	 */
	if ((Row >= XSK_ZYNQMP_EFUSEPS_AES_KEY_START_ROW) &&
			(Row <= XSK_ZYNQMP_EFUSEPS_AES_KEY_END_ROW)) {
		return XST_SUCCESS;
	}

	/* verifying the programmed bit */
	Status = XilSKey_ZynqMp_EfusePs_ReadRow(Row, EfuseType, &RowData);

	if (Status != XST_SUCCESS) {
		return Status;
	}

	if (((RowData >> Column) & 0x01) == 0x00) {
		return XSK_EFUSEPS_ERROR_VERIFICATION;
	}

	return XST_SUCCESS;

}

/*****************************************************************************/
/*
* This function returns particular row data directly from efuse array.
*
* @param	Row specifies the row number to read.
* @param	EfuseType specifies the efuse type.
* @param	RowData is a pointer to 32 bit variable to hold the data read
*		from provided data
*
* @return
*		XST_SUCCESS - On success
*		ErrorCode - on Failure
*
* @note		None.
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_ReadRow(u8 Row, XskEfusePs_Type EfuseType,
								u32 *RowData)
{
	u32 WriteValue;
	u32 ReadValue;

	WriteValue = ((EfuseType << XSK_ZYNQMP_EFUSEPS_RD_ADDR_SHIFT) &
					XSK_ZYNQMP_EFUSEPS_RD_ADDR_MASK) |
			((Row << XSK_ZYNQMP_EFUSEPS_RD_ADDR_ROW_SHIFT) &
					XSK_ZYNQMP_EFUSEPS_RD_ADDR_ROW_MASK);
	XilSKey_WriteReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
		XSK_ZYNQMP_EFUSEPS_RD_ADDR_OFFSET, WriteValue);

	/* Check for read completion */
	ReadValue = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
					XSK_ZYNQMP_EFUSEPS_ISR_OFFSET) &
				(XSK_ZYNQMP_EFUSEPS_ISR_RD_ERR_MASK |
				XSK_ZYNQMP_EFUSEPS_ISR_RD_DONE_MASK);
	while (ReadValue == 0x00) {
		ReadValue =
		XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_ISR_OFFSET) &
			(XSK_ZYNQMP_EFUSEPS_ISR_RD_ERR_MASK |
				XSK_ZYNQMP_EFUSEPS_ISR_RD_DONE_MASK);
	}
	if (ReadValue & XSK_ZYNQMP_EFUSEPS_ISR_RD_ERR_MASK) {
		return XSK_EFUSEPS_ERROR_READ;
	}

	*RowData =
		XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_RD_DATA_OFFSET);

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
* This function programs a particular bit.
*
* @param	Row specifies the row number to program.
* @param	Column specifies the column number to program.
* @param	EfuseType specifies the efuse type.
*
* @return
*		XST_SUCCESS - On success
*		ErrorCode - on Failure
*
* @note		None.
*
******************************************************************************/
static inline u32 XilSKey_ZynqMp_EfusePs_WriteBit(u8 Row, u8 Column,
						XskEfusePs_Type EfuseType)
{

	u32 WriteValue;
	u32 ReadValue;

	WriteValue = ((EfuseType << XSK_ZYNQMP_EFUSEPS_PGM_ADDR_SHIFT) &
					XSK_ZYNQMP_EFUSEPS_PGM_ADDR_MASK) |
			((Row << XSK_ZYNQMP_EFUSEPS_PGM_ADDR_ROW_SHIFT) &
					XSK_ZYNQMP_EFUSEPS_PGM_ADDR_ROW_MASK) |
			(Column & XSK_ZYNQMP_EFUSEPS_PGM_ADDR_COL_MASK);

	XilSKey_WriteReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
		XSK_ZYNQMP_EFUSEPS_PGM_ADDR_OFFSET, WriteValue);

	ReadValue = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
					XSK_ZYNQMP_EFUSEPS_ISR_OFFSET) &
				(XSK_ZYNQMP_EFUSEPS_ISR_PGM_ERR_MASK |
					XSK_ZYNQMP_EFUSEPS_ISR_PGM_DONE_MASK);
	while (ReadValue == 0x00) {
		ReadValue =
		XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
					XSK_ZYNQMP_EFUSEPS_ISR_OFFSET) &
				(XSK_ZYNQMP_EFUSEPS_ISR_PGM_ERR_MASK |
					XSK_ZYNQMP_EFUSEPS_ISR_PGM_DONE_MASK);
	}

	XilSKey_WriteReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
			XSK_ZYNQMP_EFUSEPS_ISR_OFFSET,
		XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
			XSK_ZYNQMP_EFUSEPS_ISR_OFFSET));

	return XST_SUCCESS;

}

/*****************************************************************************/
/*
* This function reloads the cache of efuse so that can be directly read from
* cache.
*
* @param	None.
*
* @return
*		XST_SUCCESS - On success
*		ErrorCode - on Failure
*
* @note		None.
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_CacheLoad()
{
	volatile u32 CacheStatus;

	/* Check the unlock status */
	if (XilSKey_ZynqMp_EfusePs_CtrlrLockStatus()) {
		XilSKey_ZynqMp_EfusePs_CtrlrUnLock();
	}

	/* Reload cache */
	XilSKey_WriteReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
			XSK_ZYNQMP_EFUSEPS_CACHE_LOAD_OFFSET,
			XSK_ZYNQMP_EFUSEPS_CACHE_LOAD_MASK);

	CacheStatus = XilSKey_ZynqMp_EfusePs_Status() &
		(XSK_ZYNQMP_EFUSEPS_STS_CACHE_LOAD_MASK |
			XSK_ZYNQMP_EFUSEPS_STS_CACHE_DONE_MASK);

	/* Waiting for cache loading completion */
	while(CacheStatus == XSK_ZYNQMP_EFUSEPS_STS_CACHE_LOAD_MASK) {
		CacheStatus = XilSKey_ZynqMp_EfusePs_Status() &
			(XSK_ZYNQMP_EFUSEPS_STS_CACHE_LOAD_MASK);
		if ((CacheStatus) == XSK_ZYNQMP_EFUSEPS_STS_CACHE_DONE_MASK) {
			break;
		}
	}

	CacheStatus = XilSKey_ZynqMp_EfusePs_Status();
	if ((CacheStatus & XSK_ZYNQMP_EFUSEPS_STS_CACHE_DONE_MASK) == 0x00) {
		return XSK_EFUSEPS_ERROR_CACHE_LOAD;
	}

	CacheStatus = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
					XSK_ZYNQMP_EFUSEPS_ISR_OFFSET);
	if ((CacheStatus & XSK_ZYNQMP_EFUSEPS_ISR_CACHE_ERR_MASK) ==
			XSK_ZYNQMP_EFUSEPS_ISR_CACHE_ERR_MASK) {
		return XSK_EFUSEPS_ERROR_CACHE_LOAD;
	}

	return XST_SUCCESS;

}

/*****************************************************************************/
/*
* This function sets all the required parameters to program efuse array.
*
* @param	InstancePtr is an instance of efuseps of Zynq MP.
*
* @return
*		XST_SUCCESS - On success
*		ErrorCode - on Failure
*
* @note		None.
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_SetWriteConditions(XilSKey_ZynqMpEPs *InstancePtr)
{
	u32 ReadReg;
	u32 Status;

	/*
	 * TBD temperature and voltage check once the sysmon driver is ready
	 * it will be added here
	 */

	/* Enable Program enable bit */
	XilSKey_ZynqMp_EfusePS_PrgrmEn();

	/* Setting Timing Constraints */
	if (InstancePtr->IntialisedTimer != TRUE) {
		XilSKey_ZynqMp_EfusePs_SetTimerValues(InstancePtr);
	}

	/* Read status and verify Tbits are read properly or not */
	ReadReg = XilSKey_ZynqMp_EfusePs_Status();
	if ((ReadReg & (XSK_ZYNQMP_EFUSEPS_STS_0_TBIT_MASK |
		XSK_ZYNQMP_EFUSEPS_STS_2_TBIT_MASK |
		XSK_ZYNQMP_EFUSEPS_STS_3_TBIT_MASK)) !=
		(XSK_ZYNQMP_EFUSEPS_STS_0_TBIT_MASK |
		XSK_ZYNQMP_EFUSEPS_STS_2_TBIT_MASK |
		XSK_ZYNQMP_EFUSEPS_STS_3_TBIT_MASK)) {
		/* program Tbits */
		Status = XilSKey_ZynqMp_EfusePs_PrgrmTbits();
		if (Status != XST_SUCCESS) {
			return Status;
		}
		XilSKey_ZynqMp_EfusePs_CacheLoad();
	}

	return XST_SUCCESS;

}

/*****************************************************************************/
/*
* This function sets timers for programming and reading from efuse.
*
* @param	InstancePtr is an instance of efuseps of Zynq MP.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline void XilSKey_ZynqMp_EfusePs_SetTimerValues(
					XilSKey_ZynqMpEPs *InstancePtr)
{
	float RefClk;
	u32 ReadReg;

	ReadReg = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_CFG_OFFSET) &
			~(XSK_ZYNQMP_EFUSEPS_CFG_CLK_SEL_MASK);
	ReadReg = ReadReg |
		((XSK_ZYNQMP_EFUSEPS_CFG_MARGIN_2_RD <<
			XSK_ZYNQMP_EFUSEPS_CFG_MARGIN_RD_SHIFT) |
			(XSK_ZYNQMP_EFUSEPS_CFG_CLK_SEL_MASK));
	XilSKey_WriteReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
			XSK_ZYNQMP_EFUSEPS_CFG_OFFSET, ReadReg);

	/* Initialized Timer */
	RefClk = XSK_ZYNQMP_EFUSEPS_PS_REF_CLK_FREQ;
	XilSKey_WriteReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
			XSK_ZYNQMP_EFUSEPS_TPGM_OFFSET,
			((u32)XilSKey_ZynqMp_EfusePs_Tprgrm(RefClk) &
			XSK_ZYNQMP_EFUSEPS_TPGM_VAL_MASK));
	XilSKey_WriteReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
			XSK_ZYNQMP_EFUSEPS_TRD_OFFSET,
			((u32)XilSKey_ZynqMp_EfusePs_Trd(RefClk) &
			XSK_ZYNQMP_EFUSEPS_TRD_VAL_MASK));
	XilSKey_WriteReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
			XSK_ZYNQMP_EFUSEPS_TSU_H_PS_OFFSET,
			((u32)XilSKey_ZynqMp_EfusePs_TsuHPs(RefClk) &
			XSK_ZYNQMP_EFUSEPS_TSU_H_PS_VAL_MASK));
	XilSKey_WriteReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
			XSK_ZYNQMP_EFUSEPS_TSU_H_PS_CS_OFFSET,
			((u32)XilSKey_ZynqMp_EfusePs_TsuHPsCs(RefClk) &
			XSK_ZYNQMP_EFUSEPS_TSU_H_PS_CS_VAL_MASK));
	XilSKey_WriteReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
			XSK_ZYNQMP_EFUSEPS_TSU_H_CS_OFFSET,
			((u32)XilSKey_ZynqMp_EfusePs_TsuHCs(RefClk) &
			XSK_ZYNQMP_EFUSEPS_TSU_H_PS_CS_VAL_DEFVAL));

	InstancePtr->IntialisedTimer = TRUE;
}

/*****************************************************************************/
/*
* This function programs secure control bits specified by user.
*
* @param	InstancePtr is an instance of efuseps of Zynq MP.
*
* @return
*		XST_SUCCESS - On success
*		ErrorCode - on Failure
*
* @note		None.
*
******************************************************************************/
static inline u32 XilSKey_ZynqMp_EfusePs_Write_SecCtrl(
				XilSKey_ZynqMpEPs *InstancePtr)
{

	u32 Status;

	/* Programming Secure and control bits of eFuse */
	Status = XilSKey_ZynqMp_EfusePs_Write_SecCtrlBits(InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Programming User control bits */
	Status = XilSKey_ZynqMp_EfusePs_Write_UsrCtrlBits(InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Programming Xilinx specific control bits */
	Status =
	XilSKey_ZynqMp_EfusePs_Write_XilinxSpecific_CntlBits(InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	return XST_SUCCESS;

}

/*****************************************************************************/
/*
* This function programs secure control bits of efuse
*
* @param	InstancePtr is an instance of efuseps of ZynqMp.
*
* @return
*		XST_SUCCESS - On success
*		ErrorCode - on Failure
*
* @note		None.
*
******************************************************************************/
static inline u32 XilSKey_ZynqMp_EfusePs_Write_SecCtrlBits(
				XilSKey_ZynqMpEPs *InstancePtr)
{
	u32 Status;
	XskEfusePs_Type EfuseType = XSK_ZYNQMP_EFUSEPS_EFUSE_0;
	u32 Row;

	Row = XSK_ZYNQMP_EFUSEPS_SEC_CTRL_ROW;
	if ((InstancePtr->PrgrmgSecCtrlBits.AesKeyRead != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.AesKeyRead == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
				XSK_ZYNQMP_EFUSEPS_SEC_AES_RDLK, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.AesKeyWrite != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.AesKeyWrite == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_SEC_AES_WRLK, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.UseAESOnly != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.UseAESOnly == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_SEC_ENC_ONLY, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.BbramDisable != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.BbramDisable == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_SEC_BRAM_DIS, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.PMUError != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.PMUError == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_SEC_ERR_DIS, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.JtagDisable != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.JtagDisable == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_SEC_JTAG_DIS, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.DFTDisable != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.DFTDisable == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_SEC_DFT_DIS, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.ProgGate0 != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.ProgGate0 == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_SEC_DIS_PROG_GATE0, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.ProgGate1 != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.ProgGate1 == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_SEC_DIS_PROG_GATE1, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.ProgGate2 != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.ProgGate2 == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_SEC_DIS_PROG_GATE2, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.SecureLock != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.SecureLock == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_SEC_LOCK, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.RSAEnable != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.RSAEnable == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_SEC_RSA_EN_BIT1, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_SEC_RSA_EN_BIT2, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.PPK0WrLock != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.PPK0WrLock == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_SEC_PPK0_WRLK, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.PPK0Revoke != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.PPK0Revoke == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_SEC_PPK0_INVLD_BIT1, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_SEC_PPK0_INVLD_BIT2, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.PPK1WrLock != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.PPK1WrLock == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_SEC_PPK1_WRLK, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.PPK1Revoke != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.PPK1Revoke == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_SEC_PPK1_INVLD_BIT1, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_SEC_PPK1_INVLD_BIT2, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}

	return XST_SUCCESS;

}

/*****************************************************************************/
/*
* This function programs misc user control bits of efuse
*
* @param	InstancePtr is an instance of efuseps of ZynqMp.
*
* @return
*		XST_SUCCESS - On success
*		ErrorCode - on Failure
*
* @note		None.
*
******************************************************************************/
static inline u32 XilSKey_ZynqMp_EfusePs_Write_UsrCtrlBits(
					XilSKey_ZynqMpEPs *InstancePtr)
{
	u32 Status;
	XskEfusePs_Type EfuseType = XSK_ZYNQMP_EFUSEPS_EFUSE_0;
	u32 Row = XSK_ZYNQMP_EFUSEPS_MISC_USR_CTRL_ROW;

	if ((InstancePtr->PrgrmgSecCtrlBits.UserWrLk0 != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.UserWrLk0 == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
				XSK_ZYNQMP_EFUSEPS_USR_WRLK_0, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.UserWrLk1 != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.UserWrLk1 == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_USR_WRLK_1, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.UserWrLk2 != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.UserWrLk2 == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_USR_WRLK_2, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.UserWrLk3 != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.UserWrLk3 == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_USR_WRLK_3, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.UserWrLk4 != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.UserWrLk4 == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_USR_WRLK_4, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.UserWrLk5 != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.UserWrLk5 == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_USR_WRLK_5, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.UserWrLk6 != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.UserWrLk6 == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_USR_WRLK_6, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.UserWrLk7 != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.UserWrLk7 == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_USR_WRLK_7, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}

	return XST_SUCCESS;

}

/*****************************************************************************/
/*
* This function programs Xilinx control bits of efuse
*
* @param	InstancePtr is an instance of efuseps of ZynqMp.
*
* @return
*		XST_SUCCESS - On success
*		ErrorCode - on Failure
*
* @note		None.
*
******************************************************************************/
static inline u32 XilSKey_ZynqMp_EfusePs_Write_XilinxSpecific_CntlBits(
					XilSKey_ZynqMpEPs *InstancePtr)
{
	u32 Status;
	XskEfusePs_Type EfuseType = XSK_ZYNQMP_EFUSEPS_EFUSE_0;
	u32 Row = XSK_ZYNQMP_EFUSEPS_XILINX_SPECIFIC_CTRL_BITS_ROW;

	if ((InstancePtr->PrgrmgSecCtrlBits.XilinxSpecfBit1 != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.XilinxSpecfBit1 == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_XILINX_SPECFC_BIT1, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.XilinxSpecfBit2 != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.XilinxSpecfBit2 == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_XILINX_SPECFC_BIT2, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.XilinxSpecfBit3 != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.XilinxSpecfBit3 == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_XILINX_SPECFC_BIT3, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.XilinxSpecfBit4 != 0x00) &&
		(InstancePtr->ReadBackSecCtrlBits.XilinxSpecfBit4 == 0x00)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_XILINX_SPECFC_BIT4, EfuseType);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}

	return XST_SUCCESS;

}

/*****************************************************************************/
/*
* This function performs CRC check of AES key
*
* @param	CrcValue is a 32 bit CRC of AES key.
*
* @return
*		XST_SUCCESS - On success
*		ErrorCode - on Failure
*
* @note		For Calculating CRC of AES key key use XilSKey_CrcCalculation()
* 			API.
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_CheckAesKeyCrc(u32 CrcValue)
{

	u32 Status = XST_SUCCESS;
	u32 ReadReg;

	/* Check the unlock status */
	if (XilSKey_ZynqMp_EfusePs_CtrlrLockStatus()) {
		/* Unlock the controller */
		XilSKey_ZynqMp_EfusePs_CtrlrUnLock();
	}

	/* writing CRC value to check AES key's CRC */
	XilSKey_WriteReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
		XSK_ZYNQMP_EFUSEPS_AES_CRC_OFFSET,
		(CrcValue & XSK_ZYNQMP_EFUSEPS_AES_CRC_VAL_MASK));

	/* Poll for CRC Done bit */
	ReadReg = XilSKey_ZynqMp_EfusePs_Status();

	while ((ReadReg & XSK_ZYNQMP_EFUSEPS_STS_AES_CRC_DONE_MASK) == 0x00) {
		ReadReg = XilSKey_ZynqMp_EfusePs_Status();
	}

	if ((ReadReg & XSK_ZYNQMP_EFUSEPS_STS_AES_CRC_PASS_MASK) == 0x00) {
		Status = XST_FAILURE;
	}


	return Status;

}

/*****************************************************************************/
/*
* This function is used to read user fuse from efuse based on read option.
*
* @param	UseFusePtr is a pointer to an array which holds the readback
*		user fuse in.
* @param	UserFuse_Num is a variable which holds the user fuse number.
*		Range is (User fuses: 0 to 7)
* @param	ReadOption is a u8 variable which has to be provided by user
*		based on this input reading is happend from cache or from efuse
*		array.
*		- 0	Reads from cache
*		- 1	Reads from efuse array
*
* @return
*		XST_SUCCESS - On success
*		ErrorCode - on Failure
*
* @note		None.
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_ReadUserFuse(u32 *UseFusePtr, u8 UserFuse_Num,
							u8 ReadOption)
{
	u32 Status = XST_SUCCESS;
	u32 Row;
	XskEfusePs_Type EfuseType = XSK_ZYNQMP_EFUSEPS_EFUSE_0;
	u32 RegNum;

	if (ReadOption ==  0) {
		*UseFusePtr = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				(XSK_ZYNQMP_EFUSEPS_USER_0_OFFSET
				+ (UserFuse_Num * 4)));
	}
	else {
		/* Unlock the controller */
		XilSKey_ZynqMp_EfusePs_CtrlrUnLock();
		/* Check the unlock status */
		if (XilSKey_ZynqMp_EfusePs_CtrlrLockStatus()) {
			return (XSK_EFUSEPS_ERROR_CONTROLLER_LOCK);
		}

		Status = XilSKey_ZynqMp_EfusePs_ReadRow(
			XSK_ZYNQMP_EFUSEPS_USR0_FUSE_ROW + UserFuse_Num,
			EfuseType, UseFusePtr);
		if (Status != XST_SUCCESS) {
			goto UNLOCK;
		}


UNLOCK:
		/* Lock the controller back */
		XilSKey_ZynqMp_EfusePs_CtrlrLock();
	}

	return Status;

}

/*****************************************************************************/
/*
* This function is used to read PPK0 hash from efuse based on read option.
*
* @param	Ppk0Hash is a pointer to an array which holds the readback
*		PPK0 hash in.
* @param	ReadOption is a u8 variable which has to be provided by user
*		based on this input reading is happend from cache or from efuse
*		array.
*		- 0	Reads from cache
*		- 1	Reads from efuse array
*
* @return
*		XST_SUCCESS - On success
*		ErrorCode - on Failure
*
* @note		None.
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_ReadPpk0Hash(u32 *Ppk0Hash, u8 ReadOption)
{
	u32 Status = XST_SUCCESS;
	u32 Row;
	XskEfusePs_Type EfuseType = XSK_ZYNQMP_EFUSEPS_EFUSE_0;
	s32 RegNum;
	u32 DataRead;

	if (ReadOption == 0) {
		for (RegNum = (XSK_ZYNQMP_EFUSEPS_PPK_HASH_REG_NUM - 1);
		RegNum >= 0; RegNum--) {
			DataRead = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_PPK0_0_OFFSET
						+ (RegNum * 4));
			XilSKey_EfusePs_ConvertBytesBeToLe((u8 *)&DataRead, (u8 *)Ppk0Hash, 8);
			Ppk0Hash++;
		}
	}
	else {
		/* Unlock the controller */
		XilSKey_ZynqMp_EfusePs_CtrlrUnLock();
		/* Check the unlock status */
		if (XilSKey_ZynqMp_EfusePs_CtrlrLockStatus()) {
			return (XSK_EFUSEPS_ERROR_CONTROLLER_LOCK);
		}

		for (Row = XSK_ZYNQMP_EFUSEPS_PPK0_SHA3_HASH_END_ROW;
			Row >= XSK_ZYNQMP_EFUSEPS_PPK0_START_ROW;
								Row--) {
			Status = XilSKey_ZynqMp_EfusePs_ReadRow(Row,
						EfuseType, &DataRead);
			XilSKey_EfusePs_ConvertBytesBeToLe((u8 *)&DataRead, (u8 *)Ppk0Hash, 8);
			if (Status != XST_SUCCESS) {
				goto UNLOCK;
			}
			Ppk0Hash++;
		}

UNLOCK:
		/* Lock the controller back */
		XilSKey_ZynqMp_EfusePs_CtrlrLock();
	}

	return Status;

}

/*****************************************************************************/
/*
* This function is used to read PPK1 hash from efuse based on read option.
*
* @param	Ppk1Hash is a pointer to an array which holds the readback
*		PPK1 hash in.
* @param	ReadOption is a u8 variable which has to be provided by user
*		based on this input reading is happend from cache or from efuse
*		array.
*		- 0	Reads from cache
*		- 1	Reads from efuse array
*
* @return
*		XST_SUCCESS - On success
*		ErrorCode - on Failure
*
* @note		None.
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_ReadPpk1Hash(u32 *Ppk1Hash, u8 ReadOption)
{
	u32 Status = XST_SUCCESS;
	u32 Row;
	XskEfusePs_Type EfuseType = XSK_ZYNQMP_EFUSEPS_EFUSE_0;
	s32 RegNum;
	u32 DataRead;

	if (ReadOption == 0) {
		for (RegNum = (XSK_ZYNQMP_EFUSEPS_PPK_HASH_REG_NUM - 1);
			RegNum >= 0;
								RegNum--) {
			DataRead = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
					XSK_ZYNQMP_EFUSEPS_PPK1_0_OFFSET
							+ (RegNum * 4));
			XilSKey_EfusePs_ConvertBytesBeToLe((u8 *)&DataRead, (u8 *)Ppk1Hash, 8);
			Ppk1Hash++;
		}
	}
	else {
		/* Unlock the controller */
		XilSKey_ZynqMp_EfusePs_CtrlrUnLock();
		/* Check the unlock status */
		if (XilSKey_ZynqMp_EfusePs_CtrlrLockStatus()) {
			return (XSK_EFUSEPS_ERROR_CONTROLLER_LOCK);
		}

		for (Row = XSK_ZYNQMP_EFUSEPS_PPK1_SHA3_HASH_END_ROW;
		Row >= XSK_ZYNQMP_EFUSEPS_PPK1_START_ROW; Row--) {
			Status = XilSKey_ZynqMp_EfusePs_ReadRow(Row,
						EfuseType, &DataRead);
			if (Status != XST_SUCCESS) {
				goto UNLOCK;
			}
			XilSKey_EfusePs_ConvertBytesBeToLe((u8 *)&DataRead, (u8 *)Ppk1Hash, 8);
			Ppk1Hash++;
		}

UNLOCK:
		/* Lock the controller back */
		XilSKey_ZynqMp_EfusePs_CtrlrLock();
	}

	return Status;

}

/*****************************************************************************/
/*
* This function is used to read SPKID from efuse based on read option.
*
* @param	SPK ID is a pointer to a 32 bit variable which holds SPK ID.
* @param	ReadOption is a u8 variable which has to be provided by user
*		based on this input reading is happend from cache or from efuse
*		array.
*		- 0	Reads from cache
*		- 1	Reads from efuse array
*
* @return
*		XST_SUCCESS - On success
*		ErrorCode - on Failure
*
* @note		None.
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_ReadSpkId(u32 *SpkId, u8 ReadOption)
{
	u32 Status = XST_SUCCESS;
	XskEfusePs_Type EfuseType = XSK_ZYNQMP_EFUSEPS_EFUSE_0;

	if (ReadOption == 0) {
		*SpkId = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_SPK_ID_OFFSET);
	}
	else {
		/* Unlock the controller */
		XilSKey_ZynqMp_EfusePs_CtrlrUnLock();
		/* Check the unlock status */
		if (XilSKey_ZynqMp_EfusePs_CtrlrLockStatus()) {
			return (XSK_EFUSEPS_ERROR_CONTROLLER_LOCK);
		}

		Status = XilSKey_ZynqMp_EfusePs_ReadRow(
			XSK_ZYNQMP_EFUSEPS_SPK_ID_ROW, EfuseType, SpkId);
		if (Status != XST_SUCCESS) {
			goto UNLOCK;
		}

UNLOCK:
		/* Lock the controller back */
		XilSKey_ZynqMp_EfusePs_CtrlrLock();

	}

	return Status;
}

/*****************************************************************************/
/*
* This function is used to read DNA from efuse.
*
* @param	DnaRead is a pointer to 32 bit variable which holds the
*		readback DNA in.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XilSKey_ZynqMp_EfusePs_ReadDna(u32 *DnaRead)
{
	u32 *DnaPtr = DnaRead;

	*DnaPtr = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_DNA_0_OFFSET);
	DnaPtr++;
	*DnaPtr = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_DNA_1_OFFSET);
	DnaPtr++;
	*DnaPtr = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_DNA_2_OFFSET);

}

/*****************************************************************************/
/*
* This function is used verify efuse keys for Zeros
*
* @param	RowStart is row number from which verification has to be
*		started.
* @param	RowEnd is row number till which verification has to be
*		ended.
* @param	EfuseType is the type of the efuse in which these rows reside.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_CheckForZeros(u8 RowStart, u8 RowEnd,
						XskEfusePs_Type EfuseType)
{

	u32 Status = XST_SUCCESS;
	u8 Row;
	u32 RowData;

	for (Row = RowStart; Row <= RowEnd; Row++) {
		Status = XilSKey_ZynqMp_EfusePs_ReadRow(Row, EfuseType,
								&RowData);
		if (Status != XST_SUCCESS) {
			return Status;
		}
		if (RowData != 0x00) {
			return XST_FAILURE;
		}
	}

	return Status;
}

/*****************************************************************************/
/*
* This function is used verify efuse keys for Zeros before programming.
*
* @param	InstancePtr is a pointer to efuse ps instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline u32 XilSKey_ZynqMp_EfusePs_CheckZeros_BfrPrgrmg(
					XilSKey_ZynqMpEPs *InstancePtr)
{
	u32 Status;

	/* Check for AES key with All zeros */
	if (InstancePtr->PrgrmAesKey == TRUE) {
		Status = XilSKey_ZynqMp_EfusePs_CheckAesKeyCrc(
				XSK_ZYNQMP_EFUSEPS_CRC_AES_ZEROS);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;/* Error code of AES is non zero */
		}
	}

	/* Check for SPK ID zeros */
	if (InstancePtr->PrgrmSpkID == TRUE) {
		Status = XilSKey_ZynqMp_EfusePs_CheckForZeros(
			XSK_ZYNQMP_EFUSEPS_SPK_ID_ROW,
			XSK_ZYNQMP_EFUSEPS_SPK_ID_ROW,
			XSK_ZYNQMP_EFUSEPS_EFUSE_0);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	/* Check Zeros for PPK0 hash */
	if (InstancePtr->PrgrmPpk0Hash == TRUE) {
		Status = XilSKey_ZynqMp_EfusePs_CheckForZeros(
			XSK_ZYNQMP_EFUSEPS_PPK0_START_ROW,
			XSK_ZYNQMP_EFUSEPS_PPK0_SHA3_HASH_END_ROW,
				XSK_ZYNQMP_EFUSEPS_EFUSE_0);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	/* Check Zeros for PPK0 hash */
	if (InstancePtr->PrgrmPpk1Hash == TRUE) {
		Status = XilSKey_ZynqMp_EfusePs_CheckForZeros(
			XSK_ZYNQMP_EFUSEPS_PPK1_START_ROW,
			XSK_ZYNQMP_EFUSEPS_PPK1_SHA3_HASH_END_ROW,
					XSK_ZYNQMP_EFUSEPS_EFUSE_0);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;

}

/*****************************************************************************/
/*
* This function throws an error if user requests already programmed User FUSE
* bit to revert, and copies the bits to be programmed in particular row into
* provided UserFuses_TobePrgrmd pointer.
*
* @param	UserFuses_Write is a pointer to user requested programming bits
*		of an User FUSE row.
* @param	UserFuses_Read is a pointer to already programmed bits of User
*		FUSE row on eFUSE.
* @param	UserFuses_TobePrgrmd holds User FUSE row bits which needs to be
*		programmed actually.
*
* @return
*		- XST_FAILURE: Returns error if user requests programmed bit to
*		revert
*		- XST_SUCCESS: If User requests valid bits.
*
* @note		If user requests a non-zero bit for making to zero throws an
*		error which is not possible
*
******************************************************************************/
static inline u32 XilSKey_ZynqMp_EfusePs_UserFuses_TobeProgrammed(
				u8 *UserFuses_Write, u8 *UserFuses_Read,
				XilSKey_UsrFuses *UserFuses_TobePrgrmd)
{
	u32 UserFuseColumn;

	for (UserFuseColumn = 0; UserFuseColumn < 32; UserFuseColumn++) {
	/* If user requests a non-zero bit for making to zero throws an error*/
		if ((UserFuses_Write[UserFuseColumn] == 0) &&
			(UserFuses_Read[UserFuseColumn] == 1)) {
			return XST_FAILURE;
		}
		if ((UserFuses_Write[UserFuseColumn] == 1) &&
			(UserFuses_Read[UserFuseColumn] == 0)) {
			UserFuses_TobePrgrmd->UserFuse[UserFuseColumn] = 1;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
* This function throws an error if user requests already programmed User FUSE
* bit to revert, and copies the User FUSE bits which needs actually to be
* programmed into provided UserFuses_TobePrgrmd pointer.
*
* @param	InstancePtr is a pointer to efuse ps instance.
* @param	UserFuses_TobePrgrmd holds User FUSE bits which needs to be
*		actually programmed.
*
* @return
*		- ErrorCode if user requests programmed bit to revert.
*		- XST_SUCCESS if user requests valid bits
*
* @note		If user requests a non-zero bit for making to zero throws an
*		error which is not possible
*
******************************************************************************/
static inline u32 XilSKey_ZynqMp_EfusePs_UserFuses_WriteChecks(
	XilSKey_ZynqMpEPs *InstancePtr, XilSKey_UsrFuses *UserFuses_ToBePrgrmd)
{
	u8 UserFuses_Read[8][32] = {{0}};
	u8 UserFuses_Write[8][32] = {{0}};
	u32 UserFuseRead;

	if (InstancePtr->PrgrmUser0Fuse == TRUE) {
		UserFuseRead = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
					XSK_ZYNQMP_EFUSEPS_USER_0_OFFSET);
		XilSKey_Efuse_ConvertBitsToBytes((u8 *)&UserFuseRead,
			&UserFuses_Read[XSK_ZYNQMP_EFUSEPS_USR0_FUSE][0],
			XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);
		XilSKey_Efuse_ConvertBitsToBytes(InstancePtr->User0Fuses,
			&(UserFuses_Write[XSK_ZYNQMP_EFUSEPS_USR0_FUSE][0]),
			XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);

	/* Checking whether requested User FUSE bit programming is possible */
		if (XilSKey_ZynqMp_EfusePs_UserFuses_TobeProgrammed(
			&UserFuses_Write[XSK_ZYNQMP_EFUSEPS_USR0_FUSE][0],
			&UserFuses_Read[XSK_ZYNQMP_EFUSEPS_USR0_FUSE][0],
		(UserFuses_ToBePrgrmd + XSK_ZYNQMP_EFUSEPS_USR0_FUSE)) !=
							XST_SUCCESS) {
			return (XSK_EFUSEPS_ERROR_WRITE_USER0_FUSE +
				XSK_EFUSEPS_ERROR_USER_BIT_CANT_REVERT);
		}
	}

	if (InstancePtr->PrgrmUser1Fuse == TRUE) {
		UserFuseRead = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_USER_1_OFFSET);
		XilSKey_Efuse_ConvertBitsToBytes((u8 *)&UserFuseRead,
			&UserFuses_Read[XSK_ZYNQMP_EFUSEPS_USR1_FUSE][0],
			XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);
		XilSKey_Efuse_ConvertBitsToBytes(InstancePtr->User1Fuses,
		&(UserFuses_Write[XSK_ZYNQMP_EFUSEPS_USR1_FUSE][0]),
			XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);

	/* Checking whether requested User FUSE bit programming is possible */
		if (XilSKey_ZynqMp_EfusePs_UserFuses_TobeProgrammed(
			&UserFuses_Write[XSK_ZYNQMP_EFUSEPS_USR1_FUSE][0],
			&UserFuses_Read[XSK_ZYNQMP_EFUSEPS_USR1_FUSE][0],
		(UserFuses_ToBePrgrmd + XSK_ZYNQMP_EFUSEPS_USR1_FUSE)) !=
							XST_SUCCESS) {
			return (XSK_EFUSEPS_ERROR_WRITE_USER1_FUSE +
				XSK_EFUSEPS_ERROR_USER_BIT_CANT_REVERT);
		}
	}

	if (InstancePtr->PrgrmUser2Fuse == TRUE) {
		UserFuseRead = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_USER_2_OFFSET);
		XilSKey_Efuse_ConvertBitsToBytes((u8 *)&UserFuseRead,
			&UserFuses_Read[XSK_ZYNQMP_EFUSEPS_USR2_FUSE][0],
			XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);
		XilSKey_Efuse_ConvertBitsToBytes(InstancePtr->User2Fuses,
			&(UserFuses_Write[XSK_ZYNQMP_EFUSEPS_USR2_FUSE][0]),
			XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);
	/* Checking whether requested User FUSE bit programming is possible */
		if (XilSKey_ZynqMp_EfusePs_UserFuses_TobeProgrammed(
			&UserFuses_Write[XSK_ZYNQMP_EFUSEPS_USR2_FUSE][0],
			&UserFuses_Read[XSK_ZYNQMP_EFUSEPS_USR2_FUSE][0],
		(UserFuses_ToBePrgrmd + XSK_ZYNQMP_EFUSEPS_USR2_FUSE)) !=
							XST_SUCCESS) {
			return (XSK_EFUSEPS_ERROR_WRITE_USER2_FUSE +
				XSK_EFUSEPS_ERROR_USER_BIT_CANT_REVERT);
		}
	}

	if (InstancePtr->PrgrmUser3Fuse == TRUE) {
		UserFuseRead = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
					XSK_ZYNQMP_EFUSEPS_USER_3_OFFSET);
		XilSKey_Efuse_ConvertBitsToBytes((u8 *)&UserFuseRead,
			&UserFuses_Read[XSK_ZYNQMP_EFUSEPS_USR3_FUSE][0],
				XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);
		XilSKey_Efuse_ConvertBitsToBytes(InstancePtr->User3Fuses,
		&(UserFuses_Write[XSK_ZYNQMP_EFUSEPS_USR3_FUSE][0]),
			XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);
	/* Checking whether requested User FUSE bit programming is possible */
		if (XilSKey_ZynqMp_EfusePs_UserFuses_TobeProgrammed(
			&UserFuses_Write[XSK_ZYNQMP_EFUSEPS_USR3_FUSE][0],
			&UserFuses_Read[XSK_ZYNQMP_EFUSEPS_USR3_FUSE][0],
		(UserFuses_ToBePrgrmd + XSK_ZYNQMP_EFUSEPS_USR3_FUSE)) !=
							XST_SUCCESS) {
			return (XSK_EFUSEPS_ERROR_WRITE_USER3_FUSE +
				XSK_EFUSEPS_ERROR_USER_BIT_CANT_REVERT);
		}
	}

	if (InstancePtr->PrgrmUser4Fuse == TRUE) {
		UserFuseRead = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
					XSK_ZYNQMP_EFUSEPS_USER_4_OFFSET);
		XilSKey_Efuse_ConvertBitsToBytes((u8 *)&UserFuseRead,
			&UserFuses_Read[XSK_ZYNQMP_EFUSEPS_USR4_FUSE][0],
			XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);
		XilSKey_Efuse_ConvertBitsToBytes(InstancePtr->User4Fuses,
		&(UserFuses_Write[XSK_ZYNQMP_EFUSEPS_USR4_FUSE][0]),
			XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);

	/* Checking whether requested User FUSE bit programming is possible */
		if (XilSKey_ZynqMp_EfusePs_UserFuses_TobeProgrammed(
			&UserFuses_Write[XSK_ZYNQMP_EFUSEPS_USR4_FUSE][0],
			&UserFuses_Read[XSK_ZYNQMP_EFUSEPS_USR4_FUSE][0],
		(UserFuses_ToBePrgrmd + XSK_ZYNQMP_EFUSEPS_USR4_FUSE)) !=
							XST_SUCCESS) {
			return (XSK_EFUSEPS_ERROR_WRITE_USER4_FUSE +
				XSK_EFUSEPS_ERROR_USER_BIT_CANT_REVERT);
		}
	}

	if (InstancePtr->PrgrmUser5Fuse == TRUE) {
		UserFuseRead = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
					XSK_ZYNQMP_EFUSEPS_USER_5_OFFSET);
		XilSKey_Efuse_ConvertBitsToBytes((u8 *)&UserFuseRead,
			&UserFuses_Read[XSK_ZYNQMP_EFUSEPS_USR5_FUSE][0],
			XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);
		XilSKey_Efuse_ConvertBitsToBytes(InstancePtr->User5Fuses,
		&(UserFuses_Write[XSK_ZYNQMP_EFUSEPS_USR5_FUSE][0]),
		XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);

	/* Checking whether requested User FUSE bit programming is possible */
		if (XilSKey_ZynqMp_EfusePs_UserFuses_TobeProgrammed(
			&UserFuses_Write[XSK_ZYNQMP_EFUSEPS_USR5_FUSE][0],
			&UserFuses_Read[XSK_ZYNQMP_EFUSEPS_USR5_FUSE][0],
		(UserFuses_ToBePrgrmd + XSK_ZYNQMP_EFUSEPS_USR5_FUSE)) !=
								XST_SUCCESS) {
			return (XSK_EFUSEPS_ERROR_WRITE_USER5_FUSE +
				XSK_EFUSEPS_ERROR_USER_BIT_CANT_REVERT);
		}
	}

	if (InstancePtr->PrgrmUser6Fuse == TRUE) {
		UserFuseRead = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
					XSK_ZYNQMP_EFUSEPS_USER_6_OFFSET);
		XilSKey_Efuse_ConvertBitsToBytes((u8 *)&UserFuseRead,
			&UserFuses_Read[XSK_ZYNQMP_EFUSEPS_USR6_FUSE][0],
			XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);
		XilSKey_Efuse_ConvertBitsToBytes(InstancePtr->User6Fuses,
		&(UserFuses_Write[XSK_ZYNQMP_EFUSEPS_USR6_FUSE][0]),
				XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);
	/* Checking whether requested User FUSE bit programming is possible */
		if (XilSKey_ZynqMp_EfusePs_UserFuses_TobeProgrammed(
			&UserFuses_Write[XSK_ZYNQMP_EFUSEPS_USR6_FUSE][0],
			&UserFuses_Read[XSK_ZYNQMP_EFUSEPS_USR6_FUSE][0],
		(UserFuses_ToBePrgrmd + XSK_ZYNQMP_EFUSEPS_USR6_FUSE)) !=
								XST_SUCCESS) {
			return (XSK_EFUSEPS_ERROR_WRITE_USER6_FUSE +
				XSK_EFUSEPS_ERROR_USER_BIT_CANT_REVERT);
		}
	}

	if (InstancePtr->PrgrmUser7Fuse == TRUE) {
		UserFuseRead = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
					XSK_ZYNQMP_EFUSEPS_USER_7_OFFSET);
		XilSKey_Efuse_ConvertBitsToBytes((u8 *)&UserFuseRead,
			&UserFuses_Read[XSK_ZYNQMP_EFUSEPS_USR7_FUSE][0],
			XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);
		XilSKey_Efuse_ConvertBitsToBytes(InstancePtr->User7Fuses,
		&(UserFuses_Write[XSK_ZYNQMP_EFUSEPS_USR7_FUSE][0]),
			XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);

	/* Checking whether requested User FUSE bit programming is possible */
		if (XilSKey_ZynqMp_EfusePs_UserFuses_TobeProgrammed(
			&UserFuses_Write[XSK_ZYNQMP_EFUSEPS_USR7_FUSE][0],
			&UserFuses_Read[XSK_ZYNQMP_EFUSEPS_USR7_FUSE][0],
		(UserFuses_ToBePrgrmd + XSK_ZYNQMP_EFUSEPS_USR7_FUSE)) !=
								XST_SUCCESS) {
			return (XSK_EFUSEPS_ERROR_WRITE_USER7_FUSE +
				XSK_EFUSEPS_ERROR_USER_BIT_CANT_REVERT);
		}
	}

	return XST_SUCCESS;
}
