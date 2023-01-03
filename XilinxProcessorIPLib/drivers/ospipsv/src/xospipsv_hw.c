/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xospipsv_hw.c
* @addtogroup ospipsv Overview
* @{
*
* This file implements the hardware functions used by the functions in
* xospipsv_control.c file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.2   sk  02/20/20 First release
* 1.3   sk   04/09/20 Added support for 64-bit address read from 32-bit proc.
*       sk  08/19/20 Reduced the usleep delay while checking transfer done.
* 1.4   sk   02/18/21 Added support for Dual byte opcode.
* 1.6   sk  02/07/22 Replaced driver version in addtogroup with Overview.
* 1.8   akm  01/03/23 Use Xil_WaitForEvent() API for register bit polling.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xospipsv_control.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/
#define MAX_STIG_DELAY_CNT	50000U	/**< Max STIG delay count */
#define MAX_DMA_DELAY_CNT	10000000U	/**< Max DMA delay count */
#define LOCK_MAX_DELAY_CNT	10000000U	/**< Max LOCK delay count */
#define TERA_MACRO		1000000000000U	/**<Macro for 10^12 */
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* @brief
* This function configures the STIG control register (Flash cmd register)
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Cmd_op is command opcode.
* @param	Rd_data_en specifies whether cmd_op requires data to read or not.
* @param	Num_rd_data_bytes is number of bytes to read.
* @param	Cmd_addr_en specifies whether cmd_op requires addr or not.
* @param	Mode_bit_en is used to represent mode bit configuration reg.
* @param	Num_addr_bytes is used to represent addr bytes (4/3 etc).
* @param	Wr_data_en specifies whether cmd_op requires write enable or not.
* @param	Num_wr_data_bytes is number of bytes to write.
* @param	Dummy is number of bytes to write.
* @param	Membank_en is used to enable STIG memory bank.
*
* @return	None.
*
******************************************************************************/
void XOspiPsv_Setup_Stig_Ctrl(const XOspiPsv *InstancePtr,
		u32 Cmd_op, u32 Rd_data_en,	u32 Num_rd_data_bytes, u32 Cmd_addr_en,
		u32 Mode_bit_en, u32 Num_addr_bytes, u32 Wr_data_en,
		u32 Num_wr_data_bytes, u32 Dummy, u32 Membank_en)
{
	u32 Val;

	Val =(((Cmd_op << (u32)XOSPIPSV_FLASH_CMD_CTRL_REG_CMD_OPCODE_FLD_SHIFT)
			& XOSPIPSV_FLASH_CMD_CTRL_REG_CMD_OPCODE_FLD_MASK) |
		((Rd_data_en <<
			(u32)XOSPIPSV_FLASH_CMD_CTRL_REG_ENB_READ_DATA_FLD_SHIFT)
			& XOSPIPSV_FLASH_CMD_CTRL_REG_ENB_READ_DATA_FLD_MASK) |
		((Num_rd_data_bytes <<
		(u32)XOSPIPSV_FLASH_CMD_CTRL_REG_NUM_RD_DATA_BYTES_FLD_SHIFT)
			& XOSPIPSV_FLASH_CMD_CTRL_REG_NUM_RD_DATA_BYTES_FLD_MASK) |
		((Cmd_addr_en <<
		(u32)XOSPIPSV_FLASH_CMD_CTRL_REG_ENB_COMD_ADDR_FLD_SHIFT)
			& XOSPIPSV_FLASH_CMD_CTRL_REG_ENB_COMD_ADDR_FLD_MASK) |
		((Mode_bit_en <<
		(u32)XOSPIPSV_FLASH_CMD_CTRL_REG_ENB_MODE_BIT_FLD_SHIFT)
			& XOSPIPSV_FLASH_CMD_CTRL_REG_ENB_MODE_BIT_FLD_MASK) |
		((Num_addr_bytes <<
		(u32)XOSPIPSV_FLASH_CMD_CTRL_REG_NUM_ADDR_BYTES_FLD_SHIFT)
			& XOSPIPSV_FLASH_CMD_CTRL_REG_NUM_ADDR_BYTES_FLD_MASK) |
		((Wr_data_en <<
		(u32)XOSPIPSV_FLASH_CMD_CTRL_REG_ENB_WRITE_DATA_FLD_SHIFT)
			& XOSPIPSV_FLASH_CMD_CTRL_REG_ENB_WRITE_DATA_FLD_MASK) |
		((Num_wr_data_bytes <<
		(u32)XOSPIPSV_FLASH_CMD_CTRL_REG_NUM_WR_DATA_BYTES_FLD_SHIFT)
			& XOSPIPSV_FLASH_CMD_CTRL_REG_NUM_WR_DATA_BYTES_FLD_MASK) |
		((Membank_en <<
		(u32)XOSPIPSV_FLASH_CMD_CTRL_REG_STIG_MEM_BANK_EN_FLD_SHIFT)
			& XOSPIPSV_FLASH_CMD_CTRL_REG_STIG_MEM_BANK_EN_FLD_MASK) |
		((Dummy <<
		(u32)XOSPIPSV_FLASH_CMD_CTRL_REG_NUM_DUMMY_CYCLES_FLD_SHIFT)
			& XOSPIPSV_FLASH_CMD_CTRL_REG_NUM_DUMMY_CYCLES_FLD_MASK) );

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_FLASH_CMD_CTRL_REG, Val);
}

/*****************************************************************************/
/**
* @brief
* This function executes the Flash command configured using Flash Command
* control register
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return
* 		- XST_SUCCESS if successful.
*		- XST_FAILURE if transfer fails.
*
* @note		Wait till the command is executed.
*
******************************************************************************/
u32 XOspiPsv_Exec_Flash_Cmd(const XOspiPsv *InstancePtr)
{
	u32 Cmd_ctrl;
	u32 Status;

	Cmd_ctrl = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_FLASH_CMD_CTRL_REG);
	Cmd_ctrl |= (XOSPIPSV_FLASH_CMD_CTRL_REG_CMD_EXEC_FLD_MASK);
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_FLASH_CMD_CTRL_REG, Cmd_ctrl);

	if (Xil_WaitForEvent((InstancePtr->Config.BaseAddress + XOSPIPSV_FLASH_CMD_CTRL_REG),
				XOSPIPSV_FLASH_CMD_CTRL_REG_CMD_EXEC_STATUS_FLD_MASK,
				0x00,
				MAX_STIG_DELAY_CNT) != (u32)XST_SUCCESS) {
		Status = (s32)XST_FAILURE;
		goto ERROR_PATH;
	}
	Status = (u32)XST_SUCCESS;

ERROR_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Read the data from RX FIFO
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return	None
*
* @note		This operation is in IO mode of reading.
*
******************************************************************************/
void XOspiPsv_FifoRead(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
{
	u32 Lower;
	u32 Upper;

	Lower = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_FLASH_RD_DATA_LOWER_REG);
	if(InstancePtr->RxBytes <= (u32)4) {
		Xil_MemCpy(Msg->RxBfrPtr, &Lower, InstancePtr->RxBytes);
	} else {
		Xil_MemCpy(Msg->RxBfrPtr, &Lower, 4);
		Upper = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_FLASH_RD_DATA_UPPER_REG);
		Xil_MemCpy(&Msg->RxBfrPtr[4], &Upper, InstancePtr->RxBytes - (u32)4);
	}
	InstancePtr->RxBytes = 0U;
}

/*****************************************************************************/
/**
* @brief
* Write data to TX FIFO
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if transfer fails.
*
* @note		This operation is in IO mode of writing.
*
******************************************************************************/
void XOspiPsv_FifoWrite(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
{
	u32 Lower = 0;
	u32 Upper = 0;

	if(InstancePtr->TxBytes <= (u32)4) {
		Xil_MemCpy(&Lower, Msg->TxBfrPtr, InstancePtr->TxBytes);
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_FLASH_WR_DATA_LOWER_REG, Lower);
	} else {
		Xil_MemCpy(&Lower, Msg->TxBfrPtr, 4);
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_FLASH_WR_DATA_LOWER_REG, Lower);
		Xil_MemCpy(&Upper, &Msg->TxBfrPtr[4],InstancePtr->TxBytes - (u32)4);
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_FLASH_WR_DATA_UPPER_REG, Upper);
	}
	InstancePtr->TxBytes = 0U;
}

/*****************************************************************************/
/**
* @brief
* This function configures the below info to write instruction register
* DataXfertype - SPI/Dual/Quad/Octal
* AddrXfertype - 3 or 4B
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return	None
*
* @note		Used in DMA or Linear operations.
*
******************************************************************************/
void XOspiPsv_Setup_Dev_Write_Instr_Reg(const XOspiPsv *InstancePtr,
		const XOspiPsv_Msg *Msg)
{
	u32 Dummy_clks = 0U;
	u32 Dataxfer_Type;
	u32 Addrxfer_Type;
	u32 Instxfer_Type;
	u32 Regval;

	switch((u32)Msg->Proto) {
		case XOSPIPSV_WRITE_1_1_1:
			Dataxfer_Type = DQ0;
			Addrxfer_Type = DQ0;
			Instxfer_Type = DQ0;
			break;
		case XOSPIPSV_WRITE_1_1_8:
			Dataxfer_Type = DQ0_7;
			Addrxfer_Type = DQ0;
			Instxfer_Type = DQ0;
			break;
		case XOSPIPSV_WRITE_1_8_8:
			Instxfer_Type = DQ0;
			Dataxfer_Type = DQ0_7;
			Addrxfer_Type = DQ0_7;
			break;
		case XOSPIPSV_WRITE_8_8_8:
			Dataxfer_Type = DQ0_7;
			Addrxfer_Type = DQ0_7;
			Instxfer_Type = DQ0_7;
			break;
		case XOSPIPSV_WRITE_8_8_0:
			Addrxfer_Type = DQ0_7;
			Instxfer_Type = DQ0_7;
			Dataxfer_Type = DQ0;
			break;
		case XOSPIPSV_WRITE_8_0_0:
			Addrxfer_Type = DQ0;
			Instxfer_Type = DQ0_7;
			Dataxfer_Type = DQ0;
			break;
		case XOSPIPSV_WRITE_8_0_8:
			Addrxfer_Type = DQ0;
			Instxfer_Type = DQ0_7;
			Dataxfer_Type = DQ0_7;
			break;
		default :
			Dataxfer_Type = DQ0;
			Addrxfer_Type = DQ0;
			Instxfer_Type = DQ0;
			break;
	}

	Regval = (((Dummy_clks <<
		(u32)XOSPIPSV_DEV_INSTR_WR_CONFIG_REG_DUMMY_WR_CLK_CYCLES_FLD_SHIFT)
			& XOSPIPSV_DEV_INSTR_WR_CONFIG_REG_DUMMY_WR_CLK_CYCLES_FLD_MASK) |
		((Dataxfer_Type  <<
		(u32)XOSPIPSV_DEV_INSTR_WR_CONFIG_REG_DATA_XFER_TYPE_EXT_MODE_FLD_SHIFT)
		& XOSPIPSV_DEV_INSTR_WR_CONFIG_REG_DATA_XFER_TYPE_EXT_MODE_FLD_MASK) |
		((Addrxfer_Type <<
		(u32)XOSPIPSV_DEV_INSTR_WR_CONFIG_REG_ADDR_XFER_TYPE_STD_MODE_FLD_SHIFT)
		& XOSPIPSV_DEV_INSTR_WR_CONFIG_REG_ADDR_XFER_TYPE_STD_MODE_FLD_MASK) |
		(((u32)Msg->Opcode <<
		(u32)XOSPIPSV_DEV_INSTR_WR_CONFIG_REG_WR_OPCODE_FLD_SHIFT ) &
			XOSPIPSV_DEV_INSTR_WR_CONFIG_REG_WR_OPCODE_FLD_MASK));

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_DEV_INSTR_WR_CONFIG_REG, Regval);

	Regval = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
					XOSPIPSV_DEV_INSTR_RD_CONFIG_REG);
	Regval &= ~XOSPIPSV_DEV_INSTR_RD_CONFIG_REG_INSTR_TYPE_FLD_MASK;
	Regval |= ((Instxfer_Type <<
		(u32)XOSPIPSV_DEV_INSTR_RD_CONFIG_REG_INSTR_TYPE_FLD_SHIFT)
		& XOSPIPSV_DEV_INSTR_RD_CONFIG_REG_INSTR_TYPE_FLD_MASK);
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_DEV_INSTR_RD_CONFIG_REG, Regval);

	if (InstancePtr->DualByteOpcodeEn != 0U) {
		Regval = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
					XOSPIPSV_OPCODE_EXT_LOWER_REG);
		Regval &= ~(u32)XOSPIPSV_OPCODE_EXT_LOWER_REG_EXT_WRITE_OPCODE_FLD_MASK;
		Regval |= ((u32)Msg->ExtendedOpcode <<
				(u32)XOSPIPSV_OPCODE_EXT_LOWER_REG_EXT_WRITE_OPCODE_FLD_SHIFT);
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_OPCODE_EXT_LOWER_REG, Regval);
	}
}

/*****************************************************************************/
/**
* @brief
* This function configures the below info to read instruction register
* DataXfertype - SPI/Dual/Quad/Octal
* AddrXfertype - 3 or 4B
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return	None
*
* @note		Used in DMA or Linear operations
*
******************************************************************************/
void XOspiPsv_Setup_Dev_Read_Instr_Reg(const XOspiPsv *InstancePtr,
		const XOspiPsv_Msg *Msg)
{
	u32 Mode_bit_en = 0;
	u32 Dataxfer_Type;
	u32 Addrxfer_Type;
	u32 Instxfer_Type;
	u32 Regval;

	switch((u32)Msg->Proto) {
		case XOSPIPSV_READ_1_1_1:
			Dataxfer_Type = DQ0;
			Addrxfer_Type = DQ0;
			Instxfer_Type = DQ0;
			break;
		case XOSPIPSV_READ_1_1_8:
			Dataxfer_Type = DQ0_7;
			Addrxfer_Type = DQ0;
			Instxfer_Type = DQ0;
			break;
		case XOSPIPSV_READ_1_8_8:
			Dataxfer_Type = DQ0_7;
			Addrxfer_Type = DQ0_7;
			Instxfer_Type = DQ0;
			break;
		case XOSPIPSV_READ_8_8_8:
			Instxfer_Type = DQ0_7;
			Dataxfer_Type = DQ0_7;
			Addrxfer_Type = DQ0_7;
			break;
		case XOSPIPSV_READ_8_0_8:
			Instxfer_Type = DQ0_7;
			Dataxfer_Type = DQ0_7;
			Addrxfer_Type = DQ0;
			break;
		default :
			Instxfer_Type = DQ0;
			Dataxfer_Type = DQ0;
			Addrxfer_Type = DQ0;
			break;
	}

	Regval = ((((u32)Msg->Dummy <<
		(u32)XOSPIPSV_DEV_INSTR_RD_CONFIG_REG_DUMMY_RD_CLK_CYCLES_FLD_SHIFT)
		& XOSPIPSV_DEV_INSTR_RD_CONFIG_REG_DUMMY_RD_CLK_CYCLES_FLD_MASK) |
		((Mode_bit_en <<
		(u32)XOSPIPSV_DEV_INSTR_RD_CONFIG_REG_MODE_BIT_ENABLE_FLD_SHIFT)
		& XOSPIPSV_DEV_INSTR_RD_CONFIG_REG_MODE_BIT_ENABLE_FLD_MASK) |
		((Dataxfer_Type <<
		(u32)XOSPIPSV_DEV_INSTR_RD_CONFIG_REG_DATA_XFER_TYPE_EXT_MODE_FLD_SHIFT)
		& XOSPIPSV_DEV_INSTR_RD_CONFIG_REG_DATA_XFER_TYPE_EXT_MODE_FLD_MASK) |
		((Addrxfer_Type <<
		(u32)XOSPIPSV_DEV_INSTR_RD_CONFIG_REG_ADDR_XFER_TYPE_STD_MODE_FLD_SHIFT)
		& XOSPIPSV_DEV_INSTR_RD_CONFIG_REG_ADDR_XFER_TYPE_STD_MODE_FLD_MASK) |
		((Instxfer_Type <<
				(u32)XOSPIPSV_DEV_INSTR_RD_CONFIG_REG_INSTR_TYPE_FLD_SHIFT)
		& XOSPIPSV_DEV_INSTR_RD_CONFIG_REG_INSTR_TYPE_FLD_MASK) |
		(((u32)Msg->Opcode <<
		(u32)XOSPIPSV_DEV_INSTR_RD_CONFIG_REG_RD_OPCODE_NON_XIP_FLD_SHIFT)
		& XOSPIPSV_DEV_INSTR_RD_CONFIG_REG_RD_OPCODE_NON_XIP_FLD_MASK) |
		(((u32)Msg->IsDDROpCode <<
		(u32)XOSPIPSV_DEV_INSTR_RD_CONFIG_REG_DDR_EN_FLD_SHIFT)
		& XOSPIPSV_DEV_INSTR_RD_CONFIG_REG_DDR_EN_FLD_MASK));
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_DEV_INSTR_RD_CONFIG_REG, Regval);

	Regval = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_RD_DATA_CAPTURE_REG);
	Regval &= ~(u32)XOSPIPSV_RD_DATA_CAPTURE_REG_DQS_ENABLE_FLD_MASK;
	if ((Msg->IsDDROpCode != 0U) ||
			(InstancePtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY)) {
		Regval |= XOSPIPSV_RD_DATA_CAPTURE_REG_DQS_ENABLE_FLD_MASK;
	}
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_RD_DATA_CAPTURE_REG, Regval);

	if (InstancePtr->DualByteOpcodeEn != 0U) {
		Regval = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
					XOSPIPSV_OPCODE_EXT_LOWER_REG);
		Regval &= ~(u32)XOSPIPSV_OPCODE_EXT_LOWER_REG_EXT_READ_OPCODE_FLD_MASK;
		Regval |= ((u32)Msg->ExtendedOpcode <<
				(u32)XOSPIPSV_OPCODE_EXT_LOWER_REG_EXT_READ_OPCODE_FLD_SHIFT);
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_OPCODE_EXT_LOWER_REG, Regval);
	}
}

/*****************************************************************************/
/**
* @brief
* This function sets the device size config register
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return	None
*
******************************************************************************/
void XOspiPsv_Setup_Devsize(const XOspiPsv *InstancePtr,
				const XOspiPsv_Msg *Msg)
{
	u32 Reg;

	Reg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_DEV_SIZE_CONFIG_REG);
	Reg &= ~(XOSPIPSV_DEV_SIZE_CONFIG_REG_NUM_ADDR_BYTES_FLD_MASK);
	if (Msg->Addrsize != 0U) {
		Reg |= ((u32)Msg->Addrsize - (u32)1);
	}

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_DEV_SIZE_CONFIG_REG, Reg);
}

/*****************************************************************************/
/**
* @brief
* This function initiates the indirect read transfer
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return	None
*
******************************************************************************/
void XOspiPsv_Start_Indr_RdTransfer(const XOspiPsv *InstancePtr)
{
	u32 Val;

	Val = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_INDIRECT_READ_XFER_CTRL_REG);
	Val |= (XOSPIPSV_INDIRECT_READ_XFER_CTRL_REG_START_FLD_MASK);

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_INDIRECT_READ_XFER_CTRL_REG, (Val));
}

/*****************************************************************************/
/**
* @brief
* This function configures the Indirect controller
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return	None
*
******************************************************************************/
void XOspiPsv_Config_IndirectAhb(const XOspiPsv *InstancePtr,
		const XOspiPsv_Msg *Msg)
{
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_INDIRECT_READ_XFER_WATERMARK_REG, XOSPIPSV_RXWATER_MARK_DEF);

	/* Configure Address */
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_INDIRECT_READ_XFER_START_REG, Msg->Addr);

	/* Configure number of bytes to read and indirect address*/
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_INDIRECT_READ_XFER_NUM_BYTES_REG, Msg->ByteCount);

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_IND_AHB_ADDR_TRIGGER_REG, XOSPIPSV_IND_TRIGGAHB_BASE);
	/* configure trigger range */
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_INDIRECT_TRIGGER_ADDR_RANGE_REG, XOSPIPSV_IND_TRIGGER_RANGE);
}

/*****************************************************************************/
/**
* @brief
* This function setup the Dma configuration
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return	None
*
******************************************************************************/
void XOspiPsv_Config_Dma(const XOspiPsv *InstancePtr, const XOspiPsv_Msg *Msg)
{
	UINTPTR AddrTemp;

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_DMA_PERIPH_CONFIG_REG, XOSPIPSV_DMA_PERIPH_CONFIG_VAL);

	if ((Msg->RxAddr64bit >= XOSPIPSV_RXADDR_OVER_32BIT) &&
			(Msg->Xfer64bit != (u8)0U)) {
		AddrTemp = (Msg->RxAddr64bit &
				XOSPIPSV_OSPIDMA_DST_ADDR_ADDR_MASK);
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_OSPIDMA_DST_ADDR, (u32)AddrTemp);
		AddrTemp = Msg->RxAddr64bit >> 32;
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_OSPIDMA_DST_ADDR_MSB, (u32)AddrTemp &
				XOSPIPSV_OSPIDMA_DST_ADDR_MSB_ADDR_MSB_MASK);
	} else {
		AddrTemp = ((UINTPTR)(Msg->RxBfrPtr) &
				XOSPIPSV_OSPIDMA_DST_ADDR_ADDR_MASK);

		if (InstancePtr->Config.IsCacheCoherent == 0U) {
			Xil_DCacheInvalidateRange((INTPTR)Msg->RxBfrPtr, (INTPTR)Msg->ByteCount);
		}
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_OSPIDMA_DST_ADDR, (u32)AddrTemp);

#if defined(__aarch64__) || defined(__arch64__)
		AddrTemp = ((UINTPTR)(Msg->RxBfrPtr) >> 32);
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_OSPIDMA_DST_ADDR_MSB, (u32)AddrTemp &
			XOSPIPSV_OSPIDMA_DST_ADDR_MSB_ADDR_MSB_MASK);
#else
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_OSPIDMA_DST_ADDR_MSB, 0x0);
#endif
	}
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_SRAM_PARTITION_CFG_REG, XOSPIPSV_SRAM_PARTITION_CFG_VAL);

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_OSPIDMA_SRC_RD_ADDR, XOSPIPSV_IND_TRIGGAHB_BASE);

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_OSPIDMA_DST_SIZE, Msg->ByteCount);

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_OSPIDMA_DST_CTRL, XOSPIPSV_DMA_DST_CTRL_DEF);
}

/*****************************************************************************/
/**
* @brief
* This function Initiates the DMA transfer
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return
* 		- XST_SUCCESS if successful.
*		- XST_FAILURE if transfer fails.
*
******************************************************************************/
u32 XOspiPsv_Exec_Dma(const XOspiPsv *InstancePtr)
{
	u32 ReadReg;
	u32 Status;

	/* Start the transfer */
	ReadReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_INDIRECT_READ_XFER_CTRL_REG);
	ReadReg |= (XOSPIPSV_INDIRECT_READ_XFER_CTRL_REG_START_FLD_MASK);

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_INDIRECT_READ_XFER_CTRL_REG, (ReadReg));

	/* Wait for max delay of 10sec to complete the transfer */
	if (Xil_WaitForEvent((InstancePtr->Config.BaseAddress + XOSPIPSV_OSPIDMA_DST_I_STS),
				XOSPIPSV_OSPIDMA_DST_I_STS_DONE_MASK,
				XOSPIPSV_OSPIDMA_DST_I_STS_DONE_MASK,
				MAX_DMA_DELAY_CNT) != (u32)XST_SUCCESS) {
		Status = (s32)XST_FAILURE;
		goto ERROR_PATH;
	}

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_OSPIDMA_DST_I_STS,
		XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_OSPIDMA_DST_I_STS));

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_INDIRECT_READ_XFER_CTRL_REG,
		(XOSPIPSV_INDIRECT_READ_XFER_CTRL_REG_IND_OPS_DONE_STATUS_FLD_MASK));

	Status = (u32)XST_SUCCESS;

ERROR_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Wait for bit to be set. This API polls for the required bit for 10sec, if
* not set then timeout occurs.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Mask is a bit mask to check for lock.
*
* @return
*		- XST_SUCCESS if lock bit is set.
*		- XST_FAILURE if fails.
*
******************************************************************************/
u32 XOspiPsv_WaitForLock(const XOspiPsv *InstancePtr, u32 Mask)
{
	u32 Status;

	if (Xil_WaitForEvent((InstancePtr->Config.BaseAddress + XOSPIPSV_DLL_OBSERVABLE_LOWER_REG),
				Mask,
				Mask,
				LOCK_MAX_DELAY_CNT) != (u32)XST_SUCCESS) {
		Status = (s32)XST_FAILURE;
		goto ERROR_PATH;
	}

	Status = (u32)XST_SUCCESS;
ERROR_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Calculate the Max window size and the corresponding Average Rx Tap.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	FlashMsg is a pointer to XOspiPsv_Msg instance.
* @param	AvgRXTap is a pointer to median Rx Tap.
* @param	MaxWindowSize is a pointer to maximum window size.
* @param	DummyIncr is a flag to indicate additional dummy.
* @param	TXTap is a Tx tap value used while doing Rx tuning.
*
* @return
*		- XST_SUCCESS if lock bit is set.
*		- XST_FAILURE if fails.
*
******************************************************************************/
u32 XOspiPsv_CalculateRxTap(XOspiPsv *InstancePtr, XOspiPsv_Msg *FlashMsg,
		u8 *AvgRXTap, u8 *MaxWindowSize, u8 DummyIncr, u32 TXTap)
{
	u32 Status;
	const u32 *DeviceIdInfo;
	u8 RXMaxTap = 0;
	u8 RXMinTap = 0;
	u8 RXTapFound = 0;
	u8 WindowSize = 0;
	u8 MaxIndex = 0;
	u8 MinIndex = 0;
	u8 Index;
	u8 Count;
	u8 MaxTap;

	MaxTap = (u8)((u32)(TERA_MACRO/InstancePtr->Config.InputClockHz) / (u32)160);
	if (InstancePtr->DllMode == XOSPIPSV_DLL_MASTER_MODE) {
		MaxTap = (u8)XOSPIPSV_DLL_MAX_TAPS;
	}

	for (Index = 0U; Index <= MaxTap; Index++) {
		Status = XOspiPsv_ConfigureTaps(InstancePtr, Index, TXTap);
		if (Status != (u32)XST_SUCCESS) {
			goto RETURN_PATH;
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
					RXMinTap = (u8)XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
						XOSPIPSV_DLL_OBSERVABLE_UPPER_REG) &
						XOSPIPSV_DLL_OBSERVABLE_UPPER_RX_DECODER_OUTPUT_FLD_MASK;
					RXMaxTap = RXMinTap;
					MaxIndex = Index;
					MinIndex = Index;
				} else {
					RXMinTap = Index;
					RXMaxTap = Index;
				}
				RXTapFound = 1;
			} else {
				if (InstancePtr->DllMode == XOSPIPSV_DLL_MASTER_MODE) {
					RXMaxTap = (u8)XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
						XOSPIPSV_DLL_OBSERVABLE_UPPER_REG) &
						XOSPIPSV_DLL_OBSERVABLE_UPPER_RX_DECODER_OUTPUT_FLD_MASK;
					MaxIndex = Index;
				} else {
					RXMaxTap = Index;
				}
			}
		}
		if ((InstancePtr->DeviceIdData != *DeviceIdInfo) || (Index == MaxTap)) {
			if (RXTapFound != 0U) {
				WindowSize = RXMaxTap - RXMinTap + 1U;
				if (WindowSize > *MaxWindowSize) {
					InstancePtr->Extra_DummyCycle = DummyIncr;
					*MaxWindowSize = WindowSize;
					if (InstancePtr->DllMode == XOSPIPSV_DLL_MASTER_MODE) {
						*AvgRXTap = (MaxIndex + MinIndex) / 2U;
					} else {
						*AvgRXTap = (RXMinTap + RXMaxTap) / 2U;
					}
				}
				RXTapFound = 0U;
				if (WindowSize >= 3U) {
					break;
				}
			}
		}
	}

	Status = (u32)XST_SUCCESS;
RETURN_PATH:
	return Status;
}

/** @} */
