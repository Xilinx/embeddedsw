/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xospipsv_control.c
* @addtogroup ospipsv_v1_5
* @{
*
* This file implements the low level functions used by the functions in
* xospipsv.c and xospipsv_options.c files.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.2   sk  02/20/20 First release
* 1.3   sk   04/09/20 Added support for 64-bit address read from 32-bit proc.
* 1.4   sk   02/18/21 Added support for Dual byte opcode.
*       sk   02/18/21 Updated RX Tuning algorithm for Master DLL mode.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xospipsv_control.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/
/**< Maximum delay count */
#define MAX_DELAY_CNT	10000U
#define TERA_MACRO	1000000000000U	/**<Macro for 10^12 */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* @brief
* Flash command based data reading using flash command control registers.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if transfer fails.
*
* @note		This operation is in IO mode of reading.
*
******************************************************************************/
u32 XOspiPsv_Stig_Read(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
{
	u32 Reqaddr;
	u32 Status;
	u32 RegVal;

	if (InstancePtr->RxBytes <= 0U) {
		Status = (u32)XST_FAILURE;
		goto ERROR_PATH;
	}

	if (Msg->Addrvalid != 0U) {
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_FLASH_CMD_ADDR_REG, Msg->Addr);
		Reqaddr = 1;
	} else {
		Reqaddr = 0U;
	}

	XOspiPsv_Setup_Stig_Ctrl(InstancePtr, (u32)Msg->Opcode,
		1, (u32)InstancePtr->RxBytes - (u32)1, Reqaddr, 0, (u32)Msg->Addrsize - (u32)1,
		0, 0, (u32)Msg->Dummy, 0);

	if (InstancePtr->DualByteOpcodeEn != 0U) {
		RegVal = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
					XOSPIPSV_OPCODE_EXT_LOWER_REG);
		RegVal &= ~(u32)XOSPIPSV_OPCODE_EXT_LOWER_REG_EXT_STIG_OPCODE_FLD_MASK;
		RegVal |= ((u32)Msg->ExtendedOpcode <<
				(u32)XOSPIPSV_OPCODE_EXT_LOWER_REG_EXT_STIG_OPCODE_FLD_SHIFT);
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_OPCODE_EXT_LOWER_REG, RegVal);
	}

	/* Execute command */
	Status = XOspiPsv_Exec_Flash_Cmd(InstancePtr);
	if (Status != (u32)XST_SUCCESS) {
		goto ERROR_PATH;
	}

	XOspiPsv_FifoRead(InstancePtr, Msg);

ERROR_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Flash command based data write using flash command control registers.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return
* 		- XST_SUCCESS if successful.
*		- XST_FAILURE if fails.
*
* @note		This operation is in IO mode of writing.
*
******************************************************************************/
u32 XOspiPsv_Stig_Write(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
{
	u32 Reqaddr;
	u32 Reqwridataen;
	u32 ByteCount;
	u32 Status;
	u32 RegVal;

	if (Msg->Addrvalid != 0U) {
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_FLASH_CMD_ADDR_REG, Msg->Addr);
		Reqaddr = 1;
	} else {
		Reqaddr = 0U;
	}
	if (InstancePtr->TxBytes != 0U) {
		Reqwridataen = 1;
		ByteCount = InstancePtr->TxBytes;
		XOspiPsv_FifoWrite(InstancePtr, Msg);
	} else {
		Reqwridataen = 0U;
		ByteCount = 1;
	}
	XOspiPsv_Setup_Stig_Ctrl(InstancePtr, (u32)Msg->Opcode,
		0, 0, Reqaddr, 0, (u32)Msg->Addrsize - (u32)1,
		Reqwridataen, (u32)ByteCount - (u32)1, 0, 0);

	if (InstancePtr->DualByteOpcodeEn != 0U) {
		RegVal = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
					XOSPIPSV_OPCODE_EXT_LOWER_REG);
		RegVal &= ~(u32)XOSPIPSV_OPCODE_EXT_LOWER_REG_EXT_STIG_OPCODE_FLD_MASK;
		RegVal |= ((u32)Msg->ExtendedOpcode <<
				(u32)XOSPIPSV_OPCODE_EXT_LOWER_REG_EXT_STIG_OPCODE_FLD_SHIFT);
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_OPCODE_EXT_LOWER_REG, RegVal);
	}

	/* Exec cmd */
	Status = XOspiPsv_Exec_Flash_Cmd(InstancePtr);

	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function Read the data using DMA
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return
* 		- XST_SUCCESS if successful.
*		- XST_FAILURE if transfer fails.
*
******************************************************************************/
u32 XOspiPsv_Dma_Read(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
{
	u32 Status;

	if ((Msg->ByteCount % 4U) != 0U) {
		InstancePtr->IsUnaligned = 1;
	}

	if (Msg->ByteCount >= (u32)4) {
		Msg->ByteCount -= (Msg->ByteCount % 4U);
		XOspiPsv_Config_Dma(InstancePtr,Msg);
		XOspiPsv_Config_IndirectAhb(InstancePtr,Msg);
		Status = XOspiPsv_Exec_Dma(InstancePtr);
		if (Status != (u32)XST_SUCCESS) {
			goto ERROR_PATH;
		}
		if (Msg->Xfer64bit != (u8)1U) {
			if (InstancePtr->Config.IsCacheCoherent == 0U) {
				Xil_DCacheInvalidateRange((INTPTR)Msg->RxBfrPtr, (INTPTR)Msg->ByteCount);
			}
		}
		if (InstancePtr->IsUnaligned != 0U) {
			InstancePtr->RecvBufferPtr += Msg->ByteCount;
			Msg->Addr += Msg->ByteCount;
		}
	}

	if (InstancePtr->IsUnaligned != 0U) {
		Msg->ByteCount = 4;
		Msg->RxBfrPtr = InstancePtr->UnalignReadBuffer;
		InstancePtr->RxBytes = (InstancePtr->RxBytes % 4U);
		XOspiPsv_Config_Dma(InstancePtr,Msg);
		XOspiPsv_Config_IndirectAhb(InstancePtr,Msg);
		Status = XOspiPsv_Exec_Dma(InstancePtr);
		if (Status != (u32)XST_SUCCESS) {
			goto ERROR_PATH;
		}
		if (InstancePtr->Config.IsCacheCoherent == 0U) {
			Xil_DCacheInvalidateRange((INTPTR)Msg->RxBfrPtr, (INTPTR)Msg->ByteCount);
		}
		Xil_MemCpy(InstancePtr->RecvBufferPtr, InstancePtr->UnalignReadBuffer,
				InstancePtr->RxBytes);
		InstancePtr->IsUnaligned = 0U;
	}

	Status = (u32)XST_SUCCESS;

ERROR_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function reads the data using Linear controller
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return
* 		- XST_SUCCESS if successful.
*		- XST_FAILURE for invalid address.
*
******************************************************************************/
u32 XOspiPsv_Dac_Read(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
{
	u32 Status;
	const u32 *Addr = (u32 *)XOSPIPSV_LINEAR_ADDR_BASE + Msg->Addr;

	if (Addr >= (u32 *)(XOSPIPSV_LINEAR_ADDR_BASE + SIZE_512MB)) {
		Status = XST_FAILURE;
		goto ERROR_PATH;
	}

	Xil_MemCpy(Msg->RxBfrPtr, Addr, InstancePtr->RxBytes);
	InstancePtr->RxBytes = 0U;

	Status = (u32)XST_SUCCESS;
ERROR_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function writes the data Using Linear controller
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return
* 		- XST_SUCCESS if successful.
*		- XST_FAILURE for invalid address.
*
******************************************************************************/
u32 XOspiPsv_Dac_Write(XOspiPsv *InstancePtr, const XOspiPsv_Msg *Msg)
{
	u32 Status;
	u32 *Addr = (u32 *)XOSPIPSV_LINEAR_ADDR_BASE + Msg->Addr;

	if (Addr >= (u32 *)(XOSPIPSV_LINEAR_ADDR_BASE + SIZE_512MB)) {
		Status = XST_FAILURE;
		goto ERROR_PATH;
	}

	Xil_MemCpy(Addr, Msg->TxBfrPtr, InstancePtr->TxBytes);
	InstancePtr->TxBytes = 0U;

	Status = (u32)XST_SUCCESS;
ERROR_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This API perform RX Tuning for SDR/DDR mode to calculate RX DLL Delay.
*
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	FlashMsg is a pointer to the XOspiPsv_Msg structure.
* @param	TXTap is TX DLL Delay value.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fails.
*
******************************************************************************/
u32 XOspiPsv_ExecuteRxTuning(XOspiPsv *InstancePtr, XOspiPsv_Msg *FlashMsg,
								u32 TXTap)
{
	u8 RXMax_Tap = 0;
	u8 RXMin_Tap = 0;
	u8 Avg_RXTap = 0;
	u8 Index;
	const u32 *DeviceIdInfo;
	u8 RXTapFound = 0;
	u32 Status;
	u8 MaxTap;
	u8 WindowSize;
	u8 Max_WindowSize = 0;
	u8 Dummy_Incr;
	u8 Dummy_Flag = 0;
	u8 Count;
	u8 Dummy = FlashMsg->Dummy;
	u8 MaxIndex = 0, MinIndex = 0;

	MaxTap = (u8)((u32)(TERA_MACRO/InstancePtr->Config.InputClockHz) / (u32)160);
	if (InstancePtr->DllMode == XOSPIPSV_DLL_MASTER_MODE) {
		MaxTap = (u8)XOSPIPSV_DLL_MAX_TAPS;
	}
	for (Dummy_Incr = 0U; Dummy_Incr <= 1U; Dummy_Incr++) {
		if (Dummy_Incr != 0U) {
				FlashMsg->Dummy = Dummy + 1U;
		}
		for (Index = 0U; Index <= MaxTap; Index++) {
			XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_PHY_CONFIGURATION_REG, (TXTap | (u32)Index |
				XOSPIPSV_PHY_CONFIGURATION_REG_PHY_CONFIG_RESET_FLD_MASK));
			XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_PHY_CONFIGURATION_REG, (TXTap | (u32)Index |
					XOSPIPSV_PHY_CONFIGURATION_REG_PHY_CONFIG_RESET_FLD_MASK |
					XOSPIPSV_PHY_CONFIGURATION_REG_PHY_CONFIG_RESYNC_FLD_MASK));
			if (InstancePtr->DllMode == XOSPIPSV_DLL_MASTER_MODE) {
				Status = XOspiPsv_WaitForLock(InstancePtr,
						XOSPIPSV_DLL_OBSERVABLE_LOWER_DLL_LOCK_FLD_MASK);
				if (Status != (u32)XST_SUCCESS) {
					goto RETURN_PATH;
				}
			}

			Count = (u8)0U;
			do {
				Count += (u8)1U;
				Status = XOspiPsv_PollTransfer(InstancePtr, FlashMsg);
				if (Status != (u32)XST_SUCCESS) {
					goto RETURN_PATH;
				}
				DeviceIdInfo = (u32 *)&(FlashMsg->RxBfrPtr[0]);
			} while((InstancePtr->DeviceIdData == *DeviceIdInfo) && (Count <= (u8)10U));
			if (InstancePtr->DeviceIdData == *DeviceIdInfo) {
				if (RXTapFound == 0U) {
					if (InstancePtr->DllMode == XOSPIPSV_DLL_MASTER_MODE) {
						RXMin_Tap = (u8)XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
							XOSPIPSV_DLL_OBSERVABLE_UPPER_REG) &
							XOSPIPSV_DLL_OBSERVABLE_UPPER_RX_DECODER_OUTPUT_FLD_MASK;
						RXMax_Tap = RXMin_Tap;
						MaxIndex = Index;
						MinIndex = Index;
					} else {
						RXMin_Tap = Index;
						RXMax_Tap = Index;
					}
					RXTapFound = 1;
				} else {
					if (InstancePtr->DllMode == XOSPIPSV_DLL_MASTER_MODE) {
						RXMax_Tap = (u8)XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
							XOSPIPSV_DLL_OBSERVABLE_UPPER_REG) &
							XOSPIPSV_DLL_OBSERVABLE_UPPER_RX_DECODER_OUTPUT_FLD_MASK;
						MaxIndex = Index;
					} else {
						RXMax_Tap = Index;
					}
				}
			}
			if ((InstancePtr->DeviceIdData != *DeviceIdInfo) || (Index == MaxTap)) {
				if (RXTapFound != 0U) {
					WindowSize = RXMax_Tap - RXMin_Tap + 1U;
					if (WindowSize > Max_WindowSize) {
						Dummy_Flag = Dummy_Incr;
						Max_WindowSize = WindowSize;
						if (InstancePtr->DllMode == XOSPIPSV_DLL_MASTER_MODE) {
							Avg_RXTap = (MaxIndex + MinIndex) / 2U;
						} else {
							Avg_RXTap = (RXMin_Tap + RXMax_Tap) / 2U;
						}
					}
					RXTapFound = 0U;
					if (WindowSize >= 3U) {
						break;
					}
				}
			}
		}
		if (Dummy_Incr == 0U) {
			RXMin_Tap = 0U;
			RXMax_Tap = 0U;
			RXTapFound = 0U;
			WindowSize = 0U;
		}
	}
	InstancePtr->Extra_DummyCycle = Dummy_Flag;

	if (Max_WindowSize < 3U) {
		Status = (u32)XST_FAILURE;
		goto RETURN_PATH;
	}

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_PHY_CONFIGURATION_REG, (TXTap | (u32)Avg_RXTap |
		XOSPIPSV_PHY_CONFIGURATION_REG_PHY_CONFIG_RESET_FLD_MASK));
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_PHY_CONFIGURATION_REG, (TXTap | (u32)Avg_RXTap |
		XOSPIPSV_PHY_CONFIGURATION_REG_PHY_CONFIG_RESET_FLD_MASK |
		XOSPIPSV_PHY_CONFIGURATION_REG_PHY_CONFIG_RESYNC_FLD_MASK));
	if (InstancePtr->DllMode == XOSPIPSV_DLL_MASTER_MODE) {
		Status = XOspiPsv_WaitForLock(InstancePtr,
				XOSPIPSV_DLL_OBSERVABLE_LOWER_DLL_LOCK_FLD_MASK);
	} else {
		Status = (u32)XST_SUCCESS;
	}

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Check for OSPI idle which means Serial interface and low level SPI pipeline
* is IDLE.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fails.
*
*
******************************************************************************/
u32 XOspiPsv_CheckOspiIdle(const XOspiPsv *InstancePtr)
{
	u32 ReadReg;
	u32 Status;
	u32 DelayCount;

	ReadReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_CONFIG_REG);
	DelayCount = 0U;
	while ((ReadReg & XOSPIPSV_CONFIG_REG_IDLE_FLD_MASK) == 0U) {
		if (DelayCount == MAX_DELAY_CNT) {
			Status = XST_FAILURE;
			goto ERROR_PATH;
		} else {
			/* Wait for 1 usec */
			usleep(1);
			DelayCount++;
			ReadReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
					XOSPIPSV_CONFIG_REG);
		}
	}

	Status = (u32)XST_SUCCESS;
ERROR_PATH:
	return Status;
}

/** @} */
