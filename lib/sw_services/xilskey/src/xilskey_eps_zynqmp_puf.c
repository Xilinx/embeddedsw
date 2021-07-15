/******************************************************************************
* Copyright (c) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xilskey_eps_zynqmp_puf.c
* This file contains the APIs for registering PUF, eFUSE programming and reading
* the PUF helper data, CHASH and Auxiliary data.
*
* </pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- --------------------------------------------------------
* 6.1   vns  10/17/16 First release.
*       vns  11/07/16 Fixed shutter value to 0x0100005e, as sysosc selection
*                     is fixed for PUF registration.
* 6.2   vns  02/18/17 Added masking for PUF auxiliary read.
* 6.6   vns  06/06/18 Added doxygen tags
* 6.7	arc  01/05/19 Fixed MISRA-C violations.
*       arc  03/15/19 Modified initial default status value as XST_FAILURE
*       mmd  03/17/19 Handled buffer underflow issue and added timeouts during
*                     syndrome data reading
*       rama 03/25/19 Added polling routine for PUF ready state
* 6.8   psl  06/07/19 Added doxygen tags.
*       psl  06/25/19 Fixed Coverity warnings.
*       psl  06/28/19 Added doxygen tags.
*       psl  07/05/19 Added Asserts for validation.
*       psl  07/23/19 Fixed input validation.
*       psl  07/29/19 Fixed MISRA-C violation
*       vns  08/29/19 Initialized Status variables
* 6.9   kpt  02/16/20 Fixed Coverity warnings
*       kpt  02/27/20 Removed XilSKey_Puf_Debug2
*                      which is used only for debug purpose
*       vns  03/18/20 Fixed Armcc compilation errors
*       kal  03/18/20 Added Temp and Voltage checks before writing Puf Helper
*                     data, Puf Chash and Puf Aux.
*       kpt  03/17/20 Replaced direct eFuse reads with cache reads
*                     and Error code is returned when user chooses
*                     read option as eFuse
*       kal 04/09/20  Added Temp and Voltage checks before unlocking the
*                     controller.
*       kal 05/14/20  Added Cache Reload in XilSKey_Write_Puf_EfusePs_SecureBits
*                     to reflect programmed bit when read from puf example.
* 7.0	am  10/04/20  Resolved MISRA C violations
* 7.2   am  07/13/21  Fixed doxygen warnings
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files *********************************/

#include "xilskey_eps_zynqmp_puf.h"
#include "xilskey_eps_zynqmp_hw.h"
#include "xilskey_eps_zynqmp.h"
#include "sleep.h"
/************************** Constant Definitions *****************************/
	/**
	* Status of PUF word syndrome ready timeout
	*/
#define XILSKEY_PUF_STATUS_SYN_WRD_RDY_TIMEOUT	(500000U)

/**************************** Type Definitions ******************************/
typedef enum {
	XSK_EFUSEPS_PUF_REGISTRATION_STARTED, /**<0x0 - PUF registration
						* started */
	XSK_EFUSEPS_PUF_REGISTRATION_COMPLETE /**<0x1 - PUF registration
						* complete */
} XilsKey_PufRegistrationState;


/***************** Macros (Inline Functions) Definitions ********************/


/************************** Variable Definitions ****************************/


/************************** Function Prototypes *****************************/

static INLINE u32 XilSkey_Puf_Validate_Access_Rules(u8 RequestType);
static INLINE u32 XilSKey_ZynqMp_EfusePs_CheckZeros_Puf(void);

static INLINE u32 XilSKey_ZynqMp_EfusePs_PufRowWrite(u8 Row, const u8 *Data,
						XskEfusePs_Type EfuseType);
static INLINE void XilSKey_Read_Puf_EfusePs_SecureBits_Regs(
		XilSKey_Puf_Secure *SecureBits);
static u32  XilSKey_WaitForPufStatus(u32 *PufStatus);
/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* This function programs the PS eFUSEs with the PUF helper data.
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
u32 XilSKey_ZynqMp_EfusePs_WritePufHelprData(const XilSKey_Puf *InstancePtr)
{
	u32 Status = (u32)XST_FAILURE;
	u8 Row;
	u32 Data;
	const u32 *DataPtr;
	const u32 *TempPtr;
	XskEfusePs_Type EfuseType;
	u8 DataInBits[32] = {0};

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Initialize the ADC */
	Status = XilSKey_ZynqMp_EfusePs_Init();
	if (Status != (u32)XST_SUCCESS) {
                goto END;
        }
	/* Vol and temperature checks */
	Status = XilSKey_ZynqMp_EfusePs_Temp_Vol_Checks();
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	DataPtr = InstancePtr->EfuseSynData;
	TempPtr = InstancePtr->EfuseSynData;

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
* This function reads the PUF helper data from eFUSE.
*
* @param	Address		Pointer to data array which holds the PUF helper
*		data read from eFUSEs.
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
	u32 Status = (u32)XST_FAILURE;
	u32 Row;
	u32 RowDataVal[128] = {0U};
	u32 *PtrEfuse2 = &RowDataVal[0];
	u32 *PtrEfuse3 = &RowDataVal[64];
	u32 *AddrPtr;
	u32 Temp;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(Address != NULL);

	AddrPtr = (u32 *)Address;
	/* Unlock the controller */
	XilSKey_ZynqMp_EfusePs_CtrlrUnLock();
	/* Check the unlock status */
	if (XilSKey_ZynqMp_EfusePs_CtrlrLockStatus() != 0U) {
		Status = (u32)(XSK_EFUSEPS_ERROR_CONTROLLER_LOCK);
		goto END;
	}

	/* Setting the timing Constraints */
        XilSKey_ZynqMp_EfusePs_SetTimerValues();

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
		Temp = ((RowDataVal[Row + 1U] &
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
			Temp = Temp | ((RowDataVal[Row + 1U] &
				XSK_ZYNQMP_EFUSEPS_PUF_ROW_LOWER_MASK));
		}
		else {
			Temp = RowDataVal[Row + 1U];
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
* This function programs eFUSE with CHash value.
*
* @param	InstancePtr	Pointer to the XilSKey_Puf instance.
*
* @return
*		- XST_SUCCESS if chash is programmed successfully.
*		- An Error code on failure
*
* @note		To generate the CHash value, please use
*		XilSKey_Puf_Registration function.
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_WritePufChash(const XilSKey_Puf *InstancePtr)
{
	u32 Status = (u32)XST_FAILURE;
	u8 Value[32] = {0U};
	u8 Column;
	XskEfusePs_Type EfuseType;
	u32 RowDataVal = 0U;
	const u8 *PufChash;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Initialize the ADC */
	Status = XilSKey_ZynqMp_EfusePs_Init();
	if (Status != (u32)XST_SUCCESS) {
                goto END;
        }
	/* Vol and temperature checks */
	Status = XilSKey_ZynqMp_EfusePs_Temp_Vol_Checks();
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	PufChash = (const u8 *)&(InstancePtr->Chash);

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
	Status = XilSKey_ZynqMp_EfusePs_ReadPufChash(&RowDataVal,
							XSK_EFUSEPS_READ_FROM_CACHE);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	if (RowDataVal != 0X00U) {
		Status = (u32)XSK_EFUSEPS_ERROR_PUF_CHASH_ALREADY_PROGRAMMED;
		goto END;
	}

	XilSKey_Efuse_ConvertBitsToBytes((const u8 *)PufChash, Value,
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
* This function reads eFUSE PUF CHash data from the eFUSE array or
* cache based on the user read option.
*
* @param	Address	Pointer which holds the read back value of the chash.
* @param	ReadOption	Indicates whether or not to read from the actual
* 		eFUSE array or from the eFUSE cache.
*		- 0(XSK_EFUSEPS_READ_FROM_CACHE) Reads from cache
*		- 1(XSK_EFUSEPS_READ_FROM_EFUSE) Reads from eFUSE array
*
* @return
*		- XST_SUCCESS if programs successfully.
*		- Errorcode on failure
*
* @note		Cache reload is required for obtaining updated values for
*		reading from cache..
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_ReadPufChash(u32 *Address, u8 ReadOption)
{
	u32 Status = (u32)XST_FAILURE;
	u32 *ChashPtr;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(Address != NULL);
	Xil_AssertNonvoid((ReadOption == XSK_EFUSEPS_READ_FROM_CACHE) ||
				(ReadOption == XSK_EFUSEPS_READ_FROM_EFUSE));

	ChashPtr = (u32 *)Address;

	if (ReadOption == XSK_EFUSEPS_READ_FROM_EFUSE) {
		Status = (u32)XSK_EFUSEPS_RD_FROM_EFUSE_NOT_ALLOWED;
	}
	else {
		*ChashPtr = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_PUF_CHASH_OFFSET);
		Status = (u32)XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
* This function programs eFUSE PUF auxiliary data.
*
* @param	InstancePtr	Pointer to the XilSKey_Puf instance.
*
* @return
*		- XST_SUCCESS if the eFUSE is programmed successfully.
*		- Errorcode on failure
*
* @note		To generate auxiliary data, please use
*		XilSKey_Puf_Registration function.
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_WritePufAux(const XilSKey_Puf *InstancePtr)
{
	u32 Status = (u32)XST_FAILURE;
	u8 Value[32] = {0U};
	u8 Column;
	XskEfusePs_Type EfuseType;
	u32 RowDataVal;
	const u8 *AuxValue;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Initialize the ADC */
	Status = XilSKey_ZynqMp_EfusePs_Init();
	if (Status != (u32)XST_SUCCESS) {
                goto END;
        }
	/* Vol and temperature checks */
	Status = XilSKey_ZynqMp_EfusePs_Temp_Vol_Checks();
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	AuxValue = (const u8 *)&(InstancePtr->Aux);
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
	Status = XilSKey_ZynqMp_EfusePs_ReadPufAux(&RowDataVal,
							XSK_EFUSEPS_READ_FROM_CACHE);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	if ((RowDataVal & XSK_ZYNQMP_EFUSEPS_PUF_MISC_AUX_MASK) != 0x00U) {
		Status = (u32)XSK_EFUSEPS_ERROR_PUF_AUX_ALREADY_PROGRAMMED;
		goto END;
	}

	XilSKey_Efuse_ConvertBitsToBytes((const u8 *)AuxValue, Value,
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
* This function reads eFUSE PUF auxiliary data from eFUSE array
* or cache based on user read option.
*
* @param	Address 	Pointer which holds the read back value of PUF's
* 		auxiliary data.
* @param	ReadOption	Indicates whether or not to read from the actual
* 		eFUSE array or from the eFUSE cache.
*		- 0(XSK_EFUSEPS_READ_FROM_CACHE) Reads from cache
*		- 1(XSK_EFUSEPS_READ_FROM_EFUSE) Reads from eFUSE array
*
* @return
* 		- XST_SUCCESS if PUF auxiliary data is read successfully.
* 		- Errorcode on failure
*
* @note		Cache reload is required for obtaining updated values for
*		reading from cache.
*
******************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_ReadPufAux(u32 *Address, u8 ReadOption)
{

	u32 Status = (u32)XST_FAILURE;
	u32 *AuxPtr;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(Address != NULL);
	Xil_AssertNonvoid((ReadOption == XSK_EFUSEPS_READ_FROM_CACHE) ||
				(ReadOption == XSK_EFUSEPS_READ_FROM_EFUSE));

	AuxPtr = (u32 *)Address;

	if (ReadOption == XSK_EFUSEPS_READ_FROM_EFUSE) {
		Status = (u32)XSK_EFUSEPS_RD_FROM_EFUSE_NOT_ALLOWED;
	}
	else {
		*AuxPtr = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
			XSK_ZYNQMP_EFUSEPS_PUF_MISC_OFFSET) &
			XSK_ZYNQMP_EFUSEPS_PUF_MISC_AUX_MASK;
		Status = (u32)XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
* This function will poll for syndrome word is ready in the PUF_WORD register
* or till the timeout occurs.
*
* @param	PufStatus Pointer to pufstatus.
*
* @return	XST_SUCCESS - In case of Success
*			XST_FAILURE - In case of Timeout.
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
 * This function performs registration of PUF which generates a new KEK
 * and associated CHash, Auxiliary and PUF-syndrome data which are unique for
 * each silicon.
 *
 * @param	InstancePtr	Pointer to the XilSKey_Puf instance.
 *
 * @return
		- XST_SUCCESS if registration/re-registration was successful.
 *		- ERROR if registration was unsuccessful
 *
 * @note	With the help of generated PUF syndrome data, it will be possible
 * to re-generate same PUF KEK.
 *
 *****************************************************************************/
u32 XilSKey_Puf_Registration(XilSKey_Puf *InstancePtr)
{
	u32 Status = (u32)XST_FAILURE;
	u32 PufStatus = 0U;
	u32 Index = 0U;
	u32 Debug = XSK_PUF_DEBUG_GENERAL;
	u32 MaxSyndromeSizeInWords;
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

		Status = XilSKey_WaitForPufStatus(&PufStatus);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XSK_EFUSEPS_ERROR_PUF_TIMEOUT;
			break;
		}

		if ((PufStatus & XSK_ZYNQMP_CSU_PUF_STATUS_OVERFLOW_MASK) ==
				XSK_ZYNQMP_CSU_PUF_STATUS_OVERFLOW_MASK) {
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
			InstancePtr->Chash = InstancePtr->SyndromeData[Index];
			InstancePtr->Aux = ((PufStatus &
			        XSK_ZYNQMP_CSU_PUF_STATUS_AUX_MASK) >> 4U);

			/* Also move the CHASH & AUX into array */
			InstancePtr->SyndromeData[XSK_ZYNQMP_PUF_SYN_LEN - 2U] =
			        InstancePtr->Chash;
			InstancePtr->SyndromeData[XSK_ZYNQMP_PUF_SYN_LEN - 1U] =
			        ((PufStatus & XSK_ZYNQMP_CSU_PUF_STATUS_AUX_MASK) << 4U);

			Status = (u32)XST_SUCCESS;
			xPuf_printf(Debug,"API: PUF Helper Data Generated!!!\r\n");
		}
		else {
			Index++;
			if (Index > MaxSyndromeSizeInWords)
			{
				xPuf_printf(Debug, "API: Overflow warning\r\n");
				Status = (u32)XSK_EFUSEPS_ERROR_PUF_DATA_OVERFLOW;
				break;
			}
		}

	} while (RegistrationStatus != XSK_EFUSEPS_PUF_REGISTRATION_COMPLETE);

ENDF:
	return Status;
}

/*****************************************************************************/
/**
 * This function regenerates the PUF data so that the PUF's output can be used
 * as the key source to the AES-GCM hardware cryptographic engine.
 *
 * @param       InstancePtr is a pointer to the XilSKey_Puf instance.
 *
 * @return
 *              - XST_SUCCESS if regeneration was successful.
 *              - ERROR if regeneration was unsuccessful
 *
 ******************************************************************************/
u32 XilSKey_Puf_Regeneration(const XilSKey_Puf *InstancePtr)
{
	u32 PufStatus;
	u32 Status = (u32)XST_FAILURE;
	u32 PufChash = 0U;
	u32 Debug = XSK_PUF_DEBUG_GENERAL;

        /* Assert validates the input arguments */
        Xil_AssertNonvoid(InstancePtr != NULL);

	Status = XilSKey_ZynqMp_EfusePs_ReadPufChash(&PufChash,
									XSK_EFUSEPS_READ_FROM_CACHE);
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
	if ((PufStatus & XSK_ZYNQMP_CSU_ISR_PUF_ACC_ERROR_MASK) != 0x0U) {
		xPuf_printf(Debug,"PufStatus : 0x%x \r\n", PufStatus);
		Status = (u32)XSK_EFUSEPS_ERROR_PUF_ACCESS;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
* This function programs the eFUSE PUF secure bits
*
* @param	WriteSecureBits		Pointer to the XilSKey_Puf_Secure
*		structure
*
* @return
*		- XST_SUCCESS if eFUSE PUF secure bits are programmed successfully.
*		- Errorcode on failure.
*
******************************************************************************/
u32 XilSKey_Write_Puf_EfusePs_SecureBits(
		const XilSKey_Puf_Secure *WriteSecureBits)
{
	u32 Status = (u32)XST_FAILURE;
	XskEfusePs_Type EfuseType = XSK_ZYNQMP_EFUSEPS_EFUSE_0;
	u8 Row = XSK_ZYNQMP_EFUSEPS_PUF_AUX_ROW;
	XilSKey_Puf_Secure SecureBits;
	u32 Debug = XSK_PUF_DEBUG_GENERAL;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(WriteSecureBits != NULL);

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

		XilSKey_Read_Puf_EfusePs_SecureBits_Regs(&SecureBits);
	}
	else {
		Status = (u32)XST_SUCCESS;
		goto END;
	}

	if ((WriteSecureBits->SynInvalid != 0x00U) &&
		(SecureBits.SynInvalid == 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
				(u8)XSK_ZYNQMP_EFUSEPS_PUF_SYN_INVALID, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			xPuf_printf(Debug,"API: Failed programming Syndrome"
						" invalid bit\r\n");
			Status = (Status |
				(u32)XSK_EFUSEPS_ERROR_WRITE_PUF_SYN_INVLD);
			goto END;
		}
	}
	if ((WriteSecureBits->SynWrLk != 0x00U) &&
		(SecureBits.SynWrLk == 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
				(u8)XSK_ZYNQMP_EFUSEPS_PUF_SYN_LOCK, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			xPuf_printf(Debug,"API: Failed programming Syndrome"
							" write lock bit\r\n");
			Status = (Status |
				(u32)XSK_EFUSEPS_ERROR_WRITE_PUF_SYN_WRLK);
			goto END;
		}
	}
	if ((WriteSecureBits->RegisterDis != 0x00U) &&
		(SecureBits.RegisterDis == 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
				(u8)XSK_ZYNQMP_EFUSEPS_PUF_REG_DIS, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			xPuf_printf(Debug,"API: Failed programming register"
							" disable bit\r\n");
			Status = (Status |
				(u32)XSK_EFUSEPS_ERROR_WRITE_PUF_SYN_REG_DIS);
			goto END;
		}
	}

	if ((WriteSecureBits->Reserved != 0x00U) &&
		(SecureBits.Reserved == 0x00U)) {
		Status = XilSKey_ZynqMp_EfusePs_WriteAndVerifyBit(Row,
				(u8)XSK_ZYNQMP_EFUSEPS_PUF_RESERVED, EfuseType);
		if (Status != (u32)XST_SUCCESS) {
			xPuf_printf(Debug,"API: Failed programming reserved"
							" bit\r\n");
			Status = (Status |
					(u32)XSK_EFUSEPS_ERROR_WRITE_PUF_RESERVED_BIT);
			goto END;
		}
	}

	Status = XilSKey_ZynqMp_EfusePs_CacheLoad();

END:
	/* Lock the controller back */
	XilSKey_ZynqMp_EfusePs_CtrlrLock();
	XilSKey_ZynqMp_EfusePS_PrgrmDisable();

	return Status;
}

/*****************************************************************************/
/**
* This function is used to read the PS eFUSE PUF secure bits from cache
* or from eFUSE array.
*
* @param	SecureBitsRead - Pointer to the XilSKey_Puf_Secure structure
*                                which holds the read eFUSE secure bits from the
*                                PUF.
* @param	ReadOption     - Indicates whether or not to read from the
*                                actual eFUSE array or from the eFUSE cache.
*		- 0(XSK_EFUSEPS_READ_FROM_CACHE) Reads from cache
*		- 1(XSK_EFUSEPS_READ_FROM_EFUSE) Reads from eFUSE array
*
* @return
*		- XST_SUCCESS if reads successfully.
*		- Errorcode on failure.
*
******************************************************************************/
u32 XilSKey_Read_Puf_EfusePs_SecureBits(
		XilSKey_Puf_Secure *SecureBitsRead, u8 ReadOption)
{

	u32 Status = (u32)XST_FAILURE;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(SecureBitsRead != NULL);
	Xil_AssertNonvoid((ReadOption == XSK_EFUSEPS_READ_FROM_CACHE) ||
				(ReadOption == XSK_EFUSEPS_READ_FROM_EFUSE));

	if (ReadOption == XSK_EFUSEPS_READ_FROM_CACHE) {
		XilSKey_Read_Puf_EfusePs_SecureBits_Regs(
				SecureBitsRead);
		Status = XST_SUCCESS;
	}
	else {
		Status = (u32)XSK_EFUSEPS_RD_FROM_EFUSE_NOT_ALLOWED;
	}

	return Status;
}

/*****************************************************************************/
/**
* This function is used to read the PS eFUSE PUF secure bits from cache
* or from eFUSE array based on user selection.
*
* @param	SecureBits is the pointer to the XilSKey_Puf_Secure
*		which holds the read eFUSE secure bits of PUF.
*
* @note		None.
*
******************************************************************************/
static INLINE void XilSKey_Read_Puf_EfusePs_SecureBits_Regs(
		XilSKey_Puf_Secure *SecureBits)
{

	u32 RegData = 0U;

	RegData = XilSKey_ReadReg(XSK_ZYNQMP_EFUSEPS_BASEADDR,
				XSK_ZYNQMP_EFUSEPS_PUF_MISC_OFFSET);

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

}

/***************************************************************************/
/**
* This API programs the given data into specified row of eFUSE.
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
static INLINE u32 XilSKey_ZynqMp_EfusePs_PufRowWrite(u8 Row,
				const u8 *Data, XskEfusePs_Type EfuseType)
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
* @return
*		- XST_SUCCESS if all rows are zero
*		- Errorcode if already programmed.
*
* @note		None.
*
******************************************************************************/
static INLINE u32 XilSKey_ZynqMp_EfusePs_CheckZeros_Puf(void)
{
	u32 Status = (u32)XST_FAILURE;
	u32 RowDataVal = 0U;

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
	if ((RowDataVal & (~((u32)XSK_ZYNQMP_EFUSEPS_TBITS_MASK <<
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
	if ((RowDataVal & (~((u32)XSK_ZYNQMP_EFUSEPS_TBITS_MASK <<
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
static INLINE u32 XilSkey_Puf_Validate_Access_Rules(u8 RequestType)
{
	u32 PufChash = 0U;
	u32 PufAux = 0U;
	u32 Status = (u32)XST_FAILURE;
	u32 Debug = XSK_PUF_DEBUG_GENERAL;
	XilSKey_SecCtrlBits ReadSecCtrlBits = {0U};
	XilSKey_Puf_Secure PufSecureBits = {0U};

	/* Read secure control register for RSA bits value from eFUSE */
	Status = XilSKey_ZynqMp_EfusePs_ReadSecCtrlBits(&ReadSecCtrlBits,
									XSK_EFUSEPS_READ_FROM_CACHE);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/* Reading PUF secure bits */
	Status = XilSKey_Read_Puf_EfusePs_SecureBits(&PufSecureBits,
									XSK_EFUSEPS_READ_FROM_CACHE);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	Status = XilSKey_ZynqMp_EfusePs_ReadPufAux(&PufAux,
									XSK_EFUSEPS_READ_FROM_CACHE);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	Status = XilSKey_ZynqMp_EfusePs_ReadPufChash(&PufChash,
									XSK_EFUSEPS_READ_FROM_CACHE);
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
