/******************************************************************************
*
* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xospipsv.c
* @addtogroup xospipsv_v1_1
* @{
*
* This file implements the functions required to use the OSPIPSV hardware to
* perform a transfer. These are accessible to the user via XOspiPsv.h.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.0   nsk  02/19/18 First release
*       sk   01/09/19 Added interrupt mode support.
*                     Remove STIG/DMA mode selection by the user, driver will
*                     take care of operating in DMA/STIG based on command.
*                     Added support for unaligned byte count read.
*       sk   02/04/19 Added support for SDR+PHY and DDR+PHY modes.
*       sk   02/07/19 Added OSPI Idling sequence.
* 1.1   sk   07/22/19 Added RX Tuning algorithm for SDR and DDR modes.
* 1.1   mus  07/31/19 Added CCI support at EL1 NS
*       sk   08/08/19 Added flash device reset support.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xospipsv.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static inline u32 XOspiPsv_Process_Read_Write(XOspiPsv *InstancePtr,
	XOspiPsv_Msg *Msg);
static inline void XOspiPsv_FifoRead(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg);
static inline u32 XOspiPsv_Stig_Read(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg);
static inline void XOspiPsv_Stig_Write(XOspiPsv *InstancePtr,
	XOspiPsv_Msg *Msg);
static inline void XOspiPsv_FifoWrite(XOspiPsv *InstancePtr,
		XOspiPsv_Msg *Msg);
static inline u32 XOspiPsv_Dac_Read(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg);
static inline u32 XOspiPsv_Dac_Write(XOspiPsv *InstancePtr, const XOspiPsv_Msg *Msg);
static inline void XOspiPsv_Disable(const XOspiPsv *InstancePtr);
static inline void XOspiPsv_Enable(const XOspiPsv *InstancePtr);
static inline void XOspiPsv_Setup_Stig_Ctrl(const XOspiPsv *InstancePtr, u32 Cmd_op,
	u32 Rd_data_en, u32 Num_rd_data_bytes, u32 Cmd_addr_en, u32 Mode_bit_en,
	u32 Num_addr_bytes, u32 Wr_data_en, u32 Num_wr_data_bytes, u32 Dummy,
	u32 Membank_en);
static inline void XOspiPsv_Setup_Dev_Write_Instr_Reg(const XOspiPsv *InstancePtr,
	const XOspiPsv_Msg *Msg);
static inline void XOspiPsv_Setup_Dev_Read_Instr_Reg(const XOspiPsv *InstancePtr,
	const XOspiPsv_Msg *Msg);
static inline void XOspiPsv_Setup_Devsize(const XOspiPsv *InstancePtr,
		const XOspiPsv_Msg *Msg);
static inline void XOspiPsv_Start_Indr_RdTransfer(const XOspiPsv *InstancePtr);
static inline void XOspiPsv_Config_IndirectAhb(const XOspiPsv *InstancePtr,
		const XOspiPsv_Msg *Msg);
static inline void XOspiPsv_Config_Dma(const XOspiPsv *InstancePtr,
		const XOspiPsv_Msg *Msg);
static inline void XOspiPsv_Exec_Dma(const XOspiPsv *InstancePtr);
static inline void XOspiPsv_DeAssertCS(const XOspiPsv *InstancePtr);
static inline void XOspiPsv_AssertCS(const XOspiPsv *InstancePtr);
static void StubStatusHandler(void *CallBackRef, u32 StatusEvent);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
*
* Initializes a specific XOspiPsv instance such that the driver is ready to use.
*
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	ConfigPtr is a reference to a structure containing information
*		about a specific OSPIPSV device. This function initializes an
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
u32 XOspiPsv_CfgInitialize(XOspiPsv *InstancePtr,
					const XOspiPsv_Config *ConfigPtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);
	u32 Status;

	/*
	 * If the device is busy, disallow the initialize and return a status
	 * indicating it is already started. This allows the user to stop the
	 * device and re-initialize, but prevents a user from inadvertently
	 * initializing. This assumes the busy flag is cleared at startup.
	 */
	if (InstancePtr->IsBusy == TRUE) {
		Status = (u32)XST_DEVICE_IS_STARTED;
	} else {

		/* Set some default values. */
		InstancePtr->IsBusy = FALSE;
		InstancePtr->Config.BaseAddress = ConfigPtr->BaseAddress;
		InstancePtr->Config.InputClockHz = ConfigPtr->InputClockHz;
		InstancePtr->Config.IsCacheCoherent = ConfigPtr->IsCacheCoherent;
		/* Other instance variable initializations */
		InstancePtr->SendBufferPtr = NULL;
		InstancePtr->RecvBufferPtr = NULL;
		InstancePtr->TxBytes = 0U;
		InstancePtr->RxBytes = 0U;
		InstancePtr->OpMode = XOSPIPSV_IDAC_MODE;
		InstancePtr->IsUnaligned = 0U;
		InstancePtr->StatusHandler = StubStatusHandler;
		InstancePtr->SdrDdrMode = XOSPIPSV_EDGE_MODE_SDR_NON_PHY;
		InstancePtr->DeviceIdData = 0U;
		InstancePtr->Extra_DummyCycle = 0U;

		/*
		 * Reset the OSPIPSV device to get it into its initial state. It is
		 * expected that device configuration will take place after this
		 * initialization is done, but before the device is started.
		 */
		XOspiPsv_Reset(InstancePtr);

		(void)XOspiPsv_SetSdrDdrMode(InstancePtr, InstancePtr->SdrDdrMode);

		InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

		Status = (u32)XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function reset the configuration register.
*
* The Upper layer software is responsible for re-configuring (if necessary)
* and restarting the OSPIPSV device after the reset.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XOspiPsv_Reset(const XOspiPsv *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress, XOSPIPSV_CONFIG_REG,
			XOSPIPSV_CONFIG_INIT_VALUE);

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress, XOSPIPSV_DEV_DELAY_REG,
			XOSPIPSV_DELY_DEF_VALUE);
}

/*****************************************************************************/
/**
*
* This function reset the OSPI flash device.
*
*
* @param	Type is Reset type.
*
* @return	- XST_SUCCESS if successful.
*		- XST_FAILURE for invalid Reset Type.
*
* @note		None
*
******************************************************************************/
u32 XOspiPsv_DeviceReset(u8 Type)
{
	u32 Status;

	if (Type == XOSPIPSV_HWPIN_RESET) {
#if EL1_NONSECURE
		Xil_Smc(PIN_REQUEST_SMC_FID, PMC_GPIO_NODE_12_ID, 0, 0, 0, 0, 0, 0);
		Xil_Smc(PIN_SET_CONFIG_SMC_FID, (((u64)PIN_CONFIG_SCHMITT_CMOS << 32) |
				PMC_GPIO_NODE_12_ID) , 0x1, 0, 0, 0, 0, 0);
		Xil_Smc(PIN_RELEASE_SMC_FID, PMC_GPIO_NODE_12_ID, 0, 0, 0, 0, 0, 0);
#else
		XOspiPsv_WriteReg(XPMC_BNK0_EN_RX_SCHMITT_HYST, 0,
			XOspiPsv_ReadReg(XPMC_BNK0_EN_RX_SCHMITT_HYST, 0) |
			XPMC_MIO12_MASK);
#endif
		XOspiPsv_WriteReg(XPMC_GPIO_DIRM, 0,
			XOspiPsv_ReadReg(XPMC_GPIO_DIRM, 0) | XPMC_MIO12_MASK);
		XOspiPsv_WriteReg(XPMC_GPIO_OUTEN, 0,
			XOspiPsv_ReadReg(XPMC_GPIO_OUTEN, 0) | XPMC_MIO12_MASK);
		XOspiPsv_WriteReg(XPMC_GPIO_DATA, 0,
			XOspiPsv_ReadReg(XPMC_GPIO_DATA, 0) | XPMC_MIO12_MASK);
#if EL1_NONSECURE
		Xil_Smc(PIN_REQUEST_SMC_FID, PMC_GPIO_NODE_12_ID, 0, 0, 0, 0, 0, 0);
		Xil_Smc(PIN_SET_CONFIG_SMC_FID, (((u64)PIN_CONFIG_TRI_STATE << 32) |
				PMC_GPIO_NODE_12_ID) , 0, 0, 0, 0, 0, 0);
		Xil_Smc(PIN_RELEASE_SMC_FID, PMC_GPIO_NODE_12_ID, 0, 0, 0, 0, 0, 0);
#else
		XOspiPsv_WriteReg(XPMC_IOU_MIO_TRI0, 0,
			XOspiPsv_ReadReg(XPMC_IOU_MIO_TRI0, 0) & ~XPMC_MIO12_MASK);
#endif
		usleep(1);
		XOspiPsv_WriteReg(XPMC_GPIO_DATA, 0,
			XOspiPsv_ReadReg(XPMC_GPIO_DATA, 0) & ~XPMC_MIO12_MASK);
		usleep(1);
		XOspiPsv_WriteReg(XPMC_GPIO_DATA, 0,
			XOspiPsv_ReadReg(XPMC_GPIO_DATA, 0) | XPMC_MIO12_MASK);
		usleep(1);
	} else {
		/* TODO In-band reset */
		Status = (u32)XST_FAILURE;
		goto RETURN_PATH;
	}

	Status = XST_SUCCESS;
RETURN_PATH:
	return Status;
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
static inline void XOspiPsv_AssertCS(const XOspiPsv *InstancePtr)
{
	u32 Cfg;
	u32 Cs;

	Cfg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_CONFIG_REG);
	Cfg &= ~(XOSPIPSV_CONFIG_REG_PERIPH_CS_LINES_FLD_MASK);
	/* Set Peripheral select lines */
	Cs = (~(1U << InstancePtr->ChipSelect)) & 0xFU;
	Cfg |= ((Cs) << XOSPIPSV_CONFIG_REG_PERIPH_CS_LINES_FLD_SHIFT);
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_CONFIG_REG, Cfg);
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
static inline void XOspiPsv_DeAssertCS(const XOspiPsv *InstancePtr)
{
	u32 Cfg;

	Cfg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_CONFIG_REG);

	/* Clear Peripheral select bit and Peripheral select lines, meaning one of
	 * CS will be used
	 */
	Cfg &= ~(XOSPIPSV_CONFIG_REG_PERIPH_CS_LINES_FLD_MASK);
	/* Set Peripheral select lines */
	Cfg |= (u32)(XOSPIPSV_CONFIG_REG_PERIPH_CS_LINES_FLD_MASK);
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_CONFIG_REG, Cfg);
}

/*****************************************************************************/
/**
*
* Disable the OSPIPSV device.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline void XOspiPsv_Disable(const XOspiPsv *InstancePtr)
{
	u32 cfg_reg;

	cfg_reg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_CONFIG_REG);

	cfg_reg &= ~(u32)(XOSPIPSV_CONFIG_REG_ENB_SPI_FLD_MASK);
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress, XOSPIPSV_CONFIG_REG,
		cfg_reg);
}

/*****************************************************************************/
/**
*
* Enable the OSPIPSV device.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline void XOspiPsv_Enable(const XOspiPsv *InstancePtr)
{
	u32 ConfigReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_CONFIG_REG);

	ConfigReg |= XOSPIPSV_CONFIG_REG_ENB_SPI_FLD_MASK;
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_CONFIG_REG, ConfigReg);
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
	u32 ReadReg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Check whether there is another transfer in progress. Not thread-safe */
	if (InstancePtr->IsBusy == TRUE) {
		Status = (u32)XST_DEVICE_BUSY;
		goto ERROR_PATH;
	}

	/*
	 * Set the busy flag, which will be cleared when the transfer is
	 * entirely done.
	 */
	InstancePtr->IsBusy = TRUE;
	InstancePtr->Msg = Msg;

	XOspiPsv_Enable(InstancePtr);
	XOspiPsv_AssertCS(InstancePtr);
	ReadReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_CONFIG_REG);
	while((ReadReg & XOSPIPSV_CONFIG_REG_IDLE_FLD_MASK) == 0U) {
		ReadReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
					XOSPIPSV_CONFIG_REG);
	}
	Status = XOspiPsv_Process_Read_Write(InstancePtr, Msg);
	ReadReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_CONFIG_REG);
	while((ReadReg & XOSPIPSV_CONFIG_REG_IDLE_FLD_MASK) == 0U) {
		ReadReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
					XOSPIPSV_CONFIG_REG);
	}
	XOspiPsv_DeAssertCS(InstancePtr);
	XOspiPsv_Disable(InstancePtr);

	InstancePtr->IsBusy = FALSE;

ERROR_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
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
* @param	Membak_en is used to enable STIG memory bank.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline void XOspiPsv_Setup_Stig_Ctrl(const XOspiPsv *InstancePtr,
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
static inline void XOspiPsv_Exec_Flash_Cmd(const XOspiPsv *InstancePtr)
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
* Read the data from RX FIFO
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return   None
*
* @note		This operation is IO mode of reading.
*
******************************************************************************/
static inline void XOspiPsv_FifoRead(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
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
		Xil_MemCpy(&Msg->RxBfrPtr[4], &Upper, InstancePtr->RxBytes - 4);
	}
	InstancePtr->RxBytes = 0U;
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
static inline u32 XOspiPsv_Stig_Read(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
{
	u32 Reqaddr;
	u32 Status;

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
		1, (u32)InstancePtr->RxBytes - 1, Reqaddr, 0, (u32)Msg->Addrsize - 1,
		0, 0, (u32)Msg->Dummy, 0);

	/* Execute command */
	XOspiPsv_Exec_Flash_Cmd(InstancePtr);

	XOspiPsv_FifoRead(InstancePtr, Msg);

	Status = (u32)XST_SUCCESS;

ERROR_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Write data to TX FIFO
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
static inline void XOspiPsv_FifoWrite(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
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
		Xil_MemCpy(&Upper, &Msg->TxBfrPtr[4],InstancePtr->TxBytes - 4);
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_FLASH_WR_DATA_UPPER_REG, Upper);
	}
	InstancePtr->TxBytes = 0U;
}

/*****************************************************************************/
/**
*
* Flash command based data write using flash command control registers.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return   None
*
* @note		This operation is IO mode of writing.
*
******************************************************************************/
static inline void XOspiPsv_Stig_Write(XOspiPsv *InstancePtr,
			XOspiPsv_Msg *Msg)
{
	u32 Reqaddr;
	u32 Reqwridataen;
	u32 ByteCount;

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
		0, 0, Reqaddr, 0, (u32)Msg->Addrsize - 1,
		Reqwridataen, (u32)ByteCount - 1, 0, 0);

	/* Exec cmd */
	XOspiPsv_Exec_Flash_Cmd(InstancePtr);
};

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
static inline void XOspiPsv_Setup_Dev_Write_Instr_Reg(const XOspiPsv *InstancePtr,
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
	Regval |= ((Instxfer_Type <<
					(u32)XOSPIPSV_DEV_INSTR_RD_CONFIG_REG_INSTR_TYPE_FLD_SHIFT)
			& XOSPIPSV_DEV_INSTR_RD_CONFIG_REG_INSTR_TYPE_FLD_MASK);
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
					XOSPIPSV_DEV_INSTR_RD_CONFIG_REG, Regval);
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
static inline void XOspiPsv_Setup_Dev_Read_Instr_Reg(const XOspiPsv *InstancePtr,
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
static inline void XOspiPsv_Setup_Devsize(const XOspiPsv *InstancePtr,
				const XOspiPsv_Msg *Msg)
{
	u32 Reg;

	Reg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_DEV_SIZE_CONFIG_REG);
	Reg &= ~(XOSPIPSV_DEV_SIZE_CONFIG_REG_NUM_ADDR_BYTES_FLD_MASK);
	if (Msg->Addrsize != 0U) {
		Reg |= ((u32)Msg->Addrsize - 1);
	}

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
static inline void XOspiPsv_Start_Indr_RdTransfer(const XOspiPsv *InstancePtr)
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
static inline void XOspiPsv_Config_IndirectAhb(const XOspiPsv *InstancePtr,
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
*
* This function Read the data using DMA
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return	None
*
* @note		None
*
******************************************************************************/
static inline void XOspiPsv_Dma_Read(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
{
	if ((Msg->ByteCount % 4U) != 0U) {
		InstancePtr->IsUnaligned = 1;
	}

	if (Msg->ByteCount >= (u32)4) {
		Msg->ByteCount -= (Msg->ByteCount % 4U);
		XOspiPsv_Config_Dma(InstancePtr,Msg);
		XOspiPsv_Config_IndirectAhb(InstancePtr,Msg);
		XOspiPsv_Exec_Dma(InstancePtr);
		if (InstancePtr->Config.IsCacheCoherent == 0) {
			Xil_DCacheInvalidateRange((UINTPTR)Msg->RxBfrPtr, Msg->ByteCount);
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
		XOspiPsv_Exec_Dma(InstancePtr);
		if (InstancePtr->Config.IsCacheCoherent == 0) {
			Xil_DCacheInvalidateRange((UINTPTR)Msg->RxBfrPtr, Msg->ByteCount);
		}
		Xil_MemCpy(InstancePtr->RecvBufferPtr, InstancePtr->UnalignReadBuffer,
				InstancePtr->RxBytes);
		InstancePtr->IsUnaligned = 0U;
	}
}
/*****************************************************************************/
/**
*
* This function setup the Dma configuration
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return	XST_SUCCESS
*           XST_FAILURE if byte count is not aligned
*
* @note		None
*
******************************************************************************/
static inline void XOspiPsv_Config_Dma(const XOspiPsv *InstancePtr,
		const XOspiPsv_Msg *Msg)
{
	UINTPTR AddrTemp;

	AddrTemp = ((UINTPTR)(Msg->RxBfrPtr) &
			XOSPIPSV_OSPIDMA_DST_ADDR_ADDR_MASK);

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_DMA_PERIPH_CONFIG_REG, XOSPIPSV_DMA_PERIPH_CONFIG_VAL);

	if (InstancePtr->Config.IsCacheCoherent == 0) {
		Xil_DCacheInvalidateRange((UINTPTR)Msg->RxBfrPtr, Msg->ByteCount);
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
static inline void XOspiPsv_Exec_Dma(const XOspiPsv *InstancePtr)
{
	u32 ReadReg;

	/* Start the transfer */
	XOspiPsv_Start_Indr_RdTransfer(InstancePtr);

	ReadReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_OSPIDMA_DST_I_STS);
	/* Wait for complete */
	while((ReadReg & XOSPIPSV_OSPIDMA_DST_I_STS_DONE_MASK) == 0U) {
		ReadReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
					XOSPIPSV_OSPIDMA_DST_I_STS);
	}

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_OSPIDMA_DST_I_STS,
		XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_OSPIDMA_DST_I_STS));

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_INDIRECT_READ_XFER_CTRL_REG,
		(XOSPIPSV_INDIRECT_READ_XFER_CTRL_REG_IND_OPS_DONE_STATUS_FLD_MASK));
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
static inline u32 XOspiPsv_Process_Read_Write(XOspiPsv *InstancePtr,
		XOspiPsv_Msg *Msg)
{
	u32 Status;

	if ((Msg->Flags != XOSPIPSV_MSG_FLAG_RX) &&
			(Msg->Flags != XOSPIPSV_MSG_FLAG_TX)) {
		Status = XST_FAILURE;
		goto ERROR_PATH;
	}

	XOspiPsv_Setup_Devsize(InstancePtr, Msg);
	if ((Msg->Flags & XOSPIPSV_MSG_FLAG_RX) != FALSE) {
		XOspiPsv_Setup_Dev_Read_Instr_Reg(InstancePtr, Msg);
		InstancePtr->RxBytes = Msg->ByteCount;
		InstancePtr->SendBufferPtr = NULL;
		InstancePtr->RecvBufferPtr = Msg->RxBfrPtr;
		if ((InstancePtr->OpMode == XOSPIPSV_IDAC_MODE) ||
					(Msg->Addrvalid == 0U)) {
			if (Msg->Addrvalid == 0U) {
				Status = XOspiPsv_Stig_Read(InstancePtr, Msg);
			} else {
				XOspiPsv_Dma_Read(InstancePtr,Msg);
				Status = (u32)XST_SUCCESS;
			}
		} else {
			Status = XOspiPsv_Dac_Read(InstancePtr, Msg);
		}
	} else {
		XOspiPsv_Setup_Dev_Write_Instr_Reg(InstancePtr, Msg);
		InstancePtr->TxBytes = Msg->ByteCount;
		InstancePtr->SendBufferPtr = Msg->TxBfrPtr;
		InstancePtr->RecvBufferPtr = NULL;
		if((InstancePtr->OpMode == XOSPIPSV_DAC_MODE) &&
				(InstancePtr->TxBytes != 0U)) {
			Status = XOspiPsv_Dac_Write(InstancePtr, Msg);
		} else {
			XOspiPsv_Stig_Write(InstancePtr, Msg);
			Status = (u32)XST_SUCCESS;
		}
	}

ERROR_PATH:
	return Status;
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
	u32 Status;
	u32 *Addr = (u32 *)XOSPIPSV_LINEAR_ADDR_BASE + Msg->Addr;

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
static inline u32 XOspiPsv_Dac_Write(XOspiPsv *InstancePtr, const XOspiPsv_Msg *Msg)
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
*
* This function performs a transfer on the bus in interrupt mode.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_DEVICE_BUSY if a transfer is already in progress.
*
* @note		None.
*
******************************************************************************/
u32 XOspiPsv_IntrTransfer(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
{
	u32 Status;
	u32 IntrMask;
	u32 ByteCount;
	u8 WriteDataEn;
	u32 Reqaddr;
	u32 ReadReg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Check whether there is another transfer in progress. Not thread-safe */
	if (InstancePtr->IsBusy == TRUE) {
		Status = XST_DEVICE_BUSY;
		goto ERROR_PATH;
	}

	/* DAC read/write not supported in interrupt mode */
	if (InstancePtr->OpMode == XOSPIPSV_DAC_MODE) {
		Status = XST_FAILURE;
		goto ERROR_PATH;
	}

	if ((Msg->Flags != XOSPIPSV_MSG_FLAG_RX) &&
			(Msg->Flags != XOSPIPSV_MSG_FLAG_TX)) {
		Status = XST_FAILURE;
		goto ERROR_PATH;
	}

	/*
	 * Set the busy flag, which will be cleared when the transfer is
	 * entirely done.
	 */
	InstancePtr->IsBusy = TRUE;
	InstancePtr->Msg = Msg;

	XOspiPsv_Enable(InstancePtr);

	XOspiPsv_AssertCS(InstancePtr);

	ReadReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_CONFIG_REG);
	while((ReadReg & XOSPIPSV_CONFIG_REG_IDLE_FLD_MASK) == 0U) {
		ReadReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_CONFIG_REG);
	}

	IntrMask = ((u32)XOSPIPSV_IRQ_MASK_REG_STIG_REQ_MASK_FLD_MASK |
			(u32)XOSPIPSV_IRQ_MASK_REG_TX_CRC_CHUNK_BRK_MASK_FLD_MASK |
			(u32)XOSPIPSV_IRQ_MASK_REG_RX_CRC_DATA_VAL_MASK_FLD_MASK |
			(u32)XOSPIPSV_IRQ_MASK_REG_RX_CRC_DATA_ERR_MASK_FLD_MASK |
			(u32)XOSPIPSV_IRQ_MASK_REG_INDIRECT_XFER_LEVEL_BREACH_MASK_FLD_MASK |
			(u32)XOSPIPSV_IRQ_MASK_REG_ILLEGAL_ACCESS_DET_MASK_FLD_MASK |
			(u32)XOSPIPSV_IRQ_MASK_REG_PROT_WR_ATTEMPT_MASK_FLD_MASK |
			(u32)XOSPIPSV_IRQ_MASK_REG_INDIRECT_TRANSFER_REJECT_MASK_FLD_MASK);

	XOspiPsv_Setup_Devsize(InstancePtr, Msg);

	if ((Msg->Flags & XOSPIPSV_MSG_FLAG_RX) != 0U) {
		XOspiPsv_Setup_Dev_Read_Instr_Reg(InstancePtr, Msg);
		InstancePtr->RxBytes = Msg->ByteCount;
		InstancePtr->RecvBufferPtr = Msg->RxBfrPtr;
		InstancePtr->SendBufferPtr = NULL;
		if (Msg->Addrvalid == 0U) {
			XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
					XOSPIPSV_IRQ_MASK_REG, IntrMask);
			XOspiPsv_Setup_Stig_Ctrl(InstancePtr, (u32)Msg->Opcode,
				1, (u32)InstancePtr->RxBytes - 1, Msg->Addrvalid, 0,
				(u32)Msg->Addrsize - 1, 0, 0, (u32)Msg->Dummy, 0);
			/* Execute the command */
			XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_FLASH_CMD_CTRL_REG,
				(XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
					XOSPIPSV_FLASH_CMD_CTRL_REG) |
					(u32)XOSPIPSV_FLASH_CMD_CTRL_REG_CMD_EXEC_FLD_MASK));
		} else {
			/* Enable DMA DONE interrupt */
			XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_OSPIDMA_DST_I_EN, XOSPIPSV_OSPIDMA_DST_I_EN_DONE_MASK);

			if (Msg->ByteCount >= (u32)4) {
				if ((Msg->ByteCount % 4U) != 0U) {
					InstancePtr->IsUnaligned = 1U;
				}
				Msg->ByteCount -= (Msg->ByteCount % 4U);
			} else {
				Msg->ByteCount = 4;
				Msg->RxBfrPtr = InstancePtr->UnalignReadBuffer;
				InstancePtr->IsUnaligned = 0U;
			}
			XOspiPsv_Config_Dma(InstancePtr,Msg);
			XOspiPsv_Config_IndirectAhb(InstancePtr,Msg);
			/* Start the transfer */
			XOspiPsv_Start_Indr_RdTransfer(InstancePtr);
		}
	} else {
		XOspiPsv_Setup_Dev_Write_Instr_Reg(InstancePtr, Msg);
		InstancePtr->TxBytes = Msg->ByteCount;
		InstancePtr->SendBufferPtr = Msg->TxBfrPtr;
		InstancePtr->RecvBufferPtr = NULL;
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_IRQ_MASK_REG, IntrMask);
		if (Msg->Addrvalid != 0U) {
			XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_FLASH_CMD_ADDR_REG, Msg->Addr);
			Reqaddr = 1;
		} else {
			Reqaddr = 0U;
		}
		if (InstancePtr->TxBytes != 0U) {
			WriteDataEn = 1;
			ByteCount = InstancePtr->TxBytes;
			XOspiPsv_FifoWrite(InstancePtr, Msg);
		} else {
			WriteDataEn = 0U;
			ByteCount = 1;
		}
		XOspiPsv_Setup_Stig_Ctrl(InstancePtr, (u32)Msg->Opcode,
			0, 0, Reqaddr, 0, (u32)Msg->Addrsize - 1,
			WriteDataEn, (u32)ByteCount - 1, 0, 0);

		/* Execute the command */
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_FLASH_CMD_CTRL_REG,
			(XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_FLASH_CMD_CTRL_REG) |
				(u32)XOSPIPSV_FLASH_CMD_CTRL_REG_CMD_EXEC_FLD_MASK));
	}

	Status = (u32)XST_SUCCESS;
ERROR_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This function handles interrupt based transfers.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_DEVICE_BUSY if a transfer is already in progress.
*
* @note		None.
*
******************************************************************************/
u32 XOspiPsv_IntrHandler(XOspiPsv *InstancePtr)
{
	u32 StatusReg;
	u32 DmaStatusReg;
	XOspiPsv_Msg *Msg;
	u32 IntrMask;
	u32 ReadReg;

	Xil_AssertNonvoid(InstancePtr != NULL);

	Msg = InstancePtr->Msg;

	if (((Msg->Flags & XOSPIPSV_MSG_FLAG_RX) != 0U) &&
					(Msg->Addrvalid != 0U)) {
		DmaStatusReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_OSPIDMA_DST_I_STS);
		if ((DmaStatusReg & XOSPIPSV_OSPIDMA_DST_I_EN_DONE_MASK) != 0U) {
			XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_INDIRECT_READ_XFER_CTRL_REG,
				(XOSPIPSV_INDIRECT_READ_XFER_CTRL_REG_IND_OPS_DONE_STATUS_FLD_MASK));
			if (InstancePtr->Config.IsCacheCoherent == 0) {
				Xil_DCacheInvalidateRange((UINTPTR)Msg->RxBfrPtr, Msg->ByteCount);
			}
			/* Clear the ISR */
			XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_OSPIDMA_DST_I_STS, XOSPIPSV_OSPIDMA_DST_I_EN_DONE_MASK);
			if (InstancePtr->IsUnaligned != 0U) {
				InstancePtr->RecvBufferPtr += Msg->ByteCount;
				Msg->Addr += Msg->ByteCount;
				Msg->ByteCount = 4;
				InstancePtr->RxBytes = (InstancePtr->RxBytes % 4U);
				Msg->RxBfrPtr = InstancePtr->UnalignReadBuffer;
				XOspiPsv_Config_Dma(InstancePtr,Msg);
				XOspiPsv_Config_IndirectAhb(InstancePtr,Msg);
				/* Start the transfer */
				XOspiPsv_Start_Indr_RdTransfer(InstancePtr);
				InstancePtr->IsUnaligned = 0U;
			} else {
				if (Msg->RxBfrPtr == InstancePtr->UnalignReadBuffer) {
					Xil_MemCpy(InstancePtr->RecvBufferPtr,
						InstancePtr->UnalignReadBuffer, InstancePtr->RxBytes);
				}
				/* Disable the interrupt */
				XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
					XOSPIPSV_OSPIDMA_DST_I_DIS, XOSPIPSV_OSPIDMA_DST_I_EN_DONE_MASK);
				InstancePtr->RxBytes = 0U;
				InstancePtr->StatusHandler(InstancePtr->StatusRef,
						XST_SPI_TRANSFER_DONE);
				XOspiPsv_DeAssertCS(InstancePtr);
				XOspiPsv_Disable(InstancePtr);
				InstancePtr->IsBusy = FALSE;
			}
		}
	} else {
		StatusReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
					XOSPIPSV_IRQ_STATUS_REG);
		if (((Msg->Flags & XOSPIPSV_MSG_FLAG_RX) != 0U) &&
					(Msg->Addrvalid == 0U)) {
			if ((StatusReg & XOSPIPSV_IRQ_MASK_REG_STIG_REQ_MASK_FLD_MASK) != 0U) {
				/* Read the data from FIFO */
				XOspiPsv_FifoRead(InstancePtr, Msg);
				StatusReg &= ~(u32)XOSPIPSV_IRQ_MASK_REG_STIG_REQ_MASK_FLD_MASK;
				StatusReg |= (u32)XST_SPI_TRANSFER_DONE;
			}
		} else {
			if (((Msg->Flags & XOSPIPSV_MSG_FLAG_TX) != 0U) &&
				((StatusReg & XOSPIPSV_IRQ_MASK_REG_STIG_REQ_MASK_FLD_MASK) != 0U)) {
				StatusReg &= ~(u32)XOSPIPSV_IRQ_MASK_REG_STIG_REQ_MASK_FLD_MASK;
				StatusReg |= (u32)XST_SPI_TRANSFER_DONE;

			}
		}

		/* Clear the interrupts */
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_IRQ_STATUS_REG,
			XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_IRQ_STATUS_REG));
		IntrMask = ((u32)XOSPIPSV_IRQ_MASK_REG_STIG_REQ_MASK_FLD_MASK |
			(u32)XOSPIPSV_IRQ_MASK_REG_TX_CRC_CHUNK_BRK_MASK_FLD_MASK |
			(u32)XOSPIPSV_IRQ_MASK_REG_RX_CRC_DATA_VAL_MASK_FLD_MASK |
			(u32)XOSPIPSV_IRQ_MASK_REG_RX_CRC_DATA_ERR_MASK_FLD_MASK |
			(u32)XOSPIPSV_IRQ_MASK_REG_INDIRECT_XFER_LEVEL_BREACH_MASK_FLD_MASK |
			(u32)XOSPIPSV_IRQ_MASK_REG_ILLEGAL_ACCESS_DET_MASK_FLD_MASK |
			(u32)XOSPIPSV_IRQ_MASK_REG_PROT_WR_ATTEMPT_MASK_FLD_MASK |
			(u32)XOSPIPSV_IRQ_MASK_REG_INDIRECT_TRANSFER_REJECT_MASK_FLD_MASK);
		 /* Disable the interrupts */
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_IRQ_MASK_REG,
			XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_IRQ_MASK_REG) & ~IntrMask);
		/* Wait for Idle */
		ReadReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_CONFIG_REG);
		while((ReadReg & XOSPIPSV_CONFIG_REG_IDLE_FLD_MASK) == 0U) {
			ReadReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
					XOSPIPSV_CONFIG_REG);
		}
		InstancePtr->StatusHandler(InstancePtr->StatusRef, StatusReg);
		XOspiPsv_DeAssertCS(InstancePtr);
		XOspiPsv_Disable(InstancePtr);
		InstancePtr->IsBusy = FALSE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * Stops the transfer of data to internal DST FIFO from stream interface and
 * also stops the issuing of new write commands to memory.
 *
 * By calling this API, any ongoing Dma transfers will be paused and DMA will
 * not issue AXI write commands to memory
 *
 * @param	InstancePtr is a pointer to the XOspiPsv instance.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void XOspiPsv_Idle(const XOspiPsv *InstancePtr)
{
	u32 ReadReg;
	u32 DmaStatus;

	Xil_AssertVoid(InstancePtr != NULL);

	/* Check for OSPI enable */
	ReadReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_CONFIG_REG);
	if ((ReadReg & XOSPIPSV_CONFIG_REG_ENB_SPI_FLD_MASK) != 0U) {
		DmaStatus = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_OSPIDMA_DST_CTRL);
		DmaStatus |= XOSPIPSV_OSPIDMA_DST_CTRL_PAUSE_STRM_MASK;
		DmaStatus |= XOSPIPSV_OSPIDMA_DST_CTRL_PAUSE_MEM_MASK;
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_OSPIDMA_DST_CTRL, DmaStatus);
	}
}

/*****************************************************************************/
/**
 *
 * Sets the status callback function, the status handler, which the driver
 * calls when it encounters conditions that should be reported to upper
 * layer software. The handler executes in an interrupt context, so it must
 * minimize the amount of processing performed. One of the following status
 * events is passed to the status handler.
 *
 * <pre>
 *
 * XST_SPI_TRANSFER_DONE		The requested data transfer is done
 *
 * </pre>
 * @param	InstancePtr is a pointer to the XOspiPsv instance.
 * @param	CallBackRef is the upper layer callback reference passed back
 *		when the callback function is invoked.
 * @param	FuncPointer is the pointer to the callback function.
 *
 * @return	None.
 *
 * @note
 *
 * The handler is called within interrupt context, so it should do its work
 * quickly and queue potentially time-consuming work to a task-level thread.
 *
 ******************************************************************************/
void XOspiPsv_SetStatusHandler(XOspiPsv *InstancePtr, void *CallBackRef,
				XOspiPsv_StatusHandler FuncPointer)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(FuncPointer != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->StatusHandler = FuncPointer;
	InstancePtr->StatusRef = CallBackRef;
}

/*****************************************************************************/
/**
 *
 * This is a stub for the status callback. The stub is here in case the upper
 * layers forget to set the handler.
 *
 * @param	CallBackRef is a pointer to the upper layer callback reference
 * @param	StatusEvent is the event that just occurred.
 * @param	ByteCount is the number of bytes transferred up until the event
 *		occurred.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
static void StubStatusHandler(void *CallBackRef, u32 StatusEvent)
{
	(void) CallBackRef;
	(void) StatusEvent;

	Xil_AssertVoidAlways();
}

/** @} */
