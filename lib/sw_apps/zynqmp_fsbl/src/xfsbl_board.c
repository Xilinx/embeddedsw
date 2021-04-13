/******************************************************************************
* Copyright (c) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xfsbl_board.c
*
* This file contains board specific code of FSBL.
* Board specific code for ZCU106 is similar to that of ZCU102, except that
* GT mux configuration and PCIe reset are not applicable for ZCU106.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  ssc  01/20/16 Initial release
* 2.0   bv   12/05/16 Made compliance to MISRAC 2012 guidelines
*                     Added ZCU106 support
* 3.0	bkm  04/18/18 Added Board specific code w.r.t VADJ
* 4.0   bsv  11/12/19 Added support for ZCU216 board
*       bsv  02/05/20 Added support for ZCU208 board
* 5.0   bsv  04/12/21 Removed unwanted I2C writes to TCA6416A
*                     for ZCU208 and ZCU216 boards
*
* </pre>
*
* @note
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xfsbl_board.h"
#if defined(XPS_BOARD_ZCU102) || defined(XPS_BOARD_ZCU106)		\
		|| defined(XPS_BOARD_ZCU104) || defined(XPS_BOARD_ZCU111) \
		|| defined(XPS_BOARD_ZCU216) || defined(XPS_BOARD_ZCU208)
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
#if defined(XPS_BOARD_ZCU104) || defined(XPS_BOARD_ZCU216) || \
	defined(XPS_BOARD_ZCU208)
static u32 XFsbl_ReadMinMaxEepromVadj(XIicPs* I2c0InstancePtr, u32 *MinVadj, u32 *MaxVadj);
static u32 XFsbl_CalVadj(u16 MinVoltage, u16 MaxVoltage);
#endif
static u32 XFsbl_BoardConfig(void);
static u32 XFsbl_FMCEnable(XIicPs* I2c0InstancePtr, XIicPs* I2c1InstancePtr);
#if defined(XPS_BOARD_ZCU102)
static void XFsbl_PcieReset(void);
#endif
/************************** Variable Definitions *****************************/
#if defined(XPS_BOARD_ZCU104) || defined(XPS_BOARD_ZCU216) || \
	defined(XPS_BOARD_ZCU208)
/*****************************************************************************/
/**
 * This function is used Read the min and max VADJ values from the FMC EEPROM.
 * These values of min and max VADJ are present in DC Load section of
 * MULTIRECORD AREA in FMC EEPROM.
 *
 *
 *
 * @param u32 *MinVadj, u32 *MaxVadj
 *
 * @return none
 *
 *****************************************************************************/
static u32 XFsbl_ReadMinMaxEepromVadj(XIicPs* I2c0InstancePtr, u32 *MinVadj, u32 *MaxVadj)
{
	u32 Count;
	u32 EepromByteCount;
	XMultipleRecord XRecord;
	u8 WriteBuffer[BUF_LEN] = {0U};
	u8 Read_Buffer[MAX_SIZE]= {0U};
	u32 UStatus;
	s32 Status;
	u32 NominalVoltage;
	u32 EepromAddr = 0x54U;
	u32 MinVoltage;
	u32 MaxVoltage;

	EepromByteCount = MAX_SIZE;
	MinVoltage = 0U;
	MaxVoltage = 0U;
	XRecord.VadjRecordFound = 0U;

	/* Select the Channel-1 of MUX for I2C EEprom Access */
	WriteBuffer[0U] = 0x1U;
	Status = XIicPs_MasterSendPolled(I2c0InstancePtr,
				WriteBuffer, 1U, TCA9548A_ADDR);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle */
	while (XIicPs_BusIsBusy(I2c0InstancePtr) == TRUE) {
		/** For MISRA-C compliance */
	}

	/* Read the contents of FMC EEPROM to Read_Buffer */
		Status = XIicPs_MasterRecvPolled(I2c0InstancePtr, Read_Buffer,
			EepromByteCount, EepromAddr);
		if (Status == XST_SUCCESS) {
			UStatus = XSFBL_EEPROM_PRESENT;
			/* Wait until bus is idle */
			while (XIicPs_BusIsBusy(I2c0InstancePtr) == TRUE) {
					/** For MISRA-C compliance */
			}
			XFsbl_Printf(DEBUG_GENERAL, "XFSBL_EEPROM PRESENT\r\n");
		}
		else
		{
			UStatus = XFSBL_FAILURE;
			goto END;
		}

    /**
     * Read the value of Nomianal Voltage
     * Minimum voltage and Maximum voltage from the
     * IPMI EEPROM DC Load multiple record area at offset 0x02
     */
	if ((Read_Buffer[0U] == 0x01U) && (Read_Buffer[5U] != 0x00U)) {
		XRecord.MultirecordHdrOff = Read_Buffer[5U] * 8U;
		do {
			XRecord.RecordType = Read_Buffer[XRecord.MultirecordHdrOff];
			XRecord.MultirecordHdrEol = (Read_Buffer
					[XRecord.MultirecordHdrOff + 1U] & 0x80U);
			XRecord.RecordLength = Read_Buffer[XRecord.MultirecordHdrOff
							+ 2U];

			if (XRecord.RecordType == DC_LOAD) {
				XRecord.OutputNumber = Read_Buffer
						[XRecord.MultirecordHdrOff +
						MULTIRECORD_HEADER_SIZE] & 0x0FU;
				if (XRecord.OutputNumber == 0x00U) {
					XRecord.VadjRecordFound = 1U;
				XRecord.VadjHdrOffset = XRecord.MultirecordHdrOff;
					XRecord.VadjDataOffset = XRecord.VadjHdrOffset
						+ MULTIRECORD_HEADER_SIZE;
					break;
				}
			}
			XRecord.MultirecordHdrOff += (XRecord.RecordLength +
					MULTIRECORD_HEADER_SIZE);
		} while (XRecord.MultirecordHdrEol == 0x00U);

	if (XRecord.VadjRecordFound == 1U) {
		Count = XRecord.VadjDataOffset + 1U;
		NominalVoltage = ((((u16)Read_Buffer[Count+1U]<<8U) & 0xFF00U)
				| Read_Buffer[Count]);
		NominalVoltage = NominalVoltage * 10U;

		Count = Count + 2U;
		MinVoltage = ((((u16)Read_Buffer[Count+1U]<<8U) & 0xFF00U)
				| Read_Buffer[Count]) * 10U;


		Count = Count + 2U;
		MaxVoltage = ((((u16)Read_Buffer[Count+1U]<<8U) & 0xFF00U)
				| Read_Buffer[Count]) * 10U;
		}

	}

	*MinVadj = MinVoltage;
	*MaxVadj = MaxVoltage;
	UStatus = XFSBL_SUCCESS;
END:
	return UStatus;
}
/*****************************************************************************/
/**
 * This function is used to calculates V_ADJ value based on the min and
 * max VADJ values from the FMC EEPROM.
 *
 * @param u16 MinVoltage, u16 MaxVoltage
 *
 * @return u32 VadjValue
 *
 *****************************************************************************/
static u32 XFsbl_CalVadj(u16 MinVoltage, u16 MaxVoltage)
{
	u32 VadjValue;

	if ((MinVoltage <= 1800U) && (MaxVoltage >= 1800U)) {
		VadjValue = SET_VADJ_1V8;
		XFsbl_Printf(DEBUG_GENERAL, "Calc_Vadj is VADJ_1V8\r\n");
	} else if ((MinVoltage <= 1500U) && (MaxVoltage >= 1500U)) {
		VadjValue = SET_VADJ_1V5;
		XFsbl_Printf(DEBUG_GENERAL, "Calc_Vadj is VADJ_1V5\r\n");
	} else if ((MinVoltage <= 1200U) && (MaxVoltage >= 1200U)) {
		VadjValue = SET_VADJ_1V2;
		XFsbl_Printf(DEBUG_GENERAL, "Calc_Vadj is VADJ_1V2\r\n");
	} else {
		VadjValue = SET_VADJ_0V0;
		XFsbl_Printf(DEBUG_GENERAL, "Calc_Vadj is VADJ_0V0\r\n");
	}

	return(VadjValue);
}
#endif
/*****************************************************************************/
/**
 * This function is used to Enable FMC_ADJ .
 * It also provides VADJ_FMC Rail Voltage to 1.2V default in ZCU104
 *
 *This function does
 *	if( EEPROM PRESENT )
 *		- Reading the FMC EEPROM minimum and maximum VADJ values
 *		- Calculate the VADJ values
 *		- Programming the VADJ rail to Calculated VADJ
 *	if( EEPROM IS BLANK )
 *	-	Programming the VADJ rail to 1.2v
 * @param I2c Instance Pointer
 *
 * @return
 *	- XFSBL_SUCCESS for successful configuration
 *	- errors as mentioned in xfsbl_error.h
 *
 *****************************************************************************/
static u32 XFsbl_FMCEnable(XIicPs* I2c0InstancePtr, XIicPs* I2c1InstancePtr)
{
	u8 WriteBuffer[BUF_LEN] = {0U};
	s32 Status;
	u32 UStatus;
	u32 SlaveAddr;
#ifndef XPS_BOARD_ZCU216
#ifndef XPS_BOARD_ZCU208
	(void) I2c1InstancePtr;
#endif
#endif
#if defined(XPS_BOARD_ZCU104) || defined(XPS_BOARD_ZCU111) || \
	defined(XPS_BOARD_ZCU216)|| defined(XPS_BOARD_ZCU208)
	XVoutCommands *VoutPtr;
	u32 VadjSetting = SET_VADJ_0V0;
	/**
	 * Lookup table for Vout_Cmd, Vout Margin High,Vout_OV_Warn_Limit,
	 * Vout_OV_Fault_Limit, Vout Margin_Low, Vout_UV_Warn_Limit
	 * Vout_UV_Fault_Limit based on Index which is similar to
	 * VadjSetting
	 */
	XVoutCommands LookupTable[] = {
		{
				VOUT_CMDL_1V2, VOUT_CMDH_1V2, VOUT_OV_WARNL_1V2,
				VOUT_OV_WARNH_1V2, VOUT_OV_FAULTL_1V2, VOUT_OV_FAULTH_1V2,
				VOUT_UV_WARNL_1V2, VOUT_UV_WARNH_1V2, VOUT_UV_FAULTL_1V2,
				VOUT_UV_FAULTH_1V2},
		{
				VOUT_CMDL_1V2, VOUT_CMDH_1V2, VOUT_OV_WARNL_1V2,
				VOUT_OV_WARNH_1V2, VOUT_OV_FAULTL_1V2, VOUT_OV_FAULTH_1V2,
				VOUT_UV_WARNL_1V2, VOUT_UV_WARNH_1V2, VOUT_UV_FAULTL_1V2,
				VOUT_UV_FAULTH_1V2},
		{
				VOUT_CMDL_1V5, VOUT_CMDH_1V5, VOUT_OV_WARNL_1V5,
				VOUT_OV_WARNH_1V5, VOUT_OV_FAULTL_1V5, VOUT_OV_FAULTH_1V5,
				VOUT_UV_WARNL_1V5, VOUT_UV_WARNH_1V5, VOUT_UV_FAULTL_1V5,
				VOUT_UV_FAULTH_1V5},
		{
				VOUT_CMDL_1V8, VOUT_CMDH_1V8, VOUT_OV_WARNL_1V8,
				VOUT_OV_WARNH_1V8, VOUT_OV_FAULTL_1V8, VOUT_OV_FAULTH_1V8,
				VOUT_UV_WARNL_1V8, VOUT_UV_WARNH_1V8, VOUT_UV_FAULTL_1V8,
				VOUT_UV_FAULTH_1V8}
	};
#if defined(XPS_BOARD_ZCU104) || defined(XPS_BOARD_ZCU216)|| \
	defined(XPS_BOARD_ZCU208)
	u32 LpcMin;
	u32 LpcMax;
#endif
#endif

	/* Change the IIC serial clock rate */
	Status = XIicPs_SetSClk(I2c0InstancePtr, IIC_SCLK_RATE_I2CMUX);
	if (Status != XST_SUCCESS) {
		UStatus  = XFSBL_ERROR_I2C_SET_SCLK;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_SET_SCLK\r\n");
	}
#if defined(XPS_BOARD_ZCU216) || defined(XPS_BOARD_ZCU208)
	Status = XIicPs_SetSClk(I2c1InstancePtr, IIC_SCLK_RATE_I2CMUX);
	if (Status != XST_SUCCESS) {
		UStatus  = XFSBL_ERROR_I2C_SET_SCLK;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_SET_SCLK\r\n");
		goto END;
	}
#endif
#if defined(XPS_BOARD_ZCU104) || defined(XPS_BOARD_ZCU216) || \
	defined(XPS_BOARD_ZCU208)
#if defined(XPS_BOARD_ZCU104)
	UStatus = XFsbl_ReadMinMaxEepromVadj(I2c0InstancePtr, &LpcMin, &LpcMax);
#else
	UStatus = XFsbl_ReadMinMaxEepromVadj(I2c1InstancePtr, &LpcMin, &LpcMax);
#endif
	if(UStatus != XFSBL_SUCCESS)
	{
		goto END;
	}
	VadjSetting = XFsbl_CalVadj(LpcMin, LpcMax);
#endif

#if defined(XPS_BOARD_ZCU102) || defined(XPS_BOARD_ZCU111) || \
	defined(XPS_BOARD_ZCU106) || defined(XPS_BOARD_ZCU216) \
	|| defined(XPS_BOARD_ZCU208)
	/* Set I2C Mux for channel-2 */
	WriteBuffer[0U] = CMD_CH_2_REG;
	SlaveAddr = PCA9544A_ADDR;
#else //ZCU104
	/* Set I2C Mux for channel-2 (IRPS5401) */
	WriteBuffer[0U] = CMD_CH_2_REG_IRPS;
	SlaveAddr = TCA9548A_ADDR;
#endif

	Status = XIicPs_MasterSendPolled(I2c0InstancePtr,
			WriteBuffer, 1U, SlaveAddr);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle */
	while (XIicPs_BusIsBusy(I2c0InstancePtr) == TRUE) {
		/** For MISRA-C compliance */
	}

	/**
	 * The below piece of code is needed for PL DDR to work
	 * (to take PL DDR out of reset). Hence including this code only when
	 * PL DDR is in design.
	 */
#if defined(XPS_BOARD_ZCU102) || defined(XPS_BOARD_ZCU106)
#ifdef XPAR_MIG_0_BASEADDR
	/* Enable Regulator (FMC ADJ) ZCU102 */
	WriteBuffer[0U] = CMD_ON_OFF_CFG;
	WriteBuffer[1U] = ON_OFF_CFG_VAL;
	SlaveAddr = MAX15301_ADDR;

	Status = XIicPs_MasterSendPolled(I2c0InstancePtr,
			WriteBuffer, 2U, SlaveAddr);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}
	/* Wait until bus is idle */
	while (XIicPs_BusIsBusy(I2c0InstancePtr) == TRUE) {
		/** For MISRA-C compliance */
	}
#endif
#endif

#if defined(XPS_BOARD_ZCU104) || defined(XPS_BOARD_ZCU111) || \
	defined(XPS_BOARD_ZCU216)|| defined(XPS_BOARD_ZCU208)
	/* PMbus Command for Page Selection */
	WriteBuffer[0U] = CMD_PAGE_CFG;
#ifdef XPS_BOARD_ZCU104
	/* Page-3 for SW-D in ZCU104 */
	WriteBuffer[1U] = DATA_SWD_CFG;
	SlaveAddr = IRPS5401_ADDR;
#else
	/* Page-2 for SW-C in ZCU111 */
	WriteBuffer[1U] = DATA_SWC_CFG;
	SlaveAddr = IRPS5401_SWC_ADDR;
#endif
	Status = XIicPs_MasterSendPolled(I2c0InstancePtr,
			WriteBuffer, 2U, SlaveAddr);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle */
	while (XIicPs_BusIsBusy(I2c0InstancePtr) == TRUE) {
		/** For MISRA-C compliance */
	}

	/* Operation Command For ON and OFF and Control Margining*/
	WriteBuffer[0U] = CMD_OPERATION_CFG;
	WriteBuffer[1U] = OPERATION_VAL;
	Status = XIicPs_MasterSendPolled(I2c0InstancePtr,
			WriteBuffer, 2U, SlaveAddr);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle */
	while (XIicPs_BusIsBusy(I2c0InstancePtr) == TRUE) {
		/** For MISRA-C compliance */
	}

	/**
	 *  Sets the format for VOUT related commands. Linear mode,
	 * -8, -9, and -12 exponents supported. No LDO support
	 *
	 */
	WriteBuffer[0U] = CMD_VOUT_MODE_CFG;
	WriteBuffer[1U] = DATA_VOUT_MODE_VAL;
	Status = XIicPs_MasterSendPolled(I2c0InstancePtr,
			WriteBuffer, 2U, SlaveAddr);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle */
	while (XIicPs_BusIsBusy(I2c0InstancePtr) == TRUE) {
		/** For MISRA-C compliance */
	}

	/**
	 * PMbus Command for VOUT_MAX Sets an upper limit on the output
	 * voltage the unit can command. Format according to VOUT_MODE
	 */
	WriteBuffer[0U] = CMD_VOUT_MAX_CFG;
	WriteBuffer[1U] = DATA_VOUT_MAX_VAL_L;
	WriteBuffer[2U] = DATA_VOUT_MAX_VAL_H;
	Status = XIicPs_MasterSendPolled(I2c0InstancePtr,
			WriteBuffer, 3U, SlaveAddr);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle */
	while (XIicPs_BusIsBusy(I2c0InstancePtr) == TRUE) {
		/** For MISRA-C compliance */
	}

	VoutPtr = &LookupTable[VadjSetting];
	/**
	 * PMbus Command for Vout CMD
	 * This is a command that is used to set the
	 * output voltage when the OPERATION command
	 * is set to ON without margining
	 * Format according to VOUT_MODE
	 */

	WriteBuffer[0U] = CMD_VOUT_CMD_CFG;
	WriteBuffer[1U] = VoutPtr->VoutCmdL;
	WriteBuffer[2U] = VoutPtr->VoutCmdH;
	Status = XIicPs_MasterSendPolled(I2c0InstancePtr,
				WriteBuffer, 3U, SlaveAddr);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL,
			"XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle */
	while (XIicPs_BusIsBusy(I2c0InstancePtr) == TRUE) {
		/** For MISRA-C compliance */
	}

	/**
	 * The VOUT_OV_WARN_LIMIT command
	 * sets threshold for the output voltage high warning.
	 * Masked until the unit reaches the programmed
	 * output voltage. Formatted according to the setting
	 * of the VOUT_MODE command.
	 */
	WriteBuffer[0U] = CMD_VOUT_OV_WARN_LIMIT;
	WriteBuffer[1U] = VoutPtr->VoutOvWarnL;
	WriteBuffer[2U] = VoutPtr->VoutOvWarnH;
	Status = XIicPs_MasterSendPolled(I2c0InstancePtr,
				WriteBuffer, 3U, SlaveAddr);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL,
			"XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle */
	while (XIicPs_BusIsBusy(I2c0InstancePtr) == TRUE) {
		/** For MISRA-C compliance */
	}

	/**
	 *  The VOUT_OV_FAULT_LIMIT command
	 *  Sets threshold for the output voltage high fault.
	 *  Masked until the unit reaches the programmed
	 *  output voltage. Formatted according to the
	 *  setting of the VOUT_MODE command.
	 */
	WriteBuffer[0U] = CMD_VOUT_OV_FAULT_LIMIT;
	WriteBuffer[1U] = VoutPtr->VoutOvFaultL;
	WriteBuffer[2U] = VoutPtr->VoutOvFaultH;
	Status = XIicPs_MasterSendPolled(I2c0InstancePtr,
				WriteBuffer, 3U, SlaveAddr);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL,
			"XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/**
	 * The VOUT_UV_WARN_LIMIT command
	 * sets threshold for the output voltage low warning.
	 * Masked until the unit reaches the programmed
	 * output voltage. Formatted according to the setting
	 * of theVOUT_MODE command.
	 */
	WriteBuffer[0U] = CMD_VOUT_UV_WARN_LIMIT;
	WriteBuffer[1U] = VoutPtr->VoutUvWarnL;
	WriteBuffer[2U] = VoutPtr->VoutUvWarnH;
	Status = XIicPs_MasterSendPolled(I2c0InstancePtr,
				WriteBuffer, 3U, SlaveAddr);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL,
			"XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle */
	while (XIicPs_BusIsBusy(I2c0InstancePtr) == TRUE) {
		/** For MISRA-C compliance */
	}

	/**
	 * The VOUT_UV_FAULT_LIMIT command
	 * Sets threshold for the output voltage low fault.
	 * Masked until the unit reaches the programmed
	 * output voltage. Formatted according to the
	 * setting of the VOUT_MODE command.
	 */
	WriteBuffer[0U] = CMD_VOUT_UV_FAULT_LIMIT;
	WriteBuffer[1U] = VoutPtr->VoutUvFaultL;
	WriteBuffer[2U] = VoutPtr->VoutUvFaultH;
	Status = XIicPs_MasterSendPolled(I2c0InstancePtr,
				WriteBuffer, 3U, SlaveAddr);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL,
			     "XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle */
	while (XIicPs_BusIsBusy(I2c0InstancePtr) == TRUE) {
		/** For MISRA-C compliance */
	}

#endif
	UStatus = XFSBL_SUCCESS;
	XFsbl_Printf(DEBUG_INFO, "FMC VADJ Configuration Successful\n\r");


END:

	return UStatus;
}
/*****************************************************************************/
/**
 * This function is used to perform GT configuration for ZCU102 board.
 * It also provides reset to GEM, enables FMC ADJ
 *
 * @param none
 *
 * @return
 * 		- XFSBL_SUCCESS for successful configuration
 * 		- errors as mentioned in xfsbl_error.h
 *
 *****************************************************************************/
static u32 XFsbl_BoardConfig(void)
{
	XIicPs I2c0Instance, I2c1Instance;
	XIicPs_Config *I2c0CfgPtr;
	s32 Status;
	u32 UStatus;
#if defined(XPS_BOARD_ZCU102) || defined(XPS_BOARD_ZCU106)
	u8 WriteBuffer[BUF_LEN] = {0U};
#endif
#if defined(XPS_BOARD_ZCU216) || defined(XPS_BOARD_ZCU208)
	XIicPs_Config *I2c1CfgPtr;
#endif

#if defined(XPS_BOARD_ZCU102)
	u32 ICMCfgLane[NUM_GT_LANES];
#endif

	/* Initialize the IIC0 driver so that it is ready to use */
	I2c0CfgPtr = XIicPs_LookupConfig(XPAR_XIICPS_0_DEVICE_ID);
	if (I2c0CfgPtr == NULL) {
		UStatus = XFSBL_ERROR_I2C_INIT;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_INIT\r\n");
		goto END;
	}

	Status = XIicPs_CfgInitialize(&I2c0Instance, I2c0CfgPtr,
			I2c0CfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_INIT;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_INIT\r\n");
		goto END;
	}

#if defined(XPS_BOARD_ZCU216) || defined(XPS_BOARD_ZCU208)
	/* Initialize the IIC1 driver so that it is ready to use */
	I2c1CfgPtr = XIicPs_LookupConfig(XPAR_XIICPS_1_DEVICE_ID);
	if (I2c1CfgPtr == NULL) {
		UStatus = XFSBL_ERROR_I2C_INIT;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_INIT\r\n");
		goto END;
	}
	Status = XIicPs_CfgInitialize(&I2c1Instance, I2c1CfgPtr,
			I2c1CfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_INIT;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_INIT\r\n");
		goto END;
	}
#endif

#if defined(XPS_BOARD_ZCU102) || defined(XPS_BOARD_ZCU106)
	/* Set the IIC serial clock rate */
	Status = XIicPs_SetSClk(&I2c0Instance, IIC_SCLK_RATE_IOEXP);
	if (Status != XST_SUCCESS) {
		UStatus  = XFSBL_ERROR_I2C_SET_SCLK;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_SET_SCLK\r\n");
		goto END;
	}
	/* Configure I/O pins as Output */
	WriteBuffer[0U] = CMD_CFG_0_REG;
	WriteBuffer[1U] = DATA_OUTPUT;
	Status = XIicPs_MasterSendPolled(&I2c0Instance,
		WriteBuffer, 2U, IOEXPANDER1_ADDR);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle to start another transfer */
	while (XIicPs_BusIsBusy(&I2c0Instance) == TRUE) {
		/*For MISRA C compliance*/
	}

	/*
	 * Deasserting I2C_MUX_RESETB
	 * And GEM3 Resetb
	 * Selecting lanes based on configuration
	 */
	WriteBuffer[0U] = CMD_OUTPUT_0_REG;
	WriteBuffer[1U] = DATA_COMMON_CFG;

#if defined(XPS_BOARD_ZCU102)
	ICMCfgLane[0U] = XFsbl_In32(SERDES_ICM_CFG0) & SERDES_ICM_CFG0_L0_ICM_CFG_MASK;
	ICMCfgLane[1U] = (XFsbl_In32(SERDES_ICM_CFG0) &
		SERDES_ICM_CFG0_L1_ICM_CFG_MASK) >> SERDES_ICM_CFG0_L1_ICM_CFG_SHIFT;
	ICMCfgLane[2U] = XFsbl_In32(SERDES_ICM_CFG1) & (SERDES_ICM_CFG1_L2_ICM_CFG_MASK);
	ICMCfgLane[3U] = (XFsbl_In32(SERDES_ICM_CFG1) &
		SERDES_ICM_CFG1_L3_ICM_CFG_MASK) >> SERDES_ICM_CFG1_L3_ICM_CFG_SHIFT;

	/* For ZCU102 board, check if GT combination is valid against the lane# */
	if (((ICMCfgLane[0U] != ICM_CFG_VAL_PCIE)
			&& (ICMCfgLane[0U] != ICM_CFG_VAL_DP)
			&& (ICMCfgLane[0U] != ICM_CFG_VAL_PWRDN)) ||
		((ICMCfgLane[1U] != ICM_CFG_VAL_PCIE)
			&& (ICMCfgLane[1U] != ICM_CFG_VAL_DP)
			&& (ICMCfgLane[1U] != ICM_CFG_VAL_PWRDN)) ||
		((ICMCfgLane[2U] != ICM_CFG_VAL_PCIE)
			&& (ICMCfgLane[2U] != ICM_CFG_VAL_USB)
			&& (ICMCfgLane[2U] != ICM_CFG_VAL_PWRDN)) ||
		((ICMCfgLane[3U] != ICM_CFG_VAL_PCIE)
			&& (ICMCfgLane[3U] != ICM_CFG_VAL_SATA)
			&& (ICMCfgLane[3U] != ICM_CFG_VAL_PWRDN))) {

		UStatus = XFSBL_ERROR_GT_LANE_SELECTION;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_GT_LANE_SELECTION\r\n");
		goto END;
	}

	/**
	 * If any of the lanes are of PCIe or PowerDown, that particular lane
	 * shall be configured as PCIe, else shall be configured
	 * as DP/USB/SATA, as applicable to that lane.
	 *
	 * Lane# 	WriteBuffer[1] bit#		bit value '0'	bit value '1'
	 * --------------------------------------------------------------
	 * Lane0			0					PCIe		DP
	 * Lane1			1					PCIe		DP
	 * Lane2			2					PCIe		USB
	 * Lane3			3					PCIe		SATA
	 */

	if (ICMCfgLane[0U] == ICM_CFG_VAL_DP) {
		WriteBuffer[1U] |= DATA_GT_L0_DP_CFG;
	}

	if (ICMCfgLane[1U] == ICM_CFG_VAL_DP) {
		WriteBuffer[1U] |= DATA_GT_L1_DP_CFG;
	}

	if (ICMCfgLane[2U] == ICM_CFG_VAL_USB) {
		WriteBuffer[1U] |= DATA_GT_L2_USB_CFG;
	}

	if (ICMCfgLane[3U] == ICM_CFG_VAL_SATA) {
		WriteBuffer[1U] |= DATA_GT_L3_SATA_CFG;
	}
#endif

	/* Send the Data */
	Status = XIicPs_MasterSendPolled(&I2c0Instance,
			WriteBuffer, 2U, IOEXPANDER1_ADDR);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle */
	while (XIicPs_BusIsBusy(&I2c0Instance) == TRUE) {
		/*For MISRA C compliance*/
	}
#endif


	Status = XFsbl_FMCEnable(&I2c0Instance, &I2c1Instance);
	if (Status != XST_SUCCESS) {
		XFsbl_Printf(DEBUG_INFO, "FMC VADJ Configuration Not Successful");
	}
	XFsbl_Printf(DEBUG_INFO, "Board Configuration successful\n\r");
	UStatus = XFSBL_SUCCESS;

END:

	return UStatus;

}

#if defined(XPS_BOARD_ZCU102)
/*****************************************************************************/
/**
 * This function is used to provide PCIe reset on ZCU102 board.
 *
 * @param none
 *
 * @return none
 *
 *****************************************************************************/
static void XFsbl_PcieReset(void)
{

	u32 RegVal;
	u32 ICMCfg0L0;

	ICMCfg0L0 = XFsbl_In32(SERDES_ICM_CFG0) & SERDES_ICM_CFG0_L0_ICM_CFG_MASK;

	/* Give reset only if we have PCIe in design */
	if (ICMCfg0L0 == ICM_CFG_VAL_PCIE)
	{

		/* Set MIO31 direction as output */
		XFsbl_Out32(GPIO_DIRM_1, GPIO_MIO31_MASK);

		/* Set MIO31 output enable */
		XFsbl_Out32(GPIO_OEN_1, GPIO_MIO31_MASK);

		/* Set MIO31 to HIGH */
		RegVal = XFsbl_In32(GPIO_DATA_1) | GPIO_MIO31_MASK;
		XFsbl_Out32(GPIO_DATA_1, RegVal);

		(void)usleep(DELAY_1_US);

		/* Set MIO31 to LOW */
		RegVal = XFsbl_In32(GPIO_DATA_1) & ~(GPIO_MIO31_MASK);
		XFsbl_Out32(GPIO_DATA_1, RegVal);

		(void)usleep(DELAY_5_US);

		/* Set MIO31 to HIGH */
		RegVal = XFsbl_In32(GPIO_DATA_1) | GPIO_MIO31_MASK;
		XFsbl_Out32(GPIO_DATA_1, RegVal);
	}

}
#endif
#endif
/*****************************************************************************/
/**
 * This function does board specific initialization.
 * Currently this is done for ZCU102 board.
 * If there isn't any board specific initialization required, it just returns.
 *
 * @param none
 *
 * @return
 * 		- XFSBL_SUCCESS for successful configuration
 * 		- errors as mentioned in xfsbl_error.h
 *
 *****************************************************************************/
u32 XFsbl_BoardInit(void)
{
	u32 Status;
#if defined(XPS_BOARD_ZCU102) || defined(XPS_BOARD_ZCU106)		\
		|| defined(XPS_BOARD_ZCU104) || defined(XPS_BOARD_ZCU111) \
		|| defined(XPS_BOARD_ZCU216) || defined(XPS_BOARD_ZCU208)
	/* Program I2C to configure GT lanes */
	Status = XFsbl_BoardConfig();
	if (Status != XFSBL_SUCCESS) {
		goto END;
	}

#if defined(XPS_BOARD_ZCU102)
	XFsbl_PcieReset();
#endif
#else
	Status = XFSBL_SUCCESS;
	goto END;
#endif

END:
	return Status;
}
