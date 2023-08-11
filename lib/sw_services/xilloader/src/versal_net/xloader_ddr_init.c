/******************************************************************************
 * Copyright (c) 2023, Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xloader_ddr_init.c
 *
 * This is the file which contains PMC and DDRMC5 handshake
 * process function.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  ro   05/24/2023 Initial release
 * 1.1   ro   08/1/2023  Handle i2c handshake across CDO Chunk boundary
 *       dd   08/11/2023 Updated doxygen comments
 * </pre>
 *
 * @note
 *
 ******************************************************************************/

/***************************** Include Files **********************************/
#include "xparameters.h"
#include "xil_printf.h"
#include "xplmi_cmd.h"
#include "sleep.h"
#include "xplmi_hw.h"
#include "xplmi_debug.h"
#include "xpm_rail.h"
#include "xloader_plat.h"
#ifdef XLOADER_PMC_IIC
#include "xiicps.h"

/************************** Constant Definitions ******************************/
#define XLOADER_MAX_BASE_ADDR_QUEUE (8U)
/************************** Variable Definitions ******************************/
typedef struct {
	u32 NumBaseAddr;
	u32 CmdTimeout;
	u32 CtrlRegOff;
	u32 BaseAddressQueue[XLOADER_MAX_BASE_ADDR_QUEUE];
} XLoader_I2cHsCmd;

static u64 TimeStart;
static u64 TimeoutNs;

#define XPLMI_WR_TRANS                       0x00U
#define XPLMI_HANDSHAKE_BIT_MASK             0X00000001U
#define XPLMI_REQ_BIT_MASK                   0x00000002U
#define XPLMI_TRANS_MASK                     0x00000004U
#define XPLMI_MORE_BIT_MASK                  0x00000008U
#define XPLMI_REPSTART_BIT_MASK              0x00000010U
#define XPLMI_SIZE_MASK                      0x001F0000U
#define XPLMI_SLAVE_ADDR_MASK  			    0X7F000000U
#define XPLMI_STATUS_ERR_MASK				0X00E00000U
#define XPLMI_REQ_BIT_CLEAR_MASK             0x00000002U
#define XPLMI_HANDSHAKE_BIT_CLEAR_MASK       0X00000001U
#define XPLMI_NUMBASE_ADDR_MASK              0X000F0000U
#define XPLMI_HANDSHAKE_TIMEOUT_MASK         0X0000FFFFU
#define XPLMI_NUMBASE_ADDR_SHIFT             16U
#define XPLMI_SLAVE_ADDR_SHIFT               24U
#define XPLMI_SIZE_SHIFT                     16U
#define XPLMI_STATUS_ERR                     0X00200000U
#define XPLMI_HANDSHAKE_BIT_CLEAR            0X00000000U
#define XPLMI_REQ_BIT_CLEAR                  0X00000000U
#define XPLMI_DATA_OFFSET_INDEX              4U
#define XPLMI_MAX_DATA_SIZE                  30U
#define XPLMI_BIT_LEN                        1U
#define XPLMI_SHORT_WORD_LEN                 2U
#define XPLMI_BYTE                           0xFFU

/************************** Function Prototypes *******************************/
static int XLoader_PmcI2cParseHandshakeCmd (XLoader_I2cHsCmd *I2cHsPtr,
		XPlmi_Cmd *Cmd);
static int XLoader_MbPmcI2cHandshakeProcess(XIicPs *Iic, u32 BaseAddress,
		u32 DataBuffOffset, u32 CtrlReg);
static u32 Xloader_CheckForTimeout(void);
/*****************************************************************************/
/**
 * This function parse the handshake command request coming from MB and
 * calls the function for handshake process.
 *
 * @param   CmdPtr is pointer to the command structure.
 *
 * @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
 *
 * @note		None.
 *
 *******************************************************************************/
int XLoader_MbPmcI2cHandshake(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 HsTimeoutStatus = TRUE;
	u32 Index = 0U;
	u32 HsDoneStatus = 0U;
	u32 CtrlRegAddr = 0U;
	u32 CtrlReg = 0U;
	static XLoader_I2cHsCmd I2cHsCmd;
	XIicPs *IicInstance; /**< Instance of the IicInstance Device */

	IicInstance = XPmRail_GetIicInstance();

	if (IicInstance->IsReady != (u32) XIL_COMPONENT_IS_READY) {
		Status = I2CInitialize(IicInstance);
		if (Status != XST_SUCCESS) {
			Status = (int)XLOADER_ERR_I2C_TRANSACTION;
			goto END;
		}
	}

	/* Parse the PLM-UB handshake command */
	Status = XLoader_PmcI2cParseHandshakeCmd(&I2cHsCmd, Cmd);
	if (Status != XST_SUCCESS) {
		goto END;
	}


	TimeoutNs = ((u64) I2cHsCmd.CmdTimeout * XPLMI_MEGA);
	TimeStart = XPlmi_GetTimerValue();

	while (HsDoneStatus != (XPLMI_BIT(I2cHsCmd.NumBaseAddr) - 1U)) {
		/*Check handshake timeout status*/
		HsTimeoutStatus = Xloader_CheckForTimeout();
		if (HsTimeoutStatus != TRUE) {
			Status = (int)XLOADER_ERR_HS_TIMEOUT;
			break;
		}

		if ((HsDoneStatus & XPLMI_BIT(Index)) == XPLMI_BIT(Index)) {
			goto UPDATE_INDEX;
		}

		/*Read the control register */
		CtrlRegAddr = I2cHsCmd.BaseAddressQueue[Index]
			      + I2cHsCmd.CtrlRegOff;
		CtrlReg = XPlmi_In32(CtrlRegAddr);

		if ((CtrlReg & XPLMI_HANDSHAKE_BIT_MASK) == XPLMI_HANDSHAKE_BIT_MASK) {
			/*block the current controller handshake process by masking HsDoneStatus variable */
			HsDoneStatus |= XPLMI_BIT(Index);
			/* Update handshake done bit (0x0) in control register */
			XPlmi_UtilRMW(CtrlRegAddr, XPLMI_HANDSHAKE_BIT_CLEAR_MASK,
				      XPLMI_HANDSHAKE_BIT_CLEAR);
			XPlmi_Printf(DEBUG_INFO, "Handshake done!!\n\r");
		}

		if ((CtrlReg & XPLMI_REQ_BIT_MASK) == XPLMI_REQ_BIT_MASK) {
			/* Process the read/write i2c transaction request raised from MB*/
			Status = XLoader_MbPmcI2cHandshakeProcess(IicInstance,
					I2cHsCmd.BaseAddressQueue[Index],
					CtrlRegAddr + XPLMI_DATA_OFFSET_INDEX, CtrlReg);
			/* Clear Request bit and update in control register */
			XPlmi_UtilRMW(CtrlRegAddr, XPLMI_REQ_BIT_CLEAR_MASK,
				      XPLMI_REQ_BIT_CLEAR);
			if (Status != XST_SUCCESS) {
				/* Updated status error bit(0x1) in control register */
				XPlmi_UtilRMW(CtrlRegAddr, XPLMI_STATUS_ERR_MASK,
					      XPLMI_STATUS_ERR);
				Status = (int)XLOADER_ERR_I2C_TRANSACTION;
				break;
			}
		}
		if (((CtrlReg & XPLMI_MORE_BIT_MASK) == XPLMI_MORE_BIT_MASK) ||
		    ((CtrlReg & XPLMI_REPSTART_BIT_MASK) == XPLMI_REPSTART_BIT_MASK)) {
			continue;
		}
UPDATE_INDEX:
		/* Update index for round robin scheduling*/
		if (Index == (I2cHsCmd.NumBaseAddr - XPLMI_BIT_LEN)) {
			Index = 0;
		}
		else {
			Index++;
		}

	}

	if ((HsDoneStatus == (XPLMI_BIT(I2cHsCmd.NumBaseAddr) - 1U))
	    && (HsTimeoutStatus != FALSE)) {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}
/*****************************************************************************/
/**
 * This function takes the argument base address and perform the write/ read
 * operation and update back the required information to control register.
 *
 * @param    Iic is i2c instance pointer.
 * @param    BaseAddress is the base address of ddrmc5.
 * @param    DataBuffOffset is the data offset to read and write update.
 * @param    CtrlReg contains the control register data.
 *
 * @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
 *
 * @note		None.
 *
 *******************************************************************************/
static int XLoader_MbPmcI2cHandshakeProcess(XIicPs *Iic, u32 BaseAddress,
		u32 DataBuffOffset, u32 CtrlReg)
{
	int Status = XST_FAILURE;
	u32 ReadData = 0U;
	u32 DataBufAddr = 0U;
	u32 Index = 0U;
	u8 DataBuff[XPLMI_MAX_DATA_SIZE] = { 0 };
	u32 RecvData = 0U;

	/* Size validation  */
	if (((CtrlReg & XPLMI_SIZE_MASK) >> XPLMI_SIZE_SHIFT) < (u32)XPLMI_MAX_DATA_SIZE) {
		/* Read the data buffer offset */
		DataBufAddr = XPlmi_In32(DataBuffOffset);
		for (Index = 0;
		     Index < ((CtrlReg & XPLMI_SIZE_MASK) >> XPLMI_SIZE_SHIFT);
		     Index++) {
			ReadData = XPlmi_In32((BaseAddress + DataBufAddr) + (Index * XPLMI_WORD_LEN));
			DataBuff[Index] = (u8)(ReadData & XPLMI_BYTE);
		}

		/*Check for repeated start bit*/
		if ((CtrlReg & XPLMI_REPSTART_BIT_MASK) == XPLMI_REPSTART_BIT_MASK) {
			Status = XIicPs_SetOptions(Iic, XIICPS_REP_START_OPTION);
			if (Status != XST_SUCCESS) {
				Status = (int)XLOADER_ERR_I2C_TRANSACTION;
				goto END;
			}
		}
		else {
			Status = XIicPs_ClearOptions(Iic, XIICPS_REP_START_OPTION);
			if (Status != XST_SUCCESS) {
				Status = (int)XLOADER_ERR_I2C_TRANSACTION;
				goto END;
			}
		}

		if ((CtrlReg & XPLMI_TRANS_MASK) == XPLMI_WR_TRANS) {
			/*
			 * Sending i2c write data request frame to DIMM along with
			 * write data offset, data size, slave address.
			 */
			Status = XIicPs_MasterSendPolled(Iic, DataBuff,
							 ((s32)((CtrlReg & XPLMI_SIZE_MASK) >> XPLMI_SIZE_SHIFT)),
							 ((u16)((CtrlReg & XPLMI_SLAVE_ADDR_MASK)
								>> XPLMI_SLAVE_ADDR_SHIFT)));
		}
		else {
			/*
			 * Receiving data from DIMM card as per the read request.
			 */
			Status = XIicPs_MasterRecvPolled(Iic, DataBuff,
							 ((s32)((CtrlReg & XPLMI_SIZE_MASK) >> XPLMI_SIZE_SHIFT)),
							 ((u16)((CtrlReg & XPLMI_SLAVE_ADDR_MASK)
								>> XPLMI_SLAVE_ADDR_SHIFT)));
		}
		if (Status != XST_SUCCESS) {
			Status = (int)XLOADER_ERR_I2C_TRANSACTION;
			goto END;
		}
		if (!((CtrlReg & XPLMI_REPSTART_BIT_MASK) == XPLMI_REPSTART_BIT_MASK)) {
			TimeStart = XPlmi_GetTimerValue();
			/* Wait 1 second until bus is idle to start another transfer */
			while (XIicPs_BusIsBusy(Iic)) {
				/* Check the timeout status */
				if (FALSE == Xloader_CheckForTimeout()) {
					Status = (int)XLOADER_ERR_I2C_BUS_BUSY;
					goto END;
				}
			}
		}
		if ((CtrlReg & XPLMI_TRANS_MASK) != XPLMI_WR_TRANS) {
			for (Index = 0;
			     Index
			     < ((CtrlReg & XPLMI_SIZE_MASK)
				>> XPLMI_SIZE_SHIFT); Index++) {
				RecvData = (u8)(DataBuff[Index] & XPLMI_BYTE);
				XPlmi_Out32(((BaseAddress + DataBufAddr) + (Index * XPLMI_WORD_LEN)),
					    RecvData);
			}
		}
		Status = XST_SUCCESS;
	}
END:
	return Status;
}
/*****************************************************************************/
/**
 *
 * This function parses the handshake command request comes from the memory controller
 * Microblaze and updates the handshake command parameter structure.
 *
 * @param    I2cHsPtr is i2c handshake command structure pointer.
 * @param    Cmd is pointer to the command structure.
 *
 * @return	None
 *
 * @note	None.
 *
 *******************************************************************************/

static int XLoader_PmcI2cParseHandshakeCmd (XLoader_I2cHsCmd *I2cHsPtr,
		XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 Index = 0U;
	const u32 *AddrPayload = NULL;

	if (Cmd->ProcessedLen == 0U) {
		Status = XPlmi_MemSetBytes(I2cHsPtr, sizeof(XLoader_I2cHsCmd), 0,
					   sizeof(XLoader_I2cHsCmd));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		I2cHsPtr->NumBaseAddr = ((Cmd->Payload[0U] & XPLMI_NUMBASE_ADDR_MASK)
					 >> XPLMI_NUMBASE_ADDR_SHIFT);

		I2cHsPtr->CmdTimeout =
			((Cmd->Payload[0U] & XPLMI_HANDSHAKE_TIMEOUT_MASK));

		I2cHsPtr->CtrlRegOff = Cmd->Payload[1U];
		Cmd->ResumeData[0U] = Cmd->PayloadLen - XPLMI_SHORT_WORD_LEN;
		AddrPayload = &Cmd->Payload[2U];

		XPlmi_Printf(DEBUG_INFO, "Num of base address count = %u\n\r",
			     I2cHsPtr->NumBaseAddr);
		XPlmi_Printf(DEBUG_INFO, "Handshake cmd timeout = %u\n\r",
			     I2cHsPtr->CmdTimeout);
		XPlmi_Printf(DEBUG_INFO, "Control register offset= 0x%x\n\r",
			     I2cHsPtr->CtrlRegOff);
	}
	else {
		Index = Cmd->ResumeData[0U];
		Cmd->ResumeData[0U] += Cmd->PayloadLen;
		AddrPayload = Cmd->Payload;
	}

	if (Cmd->ResumeData[0U] > XLOADER_MAX_BASE_ADDR_QUEUE) {
		Status = (int)XLOADER_ERR_MAX_BASE_ADDR;
		goto END;
	}

	/* copy all the requested base addresses from MB into the base address buffer */
	while (Index < Cmd->ResumeData[0U]) {
		I2cHsPtr->BaseAddressQueue[Index] = AddrPayload[Index];
		XPlmi_Printf(DEBUG_INFO, "DDRMC %u Base address offset= 0x%x\n\r",
			     Index, AddrPayload[Index]);
		Index++;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * This function is to check the timeout.
 *
 * @param    None.
 *
 * @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
 *
 * @note		None.
 *
 *******************************************************************************/
static u32 Xloader_CheckForTimeout(void)
{
	u32 Status = FALSE;
	const u32 *PmcIroFreq = XPlmi_GetPmcIroFreq();
	u32 PmcIroFreqMHz = *PmcIroFreq / XPLMI_MEGA;
	u64 TimeOutTicksReq = 0U;
	u64 TimeDiffReq = 0U;

	TimeOutTicksReq = ((TimeoutNs * PmcIroFreqMHz) + XPLMI_KILO - 1U)
			  / XPLMI_KILO;
	TimeDiffReq = TimeStart - XPlmi_GetTimerValue();

	if (TimeDiffReq >= TimeOutTicksReq) {
		XPlmi_Printf(DEBUG_INFO, "Timeout!!\n\r");
		goto END;
	}
	Status = TRUE;
END:
	return Status;
}

#else
/*****************************************************************************/
/**
 * This function parse the handshake command request coming from MB and
 * calls the function for handshake process.
 *
 * @param   Cmd is pointer to the command structure.
 *
 * @return
 *			 - XST_FAILURE  Always.
 *
 *******************************************************************************/
int XLoader_MbPmcI2cHandshake(XPlmi_Cmd *Cmd)
{
	(void) Cmd;
	return XST_FAILURE;
}
#endif
