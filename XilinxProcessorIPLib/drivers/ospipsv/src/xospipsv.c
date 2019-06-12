/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xospipsv.c
* @addtogroup ospipsv_v1_2
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
* 1.2   sk   02/03/20 Added APIs for non-blocking transfer support.
*       sk   02/20/20 Reorganize the source code, enable the interrupts
*                     by default and updated XOspiPsv_DeviceReset() API with
*                     masked data writes.
*       sk   02/20/20 Make XOspiPsv_SetDllDelay() API as user API.
*       sk   02/20/20 Added support for DLL Master mode.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xospipsv.h"
#include "xospipsv_control.h"
#include "sleep.h"
#include "xplatform_info.h"

/************************** Constant Definitions *****************************/
#define SILICON_VERSION_1	0x10U
#define XOSPIPSV_TAP_GRAN_SEL_MIN_FREQ	120000000U
#define READ_ID		0x9FU

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static inline void XOspiPsv_AssertCS(const XOspiPsv *InstancePtr);
static inline void XOspiPsv_DeAssertCS(const XOspiPsv *InstancePtr);
static inline void StubStatusHandler(void *CallBackRef, u32 StatusEvent);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* @brief
* Initializes a specific XOspiPsv instance such that the driver is ready to use.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	ConfigPtr is a reference to a structure containing information
*		about a specific OSPIPSV device. This function initializes an
*		InstancePtr object for a specific device specified by the
*		contents of Config.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_DEVICE_IS_STARTED if the device is already started.
*		It must be stopped to re-initialize.
*
******************************************************************************/
u32 XOspiPsv_CfgInitialize(XOspiPsv *InstancePtr,
					const XOspiPsv_Config *ConfigPtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);
	u32 Status;
	u32 IntrMask;

	/*
	 * If the device is busy, disallow the initialize and return a status
	 * indicating it is already started. This allows the user to stop the
	 * device and re-initialize, but prevents a user from inadvertently
	 * initializing. This assumes the busy flag is cleared at startup.
	 */
	if (InstancePtr->IsBusy == (u32)TRUE) {
		Status = (u32)XST_DEVICE_IS_STARTED;
	} else {

		/* Set some default values. */
		InstancePtr->IsBusy = (u32)FALSE;
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
		InstancePtr->DllMode = XOSPIPSV_DLL_BYPASS_MODE;

		if (XGetPSVersion_Info() != SILICON_VERSION_1) {
			InstancePtr->DllMode = XOSPIPSV_DLL_MASTER_MODE;
			if (InstancePtr->Config.InputClockHz >=
							XOSPIPSV_TAP_GRAN_SEL_MIN_FREQ) {
				XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
						XOSPIPSV_ECO_REG, 0x1);
			}
		}

		/*
		 * Reset the OSPIPSV device to get it into its initial state. It is
		 * expected that device configuration will take place after this
		 * initialization is done, but before the device is started.
		 */
		XOspiPsv_Reset(InstancePtr);

		(void)XOspiPsv_SetSdrDdrMode(InstancePtr, InstancePtr->SdrDdrMode);

		/* Enable the interrupts */
		IntrMask = ((u32)XOSPIPSV_IRQ_MASK_REG_STIG_REQ_MASK_FLD_MASK |
					(u32)XOSPIPSV_IRQ_MASK_REG_TX_CRC_CHUNK_BRK_MASK_FLD_MASK |
					(u32)XOSPIPSV_IRQ_MASK_REG_RX_CRC_DATA_VAL_MASK_FLD_MASK |
					(u32)XOSPIPSV_IRQ_MASK_REG_RX_CRC_DATA_ERR_MASK_FLD_MASK |
					(u32)XOSPIPSV_IRQ_MASK_REG_INDIRECT_XFER_LEVEL_BREACH_MASK_FLD_MASK |
					(u32)XOSPIPSV_IRQ_MASK_REG_ILLEGAL_ACCESS_DET_MASK_FLD_MASK |
					(u32)XOSPIPSV_IRQ_MASK_REG_PROT_WR_ATTEMPT_MASK_FLD_MASK |
					(u32)XOSPIPSV_IRQ_MASK_REG_INDIRECT_TRANSFER_REJECT_MASK_FLD_MASK);
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
						XOSPIPSV_IRQ_MASK_REG, IntrMask);

		/* Enable DMA DONE interrupt */
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_OSPIDMA_DST_I_EN, XOSPIPSV_OSPIDMA_DST_I_EN_DONE_MASK);

		XOspiPsv_Enable(InstancePtr);

		InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

		Status = (u32)XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function reset the configuration register.
*
* The Upper layer software is responsible for re-configuring (if necessary)
* and restarting the OSPIPSV device after the reset.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return	None.
*
******************************************************************************/
void XOspiPsv_Reset(XOspiPsv *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->IsBusy = (u32)FALSE;
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress, XOSPIPSV_CONFIG_REG,
			XOSPIPSV_CONFIG_INIT_VALUE);

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress, XOSPIPSV_DEV_DELAY_REG,
			XOSPIPSV_DELY_DEF_VALUE);
}

/*****************************************************************************/
/**
* @brief
* This function reset the OSPI flash device.
*
* @param	Type is Reset type.
*
* @return
* 		- XST_SUCCESS if successful.
*		- XST_FAILURE for invalid Reset Type.
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
			(u32)XPMC_MIO12_MASK);
#endif
		XOspiPsv_WriteReg(XPMC_GPIO_DIRM, 0,
			XOspiPsv_ReadReg(XPMC_GPIO_DIRM, 0) | (u32)XPMC_MIO12_MASK);
		XOspiPsv_WriteReg(XPMC_GPIO_OUTEN, 0,
			XOspiPsv_ReadReg(XPMC_GPIO_OUTEN, 0) | (u32)XPMC_MIO12_MASK);
		XOspiPsv_WriteReg(XPMC_GPIO_DATA, 0,
			((u32)XPMC_MIO12_DATA_MASK_LSW << XPMC_MIO12_DATA_MASK_LSW_SHIFT) |
			(u32)XPMC_MIO12_MASK);
#if EL1_NONSECURE
		Xil_Smc(PIN_REQUEST_SMC_FID, PMC_GPIO_NODE_12_ID, 0, 0, 0, 0, 0, 0);
		Xil_Smc(PIN_SET_CONFIG_SMC_FID, (((u64)PIN_CONFIG_TRI_STATE << 32) |
				PMC_GPIO_NODE_12_ID) , 0, 0, 0, 0, 0, 0);
		Xil_Smc(PIN_RELEASE_SMC_FID, PMC_GPIO_NODE_12_ID, 0, 0, 0, 0, 0, 0);
#else
		XOspiPsv_WriteReg(XPMC_IOU_MIO_TRI0, 0,
			XOspiPsv_ReadReg(XPMC_IOU_MIO_TRI0, 0) & ~(u32)XPMC_MIO12_MASK);
#endif
		usleep(1);
		XOspiPsv_WriteReg(XPMC_GPIO_DATA, 0,
				((u32)XPMC_MIO12_DATA_MASK_LSW << XPMC_MIO12_DATA_MASK_LSW_SHIFT));
		usleep(1);
		XOspiPsv_WriteReg(XPMC_GPIO_DATA, 0,
			((u32)XPMC_MIO12_DATA_MASK_LSW << XPMC_MIO12_DATA_MASK_LSW_SHIFT) |
			(u32)XPMC_MIO12_MASK);
		usleep(1);
	} else {
		/* TODO In-band reset */
		Status = (u32)XST_FAILURE;
		goto RETURN_PATH;
	}

	Status = (u32)XST_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function asserts the chip select line.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return	None
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
	Cs = (~((u32)1U << (u32)InstancePtr->ChipSelect)) & (u32)0xFU;
	Cfg |= ((Cs) << XOSPIPSV_CONFIG_REG_PERIPH_CS_LINES_FLD_SHIFT);
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_CONFIG_REG, Cfg);
}

/*****************************************************************************/
/**
* @brief
* This function De-asserts the chip select line.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return	None
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
* @brief
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
******************************************************************************/
u32 XOspiPsv_PollTransfer(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
{
	u32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Msg != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Check whether there is another transfer in progress. Not thread-safe */
	if (InstancePtr->IsBusy == (u32)TRUE) {
		Status = (u32)XST_DEVICE_BUSY;
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
	InstancePtr->IsBusy = (u32)TRUE;
	InstancePtr->Msg = Msg;

	XOspiPsv_AssertCS(InstancePtr);

	Status = XOspiPsv_CheckOspiIdle(InstancePtr);
	if (Status != (u32)XST_SUCCESS) {
		XOspiPsv_DeAssertCS(InstancePtr);
		goto ERROR_PATH;
	}

	XOspiPsv_Setup_Devsize(InstancePtr, Msg);
	if ((Msg->Flags & XOSPIPSV_MSG_FLAG_RX) != (u32)FALSE) {
		XOspiPsv_Setup_Dev_Read_Instr_Reg(InstancePtr, Msg);
		InstancePtr->RxBytes = Msg->ByteCount;
		InstancePtr->SendBufferPtr = NULL;
		InstancePtr->RecvBufferPtr = Msg->RxBfrPtr;
		if ((InstancePtr->OpMode == XOSPIPSV_IDAC_MODE) ||
					(Msg->Addrvalid == 0U)) {
			if (Msg->Addrvalid == 0U) {
				Status = XOspiPsv_Stig_Read(InstancePtr, Msg);
			} else {
				Status = XOspiPsv_Dma_Read(InstancePtr,Msg);
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
			Status = XOspiPsv_Stig_Write(InstancePtr, Msg);
		}
	}

	if (Status != (u32)XST_SUCCESS) {
		(void)XOspiPsv_CheckOspiIdle(InstancePtr);
	} else {
		Status = XOspiPsv_CheckOspiIdle(InstancePtr);
	}

	XOspiPsv_DeAssertCS(InstancePtr);

	InstancePtr->IsBusy = (u32)FALSE;

ERROR_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function start a DMA transfer.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if transfer fails.
*		- XST_DEVICE_BUSY if a transfer is already in progress.
*
******************************************************************************/
u32 XOspiPsv_StartDmaTransfer(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
{
	u32 Status;
	u32 ReadReg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Msg != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Check whether there is another transfer in progress */
	if (InstancePtr->IsBusy == (u32)TRUE) {
		Status = (u32)XST_DEVICE_BUSY;
		goto ERROR_PATH;
	}

	if ((Msg->Flags != XOSPIPSV_MSG_FLAG_RX) ||
			(InstancePtr->OpMode != XOSPIPSV_IDAC_MODE)) {
		Status = XST_FAILURE;
		goto ERROR_PATH;
	}

	/*
	 * Set the busy flag, which will be cleared when the transfer is
	 * entirely done.
	 */
	InstancePtr->IsBusy = (u32)TRUE;
	InstancePtr->Msg = Msg;

	XOspiPsv_AssertCS(InstancePtr);

	Status = XOspiPsv_CheckOspiIdle(InstancePtr);
	if (Status != (u32)XST_SUCCESS) {
		XOspiPsv_DeAssertCS(InstancePtr);
		goto ERROR_PATH;
	}

	XOspiPsv_Setup_Devsize(InstancePtr, Msg);
	XOspiPsv_Setup_Dev_Read_Instr_Reg(InstancePtr, Msg);

	InstancePtr->RxBytes = Msg->ByteCount;
	InstancePtr->SendBufferPtr = NULL;
	InstancePtr->RecvBufferPtr = Msg->RxBfrPtr;

	XOspiPsv_Config_Dma(InstancePtr,Msg);
	XOspiPsv_Config_IndirectAhb(InstancePtr,Msg);

	/* Start the transfer */
	ReadReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_INDIRECT_READ_XFER_CTRL_REG);
	ReadReg |= (XOSPIPSV_INDIRECT_READ_XFER_CTRL_REG_START_FLD_MASK);
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_INDIRECT_READ_XFER_CTRL_REG, (ReadReg));

	Status = (u32)XST_SUCCESS;
ERROR_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function check for DMA transfer complete.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return
*		- XST_SUCCESS if DMA transfer complete.
*		- XST_FAILURE if DMA transfer is not completed.
*
******************************************************************************/
u32 XOspiPsv_CheckDmaDone(XOspiPsv *InstancePtr)
{
	u32 Status;
	u32 ReadReg;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ReadReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_OSPIDMA_DST_I_STS);
	if ((ReadReg & XOSPIPSV_OSPIDMA_DST_I_STS_DONE_MASK) == 0U) {
		Status = (u32)XST_FAILURE;
		goto ERROR_PATH;
	}

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_OSPIDMA_DST_I_STS,
		XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_OSPIDMA_DST_I_STS));
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_INDIRECT_READ_XFER_CTRL_REG,
		(XOSPIPSV_INDIRECT_READ_XFER_CTRL_REG_IND_OPS_DONE_STATUS_FLD_MASK));

	Status = XOspiPsv_CheckOspiIdle(InstancePtr);

	XOspiPsv_DeAssertCS(InstancePtr);

	InstancePtr->IsBusy = (u32)FALSE;

ERROR_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function performs a transfer on the bus in interrupt mode.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Msg is a pointer to the structure containing transfer data.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_DEVICE_BUSY if a transfer is already in progress.
*
******************************************************************************/
u32 XOspiPsv_IntrTransfer(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg)
{
	u32 Status;
	u32 ByteCount;
	u8 WriteDataEn;
	u32 Reqaddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Msg != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Check whether there is another transfer in progress. Not thread-safe */
	if (InstancePtr->IsBusy == (u32)TRUE) {
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
	InstancePtr->IsBusy = (u32)TRUE;
	InstancePtr->Msg = Msg;

	XOspiPsv_AssertCS(InstancePtr);

	Status = XOspiPsv_CheckOspiIdle(InstancePtr);
	if (Status != (u32)XST_SUCCESS) {
		goto ERROR_PATH;
	}

	XOspiPsv_Setup_Devsize(InstancePtr, Msg);

	if ((Msg->Flags & XOSPIPSV_MSG_FLAG_RX) != 0U) {
		XOspiPsv_Setup_Dev_Read_Instr_Reg(InstancePtr, Msg);
		InstancePtr->RxBytes = Msg->ByteCount;
		InstancePtr->RecvBufferPtr = Msg->RxBfrPtr;
		InstancePtr->SendBufferPtr = NULL;
		if (Msg->Addrvalid == 0U) {
			XOspiPsv_Setup_Stig_Ctrl(InstancePtr, (u32)Msg->Opcode,
				1, (u32)InstancePtr->RxBytes - (u32)1, Msg->Addrvalid, 0,
				(u32)Msg->Addrsize - (u32)1, 0, 0, (u32)Msg->Dummy, 0);
			/* Execute the command */
			XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_FLASH_CMD_CTRL_REG,
				(XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
					XOSPIPSV_FLASH_CMD_CTRL_REG) |
					(u32)XOSPIPSV_FLASH_CMD_CTRL_REG_CMD_EXEC_FLD_MASK));
		} else {
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
			0, 0, Reqaddr, 0, (u32)Msg->Addrsize - (u32)1,
			WriteDataEn, (u32)ByteCount - (u32)1, 0, 0);

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
* @brief
* This function handles interrupt based transfers.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_DEVICE_BUSY if a transfer is already in progress.
*
******************************************************************************/
u32 XOspiPsv_IntrHandler(XOspiPsv *InstancePtr)
{
	u32 StatusReg;
	u32 DmaStatusReg;
	XOspiPsv_Msg *Msg;
	u32 Status;

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
			if (InstancePtr->Config.IsCacheCoherent == 0U) {
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
				InstancePtr->RxBytes = 0U;
				InstancePtr->StatusHandler(InstancePtr->StatusRef,
						XST_SPI_TRANSFER_DONE);
				XOspiPsv_DeAssertCS(InstancePtr);
				InstancePtr->IsBusy = FALSE;
			}
		}
	} else {
		StatusReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
					XOSPIPSV_IRQ_STATUS_REG);
		if ((Msg->Flags & XOSPIPSV_MSG_FLAG_RX) != 0U) {
			if ((StatusReg & XOSPIPSV_IRQ_MASK_REG_STIG_REQ_MASK_FLD_MASK) != 0U) {
				/* Read the data from FIFO */
				XOspiPsv_FifoRead(InstancePtr, Msg);
				StatusReg &= ~(u32)XOSPIPSV_IRQ_MASK_REG_STIG_REQ_MASK_FLD_MASK;
				StatusReg |= (u32)XST_SPI_TRANSFER_DONE;
			}
		} else {
			if ((StatusReg & XOSPIPSV_IRQ_MASK_REG_STIG_REQ_MASK_FLD_MASK) != 0U) {
				StatusReg &= ~(u32)XOSPIPSV_IRQ_MASK_REG_STIG_REQ_MASK_FLD_MASK;
				StatusReg |= (u32)XST_SPI_TRANSFER_DONE;
			}
		}

		/* Clear the interrupts */
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_IRQ_STATUS_REG,
			XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_IRQ_STATUS_REG));

		/* Wait for Idle */
		Status = XOspiPsv_CheckOspiIdle(InstancePtr);
		if (Status != (u32)XST_SUCCESS) {
			return (u32)XST_FAILURE;
		}

		InstancePtr->StatusHandler(InstancePtr->StatusRef, StatusReg);
		XOspiPsv_DeAssertCS(InstancePtr);
		InstancePtr->IsBusy = (u32)FALSE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief
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
* @brief
* Configure TX and RX DLL Delay. Based on the mode and reference clock
* this API calculate the RX delay and configure them in PHY configuration
* register.
*
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fails.
*
******************************************************************************/
u32 XOspiPsv_SetDllDelay(XOspiPsv *InstancePtr)
{

	XOspiPsv_Msg FlashMsg = {0};
	u32 Status;
	u32 TXTap;
	u8 ByteCnt = 4;
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 ReadBfrPtr[8];
#else
	u8 ReadBfrPtr[8]__attribute__ ((aligned(4))) = {0};
#endif

	Xil_AssertNonvoid(InstancePtr != NULL);

	Status = XOspiPsv_CheckOspiIdle(InstancePtr);
	if (Status != (u32)XST_SUCCESS) {
		goto RETURN_PATH;
	}

	if (InstancePtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_SDR_NON_PHY) {
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_PHY_CONFIGURATION_REG, 0x0U);
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_PHY_CONFIGURATION_REG,
				XOSPIPSV_PHY_CONFIGURATION_REG_PHY_CONFIG_RESYNC_FLD_MASK);
		Status = (u32)XST_SUCCESS;
		goto RETURN_PATH;
	} else if (InstancePtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			TXTap = (u32)XOSPIPSV_DDR_TX_VAL;
			if (InstancePtr->DllMode == XOSPIPSV_DLL_MASTER_MODE) {
				TXTap = (u32)XOSPIPSV_DDR_TX_VAL_MASTER;
			}
	} else {
		TXTap = XOSPIPSV_SDR_TX_VAL;
		if (InstancePtr->DllMode == XOSPIPSV_DLL_MASTER_MODE) {
			TXTap = (u32)XOSPIPSV_SDR_TX_VAL_MASTER;
		}
	}
	TXTap = TXTap <<
			XOSPIPSV_PHY_CONFIGURATION_REG_PHY_CONFIG_TX_DLL_DELAY_FLD_SHIFT;
	FlashMsg.Opcode = READ_ID;
	FlashMsg.Addrsize = 0U;
	FlashMsg.Addrvalid = 0U;
	FlashMsg.TxBfrPtr = ReadBfrPtr;
	FlashMsg.RxBfrPtr = ReadBfrPtr;
	FlashMsg.ByteCount = ByteCnt;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	FlashMsg.Dummy = 0U;
	FlashMsg.Addr = 0U;
	FlashMsg.Proto = 0U;
	FlashMsg.IsDDROpCode = 0U;
	if (InstancePtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Dummy = 8U;
		FlashMsg.Proto = XOSPIPSV_READ_8_0_8;
	}

	if (InstancePtr->DllMode == XOSPIPSV_DLL_MASTER_MODE) {
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
					XOSPIPSV_PHY_CONFIGURATION_REG, 0x0);
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
					XOSPIPSV_PHY_MASTER_CONTROL_REG, 0x4);
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_PHY_CONFIGURATION_REG,
				XOSPIPSV_PHY_CONFIGURATION_REG_PHY_CONFIG_RESET_FLD_MASK);
		Status = XOspiPsv_WaitForLock(InstancePtr,
				XOSPIPSV_DLL_OBSERVABLE_LOWER_REG_DLL_OBSERVABLE_LOWER_LOOPBACK_LOCK_FLD_MASK);
		if (Status != (u32)XST_SUCCESS) {
			goto RETURN_PATH;
		}
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_PHY_CONFIGURATION_REG,
				XOSPIPSV_PHY_CONFIGURATION_REG_PHY_CONFIG_RESET_FLD_MASK);
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_PHY_CONFIGURATION_REG,
				(XOSPIPSV_PHY_CONFIGURATION_REG_PHY_CONFIG_RESET_FLD_MASK |
				XOSPIPSV_PHY_CONFIGURATION_REG_PHY_CONFIG_RESYNC_FLD_MASK));
	}

	Status = XOspiPsv_ExecuteRxTuning(InstancePtr, &FlashMsg, TXTap);

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
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
 * @brief
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
 ******************************************************************************/
static inline void StubStatusHandler(void *CallBackRef, u32 StatusEvent)
{
	(void) CallBackRef;
	(void) StatusEvent;

	Xil_AssertVoidAlways();
}

/** @} */
