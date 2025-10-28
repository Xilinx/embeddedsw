/******************************************************************************
* Copyright (C) 2013 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsdps_card.c
* @addtogroup sdps_api SDPS APIs
* @{
*
* The xsdps_card.c file contains the interface functions of the XSdPs driver.
* See xsdps.h for a detailed description of the device and driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 3.9   mn     03/03/20 Restructured the code for more readability and modularity
*       mn     03/16/20 Move XSdPs_Select_Card API to User APIs
* 3.10  mn     06/05/20 Check Transfer completion separately from XSdPs_Read and
*                       XSdPs_Write APIs
*       mn     10/15/20 Modify power cycle API to poll for SD bus lines to go
*                       low for versal platform
* 3.12  sk     01/28/21 Added support for non-blocking write.
*       sk     02/12/21 Fix the issue in reading CID and CSD.
* 3.14  sk     10/22/21 Add support for Erase feature.
*       mn     11/28/21 Fix MISRA-C violations.
*       sk     01/10/22 Add support to read slot_type parameter.
* 4.0   sk     02/25/22 Add support for eMMC5.1.
*       sk     04/07/22 Add support to read custom tap delay values from design
*                       for SD/eMMC.
* 4.1   sk     11/10/22 Add SD/eMMC Tap delay support for Versal Net.
* 4.1   sa     01/03/23	Report error if Transfer size is greater than 2MB.
* 	sa     01/04/23 Update register bit polling logic to use Xil_WaitForEvent/
* 			Xil_WaitForEvents API.
* 	sa     01/25/23 Use instance structure to store DMA descriptor tables.
* 4.2   ap     08/09/23 reordered function XSdPs_Identify_UhsMode.
* 4.3   ap     12/22/23 Add support to read custom HS400 tap delay value from design for eMMC.
* 4.5   sk     10/28/25 Update IsCacheCoherent logic to include EL1_NS mode.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsdps_core.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
* @brief
* Performs SD read in polled mode.
*
* @param	InstancePtr Pointer to the instance to be worked on.
* @param	Arg Address passed by the user that is to be sent as
* 		argument along with the command.
* @param	BlkCnt - Block count passed by the user.
* @param	Buff - Pointer to the data buffer for a DMA transfer.
*
* @return
* 		- XST_SUCCESS if initialization was successful
* 		- XST_FAILURE if failure - could be because another transfer
* 		is in progress or command or data inhibit is set
*
******************************************************************************/
s32 XSdPs_Read(XSdPs *InstancePtr, u32 Arg, u32 BlkCnt, u8 *Buff)
{
	s32 Status;

	if ((BlkCnt * InstancePtr->BlkSize) > (32U * XSDPS_DESC_MAX_LENGTH)) {
#ifdef XSDPS_DEBUG
		xil_printf("Max transfer length supported is 2MB\n");
#endif
		Status = XST_FAILURE;
	} else {

		XSdPs_SetupReadDma(InstancePtr, (u16)BlkCnt, (u16)InstancePtr->BlkSize, Buff);

		if (BlkCnt == 1U) {
			/* Send single block read command */
			Status = XSdPs_CmdTransfer(InstancePtr, CMD17, Arg, BlkCnt);
			if (Status != XST_SUCCESS) {
				Status = XST_FAILURE;
			}
		} else {
			/* Send multiple blocks read command */
			Status = XSdPs_CmdTransfer(InstancePtr, CMD18, Arg, BlkCnt);
			if (Status != XST_SUCCESS) {
				Status = XST_FAILURE;
			}
		}
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief
* Performs SD write in polled mode.
*
* @param	InstancePtr Pointer to the instance to be worked on.
* @param	Arg Address passed by the user that is to be sent as
* 		argument along with the command.
* @param	BlkCnt - Block count passed by the user.
* @param	Buff - Pointer to the data buffer for a DMA transfer.
*
* @return
* 		- XST_SUCCESS if initialization was successful
* 		- XST_FAILURE if failure - could be because another transfer
* 		is in progress or command or data inhibit is set
*
******************************************************************************/
s32 XSdPs_Write(XSdPs *InstancePtr, u32 Arg, u32 BlkCnt, const u8 *Buff)
{
	s32 Status;

	if ((BlkCnt * InstancePtr->BlkSize) > (32U * XSDPS_DESC_MAX_LENGTH)) {
#ifdef XSDPS_DEBUG
		xil_printf("Max transfer length supported is 2MB\n");
#endif
		Status = XST_FAILURE;
	} else {

		XSdPs_SetupWriteDma(InstancePtr, (u16)BlkCnt, (u16)InstancePtr->BlkSize, Buff);

		if (BlkCnt == 1U) {
			/* Send single block write command */
			Status = XSdPs_CmdTransfer(InstancePtr, CMD24, Arg, BlkCnt);
			if (Status != XST_SUCCESS) {
				Status = XST_FAILURE;
			}
		} else {
			/* Send multiple blocks write command */
			Status = XSdPs_CmdTransfer(InstancePtr, CMD25, Arg, BlkCnt);
			if (Status != XST_SUCCESS) {
				Status = XST_FAILURE;
			}
		}
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief
* Checks for the transfer complete.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return
* 		- XST_SUCCESS if transfer was successful
* 		- XST_FAILURE if failure
* 		- XST_DEVICE_BUSY - if the transfer is still in progress
*
******************************************************************************/
s32 XSdPs_CheckTransferComplete(XSdPs *InstancePtr)
{
	u16 StatusReg;
	s32 Status;

	if (InstancePtr->IsBusy == FALSE) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/*
	 * Check for transfer complete
	 */
	StatusReg = XSdPs_ReadReg16(InstancePtr->Config.BaseAddress,
				    XSDPS_NORM_INTR_STS_OFFSET);
	if ((StatusReg & XSDPS_INTR_ERR_MASK) != 0U) {
		/* Write to clear error bits */
		XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
				 XSDPS_ERR_INTR_STS_OFFSET,
				 XSDPS_ERROR_INTR_ALL_MASK);
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	if ((StatusReg & XSDPS_INTR_TC_MASK) == 0U) {
		Status = XST_DEVICE_BUSY;
		goto RETURN_PATH;
	}

	/* Write to clear bit */
	XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
			 XSDPS_NORM_INTR_STS_OFFSET, XSDPS_INTR_TC_MASK);

	InstancePtr->IsBusy = FALSE;

	Status = XST_SUCCESS;

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* @brief
* Identifies type of card using CMD0 + CMD1 sequence
*
*
* @param	InstancePtr Pointer to the XSdPs instance.
*
******************************************************************************/
s32 XSdPs_IdentifyCard(XSdPs *InstancePtr)
{
	s32 Status;

	if ((InstancePtr->HC_Version == XSDPS_HC_SPEC_V3) &&
	    ((InstancePtr->Host_Caps & XSDPS_CAPS_SLOT_TYPE_MASK)
	     == XSDPS_CAPS_EMB_SLOT)) {
		InstancePtr->CardType = XSDPS_CHIP_EMMC;
		Status = XST_SUCCESS;
		goto RETURN_PATH;
	}

	/* 74 CLK delay after card is powered up, before the first command. */
	usleep(XSDPS_INIT_DELAY);

	/* CMD0 no response expected */
	Status = XSdPs_CmdTransfer(InstancePtr, CMD0, 0U, 0U);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Host High Capacity support & High voltage window */
	Status = XSdPs_CmdTransfer(InstancePtr, CMD1,
				   XSDPS_ACMD41_HCS | XSDPS_CMD1_HIGH_VOL, 0U);
	if (Status != XST_SUCCESS) {
		InstancePtr->CardType = XSDPS_CARD_SD;
	} else {
		InstancePtr->CardType = XSDPS_CARD_MMC;
	}

	XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
			 XSDPS_NORM_INTR_STS_OFFSET, XSDPS_NORM_INTR_ALL_MASK);
	XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
			 XSDPS_ERR_INTR_STS_OFFSET, XSDPS_ERROR_INTR_ALL_MASK);

	Status = XSdPs_Reset(InstancePtr, XSDPS_SWRST_CMD_LINE_MASK);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH ;
	}

	Status = XST_SUCCESS;

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function Initializes SD.
*
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return
* 		- XST_SUCCESS if initialization was successful
* 		- XST_FAILURE if failure - could be because
* 			a) SD is already initialized
* 			b) There is no card inserted
* 			c) One of the steps (commands) in the
			   initialization cycle failed
*
* @note		This function initializes the SD card by following its
*		initialization and identification state diagram.
*		CMD0 is sent to reset card.
*		CMD8 and ACDM41 are sent to identify voltage and
*		high capacity support
*		CMD2 and CMD3 are sent to obtain Card ID and
*		Relative card address respectively.
*		CMD9 is sent to read the card specific data.
*
******************************************************************************/
s32 XSdPs_SdCardInitialize(XSdPs *InstancePtr)
{
	s32 Status;

#ifndef UHS_MODE_ENABLE
	InstancePtr->Config.BusWidth = XSDPS_WIDTH_4;
#endif

	Status = XSdPs_SdCardEnum(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	Status = XSdPs_SdModeInit(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	Status = XST_SUCCESS;

RETURN_PATH:
	return Status;

}

/*****************************************************************************/
/**
* @brief
* This function initializes MMC
*
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return
* 		- XST_SUCCESS if initialization was successful
* 		- XST_FAILURE if failure - could be because
* 			a) MMC is already initialized
* 			b) There is no card inserted
* 			c) One of the steps (commands) in the initialization
*			   cycle failed
* @note 	This function initializes the SD card by following its
*		initialization and identification state diagram.
*		CMD0 is sent to reset card.
*		CMD1 sent to identify voltage and high capacity support
*		CMD2 and CMD3 are sent to obtain Card ID and
*		Relative card address respectively.
*		CMD9 is sent to read the card specific data.
*
******************************************************************************/
s32 XSdPs_MmcCardInitialize(XSdPs *InstancePtr)
{
	s32 Status;

	Status = XSdPs_MmcCardEnum(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	if (((InstancePtr->CardType == XSDPS_CARD_MMC) &&
	     (InstancePtr->Card_Version > CSD_SPEC_VER_3)) &&
	    (InstancePtr->HC_Version == XSDPS_HC_SPEC_V2)) {
		Status = XSdPs_MmcModeInit(InstancePtr);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto RETURN_PATH;
		}
	} else if (InstancePtr->CardType == XSDPS_CHIP_EMMC) {
		Status = XSdPs_EmmcModeInit(InstancePtr);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto RETURN_PATH;
		}
	} else {
		Status = XST_FAILURE;
		goto RETURN_PATH;

	}

	if ((InstancePtr->Mode != XSDPS_DDR52_MODE) && (InstancePtr->Mode != XSDPS_HS400_MODE)) {
		Status = XSdPs_SetBlkSize(InstancePtr, XSDPS_BLK_SIZE_512_MASK);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto RETURN_PATH;
		}
	} else {
		XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
				 XSDPS_BLK_SIZE_OFFSET, XSDPS_BLK_SIZE_512_MASK);
		InstancePtr->BlkSize = XSDPS_BLK_SIZE_512_MASK;
	}

	Status = XST_SUCCESS;

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Checks if the card is present or not.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return	None
*
******************************************************************************/
s32 XSdPs_CheckCardDetect(XSdPs *InstancePtr)
{
	u32 PresentStateReg;
	s32 Status;

	if ((InstancePtr->HC_Version == XSDPS_HC_SPEC_V3) &&
	    ((InstancePtr->Host_Caps & XSDPS_CAPS_SLOT_TYPE_MASK)
	     == XSDPS_CAPS_EMB_SLOT)) {
		Status = XST_SUCCESS;
		goto RETURN_PATH;
	}

	if (InstancePtr->Config.CardDetect != 0U) {
		/*
		 * Check the present state register to make sure
		 * card is inserted and detected by host controller
		 */
		PresentStateReg = XSdPs_ReadReg(InstancePtr->Config.BaseAddress,
						XSDPS_PRES_STATE_OFFSET);
		if ((PresentStateReg & XSDPS_PSR_CARD_INSRT_MASK) == 0U)	{
			Status = XST_FAILURE;
			goto RETURN_PATH;
		}
	}

	Status = XST_SUCCESS;

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Sends CMD0 to reset the card.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return	None
*
******************************************************************************/
s32 XSdPs_CardReset(XSdPs *InstancePtr)
{
	s32 Status;

	/* CMD0 no response expected */
	Status = XSdPs_CmdTransfer(InstancePtr, (u32)CMD0, 0U, 0U);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief
* Sends command to get the card interface details.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return	None
*
******************************************************************************/
s32 XSdPs_CardIfCond(XSdPs *InstancePtr)
{
	u32 RespOCR;
	s32 Status;

	/*
	 * CMD8; response expected
	 * 0x1AA - Supply Voltage 2.7 - 3.6V and AA is pattern
	 */
	Status = XSdPs_CmdTransfer(InstancePtr, CMD8,
				   XSDPS_CMD8_VOL_PATTERN, 0U);
	if ((Status != XST_SUCCESS) && (Status != XSDPS_CT_ERROR)) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	if (Status == XSDPS_CT_ERROR) {
		Status = XSdPs_Reset(InstancePtr, XSDPS_SWRST_CMD_LINE_MASK);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto RETURN_PATH ;
		}
	}

	RespOCR = XSdPs_ReadReg(InstancePtr->Config.BaseAddress,
				XSDPS_RESP0_OFFSET);
	if (RespOCR != XSDPS_CMD8_VOL_PATTERN) {
		InstancePtr->Card_Version = XSDPS_SD_VER_1_0;
	} else {
		InstancePtr->Card_Version = XSDPS_SD_VER_2_0;
	}

	Status = XST_SUCCESS;

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Sends command to get the card operating condition.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return	None
*
******************************************************************************/
s32 XSdPs_CardOpCond(XSdPs *InstancePtr)
{
	u32 RespOCR;
	s32 Status;
	u32 Arg;
	u32 Count = 10000U;

	/* Send ACMD41 while card is still busy with power up */
	do {
		if (InstancePtr->CardType == XSDPS_CARD_SD) {
			Status = XSdPs_CmdTransfer(InstancePtr, CMD55, 0U, 0U);
			if (Status != XST_SUCCESS) {
				Status = XST_FAILURE;
				goto RETURN_PATH;
			}

			Arg = XSDPS_ACMD41_HCS | XSDPS_ACMD41_3V3 | (0x1FFU << 15U);
			if ((InstancePtr->HC_Version == XSDPS_HC_SPEC_V3) &&
			    (InstancePtr->Config.BusWidth == XSDPS_WIDTH_8)) {
				Arg |= XSDPS_OCR_S18;
			}

			/* 0x40300000 - Host High Capacity support & 3.3V window */
			Status = XSdPs_CmdTransfer(InstancePtr, ACMD41,
						   Arg, 0U);
		} else {
			/* Send CMD1 while card is still busy with power up */
			Status = XSdPs_CmdTransfer(InstancePtr, CMD1,
						   XSDPS_ACMD41_HCS | XSDPS_CMD1_HIGH_VOL, 0U);
		}
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto RETURN_PATH;
		}

		/* Response with card capacity */
		Status = Xil_WaitForEvent(InstancePtr->Config.BaseAddress + XSDPS_RESP0_OFFSET, XSDPS_RESPOCR_READY,
					  XSDPS_RESPOCR_READY, 1U);
		if (Status == XST_SUCCESS) {
			RespOCR = XSdPs_ReadReg(InstancePtr->Config.BaseAddress, XSDPS_RESP0_OFFSET);
			break;
		}
		Count = Count - 1U;
	} while (Count != 0U);

	if (Count == 0U) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Update HCS support flag based on card capacity response */
	if ((RespOCR & XSDPS_ACMD41_HCS) != 0U) {
		InstancePtr->HCS = 1U;
	}

	if ((RespOCR & XSDPS_OCR_S18) != 0U) {
		InstancePtr->Switch1v8 = 1U;
		Status = XSdPs_Switch_Voltage(InstancePtr);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto RETURN_PATH;
		}
	}

	Status = XST_SUCCESS;

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Gets the card ID.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return	None
*
******************************************************************************/
s32 XSdPs_GetCardId(XSdPs *InstancePtr)
{
	s32 Status;
	u32 Count = 50U;

	/* CMD2 for Card ID */
	Status = XSdPs_CmdTransfer(InstancePtr, CMD2, 0U, 0U);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	InstancePtr->CardID[0] =
		XSdPs_ReadReg(InstancePtr->Config.BaseAddress,
			      XSDPS_RESP0_OFFSET);
	InstancePtr->CardID[1] =
		XSdPs_ReadReg(InstancePtr->Config.BaseAddress,
			      XSDPS_RESP1_OFFSET);
	InstancePtr->CardID[2] =
		XSdPs_ReadReg(InstancePtr->Config.BaseAddress,
			      XSDPS_RESP2_OFFSET);
	InstancePtr->CardID[3] =
		XSdPs_ReadReg(InstancePtr->Config.BaseAddress,
			      XSDPS_RESP3_OFFSET);

	if (InstancePtr->CardType == XSDPS_CARD_SD) {
		do {
			Status = XSdPs_CmdTransfer(InstancePtr, CMD3, 0U, 0U);
			if (Status != XST_SUCCESS) {
				Status = XST_FAILURE;
				goto RETURN_PATH;
			}

			/*
			 * Relative card address is stored as the upper 16 bits
			 * This is to avoid shifting when sending commands
			 */
			Status = Xil_WaitForEvent(InstancePtr->Config.BaseAddress + XSDPS_RESP0_OFFSET, 0xFFFF0000, 0U, 1U);
			if (Status != XST_SUCCESS) {
				InstancePtr->RelCardAddr = XSdPs_ReadReg(InstancePtr->Config.BaseAddress,
							   XSDPS_RESP0_OFFSET) & 0xFFFF0000U;
				break;
			}
			Count = Count - 1U;
		} while (Count != 0U);
	} else {
		/* Set relative card address */
		InstancePtr->RelCardAddr = 0x12340000U;
		Status = XSdPs_CmdTransfer(InstancePtr, CMD3, (InstancePtr->RelCardAddr), 0U);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto RETURN_PATH;
		}
	}
	if (Count == 0U) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	Status = XST_SUCCESS;

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Gets the CSD register from the card.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return	None
*
******************************************************************************/
s32 XSdPs_GetCsd(XSdPs *InstancePtr)
{
	s32 Status;
	u32 CSD[4];
	u32 BlkLen;
	u32 DeviceSize;
	u32 Mult;

	Status = XSdPs_CmdTransfer(InstancePtr, CMD9, (InstancePtr->RelCardAddr), 0U);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/*
	 * Card specific data is read.
	 * Currently not used for any operation.
	 */
	CSD[0] = XSdPs_ReadReg(InstancePtr->Config.BaseAddress,
			       XSDPS_RESP0_OFFSET);
	CSD[1] = XSdPs_ReadReg(InstancePtr->Config.BaseAddress,
			       XSDPS_RESP1_OFFSET);
	CSD[2] = XSdPs_ReadReg(InstancePtr->Config.BaseAddress,
			       XSDPS_RESP2_OFFSET);
	CSD[3] = XSdPs_ReadReg(InstancePtr->Config.BaseAddress,
			       XSDPS_RESP3_OFFSET);

	InstancePtr->CardSpecData[0] = CSD[0];
	InstancePtr->CardSpecData[1] = CSD[1];
	InstancePtr->CardSpecData[2] = CSD[2];
	InstancePtr->CardSpecData[3] = CSD[3];

	if (InstancePtr->CardType != XSDPS_CARD_SD) {
		InstancePtr->Card_Version = (u8)((u32)(CSD[3] & CSD_SPEC_VER_MASK) >> 18U);
		Status = XST_SUCCESS;
		goto RETURN_PATH;
	}

	if (((CSD[3] & CSD_STRUCT_MASK) >> 22U) == 0U) {
		BlkLen = (u32)1U << ((u32)(CSD[2] & READ_BLK_LEN_MASK) >> 8U);
		Mult = (u32)1U << ((u32)((CSD[1] & C_SIZE_MULT_MASK) >> 7U) + (u32)2U);
		DeviceSize = (CSD[1] & C_SIZE_LOWER_MASK) >> 22U;
		DeviceSize |= (CSD[2] & C_SIZE_UPPER_MASK) << 10U;
		DeviceSize = (DeviceSize + 1U) * Mult;
		DeviceSize =  DeviceSize * BlkLen;
		InstancePtr->SectorCount = (DeviceSize / XSDPS_BLK_SIZE_512_MASK);
	} else if (((CSD[3] & CSD_STRUCT_MASK) >> 22U) == 1U) {
		InstancePtr->SectorCount = (((CSD[1] & CSD_V2_C_SIZE_MASK) >> 8U) +
					    1U) * 1024U;
	} else {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	Status = XST_SUCCESS;

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Sets the card voltage to 1.8 V.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return	None
*
******************************************************************************/
s32 XSdPs_CardSetVoltage18(XSdPs *InstancePtr)
{
	s32 Status;
	u16 CtrlReg;
	u16 ClockReg;

	/* Stop the clock */
	CtrlReg = XSdPs_ReadReg16(InstancePtr->Config.BaseAddress,
				  XSDPS_CLK_CTRL_OFFSET);
	CtrlReg &= (u16)(~(XSDPS_CC_SD_CLK_EN_MASK | XSDPS_CC_INT_CLK_EN_MASK));
	XSdPs_WriteReg16(InstancePtr->Config.BaseAddress, XSDPS_CLK_CTRL_OFFSET,
			 CtrlReg);

	/* Check for 1.8V signal enable bit is cleared by Host */
	Status = XSdPs_SetVoltage18(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	ClockReg = XSdPs_ReadReg16(InstancePtr->Config.BaseAddress,
				   XSDPS_CLK_CTRL_OFFSET);
	/* Enable the clock in the controller */
	Status = XSdPs_EnableClock(InstancePtr, ClockReg);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Wait for 1mSec */
	(void)usleep(1000U);

	Status = XST_SUCCESS;

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Configures the initial Reset.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return	None
*
******************************************************************************/
s32 XSdPs_ResetConfig(XSdPs *InstancePtr)
{
	s32 Status;

	XSdPs_DisableBusPower(InstancePtr);

#ifdef versal
	if ((InstancePtr->Host_Caps & XSDPS_CAPS_SLOT_TYPE_MASK)
	    != XSDPS_CAPS_EMB_SLOT) {
		u32 Timeout = 200000U;

		/* Check for SD Bus Lines low */
		Status = Xil_WaitForEvent(InstancePtr->Config.BaseAddress + XSDPS_PRES_STATE_OFFSET, XSDPS_PSR_DAT30_SG_LVL_MASK, 0U,
					  Timeout);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
		}
	}
#endif

	Status = XSdPs_Reset(InstancePtr, XSDPS_SWRST_ALL_MASK);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH ;
	}

	XSdPs_EnableBusPower(InstancePtr);

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Configures the initial Host.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return	None
*
******************************************************************************/
void XSdPs_HostConfig(XSdPs *InstancePtr)
{
	XSdPs_ConfigPower(InstancePtr);

	XSdPs_ConfigDma(InstancePtr);

	XSdPs_ConfigInterrupt(InstancePtr);

	/*
	 * Transfer mode register - default value
	 * DMA enabled, block count enabled, data direction card to host(read)
	 */
	InstancePtr->TransferMode = XSDPS_TM_DMA_EN_MASK | XSDPS_TM_BLK_CNT_EN_MASK |
				    XSDPS_TM_DAT_DIR_SEL_MASK;

	/* Set block size to 512 by default */
	XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
			 XSDPS_BLK_SIZE_OFFSET, XSDPS_BLK_SIZE_512_MASK);
}

/*****************************************************************************/
/**
* @brief
* Checks for Reset Done bits to be cleared after a reset assert.
*
* @param	InstancePtr Pointer to the instance to be worked on.
* @param	Value Bits to be checked to be cleared.
*
* @return	None
*
******************************************************************************/
s32 XSdPs_CheckResetDone(XSdPs *InstancePtr, u8 Value)
{
	u32 Timeout = 1000000U;
	s32 Status;

	/* Proceed with initialization only after reset is complete */
	/* Using XSDPS_CLK_CTRL_OFFSET(0x2C) in place of XSDPS_SW_RST_OFFSET(0x2F) for 32bit address aligned reading */
	Status = Xil_WaitForEvent(InstancePtr->Config.BaseAddress + XSDPS_CLK_CTRL_OFFSET,
				  ((u32)Value) << 24, 0U, Timeout);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH ;
	}

	Status = XST_SUCCESS;

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Sets up the voltage switch.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return	None
*
******************************************************************************/
s32 XSdPs_SetupVoltageSwitch(XSdPs *InstancePtr)
{
	u32 Timeout = 10000;
	s32 Status;

	/* Send switch voltage command */
	Status = XSdPs_CmdTransfer(InstancePtr, CMD11, 0U, 0U);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Wait for CMD and DATA line to go low */
	Status = Xil_WaitForEvent(InstancePtr->Config.BaseAddress + XSDPS_PRES_STATE_OFFSET,
				  XSDPS_PSR_CMD_SG_LVL_MASK | XSDPS_PSR_DAT30_SG_LVL_MASK, 0U, Timeout);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH ;
	}

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Check if the CMD and DAT buses are high.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return	None
*
******************************************************************************/
s32 XSdPs_CheckBusHigh(XSdPs *InstancePtr)
{
	u32 Timeout = MAX_TIMEOUT;
	s32 Status;

	/* Wait for CMD and DATA line to go high */
	Status = Xil_WaitForEvent(InstancePtr->Config.BaseAddress + XSDPS_PRES_STATE_OFFSET,
				  XSDPS_PSR_CMD_SG_LVL_MASK | XSDPS_PSR_DAT30_SG_LVL_MASK,
				  XSDPS_PSR_CMD_SG_LVL_MASK | XSDPS_PSR_DAT30_SG_LVL_MASK, Timeout);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH ;
	}

	Status = XST_SUCCESS;

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* @brief
* API to Identify the supported UHS mode. This API will assign the
* corresponding tap delay API to the Config_TapDelay pointer based on the
* supported bus speed.
*
*
* @param	InstancePtr Pointer to the XSdPs instance.
* @param	ReadBuff Contains the response for CMD6
*
* @return	None.
*
*
******************************************************************************/
void XSdPs_Identify_UhsMode(XSdPs *InstancePtr, u8 *ReadBuff)
{
	if (((ReadBuff[13] & UHS_SDR104_SUPPORT) != 0U) &&
	    (InstancePtr->Config.InputClockHz >= XSDPS_SD_INPUT_MAX_CLK)) {
		InstancePtr->Mode = XSDPS_UHS_SPEED_MODE_SDR104;
		XSdPs_SetTapDelay_SDR104(InstancePtr);
	} else if (((ReadBuff[13] & UHS_SDR50_SUPPORT) != 0U) &&
		   (InstancePtr->Config.InputClockHz >= XSDPS_SD_SDR50_MAX_CLK)) {
		InstancePtr->Mode = XSDPS_UHS_SPEED_MODE_SDR50;
		XSdPs_SetTapDelay_SDR50(InstancePtr);
	} else if (((ReadBuff[13] & UHS_DDR50_SUPPORT) != 0U) &&
		   (InstancePtr->Config.InputClockHz >= XSDPS_SD_DDR50_MAX_CLK)) {
		InstancePtr->Mode = XSDPS_UHS_SPEED_MODE_DDR50;
		XSdPs_SetTapDelay_DDR50(InstancePtr);
	} else if (((ReadBuff[13] & UHS_SDR25_SUPPORT) != 0U) &&
		   (InstancePtr->Config.InputClockHz >= XSDPS_SD_SDR25_MAX_CLK)) {
		InstancePtr->Mode = XSDPS_UHS_SPEED_MODE_SDR25;
		XSdPs_SetTapDelay_SDR25(InstancePtr);
	} else {
		InstancePtr->Mode = XSDPS_UHS_SPEED_MODE_SDR12;
	}
}

/*****************************************************************************/
/**
*
* @brief
* API to set Tap Delay with respect to the speed modes.
*
*
* @param	InstancePtr Pointer to the XSdPs instance.
*
* @return	None
*
******************************************************************************/
void XSdPs_SetTapDelay(XSdPs *InstancePtr)
{
	if ((InstancePtr->Mode == XSDPS_DEFAULT_SPEED_MODE) ||
	    (InstancePtr->Mode == XSDPS_UHS_SPEED_MODE_SDR12)) {
		return;
	}

#ifndef versal
	/* Issue DLL Reset */
	XSdPs_DllRstCtrl(InstancePtr, 1U);
#endif

	/* Configure the Tap Delay Registers */
	XSdPs_ConfigTapDelay(InstancePtr);

#ifndef versal
	/* Release the DLL out of reset */
	XSdPs_DllRstCtrl(InstancePtr, 0U);
#endif
}

/*****************************************************************************/
/**
* @brief
* Changes the SD Bus Speed.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return	None
*
******************************************************************************/
s32 XSdPs_Change_SdBusSpeed(XSdPs *InstancePtr)
{
	s32 Status;
	u32 Arg;
	u16 BlkCnt;
	u16 BlkSize;
	u16 CtrlReg;
	u8 ReadBuff[64] = {0U};

	Status = XSdPs_CalcBusSpeed(InstancePtr, &Arg);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	BlkCnt = XSDPS_SWITCH_CMD_BLKCNT;
	BlkSize = XSDPS_SWITCH_CMD_BLKSIZE;

	XSdPs_SetupReadDma(InstancePtr, BlkCnt, BlkSize, ReadBuff);

	Status = XSdPs_CmdTransfer(InstancePtr, CMD6, Arg, BlkCnt);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Check for transfer done */
	Status = XSdps_CheckTransferDone(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	if (InstancePtr->Switch1v8 != 0U) {
		/* Set UHS mode in controller */
		CtrlReg = XSdPs_ReadReg16(InstancePtr->Config.BaseAddress,
					  XSDPS_HOST_CTRL2_OFFSET);
		CtrlReg &= (~(u16)XSDPS_HC2_UHS_MODE_MASK);
		XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
				 XSDPS_HOST_CTRL2_OFFSET,
				 CtrlReg | (u16)InstancePtr->Mode);
	}

	Status = XST_SUCCESS;

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Changes the eMMC bus speed.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return	None
*
******************************************************************************/
s32 XSdPs_Change_MmcBusSpeed(XSdPs *InstancePtr)
{
	s32 Status;
	u32 Arg;

	Status = XSdPs_CalcBusSpeed(InstancePtr, &Arg);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	Status = XSdPs_CmdTransfer(InstancePtr, CMD6, Arg, 0U);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Check for transfer done */
	Status = XSdps_CheckTransferDone(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	Status = XST_SUCCESS;

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function is used to do the Auto tuning.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return	None
*
******************************************************************************/
s32 XSdPs_AutoTuning(XSdPs *InstancePtr)
{
	s32 Status;
	u16 BlkSize;
	u8 TuningCount;

	BlkSize = XSDPS_TUNING_CMD_BLKSIZE;
	if (InstancePtr->BusWidth == XSDPS_8_BIT_WIDTH) {
		BlkSize = BlkSize * 2U;
	}
	BlkSize &= XSDPS_BLK_SIZE_MASK;
	XSdPs_WriteReg16(InstancePtr->Config.BaseAddress, XSDPS_BLK_SIZE_OFFSET,
			 BlkSize);

	InstancePtr->TransferMode = XSDPS_TM_DAT_DIR_SEL_MASK;

	XSdPs_SetExecTuning(InstancePtr);
	/*
	 * workaround which can work for 1.0/2.0 silicon for auto tuning.
	 * This can be revisited for 3.0 silicon if necessary.
	 */
	/* Wait for ~60 clock cycles to reset the tap values */
	(void)usleep(1U);

	for (TuningCount = 0U; TuningCount < MAX_TUNING_COUNT; TuningCount++) {

		if (InstancePtr->CardType == XSDPS_CARD_SD) {
			Status = XSdPs_CmdTransfer(InstancePtr, CMD19, 0U, 1U);
		} else {
			Status = XSdPs_CmdTransfer(InstancePtr, CMD21, 0U, 1U);
		}

		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto RETURN_PATH;
		}

		if ((XSdPs_ReadReg16(InstancePtr->Config.BaseAddress,
				     XSDPS_HOST_CTRL2_OFFSET) & XSDPS_HC2_EXEC_TNG_MASK) == 0U) {
			break;
		}
	}

	if ((XSdPs_ReadReg16(InstancePtr->Config.BaseAddress,
			     XSDPS_HOST_CTRL2_OFFSET) & XSDPS_HC2_SAMP_CLK_SEL_MASK) == 0U) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Wait for ~12 clock cycles to synchronize the new tap values */
	(void)usleep(1U);

	Status = XST_SUCCESS;

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* @brief
* API to setup ADMA2 descriptor table.
*
*
* @param	InstancePtr Pointer to the XSdPs instance.
* @param	BlkCnt Block count.
* @param	Buff  Pointer to data buffer.
*
* @return	None
*
******************************************************************************/
void XSdPs_SetupADMA2DescTbl(XSdPs *InstancePtr, u32 BlkCnt, const u8 *Buff)
{
	if (InstancePtr->HC_Version == XSDPS_HC_SPEC_V3) {
		XSdPs_Setup64ADMA2DescTbl(InstancePtr, BlkCnt, Buff);
	} else {
		XSdPs_Setup32ADMA2DescTbl(InstancePtr, BlkCnt, Buff);
	}
}

/*****************************************************************************/
/**
*
* @brief
* API to setup ADMA2 descriptor table for 64 Bit DMA.
*
*
* @param	InstancePtr Pointer to the XSdPs instance.
* @param	BlkCnt Block count.
*
* @return	None
*
*
******************************************************************************/
void XSdPs_SetupADMA2DescTbl64Bit(XSdPs *InstancePtr, u32 BlkCnt)
{
	u32 TotalDescLines;
	u32 DescNum;
	u32 BlkSize;

	/* Setup ADMA2 - Write descriptor table and point ADMA SAR to it */
	BlkSize = (u32)XSdPs_ReadReg16(InstancePtr->Config.BaseAddress,
				       XSDPS_BLK_SIZE_OFFSET) &
		  XSDPS_BLK_SIZE_MASK;

	if ((BlkCnt * BlkSize) < XSDPS_DESC_MAX_LENGTH) {

		TotalDescLines = 1U;

	} else {

		TotalDescLines = ((BlkCnt * BlkSize) / XSDPS_DESC_MAX_LENGTH);
		if (((BlkCnt * BlkSize) % XSDPS_DESC_MAX_LENGTH) != 0U) {
			TotalDescLines += 1U;
		}

	}

	for (DescNum = 0U; DescNum < (TotalDescLines - 1U); DescNum++) {
		InstancePtr->Adma2_DescrTbl64[DescNum].Address =
			InstancePtr->Dma64BitAddr +
			((u64)DescNum * XSDPS_DESC_MAX_LENGTH);
		InstancePtr->Adma2_DescrTbl64[DescNum].Attribute =
			XSDPS_DESC_TRAN | XSDPS_DESC_VALID;
		InstancePtr->Adma2_DescrTbl64[DescNum].Length = 0U;
	}

	InstancePtr->Adma2_DescrTbl64[TotalDescLines - 1U].Address =
		InstancePtr->Dma64BitAddr +
		((u64)DescNum * XSDPS_DESC_MAX_LENGTH);

	InstancePtr->Adma2_DescrTbl64[TotalDescLines - 1U].Attribute =
		XSDPS_DESC_TRAN | XSDPS_DESC_END | XSDPS_DESC_VALID;

	InstancePtr->Adma2_DescrTbl64[TotalDescLines - 1U].Length =
		(u16)((BlkCnt * BlkSize) - (u32)(DescNum * XSDPS_DESC_MAX_LENGTH));

	XSdPs_WriteReg(InstancePtr->Config.BaseAddress, XSDPS_ADMA_SAR_OFFSET,
		       (u32)((UINTPTR) & (InstancePtr->Adma2_DescrTbl64[0]) & ~(u32)0x0U));

#if defined(EL1_NONSECURE) && (EL1_NONSECURE == 1U)
	if (InstancePtr->Config.IsCacheCoherent == 0U) {
		Xil_DCacheFlushRange((INTPTR) & (InstancePtr->Adma2_DescrTbl64[0]),
				     (INTPTR)sizeof(XSdPs_Adma2Descriptor64) * (INTPTR)32U);
	}
#else
	Xil_DCacheFlushRange((INTPTR) & (InstancePtr->Adma2_DescrTbl64[0]),
			(INTPTR)sizeof(XSdPs_Adma2Descriptor64) * (INTPTR)32U);
#endif

	/* Clear the 64-Bit Address variable */
	InstancePtr->Dma64BitAddr = 0U;

}

/*****************************************************************************/
/**
*
* @brief
* API to reset the DLL.
*
*
* @param	InstancePtr Pointer to the XSdPs instance.
*
* @return	None
*
*
******************************************************************************/
s32 XSdPs_DllReset(XSdPs *InstancePtr)
{
	u32 ClockReg;
	s32 Status;

	/* Disable clock */
	ClockReg = XSdPs_ReadReg16(InstancePtr->Config.BaseAddress,
				   XSDPS_CLK_CTRL_OFFSET);
	ClockReg &= ~XSDPS_CC_SD_CLK_EN_MASK;
	XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
			 XSDPS_CLK_CTRL_OFFSET, (u16)ClockReg);

	/* Issue DLL Reset to load zero tap values */
	XSdPs_DllRstCtrl(InstancePtr, 1U);

	/* Wait for 2 micro seconds */
	(void)usleep(2U);

	XSdPs_DllRstCtrl(InstancePtr, 0U);

	ClockReg = XSdPs_ReadReg16(InstancePtr->Config.BaseAddress,
				   XSDPS_CLK_CTRL_OFFSET);
	/* Enable the clock in the controller */
	Status = XSdPs_EnableClock(InstancePtr, (u16)ClockReg);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief
* Identifies the eMMC speed mode.
*
* @param	InstancePtr Pointer to the instance to be worked on.
* @param	ExtCsd Extended CSD register from the card.
*
* @return	None
*
******************************************************************************/
void XSdPs_IdentifyEmmcMode(XSdPs *InstancePtr, const u8 *ExtCsd)
{
	if (InstancePtr->BusWidth < XSDPS_4_BIT_WIDTH) {
		InstancePtr->Mode = XSDPS_DEFAULT_SPEED_MODE;
	} else {
		/* Check for card supported speed */
#if defined (VERSAL_NET) && defined (ENABLE_HS400_MODE)
		if ((ExtCsd[EXT_CSD_DEVICE_TYPE_BYTE] &
		     (EXT_CSD_DEVICE_TYPE_DDR_1V8_HS400 |
		      EXT_CSD_DEVICE_TYPE_DDR_1V2_HS400)) != 0U) {
			InstancePtr->Mode = XSDPS_HS400_MODE;
			InstancePtr->IsTuningDone = 0U;
			InstancePtr->OTapDelay = SD_OTAPDLYSEL_HS200_B0;
		} else {
#endif
			if ((ExtCsd[EXT_CSD_DEVICE_TYPE_BYTE] &
			     (EXT_CSD_DEVICE_TYPE_SDR_1V8_HS200 |
			      EXT_CSD_DEVICE_TYPE_SDR_1V2_HS200)) != 0U) {
				InstancePtr->Mode = XSDPS_HS200_MODE;
				if (InstancePtr->Config.OTapDly_SDR_Clk200 != 0U) {
					InstancePtr->OTapDelay = InstancePtr->Config.OTapDly_SDR_Clk200;
				} else if (InstancePtr->Config.BankNumber == 2U) {
					InstancePtr->OTapDelay = SD_OTAPDLYSEL_HS200_B2;
				} else {
					InstancePtr->OTapDelay = SD_OTAPDLYSEL_HS200_B0;
				}
			} else if ((ExtCsd[EXT_CSD_DEVICE_TYPE_BYTE] &
				    (EXT_CSD_DEVICE_TYPE_DDR_1V8_HIGH_SPEED |
				     EXT_CSD_DEVICE_TYPE_DDR_1V2_HIGH_SPEED)) != 0U) {
				InstancePtr->Mode = XSDPS_DDR52_MODE;
				if ((InstancePtr->Config.OTapDly_DDR_Clk50 != 0U) &&
				    (InstancePtr->Config.ITapDly_DDR_Clk50 != 0U)) {
					InstancePtr->OTapDelay = InstancePtr->Config.OTapDly_DDR_Clk50;
					InstancePtr->ITapDelay = InstancePtr->Config.ITapDly_DDR_Clk50;
				} else {
					InstancePtr->OTapDelay = SD_OTAPDLYSEL_EMMC_DDR50;
					InstancePtr->ITapDelay = SD_ITAPDLYSEL_EMMC_DDR50;
				}
			} else if ((ExtCsd[EXT_CSD_DEVICE_TYPE_BYTE] &
				    EXT_CSD_DEVICE_TYPE_HIGH_SPEED) != 0U) {
				InstancePtr->Mode = XSDPS_HIGH_SPEED_MODE;
				if ((InstancePtr->Config.OTapDly_SDR_Clk50 != 0U) &&
				    (InstancePtr->Config.ITapDly_SDR_Clk50 != 0U)) {
					InstancePtr->OTapDelay = InstancePtr->Config.OTapDly_SDR_Clk50;
					InstancePtr->ITapDelay = InstancePtr->Config.ITapDly_SDR_Clk50;
				} else {
					InstancePtr->OTapDelay = SD_OTAPDLYSEL_EMMC_HSD;
					InstancePtr->ITapDelay = SD_ITAPDLYSEL_EMMC_HSD;
				}
			} else {
				InstancePtr->Mode = XSDPS_DEFAULT_SPEED_MODE;
			}
#if defined (VERSAL_NET) && defined (ENABLE_HS400_MODE)
		}
#endif
	}
}

/*****************************************************************************/
/**
* @brief
* Checks the eMMC timing.
*
* @param	InstancePtr Pointer to the instance to be worked on.
* @param	ExtCsd Extended CSD register from the card.
*
* @return	None
*
******************************************************************************/
s32 XSdPs_CheckEmmcTiming(XSdPs *InstancePtr, u8 *ExtCsd)
{
	s32 Status;

	Status = XSdPs_Get_Mmc_ExtCsd(InstancePtr, ExtCsd);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

#ifdef VERSAL_NET
	if (InstancePtr->Mode == XSDPS_HS400_MODE) {
		if (ExtCsd[EXT_CSD_HS_TIMING_BYTE] != EXT_CSD_HS_TIMING_HS400) {
			Status = XST_FAILURE;
			goto RETURN_PATH;
		}
	} else {
#endif
		if (InstancePtr->Mode == XSDPS_HS200_MODE) {
			if (ExtCsd[EXT_CSD_HS_TIMING_BYTE] != EXT_CSD_HS_TIMING_HS200) {
				Status = XST_FAILURE;
				goto RETURN_PATH;
			}
		} else if ((InstancePtr->Mode == XSDPS_HIGH_SPEED_MODE) ||
			   (InstancePtr->Mode == XSDPS_DDR52_MODE)) {
			if (ExtCsd[EXT_CSD_HS_TIMING_BYTE] != EXT_CSD_HS_TIMING_HIGH) {
				Status = XST_FAILURE;
				goto RETURN_PATH;
			}
		} else {
			Status = XST_FAILURE;
		}
#ifdef VERSAL_NET
	}
#endif

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Sets the clock to the passed frequency.
*
* @param	InstancePtr Pointer to the instance to be worked on.
* @param	SelFreq  Selected frequency
*
* @return	None
*
******************************************************************************/
s32 XSdPs_SetClock(XSdPs *InstancePtr, u32 SelFreq)
{
	u16 ClockReg;
	s32 Status;
	u32 Reg;
	u32 Timeout = 1000U;

	/* Disable clock */
	XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
			 XSDPS_CLK_CTRL_OFFSET, 0U);

	/* If selected frequency is zero, return from here */
	if (SelFreq == 0U) {
		Status = XST_SUCCESS;
		goto RETURN_PATH ;
	}

#ifdef VERSAL_NET
	if (InstancePtr->CardType == XSDPS_CHIP_EMMC) {
		Reg = XSdPs_ReadReg(InstancePtr->Config.BaseAddress,
				    XSDPS_PHYCTRLREG2_OFFSET);
		Reg &= ~XSDPS_PHYREG2_DLL_EN_MASK;
		if ((InstancePtr->Mode != XSDPS_DEFAULT_SPEED_MODE) &&
		    (SelFreq > XSDPD_MIN_DLL_MODE_CLK)) {
			XSdPs_WriteReg(InstancePtr->Config.BaseAddress,
				       XSDPS_PHYCTRLREG2_OFFSET, Reg);
			Reg &= ~XSDPS_PHYREG2_FREQ_SEL_MASK;
			Reg &= ~XSDPS_PHYREG2_TRIM_ICP_MASK;
			Reg &= ~XSDPS_PHYREG2_DLYTX_SEL_MASK;
			Reg &= ~XSDPS_PHYREG2_DLYRX_SEL_MASK;
			Reg |= (XSDPS_PHYREG2_TRIM_ICP_DEF_VAL <<
				XSDPS_PHYREG2_TRIM_ICP_SHIFT);
			if (SelFreq == XSDPS_MMC_DDR_MAX_CLK) {
				Reg |= (XSDPS_FREQ_SEL_50MHZ_79MHz <<
					XSDPS_PHYREG2_FREQ_SEL_SHIFT);
			}

			XSdPs_WriteReg(InstancePtr->Config.BaseAddress,
				       XSDPS_PHYCTRLREG2_OFFSET, Reg);
		} else {
			Reg |= XSDPS_PHYREG2_DLYTX_SEL_MASK;
			Reg |= XSDPS_PHYREG2_DLYRX_SEL_MASK;
			XSdPs_WriteReg(InstancePtr->Config.BaseAddress,
				       XSDPS_PHYCTRLREG2_OFFSET, Reg);
		}
	}
#endif

	/* Calculate the clock */
	ClockReg = (u16)XSdPs_CalcClock(InstancePtr, SelFreq);

	/* Enable the clock in the controller */
	Status = XSdPs_EnableClock(InstancePtr, ClockReg);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
	}

#ifdef VERSAL_NET
	if ((InstancePtr->CardType == XSDPS_CHIP_EMMC) &&
	    (InstancePtr->Mode != XSDPS_DEFAULT_SPEED_MODE) &&
	    (SelFreq > XSDPD_MIN_DLL_MODE_CLK)) {
		Reg = XSdPs_ReadReg(InstancePtr->Config.BaseAddress,
				    XSDPS_PHYCTRLREG2_OFFSET);
		Reg |= XSDPS_PHYREG2_DLL_EN_MASK;
		XSdPs_WriteReg(InstancePtr->Config.BaseAddress,
			       XSDPS_PHYCTRLREG2_OFFSET, Reg);

		/* Wait for 1000 micro sec for DLL READY */
		Status = Xil_WaitForEvent(InstancePtr->Config.BaseAddress + XSDPS_PHYCTRLREG2_OFFSET,
					  XSDPS_PHYREG2_DLL_RDY_MASK, XSDPS_PHYREG2_DLL_RDY_MASK, Timeout);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto RETURN_PATH ;
		}
	}
#else
	(void)Reg;
	(void)Timeout;
#endif

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Checks if the voltage is set to 1.8 V or not.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return
* 		- XST_SUCCESS if voltage is 1.8V
* 		- XST_FAILURE if voltage is not 1.8V
*
******************************************************************************/
s32 XSdPs_CheckVoltage18(XSdPs *InstancePtr)
{
	u32 Status;

	if ((XSdPs_ReadReg16(InstancePtr->Config.BaseAddress,
			     XSDPS_HOST_CTRL2_OFFSET) & XSDPS_HC2_1V8_EN_MASK) == 0U) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	Status = XST_SUCCESS;

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Initializes the command sequence.
*
* @param	InstancePtr Pointer to the instance to be worked on.
* @param	Arg Address passed by the user that is to be sent as
* 		argument along with the command.
* @param	BlkCnt - Block count passed by the user.
*
* @return	None
*
******************************************************************************/
s32 XSdPs_SetupCmd(XSdPs *InstancePtr, u32 Arg, u32 BlkCnt)
{
	s32 Status;

	/*
	 * Check the command inhibit to make sure no other
	 * command transfer is in progress
	 */
	Status = XSdPs_CheckBusIdle(InstancePtr, XSDPS_PSR_INHIBIT_CMD_MASK);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH ;
	}

	/* Write block count register */
	XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
			 XSDPS_BLK_CNT_OFFSET, (u16)BlkCnt);

	XSdPs_WriteReg8(InstancePtr->Config.BaseAddress,
			XSDPS_TIMEOUT_CTRL_OFFSET, 0xEU);

	/* Write argument register */
	XSdPs_WriteReg(InstancePtr->Config.BaseAddress,
		       XSDPS_ARGMT_OFFSET, Arg);

	XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
			 XSDPS_NORM_INTR_STS_OFFSET, XSDPS_NORM_INTR_ALL_MASK);
	XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
			 XSDPS_ERR_INTR_STS_OFFSET, XSDPS_ERROR_INTR_ALL_MASK);

	Status = XST_SUCCESS;

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Initiates the Cmd transfer to SD card.
*
* @param	InstancePtr Pointer to the instance to be worked on.
* @param	Cmd Command to be sent
*
* @return
* 		- XST_SUCCESS if initialization was successful
* 		- XST_FAILURE if failure
*
******************************************************************************/
s32 XSdPs_SendCmd(XSdPs *InstancePtr, u32 Cmd)
{
	u32 PresentStateReg;
	u32 CommandReg;
	s32 Status;

	/* Command register is set to trigger transfer of command */
	CommandReg = XSdPs_FrameCmd(InstancePtr, Cmd);

	/*
	 * Mask to avoid writing to reserved bits 31-30
	 * This is necessary because 0x8000 is used  by this software to
	 * distinguish between ACMD and CMD of same number
	 */
	CommandReg = CommandReg & 0x3FFFU;

	/*
	 * Check for data inhibit in case of command using DAT lines.
	 * For Tuning Commands DAT lines check can be ignored.
	 */
	if ((Cmd != CMD21) && (Cmd != CMD19)) {
		PresentStateReg = XSdPs_ReadReg(InstancePtr->Config.BaseAddress,
						XSDPS_PRES_STATE_OFFSET);
		if (((PresentStateReg & XSDPS_PSR_INHIBIT_DAT_MASK) != 0U) &&
		    ((CommandReg & XSDPS_DAT_PRESENT_SEL_MASK) != 0U)) {
			Status = XST_FAILURE;
			goto RETURN_PATH;
		}
	}

	XSdPs_WriteReg(InstancePtr->Config.BaseAddress, XSDPS_XFER_MODE_OFFSET,
		       (CommandReg << 16) | InstancePtr->TransferMode);

	Status = XST_SUCCESS;

RETURN_PATH:
	return Status;

}

/*****************************************************************************/
/**
* @brief
* Sets the address of the first write block to be erased.
*
* @param	InstancePtr Pointer to the instance to be worked on.
* @param	StartAddr Address of the first write block.
*
* @return
* 		- XST_SUCCESS if Set start Address is successful
* 		- XST_FAILURE if failure - could be because failed to set EndAddr.
*
******************************************************************************/
s32 XSdPs_SetStartAddr(XSdPs *InstancePtr, u32 StartAddr)
{
	s32 Status;

	if (InstancePtr->CardType == XSDPS_CARD_SD) {
		Status = XSdPs_CmdTransfer(InstancePtr, CMD32, StartAddr, 0U);
	} else {
		Status = XSdPs_CmdTransfer(InstancePtr, CMD35, StartAddr, 0U);
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief
* Sets the address of the last write block to be erased.
*
* @param	InstancePtr Pointer to the instance to be worked on.
* @param	EndAddr Address of the last write block.
*
* @return
* 		- XST_SUCCESS if Set End Address is successful.
* 		- XST_FAILURE if failure - could be because failed to set EndAddr.
*
******************************************************************************/
s32 XSdPs_SetEndAddr(XSdPs *InstancePtr, u32 EndAddr)
{
	s32 Status;

	if (InstancePtr->CardType == XSDPS_CARD_SD) {
		Status = XSdPs_CmdTransfer(InstancePtr, CMD33, EndAddr, 0U);
	} else {
		Status = XSdPs_CmdTransfer(InstancePtr, CMD36, EndAddr, 0U);
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief
* Sends Erase command to the device and wait for transfer complete.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return
* 		- XST_SUCCESS if erase operation is successful
* 		- XST_FAILURE if failure - could be because erase operation failed.
*
******************************************************************************/
s32 XSdPs_SendErase(XSdPs *InstancePtr)
{
	s32 Status;

	Status = XSdPs_CmdTransfer(InstancePtr, CMD38, 0U, 0U);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH ;
	}

	/* Check for transfer done */
	Status = XSdps_CheckTransferDone(InstancePtr);

RETURN_PATH:
	return Status;
}

/** @} */
