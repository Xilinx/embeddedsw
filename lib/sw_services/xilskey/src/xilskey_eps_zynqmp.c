/******************************************************************************
*
* Copyright (C) 2015 - 2019 Xilinx, Inc.  All rights reserved.
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
*                      feature. Added temperature and voltage checks, while
*                      programming and reading eFUSE array. Added separate
*                      function to set timing parameters and sysmon PSU
*                      driver initialization. Added init function while
*                      from eFUSE. Added appropriate error codes on failure
*                      returns.
*       vns   08/24/16 Fixed eFUSE ZynqMP programming by adding unlocking
*                      before eFUSE PS initialization.
* 6.2   vns   02/18/17 Added margin reads for verifying, added CRC check,
*                      Removed temperature checks for each bit. Added
*                      temperature checks in all read APIs.
* 6.4   vns   02/19/18 Added efuse cache reload call in function
*                      XilSKey_ZynqMp_EfusePs_Write(), so on successful
*                      efuse programming, programmed fuses can directly read
*                      from cache of the efuse.
* 6.6   vns   06/06/18 Added doxygen tags
*       vns   09/18/18 Added APIs to support eFUSE programming from linux
*       vns   10/11/18 Added support to re-program non-zero SPKID
* 6.7	arc   01/05/19 Fixed MISRA-C violations.
*       arc   25/02/19 Added NULL checks and validations for input params
*                      and add timeouts and status info
*       arc   03/15/19 Modified initial default status value as XST_FAILURE
* 6.7   psl   03/21/19 Fixed MISRA-C violation.
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
static u8 Init_Done;

/************************** Function Prototypes *****************************/

static inline u32 XilSKey_ZynqMp_EfusePsWrite_Checks(
				XilSKey_ZynqMpEPs *InstancePtr);
static inline u32 XilSKey_ZynqMp_EfusePs_WriteAndVerify_RowRange(u8 *Data,
		u8 RowStart, u8 RowEnd, XskEfusePs_Type EfuseType);
static inline u32 XilSKey_ZynqMp_EfusePs_PrgrmTbits(void);
static inline u32 XilSKey_ZynqMp_EfusePs_WriteBit(u8 Row, u8 Column,
						XskEfusePs_Type EfuseType);
static inline void XilSKey_ZynqMp_EfusePs_SetTimerValues(void);
static inline u32 XilSKey_ZynqMp_EfusePs_Write_SecCtrl(
				XilSKey_ZynqMpEPs *InstancePtr);
static inline u32 XilSKey_ZynqMp_EfusePs_Write_SecCtrlBits(
				XilSKey_ZynqMpEPs *InstancePtr);
static inline u32 XilSKey_ZynqMp_EfusePs_Write_UsrCtrlBits(
				XilSKey_ZynqMpEPs *InstancePtr);
static inline u32 XilSKey_ZynqMp_EfusePs_ReadSecCtrlBits_Regs(
	XilSKey_SecCtrlBits *ReadBackSecCtrlBits, u8 ReadOption);
static inline u32 XilSKey_ZynqMp_EfusePs_CheckZeros_BfrPrgrmg(
				XilSKey_ZynqMpEPs *InstancePtr);
static inline u32 XilSKey_ZynqMp_EfusePs_UserFuses_WriteChecks(
	XilSKey_ZynqMpEPs *InstancePtr, XilSKey_UsrFuses *ToBePrgrmd);
static inline u32 XilSKey_ZynqMp_EfusePs_UserFuses_TobeProgrammed(
			u8 *UserFuses_Write, u8 *UserFuses_Read,
			XilSKey_UsrFuses *UserFuses_ToBePrgrmd);
u32 XilSKey_ZynqMp_EfusePs_SetWriteConditions(void);
u32 XilSKey_ZynqMp_EfusePs_ReadRow(u8 Row, XskEfusePs_Type EfuseType,
							u32 *RowData);
u32 XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(u8 Row, u8 Column,
						XskEfusePs_Type EfuseType);
u32 XilSKey_ZynqMp_EfusePs_CheckForZeros(u8 RowStart, u8 RowEnd,
						XskEfusePs_Type EfuseType);
static inline u32 XilSKey_ZynqMp_EfusePs_Enable_Rsa(u8 *SecBits_read);
u32 XilSKey_ZynqMp_EfusePs_Init(void);
static u32 XilSKey_ZynqMpEfuseRead(const u32 AddrHigh, const u32 AddrLow);
static u32 XilSKey_ZynqMpEfuseWrite(const u32 AddrHigh, const u32 AddrLow);
u32 XilSKey_ZynqMp_EfusePs_ReadPufChash(u32 *Address, u8 ReadOption);
static u32 XilSkey_ZynqMpUsrFuseRd(u32 Offset, u32 *Buffer, u32 Size, u8 UsrFuseNum);

/************************** Function Definitions *****************************/

/***************************************************************************/
/**
* This function is used to program the PS efuse of ZynqMP, based on user
* inputs
*
* @param	InstancePtr	Pointer to the XilSKey_ZynqMpEPs.
*
* @return
* 		- XST_SUCCESS if programs successfully.
* 		- Errorcode on failure
*
* @note		On successful efuse programming cache is been reloaded,
* 		so programmed efuse bits can directly read from efuse cache.
*
****************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_Write(XilSKey_ZynqMpEPs *InstancePtr)
{
	u32 Status;
	u8 AesKeyInBits[XSK_ZYNQMP_EFUSEPS_AES_KEY_LEN_IN_BITS] = {0};
	u8 Ppk0InBits[XSK_ZYNQMP_EFUSEPS_PPK_SHA3HASH_LEN_IN_BITS] = {0};
	u8 Ppk1InBits[XSK_ZYNQMP_EFUSEPS_PPK_SHA3HASH_LEN_IN_BITS] = {0};
	u8 SpkIdInBits[XSK_ZYNQMP_EFUSEPS_SPKID_LEN_IN_BITS] = {0};
	u8 SpkIdInBitsRd[XSK_ZYNQMP_EFUSEPS_SPKID_LEN_IN_BITS] = {0};
	XilSKey_UsrFuses UsrFuses_ToPrgm[8] = {0};
	u32 AesCrc;
	u32 SpkId;
	u8 Column;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Unlock the controller */
	XilSKey_ZynqMp_EfusePs_CtrlrUnLock();

	/* Check the unlock status */
	if (XilSKey_ZynqMp_EfusePs_CtrlrLockStatus() != 0U) {
		Status = (u32)(XSK_EFUSEPS_ERROR_CONTROLLER_LOCK);
		goto UNLOCK;
	}

	Status = XilSKey_ZynqMp_EfusePs_Init();
	if (Status != (u32)XST_SUCCESS) {
		goto UNLOCK;
	}
	/* Conditions to check programming is possible or not */
	Status = XilSKey_ZynqMp_EfusePsWrite_Checks(InstancePtr);
	if (Status != (u32)XST_SUCCESS) {
		Status = (Status + (u32)XSK_EFUSEPS_ERROR_BEFORE_PROGRAMMING);
		goto UNLOCK;
	}
	if (InstancePtr->PrgrmSpkID == TRUE) {
		XilSKey_Efuse_ConvertBitsToBytes((u8 *)(InstancePtr->SpkId),
			SpkIdInBits, XSK_ZYNQMP_EFUSEPS_SPKID_LEN_IN_BITS);
		SpkId = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
			XSK_ZYNQMP_EFUSEPS_SPK_ID_OFFSET);
		XilSKey_Efuse_ConvertBitsToBytes((u8 *)&SpkId,
			SpkIdInBitsRd, XSK_ZYNQMP_EFUSEPS_SPKID_LEN_IN_BITS);
		/* Check if it is poosible to program or not */
		for (Column = 0U; Column < XSK_ZYNQMP_EFUSEPS_SPKID_LEN_IN_BITS;
							Column++) {
			/* If user requests a non-zero bit */
			if ((SpkIdInBits[Column] == 0U) &&
				(SpkIdInBitsRd[Column] == 1U)) {
				Status = (u32)XSK_EFUSEPS_ERROR_BEFORE_PROGRAMMING |
				 (u32)XSK_EFUSEPS_ERROR_SPKID_BIT_CANT_REVERT;
				goto UNLOCK;
			}
			if ((SpkIdInBits[Column] == 1U) &&
				(SpkIdInBitsRd[Column] == 1U)) {
				SpkIdInBits[Column] = 0U;
			}
		}
	}

	/* Validation of requested User FUSES bits */
	Status = XilSKey_ZynqMp_EfusePs_UserFuses_WriteChecks(
			InstancePtr, &UsrFuses_ToPrgm[0]);
	if (Status != (u32)XST_SUCCESS) {
		Status = (Status + (u32)XSK_EFUSEPS_ERROR_BEFORE_PROGRAMMING);
		goto UNLOCK;
	}

	/* Setting all the conditions for writing into eFuse */
	Status = XilSKey_ZynqMp_EfusePs_SetWriteConditions();
	if (Status != (u32)XST_SUCCESS) {
		Status = (Status + (u32)XSK_EFUSEPS_ERROR_BEFORE_PROGRAMMING);
		goto END;
	}

	/* Check for Zeros for Programming eFuse */
	Status = XilSKey_ZynqMp_EfusePs_CheckZeros_BfrPrgrmg(InstancePtr);
	if (Status != (u32)XST_SUCCESS) {
		Status = (Status + (u32)XSK_EFUSEPS_ERROR_BEFORE_PROGRAMMING);
		goto END;
	}

	if (InstancePtr->PrgrmAesKey == TRUE) {
		XilSKey_Efuse_ConvertBitsToBytes(InstancePtr->AESKey,
			AesKeyInBits, XSK_ZYNQMP_EFUSEPS_AES_KEY_LEN_IN_BITS);
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerify_RowRange(
			AesKeyInBits,XSK_ZYNQMP_EFUSEPS_AES_KEY_START_ROW,
				XSK_ZYNQMP_EFUSEPS_AES_KEY_END_ROW,
				XSK_ZYNQMP_EFUSEPS_EFUSE_0);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status + (u32)XSK_EFUSEPS_ERROR_WRITE_AES_KEY);
			goto END;
		}
		/* Reload cache to verify CRC of programmed AES key */
		Status = XilSKey_ZynqMp_EfusePs_CacheLoad();
		if (Status != (u32)XST_SUCCESS) {
			Status = Status + (u32)XSK_EFUSEPS_ERROR_VERIFICATION +
					(u32)XSK_EFUSEPS_ERROR_WRITE_AES_KEY;
			goto END;
		}
		/* Calculates AES key's CRC */
		AesCrc = XilSkey_CrcCalculation_AesKey(&InstancePtr->AESKey[0]);
		/* Verifies the Aes key programmed with CRC */
		Status = XilSKey_ZynqMp_EfusePs_CheckAesKeyCrc(AesCrc);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XSK_EFUSEPS_ERROR_VERIFICATION +
					(u32)XSK_EFUSEPS_ERROR_WRITE_AES_KEY;
			goto END;
		}
	}

	if (InstancePtr->PrgrmUser0Fuse == TRUE) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerify_RowRange(
			UsrFuses_ToPrgm[XSK_ZYNQMP_EFUSEPS_USR0_FUSE].UserFuse,
			XSK_ZYNQMP_EFUSEPS_USR0_FUSE_ROW,
			XSK_ZYNQMP_EFUSEPS_USR0_FUSE_ROW,
				XSK_ZYNQMP_EFUSEPS_EFUSE_0);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status + (u32)XSK_EFUSEPS_ERROR_WRITE_USER0_FUSE);
			goto END;
		}
	}
	if (InstancePtr->PrgrmUser1Fuse == TRUE) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerify_RowRange(
			UsrFuses_ToPrgm[XSK_ZYNQMP_EFUSEPS_USR1_FUSE].UserFuse,
			XSK_ZYNQMP_EFUSEPS_USR1_FUSE_ROW,
			XSK_ZYNQMP_EFUSEPS_USR1_FUSE_ROW,
				XSK_ZYNQMP_EFUSEPS_EFUSE_0);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status + (u32)XSK_EFUSEPS_ERROR_WRITE_USER1_FUSE);
			goto END;
		}
	}
	if (InstancePtr->PrgrmUser2Fuse == TRUE) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerify_RowRange(
			UsrFuses_ToPrgm[XSK_ZYNQMP_EFUSEPS_USR2_FUSE].UserFuse,
			XSK_ZYNQMP_EFUSEPS_USR2_FUSE_ROW,
			XSK_ZYNQMP_EFUSEPS_USR2_FUSE_ROW,
				XSK_ZYNQMP_EFUSEPS_EFUSE_0);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status + (u32)XSK_EFUSEPS_ERROR_WRITE_USER2_FUSE);
			goto END;
		}
	}
	if (InstancePtr->PrgrmUser3Fuse == TRUE) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerify_RowRange(
			UsrFuses_ToPrgm[XSK_ZYNQMP_EFUSEPS_USR3_FUSE].UserFuse,
			XSK_ZYNQMP_EFUSEPS_USR3_FUSE_ROW,
			XSK_ZYNQMP_EFUSEPS_USR3_FUSE_ROW,
					XSK_ZYNQMP_EFUSEPS_EFUSE_0);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status + (u32)XSK_EFUSEPS_ERROR_WRITE_USER3_FUSE);
			goto END;
		}
	}
	if (InstancePtr->PrgrmUser4Fuse == TRUE) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerify_RowRange(
			UsrFuses_ToPrgm[XSK_ZYNQMP_EFUSEPS_USR4_FUSE].UserFuse,
			XSK_ZYNQMP_EFUSEPS_USR4_FUSE_ROW,
			XSK_ZYNQMP_EFUSEPS_USR4_FUSE_ROW,
					XSK_ZYNQMP_EFUSEPS_EFUSE_0);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status + (u32)XSK_EFUSEPS_ERROR_WRITE_USER4_FUSE);
			goto END;
		}
	}
	if (InstancePtr->PrgrmUser5Fuse == TRUE) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerify_RowRange(
			UsrFuses_ToPrgm[XSK_ZYNQMP_EFUSEPS_USR5_FUSE].UserFuse,
			XSK_ZYNQMP_EFUSEPS_USR5_FUSE_ROW,
			XSK_ZYNQMP_EFUSEPS_USR5_FUSE_ROW,
				XSK_ZYNQMP_EFUSEPS_EFUSE_0);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status + (u32)XSK_EFUSEPS_ERROR_WRITE_USER5_FUSE);
			goto END;
		}
	}
	if (InstancePtr->PrgrmUser6Fuse == TRUE) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerify_RowRange(
			UsrFuses_ToPrgm[XSK_ZYNQMP_EFUSEPS_USR6_FUSE].UserFuse,
			XSK_ZYNQMP_EFUSEPS_USR6_FUSE_ROW,
			XSK_ZYNQMP_EFUSEPS_USR6_FUSE_ROW,
				XSK_ZYNQMP_EFUSEPS_EFUSE_0);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status + (u32)XSK_EFUSEPS_ERROR_WRITE_USER6_FUSE);
			goto END;
		}
	}
	if (InstancePtr->PrgrmUser7Fuse == TRUE) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerify_RowRange(
			UsrFuses_ToPrgm[XSK_ZYNQMP_EFUSEPS_USR7_FUSE].UserFuse,
			XSK_ZYNQMP_EFUSEPS_USR7_FUSE_ROW,
			XSK_ZYNQMP_EFUSEPS_USR7_FUSE_ROW,
					XSK_ZYNQMP_EFUSEPS_EFUSE_0);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status + (u32)XSK_EFUSEPS_ERROR_WRITE_USER7_FUSE);
			goto END;
		}
	}
	if (InstancePtr->PrgrmSpkID == TRUE) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerify_RowRange(
			SpkIdInBits, XSK_ZYNQMP_EFUSEPS_SPK_ID_ROW,
				XSK_ZYNQMP_EFUSEPS_SPK_ID_ROW,
					XSK_ZYNQMP_EFUSEPS_EFUSE_0);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status + (u32)XSK_EFUSEPS_ERROR_WRITE_SPK_ID);
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
			if (Status != (u32)XST_SUCCESS) {
				Status = (Status +
					(u32)XSK_EFUSEPS_ERROR_WRITE_PPK0_HASH);
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
			if (Status != (u32)XST_SUCCESS) {
				Status = (Status +
					(u32)XSK_EFUSEPS_ERROR_WRITE_PPK1_HASH);
				goto END;
			}
		}
	}

	/* Programming Secure and control bits */
	Status = XilSKey_ZynqMp_EfusePs_Write_SecCtrl(InstancePtr);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/* Reload the cache */
	Status = XilSKey_ZynqMp_EfusePs_CacheLoad();
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	/* Check the temperature and voltage(VCC_AUX and VCC_PINT_LP) */
	Status = XilSKey_ZynqMp_EfusePs_Temp_Vol_Checks();
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)(Status | (u32)XSK_EFUSEPS_ERROR_CMPLTD_EFUSE_PRGRM_WITH_ERR);
		goto END;
	}
END:
	XilSKey_ZynqMp_EfusePS_PrgrmDisable();

UNLOCK:
	/* Lock the controller back */
	XilSKey_ZynqMp_EfusePs_CtrlrLock();

	return Status;

}

/*****************************************************************************/
/**
* This function is used to read the PS efuse secure control bits from cache or
* eFUSE based on user input provided.
*
* @param	ReadBackSecCtrlBits	Pointer to the XilSKey_SecCtrlBits
*		which holds the read secure control bits.
* @param	ReadOption 	Variable which has to be provided by user
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
	u32 Status;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(ReadBackSecCtrlBits != NULL);

	if (ReadOption == 0U) {
		Status = XilSKey_ZynqMp_EfusePs_ReadSecCtrlBits_Regs(
					ReadBackSecCtrlBits, ReadOption);
	}
	else {
		/* Unlock the controller */
		XilSKey_ZynqMp_EfusePs_CtrlrUnLock();
		/* Check the unlock status */
		if (XilSKey_ZynqMp_EfusePs_CtrlrLockStatus() != 0U) {
			Status = (u32)(XSK_EFUSEPS_ERROR_CONTROLLER_LOCK);
			goto END;
		}
		Status = XilSKey_ZynqMp_EfusePs_Init();
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		/* Vol and temperature checks */
		Status = XilSKey_ZynqMp_EfusePs_Temp_Vol_Checks();
		if (Status != (u32)XST_SUCCESS) {
			 goto END;
		}
		Status = XilSKey_ZynqMp_EfusePs_ReadSecCtrlBits_Regs(
					ReadBackSecCtrlBits,ReadOption);
		XilSKey_ZynqMp_EfusePs_CtrlrUnLock();
	}
END:
	return Status;

}

/*****************************************************************************/
/**
* This function is used to read the PS efuse secure control bits from cache
* or from eFUSE array based on user selection.
*
* @param	ReadBackSecCtrlBits is the pointer to the XilSKey_SecCtrlBits
*		which holds the read secure control bits.
* @param	ReadOption is a u8 variable which has to be provided by user
*		based on this input reading is happened from cache or from
*		efuse array.
*		- 0	Reads from cache
*		- 1	Reads from efuse array
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline u32 XilSKey_ZynqMp_EfusePs_ReadSecCtrlBits_Regs(
		XilSKey_SecCtrlBits *ReadBackSecCtrlBits, u8 ReadOption)
{

	u32 RegData = 0U;
	u32 Status = (u32)XST_FAILURE;
#ifdef XSK_ZYNQ_ULTRA_MP_PLATFORM
	u32 Silicon_Ver;
#endif

	if (ReadOption == 0U) {
		RegData = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_OFFSET);
		Status = (u32)XST_SUCCESS;
	}
	else {
		Status = XilSKey_ZynqMp_EfusePs_ReadRow(
				XSK_ZYNQMP_EFUSEPS_MISC_USR_CTRL_ROW,
				XSK_ZYNQMP_EFUSEPS_EFUSE_0, &RegData);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}
	ReadBackSecCtrlBits->UserWrLk0 =
		(u8)(RegData & XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_0_MASK);
	ReadBackSecCtrlBits->UserWrLk1 =
		(u8)((RegData & XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_1_MASK) >>
			XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_1_SHIFT);
	ReadBackSecCtrlBits->UserWrLk2 =
		(u8)((RegData & XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_2_MASK) >>
			XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_2_SHIFT);
	ReadBackSecCtrlBits->UserWrLk3 =
		(u8)((RegData & XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_3_MASK) >>
			XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_3_SHIFT);
	ReadBackSecCtrlBits->UserWrLk4 =
		(u8)((RegData & XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_4_MASK) >>
			XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_4_SHIFT);
	ReadBackSecCtrlBits->UserWrLk5 =
		(u8)((RegData & XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_5_MASK) >>
			XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_5_SHIFT);
	ReadBackSecCtrlBits->UserWrLk6 =
		(u8)((RegData & XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_6_MASK) >>
			XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_6_SHIFT);
	ReadBackSecCtrlBits->UserWrLk7 =
		(u8)((RegData & XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_7_MASK) >>
			XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_7_SHIFT);

	ReadBackSecCtrlBits->LBistEn =
		(u8)((RegData & XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_LBIST_EN_MASK) >>
		XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_LBIST_EN_SHIFT);
	ReadBackSecCtrlBits->LpdScEn =
		(u8)((RegData & XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_LPD_SC_EN_MASK) >>
		XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_LPD_SC_EN_SHIFT);
	ReadBackSecCtrlBits->FpdScEn =
		(u8)((RegData & XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_FPD_SC_EN_MASK) >>
		XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_FPD_SC_EN_SHIFT);

	if (ReadOption == 0U) {
		RegData = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_SEC_CTRL_OFFSET);
	}
	else {
		Status = XilSKey_ZynqMp_EfusePs_ReadRow(
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_ROW,
			XSK_ZYNQMP_EFUSEPS_EFUSE_0, &RegData);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}
	ReadBackSecCtrlBits->AesKeyRead =
		(u8)(RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_AES_RDLK_MASK);
	ReadBackSecCtrlBits->AesKeyWrite =
		(u8)((RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_AES_WRLK_MASK) >>
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_AES_WRLK_SHIFT);
	ReadBackSecCtrlBits->EncOnly =
		(u8)((RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_ENC_ONLY_MASK) >>
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_ENC_ONLY_SHIFT);
	ReadBackSecCtrlBits->BbramDisable =
		(u8)((RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_BBRAM_DIS_MASK) >>
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_BBRAM_DIS_SHIFT);
	ReadBackSecCtrlBits->ErrorDisable =
		(u8)((RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_ERR_DIS_MASK) >>
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_ERR_DIS_SHIFT);
	ReadBackSecCtrlBits->JtagDisable =
		(u8)((RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_JTAG_DIS_MASK) >>
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_JTAG_DIS_SHIFT);
	ReadBackSecCtrlBits->DFTDisable =
		(u8)((RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_DFT_DIS_MASK) >>
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_DFT_DIS_SHIFT);
	ReadBackSecCtrlBits->ProgGate =
		(u8)((RegData & (u32)XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PROG_GATE_MASK) >>
			(u32)XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PROG_GATE_0_SHIFT);
	ReadBackSecCtrlBits->SecureLock =
		(u8)((RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_LOCK_MASK) >>
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_LOCK_SHIFT);
	/*
	 * RSA authentication enable is
	 *  11:25 bits from silicon version 3.0 and
	 *  24:25 bits for 1.0 and 2.0 silicon version
	 */
#ifdef XSK_ZYNQ_ULTRA_MP_PLATFORM
	Silicon_Ver = XGetPSVersion_Info();
	if (Silicon_Ver > (u32)XPS_VERSION_2) {
		ReadBackSecCtrlBits->RSAEnable =
		(u16)((RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_RSA_EN_MASK) >>
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_RSA_EN_SHIFT);
	}
	else {
		ReadBackSecCtrlBits->RSAEnable =
			(u16)((RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_RSA_EN_MASK) >>
			(u32)XSK_ZYNQMP_EFUSEPS_SEC_RSA_EN_BIT14);
	}
#endif
	ReadBackSecCtrlBits->PPK0WrLock =
		(u8)((RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PPK0_WRLK_MASK) >>
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PPK0_WRLK_SHIFT);
	ReadBackSecCtrlBits->PPK0InVld =
		(u8)((RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PPK0_INVLD_MASK) >>
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PPK0_INVLD_SHIFT);
	ReadBackSecCtrlBits->PPK1WrLock =
		(u8)((RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PPK1_WRLK_MASK) >>
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PPK1_WRLK_SHIFT);
	ReadBackSecCtrlBits->PPK1InVld =
		(u8)((RegData & XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PPK1_INVLD_MASK) >>
			XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PPK1_INVLD_SHIFT);
	/* Read PBR error */
	if (ReadOption == 0U) {
		RegData = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_PBR_BOOT_ERR_OFFSET);
	}
	else {
		Status = XilSKey_ZynqMp_EfusePs_ReadRow(
				XSK_ZYNQMP_EFUSEPS_PBR_BOOT_ERR_ROW,
			XSK_ZYNQMP_EFUSEPS_EFUSE_0, &RegData);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}
	ReadBackSecCtrlBits->PbrBootErr = (u8)(RegData &
			XSK_ZYNQMP_EFUSEPS_PBR_BOOT_ERR_MASK);

	/* Read Reserved bits */
	if (ReadOption == 0U) {
		RegData = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_RESERVED_OFFSET);
	}
	else {
		Status = XilSKey_ZynqMp_EfusePs_ReadRow(
				XSK_ZYNQMP_EFUSEPS_RESERVED_ROW,
			XSK_ZYNQMP_EFUSEPS_EFUSE_0, &RegData);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}
	ReadBackSecCtrlBits->Reserved1 = (u16)(RegData &
			XSK_ZYNQMP_EFUSEPS_RESERVED1_MASK);
	ReadBackSecCtrlBits->Reserved2 = (u16)(RegData &
			XSK_ZYNQMP_EFUSEPS_RESERVED2_MASK) >>
				XSK_ZYNQMP_EFUSEPS_RESERVED_SHIFT;

END:

	return Status;
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
	u32 Status;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/**
	 * Check the temperature and voltage(VCC_AUX and VCC_PINT_LP)
	 */
	Status = XilSKey_ZynqMp_EfusePs_Temp_Vol_Checks();
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}


	/* Read secure and control bits */
	Status = XilSKey_ZynqMp_EfusePs_ReadSecCtrlBits(
			&(InstancePtr->ReadBackSecCtrlBits), 0);
	if(Status != (u32)XST_SUCCESS) {
		goto END;
	}
		if (InstancePtr->PrgrmAesKey == TRUE) {
			if (InstancePtr->ReadBackSecCtrlBits.AesKeyWrite ==
									TRUE) {
				Status = ((u32)XSK_EFUSEPS_ERROR_FUSE_PROTECTED +
					(u32)XSK_EFUSEPS_ERROR_WRITE_AES_KEY);
				goto END;
			}
		}
		if (InstancePtr->PrgrmSpkID == TRUE) {
			if (XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_PGM_LOCK_OFFSET) != 0x00U) {
				Status = ((u32)XSK_EFUSEPS_ERROR_FUSE_PROTECTED +
					(u32)XSK_EFUSEPS_ERROR_WRITE_SPK_ID);
				goto END;
			}
		}
		if (((InstancePtr->PrgrmUser0Fuse == TRUE) &&
		(InstancePtr->ReadBackSecCtrlBits.UserWrLk0 == TRUE))) {
			Status = ((u32)XSK_EFUSEPS_ERROR_FUSE_PROTECTED |
					(u32)XSK_EFUSEPS_ERROR_WRITE_USER0_FUSE);
			goto END;
		}

		if (((InstancePtr->PrgrmUser1Fuse == TRUE) &&
		(InstancePtr->ReadBackSecCtrlBits.UserWrLk1 == TRUE))) {
			Status = ((u32)XSK_EFUSEPS_ERROR_FUSE_PROTECTED |
					(u32)XSK_EFUSEPS_ERROR_WRITE_USER1_FUSE);
			goto END;
		}
		if (((InstancePtr->PrgrmUser2Fuse == TRUE) &&
		(InstancePtr->ReadBackSecCtrlBits.UserWrLk2 == TRUE))) {
			Status = ((u32)XSK_EFUSEPS_ERROR_FUSE_PROTECTED |
					(u32)XSK_EFUSEPS_ERROR_WRITE_USER2_FUSE);
			goto END;
		}
		if (((InstancePtr->PrgrmUser3Fuse == TRUE) &&
		(InstancePtr->ReadBackSecCtrlBits.UserWrLk3 == TRUE))) {
			Status = ((u32)XSK_EFUSEPS_ERROR_FUSE_PROTECTED |
					(u32)XSK_EFUSEPS_ERROR_WRITE_USER3_FUSE);
			goto END;
		}
		if (((InstancePtr->PrgrmUser4Fuse == TRUE) &&
		(InstancePtr->ReadBackSecCtrlBits.UserWrLk4 == TRUE))) {
			Status = ((u32)XSK_EFUSEPS_ERROR_FUSE_PROTECTED |
					(u32)XSK_EFUSEPS_ERROR_WRITE_USER4_FUSE);
			goto END;
		}
		if (((InstancePtr->PrgrmUser5Fuse == TRUE) &&
		(InstancePtr->ReadBackSecCtrlBits.UserWrLk5 == TRUE))) {
			Status = ((u32)XSK_EFUSEPS_ERROR_FUSE_PROTECTED |
				(u32)XSK_EFUSEPS_ERROR_WRITE_USER5_FUSE);
			goto END;
		}
		if (((InstancePtr->PrgrmUser6Fuse == TRUE) &&
		(InstancePtr->ReadBackSecCtrlBits.UserWrLk6 == TRUE))) {
			Status = ((u32)XSK_EFUSEPS_ERROR_FUSE_PROTECTED |
					(u32)XSK_EFUSEPS_ERROR_WRITE_USER6_FUSE);
			goto END;
		}
		if (((InstancePtr->PrgrmUser7Fuse == TRUE) &&
		(InstancePtr->ReadBackSecCtrlBits.UserWrLk7 == TRUE))) {
			Status = ((u32)XSK_EFUSEPS_ERROR_FUSE_PROTECTED |
					(u32)XSK_EFUSEPS_ERROR_WRITE_USER7_FUSE);
			goto END;
		}
		if (InstancePtr->PrgrmPpk0Hash == TRUE) {
			if (InstancePtr->ReadBackSecCtrlBits.PPK0WrLock ==
								TRUE) {
				Status = ((u32)XSK_EFUSEPS_ERROR_FUSE_PROTECTED +
					(u32)XSK_EFUSEPS_ERROR_WRITE_PPK0_HASH);
				goto END;
			}
			/* Check for Zeros */
			for (RowOffset = XSK_ZYNQMP_EFUSEPS_PPK0_0_OFFSET;
				RowOffset < XSK_ZYNQMP_EFUSEPS_PPK0_11_OFFSET;
				RowOffset = RowOffset + 4U) {
				if (XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
							RowOffset) != 0x00U) {
					Status = (u32)(
					XSK_EFUSEPS_ERROR_PPK0_HASH_ALREADY_PROGRAMMED);
					goto END;
				}
			}
		}
		if (InstancePtr->PrgrmPpk1Hash == TRUE) {
			if (InstancePtr->ReadBackSecCtrlBits.PPK1WrLock ==
								TRUE) {
				Status = ((u32)XSK_EFUSEPS_ERROR_FUSE_PROTECTED +
					(u32)XSK_EFUSEPS_ERROR_WRITE_PPK1_HASH);
				goto END;
			}
			/* Check for Zeros */
			for (RowOffset = XSK_ZYNQMP_EFUSEPS_PPK1_0_OFFSET;
				RowOffset < XSK_ZYNQMP_EFUSEPS_PPK1_11_OFFSET;
				RowOffset = RowOffset + 4U) {
				if (XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
							RowOffset) != 0x00U) {
					Status = (u32)(
					XSK_EFUSEPS_ERROR_PPK1_HASH_ALREADY_PROGRAMMED);
					goto END;
				}
			}
		}
END:
	return Status;

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
	u32 Status = (u32)XST_FAILURE;
	u32 Bit;
	u8 Bit_u8;

	if (RowStart > RowEnd) {
		goto END;
	}

	for (Row = RowStart; Row <= RowEnd; Row++) {
		for (Column = 0U; Column < 32U; Column++) {
			Bit_u8 = (Row - RowStart);
			Bit = (u32)Bit_u8 * (u32)XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW;
			Bit = Bit + (u32)Column;
			if (Data[Bit] != 0U) {
				Status =
				XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
							Column, EfuseType);
				if (Status != (u32)XST_SUCCESS) {
					goto END;
				}
			}
		}
	}
	Status = (u32)XST_SUCCESS;
END:
	return Status;
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
static inline u32 XilSKey_ZynqMp_EfusePs_PrgrmTbits(void)
{
	u32 RowDataVal = 0U;
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
					XSK_ZYNQMP_EFUSEPS_EFUSE_0, &RowDataVal);
	if (Status != (u32)XST_SUCCESS) {
		 goto END;
	}
	if (((RowDataVal >> XSK_ZYNQMP_EFUSEPS_TBITS_SHIFT) &
			XSK_ZYNQMP_EFUSEPS_TBITS_MASK) != 0x00U) {
		Status = (u32)XSK_EFUSEPS_ERROR_PROGRAMMING_TBIT_PATTERN;
		goto END;
	}

	Status = XilSKey_ZynqMp_EfusePs_ReadRow(XSK_ZYNQMP_EFUSEPS_TBITS_ROW,
			XSK_ZYNQMP_EFUSEPS_EFUSE_2, &RowDataVal);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	if (((RowDataVal >> XSK_ZYNQMP_EFUSEPS_TBITS_SHIFT) &
				XSK_ZYNQMP_EFUSEPS_TBITS_MASK) != 0x00U) {
		Status = (u32)XSK_EFUSEPS_ERROR_PROGRAMMING_TBIT_PATTERN;
		goto END;
	}

	Status = XilSKey_ZynqMp_EfusePs_ReadRow(XSK_ZYNQMP_EFUSEPS_TBITS_ROW,
					XSK_ZYNQMP_EFUSEPS_EFUSE_3, &RowDataVal);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	if (((RowDataVal >> XSK_ZYNQMP_EFUSEPS_TBITS_SHIFT) &
				XSK_ZYNQMP_EFUSEPS_TBITS_MASK) != 0x00U) {
		Status = (u32)XSK_EFUSEPS_ERROR_PROGRAMMING_TBIT_PATTERN;
		goto END;
	}

	/* Programming Tbits */
	for (Column = 28U; Column <= 31U; Column++) {
		if ((Column == 28U) || (Column == 30U)) {
			continue;
		}
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(
				XSK_ZYNQMP_EFUSEPS_TBITS_ROW, Column,
				XSK_ZYNQMP_EFUSEPS_EFUSE_0);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(
				XSK_ZYNQMP_EFUSEPS_TBITS_ROW, Column,
					XSK_ZYNQMP_EFUSEPS_EFUSE_2);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(
				XSK_ZYNQMP_EFUSEPS_TBITS_ROW, Column,
					XSK_ZYNQMP_EFUSEPS_EFUSE_3);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}

	XilSKey_WriteReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
		XSK_ZYNQMP_EFUSEPS_TBITS_PRGRMG_EN_OFFSET, TbitsPrgrmReg);
END:
	return Status;

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
	u32 RowDataVal = 0U;
	u32 Status;
	u8 MarginRead;
	u32 ReadReg;

	/* Programming bit */
	Status = XilSKey_ZynqMp_EfusePs_WriteBit(Row, Column, EfuseType);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
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
		Status = (u32)XST_SUCCESS;
		goto END;
	}

	/* verifying the programmed bit */
	for (MarginRead = XSK_ZYNQMP_EFUSEPS_CFG_NORMAL_RD;
			MarginRead <= XSK_ZYNQMP_EFUSEPS_CFG_MARGIN_2_RD;
				MarginRead++) {

		ReadReg = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_CFG_OFFSET) &
				(~XSK_ZYNQMP_EFUSEPS_CFG_MARGIN_RD_MASK);
		ReadReg = ReadReg |
				(u32)((u32)MarginRead << (u32)XSK_ZYNQMP_EFUSEPS_CFG_MARGIN_RD_SHIFT);

		XilSKey_WriteReg(XSK_ZYNQMP_EFUSEPS_BASEADDR, \
				XSK_ZYNQMP_EFUSEPS_CFG_OFFSET, ReadReg);

		Status = XilSKey_ZynqMp_EfusePs_ReadRow(Row, EfuseType, &RowDataVal);

		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}

		if (((RowDataVal >> Column) & 0x01U) == 0x00U) {
			Status = (u32)XSK_EFUSEPS_ERROR_VERIFICATION;
			goto END;
		}
	}
END:
	return Status;

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
	u32 ReadValue = 0U;
	u32 EfusePsType = (u32)EfuseType;
	u32 Status = (u32)XST_FAILURE;
	u32 TimeOut = 0U;

	WriteValue = ((EfusePsType << (u32)XSK_ZYNQMP_EFUSEPS_RD_ADDR_SHIFT) &
					(u32)XSK_ZYNQMP_EFUSEPS_RD_ADDR_MASK) |
			(((u32)Row << (u32)XSK_ZYNQMP_EFUSEPS_RD_ADDR_ROW_SHIFT) &
					(u32)XSK_ZYNQMP_EFUSEPS_RD_ADDR_ROW_MASK);
	XilSKey_WriteReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
		XSK_ZYNQMP_EFUSEPS_RD_ADDR_OFFSET, WriteValue);

	while (TimeOut < XSK_POLL_TIMEOUT) {
		/* Check for read completion */
		ReadValue = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
					XSK_ZYNQMP_EFUSEPS_ISR_OFFSET) &
				(XSK_ZYNQMP_EFUSEPS_ISR_RD_ERR_MASK |
				XSK_ZYNQMP_EFUSEPS_ISR_RD_DONE_MASK);
		if (ReadValue != 0x00U) {
			break;
		}
		TimeOut = TimeOut + 1U;
	}

	if ((ReadValue & XSK_ZYNQMP_EFUSEPS_ISR_RD_DONE_MASK) == 0U) {
		Status = (u32)XSK_EFUSEPS_ERROR_READ_NOT_DONE;
		goto END;
	}

	if ((ReadValue & XSK_ZYNQMP_EFUSEPS_ISR_RD_ERR_MASK) != 0U) {
		Status = (u32)XSK_EFUSEPS_ERROR_READ;
		goto END;
	}

	*RowData =
		XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_RD_DATA_OFFSET);
	Status = (u32)XST_SUCCESS;
END:
	 return Status;
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
	u32 EfusePsType = (u32)EfuseType;

	WriteValue = ((EfusePsType << (u32)XSK_ZYNQMP_EFUSEPS_PGM_ADDR_SHIFT) &
					(u32)XSK_ZYNQMP_EFUSEPS_PGM_ADDR_MASK) |
			(((u32)Row << (u32)XSK_ZYNQMP_EFUSEPS_PGM_ADDR_ROW_SHIFT) &
					(u32)XSK_ZYNQMP_EFUSEPS_PGM_ADDR_ROW_MASK) |
			((u32)Column & (u32)XSK_ZYNQMP_EFUSEPS_PGM_ADDR_COL_MASK);

	XilSKey_WriteReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
		XSK_ZYNQMP_EFUSEPS_PGM_ADDR_OFFSET, WriteValue);

	ReadValue = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
					XSK_ZYNQMP_EFUSEPS_ISR_OFFSET) &
				(XSK_ZYNQMP_EFUSEPS_ISR_PGM_ERR_MASK |
					XSK_ZYNQMP_EFUSEPS_ISR_PGM_DONE_MASK);
	while (ReadValue == 0x00U) {
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

	return (u32)XST_SUCCESS;

}

/*****************************************************************************/
/*
* This function reloads the cache of efuse so that can be directly read from
* cache.
*
* @param	None.
*
* @return
*		- XST_SUCCESS on successful cache reload
*		- ErrorCode on failure
*
* @note		Not recommended to call this API
*		frequently, if this API is called all the cache memory is reloded
*		by reading eFUSE array, reading eFUSE bit multiple times may
*		diminish the life time.
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_CacheLoad(void)
{
	volatile u32 CacheStatus;
	u32 Status = (u32)XST_FAILURE;

	/* Check the unlock status */
	if (XilSKey_ZynqMp_EfusePs_CtrlrLockStatus() != 0U) {
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
		if ((CacheStatus) == (u32)XSK_ZYNQMP_EFUSEPS_STS_CACHE_DONE_MASK) {
			break;
		}
	}

	CacheStatus = XilSKey_ZynqMp_EfusePs_Status();
	if ((CacheStatus & XSK_ZYNQMP_EFUSEPS_STS_CACHE_DONE_MASK) == 0x00U) {
		Status = (u32)XSK_EFUSEPS_ERROR_CACHE_LOAD;
		goto END;
	}

	CacheStatus = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
					XSK_ZYNQMP_EFUSEPS_ISR_OFFSET);
	if ((CacheStatus & XSK_ZYNQMP_EFUSEPS_ISR_CACHE_ERR_MASK) ==
			XSK_ZYNQMP_EFUSEPS_ISR_CACHE_ERR_MASK) {
		Status = (u32)XSK_EFUSEPS_ERROR_CACHE_LOAD;
		goto END;
	}
	Status = (u32)XST_SUCCESS;
END:
	return Status;

}

/*****************************************************************************/
/*
* This function sets all the required parameters to program efuse array.
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
u32 XilSKey_ZynqMp_EfusePs_SetWriteConditions(void)
{
	u32 ReadReg;
	u32 Status;

	/* Enable Program enable bit */
	XilSKey_ZynqMp_EfusePS_PrgrmEn();

	/* Setting the timing Constraints and initializing the sysmon */
	Status = XilSKey_ZynqMp_EfusePs_Init();
	if (Status != (u32)XST_SUCCESS) {
		goto END;
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
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		Status = XilSKey_ZynqMp_EfusePs_CacheLoad();
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}
END:
	return Status;

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
static inline void XilSKey_ZynqMp_EfusePs_SetTimerValues(void)
{
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
#ifdef XSK_ZYNQ_ULTRA_MP_PLATFORM
	float RefClk;
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
#endif

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

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Programming Secure and control bits of eFuse */
	Status = XilSKey_ZynqMp_EfusePs_Write_SecCtrlBits(InstancePtr);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/* Programming User control bits */
	Status = XilSKey_ZynqMp_EfusePs_Write_UsrCtrlBits(InstancePtr);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
END:
	return Status;

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
	u32 Status = (u32)XST_FAILURE;
	XskEfusePs_Type EfuseType = XSK_ZYNQMP_EFUSEPS_EFUSE_0;
	u32 Row;
	u32 RowDataVal;
	u8 DataInBits[XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW] = {0};

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Row = XSK_ZYNQMP_EFUSEPS_SEC_CTRL_ROW;

	if ((InstancePtr->PrgrmgSecCtrlBits.AesKeyRead != 0x00U) ||
		(InstancePtr->PrgrmgSecCtrlBits.AesKeyWrite != 0x00U) ||
		(InstancePtr->PrgrmgSecCtrlBits.EncOnly != 0x00U) ||
		(InstancePtr->PrgrmgSecCtrlBits.BbramDisable != 0x00U) ||
		(InstancePtr->PrgrmgSecCtrlBits.ErrorDisable != 0x00U) ||
		(InstancePtr->PrgrmgSecCtrlBits.JtagDisable != 0x00U) ||
		(InstancePtr->PrgrmgSecCtrlBits.DFTDisable != 0x00U) ||
		(InstancePtr->PrgrmgSecCtrlBits.ProgGate != 0x00U) ||
		(InstancePtr->PrgrmgSecCtrlBits.SecureLock != 0x00U) ||
		(InstancePtr->PrgrmgSecCtrlBits.RSAEnable != 0x00U) ||
		(InstancePtr->PrgrmgSecCtrlBits.PPK0WrLock != 0x00U) ||
		(InstancePtr->PrgrmgSecCtrlBits.PPK0InVld != 0x00U) ||
		(InstancePtr->PrgrmgSecCtrlBits.PPK1WrLock != 0x00U) ||
		(InstancePtr->PrgrmgSecCtrlBits.PPK1InVld != 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_ReadRow((u8)Row, EfuseType,
							&RowDataVal);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		XilSKey_Efuse_ConvertBitsToBytes((u8 *)&RowDataVal, DataInBits,
			XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);
	}

	if ((InstancePtr->PrgrmgSecCtrlBits.AesKeyRead != 0x00U) &&
		(DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_AES_RDLK] == 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit((u8)Row,
				XSK_ZYNQMP_EFUSEPS_SEC_AES_RDLK, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status +
				(u32)XSK_EFUSEPS_ERROR_WRTIE_AES_CRC_LK_BIT);
			goto END;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.AesKeyWrite != 0x00U) &&
		(DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_AES_WRLK] == 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit((u8)Row,
			(u8)XSK_ZYNQMP_EFUSEPS_SEC_AES_WRLK, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status +
				(u32)XSK_EFUSEPS_ERROR_WRTIE_AES_WR_LK_BIT);
			goto END;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.EncOnly != 0x00U) &&
		(DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_ENC_ONLY] == 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit((u8)Row,
			(u8)XSK_ZYNQMP_EFUSEPS_SEC_ENC_ONLY, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status +
				(u32)XSK_EFUSEPS_ERROR_WRTIE_USE_AESONLY_EN_BIT);
			goto END;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.BbramDisable != 0x00U) &&
		(DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_BRAM_DIS] == 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit((u8)Row,
			(u8)XSK_ZYNQMP_EFUSEPS_SEC_BRAM_DIS, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status +
				(u32)XSK_EFUSEPS_ERROR_WRTIE_BBRAM_DIS_BIT);
			goto END;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.ErrorDisable != 0x00U) &&
		(DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_ERR_DIS] == 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit((u8)Row,
			(u8)XSK_ZYNQMP_EFUSEPS_SEC_ERR_DIS, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status +
				(u32)XSK_EFUSEPS_ERROR_WRTIE_PMU_ERR_DIS_BIT);
			goto END;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.JtagDisable != 0x00U) &&
		(DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_JTAG_DIS] == 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit((u8)Row,
			(u8)XSK_ZYNQMP_EFUSEPS_SEC_JTAG_DIS, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status +
				(u32)XSK_EFUSEPS_ERROR_WRTIE_JTAG_DIS_BIT);
			goto END;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.DFTDisable != 0x00U) &&
		(DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_DFT_DIS] == 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit((u8)Row,
			(u8)XSK_ZYNQMP_EFUSEPS_SEC_DFT_DIS, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status +
				(u32)XSK_EFUSEPS_ERROR_WRTIE_DFT_MODE_DIS_BIT);
			goto END;
		}
	}
	if (InstancePtr->PrgrmgSecCtrlBits.ProgGate != 0x00U) {
		if (DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_DIS_PROG_GATE0] == 0x00U) {
			Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit((u8)Row,
					(u8)XSK_ZYNQMP_EFUSEPS_SEC_DIS_PROG_GATE0, EfuseType);
			if (Status != (u32)XST_SUCCESS) {
				Status = (u32)(Status +
				(u32)XSK_EFUSEPS_ERROR_WRTIE_PROG_GATE0_DIS_BIT);
				goto END;
			}
		}
		if (DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_DIS_PROG_GATE1] == 0x00U) {
			Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit((u8)Row,
				(u8)XSK_ZYNQMP_EFUSEPS_SEC_DIS_PROG_GATE1, EfuseType);
			if (Status != (u32)XST_SUCCESS) {
				Status = (u32)(Status +
					(u32)XSK_EFUSEPS_ERROR_WRTIE_PROG_GATE1_DIS_BIT);
				goto END;
			}
		}
		if (DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_DIS_PROG_GATE2] == 0x00U) {
			Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit((u8)Row,
				(u8)XSK_ZYNQMP_EFUSEPS_SEC_DIS_PROG_GATE2, EfuseType);
			if (Status != (u32)XST_SUCCESS) {
				Status = (u32)(Status +
					(u32)XSK_EFUSEPS_ERROR_WRTIE_PROG_GATE2_DIS_BIT);
				goto END;
			}
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.SecureLock != 0x00U) &&
		(DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_LOCK] == 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit((u8)Row,
			(u8)XSK_ZYNQMP_EFUSEPS_SEC_LOCK, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status +
				(u32)XSK_EFUSEPS_ERROR_WRTIE_SEC_LOCK_BIT);
			goto END;
		}
	}
	if (InstancePtr->PrgrmgSecCtrlBits.RSAEnable != 0x00U) {
		Status = XilSKey_ZynqMp_EfusePs_Enable_Rsa(DataInBits);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status + (u32)XSK_EFUSEPS_ERROR_WRITE_RSA_AUTH_BIT);
			goto END;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.PPK0WrLock != 0x00U) &&
		(DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_PPK0_WRLK] == 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit((u8)Row,
			(u8)XSK_ZYNQMP_EFUSEPS_SEC_PPK0_WRLK, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status +
				(u32)XSK_EFUSEPS_ERROR_WRTIE_PPK0_WR_LK_BIT);
			goto END;
		}
	}
	if (InstancePtr->PrgrmgSecCtrlBits.PPK0InVld != 0x00U) {
		if (DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_PPK0_INVLD_BIT1]
							== 0x00U) {
			Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit((u8)Row,
					(u8)XSK_ZYNQMP_EFUSEPS_SEC_PPK0_INVLD_BIT1,
							EfuseType);
			if (Status != (u32)XST_SUCCESS) {
				Status = (u32)(Status +
					(u32)XSK_EFUSEPS_ERROR_WRTIE_PPK0_RVK_BIT);
				goto END;
			}
		}
		if (DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_PPK0_INVLD_BIT2]
								== 0x00U) {
			Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit((u8)Row,
					(u8)XSK_ZYNQMP_EFUSEPS_SEC_PPK0_INVLD_BIT2,
								EfuseType);
			if (Status != (u32)XST_SUCCESS) {
				Status = (u32)(Status +
					(u32)XSK_EFUSEPS_ERROR_WRTIE_PPK0_RVK_BIT);
				goto END;
			}
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.PPK1WrLock != 0x00U) &&
		(DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_PPK1_WRLK] == 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit((u8)Row,
			(u8)XSK_ZYNQMP_EFUSEPS_SEC_PPK1_WRLK, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)(Status +
				(u32)XSK_EFUSEPS_ERROR_WRTIE_PPK1_WR_LK_BIT);
			goto END;
		}
	}
	if (InstancePtr->PrgrmgSecCtrlBits.PPK1InVld != 0x00U) {
		if (DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_PPK1_INVLD_BIT1]
								== 0x00U) {
			Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit((u8)Row,
					(u8)XSK_ZYNQMP_EFUSEPS_SEC_PPK1_INVLD_BIT1,
								EfuseType);
			if (Status != (u32)XST_SUCCESS) {
				Status = (u32)(Status +
					(u32)XSK_EFUSEPS_ERROR_WRTIE_PPK1_RVK_BIT);
				goto END;
			}
		}
		if (DataInBits[XSK_ZYNQMP_EFUSEPS_SEC_PPK1_INVLD_BIT2]
								== 0x00U) {
			Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit((u8)Row,
					(u8)XSK_ZYNQMP_EFUSEPS_SEC_PPK1_INVLD_BIT2,
								EfuseType);
			if (Status != (u32)XST_SUCCESS) {
				Status = (u32)(Status +
					(u32)XSK_EFUSEPS_ERROR_WRTIE_PPK1_RVK_BIT);
				goto END;
			}
		}
	}
	/* Programming PBR BOOT Error */
	if (InstancePtr->PrgrmgSecCtrlBits.PbrBootErr == TRUE) {
		Row = XSK_ZYNQMP_EFUSEPS_PBR_BOOT_ERR_ROW;
		Status = XilSKey_ZynqMp_EfusePs_ReadRow(
			(u8)Row, XSK_ZYNQMP_EFUSEPS_EFUSE_0, &RowDataVal);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		XilSKey_Efuse_ConvertBitsToBytes((u8 *)&RowDataVal, DataInBits,
					XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);

		if (DataInBits[XSK_ZYNQMP_EFUSEPS_PBR_BOOT_ERR_0] == 0x00U) {
			Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit((u8)Row,
					(u8)XSK_ZYNQMP_EFUSEPS_PBR_BOOT_ERR_0,
								EfuseType);
			if (Status != (u32)XST_SUCCESS) {
				Status = (u32)(Status +
				(u32)XSK_EFUSEPS_ERROR_WRITE_PBR_BOOT_ERR_BIT);
				goto END;
			}
		}
		if (DataInBits[XSK_ZYNQMP_EFUSEPS_PBR_BOOT_ERR_1] == 0x00U) {
			Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit((u8)Row,
					(u8)XSK_ZYNQMP_EFUSEPS_PBR_BOOT_ERR_1,
								EfuseType);
			if (Status != (u32)XST_SUCCESS) {
				Status = (u32)(Status +
				(u32)XSK_EFUSEPS_ERROR_WRITE_PBR_BOOT_ERR_BIT);
				goto END;
			}
		}
		if (DataInBits[XSK_ZYNQMP_EFUSEPS_PBR_BOOT_ERR_2] == 0x00U) {
			Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit((u8)Row,
					(u8)XSK_ZYNQMP_EFUSEPS_PBR_BOOT_ERR_2,
								EfuseType);
			if (Status != (u32)XST_SUCCESS) {
				Status = (u32)(Status +
				(u32)XSK_EFUSEPS_ERROR_WRITE_PBR_BOOT_ERR_BIT);
				goto END;
			}
		}
	}
	Status = (u32)XST_SUCCESS;
END:
	return Status;

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
	u8 Row = XSK_ZYNQMP_EFUSEPS_MISC_USR_CTRL_ROW;
	u8 DataInBits[XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW] = {0};
	u32 RowDataVal;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	if ((InstancePtr->PrgrmgSecCtrlBits.UserWrLk0 != 0x00U) ||
		(InstancePtr->PrgrmgSecCtrlBits.UserWrLk1 != 0x00U) ||
		(InstancePtr->PrgrmgSecCtrlBits.UserWrLk2 != 0x00U) ||
		(InstancePtr->PrgrmgSecCtrlBits.UserWrLk3 != 0x00U) ||
		(InstancePtr->PrgrmgSecCtrlBits.UserWrLk4 != 0x00U) ||
		(InstancePtr->PrgrmgSecCtrlBits.UserWrLk5 != 0x00U) ||
		(InstancePtr->PrgrmgSecCtrlBits.UserWrLk6 != 0x00U) ||
		(InstancePtr->PrgrmgSecCtrlBits.UserWrLk7 != 0x00U) ||
		(InstancePtr->PrgrmgSecCtrlBits.LBistEn != 0x00U) ||
		(InstancePtr->PrgrmgSecCtrlBits.FpdScEn != 0x00U) ||
		(InstancePtr->PrgrmgSecCtrlBits.LpdScEn != 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_ReadRow(Row, EfuseType,
							&RowDataVal);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		XilSKey_Efuse_ConvertBitsToBytes((u8 *)&RowDataVal, DataInBits,
			XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);
	}
	else {
		Status = (u32)XST_SUCCESS;
		goto END;
	}

	if ((InstancePtr->PrgrmgSecCtrlBits.UserWrLk0 != 0x00U) &&
		(DataInBits[XSK_ZYNQMP_EFUSEPS_USR_WRLK_0] == 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
				XSK_ZYNQMP_EFUSEPS_USR_WRLK_0, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status + (u32)XSK_EFUSEPS_ERROR_WRTIE_USER0_LK_BIT);
			goto END;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.UserWrLk1 != 0x00U) &&
		(DataInBits[XSK_ZYNQMP_EFUSEPS_USR_WRLK_1] == 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_USR_WRLK_1, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status + (u32)XSK_EFUSEPS_ERROR_WRTIE_USER1_LK_BIT);
			goto END;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.UserWrLk2 != 0x00U) &&
		(DataInBits[XSK_ZYNQMP_EFUSEPS_USR_WRLK_2] == 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_USR_WRLK_2, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status + (u32)XSK_EFUSEPS_ERROR_WRTIE_USER2_LK_BIT);
			goto END;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.UserWrLk3 != 0x00U) &&
		(DataInBits[XSK_ZYNQMP_EFUSEPS_USR_WRLK_3] == 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_USR_WRLK_3, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status + (u32)XSK_EFUSEPS_ERROR_WRTIE_USER3_LK_BIT);
			goto END;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.UserWrLk4 != 0x00U) &&
		(DataInBits[XSK_ZYNQMP_EFUSEPS_USR_WRLK_4] == 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_USR_WRLK_4, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status + (u32)XSK_EFUSEPS_ERROR_WRTIE_USER4_LK_BIT);
			goto END;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.UserWrLk5 != 0x00U) &&
		(DataInBits[XSK_ZYNQMP_EFUSEPS_USR_WRLK_5] == 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_USR_WRLK_5, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status + (u32)XSK_EFUSEPS_ERROR_WRTIE_USER5_LK_BIT);
			goto END;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.UserWrLk6 != 0x00U) &&
		(DataInBits[XSK_ZYNQMP_EFUSEPS_USR_WRLK_6] == 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_USR_WRLK_6, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status + (u32)XSK_EFUSEPS_ERROR_WRTIE_USER6_LK_BIT);
			goto END;
		}
	}
	if ((InstancePtr->PrgrmgSecCtrlBits.UserWrLk7 != 0x00U) &&
		(DataInBits[XSK_ZYNQMP_EFUSEPS_USR_WRLK_7] == 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_USR_WRLK_7, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status + (u32)XSK_EFUSEPS_ERROR_WRTIE_USER7_LK_BIT);
			goto END;
		}
	}

	if ((InstancePtr->PrgrmgSecCtrlBits.LBistEn != 0x00U) &&
		(DataInBits[XSK_ZYNQMP_EFUSEPS_LBIST_EN] == 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
			XSK_ZYNQMP_EFUSEPS_LBIST_EN, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			Status = (Status + (u32)XSK_EFUSEPS_ERROR_WRITE_LBIST_EN_BIT);
			goto END;
		}
	}
	if (InstancePtr->PrgrmgSecCtrlBits.LpdScEn != 0x00U) {
		if (DataInBits[XSK_ZYNQMP_EFUSEPS_LPD_SC_EN_0] == 0x00U) {
			Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
				XSK_ZYNQMP_EFUSEPS_LPD_SC_EN_0, EfuseType);
			if (Status != (u32)XST_SUCCESS) {
				Status = (Status +
					(u32)XSK_EFUSEPS_ERROR_WRITE_LPD_SC_EN_BIT);
				goto END;
			}
		}
		if (DataInBits[XSK_ZYNQMP_EFUSEPS_LPD_SC_EN_1] == 0x00U) {
			Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
				XSK_ZYNQMP_EFUSEPS_LPD_SC_EN_1, EfuseType);
			if (Status != (u32)XST_SUCCESS) {
				Status = (Status +
					(u32)XSK_EFUSEPS_ERROR_WRITE_LPD_SC_EN_BIT);
				goto END;
			}
		}
		if (DataInBits[XSK_ZYNQMP_EFUSEPS_LPD_SC_EN_2] == 0x00U) {
			Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
				XSK_ZYNQMP_EFUSEPS_LPD_SC_EN_2, EfuseType);
			if (Status != (u32)XST_SUCCESS) {
				Status = (Status +
					(u32)XSK_EFUSEPS_ERROR_WRITE_LPD_SC_EN_BIT);
				goto END;
			}
		}
	}
	if (InstancePtr->PrgrmgSecCtrlBits.FpdScEn != 0x00U) {
		if (DataInBits[XSK_ZYNQMP_EFUSEPS_FPD_SC_EN_0] == 0x00U) {
			Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
				XSK_ZYNQMP_EFUSEPS_FPD_SC_EN_0, EfuseType);
			if (Status != (u32)XST_SUCCESS) {
				Status = (Status +
					(u32)XSK_EFUSEPS_ERROR_WRITE_FPD_SC_EN_BIT);
				goto END;
			}
		}
		if (DataInBits[XSK_ZYNQMP_EFUSEPS_FPD_SC_EN_1] == 0x00U) {
			Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
				XSK_ZYNQMP_EFUSEPS_FPD_SC_EN_1, EfuseType);
			if (Status != (u32)XST_SUCCESS) {
				Status = (Status +
					(u32)XSK_EFUSEPS_ERROR_WRITE_FPD_SC_EN_BIT);
				goto END;
			}
		}
		if (DataInBits[XSK_ZYNQMP_EFUSEPS_FPD_SC_EN_2] == 0x00U) {
			Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
				XSK_ZYNQMP_EFUSEPS_FPD_SC_EN_2, EfuseType);
			if (Status != (u32)XST_SUCCESS) {
				Status = (Status +
				(u32)XSK_EFUSEPS_ERROR_WRITE_FPD_SC_EN_BIT);
				goto END;
			}
		}
	}
END:
	return Status;

}

/*****************************************************************************/
/**
* This function performs CRC check of AES key
*
* @param	CrcValue	A 32 bit CRC value of an expected AES key.
*
* @return
*		- XST_SUCCESS on successful CRC check.
*		- ErrorCode on failure
*
* @note		For Calculating CRC of AES key  use XilSKey_CrcCalculation()
* 		API or XilSkey_CrcCalculation_AesKey() API.
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_CheckAesKeyCrc(u32 CrcValue)
{

	u32 Status;
	u32 ReadReg = 0U;
	u32 TimeOut = 0U;

	/* Check the unlock status */
	if (XilSKey_ZynqMp_EfusePs_CtrlrLockStatus() != 0U) {
		/* Unlock the controller */
		XilSKey_ZynqMp_EfusePs_CtrlrUnLock();
	}
	Status = XilSKey_ZynqMp_EfusePs_Init();
	if (Status != (u32)XST_SUCCESS) {
		 goto END;
	}
	/* Vol and temperature checks */
	Status = XilSKey_ZynqMp_EfusePs_Temp_Vol_Checks();
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/* writing CRC value to check AES key's CRC */
	XilSKey_WriteReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
		XSK_ZYNQMP_EFUSEPS_AES_CRC_OFFSET,
		(CrcValue & XSK_ZYNQMP_EFUSEPS_AES_CRC_VAL_MASK));

	while (TimeOut < XSK_POLL_TIMEOUT) {
		/* Poll for CRC Done bit */
		ReadReg = XilSKey_ZynqMp_EfusePs_Status();
		if ((ReadReg & XSK_ZYNQMP_EFUSEPS_STS_AES_CRC_DONE_MASK) !=
								0x00U) {
			break;
		}
		TimeOut = TimeOut + 1U;
	}

	if ((ReadReg & XSK_ZYNQMP_EFUSEPS_STS_AES_CRC_DONE_MASK) ==
								0x00U) {
		Status = (u32)XST_FAILURE;
		goto END;
	}

	if ((ReadReg & XSK_ZYNQMP_EFUSEPS_STS_AES_CRC_PASS_MASK) == 0x00U) {
		Status = (u32)XST_FAILURE;
	}

END:
	return Status;

}

/*****************************************************************************/
/**
* This function is used to read user fuse from efuse or cache based on user's
* read option.
*
* @param	UseFusePtr	Pointer to an array which holds the readback
*		user fuse in.
* @param	UserFuse_Num	A variable which holds the user fuse number.
*		Range is (User fuses: 0 to 7)
* @param	ReadOption 	A variable which has to be provided by user
*		based on this input reading is happend from cache or from efuse
*		array.
*		- 0	Reads from cache
*		- 1	Reads from efuse array
*
* @return
*		- XST_SUCCESS on successful read
*		- ErrorCode on failure
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_ReadUserFuse(u32 *UseFusePtr, u8 UserFuse_Num,
							u8 ReadOption)
{
	u32 Status = (u32)XST_FAILURE;
	XskEfusePs_Type EfuseType = XSK_ZYNQMP_EFUSEPS_EFUSE_0;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(UseFusePtr != NULL);
	Xil_AssertNonvoid(UserFuse_Num <= (XSK_ZYNQMP_EFUSEPS_USR_FUSE_REG_NUM
						- 1U));

	if (ReadOption ==  0U) {
		*UseFusePtr = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				(XSK_ZYNQMP_EFUSEPS_USER_0_OFFSET
				+ ((u32)UserFuse_Num * 4U)));
		Status = (u32)XST_SUCCESS;
	}
	else {
		/* Unlock the controller */
		XilSKey_ZynqMp_EfusePs_CtrlrUnLock();
		/* Check the unlock status */
		if (XilSKey_ZynqMp_EfusePs_CtrlrLockStatus() != 0U) {
			Status = (u32)(XSK_EFUSEPS_ERROR_CONTROLLER_LOCK);
			goto UNLOCK;
		}
		Status = XilSKey_ZynqMp_EfusePs_Init();
		if (Status != (u32)XST_SUCCESS) {
			goto UNLOCK;
		}
		/* Vol and temperature checks */
		Status = XilSKey_ZynqMp_EfusePs_Temp_Vol_Checks();
		if (Status != (u32)XST_SUCCESS) {
			goto UNLOCK;
		}

		Status = XilSKey_ZynqMp_EfusePs_ReadRow(
			XSK_ZYNQMP_EFUSEPS_USR0_FUSE_ROW + UserFuse_Num,
			EfuseType, UseFusePtr);
		if (Status != (u32)XST_SUCCESS) {
			goto UNLOCK;
		}


UNLOCK:
		/* Lock the controller back */
		XilSKey_ZynqMp_EfusePs_CtrlrLock();
	}
	return Status;

}

/*****************************************************************************/
/**
* This function is used to read PPK0 hash from efuse or cache based on user's
* read option.
*
* @param	Ppk0Hash	A pointer to an array which holds the readback
*		PPK0 hash in.
* @param	ReadOption	A variable which has to be provided by user
*		based on this input reading is happend from cache or from efuse
*		array.
*		- 0	Reads from cache
*		- 1	Reads from efuse array
*
* @return
*		- XST_SUCCESS on successful read
*		- ErrorCode on failure
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_ReadPpk0Hash(u32 *Ppk0Hash, u8 ReadOption)
{
	u32 Status = (u32)XST_FAILURE;
	u32 Row;
	XskEfusePs_Type EfuseType = XSK_ZYNQMP_EFUSEPS_EFUSE_0;
	s32 RegNum;
	u32 DataRead;
	s32 Reg = (s32)(XSK_ZYNQMP_EFUSEPS_PPK_HASH_REG_NUM - 1U);
	u32 * Ppk0hashPtr = Ppk0Hash;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(Ppk0hashPtr != NULL);

	if (ReadOption == 0U) {
		for (RegNum = Reg; RegNum >= (s32)0; RegNum--) {
			DataRead = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_PPK0_0_OFFSET
						+ ((u32)RegNum * 4U));
			XilSKey_EfusePs_ConvertBytesBeToLe((u8 *)&DataRead, (u8 *)Ppk0hashPtr, 8);
			Ppk0hashPtr++;
		}
		Status = (u32)XST_SUCCESS;
	}
	else {
		/* Unlock the controller */
		XilSKey_ZynqMp_EfusePs_CtrlrUnLock();
		/* Check the unlock status */
		if (XilSKey_ZynqMp_EfusePs_CtrlrLockStatus() != 0U) {
			Status = (u32)(XSK_EFUSEPS_ERROR_CONTROLLER_LOCK);
			goto UNLOCK;
		}
		Status = XilSKey_ZynqMp_EfusePs_Init();
		if (Status != (u32)XST_SUCCESS) {
			goto UNLOCK;
		}
		/* Vol and temperature checks */
		Status = XilSKey_ZynqMp_EfusePs_Temp_Vol_Checks();
		if (Status != (u32)XST_SUCCESS) {
			goto UNLOCK;
		}

		for (Row = XSK_ZYNQMP_EFUSEPS_PPK0_SHA3_HASH_END_ROW;
			Row >= XSK_ZYNQMP_EFUSEPS_PPK0_START_ROW;
								Row--) {
			Status = XilSKey_ZynqMp_EfusePs_ReadRow((u8)Row,
						EfuseType, &DataRead);
			XilSKey_EfusePs_ConvertBytesBeToLe((u8 *)&DataRead, (u8 *)Ppk0hashPtr, 8);
			if (Status != (u32)XST_SUCCESS) {
				goto UNLOCK;
			}
			Ppk0hashPtr++;
		}

UNLOCK:
		/* Lock the controller back */
		XilSKey_ZynqMp_EfusePs_CtrlrLock();
	}
	return Status;

}

/*****************************************************************************/
/**
* This function is used to read PPK1 hash from efuse or cache based on user's
* read option.
*
* @param	Ppk1Hash	Pointer to an array which holds the readback
*		PPK1 hash in.
* @param	ReadOption	A variable which has to be provided by user
*		based on this input reading is happend from cache or from efuse
*		array.
*		- 0	Reads from cache
*		- 1	Reads from efuse array
*
* @return
*		- XST_SUCCESS on successful read
*		- ErrorCode on failure
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_ReadPpk1Hash(u32 *Ppk1Hash, u8 ReadOption)
{
	u32 Status = (u32)XST_FAILURE;
	u32 Row;
	XskEfusePs_Type EfuseType = XSK_ZYNQMP_EFUSEPS_EFUSE_0;
	s32 RegNum;
	u32 DataRead;
	s32 Reg = (s32)(XSK_ZYNQMP_EFUSEPS_PPK_HASH_REG_NUM - 1U);
	u32 * Ppk1hashPtr = Ppk1Hash;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(Ppk1hashPtr != NULL);

	if (ReadOption == 0U) {
		for (RegNum = Reg; RegNum >= (s32)0; RegNum--) {
			DataRead = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
					XSK_ZYNQMP_EFUSEPS_PPK1_0_OFFSET
							+ ((u32)RegNum * 4U));
			XilSKey_EfusePs_ConvertBytesBeToLe((u8 *)&DataRead, (u8 *)Ppk1hashPtr, 8U);
			Ppk1hashPtr++;
		}
		Status = (u32)XST_SUCCESS;
	}
	else {
		/* Unlock the controller */
		XilSKey_ZynqMp_EfusePs_CtrlrUnLock();
		/* Check the unlock status */
		if (XilSKey_ZynqMp_EfusePs_CtrlrLockStatus() != 0U) {
			Status = (u32)(XSK_EFUSEPS_ERROR_CONTROLLER_LOCK);
			goto UNLOCK;
		}
		Status = XilSKey_ZynqMp_EfusePs_Init();
		if (Status != (u32)XST_SUCCESS) {
			goto UNLOCK;
		}
		/* Vol and temperature checks */
		Status = XilSKey_ZynqMp_EfusePs_Temp_Vol_Checks();
		if (Status != (u32)XST_SUCCESS) {
			goto UNLOCK;
		}

		for (Row = XSK_ZYNQMP_EFUSEPS_PPK1_SHA3_HASH_END_ROW;
		Row >= XSK_ZYNQMP_EFUSEPS_PPK1_START_ROW; Row--) {
			Status = XilSKey_ZynqMp_EfusePs_ReadRow((u8)Row,
						EfuseType, &DataRead);
			if (Status != (u32)XST_SUCCESS) {
				goto UNLOCK;
			}
			XilSKey_EfusePs_ConvertBytesBeToLe((u8 *)&DataRead, (u8 *)Ppk1hashPtr, 8);
			Ppk1hashPtr++;
		}

UNLOCK:
		/* Lock the controller back */
		XilSKey_ZynqMp_EfusePs_CtrlrLock();
	}
	return Status;

}

/*****************************************************************************/
/**
* This function is used to read SPKID from efuse or cache based on user's
* read option.
*
* @param	SpkId	Pointer to a 32 bit variable which holds SPK ID.
* @param	ReadOption	A variable which has to be provided by user
*		based on this input reading is happend from cache or from efuse
*		array.
*		- 0	Reads from cache
*		- 1	Reads from efuse array
*
* @return
*		- XST_SUCCESS on successful read
*		- ErrorCode on failure
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_ReadSpkId(u32 *SpkId, u8 ReadOption)
{
	u32 Status = (u32)XST_FAILURE;
	XskEfusePs_Type EfuseType = XSK_ZYNQMP_EFUSEPS_EFUSE_0;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(SpkId != NULL);

	if (ReadOption == 0U) {
		*SpkId = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_SPK_ID_OFFSET);
		Status = (u32)XST_SUCCESS;
	}
	else {
		/* Unlock the controller */
		XilSKey_ZynqMp_EfusePs_CtrlrUnLock();
		/* Check the unlock status */
		if (XilSKey_ZynqMp_EfusePs_CtrlrLockStatus() != 0U) {
			Status = (u32)(XSK_EFUSEPS_ERROR_CONTROLLER_LOCK);
			goto UNLOCK;
		}
		Status = XilSKey_ZynqMp_EfusePs_Init();
		if (Status != (u32)XST_SUCCESS) {
			goto UNLOCK;
		}
		/* Vol and temperature checks */
		Status = XilSKey_ZynqMp_EfusePs_Temp_Vol_Checks();
		if (Status != (u32)XST_SUCCESS) {
			goto UNLOCK;
		}

		Status = XilSKey_ZynqMp_EfusePs_ReadRow(
			XSK_ZYNQMP_EFUSEPS_SPK_ID_ROW, EfuseType, SpkId);
		if (Status != (u32)XST_SUCCESS) {
			goto UNLOCK;
		}

UNLOCK:
		/* Lock the controller back */
		XilSKey_ZynqMp_EfusePs_CtrlrLock();

	}
	return Status;
}

/*****************************************************************************/
/**
* This function is used to read DNA from efuse.
*
* @param	DnaRead	Pointer to 32 bit variable which holds the
*		readback DNA in.
*
* @return	None.
*
******************************************************************************/
void XilSKey_ZynqMp_EfusePs_ReadDna(u32 *DnaRead)
{

	u32 *DnaPtr;

	/* Assert validates the input arguments */
	Xil_AssertVoid(DnaRead != NULL);

	DnaPtr = DnaRead;

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

	u32 Status = (u32)XST_FAILURE;
	u8 Row;
	u32 RowDataVal = 0U;

	for (Row = RowStart; Row <= RowEnd; Row++) {
		Status = XilSKey_ZynqMp_EfusePs_ReadRow(Row, EfuseType,
								&RowDataVal);
		if (Status != (u32)XST_SUCCESS) {
			break;
		}
		if (RowDataVal != 0x00U) {
			Status = (u32)XST_FAILURE;
			break;
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
	u32 Status = (u32)XST_FAILURE;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check for AES key with All zeros */
	if (InstancePtr->PrgrmAesKey == TRUE) {
		Status = XilSKey_ZynqMp_EfusePs_CheckAesKeyCrc(
				XSK_ZYNQMP_EFUSEPS_CRC_AES_ZEROS);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XSK_EFUSEPS_ERROR_AES_ALREADY_PROGRAMMED;
			goto END;
		}
	}

	/* Check Zeros for PPK0 hash */
	if (InstancePtr->PrgrmPpk0Hash == TRUE) {
		Status = XilSKey_ZynqMp_EfusePs_CheckForZeros(
			XSK_ZYNQMP_EFUSEPS_PPK0_START_ROW,
			XSK_ZYNQMP_EFUSEPS_PPK0_SHA3_HASH_END_ROW,
				XSK_ZYNQMP_EFUSEPS_EFUSE_0);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XSK_EFUSEPS_ERROR_PPK0_HASH_ALREADY_PROGRAMMED;
			goto END;
		}
	}

	/* Check Zeros for PPK0 hash */
	if (InstancePtr->PrgrmPpk1Hash == TRUE) {
		Status = XilSKey_ZynqMp_EfusePs_CheckForZeros(
			XSK_ZYNQMP_EFUSEPS_PPK1_START_ROW,
			XSK_ZYNQMP_EFUSEPS_PPK1_SHA3_HASH_END_ROW,
					XSK_ZYNQMP_EFUSEPS_EFUSE_0);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XSK_EFUSEPS_ERROR_PPK1_HASH_ALREADY_PROGRAMMED;
			goto END;
		}
	}
	Status = (u32)XST_SUCCESS;
END:
	return Status;

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
				XilSKey_UsrFuses *UserFuses_ToBePrgrmd)
{
	u32 UserFuseColumn;
	u32 Status;

	for (UserFuseColumn = 0U; UserFuseColumn < 32U; UserFuseColumn++) {
	/* If user requests a non-zero bit for making to zero throws an error*/
		if ((UserFuses_Write[UserFuseColumn] == 0U) &&
			(UserFuses_Read[UserFuseColumn] == 1U)) {
			Status = (u32)XST_FAILURE;
			goto END;
		}
		if ((UserFuses_Write[UserFuseColumn] == 1U) &&
			(UserFuses_Read[UserFuseColumn] == 0U)) {
			UserFuses_ToBePrgrmd->UserFuse[UserFuseColumn] = 1U;
		}
	}
	Status = (u32)XST_SUCCESS;
END:
	return Status;
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
	XilSKey_ZynqMpEPs *InstancePtr, XilSKey_UsrFuses *ToBePrgrmd)
{
	u8 UserFuses_Read[8][32] = {{0},{0},{0},{0},{0},{0},{0},{0}};
	u8 UserFuses_Write[8][32] = {{0},{0},{0},{0},{0},{0},{0},{0}};
	u32 UserFuseRead;
	XilSKey_UsrFuses *UserEFuseToPrg;
	u32 Status = (u32)XST_FAILURE;

	if (InstancePtr->PrgrmUser0Fuse == TRUE) {
		UserFuseRead = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
					XSK_ZYNQMP_EFUSEPS_USER_0_OFFSET);
		XilSKey_Efuse_ConvertBitsToBytes((u8 *)&UserFuseRead,
			&UserFuses_Read[XSK_ZYNQMP_EFUSEPS_USR0_FUSE][0],
			XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);
		XilSKey_Efuse_ConvertBitsToBytes(InstancePtr->User0Fuses,
			&(UserFuses_Write[XSK_ZYNQMP_EFUSEPS_USR0_FUSE][0]),
			XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);

		UserEFuseToPrg = (ToBePrgrmd + (u32)XSK_ZYNQMP_EFUSEPS_USR0_FUSE);
	/* Checking whether requested User FUSE bit programming is possible */
		if (XilSKey_ZynqMp_EfusePs_UserFuses_TobeProgrammed(
			&UserFuses_Write[XSK_ZYNQMP_EFUSEPS_USR0_FUSE][0],
			&UserFuses_Read[XSK_ZYNQMP_EFUSEPS_USR0_FUSE][0],
			UserEFuseToPrg) != (u32)XST_SUCCESS) {
			Status = ((u32)XSK_EFUSEPS_ERROR_WRITE_USER0_FUSE +
				(u32)XSK_EFUSEPS_ERROR_USER_BIT_CANT_REVERT);
			goto END;
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
		UserEFuseToPrg = (ToBePrgrmd + (u32)XSK_ZYNQMP_EFUSEPS_USR1_FUSE);
	/* Checking whether requested User FUSE bit programming is possible */
		if (XilSKey_ZynqMp_EfusePs_UserFuses_TobeProgrammed(
			&UserFuses_Write[XSK_ZYNQMP_EFUSEPS_USR1_FUSE][0],
			&UserFuses_Read[XSK_ZYNQMP_EFUSEPS_USR1_FUSE][0],
			UserEFuseToPrg) != (u32)XST_SUCCESS) {
			Status = ((u32)XSK_EFUSEPS_ERROR_WRITE_USER1_FUSE +
				(u32)XSK_EFUSEPS_ERROR_USER_BIT_CANT_REVERT);
			goto END;
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
		UserEFuseToPrg = (ToBePrgrmd + (u32)XSK_ZYNQMP_EFUSEPS_USR2_FUSE);
	/* Checking whether requested User FUSE bit programming is possible */
		if (XilSKey_ZynqMp_EfusePs_UserFuses_TobeProgrammed(
			&UserFuses_Write[XSK_ZYNQMP_EFUSEPS_USR2_FUSE][0],
			&UserFuses_Read[XSK_ZYNQMP_EFUSEPS_USR2_FUSE][0],
			UserEFuseToPrg) != (u32)XST_SUCCESS) {
			Status = ((u32)XSK_EFUSEPS_ERROR_WRITE_USER2_FUSE +
				(u32)XSK_EFUSEPS_ERROR_USER_BIT_CANT_REVERT);
			goto END;
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
		UserEFuseToPrg = (ToBePrgrmd + (u32)XSK_ZYNQMP_EFUSEPS_USR3_FUSE);
	/* Checking whether requested User FUSE bit programming is possible */
		if (XilSKey_ZynqMp_EfusePs_UserFuses_TobeProgrammed(
			&UserFuses_Write[XSK_ZYNQMP_EFUSEPS_USR3_FUSE][0],
			&UserFuses_Read[XSK_ZYNQMP_EFUSEPS_USR3_FUSE][0],
			UserEFuseToPrg) != (u32)XST_SUCCESS) {
			Status = ((u32)XSK_EFUSEPS_ERROR_WRITE_USER3_FUSE +
				(u32)XSK_EFUSEPS_ERROR_USER_BIT_CANT_REVERT);
			goto END;
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
		UserEFuseToPrg = (ToBePrgrmd + (u32)XSK_ZYNQMP_EFUSEPS_USR4_FUSE);
	/* Checking whether requested User FUSE bit programming is possible */
		if (XilSKey_ZynqMp_EfusePs_UserFuses_TobeProgrammed(
			&UserFuses_Write[XSK_ZYNQMP_EFUSEPS_USR4_FUSE][0],
			&UserFuses_Read[XSK_ZYNQMP_EFUSEPS_USR4_FUSE][0],
			UserEFuseToPrg) != (u32)XST_SUCCESS) {
			Status = ((u32)XSK_EFUSEPS_ERROR_WRITE_USER4_FUSE +
				(u32)XSK_EFUSEPS_ERROR_USER_BIT_CANT_REVERT);
			goto END;
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
		UserEFuseToPrg = (ToBePrgrmd + (u32)XSK_ZYNQMP_EFUSEPS_USR5_FUSE);
	/* Checking whether requested User FUSE bit programming is possible */
		if (XilSKey_ZynqMp_EfusePs_UserFuses_TobeProgrammed(
			&UserFuses_Write[XSK_ZYNQMP_EFUSEPS_USR5_FUSE][0],
			&UserFuses_Read[XSK_ZYNQMP_EFUSEPS_USR5_FUSE][0],
			UserEFuseToPrg) != (u32)XST_SUCCESS) {
			Status = ((u32)XSK_EFUSEPS_ERROR_WRITE_USER5_FUSE +
				(u32)XSK_EFUSEPS_ERROR_USER_BIT_CANT_REVERT);
			goto END;
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
		UserEFuseToPrg = (ToBePrgrmd + (u32)XSK_ZYNQMP_EFUSEPS_USR6_FUSE);
	/* Checking whether requested User FUSE bit programming is possible */
		if (XilSKey_ZynqMp_EfusePs_UserFuses_TobeProgrammed(
			&UserFuses_Write[XSK_ZYNQMP_EFUSEPS_USR6_FUSE][0],
			&UserFuses_Read[XSK_ZYNQMP_EFUSEPS_USR6_FUSE][0],
			UserEFuseToPrg) != (u32)XST_SUCCESS) {
			Status = ((u32)XSK_EFUSEPS_ERROR_WRITE_USER6_FUSE +
				(u32)XSK_EFUSEPS_ERROR_USER_BIT_CANT_REVERT);
			goto END;
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

		UserEFuseToPrg = (ToBePrgrmd + XSK_ZYNQMP_EFUSEPS_USR7_FUSE);
	/* Checking whether requested User FUSE bit programming is possible */
		if (XilSKey_ZynqMp_EfusePs_UserFuses_TobeProgrammed(
			&UserFuses_Write[XSK_ZYNQMP_EFUSEPS_USR7_FUSE][0],
			&UserFuses_Read[XSK_ZYNQMP_EFUSEPS_USR7_FUSE][0],
			UserEFuseToPrg) != (u32)XST_SUCCESS) {
			Status = ((u32)XSK_EFUSEPS_ERROR_WRITE_USER7_FUSE +
				(u32)XSK_EFUSEPS_ERROR_USER_BIT_CANT_REVERT);
			goto END;
		}
	}
	Status = (u32)XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/*
* This function programs RSA enable secure control bits of efuse
*
* @param	SecBits_read is a pointer which holds 32 bits of secure
*		control register.
*
* @return
*		XST_SUCCESS - On success
*		ErrorCode - on Failure
*
* @note		For ZynqMP silicon version 1.0 and 2.0 RSA authentication is
*		enabled only by programming 24 and 25 bits of SEC_CTRL register
*		but from silicon V3.0 bits 11:25 should be programmed
*
******************************************************************************/
static inline u32 XilSKey_ZynqMp_EfusePs_Enable_Rsa(
					u8 *SecBits_read)
{
	u32 Bit;
	u32 Status = (u32)XST_FAILURE;
	XskEfusePs_Type EfuseType = XSK_ZYNQMP_EFUSEPS_EFUSE_0;
	u32 Row = XSK_ZYNQMP_EFUSEPS_SEC_CTRL_ROW;
	u32 BitStart = 0U;
	u32 BitEnd = 0U;

#ifdef XSK_ZYNQ_ULTRA_MP_PLATFORM
	u32 Silicon_Ver = XGetPSVersion_Info();

	/* Program all 15 bits to enable RSA authentication */
	if (Silicon_Ver > (u32)XPS_VERSION_2) {
		BitStart = (u32)XSK_ZYNQMP_EFUSEPS_SEC_RSA_EN_BIT1;
		BitEnd = (u32)XSK_ZYNQMP_EFUSEPS_SEC_RSA_EN_BIT15;
	}
	/* Program only 24 and 25 bits of SEC_CTRL register */
	else {
		BitStart = (u32)XSK_ZYNQMP_EFUSEPS_SEC_RSA_EN_BIT14;
		BitEnd = (u32)XSK_ZYNQMP_EFUSEPS_SEC_RSA_EN_BIT15;
	}
#endif

	for (Bit = BitStart; Bit <= BitEnd; Bit++) {
		/* Program only if the bit is not already programmed */
		if (SecBits_read[Bit] == 0x00U) {
			Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit((u8)Row,
					(u8)Bit, EfuseType);
			if (Status != (u32)XST_SUCCESS) {
				goto END;
			}
		}
	}
	Status = (u32)XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/*
* This function initializes sysmonpsu driver and set all timing parameters.
*
* @param	None
*
* @return
*		XST_SUCCESS - On success
*		ErrorCode - on Failure
*
* @note		None.
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_Init(void)
{
	u32 Status = (u32)XST_FAILURE;

	if (Init_Done != TRUE) {
		/* Initialize sysmon PSU */
		Status = XilSKey_EfusePs_XAdcInit();
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		/* Set the timing constraints */
		XilSKey_ZynqMp_EfusePs_SetTimerValues();
		Init_Done = TRUE;
	}
	Status = (u32)XST_SUCCESS;

END:

	return Status;
}

/*****************************************************************************/
/*
* This function provides access to efuse memory
*
* @param	AddrHigh	Higher 32-bit address of the XilSKey_Efuse structure.
* @param	AddrLow		Lower 32-bit address of the XilSKey_Efuse structure.
*
* @return
*		XST_SUCCESS - On success
*		ErrorCode - on Failure
*
******************************************************************************/
u32 XilSkey_ZynqMpEfuseAccess(const u32 AddrHigh, const u32 AddrLow)
{
	u32 Status;
	u64 Addr = ((u64)AddrHigh << 32U) | AddrLow;
	XilSKey_Efuse *EfuseAccess = (XilSKey_Efuse *)(UINTPTR)Addr;

	/* Read bits */
	if (EfuseAccess->Flag == 0x0U) {
		Status = XilSKey_ZynqMpEfuseRead(AddrHigh, AddrLow);

	}
	/* Write bits */
	else {
		Status = XilSKey_ZynqMpEfuseWrite(AddrHigh, AddrLow);
	}

		return Status;
}

/*****************************************************************************/
/*
* This function provides support to program efuse memory
*
* @param	AddrHigh	Higher 32-bit address of the XilSKey_Efuse
*				structure.
* @param	AddrLow		Lower 32-bit address of the XilSKey_Efuse
				structure.
*
* @return
*		XST_SUCCESS - On success
*		ErrorCode - on Failure
*
* @note		None.
*
******************************************************************************/
static u32 XilSKey_ZynqMpEfuseWrite(const u32 AddrHigh, const u32 AddrLow)
{
	u32 Status = (u32)XST_FAILURE;
	u64 Addr = ((u64)AddrHigh << 32U) | AddrLow;
	XilSKey_Efuse *EfuseAccess = (XilSKey_Efuse *)(UINTPTR)Addr;
	u8 *Val = (u8 *)(UINTPTR)EfuseAccess->Src;
	u32 *Val32;
	XilSKey_ZynqMpEPs EfuseInstance = {0};
	u8 Index;
	u32 ReadReg;

	switch(EfuseAccess->Offset) {
		case (XSK_ZYNQMP_EFUSEPS_USER_0_OFFSET &
				XSK_EFUSEPS_OFFSET_MASK):
			if (EfuseAccess->Size != XSK_EFUSEPS_ONE_WORD) {
				Status = (u32)XSK_EFUSEPS_ERROR_BYTES_REQUEST;
				goto END;
			}
			EfuseInstance.PrgrmUser0Fuse = TRUE;
			for (Index = 0U;
			Index < XSK_ZYNQMP_EFUSEPS_USER_FUSE_ROW_LEN_IN_BYTES;
								Index++) {
				EfuseInstance.User0Fuses[Index] = *Val;
				Val = Val + 1U;
			}
			Status = (u32)XST_SUCCESS;
			break;
		case (XSK_ZYNQMP_EFUSEPS_USER_1_OFFSET &
				XSK_EFUSEPS_OFFSET_MASK):
			if (EfuseAccess->Size != XSK_EFUSEPS_ONE_WORD) {
				Status = (u32)XSK_EFUSEPS_ERROR_BYTES_REQUEST;
				goto END;
			}
			EfuseInstance.PrgrmUser1Fuse = TRUE;
			for (Index = 0U;
			 Index < XSK_ZYNQMP_EFUSEPS_USER_FUSE_ROW_LEN_IN_BYTES;
								Index++) {
				EfuseInstance.User1Fuses[Index] = *Val;
				Val = Val + 1U;
			}
			Status = (u32)XST_SUCCESS;
			break;
		case (XSK_ZYNQMP_EFUSEPS_USER_2_OFFSET &
				XSK_EFUSEPS_OFFSET_MASK):
			if (EfuseAccess->Size != XSK_EFUSEPS_ONE_WORD) {
				Status = (u32)XSK_EFUSEPS_ERROR_BYTES_REQUEST;
				goto END;
			}
			EfuseInstance.PrgrmUser2Fuse = TRUE;
			for (Index = 0U;
			 Index < XSK_ZYNQMP_EFUSEPS_USER_FUSE_ROW_LEN_IN_BYTES;
								Index++) {
				EfuseInstance.User2Fuses[Index] = *Val;
				Val = Val + 1U;
			}
			Status = (u32)XST_SUCCESS;
			break;
		case (XSK_ZYNQMP_EFUSEPS_USER_3_OFFSET &
				XSK_EFUSEPS_OFFSET_MASK):
			if (EfuseAccess->Size != XSK_EFUSEPS_ONE_WORD) {
				Status = (u32)XSK_EFUSEPS_ERROR_BYTES_REQUEST;
				goto END;
			}
			EfuseInstance.PrgrmUser3Fuse = TRUE;
			for (Index = 0U;
			 Index < XSK_ZYNQMP_EFUSEPS_USER_FUSE_ROW_LEN_IN_BYTES;
								Index++) {
				EfuseInstance.User3Fuses[Index] = *Val;
				Val = Val + 1U;
			}
			Status = (u32)XST_SUCCESS;
			break;
		case (XSK_ZYNQMP_EFUSEPS_USER_4_OFFSET &
				XSK_EFUSEPS_OFFSET_MASK):
			if (EfuseAccess->Size != XSK_EFUSEPS_ONE_WORD) {
				Status = (u32)XSK_EFUSEPS_ERROR_BYTES_REQUEST;
				goto END;
			}
			EfuseInstance.PrgrmUser4Fuse = TRUE;
			for (Index = 0U;
			 Index < XSK_ZYNQMP_EFUSEPS_USER_FUSE_ROW_LEN_IN_BYTES;
								Index++) {
				EfuseInstance.User4Fuses[Index] = *Val;
				Val = Val + 1U;
			}
			Status = (u32)XST_SUCCESS;
			break;
		case (XSK_ZYNQMP_EFUSEPS_USER_5_OFFSET &
				XSK_EFUSEPS_OFFSET_MASK):
			if (EfuseAccess->Size != XSK_EFUSEPS_ONE_WORD) {
				Status = (u32)XSK_EFUSEPS_ERROR_BYTES_REQUEST;
				goto END;
			}
			EfuseInstance.PrgrmUser5Fuse = TRUE;
			for (Index = 0U;
			 Index < XSK_ZYNQMP_EFUSEPS_USER_FUSE_ROW_LEN_IN_BYTES;
								Index++) {
				EfuseInstance.User5Fuses[Index] = *Val;
				Val = Val + 1U;
			}
			Status = (u32)XST_SUCCESS;
			break;
		case (XSK_ZYNQMP_EFUSEPS_USER_6_OFFSET &
				XSK_EFUSEPS_OFFSET_MASK):
			if (EfuseAccess->Size != XSK_EFUSEPS_ONE_WORD) {
				Status = (u32)XSK_EFUSEPS_ERROR_BYTES_REQUEST;
				goto END;
			}
			EfuseInstance.PrgrmUser6Fuse = TRUE;
			for (Index = 0U;
			 Index < XSK_ZYNQMP_EFUSEPS_USER_FUSE_ROW_LEN_IN_BYTES;
								Index++) {
				EfuseInstance.User6Fuses[Index] = *Val;
				Val = Val + 1U;
			}
			Status = (u32)XST_SUCCESS;
			break;
		case (XSK_ZYNQMP_EFUSEPS_USER_7_OFFSET &
				XSK_EFUSEPS_OFFSET_MASK):
			if (EfuseAccess->Size != XSK_EFUSEPS_ONE_WORD) {
				Status = (u32)XSK_EFUSEPS_ERROR_BYTES_REQUEST;
				goto END;
			}
			EfuseInstance.PrgrmUser7Fuse = TRUE;
			for (Index = 0U;
			 Index < XSK_ZYNQMP_EFUSEPS_USER_FUSE_ROW_LEN_IN_BYTES;
								Index++) {
				EfuseInstance.User7Fuses[Index] = *Val;
				Val = Val + 1U;
			}
			Status = (u32)XST_SUCCESS;
			break;
		case (XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_OFFSET &
				XSK_EFUSEPS_OFFSET_MASK):
			if (EfuseAccess->Size != XSK_EFUSEPS_ONE_WORD) {
				Status = (u32)XSK_EFUSEPS_ERROR_BYTES_REQUEST;
				goto END;
			}
			Val32 = (u32 *)Val;
			ReadReg = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_OFFSET);
			*Val32 = *Val32 & (~ReadReg);
			/* No new bits needs to be programmed */
			if (*Val32 == 0x00U) {
				Status = (u32)XST_SUCCESS;
				goto END;
			}
			if ((*Val32 &
			     ((u32)XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_RESERVED_MASK |
			    (u32)XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_LBIST_EN_MASK)) != 0U) {
				Status = (u32)XSK_EFUSEPS_ERROR_RESRVD_BITS_PRGRMG;
				goto END;
			}

			if ((*Val32 &
			   XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_0_MASK) !=
					0x00U) {
				EfuseInstance.PrgrmgSecCtrlBits.UserWrLk0 = TRUE;
			}
			if ((*Val32 &
			   XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_1_MASK) !=
					0x00U) {
				EfuseInstance.PrgrmgSecCtrlBits.UserWrLk1 = TRUE;
			}
			if ((*Val32 &
			   XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_2_MASK) !=
					0x00U) {
				EfuseInstance.PrgrmgSecCtrlBits.UserWrLk2 = TRUE;
			}
			if ((*Val32 &
			   XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_3_MASK) !=
					0x00U) {
				EfuseInstance.PrgrmgSecCtrlBits.UserWrLk3 = TRUE;
			}
			if ((*Val32 &
			   XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_4_MASK) !=
					0x00U) {
				EfuseInstance.PrgrmgSecCtrlBits.UserWrLk4 = TRUE;
			}
			if ((*Val32 &
			   XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_5_MASK) !=
					0x00U) {
				EfuseInstance.PrgrmgSecCtrlBits.UserWrLk5 = TRUE;
			}
			if ((*Val32 &
			   XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_6_MASK) !=
					0x00U) {
				EfuseInstance.PrgrmgSecCtrlBits.UserWrLk6 = TRUE;
			}
			if ((*Val32 &
			   XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_USR_WRLK_7_MASK) !=
					0x00U) {
				EfuseInstance.PrgrmgSecCtrlBits.UserWrLk7 = TRUE;
			}
			if ((*Val32 &
			   XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_FPD_SC_EN_MASK) !=
					0x00U) {
				EfuseInstance.PrgrmgSecCtrlBits.FpdScEn = TRUE;
			}
			if ((*Val32 &
			   XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_LPD_SC_EN_MASK) !=
					0x00U) {
				EfuseInstance.PrgrmgSecCtrlBits.LpdScEn = TRUE;
			}
			Status = (u32)XST_SUCCESS;
			break;
		case (XSK_ZYNQMP_EFUSEPS_SEC_CTRL_OFFSET &
				XSK_EFUSEPS_OFFSET_MASK):
			if (EfuseAccess->Size != XSK_EFUSEPS_ONE_WORD) {
				Status = (u32)XSK_EFUSEPS_ERROR_BYTES_REQUEST;
				goto END;
			}
			Val32 = (u32 *)Val;
			ReadReg = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
					XSK_ZYNQMP_EFUSEPS_SEC_CTRL_OFFSET);
			/* Only taking the bits to be programmed */
			*Val32 = *Val32 & (~ReadReg);
			if (*Val32 == 0x00U) {
				Status = (u32)XST_SUCCESS;
				goto END;
			}
			if ((*Val32 &
				(XSK_ZYNQMP_EFUSEPS_SEC_CTRL_RSA_EN_MASK |
				XSK_ZYNQMP_EFUSEPS_SEC_CTRL_ENC_ONLY_MASK |
				XSK_ZYNQMP_EFUSEPS_SEC_CTRL_JTAG_DIS_MASK |
				XSK_ZYNQMP_EFUSEPS_SEC_CTRL_DFT_DIS_MASK)) !=
								0x00U) {
				Status = (u32)XSK_EFUSEPS_ERROR_RESRVD_BITS_PRGRMG;
				goto END;
			}
			if ((*Val32 &
			   XSK_ZYNQMP_EFUSEPS_SEC_CTRL_AES_RDLK_MASK) != 0x00U) {
				EfuseInstance.PrgrmgSecCtrlBits.AesKeyRead =
									TRUE;
			}
			if ((*Val32 &
			   XSK_ZYNQMP_EFUSEPS_SEC_CTRL_AES_WRLK_MASK) != 0x00U) {
				EfuseInstance.PrgrmgSecCtrlBits.AesKeyWrite =
									TRUE;
			}
			if ((*Val32 &
			   XSK_ZYNQMP_EFUSEPS_SEC_CTRL_BBRAM_DIS_MASK) !=
									0x00U) {
				EfuseInstance.PrgrmgSecCtrlBits.BbramDisable =
									TRUE;
			}
			if ((*Val32 &
			    XSK_ZYNQMP_EFUSEPS_SEC_CTRL_ERR_DIS_MASK) != 0x00U) {
				EfuseInstance.PrgrmgSecCtrlBits.ErrorDisable =
									TRUE;
			}
			if ((*Val32 &
			   XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PROG_GATE_MASK) != 0x00U){
				EfuseInstance.PrgrmgSecCtrlBits.ProgGate = TRUE;
			}
			if ((*Val32 &
			  XSK_ZYNQMP_EFUSEPS_SEC_CTRL_LOCK_MASK) != 0x00U) {
				EfuseInstance.PrgrmgSecCtrlBits.SecureLock =
									TRUE;
			}
			if ((*Val32 &
			   XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PPK0_WRLK_MASK) !=
									0x00U) {
				EfuseInstance.PrgrmgSecCtrlBits.PPK0WrLock =
									TRUE;
			}
			if ((*Val32 &
			  XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PPK0_INVLD_MASK) !=
									0x00U) {
				EfuseInstance.PrgrmgSecCtrlBits.PPK0InVld =
									TRUE;
			}
			if ((*Val32 &
			  XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PPK1_WRLK_MASK) !=
									0x00U) {
				EfuseInstance.PrgrmgSecCtrlBits.PPK1WrLock =
									TRUE;
			}
			if ((*Val32 &
			  XSK_ZYNQMP_EFUSEPS_SEC_CTRL_PPK1_INVLD_MASK) !=
									0x00U) {
				EfuseInstance.PrgrmgSecCtrlBits.PPK1InVld =
									TRUE;
			}
			Status = (u32)XST_SUCCESS;
			break;
		case (XSK_ZYNQMP_EFUSEPS_SPK_ID_OFFSET &
				XSK_EFUSEPS_OFFSET_MASK):
			if (EfuseAccess->Size != XSK_EFUSEPS_ONE_WORD) {
				Status = (u32)XSK_EFUSEPS_ERROR_BYTES_REQUEST;
				goto END;
			}
			EfuseInstance.PrgrmSpkID = TRUE;
			for (Index = 0U;
			     Index < XSK_ZYNQMP_EFUSEPS_SPKID_LEN_IN_BYTES;
			     Index++) {
				EfuseInstance.SpkId[Index] = *Val;
				Val = Val + 1U;
			}
			Status = (u32)XST_SUCCESS;
			break;
		case (XSK_ZYNQMP_EFUSEPS_AES_KEY_START_ROW << 2U):
			if (EfuseAccess->Size !=
			    (XSK_ZYNQMP_EFUSEPS_AES_KEY_LEN_IN_BYTES >> 2U)) {
				Status = (u32)XSK_EFUSEPS_ERROR_BYTES_REQUEST;
				goto END;
			}
			EfuseInstance.PrgrmAesKey = TRUE;
			for (Index = 0U;
			     Index < XSK_ZYNQMP_EFUSEPS_AES_KEY_LEN_IN_BYTES;
			     Index++) {
				EfuseInstance.AESKey[Index] = *Val;
				Val = Val + 1U;
			}
			Status = (u32)XST_SUCCESS;
			break;
		case (XSK_ZYNQMP_EFUSEPS_PPK0_0_OFFSET &
				XSK_EFUSEPS_OFFSET_MASK):
			if (EfuseAccess->Size !=
			    (XSK_ZYNQMP_EFUSEPS_PPK_HASH_LEN_IN_BYTES >> 2)) {
				Status = (u32)XSK_EFUSEPS_ERROR_BYTES_REQUEST;
				goto END;
			}
			EfuseInstance.PrgrmPpk0Hash = TRUE;
			EfuseInstance.IsPpk0Sha3Hash = TRUE;
			for (Index = 0U;
			     Index < XSK_ZYNQMP_EFUSEPS_PPK_HASH_LEN_IN_BYTES;
			     Index++) {
				EfuseInstance.Ppk0Hash[Index] = *Val;
				Val = Val + 1U;
			}
			Status = (u32)XST_SUCCESS;
			break;
		case (XSK_ZYNQMP_EFUSEPS_PPK1_0_OFFSET &
				XSK_EFUSEPS_OFFSET_MASK):
			if (EfuseAccess->Size !=
			   (XSK_ZYNQMP_EFUSEPS_PPK_HASH_LEN_IN_BYTES >> 2U)) {
				Status = (u32)XSK_EFUSEPS_ERROR_BYTES_REQUEST;
				goto END;
			}
			EfuseInstance.PrgrmPpk1Hash = TRUE;
			EfuseInstance.IsPpk0Sha3Hash = TRUE;
			for (Index = 0U;
			     Index < XSK_ZYNQMP_EFUSEPS_PPK_HASH_LEN_IN_BYTES;
			     Index++) {
				EfuseInstance.Ppk1Hash[Index] = *Val;
				Val = Val + 1U;
			}
			Status = (u32)XST_SUCCESS;
			break;
		default:
			Status = (u32)XSK_EFUSEPS_ERROR_ADDR_ACCESS;
			break;
	}

	if (Status == (u32)XST_SUCCESS) {
		Status = XilSKey_ZynqMp_EfusePs_Write(&EfuseInstance);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}
END:

	return Status;
}

/*****************************************************************************/
/*
* This function provides support to read user efuses
*
* @param	Offset	Offset specifies the user fuses offset to be read.
* @param	Buffer	Requested user fuses values will be stored in this
*			pointer.
* @param	Size	To be specified in words.
* @param	UsrFuseNum	Userfuse number
*
* @return
*		XST_SUCCESS - On success
*		ErrorCode - on Failure
*
* @note		None.
*
******************************************************************************/
static u32 XilSkey_ZynqMpUsrFuseRd(u32 Offset, u32 *Buffer,
					u32 Size, u8 UsrFuseNum)
{
	u32 Status;
	u8 FuseNum = UsrFuseNum;
	u32 *Value = Buffer;
	u32 Words = Size;

	/* Check if the requested bytes are exceeding */
	if ((Offset + (Words * XSK_EFUSEPS_BYTES_IN_WORD) -
				XSK_EFUSEPS_BYTES_IN_WORD) >
		((XSK_ZYNQMP_EFUSEPS_USER_7_OFFSET & 0xFFU))) {
		Status = (u32)XSK_EFUSEPS_ERROR_BYTES_REQUEST;
		goto END;
	}
	do {
		Status = XilSKey_ZynqMp_EfusePs_ReadUserFuse(Value, FuseNum,
				XSK_EFUSEPS_READ_FROM_CACHE);
		Value++;
		FuseNum++;
		Words--;
	} while (Words != 0U);
END:
	return Status;
}

/*****************************************************************************/
/*
* This function provides support to read  efuse memory
*
* @param	AddrHigh	Higher 32-bit address of the
*				XilSKey_Efuse structure.
* @param	AddrLow		Lower 32-bit address of the XilSKey_Efuse
*				structure.
*
* @return
*		XST_SUCCESS - On success
*		ErrorCode - on Failure
*
* @note		None.
*
******************************************************************************/
static u32 XilSKey_ZynqMpEfuseRead(const u32 AddrHigh, const u32 AddrLow)
{
	u32 Status = (u32)XST_FAILURE;
	u64 Addr = ((u64)AddrHigh << 32) | AddrLow;
	XilSKey_Efuse *EfuseAccess = (XilSKey_Efuse *)(UINTPTR)Addr;
	u32 *Val = (u32 *)(UINTPTR)EfuseAccess->Src;
	u8 UsrEfuseNo;

	switch(EfuseAccess->Offset) {
		case (XSK_ZYNQMP_EFUSEPS_DNA_0_OFFSET &
			XSK_EFUSEPS_OFFSET_MASK):
			if (EfuseAccess->Size !=
				(XSK_ZYNQMP_EFUSEPS_DNA_LEN_IN_BYTES >> 2)) {
				Status = (u32)XSK_EFUSEPS_ERROR_BYTES_REQUEST;
				goto END;
			}
			XilSKey_ZynqMp_EfusePs_ReadDna(Val);
			Status = (u32)XST_SUCCESS;
			break;
		case (XSK_ZYNQMP_EFUSEPS_USER_0_OFFSET &
			XSK_EFUSEPS_OFFSET_MASK):
			UsrEfuseNo = XSK_ZYNQMP_EFUSEPS_USR0_FUSE;
			Status = XilSkey_ZynqMpUsrFuseRd(EfuseAccess->Offset,
					Val, EfuseAccess->Size,
					UsrEfuseNo);
			break;
		case (XSK_ZYNQMP_EFUSEPS_USER_1_OFFSET &
				XSK_EFUSEPS_OFFSET_MASK):
			UsrEfuseNo = XSK_ZYNQMP_EFUSEPS_USR1_FUSE;
			Status = XilSkey_ZynqMpUsrFuseRd(EfuseAccess->Offset,
							 Val,
							 EfuseAccess->Size,
							 UsrEfuseNo);
			break;
		case (XSK_ZYNQMP_EFUSEPS_USER_2_OFFSET &
				XSK_EFUSEPS_OFFSET_MASK):
			UsrEfuseNo = XSK_ZYNQMP_EFUSEPS_USR2_FUSE;
			Status = XilSkey_ZynqMpUsrFuseRd(EfuseAccess->Offset,
							 Val,
							 EfuseAccess->Size,
							 UsrEfuseNo);
			break;
		case (XSK_ZYNQMP_EFUSEPS_USER_3_OFFSET &
				XSK_EFUSEPS_OFFSET_MASK):
			UsrEfuseNo = XSK_ZYNQMP_EFUSEPS_USR3_FUSE;
			Status = XilSkey_ZynqMpUsrFuseRd(EfuseAccess->Offset,
							 Val,
							 EfuseAccess->Size,
							 UsrEfuseNo);
			break;
		case (XSK_ZYNQMP_EFUSEPS_USER_4_OFFSET &
				XSK_EFUSEPS_OFFSET_MASK):
			UsrEfuseNo = XSK_ZYNQMP_EFUSEPS_USR4_FUSE;
			Status = XilSkey_ZynqMpUsrFuseRd(EfuseAccess->Offset,
							 Val,
							 EfuseAccess->Size,
							 UsrEfuseNo);
			break;
		case (XSK_ZYNQMP_EFUSEPS_USER_5_OFFSET &
				XSK_EFUSEPS_OFFSET_MASK):
			UsrEfuseNo = XSK_ZYNQMP_EFUSEPS_USR5_FUSE;
			Status = XilSkey_ZynqMpUsrFuseRd(EfuseAccess->Offset,
							 Val,
							 EfuseAccess->Size,
							 UsrEfuseNo);
			break;
		case (XSK_ZYNQMP_EFUSEPS_USER_6_OFFSET &
				XSK_EFUSEPS_OFFSET_MASK):
			UsrEfuseNo = XSK_ZYNQMP_EFUSEPS_USR6_FUSE;
			Status = XilSkey_ZynqMpUsrFuseRd(EfuseAccess->Offset,
							 Val,
							 EfuseAccess->Size,
							 UsrEfuseNo);
			break;
		case (XSK_ZYNQMP_EFUSEPS_USER_7_OFFSET &
				XSK_EFUSEPS_OFFSET_MASK):
			UsrEfuseNo = XSK_ZYNQMP_EFUSEPS_USR7_FUSE;
			Status = XilSkey_ZynqMpUsrFuseRd(EfuseAccess->Offset,
							 Val,
							 EfuseAccess->Size,
							 UsrEfuseNo);
			break;
		case (XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_OFFSET &
				XSK_EFUSEPS_OFFSET_MASK):
			if (EfuseAccess->Size != XSK_EFUSEPS_ONE_WORD) {
				Status = (u32)XSK_EFUSEPS_ERROR_BYTES_REQUEST;
				goto END;
			}
			*Val = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_MISC_USER_CTRL_OFFSET);
			Status = (u32)XST_SUCCESS;
			break;
		case (XSK_ZYNQMP_EFUSEPS_PUF_CHASH_OFFSET &
				XSK_EFUSEPS_OFFSET_MASK):
			if (EfuseAccess->Size != XSK_EFUSEPS_ONE_WORD) {
				Status = (u32)XSK_EFUSEPS_ERROR_BYTES_REQUEST;
				goto END;
			}
			*Val = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_PUF_CHASH_OFFSET);
			Status = (u32)XST_SUCCESS;
			break;
		case (XSK_ZYNQMP_EFUSEPS_PUF_MISC_OFFSET &
				XSK_EFUSEPS_OFFSET_MASK):
			if (EfuseAccess->Size != XSK_EFUSEPS_ONE_WORD) {
				Status = (u32)XSK_EFUSEPS_ERROR_BYTES_REQUEST;
				goto END;
			}
			*Val = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
					XSK_ZYNQMP_EFUSEPS_PUF_MISC_OFFSET);
			Status = (u32)XST_SUCCESS;
			break;
		case (XSK_ZYNQMP_EFUSEPS_SEC_CTRL_OFFSET &
				XSK_EFUSEPS_OFFSET_MASK):
			if (EfuseAccess->Size != XSK_EFUSEPS_ONE_WORD) {
				Status = (u32)XSK_EFUSEPS_ERROR_BYTES_REQUEST;
				goto END;
			}
			*Val = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
					XSK_ZYNQMP_EFUSEPS_SEC_CTRL_OFFSET);
			Status = (u32)XST_SUCCESS;
			break;
		case (XSK_ZYNQMP_EFUSEPS_SPK_ID_OFFSET &
				XSK_EFUSEPS_OFFSET_MASK):
			if (EfuseAccess->Size != XSK_EFUSEPS_ONE_WORD) {
				Status = (u32)XSK_EFUSEPS_ERROR_BYTES_REQUEST;
				goto END;
			}
			Status = XilSKey_ZynqMp_EfusePs_ReadSpkId(Val,
					XSK_EFUSEPS_READ_FROM_CACHE);
			break;
		case (XSK_ZYNQMP_EFUSEPS_PPK0_0_OFFSET &
				XSK_EFUSEPS_OFFSET_MASK):
			if (EfuseAccess->Size !=
			    (XSK_ZYNQMP_EFUSEPS_PPK_HASH_LEN_IN_BYTES >> 2)) {
				Status = (u32)XSK_EFUSEPS_ERROR_BYTES_REQUEST;
				goto END;
			}
			Status = XilSKey_ZynqMp_EfusePs_ReadPpk0Hash(Val,
					XSK_EFUSEPS_READ_FROM_CACHE);
			break;
		case (XSK_ZYNQMP_EFUSEPS_PPK1_0_OFFSET &
				XSK_EFUSEPS_OFFSET_MASK):
			if (EfuseAccess->Size !=
			    (XSK_ZYNQMP_EFUSEPS_PPK_HASH_LEN_IN_BYTES >> 2)) {
				Status = (u32)XSK_EFUSEPS_ERROR_BYTES_REQUEST;
				goto END;
			}
			Status = XilSKey_ZynqMp_EfusePs_ReadPpk1Hash(Val,
						XSK_EFUSEPS_READ_FROM_CACHE);
			break;
		default:
			Status = (u32)XSK_EFUSEPS_ERROR_ADDR_ACCESS;
			break;
	}
END:
	return Status;
}
