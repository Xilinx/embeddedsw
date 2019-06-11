/******************************************************************************
*
* Copyright (C) 2016 - 2019 Xilinx, Inc.  All rights reserved.
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
* @file xilskey_eps_zynqmp_puf.c
* This file contains the APIs for registring PUF, eFUSE programming and reading
* the PUF helper data, CHASH and Auxilary data.
*
* </pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- --------------------------------------------------------
* 6.1   vns  10/17/16 First release.
*       vns  11/07/16 Fixed shutter value to 0x0100005e, as sysosc selection
*                     is fixed for PUF registration.
* 6.2   vns  02/18/17 Added masking for PUF auxilary read.
* 6.6   vns  06/06/18 Added doxygen tags
* 6.7	arc  01/05/19 Fixed MISRA-C violations.
*       arc  03/15/19 Modified initial default status value as XST_FAILURE
*       mmd  03/17/19 Handled buffer underflow issue and added timeouts during
*                     syndrome data reading
*       rama 03/25/19 Added polling routine for PUF ready state
* </pre>
*
*****************************************************************************/

/***************************** Include Files *********************************/

#include "xilskey_eps_zynqmp_puf.h"
#include "xilskey_eps_zynqmp_hw.h"
#include "sleep.h"
/************************** Constant Definitions *****************************/
#define XILSKEY_PUF_STATUS_SYN_WRD_RDY_TIMEOUT	(500000U)

/**************************** Type Definitions ******************************/
typedef enum {
	XSK_EFUSEPS_PUF_REGISTRATION_STARTED,
	XSK_EFUSEPS_PUF_REGISTRATION_COMPLETE
} XilsKey_PufRegistrationState;


/***************** Macros (Inline Functions) Definitions ********************/


/************************** Variable Definitions ****************************/


/************************** Function Prototypes *****************************/

u32 XilSKey_ZynqMp_EfusePs_SetWriteConditions(void);
u32 XilSKey_ZynqMp_EfusePs_CheckForZeros(u8 RowStart, u8 RowEnd,
						XskEfusePs_Type EfuseType);
u32 XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(u8 Row, u8 Column,
						XskEfusePs_Type EfuseType);
u32 XilSKey_ZynqMp_EfusePs_ReadRow(u8 Row, XskEfusePs_Type EfuseType,
							u32 *RowData);
u32 XilSKey_ZynqMp_EfusePs_Init(void);
static inline u32 XilSkey_Puf_Validate_Access_Rules(u8 RequestType);
static inline u32 XilSKey_ZynqMp_EfusePs_CheckZeros_Puf(void);

static inline u32 XilSKey_ZynqMp_EfusePs_PufRowWrite(u8 Row, u8 *Data,
						XskEfusePs_Type EfuseType);
static inline u32 XilSKey_Read_Puf_EfusePs_SecureBits_Regs(
		XilSKey_Puf_Secure *SecureBits, u8 ReadOption);
static u32  XilSKey_WaitForPufStatus(u32 *PufStatus);
/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* This function programs the PS efuse's with puf helper data of ZynqMp.
*
* @param	InstancePtr	Pointer to the XilSKey_Puf instance.
*
* @return
*		- XST_SUCCESS if programs successfully.
*		- Errorcode on failure
*
* @note		To generate PufSyndromeData please use
*		XilSKey_Puf_Registration API
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_WritePufHelprData(XilSKey_Puf *InstancePtr)
{
	u8 Row;
	u32 Data;
	u32 *DataPtr = InstancePtr->EfuseSynData;
	u32 *TempPtr = InstancePtr->EfuseSynData;
	XskEfusePs_Type EfuseType;
	u8 DataInBits[32];
	u32 Status;

	/* Unlock the controller */
	XilSKey_ZynqMp_EfusePs_CtrlrUnLock();

	/* Check the unlock status */
	if (XilSKey_ZynqMp_EfusePs_CtrlrLockStatus() != 0U) {
		Status = (u32)(XSK_EFUSEPS_ERROR_CONTROLLER_LOCK);
		goto END;
	}

	Status = XilSKey_ZynqMp_EfusePs_SetWriteConditions();
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	/* Check for zeros */
	Status = XilSKey_ZynqMp_EfusePs_CheckZeros_Puf();
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	EfuseType = XSK_ZYNQMP_EFUSEPS_EFUSE_2;
	/* Write Helper Data */
	for (Row = 0U; Row <= XSK_ZYNQMP_EFUSEPS_PUF_ROW_END; Row++) {
		if (Row == 0U) {
			Data = (u32)(((*DataPtr) &
				XSK_ZYNQMP_EFUSEPS_PUF_ROW_UPPER_MASK) >>
				XSK_ZYNQMP_EFUSEPS_PUF_ROW_HALF_WORD_SHIFT);
		}
		else {
			Data = (u32)((((*DataPtr) &
				XSK_ZYNQMP_EFUSEPS_PUF_ROW_UPPER_MASK) >>
				XSK_ZYNQMP_EFUSEPS_PUF_ROW_HALF_WORD_SHIFT) |
			(((*TempPtr) & XSK_ZYNQMP_EFUSEPS_PUF_ROW_LOWER_MASK) <<
				XSK_ZYNQMP_EFUSEPS_PUF_ROW_HALF_WORD_SHIFT));
		}
		TempPtr = DataPtr;
		DataPtr++;
		XilSKey_Efuse_ConvertBitsToBytes((u8 *)&Data, DataInBits,
				XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);
		Status = XilSKey_ZynqMp_EfusePs_PufRowWrite(Row, DataInBits, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}
	DataPtr--;

	EfuseType = XSK_ZYNQMP_EFUSEPS_EFUSE_3;
	for (Row = 0U; Row <= XSK_ZYNQMP_EFUSEPS_PUF_ROW_END; Row++) {
		if (Row == 0U) {
			Data = (u32)((*DataPtr) &
				XSK_ZYNQMP_EFUSEPS_PUF_ROW_LOWER_MASK);
		}
		else {
			Data = *DataPtr;
		}
		DataPtr++;

		XilSKey_Efuse_ConvertBitsToBytes((u8 *)&Data, DataInBits,
				XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);
		Status = XilSKey_ZynqMp_EfusePs_PufRowWrite(Row, DataInBits,
								EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}
END:
	/* Lock the controller back */
	XilSKey_ZynqMp_EfusePs_CtrlrLock();
	XilSKey_ZynqMp_EfusePS_PrgrmDisable();

	return Status;

}

/*****************************************************************************/
/**
* This function reads the puf helper data from eFUSE.
*
* @param	Address		Pointer to data array which holds the Puf helper
*		data read from ZynqMp efuse.
*
* @return
*		- XST_SUCCESS if reads successfully.
*		- Errorcode on failure.
*
* @note		This function only reads from eFUSE non-volatile memory. There
*		is no option to read from Cache.
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_ReadPufHelprData(u32 *Address)
{

	u32 Status;
	u32 Row;
	u32 RowDataVal[128] = {0U};
	u32 *PtrEfuse2 = &RowDataVal[0];
	u32 *PtrEfuse3 = &RowDataVal[64];
	u32 *AddrPtr = (u32 *)Address;
	u32 Temp;

	/* Unlock the controller */
	XilSKey_ZynqMp_EfusePs_CtrlrUnLock();
	/* Check the unlock status */
	if (XilSKey_ZynqMp_EfusePs_CtrlrLockStatus() != 0U) {
		Status = (u32)(XSK_EFUSEPS_ERROR_CONTROLLER_LOCK);
		goto END;
	}
	/* Init timer and AMS */
	Status = XilSKey_ZynqMp_EfusePs_Init();
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	for (Row = 0U; Row <= XSK_ZYNQMP_EFUSEPS_PUF_ROW_END; Row++) {
		Status = XilSKey_ZynqMp_EfusePs_ReadRow((u8)Row,
			XSK_ZYNQMP_EFUSEPS_EFUSE_2, PtrEfuse2);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		Status = XilSKey_ZynqMp_EfusePs_ReadRow((u8)Row,
			XSK_ZYNQMP_EFUSEPS_EFUSE_3, PtrEfuse3);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		PtrEfuse2++;
		PtrEfuse3++;

	}

	for (Row = 0U; Row < XSK_ZYNQMP_EFUSEPS_PUF_ROW_END; Row++) {
		Temp = (RowDataVal[Row] & XSK_ZYNQMP_EFUSEPS_PUF_ROW_LOWER_MASK) <<
				XSK_ZYNQMP_EFUSEPS_PUF_ROW_HALF_WORD_SHIFT;
		Temp = ((RowDataVal[Row + 1] &
			XSK_ZYNQMP_EFUSEPS_PUF_ROW_UPPER_MASK) >>
			XSK_ZYNQMP_EFUSEPS_PUF_ROW_HALF_WORD_SHIFT) | Temp;

		*AddrPtr = Temp;
		AddrPtr++;
	}
	for (Row = XSK_ZYNQMP_EFUSEPS_PUF_ROW_END;
			Row < (XSK_ZYNQMP_EFUSEPS_PUF_TOTAL_ROWS - 1U);
								Row++) {
		if (Row == XSK_ZYNQMP_EFUSEPS_PUF_ROW_END) {
			Temp = (RowDataVal[Row] &
				XSK_ZYNQMP_EFUSEPS_PUF_ROW_LOWER_MASK) <<
				XSK_ZYNQMP_EFUSEPS_PUF_ROW_HALF_WORD_SHIFT;
			Temp = Temp | ((RowDataVal[Row + 1] &
				XSK_ZYNQMP_EFUSEPS_PUF_ROW_LOWER_MASK));
		}
		else {
			Temp = RowDataVal[Row + 1];
		}
		*AddrPtr = Temp;
		AddrPtr++;
	}

END:
	/* Lock the controller back */
	XilSKey_ZynqMp_EfusePs_CtrlrLock();

	return Status;

}

/*****************************************************************************/
/**
* This API programs eFUSE with CHash value.
*
* @param	InstancePtr	Pointer to the XilSKey_Puf instance.
*
* @return
*		- XST_SUCCESS if chash is programmed successfully.
*		- Errorcode on failure
*
* @note		To generate CHash value please use
*		XilSKey_Puf_Registration API
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_WritePufChash(XilSKey_Puf *InstancePtr)
{

	u32 Status;
	u8 Value[32] = {0U};
	u8 Column;
	XskEfusePs_Type EfuseType;
	u8 *PufChash = (u8 *)&(InstancePtr->Chash);

	/* Unlock the controller */
	XilSKey_ZynqMp_EfusePs_CtrlrUnLock();
	/* Check the unlock status */
	if (XilSKey_ZynqMp_EfusePs_CtrlrLockStatus() != 0U) {
		Status = (u32)(XSK_EFUSEPS_ERROR_CONTROLLER_LOCK);
		goto END;
	}

	Status = XilSKey_ZynqMp_EfusePs_SetWriteConditions();
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	EfuseType = XSK_ZYNQMP_EFUSEPS_EFUSE_0;
	/* Check for Zeros */
	if (XilSKey_ZynqMp_EfusePs_CheckForZeros(
		XSK_ZYNQMP_EFUSEPS_PUF_CHASH_ROW,
		XSK_ZYNQMP_EFUSEPS_PUF_CHASH_ROW, EfuseType) != (u32)XST_SUCCESS) {
		Status = (u32)XST_FAILURE; /* Error code */
		goto END;
	}

	XilSKey_Efuse_ConvertBitsToBytes((u8 *)PufChash, Value,
			XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);

	for (Column = 0U; Column < XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW;
							Column++) {
		if (Value[Column] != 0x00U) {
			Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(
					XSK_ZYNQMP_EFUSEPS_PUF_CHASH_ROW,
						Column, EfuseType);
			if (Status != (u32)XST_SUCCESS) {
				goto END;
			}
		}
	}
END:
	/* Lock the controller back */
	XilSKey_ZynqMp_EfusePs_CtrlrLock();
	XilSKey_ZynqMp_EfusePS_PrgrmDisable();

	return Status;
}

/*****************************************************************************/
/**
* This API reads efuse puf CHash Data from efuse array or cache based on the
* user read option.
*
* @param	Address	Pointer which holds the read back value of chash
* @param	ReadOption	A u8 variable which has to be provided by user
*		based on this input reading is happend from cache or from efuse
*		array.
*		- 0(XSK_EFUSEPS_READ_FROM_CACHE)Reads from cache
*		- 1(XSK_EFUSEPS_READ_FROM_EFUSE)Reads from efuse array
*
* @return
*		- XST_SUCCESS if programs successfully.
*		- Errorcode on failure
*
* @note		Cache reload is required for obtaining updated values for
*		ReadOption 0.
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_ReadPufChash(u32 *Address, u8 ReadOption)
{

	u32 Data;
	u32 Status = (u32)XST_FAILURE;
	u32 *ChashPtr = (u32 *)Address;

	if (ReadOption == XSK_EFUSEPS_READ_FROM_EFUSE) {
		/* Unlock the controller */
		XilSKey_ZynqMp_EfusePs_CtrlrUnLock();
		/* Check the unlock status */
		if (XilSKey_ZynqMp_EfusePs_CtrlrLockStatus() != 0U) {
			Status = (u32)(XSK_EFUSEPS_ERROR_CONTROLLER_LOCK);
			goto END;
		}
		/* Init timer and AMS */
		Status = XilSKey_ZynqMp_EfusePs_Init();
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		Status = XilSKey_ZynqMp_EfusePs_ReadRow(
			XSK_ZYNQMP_EFUSEPS_PUF_CHASH_ROW,
			XSK_ZYNQMP_EFUSEPS_EFUSE_0, &Data);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
END:
		/* Lock the controller back */
		XilSKey_ZynqMp_EfusePs_CtrlrLock();
	}
	else {
		Data = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_PUF_CHASH_OFFSET);
		Status = (u32)XST_SUCCESS;
	}

	*ChashPtr = Data;

	return Status;

}

/*****************************************************************************/
/**
* This API programs efuse puf Auxilary Data.
*
* @param	InstancePtr	Pointer to the XilSKey_Puf instance.
*
* @return
*		- XST_SUCCESS if programs successfully.
*		- Errorcode on failure
*
* @note		To generate Auxilary data please use the below API
*		u32 XilSKey_Puf_Registration(XilSKey_Puf *InstancePtr)
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_WritePufAux(XilSKey_Puf *InstancePtr)
{

	u32 Status;
	u8 Value[32] = {0U};
	u8 Column;
	XskEfusePs_Type EfuseType;
	u32 RowDataVal;
	u8 *AuxValue = (u8 *)&(InstancePtr->Aux);

	/* Unlock the controller */
	XilSKey_ZynqMp_EfusePs_CtrlrUnLock();

	/* Check the unlock status */
	if (XilSKey_ZynqMp_EfusePs_CtrlrLockStatus() != 0U) {
		Status = (u32)(XSK_EFUSEPS_ERROR_CONTROLLER_LOCK);
		goto END;
	}

	Status = XilSKey_ZynqMp_EfusePs_SetWriteConditions();
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	EfuseType = XSK_ZYNQMP_EFUSEPS_EFUSE_0;
	/* Check for Zeros */
	if (XilSKey_ZynqMp_EfusePs_ReadRow(
		XSK_ZYNQMP_EFUSEPS_PUF_AUX_ROW, EfuseType, &RowDataVal)
						!= (u32)XST_SUCCESS) {
		Status = (u32)XST_FAILURE; /* Error code */
		goto END;
	}
	if ((RowDataVal & XSK_ZYNQMP_EFUSEPS_PUF_MISC_AUX_MASK) != 0x00U) {
		Status = (u32)XST_FAILURE;
		goto END;
	}

	XilSKey_Efuse_ConvertBitsToBytes((u8 *)AuxValue, Value,
					XSK_ZYNQMP_PUF_AUX_LEN_IN_BITS);

	for (Column = 0U; Column < XSK_ZYNQMP_PUF_AUX_LEN_IN_BITS;
							Column++) {
		if (Value[Column] != 0x00U) {
			Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(
					XSK_ZYNQMP_EFUSEPS_PUF_AUX_ROW,
					Column, EfuseType);
			if (Status != (u32)XST_SUCCESS) {
				goto END;
			}
		}
	}
END:
	/* Lock the controller back */
	XilSKey_ZynqMp_EfusePs_CtrlrLock();
	XilSKey_ZynqMp_EfusePS_PrgrmDisable();

	return Status;

}

/*****************************************************************************/
/**
* This API reads efuse puf Auxilary Data from efuse array or cache based on
* user read option.
*
* @param	Address 	Pointer which holds the read back value of Auxilary
* @param	ReadOption	A u8 variable which has to be provided by user
*		based on this input reading is happend from cache or from efuse
*		array.
*		- 0(XSK_EFUSEPS_READ_FROM_CACHE)Reads from cache
*		- 1(XSK_EFUSEPS_READ_FROM_EFUSE)Reads from efuse array
*
* @return
* 		- XST_SUCCESS if programs successfully.
* 		- Errorcode on failure
*
* @note		Cache reload is required for obtaining updated values for
*		ReadOption 0.
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_ReadPufAux(u32 *Address, u8 ReadOption)
{

	u32 Data;
	u32 Status = (u32)XST_FAILURE;
	u32 *AuxPtr = (u32 *)Address;

	if (ReadOption == XSK_EFUSEPS_READ_FROM_EFUSE) {
		/* Unlock the controller */
		XilSKey_ZynqMp_EfusePs_CtrlrUnLock();
		/* Check the unlock status */
		if (XilSKey_ZynqMp_EfusePs_CtrlrLockStatus() != 0U) {
			Status = (u32)(XSK_EFUSEPS_ERROR_CONTROLLER_LOCK);
			goto END;
		}
		/* Init timer and AMS */
		Status = XilSKey_ZynqMp_EfusePs_Init();
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		Status = XilSKey_ZynqMp_EfusePs_ReadRow(
				XSK_ZYNQMP_EFUSEPS_PUF_AUX_ROW,
				XSK_ZYNQMP_EFUSEPS_EFUSE_0, &Data);
		Data = Data & XSK_ZYNQMP_EFUSEPS_PUF_MISC_AUX_MASK;
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}

END:
		/* Lock the controller back */
		XilSKey_ZynqMp_EfusePs_CtrlrLock();
	}
	else {
		Data = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
			XSK_ZYNQMP_EFUSEPS_PUF_MISC_OFFSET) &
			XSK_ZYNQMP_EFUSEPS_PUF_MISC_AUX_MASK;
		Status = (u32)XST_SUCCESS;
	}

	*AuxPtr = Data;

	return Status;

}

/*****************************************************************************/
/**
* This function will poll for syndrome word is ready in the PUF_WORD register
* or till the timeout occurs.
*
* @param	None.
*
* @return	XST_SUCCESS - Incase of Success
*			XST_FAILURE - Incase of Timeout.
*
* @note		None.
*
******************************************************************************/
static u32  XilSKey_WaitForPufStatus(u32 *PufStatus)
{
	u32 Timeout = XILSKEY_PUF_STATUS_SYN_WRD_RDY_TIMEOUT/100U;
	u32 TimeoutFlag = (u32)XST_FAILURE;

	while(Timeout != 0U) {
		*PufStatus = XilSKey_ReadReg(XSK_ZYNQMP_CSU_BASEADDR,
											XSK_ZYNQMP_CSU_PUF_STATUS);
		if ((*PufStatus & XSK_ZYNQMP_CSU_PUF_STATUS_SYN_WRD_RDY_MASK) ==
							XSK_ZYNQMP_CSU_PUF_STATUS_SYN_WRD_RDY_MASK) {
			TimeoutFlag = (u32)XST_SUCCESS;
			goto done;
		}
		usleep(100U);
		Timeout--;
	}

done:
	return TimeoutFlag;
}


/*****************************************************************************/
/**
 * PUF Registration/Re-registration
 *
 * @param	InstancePtr	Pointer to the XilSKey_Puf instance.
 *
 * @return
		- XST_SUCCESS if registration/re-registration was successful.
 *		- ERROR if registration was unsuccessful
 *
 * @note	Updates the syndrome data @ InstancePtr->SyndromeData
 *
 *****************************************************************************/
u32 XilSKey_Puf_Registration(XilSKey_Puf *InstancePtr)
{
	u32 Status;
	s8 Timeout;
	u32 PufStatus;
	u32 Index = 0U;
	u32 Debug = XSK_PUF_DEBUG_GENERAL;
	u32 MaxSyndromeSizeInWords = XSK_ZYNQMP_PUF_SYN_DATA_LEN_IN_BYTES;
	XilsKey_PufRegistrationState RegistrationStatus;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	xPuf_printf(Debug,"API: PUF Registration\r\n");
	/* Update the shutter value, as forced sysoc selection */
	InstancePtr->ShutterValue = XSK_ZYNQMP_PUF_SHUTTER_VALUE;

	Status = XilSkey_Puf_Validate_Access_Rules(XSK_ZYNQMP_PUF_REGISTRATION);
	if(Status != (u32)XST_SUCCESS) {
		xPuf_printf(Debug,"API: Registration Failed:0x%08x\r\n",
								Status);
		goto ENDF;
	}

	/* Update the PUF configuration registers */
	XilSKey_WriteReg(XSK_ZYNQMP_CSU_BASEADDR,
		XSK_ZYNQMP_CSU_PUF_CFG0, XSK_ZYNQMP_PUF_CFG0_INIT_VAL);

	if (InstancePtr->RegistrationMode == XSK_ZYNQMP_PUF_MODE4K) {
		XilSKey_WriteReg(XSK_ZYNQMP_CSU_BASEADDR,
		XSK_ZYNQMP_CSU_PUF_CFG1, XSK_ZYNQMP_PUF_CFG1_INIT_VAL_4K);
		MaxSyndromeSizeInWords = XSK_ZYNQMP_MAX_RAW_4K_PUF_SYN_LEN;
	}
	else {
		Status = (u32)XSK_EFUSEPS_ERROR_PUF_INVALID_REG_MODE;
		xPuf_printf(Debug,"API:Invalid Registration Mode:0x%08x\r\n",
							Status);
		goto ENDF;
	}

	/* Configure the PUF shutter Value */
	XilSKey_WriteReg(XSK_ZYNQMP_CSU_BASEADDR, XSK_ZYNQMP_CSU_PUF_SHUT,
				InstancePtr->ShutterValue);

	/**
	 * Request PUF to register.
	 * This will trigger an interrupt to CSUROM
	 */
	XilSKey_WriteReg(XSK_ZYNQMP_CSU_BASEADDR, XSK_ZYNQMP_CSU_PUF_CMD,
				XSK_ZYNQMP_PUF_REGISTRATION);

	RegistrationStatus = XSK_EFUSEPS_PUF_REGISTRATION_STARTED;
	do {

		Timeout = XilSKey_WaitForPufStatus(&PufStatus);
		if (Timeout != 0U) {
			Status = (u32)XSK_EFUSEPS_ERROR_PUF_TIMEOUT;
			break;
		}

		if (PufStatus & XSK_ZYNQMP_CSU_PUF_STATUS_OVERFLOW_MASK) {
			xPuf_printf(Debug, "API: Overflow warning\r\n");
			Status = (u32)XSK_EFUSEPS_ERROR_PUF_DATA_OVERFLOW;
			break;
		}

		InstancePtr->SyndromeData[Index] =
		    XilSKey_ReadReg(XSK_ZYNQMP_CSU_BASEADDR,XSK_ZYNQMP_CSU_PUF_WORD);

		if ((PufStatus & XSK_ZYNQMP_CSU_PUF_STATUS_KEY_RDY_MASK) ==
		    XSK_ZYNQMP_CSU_PUF_STATUS_KEY_RDY_MASK) {
			if (Index < MaxSyndromeSizeInWords) {
				xPuf_printf(Debug,
				    "API: Underflow warning (Syndrome Data Length = %d)\r\n",
				    Index);
				Status = (u32)XSK_EFUSEPS_ERROR_PUF_DATA_UNDERFLOW;
				break;
			}
			RegistrationStatus = XSK_EFUSEPS_PUF_REGISTRATION_COMPLETE;

			/* Capture CHASH & AUX */
			InstancePtr->Chash = InstancePtr->SyndromeData[Index - 1U];
			InstancePtr->Aux = ((PufStatus &
			        XSK_ZYNQMP_CSU_PUF_STATUS_AUX_MASK) >> 4U);

			/* Also move the CHASH & AUX into array */
			InstancePtr->SyndromeData[XSK_ZYNQMP_PUF_SYN_LEN - 2U] =
			        InstancePtr->Chash;
			InstancePtr->SyndromeData[XSK_ZYNQMP_PUF_SYN_LEN - 1U] =
			        ((PufStatus & XSK_ZYNQMP_CSU_PUF_STATUS_AUX_MASK) << 4U);

			Status = (u32)XST_SUCCESS;
			xPuf_printf(Debug,"API: PUF Helper Data Generated!!!\r\n");
			break;
		}

		Index++;
		if (Index > MaxSyndromeSizeInWords)
		{
			xPuf_printf(Debug, "API: Overflow warning\r\n");
			Status = (u32)XSK_EFUSEPS_ERROR_PUF_DATA_OVERFLOW;
			break;
		}

	} while (RegistrationStatus != XSK_EFUSEPS_PUF_REGISTRATION_COMPLETE);

ENDF:
	return Status;
}

/*****************************************************************************/
/**
 * PUF Re-generation
 *
 * @param       InstancePtr is a pointer to the XilSKey_Puf instance.
 *
 * @return
 *              - XST_SUCCESS if regeneration was successful.
 *              - ERROR if regeneration was unsuccessful
 *
 ******************************************************************************/
u32 XilSKey_Puf_Regeneration(XilSKey_Puf *InstancePtr)
{
	u32 PufStatus;
	u32 Status;
	u32 PufChash = 0U;
	u32 Debug = XSK_PUF_DEBUG_GENERAL;

        /* Assert validates the input arguments */
        Xil_AssertNonvoid(InstancePtr != NULL);

	Status = XilSKey_ZynqMp_EfusePs_ReadPufChash(&PufChash, 0);
        if (Status != (u32)XST_SUCCESS) {
                goto END;
        }
	if (PufChash == 0U) {
		Status = (u32)XSK_EFUSEPS_ERROR_PUF_INVALID_REQUEST;
		xPuf_printf(Debug,"PUF regeneration is not allowed"
			", as PUF data is not stored in eFuse\r\n");
		goto END;
	}
	xPuf_printf(Debug,"API: PUF Regeneration\r\n");

	/* Update the PUF configuration registers */
	XilSKey_WriteReg(XSK_ZYNQMP_CSU_BASEADDR,
		XSK_ZYNQMP_CSU_PUF_CFG0, XSK_ZYNQMP_PUF_CFG0_INIT_VAL);

	/* Configure the PUF shutter Value */
	XilSKey_WriteReg(XSK_ZYNQMP_CSU_BASEADDR, XSK_ZYNQMP_CSU_PUF_SHUT,
				XSK_ZYNQMP_PUF_SHUTTER_VALUE);
	/* PUF key to device key */
	XilSKey_WriteReg(XSK_ZYNQMP_CSU_BASEADDR, XSK_ZYNQMP_CSU_PUF_CMD,
		XSK_ZYNQMP_PUF_REGENERATION);

	/* Wait till the data word ready */
	usleep(3000);

	PufStatus = XilSKey_ReadReg(XSK_ZYNQMP_CSU_BASEADDR,
					XSK_ZYNQMP_CSU_ISR);
	xPuf_printf(Debug,"PufStatus : 0x%x \r\n", PufStatus);
END:
	return Status;
}
/*****************************************************************************/
/**
 * PUF Debug 2 operation
 *
 * @param	InstancePtr	Pointer to the XilSKey_Puf instance.
 *
 *
 * @return
 *		- XST_SUCCESS if debug 2 mode was successful.
 *		- ERROR if registration was unsuccessful.
 *
 * @note	Updates the Debug 2 mode result @ InstancePtr->Debug2Data
 *
 ******************************************************************************/
u32 XilSKey_Puf_Debug2(XilSKey_Puf *InstancePtr)
{
	u32 PufStatus;
	u32 Index;
	u32 Debug = XSK_PUF_DEBUG_GENERAL;

	xPuf_printf(Debug,"API: PUF Debug 2\r\n");

	/**
	 * Request PUF for Debug 2.
	 * This will trigger an interrupt to CSUROM
	 */
	XilSKey_WriteReg(XSK_ZYNQMP_CSU_BASEADDR,
				XSK_ZYNQMP_CSU_PUF_CMD, 5);

	/**
	 * Wait till the PUF word ready &
	 * capture the test mode 2 result
	 * Repeat procedure for 160 times.
	 *
	 * ERROR:if timeout happens before word ready
	 * Timeout value??? - TBD
	 */
	PufStatus = XilSKey_ReadReg(XSK_ZYNQMP_CSU_BASEADDR,
					XSK_ZYNQMP_CSU_PUF_STATUS);
	for(Index = 0U; Index < 36U; Index++) {
		do {
			PufStatus = XilSKey_ReadReg(XSK_ZYNQMP_CSU_BASEADDR,
						XSK_ZYNQMP_CSU_PUF_STATUS);
		}while ((PufStatus &
			XSK_ZYNQMP_CSU_PUF_STATUS_SYN_WRD_RDY_MASK) !=
			XSK_ZYNQMP_CSU_PUF_STATUS_SYN_WRD_RDY_MASK);

		InstancePtr->Debug2Data[Index] =
			XilSKey_ReadReg(XSK_ZYNQMP_CSU_BASEADDR,
				XSK_ZYNQMP_CSU_PUF_WORD);
	}
	xPuf_printf(Debug,"API: PUF Debug 2 completed\r\n");

	return (u32)XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function programs the eFUSE PUF secure bits
*
* @param	WriteSecureBits		Pointer to the XilSKey_Puf_Secure
*		structure
*
* @return
*		XST_SUCCESS - On success
*		ErrorCode - on Failure
*
*
******************************************************************************/
u32 XilSKey_Write_Puf_EfusePs_SecureBits(XilSKey_Puf_Secure *WriteSecureBits)
{
	u32 Status;
	XskEfusePs_Type EfuseType = XSK_ZYNQMP_EFUSEPS_EFUSE_0;
	u8 Row = XSK_ZYNQMP_EFUSEPS_PUF_AUX_ROW;
	u32 RowDataVal;
	u8 DataInBits[XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW] = {0U};
	u32 Debug = XSK_PUF_DEBUG_GENERAL;

	/* If user requests any of the secure bit to be programmed */
	if ((WriteSecureBits->SynInvalid != 0x00U) ||
		(WriteSecureBits->SynWrLk != 0x00U) ||
		(WriteSecureBits->RegisterDis != 0x00U) ||
		(WriteSecureBits->Reserved != 0x00U)) {
		/* Unlock the controller */
		XilSKey_ZynqMp_EfusePs_CtrlrUnLock();

		/* Check the unlock status */
		if (XilSKey_ZynqMp_EfusePs_CtrlrLockStatus() != 0U) {
			Status = (u32)(XSK_EFUSEPS_ERROR_CONTROLLER_LOCK);
			goto END;
		}

		Status = XilSKey_ZynqMp_EfusePs_SetWriteConditions();
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		Status = XilSKey_ZynqMp_EfusePs_ReadRow(Row, EfuseType,
							&RowDataVal);
		if (Status != (u32)XST_SUCCESS) {
			xPuf_printf(Debug,
				"API: Failed at reading secure bits\r\n");
			goto END;
		}
		XilSKey_Efuse_ConvertBitsToBytes((u8 *)&RowDataVal, DataInBits,
			XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW);
	}
	else {
		Status = (u32)XST_SUCCESS;
		goto END;
	}

	if ((WriteSecureBits->SynInvalid != 0x00U) &&
		(DataInBits[XSK_ZYNQMP_EFUSEPS_PUF_SYN_INVALID] == 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
				XSK_ZYNQMP_EFUSEPS_PUF_SYN_INVALID, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			xPuf_printf(Debug,"API: Failed programming Syndrome"
						" invalid bit\r\n");
			Status = (Status +
				(u32)XSK_EFUSEPS_ERROR_WRITE_PUF_SYN_INVLD);
			goto END;
		}
	}
	if ((WriteSecureBits->SynWrLk != 0x00U) &&
		(DataInBits[XSK_ZYNQMP_EFUSEPS_PUF_SYN_LOCK] == 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
				XSK_ZYNQMP_EFUSEPS_PUF_SYN_LOCK, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			xPuf_printf(Debug,"API: Failed programming Syndrome"
							" write lock bit\r\n");
			Status = (Status +
				(u32)XSK_EFUSEPS_ERROR_WRITE_PUF_SYN_WRLK);
			goto END;
		}
	}
	if ((WriteSecureBits->RegisterDis != 0x00U) &&
		(DataInBits[XSK_ZYNQMP_EFUSEPS_PUF_REG_DIS] == 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
				XSK_ZYNQMP_EFUSEPS_PUF_REG_DIS, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			xPuf_printf(Debug,"API: Failed programming register"
							" disable bit\r\n");
			Status = (Status +
				(u32)XSK_EFUSEPS_ERROR_WRITE_PUF_SYN_REG_DIS);
			goto END;
		}
	}

	if ((WriteSecureBits->Reserved != 0x00U) &&
		(DataInBits[XSK_ZYNQMP_EFUSEPS_PUF_RESERVED] == 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
				XSK_ZYNQMP_EFUSEPS_PUF_RESERVED, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			xPuf_printf(Debug,"API: Failed programming reserved"
							" bit\r\n");
			Status = (Status +
					(u32)XSK_EFUSEPS_ERROR_WRITE_PUF_RESERVED_BIT);
			goto END;
		}
	}

END:
	/* Lock the controller back */
	XilSKey_ZynqMp_EfusePs_CtrlrLock();
	XilSKey_ZynqMp_EfusePS_PrgrmDisable();

	return Status;

}

/*****************************************************************************/
/**
* This function is used to read the PS efuse PUF secure bits from cache
* or from eFUSE array based on user selection.
*
* @param	SecureBits	Pointer to the XilSKey_Puf_Secure
*		which holds the read eFUSE secure bits of PUF.
* @param	ReadOption	A u8 variable which has to be provided by user
*		based on this input reading is happened from cache or from
*		efuse array.
*		- 0(XSK_EFUSEPS_READ_FROM_CACHE) Reads from cache
*		- 1(XSK_EFUSEPS_READ_FROM_EFUSE) Reads from efuse array
*
* @return
*		XST_SUCCESS - On success
*		ErrorCode - on Failure
*
******************************************************************************/
u32 XilSKey_Read_Puf_EfusePs_SecureBits(
		XilSKey_Puf_Secure *SecureBitsRead, u8 ReadOption)
{

	u32 Status;

	if (ReadOption == XSK_EFUSEPS_READ_FROM_CACHE) {
		Status = XilSKey_Read_Puf_EfusePs_SecureBits_Regs(
				SecureBitsRead, ReadOption);
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
		Status = XilSKey_Read_Puf_EfusePs_SecureBits_Regs(
					SecureBitsRead,ReadOption);
		XilSKey_ZynqMp_EfusePs_CtrlrUnLock();
	}
END:
	return Status;
}

/*****************************************************************************/
/**
* This function is used to read the PS efuse PUF secure bits from cache
* or from eFUSE array based on user selection.
*
* @param	SecureBits is the pointer to the XilSKey_Puf_Secure
*		which holds the read eFUSE secure bits of PUF.
* @param	ReadOption is a u8 variable which has to be provided by user
*		based on this input reading is happened from cache or from
*		efuse array.
*		- 0(XSK_EFUSEPS_READ_FROM_CACHE) Reads from cache
*		- 1(XSK_EFUSEPS_READ_FROM_EFUSE) Reads from efuse array
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline u32 XilSKey_Read_Puf_EfusePs_SecureBits_Regs(
		XilSKey_Puf_Secure *SecureBits, u8 ReadOption)
{

	u32 RegData = 0U;
	u32 Status;

	if (ReadOption == XSK_EFUSEPS_READ_FROM_CACHE) {
		RegData = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_PUF_MISC_OFFSET);
		Status = (u32)XST_SUCCESS;
	}
	else {
		Status = XilSKey_ZynqMp_EfusePs_ReadRow(
				XSK_ZYNQMP_EFUSEPS_PUF_AUX_ROW,
					XSK_ZYNQMP_EFUSEPS_EFUSE_0, &RegData);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}

	SecureBits->SynInvalid =
		(u8)((RegData & XSK_ZYNQMP_EFUSEPS_PUF_MISC_SYN_INVLD_MASK) >>
				XSK_ZYNQMP_EFUSEPS_PUF_MISC_SYN_INVLD_SHIFT);
	SecureBits->SynWrLk =
		(u8)((RegData & XSK_ZYNQMP_EFUSEPS_PUF_MISC_SYN_WRLK_MASK) >>
				XSK_ZYNQMP_EFUSEPS_PUF_MISC_SYN_WRLK_SHIFT);
	SecureBits->RegisterDis =
		(u8)((RegData & XSK_ZYNQMP_EFUSEPS_PUF_MISC_REG_DIS_MASK) >>
			XSK_ZYNQMP_EFUSEPS_PUF_MISC_REG_DIS_SHIFT);
	SecureBits->Reserved =
			(u8)((RegData & XSK_ZYNQMP_EFUSEPS_PUF_MISC_RESERVED_MASK) >>
				XSK_ZYNQMP_EFUSEPS_PUF_MISC_RESERVED_SHIFT);
END:

	return Status;

}

/***************************************************************************/
/**
* This API programs the given data into specified row of efuse.
*
* @param	Row specifies the row number to be programmed.
* @param	Data is pointer to 32 bit variable which holds data to be
*		programmed.
*
* @return
*		- XST_SUCCESS if programs successfully.
*		- Errorcode on failure
*
* @note		None.
*
******************************************************************************/
static inline u32 XilSKey_ZynqMp_EfusePs_PufRowWrite(u8 Row,
				u8 *Data, XskEfusePs_Type EfuseType)
{

	u8 Column;
	u32 Status = (u32)XST_FAILURE;

	for (Column = 0U; Column < XSK_ZYNQMP_EFUSEPS_MAX_BITS_IN_ROW;
						Column++) {
		if (Data[Column] != 0x00U) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
							Column, EfuseType);
			if (Status != (u32)XST_SUCCESS) {
				goto END;
			}
		}
	}
	Status = (u32)XST_SUCCESS;
END:
	return Status;

}

/***************************************************************************/
/**
* This function checks whether PUF is already programmed or not.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if all rows are zero
*		- Errorcode if already programmed.
*
* @note		None.
*
******************************************************************************/
static inline u32 XilSKey_ZynqMp_EfusePs_CheckZeros_Puf(void)
{
	u32 RowDataVal = 0U;
	u32 Status;

	/*
	 * By the time of checking PUF syndrome data T bits
	 * might be programmed so complete 0th row cannot
	 * be checked as zeroth row contains Tbits
	 */
	Status = XilSKey_ZynqMp_EfusePs_ReadRow(
			XSK_ZYNQMP_EFUSEPS_PUF_ROW_START,
			XSK_ZYNQMP_EFUSEPS_EFUSE_2, &RowDataVal);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	if ((RowDataVal & (~(XSK_ZYNQMP_EFUSEPS_TBITS_MASK <<
			XSK_ZYNQMP_EFUSEPS_TBITS_SHIFT))) != 0x00U) {
		Status = (u32)XSK_EFUSEPS_ERROR_PUF_DATA_ALREADY_PROGRAMMED;
		goto END;
	}

	Status = XilSKey_ZynqMp_EfusePs_ReadRow(
			XSK_ZYNQMP_EFUSEPS_PUF_ROW_START,
			XSK_ZYNQMP_EFUSEPS_EFUSE_3, &RowDataVal);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	if ((RowDataVal & (~(XSK_ZYNQMP_EFUSEPS_TBITS_MASK <<
			XSK_ZYNQMP_EFUSEPS_TBITS_SHIFT))) != 0x00U) {
		Status = (u32)XSK_EFUSEPS_ERROR_PUF_DATA_ALREADY_PROGRAMMED;
		goto END;
	}

	if (XilSKey_ZynqMp_EfusePs_CheckForZeros(
		(XSK_ZYNQMP_EFUSEPS_PUF_ROW_START + 1U),
		XSK_ZYNQMP_EFUSEPS_PUF_ROW_END, XSK_ZYNQMP_EFUSEPS_EFUSE_2) !=
								(u32)XST_SUCCESS) {
		Status = (u32)XSK_EFUSEPS_ERROR_PUF_DATA_ALREADY_PROGRAMMED;
		goto END;
	}

	if (XilSKey_ZynqMp_EfusePs_CheckForZeros(
		(XSK_ZYNQMP_EFUSEPS_PUF_ROW_START + 1U),
		XSK_ZYNQMP_EFUSEPS_PUF_ROW_END,
		XSK_ZYNQMP_EFUSEPS_EFUSE_3) != (u32)XST_SUCCESS) {
		Status = (u32)XSK_EFUSEPS_ERROR_PUF_DATA_ALREADY_PROGRAMMED;
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * Validates the PUF access rules
 *
 * @param	RequestType is an input to validate the access rules against
 *		Registration, Re-registration, Testmode1 & Testmode2
 *
 * @return	XST_SUCCESS if validation was successful.
 *		ERROR if validation failed
 *
 * @note	None
 *
 ******************************************************************************/
static inline u32 XilSkey_Puf_Validate_Access_Rules(u8 RequestType)
{
	u32 PufChash = 0U;
	u32 PufAux = 0U;
	u32 Status;
	u32 Debug = XSK_PUF_DEBUG_GENERAL;
	XilSKey_SecCtrlBits ReadSecCtrlBits;
	XilSKey_Puf_Secure PufSecureBits;

	/* Read secure control register for RSA bits value from eFUSE */
	Status = XilSKey_ZynqMp_EfusePs_ReadSecCtrlBits(&ReadSecCtrlBits, 0);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/* Reading PUF secure bits */
	Status = XilSKey_Read_Puf_EfusePs_SecureBits(&PufSecureBits, 0);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	Status = XilSKey_ZynqMp_EfusePs_ReadPufAux(&PufAux, 0);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	Status = XilSKey_ZynqMp_EfusePs_ReadPufChash(&PufChash, 0);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	if (RequestType == XSK_ZYNQMP_PUF_REGISTRATION) {
		/**
		 * To allow registration
		 * 1. Make sure that PUF registration is not disabled
		 * 2. If it re-registration then
		 * 	2.1  Make sure that use RSA bits set in eFUSE.
		 */
		if (PufSecureBits.RegisterDis != 0U ) {
			Status = (u32)XSK_EFUSEPS_ERROR_PUF_REG_DISABLED;
			xPuf_printf(Debug,
				"API: PUF Registration not allowed "
			"(Disabled in eFUSE):0x%08x\r\n",Status);
		}
		else if ((PufChash != 0U) || (PufAux != 0U)) {
			if (ReadSecCtrlBits.RSAEnable == 0U) {
				Status =
				(u32)XSK_EFUSEPS_ERROR_PUF_REG_WO_AUTH;
				xPuf_printf(Debug,
				"API:Registration not allowed w/o "
				"Authentication:0x%08x\r\n", Status);
			}
		}
		else {
			Status = (u32)XST_SUCCESS;
		}
	}
	else {
		Status = (u32)XSK_EFUSEPS_ERROR_PUF_INVALID_REQUEST;
		xPuf_printf(Debug,
		"API: Invalid Request type for validation:0x%08x\r\n",
			Status);
	}

END:
	return Status;

}
