/******************************************************************************
* Copyright (C) 2013 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsdps_host.c
* @addtogroup sdps_api SDPS APIs
* @{
*
* The xsdps_host.c file contains the interface functions of the XSdPs driver.
* See xsdps.h for a detailed description of the device and driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 3.9   mn     03/03/20 Restructured the code for more readability and modularity
*       mn     03/16/20 Add code to get card ID for MMC/eMMC
* 3.10  mn     07/09/20 Modified code to prevent removing pull up on D3 line
*       mn     07/30/20 Read 16Bit value for Block Size Register
* 3.11  sk     12/01/20 Tap programming sequence updates like disable OTAPEN
*                       always, write zero to tap register for zero tap value.
*       sk     12/17/20 Removed checking platform specific SD macros and used
*                       Baseaddress instead.
* 3.13  sk     08/10/21 Limit the SD operating frequency to 19MHz for Versal.
* 3.14  sk     10/22/21 Add support for Erase feature.
*       mn     11/28/21 Fix MISRA-C violations.
*       sk     01/10/22 Add support to read slot_type parameter.
* 4.0   sk     02/25/22 Add support for eMMC5.1.
*       sk     04/07/22 Add support to read custom tap delay values from design
*                       for SD/eMMC.
*       sk     06/03/22 Fix issue in internal clock divider calculation logic.
* 4.1   sk     11/10/22 Add SD/eMMC Tap delay support for Versal Net.
* 	sa     01/04/23	Update register bit polling logic to use Xil_WaitForEvent/
* 			Xil_WaitForEvents API.
* 	sa     01/25/23 Use instance structure to store DMA descriptor tables.
* 4.2   ap     08/09/23 Restructured XSdPs_FrameCmd API
* 4.3   ap     12/22/23 Add support to read custom HS400 tap delay value from design for eMMC.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsdps_core.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

#if defined (__aarch64__) && (EL1_NONSECURE == 1)
void XSdps_Smc(XSdPs *InstancePtr, u32 RegOffset, u32 Mask, u32 Val)
{
	(void)Xil_Smc(MMIO_WRITE_SMC_FID, (u64)(InstancePtr->SlcrBaseAddr +
						RegOffset) | ((u64)Mask << 32),
		      (u64)Val, 0, 0, 0, 0, 0);
}
#endif

/*****************************************************************************/
/**
* @brief
* Sets the tap delay for SDR104 uhs mode.
*
* @param        InstancePtr Pointer to the instance to be worked on.
*
* @return       None
*
*******************************************************************************/
void XSdPs_SetTapDelay_SDR104(XSdPs *InstancePtr)
{
	if (InstancePtr->Config.OTapDly_SDR_Clk200 != 0U) {
		InstancePtr->OTapDelay = InstancePtr->Config.OTapDly_SDR_Clk200;
	} else if (InstancePtr->Config.BankNumber == 2U) {
		InstancePtr->OTapDelay = SD_OTAPDLYSEL_SD104_B2;
	} else {
		InstancePtr->OTapDelay = SD_OTAPDLYSEL_SD104_B0;
	}
}

/*****************************************************************************/
/**
* @brief
* Sets the tap delay for SDR50 uhs mode.
*
* @param        InstancePtr Pointer to the instance to be worked on.
*
* @return       None
*
*******************************************************************************/
void XSdPs_SetTapDelay_SDR50(XSdPs *InstancePtr)
{
	if (InstancePtr->Config.OTapDly_SDR_Clk100 != 0U) {
		InstancePtr->OTapDelay = InstancePtr->Config.OTapDly_SDR_Clk100;
	} else {
		InstancePtr->OTapDelay = SD_OTAPDLYSEL_SD50;
	}
}

/*****************************************************************************/
/**
* @brief
* Sets the tap delay for DDR50 uhs mode.
*
* @param        InstancePtr Pointer to the instance to be worked on.
*
* @return       None
*
*******************************************************************************/
void XSdPs_SetTapDelay_DDR50(XSdPs *InstancePtr)
{
	if ((InstancePtr->Config.OTapDly_DDR_Clk50 != 0U) &&
	    (InstancePtr->Config.ITapDly_DDR_Clk50 != 0U)) {
		InstancePtr->OTapDelay = InstancePtr->Config.OTapDly_DDR_Clk50;
		InstancePtr->ITapDelay = InstancePtr->Config.ITapDly_DDR_Clk50;
		if ((InstancePtr->Config.SlotType == XSDPS_SLOTTYPE_SDADIR) &&
		    (InstancePtr->ITapDelay == SD_ITAPDLYSEL_SD_DDR50)) {
			InstancePtr->ITapDelay = SD_AUTODIR_ITAPDLYSEL_DDR50;
		}
	} else if (InstancePtr->Config.SlotType == XSDPS_SLOTTYPE_SDADIR) {
		InstancePtr->ITapDelay = SD_AUTODIR_ITAPDLYSEL_DDR50;
		InstancePtr->OTapDelay = SD_OTAPDLYSEL_SD_DDR50;
	} else {
		InstancePtr->ITapDelay = SD_ITAPDLYSEL_SD_DDR50;
		InstancePtr->OTapDelay = SD_OTAPDLYSEL_SD_DDR50;
	}
}

/*****************************************************************************/
/**
* @brief
* Sets the tap delay for SDR25 uhs mode.
*
* @param        InstancePtr Pointer to the instance to be worked on.
*
* @return       None
*
*******************************************************************************/
void XSdPs_SetTapDelay_SDR25(XSdPs *InstancePtr)
{
	if ((InstancePtr->Config.OTapDly_SDR_Clk50 != 0U) &&
	    (InstancePtr->Config.ITapDly_SDR_Clk50 != 0U)) {
		InstancePtr->OTapDelay = InstancePtr->Config.OTapDly_SDR_Clk50;
		InstancePtr->ITapDelay = InstancePtr->Config.ITapDly_SDR_Clk50;
		if ((InstancePtr->Config.SlotType == XSDPS_SLOTTYPE_SDADIR) &&
		    (InstancePtr->ITapDelay == SD_ITAPDLYSEL_HSD)) {
			InstancePtr->ITapDelay = SD_AUTODIR_ITAPDLYSEL_SDR25;
		}
	} else if (InstancePtr->Config.SlotType == XSDPS_SLOTTYPE_SDADIR) {
		InstancePtr->ITapDelay = SD_AUTODIR_ITAPDLYSEL_SDR25;
		InstancePtr->OTapDelay = SD_OTAPDLYSEL_SD_HSD;
	} else {
		InstancePtr->ITapDelay = SD_ITAPDLYSEL_HSD;
		InstancePtr->OTapDelay = SD_OTAPDLYSEL_SD_HSD;
	}
}

/*****************************************************************************/
/**
*
* @brief
* Switches the SD card voltage from 3v3 to 1v8
*
*
* @param	InstancePtr Pointer to the XSdPs instance.
*
******************************************************************************/
s32 XSdPs_Switch_Voltage(XSdPs *InstancePtr)
{
	s32 Status;

	/* Setup the voltage switching sequence */
	Status = XSdPs_SetupVoltageSwitch(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Set the card voltage to 1.8V */
	Status = XSdPs_CardSetVoltage18(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Check if the bus is high */
	Status = XSdPs_CheckBusHigh(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
	}

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Initiates the transfer to or from SD card.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return
* 		- XST_SUCCESS if initialization was successful
* 		- XST_FAILURE if failure
*
******************************************************************************/
s32 XSdPs_SetupTransfer(XSdPs *InstancePtr)
{
	u32 PresentStateReg;
	s32 Status;

	if ((InstancePtr->HC_Version != XSDPS_HC_SPEC_V3) ||
	    ((InstancePtr->Host_Caps & XSDPS_CAPS_SLOT_TYPE_MASK)
	     != XSDPS_CAPS_EMB_SLOT)) {
		if (InstancePtr->Config.CardDetect != 0U) {
			/* Check status to ensure card is initialized */
			PresentStateReg = XSdPs_ReadReg(InstancePtr->Config.BaseAddress,
							XSDPS_PRES_STATE_OFFSET);
			if ((PresentStateReg & XSDPS_PSR_CARD_INSRT_MASK) == 0x0U) {
				Status = XST_FAILURE;
				goto RETURN_PATH;
			}
		}
	}

	/* Set block size to 512 if not already set */
	if (XSdPs_ReadReg16(InstancePtr->Config.BaseAddress,
			    XSDPS_BLK_SIZE_OFFSET) != XSDPS_BLK_SIZE_512_MASK ) {
		Status = XSdPs_SetBlkSize(InstancePtr,
					  XSDPS_BLK_SIZE_512_MASK);
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
* Resets the SD card.
*
* @param	InstancePtr Pointer to the instance to be worked on.
* @param	Value Type of reset
*
* @return
* 		- XST_SUCCESS if initialization was successful
* 		- XST_FAILURE if failure
*
******************************************************************************/
s32 XSdPs_Reset(XSdPs *InstancePtr, u8 Value)
{
	s32 Status;

	/* "Software reset for all" is initiated */
	XSdPs_WriteReg8(InstancePtr->Config.BaseAddress, XSDPS_SW_RST_OFFSET,
			Value);

	Status = XSdPs_CheckResetDone(InstancePtr, Value);
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
* Sets bit to start execution of tuning.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return	None
*
******************************************************************************/
void XSdPs_SetExecTuning(XSdPs *InstancePtr)
{
	u16 CtrlReg;

	CtrlReg = XSdPs_ReadReg16(InstancePtr->Config.BaseAddress,
				  XSDPS_HOST_CTRL2_OFFSET);
	CtrlReg |= XSDPS_HC2_EXEC_TNG_MASK;
	XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
			 XSDPS_HOST_CTRL2_OFFSET, CtrlReg);
}

/*****************************************************************************/
/**
* @brief
* Initializes SD mode.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return
* 		- XST_SUCCESS if initialization is successful
* 		- XST_FAILURE if failure
*
******************************************************************************/
s32 XSdPs_SdModeInit(XSdPs *InstancePtr)
{
	s32 Status;
#ifdef __ICCARM__
#pragma data_alignment = 32
	static u8 SCR[8] = { 0U };
#else
	static u8 SCR[8] __attribute__ ((aligned(32))) = { 0U };
#endif
#if SD_HS_MODE_ENABLE
	u8 ReadBuff[64] = { 0U };
#endif

	Status = XSdPs_Get_BusWidth(InstancePtr, SCR);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	if ((SCR[1] & WIDTH_4_BIT_SUPPORT) != 0U) {
		InstancePtr->BusWidth = XSDPS_4_BIT_WIDTH;
		Status = XSdPs_Change_BusWidth(InstancePtr);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto RETURN_PATH;
		}
	}

#if SD_HS_MODE_ENABLE
	/* Get speed supported by device */
	Status = XSdPs_Get_BusSpeed(InstancePtr, ReadBuff);
	if (Status != XST_SUCCESS) {
		goto RETURN_PATH;
	}

	if (((SCR[2] & SCR_SPEC_VER_3) != 0U) &&
	    (ReadBuff[13] >= UHS_SDR50_SUPPORT) &&
	    (InstancePtr->Config.BusWidth == XSDPS_WIDTH_8) &&
	    (InstancePtr->Switch1v8 == 0U)) {

		InstancePtr->Switch1v8 = 1U;

		Status = XSdPs_CardSetVoltage18(InstancePtr);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto RETURN_PATH;
		}
	}

	if (InstancePtr->Switch1v8 != 0U) {

		/* Identify the UHS mode supported by card */
		XSdPs_Identify_UhsMode(InstancePtr, ReadBuff);

		Status = XSdPs_Change_BusSpeed(InstancePtr);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto RETURN_PATH;
		}
	} else {
		/*
		 * card supports CMD6 when SD_SPEC field in SCR register
		 * indicates that the Physical Layer Specification Version
		 * is 1.10 or later. So for SD v1.0 cmd6 is not supported.
		 */
		if (SCR[0] != 0U) {
			/* Check for high speed support */
			if (((ReadBuff[13] & HIGH_SPEED_SUPPORT) != 0U) &&
			    (InstancePtr->BusWidth >= XSDPS_4_BIT_WIDTH)) {
				InstancePtr->Mode = XSDPS_HIGH_SPEED_MODE;
				if (InstancePtr->Config.OTapDly_SDR_Clk50 &&
				    InstancePtr->Config.ITapDly_SDR_Clk50) {
					InstancePtr->OTapDelay = InstancePtr->Config.OTapDly_SDR_Clk50;
					InstancePtr->ITapDelay = InstancePtr->Config.ITapDly_SDR_Clk50;
					if ((InstancePtr->Config.SlotType == XSDPS_SLOTTYPE_SDADIR) &&
					    (InstancePtr->ITapDelay == SD_ITAPDLYSEL_HSD)) {
						InstancePtr->ITapDelay = SD_AUTODIR_ITAPDLYSEL_HSD;
					}
				} else if (InstancePtr->Config.SlotType == XSDPS_SLOTTYPE_SDADIR) {
					InstancePtr->ITapDelay = SD_AUTODIR_ITAPDLYSEL_HSD;
					InstancePtr->OTapDelay = SD_OTAPDLYSEL_SD_HSD;
				} else {
					InstancePtr->ITapDelay = SD_ITAPDLYSEL_HSD;
					InstancePtr->OTapDelay = SD_OTAPDLYSEL_SD_HSD;
				}

				Status = XSdPs_Change_BusSpeed(InstancePtr);
				if (Status != XST_SUCCESS) {
					Status = XST_FAILURE;
					goto RETURN_PATH;
				}
			}
		}
	}
#endif

	Status = XSdPs_SetBlkSize(InstancePtr, XSDPS_BLK_SIZE_512_MASK);
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
* Initializes MMC mode.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return
* 		- XST_SUCCESS if initialization is successful
* 		- XST_FAILURE if failure
*
******************************************************************************/
s32 XSdPs_MmcModeInit(XSdPs *InstancePtr)
{
	s32 Status;
#ifdef __ICCARM__
#pragma data_alignment = 32
	static u8 ExtCsd[512];
#else
	static u8 ExtCsd[512] __attribute__ ((aligned(32)));
#endif

	InstancePtr->BusWidth = XSDPS_4_BIT_WIDTH;
	Status = XSdPs_Change_BusWidth(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	Status = XSdPs_Get_Mmc_ExtCsd(InstancePtr, ExtCsd);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	InstancePtr->SectorCount = ((u32)ExtCsd[EXT_CSD_SEC_COUNT_BYTE4]) << 24;
	InstancePtr->SectorCount |= (u32)ExtCsd[EXT_CSD_SEC_COUNT_BYTE3] << 16;
	InstancePtr->SectorCount |= (u32)ExtCsd[EXT_CSD_SEC_COUNT_BYTE2] << 8;
	InstancePtr->SectorCount |= (u32)ExtCsd[EXT_CSD_SEC_COUNT_BYTE1];

	if (((ExtCsd[EXT_CSD_DEVICE_TYPE_BYTE] &
	      EXT_CSD_DEVICE_TYPE_HIGH_SPEED) != 0U) &&
	    (InstancePtr->BusWidth >= XSDPS_4_BIT_WIDTH)) {
		InstancePtr->Mode = XSDPS_HIGH_SPEED_MODE;
		Status = XSdPs_Change_BusSpeed(InstancePtr);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto RETURN_PATH;
		}

		Status = XSdPs_Get_Mmc_ExtCsd(InstancePtr, ExtCsd);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto RETURN_PATH;
		}

		if (ExtCsd[EXT_CSD_HS_TIMING_BYTE] != EXT_CSD_HS_TIMING_HIGH) {
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
* Initializes eMMC mode.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return
* 		- XST_SUCCESS if initialization is successful
* 		- XST_FAILURE if failure
*
******************************************************************************/
s32 XSdPs_EmmcModeInit(XSdPs *InstancePtr)
{
	s32 Status;

#ifdef __ICCARM__
#pragma data_alignment = 32
	static u8 ExtCsd[512];
#else
	static u8 ExtCsd[512] __attribute__ ((aligned(32)));
#endif

	if ((InstancePtr->HC_Version == XSDPS_HC_SPEC_V3) &&
	    (InstancePtr->Config.BusWidth == XSDPS_WIDTH_8)) {
		/* in case of eMMC data width 8-bit */
		InstancePtr->BusWidth = XSDPS_8_BIT_WIDTH;
	} else if (InstancePtr->Config.BusWidth == XSDPS_WIDTH_4) {
		/* in case of eMMC data width 4-bit */
		InstancePtr->BusWidth = XSDPS_4_BIT_WIDTH;
	} else {
		/* in case of eMMC data width 1-bit */
		InstancePtr->BusWidth = XSDPS_1_BIT_WIDTH;
	}

	Status = XSdPs_Change_BusWidth(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Get Extended CSD */
	Status = XSdPs_Get_Mmc_ExtCsd(InstancePtr, ExtCsd);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	InstancePtr->SectorCount = ((u32)ExtCsd[EXT_CSD_SEC_COUNT_BYTE4]) << 24;
	InstancePtr->SectorCount |= (u32)ExtCsd[EXT_CSD_SEC_COUNT_BYTE3] << 16;
	InstancePtr->SectorCount |= (u32)ExtCsd[EXT_CSD_SEC_COUNT_BYTE2] << 8;
	InstancePtr->SectorCount |= (u32)ExtCsd[EXT_CSD_SEC_COUNT_BYTE1];

	XSdPs_IdentifyEmmcMode(InstancePtr, ExtCsd);

	if (InstancePtr->Mode != XSDPS_DEFAULT_SPEED_MODE) {
		Status = XSdPs_Change_BusSpeed(InstancePtr);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto RETURN_PATH;
		}

		Status = XSdPs_CheckEmmcTiming(InstancePtr, ExtCsd);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto RETURN_PATH;
		}

		if (InstancePtr->Mode == XSDPS_DDR52_MODE) {
			Status = XSdPs_Change_BusWidth(InstancePtr);
			if (Status != XST_SUCCESS) {
				Status = XST_FAILURE;
				goto RETURN_PATH;
			}
		}
	}

	/* Enable Rst_n_Fun bit if it is disabled */
	if (ExtCsd[EXT_CSD_RST_N_FUN_BYTE] == EXT_CSD_RST_N_FUN_TEMP_DIS) {
		Status = XSdPs_Set_Mmc_ExtCsd(InstancePtr, XSDPS_MMC_RST_FUN_EN_ARG);
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
* Disables the bus power.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return	None
*
******************************************************************************/
void XSdPs_DisableBusPower(XSdPs *InstancePtr)
{
	/* Disable SD bus power and issue eMMC HW reset */
	if (InstancePtr->HC_Version == XSDPS_HC_SPEC_V3) {
		XSdPs_WriteReg8(InstancePtr->Config.BaseAddress,
				XSDPS_POWER_CTRL_OFFSET, XSDPS_PC_EMMC_HW_RST_MASK);
	} else {
		XSdPs_WriteReg8(InstancePtr->Config.BaseAddress,
				XSDPS_POWER_CTRL_OFFSET, 0x0);
	}

	/* 1ms delay to poweroff card */
	(void)usleep(1000U);
}

/*****************************************************************************/
/**
* @brief
* Enables the bus power.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return	None
*
******************************************************************************/
void XSdPs_EnableBusPower(XSdPs *InstancePtr)
{
	/* Select voltage and enable bus power. */
	if (InstancePtr->HC_Version == XSDPS_HC_SPEC_V3) {
		XSdPs_WriteReg8(InstancePtr->Config.BaseAddress,
				XSDPS_POWER_CTRL_OFFSET,
				(XSDPS_PC_BUS_VSEL_3V3_MASK | XSDPS_PC_BUS_PWR_MASK) &
				~XSDPS_PC_EMMC_HW_RST_MASK);
	} else {
		XSdPs_WriteReg8(InstancePtr->Config.BaseAddress,
				XSDPS_POWER_CTRL_OFFSET,
				XSDPS_PC_BUS_VSEL_3V3_MASK | XSDPS_PC_BUS_PWR_MASK);
	}

	/* 0.2ms Delay after bus power on*/
	usleep(200);
}

/*****************************************************************************/
/**
* @brief
* Enumerates the SD card.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return	None
*
******************************************************************************/
s32 XSdPs_SdCardEnum(XSdPs *InstancePtr)
{
	s32 Status;

	/* Check if the card is present */
	Status = XSdPs_CheckCardDetect(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Reset the SD card */
	Status = XSdPs_CardReset(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Get the card interface condition */
	Status = XSdPs_CardIfCond(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Get the card operating condition */
	Status = XSdPs_CardOpCond(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Get the card ID */
	Status = XSdPs_GetCardId(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Get the CSD register */
	Status = XSdPs_GetCsd(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Change clock to default clock 25MHz */
	/*
	 * SD default speed mode timing should be closed at 19 MHz.
	 * The reason for this is SD requires a voltage level shifter.
	 * This limitation applies to ZynqMPSoC.
	 */
	if (InstancePtr->HC_Version == XSDPS_HC_SPEC_V3) {
		InstancePtr->BusSpeed = SD_CLK_19_MHZ;
	} else {
		InstancePtr->BusSpeed = SD_CLK_25_MHZ;
	}
	Status = XSdPs_Change_ClkFreq(InstancePtr, InstancePtr->BusSpeed);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Select the card to transition to transfer state */
	Status = XSdPs_Select_Card(InstancePtr);
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
* Enumerates the MMC card.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return	None
*
******************************************************************************/
s32 XSdPs_MmcCardEnum(XSdPs *InstancePtr)
{
	s32 Status;

	/* Check if the card is preset */
	Status = XSdPs_CheckCardDetect(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Reset the card */
	Status = XSdPs_CardReset(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Get the card operating condition */
	Status = XSdPs_CardOpCond(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Get the card ID */
	Status = XSdPs_GetCardId(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Get the CSD register */
	Status = XSdPs_GetCsd(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Change clock to default clock 26MHz */
	InstancePtr->BusSpeed = SD_CLK_26_MHZ;
	Status = XSdPs_Change_ClkFreq(InstancePtr, InstancePtr->BusSpeed);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Send select card command to transition to transfer state */
	Status = XSdPs_Select_Card(InstancePtr);
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
* Performs SD tuning.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return	None
*
******************************************************************************/
s32 XSdPs_Execute_Tuning(XSdPs *InstancePtr)
{
	s32 Status;

#if !defined (versal) && !defined (VERSAL_NET)
	/* Issue DLL Reset to load new SDHC tuned tap values */
	Status = XSdPs_DllReset(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

#endif

	/* Perform the auto tuning */
	Status = XSdPs_AutoTuning(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

#if !defined (versal) && !defined (VERSAL_NET)
	/* Issue DLL Reset to load new SDHC tuned tap values */
	Status = XSdPs_DllReset(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

#endif

	Status = XST_SUCCESS;

RETURN_PATH:
	return Status;

}

/*****************************************************************************/
/**
* @brief
* Enables the clock.
*
* @param	InstancePtr Pointer to the instance to be worked on.
* @param	ClockReg Clock value to be set.
*
* @return
* 		- XST_SUCCESS if success
* 		- XST_FAILURE if failure
*
******************************************************************************/
s32 XSdPs_EnableClock(XSdPs *InstancePtr, u16 ClockReg)
{
	u32 Timeout = 150000U;
	s32 Status;
	u16 ClkReg = ClockReg;

	ClkReg |= (u16)XSDPS_CC_INT_CLK_EN_MASK;
	XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
			 XSDPS_CLK_CTRL_OFFSET, ClkReg);

	/* Wait for 150ms for internal clock to stabilize */
	Status = Xil_WaitForEvent(InstancePtr->Config.BaseAddress + XSDPS_CLK_CTRL_OFFSET,
				  XSDPS_CC_INT_CLK_STABLE_MASK, XSDPS_CC_INT_CLK_STABLE_MASK, Timeout);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH ;
	}

	/* Enable SD clock */
	ClkReg |= XSDPS_CC_SD_CLK_EN_MASK;
	XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
			 XSDPS_CLK_CTRL_OFFSET, ClkReg);

	Status = XST_SUCCESS;

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Calculates the bus speed.
*
* @param	InstancePtr Pointer to the instance to be worked on.
* @param	Arg Argument to be sent along with the command.
* 		This could be address or any other information
*
* @return
* 		- XST_SUCCESS if success
* 		- XST_FAILURE if failure
*
******************************************************************************/
s32 XSdPs_CalcBusSpeed(XSdPs *InstancePtr, u32 *Arg)
{
	s32 Status = XST_SUCCESS;

	if (InstancePtr->CardType == XSDPS_CARD_SD) {
		switch (InstancePtr->Mode) {
			case XSDPS_UHS_SPEED_MODE_SDR12:
				*Arg = XSDPS_SWITCH_CMD_SDR12_SET;
				InstancePtr->BusSpeed = XSDPS_SD_SDR12_MAX_CLK;
				break;
			case XSDPS_UHS_SPEED_MODE_SDR25:
				*Arg = XSDPS_SWITCH_CMD_SDR25_SET;
				InstancePtr->BusSpeed = XSDPS_SD_SDR25_MAX_CLK;
				break;
			case XSDPS_UHS_SPEED_MODE_SDR50:
				*Arg = XSDPS_SWITCH_CMD_SDR50_SET;
				InstancePtr->BusSpeed = XSDPS_SD_SDR50_MAX_CLK;
				break;
			case XSDPS_UHS_SPEED_MODE_SDR104:
				*Arg = XSDPS_SWITCH_CMD_SDR104_SET;
				InstancePtr->BusSpeed = XSDPS_SD_SDR104_MAX_CLK;
				break;
			case XSDPS_UHS_SPEED_MODE_DDR50:
				*Arg = XSDPS_SWITCH_CMD_DDR50_SET;
				InstancePtr->BusSpeed = XSDPS_SD_DDR50_MAX_CLK;
				break;
			case XSDPS_HIGH_SPEED_MODE:
				*Arg = XSDPS_SWITCH_CMD_HS_SET;
				InstancePtr->BusSpeed = XSDPS_CLK_50_MHZ;
				break;
			default:
				Status = XST_FAILURE;
				break;
		}
	} else {
		switch (InstancePtr->Mode) {
#ifdef VERSAL_NET
			case XSDPS_HS400_MODE:
				if (InstancePtr->IsTuningDone == 0U) {
					*Arg = XSDPS_MMC_HS200_ARG;
				} else {
					*Arg = XSDPS_MMC_HS400_ARG;
				}
				InstancePtr->BusSpeed = XSDPS_MMC_HS200_MAX_CLK;
				break;
#endif
			case XSDPS_HS200_MODE:
				*Arg = XSDPS_MMC_HS200_ARG;
				InstancePtr->BusSpeed = XSDPS_MMC_HS200_MAX_CLK;
				break;
			case XSDPS_DDR52_MODE:
				*Arg = XSDPS_MMC_HIGH_SPEED_ARG;
				InstancePtr->BusSpeed = XSDPS_MMC_DDR_MAX_CLK;
				break;
			case XSDPS_HIGH_SPEED_MODE:
				*Arg = XSDPS_MMC_HIGH_SPEED_ARG;
				InstancePtr->BusSpeed = XSDPS_MMC_HSD_MAX_CLK;
				break;
			default:
				Status = XST_FAILURE;
				break;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function is used to do the DMA transfer to or from SD card.
*
* @param	InstancePtr Pointer to the instance to be worked on.
* @param	BlkCnt  Block count passed by the user.
* @param	BlkSize  Block size passed by the user.
* @param	Buff  Pointer to the data buffer for a DMA transfer.
*
* @return
* 		- XST_SUCCESS if initialization was successful
* 		- XST_FAILURE if failure - could be because another transfer
* 			is in progress or command or data inhibit is set
*
******************************************************************************/
void XSdPs_SetupReadDma(XSdPs *InstancePtr, u16 BlkCnt, u16 BlkSize, u8 *Buff)
{
	BlkSize &= XSDPS_BLK_SIZE_MASK;

	XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
			 XSDPS_BLK_SIZE_OFFSET, BlkSize);

	if (InstancePtr->Dma64BitAddr >= ADDRESS_BEYOND_32BIT) {
		XSdPs_SetupADMA2DescTbl64Bit(InstancePtr, BlkCnt);
	} else {
		XSdPs_SetupADMA2DescTbl(InstancePtr, BlkCnt, Buff);
		if (InstancePtr->Config.IsCacheCoherent == 0U) {
			Xil_DCacheInvalidateRange((INTPTR)Buff,
						  ((INTPTR)BlkCnt * (INTPTR)BlkSize));
		}
	}

	if (BlkCnt == 1U) {
		InstancePtr->TransferMode = XSDPS_TM_BLK_CNT_EN_MASK |
					    XSDPS_TM_DAT_DIR_SEL_MASK | XSDPS_TM_DMA_EN_MASK;
	} else {
		InstancePtr->TransferMode = XSDPS_TM_AUTO_CMD12_EN_MASK |
					    XSDPS_TM_BLK_CNT_EN_MASK | XSDPS_TM_DAT_DIR_SEL_MASK |
					    XSDPS_TM_DMA_EN_MASK | XSDPS_TM_MUL_SIN_BLK_SEL_MASK;
	}
}

/*****************************************************************************/
/**
* @brief
* This function is used to do the DMA transfer to or from SD card.
*
* @param	InstancePtr Pointer to the instance to be worked on.
* @param	BlkCnt  Block count passed by the user.
* @param	BlkSize  Block size passed by the user.
* @param	Buff  Pointer to the data buffer for a DMA transfer.
*
* @return
* 		- XST_SUCCESS if initialization was successful
* 		- XST_FAILURE if failure - could be because another transfer
* 			is in progress or command or data inhibit is set
*
******************************************************************************/
void XSdPs_SetupWriteDma(XSdPs *InstancePtr, u16 BlkCnt, u16 BlkSize, const u8 *Buff)
{
	BlkSize &= XSDPS_BLK_SIZE_MASK;

	XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
			 XSDPS_BLK_SIZE_OFFSET, BlkSize);

	if (InstancePtr->Dma64BitAddr >= ADDRESS_BEYOND_32BIT) {
		XSdPs_SetupADMA2DescTbl64Bit(InstancePtr, BlkCnt);
	} else {
		XSdPs_SetupADMA2DescTbl(InstancePtr, BlkCnt, Buff);
		if (InstancePtr->Config.IsCacheCoherent == 0U) {
			Xil_DCacheFlushRange((INTPTR)Buff,
					     ((INTPTR)BlkCnt * (INTPTR)BlkSize));
		}
	}

	if (BlkCnt == 1U) {
		InstancePtr->TransferMode = XSDPS_TM_BLK_CNT_EN_MASK |
					    XSDPS_TM_DMA_EN_MASK;
	} else {
		InstancePtr->TransferMode = XSDPS_TM_AUTO_CMD12_EN_MASK |
					    XSDPS_TM_BLK_CNT_EN_MASK |
					    XSDPS_TM_MUL_SIN_BLK_SEL_MASK | XSDPS_TM_DMA_EN_MASK;
	}
}

/*****************************************************************************/
/**
*
* @brief
* API to setup ADMA2 descriptor table for 32-bit DMA.
*
*
* @param	InstancePtr Pointer to the XSdPs instance.
* @param	BlkCnt Block count.
* @param	Buff Pointer to data buffer.
*
* @return	None
*
*
******************************************************************************/
void XSdPs_Setup32ADMA2DescTbl(XSdPs *InstancePtr, u32 BlkCnt, const u8 *Buff)
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
		InstancePtr->Adma2_DescrTbl32[DescNum].Address =
			(u32)((UINTPTR)Buff + ((UINTPTR)DescNum * XSDPS_DESC_MAX_LENGTH));
		InstancePtr->Adma2_DescrTbl32[DescNum].Attribute =
			XSDPS_DESC_TRAN | XSDPS_DESC_VALID;
		InstancePtr->Adma2_DescrTbl32[DescNum].Length = 0U;
	}

	InstancePtr->Adma2_DescrTbl32[TotalDescLines - 1U].Address =
		(u32)((UINTPTR)Buff + ((UINTPTR)DescNum * XSDPS_DESC_MAX_LENGTH));

	InstancePtr->Adma2_DescrTbl32[TotalDescLines - 1U].Attribute =
		XSDPS_DESC_TRAN | XSDPS_DESC_END | XSDPS_DESC_VALID;

	InstancePtr->Adma2_DescrTbl32[TotalDescLines - 1U].Length =
		(u16)((BlkCnt * BlkSize) - (u32)(DescNum * XSDPS_DESC_MAX_LENGTH));

	XSdPs_WriteReg(InstancePtr->Config.BaseAddress, XSDPS_ADMA_SAR_OFFSET,
		       (u32)((UINTPTR) & (InstancePtr->Adma2_DescrTbl32[0]) & ~(u32)0x0U));

	if (InstancePtr->Config.IsCacheCoherent == 0U) {
		Xil_DCacheFlushRange((INTPTR) & (InstancePtr->Adma2_DescrTbl32[0]),
				     (INTPTR)sizeof(XSdPs_Adma2Descriptor32) * (INTPTR)32U);
	}
}

/*****************************************************************************/
/**
*
* @brief
* API to setup ADMA2 descriptor table for 64-bit DMA.
*
*
* @param	InstancePtr Pointer to the XSdPs instance.
* @param	BlkCnt Block count.
* @param	Buff  Pointer to data buffer.
*
* @return	None
*
******************************************************************************/
void XSdPs_Setup64ADMA2DescTbl(XSdPs *InstancePtr, u32 BlkCnt, const u8 *Buff)
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
			((UINTPTR)Buff + ((UINTPTR)DescNum * XSDPS_DESC_MAX_LENGTH));
		InstancePtr->Adma2_DescrTbl64[DescNum].Attribute =
			XSDPS_DESC_TRAN | XSDPS_DESC_VALID;
		InstancePtr->Adma2_DescrTbl64[DescNum].Length = 0U;
	}

	InstancePtr->Adma2_DescrTbl64[TotalDescLines - 1U].Address =
		(u64)((UINTPTR)Buff + ((UINTPTR)DescNum * XSDPS_DESC_MAX_LENGTH));

	InstancePtr->Adma2_DescrTbl64[TotalDescLines - 1U].Attribute =
		XSDPS_DESC_TRAN | XSDPS_DESC_END | XSDPS_DESC_VALID;

	InstancePtr->Adma2_DescrTbl64[TotalDescLines - 1U].Length =
		(u16)((BlkCnt * BlkSize) - (u32)(DescNum * XSDPS_DESC_MAX_LENGTH));

#if defined(__aarch64__) || defined(__arch64__)
	XSdPs_WriteReg(InstancePtr->Config.BaseAddress, XSDPS_ADMA_SAR_EXT_OFFSET,
		       (u32)((UINTPTR)(InstancePtr->Adma2_DescrTbl64) >> 32U));
#endif

	XSdPs_WriteReg(InstancePtr->Config.BaseAddress, XSDPS_ADMA_SAR_OFFSET,
		       (u32)((UINTPTR) & (InstancePtr->Adma2_DescrTbl64[0]) & ~(u32)0x0U));

	if (InstancePtr->Config.IsCacheCoherent == 0U) {
		Xil_DCacheFlushRange((INTPTR) & (InstancePtr->Adma2_DescrTbl64[0]),
				     (INTPTR)sizeof(XSdPs_Adma2Descriptor64) * (INTPTR)32U);
	}
}

/*****************************************************************************/
/**
* @brief
* Calculates the clock divisor value.
*
* @param	InstancePtr Pointer to the instance to be worked on.
* @param	SelFreq Selected frequency
*
* @return	Clock divisor value
*
******************************************************************************/
u32 XSdPs_CalcClock(XSdPs *InstancePtr, u32 SelFreq)
{
	u16 ClockVal = 0U;
	u16 DivCnt;
	u16 Divisor = 0U;

	if (InstancePtr->HC_Version == XSDPS_HC_SPEC_V3) {
		/* Calculate divisor */
		if (InstancePtr->Config.InputClockHz <= SelFreq) {
			Divisor = 0U;
		} else {
			for (DivCnt = 2U; DivCnt <= XSDPS_CC_EXT_MAX_DIV_CNT; DivCnt += 2U) {
				if (((InstancePtr->Config.InputClockHz) / DivCnt) <= SelFreq) {
					Divisor = DivCnt >> 1;
					break;
				}
			}
		}
	} else {
		/* Calculate divisor */
		for (DivCnt = 0x1U; DivCnt <= XSDPS_CC_MAX_DIV_CNT; DivCnt <<= 1U) {
			if (((InstancePtr->Config.InputClockHz) / DivCnt) <= SelFreq) {
				Divisor = DivCnt / 2U;
				break;
			}
		}
	}

	ClockVal |= (Divisor & XSDPS_CC_SDCLK_FREQ_SEL_MASK) << XSDPS_CC_DIV_SHIFT;
	ClockVal |= ((Divisor >> 8U) & XSDPS_CC_SDCLK_FREQ_SEL_EXT_MASK) << XSDPS_CC_EXT_DIV_SHIFT;

	return ClockVal;
}

/*****************************************************************************/
/**
*
* @brief
* API to Set or Reset the DLL.
*
*
* @param	InstancePtr Pointer to the XSdPs instance.
* @param	EnRst Flag indicating whether to Assert or De-assert Reset.
*
* @return	None
*
* @note		This API is specific to ZynqMP platform.
*
******************************************************************************/
void XSdPs_DllRstCtrl(XSdPs *InstancePtr, u8 EnRst)
{
#ifndef versal
	u32 BaseAddress;
	u32 DllCtrl;

	BaseAddress = InstancePtr->Config.BaseAddress;
	if (BaseAddress == XSDPS_ZYNQMP_SD0_BASE) {
#if defined (__aarch64__) && (EL1_NONSECURE == 1)
		(void)DllCtrl;

		XSdps_Smc(InstancePtr, SD_DLL_CTRL, SD0_DLL_RST, (EnRst == 1U) ? SD0_DLL_RST : 0U);
#else
		DllCtrl = XSdPs_ReadReg(InstancePtr->SlcrBaseAddr, SD_DLL_CTRL);
		if (EnRst == 1U) {
			DllCtrl |= SD0_DLL_RST;
		} else {
			DllCtrl &= ~SD0_DLL_RST;
		}
		XSdPs_WriteReg(InstancePtr->SlcrBaseAddr, SD_DLL_CTRL, DllCtrl);
#endif
	} else {
#if defined (__aarch64__) && (EL1_NONSECURE == 1)
		(void)DllCtrl;

		XSdps_Smc(InstancePtr, SD_DLL_CTRL, SD1_DLL_RST, (EnRst == 1U) ? SD1_DLL_RST : 0U);
#else
		DllCtrl = XSdPs_ReadReg(InstancePtr->SlcrBaseAddr, SD_DLL_CTRL);
		if (EnRst == 1U) {
			DllCtrl |= SD1_DLL_RST;
		} else {
			DllCtrl &= ~SD1_DLL_RST;
		}
		XSdPs_WriteReg(InstancePtr->SlcrBaseAddr, SD_DLL_CTRL, DllCtrl);
#endif
	}
#else
	(void)InstancePtr;
	(void)EnRst;
#endif
}

/*****************************************************************************/
/**
*
* @brief
* Function to configure the Tap Delays.
*
*
* @param	InstancePtr Pointer to the XSdPs instance.
*
* @return	None
*
*
******************************************************************************/
void XSdPs_ConfigTapDelay(XSdPs *InstancePtr)
{
	u32 BaseAddress;
	u32 TapDelay;
	u32 ITapDelay;
	u32 OTapDelay;

	BaseAddress = InstancePtr->Config.BaseAddress;
	TapDelay = 0U;
	ITapDelay = InstancePtr->ITapDelay;
	OTapDelay = InstancePtr->OTapDelay;

#ifdef versal
	(void) BaseAddress;
	if (ITapDelay != 0U) {
		TapDelay = SD_ITAPCHGWIN;
		XSdPs_WriteReg(InstancePtr->Config.BaseAddress, SD_ITAPDLY, TapDelay);
		/* Program the ITAPDLY */
		TapDelay |= SD_ITAPDLYENA;
		XSdPs_WriteReg(InstancePtr->Config.BaseAddress, SD_ITAPDLY, TapDelay);
		TapDelay |= ITapDelay;
		XSdPs_WriteReg(InstancePtr->Config.BaseAddress, SD_ITAPDLY, TapDelay);
		TapDelay &= ~SD_ITAPCHGWIN;
		XSdPs_WriteReg(InstancePtr->Config.BaseAddress, SD_ITAPDLY, TapDelay);
	} else {
		XSdPs_WriteReg(InstancePtr->Config.BaseAddress, SD_ITAPDLY, 0x0);
	}

	/* Program the OTAPDLY */
	if (OTapDelay != 0U) {
		XSdPs_WriteReg(InstancePtr->Config.BaseAddress, SD_OTAPDLY, OTapDelay);
	} else {
		XSdPs_WriteReg(InstancePtr->Config.BaseAddress, SD_OTAPDLY, 0x0);
	}
#else
	if (BaseAddress == XSDPS_ZYNQMP_SD0_BASE) {
#if defined (__aarch64__) && (EL1_NONSECURE == 1)
		(void)TapDelay;
		if (ITapDelay != 0U) {
			XSdps_Smc(InstancePtr, SD_ITAPDLY, SD0_ITAPCHGWIN, SD0_ITAPCHGWIN);
			XSdps_Smc(InstancePtr, SD_ITAPDLY, SD0_ITAPDLYENA, SD0_ITAPDLYENA);
			XSdps_Smc(InstancePtr, SD_ITAPDLY, SD0_ITAPDLY_SEL_MASK, ITapDelay);
			XSdps_Smc(InstancePtr, SD_ITAPDLY, SD0_ITAPCHGWIN, 0U);
		} else {
			XSdps_Smc(InstancePtr, SD_ITAPDLY, (SD0_ITAPDLY_SEL_MASK |
							    SD0_ITAPCHGWIN | SD0_ITAPDLYENA), 0x0);
		}
		if (OTapDelay != 0U) {
			XSdps_Smc(InstancePtr, SD_OTAPDLY, SD0_OTAPDLY_SEL_MASK, OTapDelay);
		} else {
			XSdps_Smc(InstancePtr, SD_OTAPDLY, SD0_OTAPDLY_SEL_MASK, 0x0);
		}
#else
		if (ITapDelay != 0U) {
			TapDelay = XSdPs_ReadReg(InstancePtr->SlcrBaseAddr, SD_ITAPDLY);
			TapDelay |= SD0_ITAPCHGWIN;
			XSdPs_WriteReg(InstancePtr->SlcrBaseAddr, SD_ITAPDLY, TapDelay);
			/* Program the ITAPDLY */
			TapDelay |= SD0_ITAPDLYENA;
			XSdPs_WriteReg(InstancePtr->SlcrBaseAddr, SD_ITAPDLY, TapDelay);
			TapDelay &= ~SD0_ITAPDLY_SEL_MASK;
			TapDelay |= ITapDelay;
			XSdPs_WriteReg(InstancePtr->SlcrBaseAddr, SD_ITAPDLY, TapDelay);
			TapDelay &= ~SD0_ITAPCHGWIN;
			XSdPs_WriteReg(InstancePtr->SlcrBaseAddr, SD_ITAPDLY, TapDelay);
		} else {
			TapDelay = XSdPs_ReadReg(InstancePtr->SlcrBaseAddr, SD_ITAPDLY);
			TapDelay &= ~((u32)SD0_ITAPDLY_SEL_MASK | (u32)SD0_ITAPCHGWIN | (u32)SD0_ITAPDLYENA);
			XSdPs_WriteReg(InstancePtr->SlcrBaseAddr, SD_ITAPDLY, TapDelay);
		}
		if (OTapDelay != 0U) {
			/* Program the OTAPDLY */
			TapDelay = XSdPs_ReadReg(InstancePtr->SlcrBaseAddr, SD_OTAPDLY);
			TapDelay &= ~SD0_OTAPDLY_SEL_MASK;
			TapDelay |= OTapDelay;
			XSdPs_WriteReg(InstancePtr->SlcrBaseAddr, SD_OTAPDLY, TapDelay);
		} else {
			TapDelay = XSdPs_ReadReg(InstancePtr->SlcrBaseAddr, SD_OTAPDLY);
			TapDelay &= ~SD0_OTAPDLY_SEL_MASK;
			XSdPs_WriteReg(InstancePtr->SlcrBaseAddr, SD_OTAPDLY, TapDelay);
		}
#endif
	} else {
		ITapDelay = ITapDelay << 16U;
		OTapDelay = OTapDelay << 16U;
#if defined (__aarch64__) && (EL1_NONSECURE == 1)
		(void)TapDelay;
		if (ITapDelay != 0U) {
			XSdps_Smc(InstancePtr, SD_ITAPDLY, SD1_ITAPCHGWIN, SD1_ITAPCHGWIN);
			XSdps_Smc(InstancePtr, SD_ITAPDLY, SD1_ITAPDLYENA, SD1_ITAPDLYENA);
			XSdps_Smc(InstancePtr, SD_ITAPDLY, SD1_ITAPDLY_SEL_MASK, ITapDelay);
			XSdps_Smc(InstancePtr, SD_ITAPDLY, SD1_ITAPCHGWIN, 0U);
		} else {
			XSdps_Smc(InstancePtr, SD_ITAPDLY, (SD1_ITAPDLY_SEL_MASK |
							    SD1_ITAPCHGWIN | SD1_ITAPDLYENA), 0x0);
		}
		if (OTapDelay != 0U) {
			XSdps_Smc(InstancePtr, SD_OTAPDLY, SD1_OTAPDLY_SEL_MASK, OTapDelay);
		} else {
			XSdps_Smc(InstancePtr, SD_OTAPDLY, SD1_OTAPDLY_SEL_MASK, 0x0);
		}
#else
		if (ITapDelay != 0U) {
			TapDelay = XSdPs_ReadReg(InstancePtr->SlcrBaseAddr, SD_ITAPDLY);
			TapDelay |= SD1_ITAPCHGWIN;
			XSdPs_WriteReg(InstancePtr->SlcrBaseAddr, SD_ITAPDLY, TapDelay);
			/* Program the ITAPDLY */
			TapDelay |= SD1_ITAPDLYENA;
			XSdPs_WriteReg(InstancePtr->SlcrBaseAddr, SD_ITAPDLY, TapDelay);
			TapDelay &= ~SD1_ITAPDLY_SEL_MASK;
			TapDelay |= ITapDelay;
			XSdPs_WriteReg(InstancePtr->SlcrBaseAddr, SD_ITAPDLY, TapDelay);
			TapDelay &= ~SD1_ITAPCHGWIN;
			XSdPs_WriteReg(InstancePtr->SlcrBaseAddr, SD_ITAPDLY, TapDelay);
		} else {
			TapDelay = XSdPs_ReadReg(InstancePtr->SlcrBaseAddr, SD_ITAPDLY);
			TapDelay &= ~(u32)(SD1_ITAPDLY_SEL_MASK | SD1_ITAPCHGWIN | SD1_ITAPDLYENA);
			XSdPs_WriteReg(InstancePtr->SlcrBaseAddr, SD_ITAPDLY, TapDelay);
		}
		if (OTapDelay != 0U) {
			/* Program the OTAPDLY */
			TapDelay = XSdPs_ReadReg(InstancePtr->SlcrBaseAddr, SD_OTAPDLY);
			TapDelay &= ~SD1_OTAPDLY_SEL_MASK;
			TapDelay |= OTapDelay;
			XSdPs_WriteReg(InstancePtr->SlcrBaseAddr, SD_OTAPDLY, TapDelay);
		} else {
			TapDelay = XSdPs_ReadReg(InstancePtr->SlcrBaseAddr, SD_OTAPDLY);
			TapDelay &= ~SD1_OTAPDLY_SEL_MASK;
			XSdPs_WriteReg(InstancePtr->SlcrBaseAddr, SD_OTAPDLY, TapDelay);
		}
#endif
	}
#endif /* versal */
}

/*****************************************************************************/
/**
* @brief
* Sets voltage to 1.8 V.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return
* 		- XST_SUCCESS if successful
* 		- XST_FAILURE if failure
*
******************************************************************************/
s32 XSdPs_SetVoltage18(XSdPs *InstancePtr)
{
	s32 Status;
	u16 CtrlReg;

	/* Enabling 1.8V in controller */
	CtrlReg = XSdPs_ReadReg16(InstancePtr->Config.BaseAddress,
				  XSDPS_HOST_CTRL2_OFFSET);
	CtrlReg |= XSDPS_HC2_1V8_EN_MASK;
	XSdPs_WriteReg16(InstancePtr->Config.BaseAddress, XSDPS_HOST_CTRL2_OFFSET,
			 CtrlReg);

	/* Wait minimum 5mSec */
	(void)usleep(5000U);

	/* Check for 1.8V signal enable bit is cleared by Host */
	Status = XSdPs_CheckVoltage18(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief
* Configures the Power Level.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return	None
*
******************************************************************************/
void XSdPs_ConfigPower(XSdPs *InstancePtr)
{
	u8 PowerLevel;

	if ((InstancePtr->Host_Caps & XSDPS_CAP_VOLT_3V3_MASK) != 0U) {
		PowerLevel = XSDPS_PC_BUS_VSEL_3V3_MASK;
	} else if ((InstancePtr->Host_Caps & XSDPS_CAP_VOLT_3V0_MASK) != 0U) {
		PowerLevel = XSDPS_PC_BUS_VSEL_3V0_MASK;
	} else if ((InstancePtr->Host_Caps & XSDPS_CAP_VOLT_1V8_MASK) != 0U) {
		PowerLevel = XSDPS_PC_BUS_VSEL_1V8_MASK;
	} else {
		PowerLevel = 0U;
	}

	/* Select voltage based on capability and enable bus power. */
	XSdPs_WriteReg8(InstancePtr->Config.BaseAddress,
			XSDPS_POWER_CTRL_OFFSET,
			PowerLevel | XSDPS_PC_BUS_PWR_MASK);
}

/*****************************************************************************/
/**
* @brief
* Configures the DMA.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return	None
*
******************************************************************************/
void XSdPs_ConfigDma(XSdPs *InstancePtr)
{
	if (InstancePtr->HC_Version == XSDPS_HC_SPEC_V3) {
		/* Enable ADMA2 in 64bit mode. */
		XSdPs_WriteReg8(InstancePtr->Config.BaseAddress,
				XSDPS_HOST_CTRL1_OFFSET,
				XSDPS_HC_DMA_ADMA2_64_MASK);
	} else {
		/* Enable ADMA2 in 32bit mode. */
		XSdPs_WriteReg8(InstancePtr->Config.BaseAddress,
				XSDPS_HOST_CTRL1_OFFSET,
				XSDPS_HC_DMA_ADMA2_32_MASK);
	}
}

/*****************************************************************************/
/**
* @brief
* Configures the Interrupts.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return	None
*
******************************************************************************/
void XSdPs_ConfigInterrupt(XSdPs *InstancePtr)
{
	/* Enable all interrupt status except card interrupt initially */
	XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
			 XSDPS_NORM_INTR_STS_EN_OFFSET,
			 XSDPS_NORM_INTR_ALL_MASK & (~XSDPS_INTR_CARD_MASK));

	XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
			 XSDPS_ERR_INTR_STS_EN_OFFSET,
			 XSDPS_ERROR_INTR_ALL_MASK);

	/* Disable all interrupt signals by default. */
	XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
			 XSDPS_NORM_INTR_SIG_EN_OFFSET, 0x0U);
	XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
			 XSDPS_ERR_INTR_SIG_EN_OFFSET, 0x0U);

}

/*****************************************************************************/
/**
* Generates SD command.
*
* @param	InstancePtr Pointer to the instance to be worked on.
* @param	Cmd Command to be sent.
* @param	Arg Argument to be sent along with the command.
* 		This could be address or any other information
* @param	BlkCnt Block count passed by the user.
*
* @return
* 		- XST_SUCCESS if initialization was successful
* 		- XST_FAILURE if failure - could be because another transfer
* 			is in progress or command or data inhibit is set
*
******************************************************************************/
s32 XSdPs_CmdTransfer(XSdPs *InstancePtr, u32 Cmd, u32 Arg, u32 BlkCnt)
{
	u32 Timeout = 10000000U;
	u32 StatusReg;
	s32 Status;
	u32 Mask;

	Status = XSdPs_SetupCmd(InstancePtr, Arg, BlkCnt);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	Status = XSdPs_SendCmd(InstancePtr, Cmd);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Polling for response for now */
	Mask = XSDPS_INTR_ERR_MASK | XSDPS_INTR_CC_MASK;
	if ((Cmd == CMD21) || (Cmd == CMD19)) {
		Mask |= XSDPS_INTR_BRR_MASK;
	}

	Status = Xil_WaitForEvents(InstancePtr->Config.BaseAddress + XSDPS_NORM_INTR_STS_OFFSET,
				   Mask, Mask, Timeout, &StatusReg);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	if (((Cmd == CMD21) || (Cmd == CMD19)) && (StatusReg & XSDPS_INTR_BRR_MASK) != 0U) {
		XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
				 XSDPS_NORM_INTR_STS_OFFSET, XSDPS_INTR_BRR_MASK);
	}
	if ((StatusReg & XSDPS_INTR_ERR_MASK) != 0U) {
		Status = (s32)XSdPs_ReadReg16(InstancePtr->Config.BaseAddress,
					      XSDPS_ERR_INTR_STS_OFFSET);
		if (((u32)Status & ~XSDPS_INTR_ERR_CT_MASK) == 0U) {
			Status = XSDPS_CT_ERROR;
		}
		/* Write to clear error bits */
		XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
				 XSDPS_ERR_INTR_STS_OFFSET,
				 XSDPS_ERROR_INTR_ALL_MASK);
		goto RETURN_PATH;
	}

	/* Write to clear bit */
	XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
			 XSDPS_NORM_INTR_STS_OFFSET,
			 XSDPS_INTR_CC_MASK);

	Status = XST_SUCCESS;

RETURN_PATH:
	return Status;

}

/*****************************************************************************/
/**
* Checks if the transfer is completed successfully.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return	None
*
******************************************************************************/
s32 XSdps_CheckTransferDone(XSdPs *InstancePtr)
{
	u32 Timeout = 5000000U;
	u32 StatusReg;
	s32 Status;
	u32 Mask;

	/*
	 * Check for transfer complete
	 * Polling for response for now
	 */
	Mask = XSDPS_INTR_ERR_MASK | XSDPS_INTR_TC_MASK;
	Status = Xil_WaitForEvents(InstancePtr->Config.BaseAddress + XSDPS_NORM_INTR_STS_OFFSET,
				   Mask, Mask, Timeout, &StatusReg);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	if ((StatusReg & XSDPS_INTR_ERR_MASK) != 0U) {
		/* Write to clear error bits */
		XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
				 XSDPS_ERR_INTR_STS_OFFSET,
				 XSDPS_ERROR_INTR_ALL_MASK);
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	/* Write to clear bit */
	XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
			 XSDPS_NORM_INTR_STS_OFFSET, XSDPS_INTR_TC_MASK);

	Status = XST_SUCCESS;

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Checks if the CMD/DATA bus is idle or not.
*
* @param	InstancePtr Pointer to the instance to be worked on.
* @param	Value Value to be selected Cmd bus or Dat bus.
*
* @return	None
*
******************************************************************************/
s32 XSdPs_CheckBusIdle(XSdPs *InstancePtr, u32 Value)
{
	u32 Timeout = 10000000U;
	u32 PresentStateReg;
	s32 Status;

	PresentStateReg = XSdPs_ReadReg(InstancePtr->Config.BaseAddress,
					XSDPS_PRES_STATE_OFFSET);
	/* Check for Card Present */
	if ((PresentStateReg & XSDPS_PSR_CARD_INSRT_MASK) != 0U) {
		/* Check for SD idle */
		Status = Xil_WaitForEvent(InstancePtr->Config.BaseAddress + XSDPS_PRES_STATE_OFFSET,
					  Value, 0U, Timeout);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto RETURN_PATH ;
		}
	}
	Status = XST_SUCCESS;

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function frames the Command register for a particular command.
*
*
* @param	InstancePtr Pointer to the instance to be worked on.
* @param	Cmd Command to be sent.
*
* @return	Command register value complete with response type and
* 		data, CRC and index related flags.
*
* @note This generates only the command register value i.e.
* the upper 16 bits of the transfer mode and command register.
* This value is already shifted to be upper 16 bits and can be directly
* OR'ed with transfer mode register value.
*
******************************************************************************/
u32 XSdPs_FrameCmd(XSdPs *InstancePtr, u32 Cmd)
{
	u32 RetVal;

	RetVal = Cmd;

	switch (Cmd) {
		case CMD0:
		case CMD4:
			RetVal |= RESP_NONE;
			break;
		case CMD1:
		case ACMD41:
			RetVal |= RESP_R3;
			break;
		case CMD2:
		case CMD9:
			RetVal |= RESP_R2;
			break;
		case CMD3:
			if (InstancePtr->CardType == XSDPS_CARD_SD) {
				RetVal |= RESP_R6;
			} else {
				RetVal |= RESP_R1;
			}
			break;
		case CMD5:
		case CMD38:
			RetVal |= RESP_R1B;
			break;
		case CMD6:
			if (InstancePtr->CardType == XSDPS_CARD_SD) {
				RetVal |= RESP_R1 | (u32)XSDPS_DAT_PRESENT_SEL_MASK;
			} else {
				RetVal |= RESP_R1B;
			}
			break;
		case ACMD6:
		case CMD7:
		case CMD10:
		case CMD11:
		case CMD12:
		case CMD16:
		case CMD32:
		case CMD33:
		case CMD35:
		case CMD36:
		case ACMD42:
		case CMD52:
		case CMD55:
			RetVal |= RESP_R1;
			break;
		case CMD8:
			if (InstancePtr->CardType == XSDPS_CARD_SD) {
				RetVal |= RESP_R1;
			} else {
				RetVal |= RESP_R1 | (u32)XSDPS_DAT_PRESENT_SEL_MASK;
			}
			break;
		case ACMD13:
		case CMD17:
		case CMD18:
		case CMD19:
		case CMD21:
		case CMD23:
		case ACMD23:
		case CMD24:
		case CMD25:
		case ACMD51:
			RetVal |= RESP_R1 | (u32)XSDPS_DAT_PRESENT_SEL_MASK;
			break;
		case CMD58:
			break;
		default :
			RetVal |= Cmd;
			break;
	}

	return RetVal;
}

#ifdef VERSAL_NET
/*****************************************************************************/
/**
* @brief
* Selects the HS400 timing mode.
*
* @param	InstancePtr Pointer to the instance to be worked on.
*
* @return	- XST_SUCCESS if successful
* 			- XST_FAILURE if failure occurred.
*
******************************************************************************/
u32 XSdPs_Select_HS400(XSdPs *InstancePtr)
{
	u32 Status;
	u32 StatusReg;

	InstancePtr->Mode = XSDPS_HIGH_SPEED_MODE;
	Status = XSdPs_Change_MmcBusSpeed(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	InstancePtr->OTapDelay = SD_OTAPDLYSEL_EMMC_HSD;
	Status = XSdPs_Change_ClkFreq(InstancePtr, InstancePtr->BusSpeed);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	InstancePtr->Mode = XSDPS_HS400_MODE;
	Status = XSdPs_Change_BusWidth(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	Status = XSdPs_Change_MmcBusSpeed(InstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	if (InstancePtr->Config.OTapDly_DDR_Clk200 != 0U){
		InstancePtr->OTapDelay = InstancePtr->Config.OTapDly_DDR_Clk200;
	} else {
		InstancePtr->OTapDelay = SD_OTAPDLYSEL_HS400;
	}
	Status = XSdPs_Change_ClkFreq(InstancePtr, InstancePtr->BusSpeed);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto RETURN_PATH;
	}

	StatusReg = XSdPs_ReadReg16(InstancePtr->Config.BaseAddress,
				    XSDPS_HOST_CTRL2_OFFSET);
	StatusReg &= (~(u32)XSDPS_HC2_UHS_MODE_MASK);
	StatusReg |= XSDPS_HC2_HS400_MASK;
	XSdPs_WriteReg16(InstancePtr->Config.BaseAddress,
			 XSDPS_HOST_CTRL2_OFFSET, (u16)StatusReg);

RETURN_PATH:
	return Status;
}
#endif

/** @} */
