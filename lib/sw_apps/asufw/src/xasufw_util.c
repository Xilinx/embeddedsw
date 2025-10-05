/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_util.c
 *
 * This file contains the code for util APIs in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   03/29/24 Initial release
 *       am   06/26/24 Added XAsufw_CryptoCoreReleaseReset(), XAsufw_CryptoCoreSetReset() and
 *                     XAsufw_RCMW() function.
 *  	 ss   07/11/24 Added XAsufw_ChangeEndiannessAndCpy function.
 *       ss   08/20/24 Updated description for XAsufw_ChangeEndianness() function.
 *       am   09/13/24 Fixed pointer conversion error for cpp compiler.
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 *       ss   10/05/24 Added XAsufw_IsBufferNonZero function.
 * 1.1   ma   02/03/25 Updated TempVar in XAsufw_ChangeEndianness with volatile and zeroize at
 *                     the end.
 *       vns  02/06/25 Removed XAsufw_ChangeEndiannessAndCpy() function which is not in use
 *       vns  02/12/25 Removed XAsufw_RCMW() API
 *       vns  02/21/25 Added XAsufw_NvmEfuseWriteOffChipRevokeId() API
 * 1.2   am   05/18/25 Added XAsufw_WriteDataToRegs() API
 *       rmv  07/09/25 Added XAsufw_AsciiToInt() function.
 *       rmv  09/10/25 Simplified XAsufw_SMemSet() error handling by combining status checks
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_util.h"
#include "xasufw_status.h"
#include "xasufw_ipi.h"
#include "xil_sutil.h"

/************************************ Constant Definitions ***************************************/
#define XASUFW_PLM_XILNVM_MODULE_ID			(11U) /**< XilNVM Module ID in PLM */
#define XASUFW_PLM_XILNVM_WRITE_OFFCHIP_REVOKE_API_ID	(16U) /**< XilNVM Write off chip
								*  revoke API ID */
#define XASUFW_PLM_XILNVM_MAX_PAYLOAD_LEN		(3U) /**< Maximum payload length */
#define XASUFW_PLM_XILNVM_RESP_LEN			(1U) /**< Response buffer length */
#define XASUFW_PLM_XILNVM_CMD_HDR_INDEX			(0U) /**< PLM command payload index of header */
#define XASUFW_PLM_XILNVM_ENVCTRL_INDEX			(1U) /**< PLM command payload index of environment
								* control */
#define XASUFW_PLM_XILNVM_OFFCHID_INDEX			(2U) /**< PLM command payload index of off chip ID */
#define XASUFW_PLM_XILNVM_ENV_MON_CHECK			(0U) /**< Perform environment checks */

#define XASUFW_ENDIANNESS_SWAP_OFFSET			(1U) /**< Position offset for swapping */
#define XASUFW_ENDIANNESS_SWAP_MIN_LENGTH		(2U) /**< Minimum length to make a swap */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function will Read, Modify and Write to a register.
 *
 * @param	Addr	Address of the register.
 * @param	Mask	Bits to be modified.
 * @param	Value	Value to be written to the register.
 *
 *************************************************************************************************/
void XAsufw_RMW(u32 Addr, u32 Mask, u32 Value)
{
	u32 Val;

	/** Read the value from the register. */
	Val = XAsufw_ReadReg(Addr);

	/** Reset designated bits in a register value to zero and replace them with the given value. */
	Val = (Val & (~Mask)) | (Mask & Value);

	/** Update the value to the register. */
	XAsufw_WriteReg(Addr, Val);
}

/*****************************************************************************/
/**
 * @brief	This function takes the hardware core out of reset.
 *
 * @param	BaseAddress	Base address of the core.
 * @param	Offset		Offset of the reset register.
 *
 *****************************************************************************/
void XAsufw_CryptoCoreReleaseReset(u32 BaseAddress, u32 Offset)
{
	XAsufw_WriteReg((BaseAddress + Offset), XASUFW_RESET_SET);
	XAsufw_WriteReg((BaseAddress + Offset), XASUFW_RESET_UNSET);
}

/*****************************************************************************/
/**
 * @brief	This function places the hardware core into the reset.
 *
 * @param	BaseAddress	Base address of the core.
 * @param	Offset		Offset of the reset register.
 *
 *****************************************************************************/
void XAsufw_CryptoCoreSetReset(u32 BaseAddress, u32 Offset)
{
	XAsufw_WriteReg((BaseAddress + Offset), XASUFW_RESET_SET);
}

/*************************************************************************************************/
/**
 * @brief	This function changes the endianness of data and stores it in the same buffer.
 *
 * @param	Buffer	Pointer to the buffer whose endianness needs to be changed.
 * @param	Length	Length of the buffer in bytes.
 *
 * @return
 *	-	XASUFW_SUCCESS, if endianness is changed successfully.
 *	-	XASUFW_INVALID_PARAM, if input parameters are invalid.
 *	-	XASUFW_FAILURE, if critical data secure zeroization is failed.
 *
 * @note
 * 	- This API supports endianness change only for even lengths.
 *
 *************************************************************************************************/
s32 XAsufw_ChangeEndianness(u8 *Buffer, u32 Length)
{
	s32 Status = XASUFW_FAILURE;
	volatile u32 Index;
	volatile u8 TempVar;

	if ((Buffer == NULL) || (Length == 0U) ||
				((Length % XASUFW_ENDIANNESS_SWAP_MIN_LENGTH) != 0U)) {
		Status = XASUFW_INVALID_PARAM;
	} else {
		for (Index = 0U; Index < (Length / XASUFW_ENDIANNESS_SWAP_MIN_LENGTH); Index++) {
			TempVar = Buffer[Index];
			Buffer[Index] = Buffer[Length - Index - XASUFW_ENDIANNESS_SWAP_OFFSET];
			Buffer[Length - Index - XASUFW_ENDIANNESS_SWAP_OFFSET] = TempVar;
		}
		if (Index == (Length / XASUFW_ENDIANNESS_SWAP_MIN_LENGTH)) {
			Status = XASUFW_SUCCESS;
		}
	}

	Status = XAsufw_UpdateBufStatus(Status, Xil_SecureZeroize((u8 *)(UINTPTR)&TempVar,
					sizeof(u8)));

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function checks whether the buffer has a non-zero value or not.
 *
 * @param	Buffer	Pointer to the buffer whose value needs to be checked.
 * @param	Length	Length of the buffer in bytes.
 *
 * @return
 *	-	XASUFW_SUCCESS, if buffer has non-zero value.
 *	-	XASUFW_INVALID_PARAM, if input parameters are invalid.
 *	-	XASUFW_FAILURE, if buffer has all zeroes as values.
 *
 *************************************************************************************************/
s32 XAsufw_IsBufferNonZero(u8 *Buffer, u32 Length)
{
	s32 Status = XASUFW_FAILURE;
	volatile u32 Index;

	if ((Buffer == NULL) || (Length == 0U)) {
		Status = XASUFW_INVALID_PARAM;
	} else {
		for (Index = 0U; Index < Length ; Index++) {
			if (Buffer[Index] != 0U) {
				Status = XASUFW_SUCCESS;
				break;
			}
		}
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to PLM for programming the requested off chip
 * Revocation Id eFuses.
 *
 * @param	OffChipRevokeIdNum	Off Chip Revocation ID number which needs to be programmed
 *
 * @return
 *	-	XASUFW_SUCCESS, if efuse off chip revoke ID is programmed.
 *	-	XASUFW_ERR_IPI_SEND_PLM_EFUSE_PRGM, if the IPI request sending to PLM fails.
 *	-	XASUFW_ERR_IPI_RSP_PLM_EFUSE_PRGM, if the IPI response fails.
 *
 *************************************************************************************************/
s32 XAsufw_NvmEfuseWriteOffChipRevokeId(const u32 OffChipRevokeIdNum)
{
	s32 Status = XASUFW_FAILURE;
	u32 Payload[XASUFW_PLM_XILNVM_MAX_PAYLOAD_LEN];
	u32 Response = (u32)XASUFW_FAILURE;

	/** Fill the payload with command header and Off chip revocation ID to be programmed */
	Payload[XASUFW_PLM_XILNVM_CMD_HDR_INDEX] = XASUFW_PLM_IPI_HEADER(0U,
					XASUFW_PLM_XILNVM_WRITE_OFFCHIP_REVOKE_API_ID,
					XASUFW_PLM_XILNVM_MODULE_ID);
	Payload[XASUFW_PLM_XILNVM_ENVCTRL_INDEX] = XASUFW_PLM_XILNVM_ENV_MON_CHECK;
	Payload[XASUFW_PLM_XILNVM_OFFCHID_INDEX] = OffChipRevokeIdNum;

	/** Send an IPI request to PLM to write efuse off chip revoke ID */
	Status = XAsufw_SendIpiToPlm(Payload, XASUFW_PLM_XILNVM_MAX_PAYLOAD_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(XASUFW_ERR_IPI_SEND_PLM_EFUSE_PRGM, Status);
		goto END;
	}

	/** Read IPI response */
	Status = XAsufw_ReadIpiRespFromPlm(&Response, XASUFW_PLM_XILNVM_RESP_LEN);
	if ((Status != XASUFW_SUCCESS) || (Response != (u32)XASUFW_SUCCESS)) {
		Status = XAsufw_UpdateErrorStatus(XASUFW_ERR_IPI_RSP_PLM_EFUSE_PRGM, Status);
	}
END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sets the memory of given size with zeroes by calling Xil_SMemset
 *		twice for redundancy.
 *
 * @param	Dest		Pointer to the buffer whose memory needs to be cleared.
 * @param	DestSize	Size of the buffer in bytes.
 *
 * @return
 *	-	XASUFW_SUCCESS, if Xil_SMemSet is successful.
 *	-	XASUFW_FAILURE, if Xil_SMemSet fails.
 *
 *************************************************************************************************/
s32 XAsufw_SMemSet(void *Dest, const u32 DestSize)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	Status = Xil_SMemSet(Dest, DestSize, 0x00U, DestSize);
	Status |= Xil_SMemSet(Dest, DestSize, 0x00U, DestSize);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function writes a sequence of data words to the specified registers by
 * 		swapping the endian.
 *
 * @param	BaseAddress	Base address of the peripheral.
 * @param	RegOffset	Starting register offset for writing data.
 * @param	DataArray	Array of data words to be written to the registers.
 * @param	NumOfWords	Number of data words to write to the registers.
 *
 * @return	- XASUFW_SUCCESS, if write data to registers is successful.
 * 		- XASUFW_FAILURE, if write data to registers fails.
 *
 *************************************************************************************************/
s32 XAsufw_WriteDataToRegsWithEndianSwap(u32 BaseAddress, u32 RegOffset, const u32 *DataArray,
	u32 NumOfWords)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	volatile u32 Index = 0U;

	/** Write data words to the respective registers by converting them to big-endian. */
	for (Index = 0U; Index < NumOfWords; Index++) {
		XAsufw_WriteReg((BaseAddress + RegOffset), Xil_Htonl(DataArray[Index]));
		RegOffset = RegOffset - XASUFW_WORD_LEN_IN_BYTES;
	}

	/** If all data words were written successfully, return success. */
	if ((Index == NumOfWords) && (NumOfWords != 0U)) {
		Status = XASUFW_SUCCESS;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function converts ASCII value to integer form.
 *
 * @param	Buf	Pointer to Data buffer.
 * @param	Len	Length of ascii value.
 *
 * @return
 *	- ASCII to integer converted value.
 *
 *************************************************************************************************/
u32 XAsufw_AsciiToInt(const u8 *Buf, u32 Len)
{
	u32 Val = 0U;
	u32 Idx = 0U;
	u32 LenTmp = Len;

	/** Convert ASCII value to integer. */
	while (LenTmp != 0U) {
		Val = ((Val * 10U) + (Buf[Idx] - (u8)'0'));
		Idx++;
		LenTmp--;
	}

	return Val;
}
/** @} */
