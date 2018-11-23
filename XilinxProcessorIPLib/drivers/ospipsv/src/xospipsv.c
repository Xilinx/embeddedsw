/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
*
* @file xospipsv.c
* @addtogroup xospipsv_v1_0
* @{
*
* This file implements the functions required to use the OSPIPS hardware to
* perform a transfer. These are accessible to the user via XOspiPsv.h.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.0   nsk  02/19/18 First release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xospipsv.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static inline u32 XOspiPsv_Process_Read_Write(XOspiPsv *InstancePtr,
	XOspiPsv_Msg *Msg);
static inline u32 XOspiPsv_Process_Stig_Read(XOspiPsv *InstancePtr,
	XOspiPsv_Msg *Msg);
static inline u32 XOspiPsv_Process_Stig_Data_Write(XOspiPsv *InstancePtr,
	XOspiPsv_Msg *Msg);
static inline void XOspiPsv_Process_Stig_Command_Write(XOspiPsv *InstancePtr,
	XOspiPsv_Msg *Msg);
static inline u32 XOspiPsv_Dac_Read(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg);
static inline u32 XOspiPsv_Dac_Write(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg);
static inline void XOspiPsv_Dac_Configure(XOspiPsv *InstancePtr,
	XOspiPsv_Msg *Msg);
static inline u32 XOspiPsv_Check_Idle(XOspiPsv *InstancePtr);
static inline void XOspiPsv_Disable(XOspiPsv *InstancePtr);
static inline void XOspiPsv_Enable(XOspiPsv *InstancePtr);
static inline void XOspiPsv_Disable_Dac(XOspiPsv *InstancePtr);
static inline void XOspiPsv_Enable_Dac(XOspiPsv *InstancePtr);
static inline u32 XOspiPsv_Indr_RdTransfer_Complete(XOspiPsv *InstancePtr);
static inline void XOspiPsv_Setup_Stig_Ctrl(XOspiPsv *InstancePtr, u32 Cmd_op,
	u32 Rd_data_en, u32 Num_rd_data_bytes, u32 Cmd_addr_en, u32 Mode_bit_en,
	u32 Num_addr_bytes, u32 Wr_data_en, u32 Num_wr_data_bytes, u32 Dummy,
	u32 Membank_en);
static inline void XOspiPsv_EnableDma(XOspiPsv *InstancePtr);
static inline void XOspiPsv_DisableDma(XOspiPsv *InstancePtr);
static inline void XOspiPsv_Setup_Dev_Write_Instr_Reg(XOspiPsv *InstancePtr,
	XOspiPsv_Msg *Msg);
static inline void XOspiPsv_Setup_Dev_Read_Instr_Reg(XOspiPsv *InstancePtr,
	XOspiPsv_Msg *Msg);
static inline void XOspiPsv_Setup_Devsize(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg);
static inline void XOspiPsv_Start_Indr_RdTransfer(XOspiPsv *InstancePtr);
static inline void XOspiPsv_Config_IndirectAhb(XOspiPsv *InstancePtr,
		XOspiPsv_Msg *Msg) ;
static inline void XOspiPsv_Config_Dma(XOspiPsv *InstancePtr,
		XOspiPsv_Msg *Msg) ;
static inline void XOspiPsv_Exec_Dma(XOspiPsv *InstancePtr);
s32 XOspiPsv_DeAssertCS(XOspiPsv *InstancePtr);
s32 XOspiPsv_AssertCS(XOspiPsv *InstancePtr);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
*
* Initializes a specific XOspiPsv instance such that the driver is ready to use.
*
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	ConfigPtr is a reference to a structure containing information
*		about a specific OSPIPS device. This function initializes an
*		InstancePtr object for a specific device specified by the
*		contents of Config.
* @param	EffectiveAddr is the device base address in the virtual memory
*		address space. The caller is responsible for keeping the address
*		mapping from EffectiveAddr to the device physical base address
*		unchanged once this function is invoked. Unexpected errors may
*		occur if the address mapping changes after this function is
*		called. If address translation is not used, use
*		ConfigPtr->Config.BaseAddress for this device.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_DEVICE_IS_STARTED if the device is already started.
*		It must be stopped to re-initialize.
*
* @note		None.
*
******************************************************************************/
s32 XOspiPsv_CfgInitialize(XOspiPsv *InstancePtr, XOspiPsv_Config *ConfigPtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);
	s32 Status;

	/*
	 * If the device is busy, disallow the initialize and return a status
	 * indicating it is already started. This allows the user to stop the
	 * device and re-initialize, but prevents a user from inadvertently
	 * initializing. This assumes the busy flag is cleared at startup.
	 */
	if (InstancePtr->IsBusy == TRUE) {
		Status = (s32)XST_DEVICE_IS_STARTED;
	} else {

		/* Set some default values. */
		InstancePtr->IsBusy = FALSE;
		InstancePtr->Config.BaseAddress = ConfigPtr->BaseAddress;
		InstancePtr->Config.InputClockHz = ConfigPtr->InputClockHz;
		/* Other instance variable initializations */
		InstancePtr->SendBufferPtr = NULL;
		InstancePtr->RecvBufferPtr = NULL;
		InstancePtr->TxBytes = 0;
		InstancePtr->RxBytes = 0;
		InstancePtr->OpMode = XOSPIPSV_READMODE_DMA;

		/*
		 * Reset the OSPIPS device to get it into its initial state. It is
		 * expected that device configuration will take place after this
		 * initialization is done, but before the device is started.
		 */
		XOspiPsv_Reset(InstancePtr);

		InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
*
* Resets the OSPIPS device. Reset must only be called after the driver has
* been initialized. Any data transfer that is in progress is aborted.
*
* The Upper layer software is responsible for re-configuring (if necessary)
* and restarting the OSPIPS device after the reset.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XOspiPsv_Reset(XOspiPsv *InstancePtr)
{
	u32 ConfigReg;

	Xil_AssertVoid(InstancePtr != NULL);

	ConfigReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_CONFIG_REG);
	ConfigReg &= ~XOSPIPSV_CONFIG_REG_ENB_SPI_FLD_MASK;

	ConfigReg &= ~((u32)XOSPIPSV_CONFIG_REG_ENB_AHB_ADDR_REMAP_FLD_MASK);
	ConfigReg &= ~((u32)XOSPIPSV_CONFIG_REG_ENB_DIR_ACC_CTLR_FLD_MASK);
	ConfigReg &= ~(XOSPIPSV_CONFIG_REG_ENB_LEGACY_IP_MODE_FLD_MASK);
	ConfigReg &= ~(XOSPIPSV_CONFIG_REG_PERIPH_SEL_DEC_FLD_MASK);
	ConfigReg &= (XOSPIPSV_CONFIG_REG_PERIPH_CS_LINES_FLD_MASK);
	ConfigReg &= ~(XOSPIPSV_CONFIG_REG_MSTR_BAUD_DIV_FLD_MASK);

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress, XOSPIPSV_CONFIG_REG,
		ConfigReg);
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress, XOSPIPSV_DEV_DELAY_REG,
			XOSPIPSV_DELY_DEF_VALUE);
}

/*****************************************************************************/
/**
*
* This function asserts the chip select line.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return	None
*
* @note		None.
*
******************************************************************************/
s32 XOspiPsv_AssertCS(XOspiPsv *InstancePtr)
{
	u32 Cfg;
	u32 Cs;

	if (InstancePtr->ChipSelect > 2U) {
		return (s32)XST_FAILURE;
	}

	Cfg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress, XOSPIPSV_CONFIG_REG);
	Cfg &= ~(XOSPIPSV_CONFIG_REG_PERIPH_CS_LINES_FLD_MASK);
	/* Set Peripheral select lines */
	Cs = (~(1U << InstancePtr->ChipSelect)) & 0xFU;
	Cfg |= ((Cs) << XOSPIPSV_CONFIG_REG_PERIPH_CS_LINES_FLD_SHIFT);
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress, XOSPIPSV_CONFIG_REG, Cfg);

	Cfg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress, XOSPIPSV_CONFIG_REG);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function De-asserts the chip select line.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return	None
*
* @note		None.
*
******************************************************************************/
s32 XOspiPsv_DeAssertCS(XOspiPsv *InstancePtr)
{
	u32 Cfg;

	if (InstancePtr->ChipSelect > 2U) {
		return (s32)XST_FAILURE;
	}

	Cfg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress, XOSPIPSV_CONFIG_REG);

	/* Clear Peripheral select bit and Peripheral select lines, meaning one of
	 * CS will be used
	 */
	Cfg &= ~(XOSPIPSV_CONFIG_REG_PERIPH_CS_LINES_FLD_MASK);
	/* Set Peripheral select lines */
	Cfg |= (XOSPIPSV_CONFIG_REG_PERIPH_CS_LINES_FLD_MASK);
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress, XOSPIPSV_CONFIG_REG, Cfg);
	Cfg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress, XOSPIPSV_CONFIG_REG);

	return (s32)XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Disable the OSPIPS device.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline void XOspiPsv_Disable(XOspiPsv *InstancePtr)
{
	u32 cfg_reg;

	cfg_reg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_CONFIG_REG);

	cfg_reg &= ~(XOSPIPSV_CONFIG_REG_ENB_SPI_FLD_MASK);
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress, XOSPIPSV_CONFIG_REG,
		cfg_reg);
}

/*****************************************************************************/
/**
*
* Enable the OSPIPS device.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline void XOspiPsv_Enable(XOspiPsv *InstancePtr)
{
	u32 Cfg_reg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_CONFIG_REG);

	Cfg_reg |= XOSPIPSV_CONFIG_REG_ENB_SPI_FLD_MASK;
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_CONFIG_REG, Cfg_reg);
}

/*****************************************************************************/
/**
*
* Disable the DAC in OSPIPS device.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline void XOspiPsv_Disable_Dac(XOspiPsv *InstancePtr)
{
	u32 Cfg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_CONFIG_REG);

	Cfg &= (~(XOSPIPSV_CONFIG_REG_ENB_DIR_ACC_CTLR_FLD_MASK));
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_CONFIG_REG, Cfg);
}

/*****************************************************************************/
/**
*
* Enable the DAC in OSPIPS device.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline void XOspiPsv_Enable_Dac(XOspiPsv *InstancePtr)
{
	u32 Cfg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_CONFIG_REG);

	Cfg |= (XOSPIPSV_CONFIG_REG_ENB_DIR_ACC_CTLR_FLD_MASK);
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_CONFIG_REG, Cfg);
}

/*****************************************************************************/
/**
*
* This function performs a transfer on the bus in polled mode. The messages
* passed are all transferred on the bus between one CS assert and de-assert.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if transfer fails.
*		- XST_DEVICE_BUSY if a transfer is already in progress.
*
* @note		None.
*
******************************************************************************/
u32 XOspiPsv_PollTransfer(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
{
	u32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Check whether there is another transfer in progress. Not thread-safe */
	if (InstancePtr->IsBusy == TRUE) {
		return (u32)XST_DEVICE_BUSY;
	}

	/*
	 * Set the busy flag, which will be cleared when the transfer is
	 * entirely done.
	 */
	InstancePtr->IsBusy = TRUE;
	InstancePtr->Msg = Msg;
	XOspiPsv_Enable(InstancePtr);
	Status = XOspiPsv_Process_Read_Write(InstancePtr, Msg);
	InstancePtr->IsBusy = FALSE;

	return Status;
}

/*****************************************************************************/
/**
*
* This function configures the STIG control register (Flash cmd register)
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Cmd_op is command opcode.
* @param	Rd_data_en specifies whether cmd_op is requires data to read or not.
* @param	Num_rd_data_bytes is number of bytes to read.
* @param	Cmd_addr_en specifies whether cmd_op requires addr or not.
* @param	Mode_bit_en is used to represent mode bit configuration reg.
* @param	Num_addr_bytes is used to represent addr bytes (4/3 etc).
* @param	Wr_data_en specifies whether cmd_op requires write enable or not.
* @param	Num_wr_data_bytes is number of bytes to write.
* @param	Dummy is number of bytes to write.
* @param	Membak_en is used to enable STIG memory bank.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline void XOspiPsv_Setup_Stig_Ctrl(XOspiPsv *InstancePtr, u32 Cmd_op,
		u32 Rd_data_en,	u32 Num_rd_data_bytes, u32 Cmd_addr_en, u32 Mode_bit_en,
		u32 Num_addr_bytes, u32 Wr_data_en, u32 Num_wr_data_bytes, u32 Dummy,
		u32 Membank_en)
{
	u32 Val;

	Val =(((Cmd_op << (u32)XOSPIPSV_FLASH_CMD_CTRL_REG_CMD_OPCODE_FLD_SHIFT)
			& XOSPIPSV_FLASH_CMD_CTRL_REG_CMD_OPCODE_FLD_MASK) |
		((Rd_data_en << (u32)XOSPIPSV_FLASH_CMD_CTRL_REG_ENB_READ_DATA_FLD_SHIFT)
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
*
* This function executes the Flash command configured using Flash Command
* control register
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return	None.
*
* @note		Wait till the command executed.
*
******************************************************************************/
static inline void XOspiPsv_Exec_Flash_Cmd(XOspiPsv *InstancePtr)
{
	u32 Cmd_ctrl;

	Cmd_ctrl = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_FLASH_CMD_CTRL_REG);
	Cmd_ctrl |= (XOSPIPSV_FLASH_CMD_CTRL_REG_CMD_EXEC_FLD_MASK);
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_FLASH_CMD_CTRL_REG, Cmd_ctrl);

	Cmd_ctrl = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_FLASH_CMD_CTRL_REG);

	while ((Cmd_ctrl & XOSPIPSV_FLASH_CMD_CTRL_REG_CMD_EXEC_STATUS_FLD_MASK)
		!= 0U) {
		Cmd_ctrl = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_FLASH_CMD_CTRL_REG);
	}
}

/*****************************************************************************/
/**
*
* Flash command based data reading using flash command control registers.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if transfer fails.
*
* @note		This operation is IO mode of reading.
*
******************************************************************************/
static inline u32 XOspiPsv_Process_Stig_Read(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
{
	u32 Cmd_addr = Msg->Addr;
	u32 Reqaddr = 0;
	s32 ByteCount;
	u32 Lower;
	u32 Upper;
	u32 CopyOffset = 4;

	if (Msg->Addrvalid != 0U) {
		Reqaddr = 1;
	}
	if (InstancePtr->RxBytes <= 0) {
		return (u32)XST_FAILURE;
	}
	while (InstancePtr->RxBytes > 0) {
		if (InstancePtr->RxBytes >= 8) {
			ByteCount = 8;
		} else {
			ByteCount = InstancePtr->RxBytes;
		}
		if (Msg->Addrvalid != 0U) {
			XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_FLASH_CMD_ADDR_REG, Cmd_addr);

		}
		XOspiPsv_Setup_Stig_Ctrl(InstancePtr, (u32)Msg->Opcode,
			1, (u32)ByteCount - 1, Reqaddr, 0, (u32)Msg->Addrsize - 1,
			0, 0, (u32)Msg->Dummy, 0);

		/* Exec cmd */
		XOspiPsv_Exec_Flash_Cmd(InstancePtr);

		/* Read data */
		Lower = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_FLASH_RD_DATA_LOWER_REG);
		if(InstancePtr->RxBytes < 4)
			memcpy(Msg->RxBfrPtr, &Lower, InstancePtr->RxBytes);
		else
			memcpy(Msg->RxBfrPtr, &Lower, 4);
		if (InstancePtr->RxBytes > 4) {
			Upper = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
					XOSPIPSV_FLASH_RD_DATA_UPPER_REG);
			Msg->RxBfrPtr += 4;
			Cmd_addr += 4;
			if(InstancePtr->RxBytes < 8)
				memcpy(Msg->RxBfrPtr, &Upper, InstancePtr->RxBytes - 4);
			else
				memcpy(Msg->RxBfrPtr, &Upper, 4);
		}
		InstancePtr->RxBytes -= ByteCount;
		if (InstancePtr->RxBytes != 0) {
			Msg->RxBfrPtr += 4U;
			Cmd_addr += 4U;
		}
		while(XOspiPsv_Check_Idle(InstancePtr) == 0U);
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Flash command based data write using flash command control registers.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if transfer fails.
*
* @note		This operation is IO mode of writing.
*
******************************************************************************/
static inline u32 XOspiPsv_Process_Stig_Data_Write(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
{
	u32 Cmd_addr = Msg->Addr;
	u32 Lower = 0;
	u32 Upper = 0;
	u32 Reqaddr = 0;
	u32 Reqwridataen = 1;
	s32 ByteCount = 0;
	u32 CopyOffset = 4;

	if (Msg->Addrvalid != 0U) {
		Reqaddr = 1;
	}

	ByteCount = InstancePtr->TxBytes;
	if(InstancePtr->TxBytes < 4)
		memcpy(&Lower, Msg->TxBfrPtr, InstancePtr->TxBytes);
	else
		memcpy(&Lower, Msg->TxBfrPtr, 4);
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_FLASH_WR_DATA_LOWER_REG, Lower);

	if (InstancePtr->TxBytes > 4) {
		Msg->TxBfrPtr += 4;
		if(InstancePtr->TxBytes < 8)
			memcpy(&Upper, Msg->TxBfrPtr,InstancePtr->TxBytes - 4);
		else {
			memcpy(&Upper, Msg->TxBfrPtr,4);
		}
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_FLASH_WR_DATA_UPPER_REG, Upper);
	}
	if (Msg->Addrvalid != 0U) {
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_FLASH_CMD_ADDR_REG, Cmd_addr);
		XOspiPsv_Setup_Stig_Ctrl(InstancePtr, (u32)Msg->Opcode,
			0, 0, Reqaddr, 0, (u32)Msg->Addrsize - 1,
			Reqwridataen, (u32)ByteCount - 1,0, 0);
	}
	/* Exec cmd */
	XOspiPsv_Exec_Flash_Cmd(InstancePtr);
	while(XOspiPsv_Check_Idle(InstancePtr) == 0U);

	return XST_SUCCESS;
};

/*****************************************************************************/
/**
*
* Flash command based command write to flash using flash command control registers.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return	None
*
* @note		This operation is IO mode of writing.
*
******************************************************************************/
static inline void XOspiPsv_Process_Stig_Command_Write(XOspiPsv *InstancePtr,
		XOspiPsv_Msg *Msg)
{
	u32 Reqaddr = 0;
	u32 Reqwridataen = 0;
	u32 ByteCount = 1;

	if (Msg->Addrvalid != 0U) {
		Reqaddr = 1;
	}

	XOspiPsv_Setup_Stig_Ctrl(InstancePtr, (u32)Msg->Opcode, 0, 0,
		Reqaddr, 0, (u32)Msg->Addrsize - 1, Reqwridataen,
		(u32)ByteCount, (u32)Msg->Dummy, 0);

	if (Msg->Addrvalid != 0U) {
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_FLASH_CMD_ADDR_REG, Msg->Addr);
	}
	/* Exec cmd */
	XOspiPsv_Exec_Flash_Cmd(InstancePtr);
}

/*****************************************************************************/
/**
*
* Enable the DMA in the Controller.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline void XOspiPsv_EnableDma(XOspiPsv *InstancePtr)
{
	u32 Val;

	Val = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_CONFIG_REG);

	/* Wait for idle */
	while(XOspiPsv_Check_Idle(InstancePtr) == 0U);

	/* Disable the direct access ctrlr */
	Val &= ~(XOSPIPSV_CONFIG_REG_ENB_DIR_ACC_CTLR_FLD_MASK);

	/* Enable DMA */
	Val |= (XOSPIPSV_CONFIG_REG_ENB_DMA_IF_FLD_MASK);

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress, XOSPIPSV_CONFIG_REG,
		Val);
}

/*****************************************************************************/
/**
*
* Disable DMA in the controller.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline void XOspiPsv_DisableDma(XOspiPsv *InstancePtr)
{
	u32 Val;

	Val = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress, XOSPIPSV_CONFIG_REG);

	/* Wait for idle */
	while(XOspiPsv_Check_Idle(InstancePtr) == 0U);

	/* Enable DMA */
	Val &= ~(XOSPIPSV_CONFIG_REG_ENB_DMA_IF_FLD_MASK);

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress, XOSPIPSV_CONFIG_REG, Val);
}

/*****************************************************************************/
/**
*
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
static inline void XOspiPsv_Setup_Dev_Write_Instr_Reg(XOspiPsv *InstancePtr,
		XOspiPsv_Msg *Msg)
{
	u32 Dummy_clks = 0U;
	u32 Dataxfer_Type = DQ0;
	u32 Addrxfer_Type = DQ0;
	u32 Regval;

	switch((u32)Msg->Proto) {
		case XOSPIPSV_WRITE_1_1_1:
			Dataxfer_Type = DQ0;
			break;
		case XOSPIPSV_WRITE_1_1_2:
			Dataxfer_Type = DQ0_1;
			break;
		case XOSPIPSV_WRITE_1_1_4:
			Dataxfer_Type = DQ0_3;
			break;
		case XOSPIPSV_WRITE_1_1_8:
			Dataxfer_Type = DQ0_7;
			break;
		case XOSPIPSV_WRITE_1_8_8:
			Dataxfer_Type = DQ0_7;
			Addrxfer_Type = DQ0_7;
			break;
		default :
			Dataxfer_Type = DQ0;
			Addrxfer_Type = DQ0;
			break;
	}

	Regval = (((Dummy_clks <<
		(u32)XOSPIPSV_DEV_INSTR_WR_CONFIG_REG_DUMMY_WR_CLK_CYCLES_FLD_SHIFT)
			& XOSPIPSV_DEV_INSTR_WR_CONFIG_REG_DUMMY_WR_CLK_CYCLES_FLD_MASK) |
		((Dataxfer_Type  <<
		(u32)XOSPIPSV_DEV_INSTR_WR_CONFIG_REG_DATA_XFER_TYPE_EXT_MODE_FLD_SHIFT)
			& XOSPIPSV_DEV_INSTR_WR_CONFIG_REG_DATA_XFER_TYPE_EXT_MODE_FLD_MASK) |
		((Addrxfer_Type <<
		(u32)XOSPIPSV_DEV_INSTR_WR_CONFIG_REG_ADDR_XFER_TYPE_STD_MODE_FLD_SHIFT )
			& XOSPIPSV_DEV_INSTR_WR_CONFIG_REG_ADDR_XFER_TYPE_STD_MODE_FLD_MASK) |
		(((u32)Msg->Opcode <<
		(u32)XOSPIPSV_DEV_INSTR_WR_CONFIG_REG_WR_OPCODE_FLD_SHIFT ) &
			XOSPIPSV_DEV_INSTR_WR_CONFIG_REG_WR_OPCODE_FLD_MASK));

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress, XOSPIPSV_DEV_INSTR_WR_CONFIG_REG,
		Regval);
}

/*****************************************************************************/
/**
*
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
static inline void XOspiPsv_Setup_Dev_Read_Instr_Reg(XOspiPsv *InstancePtr,
		XOspiPsv_Msg *Msg)
{
	u32 Mode_bit_en = 0;
	u32 Dataxfer_Type = DQ0;
	u32 Addrxfer_Type = DQ0;
	u32 Instxfer_Type = DQ0;
	u32 Regval;

	switch((u32)Msg->Proto) {
		case XOSPIPSV_READ_1_1_1:
			Dataxfer_Type = DQ0;
			break;
		case XOSPIPSV_READ_1_1_2:
			Dataxfer_Type = DQ0_1;
			break;
		case XOSPIPSV_READ_1_1_4:
			Dataxfer_Type = DQ0_3;
			break;
		case XOSPIPSV_READ_1_1_8:
			Dataxfer_Type = DQ0_7;
			break;
		case XOSPIPSV_READ_1_8_8:
			Dataxfer_Type = DQ0_7;
			Addrxfer_Type = DQ0_7;
			break;
		default :
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
		((Instxfer_Type << (u32)XOSPIPSV_DEV_INSTR_RD_CONFIG_REG_INSTR_TYPE_FLD_SHIFT)
		& XOSPIPSV_DEV_INSTR_RD_CONFIG_REG_INSTR_TYPE_FLD_MASK) |
		(((u32)Msg->Opcode <<
		(u32)XOSPIPSV_DEV_INSTR_RD_CONFIG_REG_RD_OPCODE_NON_XIP_FLD_SHIFT)
		& XOSPIPSV_DEV_INSTR_RD_CONFIG_REG_RD_OPCODE_NON_XIP_FLD_MASK) );

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress, XOSPIPSV_DEV_INSTR_RD_CONFIG_REG,
		Regval);
}

/*****************************************************************************/
/**
*
* This function sets the device size config register
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return	None
*
* @note		None
*
******************************************************************************/
static inline void XOspiPsv_Setup_Devsize(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
{
	u32 Reg;

	Reg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_DEV_SIZE_CONFIG_REG);
	Reg &= ~(XOSPIPSV_DEV_SIZE_CONFIG_REG_NUM_ADDR_BYTES_FLD_MASK);
	Reg |= ((u32)Msg->Addrsize - 1);

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_DEV_SIZE_CONFIG_REG, Reg);
}

/*****************************************************************************/
/**
*
* This function initiates the indirect read transfer
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return	None
*
* @note		None
*
******************************************************************************/
static inline void XOspiPsv_Start_Indr_RdTransfer(XOspiPsv *InstancePtr)
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
*
* This function returns the indirect read transfer completion status
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return	None
*
* @note		None
*
******************************************************************************/
static inline u32 XOspiPsv_Indr_RdTransfer_Complete(XOspiPsv *InstancePtr)
{
	u32 Val;

	Val = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_OSPIDMA_DST_I_STS);
	return (Val & XOSPIPSV_OSPIDMA_DST_I_STS_DONE_MASK);
}

/*****************************************************************************/
/**
*
* This function configures the Indirect controller
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return	None
*
* @note		None
*
******************************************************************************/
static inline void XOspiPsv_Config_IndirectAhb(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
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
*
* This function setup the Dma configuration
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return	None
*
* @note		None
*
******************************************************************************/
static inline void XOspiPsv_Config_Dma(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
{
	s32 Remainder;
	s32 DmaRxBytes;
	u64 AddrTemp;

	AddrTemp = (u64)((INTPTR)(Msg->RxBfrPtr) &
			XOSPIPSV_OSPIDMA_DST_ADDR_ADDR_MASK);

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_DMA_PERIPH_CONFIG_REG, XOSPIPSV_DMA_PERIPH_CONFIG_VAL);


	Remainder = Msg->ByteCount % 4;
	DmaRxBytes = Msg->ByteCount;
	if (Remainder != 0) {
		/* This is done to make Dma bytes aligned */
		DmaRxBytes = Msg->ByteCount - Remainder;
		Msg->ByteCount = (u32)DmaRxBytes;
	}

	Xil_DCacheInvalidateRange((INTPTR)Msg->RxBfrPtr, DmaRxBytes);
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_OSPIDMA_DST_ADDR, (u32)AddrTemp);

#ifdef __aarch64__
	AddrTemp = (u64)((INTPTR)(Msg->RxBfrPtr) >> 32);
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_OSPIDMA_DST_ADDR_MSB, (u32)AddrTemp &
		XOSPIPSV_OSPIDMA_DST_ADDR_MSB_ADDR_MSB_MASK);
#endif
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_SRAM_PARTITION_CFG_REG, XOSPIPSV_SRAM_PARTITION_CFG_VAL);

	XOspiPsv_Setup_Dev_Read_Instr_Reg(InstancePtr, Msg);

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_OSPIDMA_SRC_RD_ADDR, XOSPIPSV_IND_TRIGGAHB_BASE);

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_OSPIDMA_DST_SIZE, Msg->ByteCount);

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_OSPIDMA_DST_CTRL,  XOSPIPSV_DMA_DST_CTRL_DEF);
}

/*****************************************************************************/
/**
*
* This function Initiates the DMA transfer
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return	None
*
* @note		None
*
******************************************************************************/
static inline void XOspiPsv_Exec_Dma(XOspiPsv *InstancePtr)
{
	/* Start the transfer */
	XOspiPsv_Start_Indr_RdTransfer(InstancePtr);

	/* Wait for complete */
	while(XOspiPsv_Indr_RdTransfer_Complete(InstancePtr) == 0U);
	/* Check for Idle */
	while(XOspiPsv_Check_Idle(InstancePtr) == 0U);
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_OSPIDMA_DST_I_STS,
		XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_OSPIDMA_DST_I_STS));
	while(XOspiPsv_Check_Idle(InstancePtr) == 0U);

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_INDIRECT_READ_XFER_CTRL_REG,
		(XOSPIPSV_INDIRECT_READ_XFER_CTRL_REG_IND_OPS_DONE_STATUS_FLD_MASK));
}

/*****************************************************************************/
/**
*
* This function returns the OSPI controller idle state
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return	Idle state of OSPI controller
*
* @note		None
*
******************************************************************************/
static inline u32 XOspiPsv_Check_Idle(XOspiPsv *InstancePtr)
{

	return (XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_CONFIG_REG) & XOSPIPSV_CONFIG_REG_IDLE_FLD_MASK);
}

/*****************************************************************************/
/**
*
* This function setup the OSPI Read/Write
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return	None
*
* @note		None
*
******************************************************************************/
static inline u32 XOspiPsv_Process_Read_Write(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
{

	u32 Status = (u32)XST_SUCCESS;

	if (((Msg->Flags & XOSPIPSV_MSG_FLAG_RX) != FALSE) &&
		((Msg->Flags & XOSPIPSV_MSG_FLAG_TX) == FALSE)) {
			InstancePtr->RxBytes = (s32)Msg->ByteCount;
			InstancePtr->SendBufferPtr = NULL;
			InstancePtr->RecvBufferPtr = Msg->RxBfrPtr;
			XOspiPsv_AssertCS(InstancePtr);
			if((InstancePtr->OpMode == XOSPIPSV_READMODE_IO) ||
					((u32)Msg->ByteCount <= XOSPIPSV_IOMODE_BYTECNT))
					Status = XOspiPsv_Process_Stig_Read(InstancePtr,
						 Msg);
			else if(InstancePtr->OpMode == XOSPIPSV_READMODE_DMA) {
				XOspiPsv_Setup_Devsize(InstancePtr, Msg);
				XOspiPsv_Config_IndirectAhb(InstancePtr,Msg);
				XOspiPsv_Config_Dma(InstancePtr,Msg);
				XOspiPsv_Exec_Dma(InstancePtr);
			} else {
				XOspiPsv_Dac_Configure(InstancePtr, Msg);
				while(XOspiPsv_Check_Idle(InstancePtr) == 0U);
				return XOspiPsv_Dac_Read(InstancePtr, Msg);
			}
			while(XOspiPsv_Check_Idle(InstancePtr) == 0U);
			XOspiPsv_DeAssertCS(InstancePtr);
	} else if (((Msg->Flags & XOSPIPSV_MSG_FLAG_TX) != FALSE) &&
		((Msg->Flags & XOSPIPSV_MSG_FLAG_RX) == FALSE)) {
			InstancePtr->TxBytes = (s32)Msg->ByteCount;
			InstancePtr->SendBufferPtr = Msg->TxBfrPtr;
			InstancePtr->RecvBufferPtr = NULL;
			XOspiPsv_AssertCS(InstancePtr);
			if (InstancePtr->TxBytes != 0U) {
				if(InstancePtr->OpMode == XOSPIPSV_READMODE_DAC) {
					while(XOspiPsv_Check_Idle(InstancePtr) == 0U);
					XOspiPsv_Dac_Configure(InstancePtr, Msg);
					while(XOspiPsv_Check_Idle(InstancePtr) == 0U);
					Status = XOspiPsv_Dac_Write(InstancePtr, Msg);
				} else {
					Status = XOspiPsv_Process_Stig_Data_Write(InstancePtr,
						Msg);
				}
			} else {
				XOspiPsv_Process_Stig_Command_Write(InstancePtr, Msg);
			}
			while(XOspiPsv_Check_Idle(InstancePtr) == 0U);
			XOspiPsv_DeAssertCS(InstancePtr);
		}
	return Status;
}


/*****************************************************************************/
/**
*
* This function enables Linear controller in OSPI
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return	None
*
* @note		None
*
******************************************************************************/
static inline void XOspiPsv_Dac_Configure(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
{
	/* Wait for idle */
	while (XOspiPsv_Check_Idle(InstancePtr) ==0U);

	if (Msg->Flags & XOSPIPSV_MSG_FLAG_RX) {
		XOspiPsv_Setup_Dev_Read_Instr_Reg(InstancePtr, Msg);
	} else {
		XOspiPsv_Setup_Dev_Write_Instr_Reg(InstancePtr, Msg);
	}

	XOspiPsv_Setup_Devsize(InstancePtr, Msg);
	XOspiPsv_Enable(InstancePtr);
	XOspiPsv_Enable_Dac(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function reads the data using Linear controller
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return	None
*
* @note		None
*
******************************************************************************/
static inline u32 XOspiPsv_Dac_Read(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
{
	u32 Addr = (long unsigned int)XOSPIPSV_LINEAR_ADDR_BASE + Msg->Addr;
	u32 Val;
	u32 ByteCount;

	while (InstancePtr->RxBytes > 0) {
		if (Addr >= (XOSPIPSV_LINEAR_ADDR_BASE + SIZE_512MB)) {
			xil_printf("Dac supports only 512Mb data Read\n\r");
			return XST_FAILURE;
		}
		if (InstancePtr->RxBytes >= 4)
			ByteCount = 4;
		else
			ByteCount = InstancePtr->RxBytes;

		Val = Xil_In32(Addr);
		memcpy(Msg->RxBfrPtr, &Val, ByteCount);
		InstancePtr->RxBytes -= ByteCount;
		if (InstancePtr->RxBytes) {
			Msg->RxBfrPtr += 4;
			Addr += 4;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function writes the data Using Linear controller
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return	None
*
* @note		None
*
******************************************************************************/
static inline u32 XOspiPsv_Dac_Write(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
{
	u32 Addr = (long unsigned int)XOSPIPSV_LINEAR_ADDR_BASE + Msg->Addr;
	u32 Val;
	u32 ByteCount;

	while(InstancePtr->TxBytes > 0) {
		if (Addr >= (XOSPIPSV_LINEAR_ADDR_BASE + SIZE_512MB)) {
			xil_printf("Dac supports only 512Mb data Write\n\r");
			return XST_FAILURE;
		}
		if(InstancePtr->TxBytes >= 4)
			ByteCount = 4;
		else
			ByteCount = InstancePtr->TxBytes;

		memcpy(&Val, Msg->TxBfrPtr, 4);
		Xil_Out32(Addr, Val);
		InstancePtr->TxBytes -= ByteCount;
		if (InstancePtr->TxBytes) {
			Msg->TxBfrPtr += 4;
			Addr += 4;
		}
	}

	return XST_SUCCESS;
}

/** @} */
