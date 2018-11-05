/******************************************************************************
*
* Copyright (C) 2015 - 17 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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
* 3.0	bkm  18/4/18  Added Board specific code w.r.t VADJ
*
* </pre>
*
* @note
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xfsbl_board.h"
#if defined(XPS_BOARD_ZCU102) || defined(XPS_BOARD_ZCU106)		\
		|| defined(XPS_BOARD_ZCU104) || defined(XPS_BOARD_ZCU111)
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
#ifdef XPS_BOARD_ZCU104
static u32 XFsbl_ReadMinMaxEepromVadj(u16 *MinVadj, u16 *MaxVadj);
static u32 XFsbl_CalVadj(u16 MinVoltage, u16 MaxVoltage);
#endif
static u32 XFsbl_BoardConfig(void);
static void XFsbl_UsbPhyReset(void);
#if defined(XPS_BOARD_ZCU102)
static void XFsbl_PcieReset(void);
#endif
/************************** Variable Definitions *****************************/
u8 Read_Buffer[MAX_SIZE];
#ifdef XPS_BOARD_ZCU104
/*****************************************************************************/
/**
 * This function is used Read the min and max VADJ values from the FMC EEPROM.
 * These values of min and max VADJ are present in DC Load section of
 * MULTIRECORD AREA in FMC EEPROM.
 *
 *
 *
 * @param u16 *MinVadj, u16 *MaxVadj
 *
 * @return none
 *
 *****************************************************************************/
static u32 XFsbl_ReadMinMaxEepromVadj(u16 *MinVadj, u16 *MaxVadj)
{
	XIicPs I2c0InstancePtr;
	u32 Count, EepromByteCount;
	XIicPs_Config *I2c0CfgPtr;
	XMultipleRecord Ptr;
	u8 WriteBuffer[BUF_LEN] = {0};
	u32 UStatus;
	s32 Status;
	u16 NominalVoltage;
	u16 EepromAddr = 0x54U;
	u16 MinVoltage;
	u16 MaxVoltage;

	EepromByteCount = 256;
	MinVoltage = 0;
	MaxVoltage = 0;
	Ptr.VadjRecordFound = 0;

	/* Initialize the IIC0 driver so that it is ready to use */
	I2c0CfgPtr = XIicPs_LookupConfig(XPAR_XIICPS_0_DEVICE_ID);
	if (I2c0CfgPtr == NULL) {
		UStatus = XFSBL_ERROR_I2C_INIT;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_INIT\r\n");
		goto END;
	}

	Status = XIicPs_CfgInitialize(&I2c0InstancePtr, I2c0CfgPtr,
			I2c0CfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_INIT;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_INIT\r\n");
		goto END;
	}

	/* Set the IIC serial clock rate */
	Status = XIicPs_SetSClk(&I2c0InstancePtr, IIC_SCLK_RATE_I2CMUX);
	if (Status != XST_SUCCESS) {
		UStatus  = XFSBL_ERROR_I2C_SET_SCLK;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_SET_SCLK\r\n");
		goto END;
	}

	/* Select the Channel-1 of MUX for I2C EEprom Access */
	WriteBuffer[0] = 0x1;
	Status = XIicPs_MasterSendPolled(&I2c0InstancePtr,
				WriteBuffer, 1, TCA9548A_ADDR);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle */
	while (XIicPs_BusIsBusy(&I2c0InstancePtr) > 0) {
		/**
		 * For MISRA C
		 * compliance
		 */
	}

	/* Read the contents of FMC EEPROM to Read_Buffer */
		Status = XIicPs_MasterRecvPolled(&I2c0InstancePtr, Read_Buffer,
			EepromByteCount, EepromAddr);
		if (Status == XST_SUCCESS) {
			UStatus = XSFBL_EEPROM_PRESENT;
			/* Wait until bus is idle */
			while (XIicPs_BusIsBusy(&I2c0InstancePtr)!=XST_SUCCESS) {
					/**
					 * For MISRA C
					 * compliance
					 */
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
	if ((Read_Buffer[0] == 0x01) && (Read_Buffer[5] != 0x00)) {
		Ptr.MultirecordHdrOff = Read_Buffer[5] * 8;
		do {
			Ptr.RecordType = Read_Buffer[Ptr.MultirecordHdrOff];
			Ptr.MultirecordHdrEol = (Read_Buffer
					[Ptr.MultirecordHdrOff + 1] & 0x80);
			Ptr.RecordLength = Read_Buffer[Ptr.MultirecordHdrOff
							+ 2];

			if (Ptr.RecordType == DC_LOAD) {
				Ptr.OutputNumber = Read_Buffer
						[Ptr.MultirecordHdrOff +
						MULTIRECORD_HEADER_SIZE] & 0x0F;
				if (Ptr.OutputNumber == 0x00) {
					Ptr.VadjRecordFound = 1;
				Ptr.VadjHdrOffset = Ptr.MultirecordHdrOff;
					Ptr.VadjDataOffset = Ptr.VadjHdrOffset
						+ MULTIRECORD_HEADER_SIZE;
					break;
				}
			}
			Ptr.MultirecordHdrOff += (Ptr.RecordLength +
					MULTIRECORD_HEADER_SIZE);
		} while (Ptr.MultirecordHdrEol == 0x00);

	if (Ptr.VadjRecordFound == 1) {
		Count = Ptr.VadjDataOffset + 1;
		NominalVoltage = ((((u16)Read_Buffer[Count+1]<<8) & 0xFF00)
				| Read_Buffer[Count]);
		NominalVoltage = NominalVoltage * 10;

		Count = Count + 2;
		MinVoltage = ((((u16)Read_Buffer[Count+1]<<8) & 0xFF00)
				| Read_Buffer[Count]) * 10;


		Count = Count + 2;
		MaxVoltage = ((((u16)Read_Buffer[Count+1]<<8) & 0xFF00)
				| Read_Buffer[Count]) * 10;
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

	if ((MinVoltage <= 1800) && (MaxVoltage >= 1800)) {
		VadjValue = SET_VADJ_1V8;
		XFsbl_Printf(DEBUG_GENERAL, "Calc_Vadj is VADJ_1V8\r\n");
	} else if ((MinVoltage <= 1500) && (MaxVoltage >= 1500)) {
		VadjValue = SET_VADJ_1V5;
		XFsbl_Printf(DEBUG_GENERAL, "Calc_Vadj is VADJ_1V5\r\n");
	} else if ((MinVoltage <= 1200) && (MaxVoltage >= 1200)) {
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
 * @param none
 *
 * @return
 *	- XFSBL_SUCCESS for successful configuration
 *	- errors as mentioned in xfsbl_error.h
 *
 *****************************************************************************/
static u32 XFsbl_FMCEnable(void)
{
	XIicPs I2c0InstancePtr;
	XIicPs_Config *I2c0CfgPtr;
	u8 WriteBuffer[BUF_LEN] = {0};
	s32 Status;
	u32 UStatus = 0;
	u16 SlaveAddr;
#if defined(XPS_BOARD_ZCU104) || defined(XPS_BOARD_ZCU111)
	XVoutCommands *VoutPtr;
	u32 VadjSetting;
	VadjSetting = SET_VADJ_0V0;
#ifdef XPS_BOARD_ZCU104
	u16 LpcMin, LpcMax;
	LpcMin = 0;
	LpcMax = 0;
#endif
#endif

	/* Initialize the IIC0 driver so that it is ready to use */
	I2c0CfgPtr = XIicPs_LookupConfig(XPAR_XIICPS_0_DEVICE_ID);
	if (I2c0CfgPtr == NULL) {
		UStatus = XFSBL_ERROR_I2C_INIT;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_INIT\r\n");
		goto END;
	}

	Status = XIicPs_CfgInitialize(&I2c0InstancePtr, I2c0CfgPtr,
			I2c0CfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_INIT;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_INIT\r\n");
		goto END;
	}

	/* Change the IIC serial clock rate */
	Status = XIicPs_SetSClk(&I2c0InstancePtr, IIC_SCLK_RATE_I2CMUX);
	if (Status != XST_SUCCESS) {
		UStatus  = XFSBL_ERROR_I2C_SET_SCLK;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_SET_SCLK\r\n");
		goto END;
	}

#ifdef XPS_BOARD_ZCU104
	UStatus = XFsbl_ReadMinMaxEepromVadj(&LpcMin, &LpcMax);
	if(UStatus != XFSBL_SUCCESS)
	{
		goto END;
	}
	VadjSetting = XFsbl_CalVadj(LpcMin, LpcMax);
#endif

#if defined(XPS_BOARD_ZCU102) || defined(XPS_BOARD_ZCU111) || defined(XPS_BOARD_ZCU106)
	/* Set I2C Mux for channel-2 */
	WriteBuffer[0] = CMD_CH_2_REG;
	SlaveAddr = PCA9544A_ADDR;
#endif

#ifdef XPS_BOARD_ZCU104
	/* Set I2C Mux for channel-2 (IRPS5401) */
	WriteBuffer[0] = CMD_CH_2_REG_IRPS;
	SlaveAddr = TCA9548A_ADDR;
#endif

	Status = XIicPs_MasterSendPolled(&I2c0InstancePtr,
			WriteBuffer, 1, SlaveAddr);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle */
	while (XIicPs_BusIsBusy(&I2c0InstancePtr) > 0) {
		/**
		 * For MISRA C
		 * compliance
		 */
	}

	/**
	 * The below piece of code is needed for PL DDR to work
	 * (to take PL DDR out of reset). Hence including this code only when
	 * PL DDR is in design.
	 */
#if defined(XPS_BOARD_ZCU102) || defined(XPS_BOARD_ZCU106)
#ifdef XPAR_MIG_0_BASEADDR
	/* Enable Regulator (FMC ADJ) ZCU102 */
	WriteBuffer[0] = CMD_ON_OFF_CFG;
	WriteBuffer[1] = ON_OFF_CFG_VAL;
	SlaveAddr = MAX15301_ADDR;

	Status = XIicPs_MasterSendPolled(&I2c0InstancePtr,
			WriteBuffer, 2, SlaveAddr);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}
#endif
#endif

#if defined(XPS_BOARD_ZCU104) || defined(XPS_BOARD_ZCU111)
	/* PMbus Command for Page Selection */
	WriteBuffer[0] = CMD_PAGE_CFG;
#ifdef XPS_BOARD_ZCU104
	/* Page-3 for SW-D in ZCU104 */
	WriteBuffer[1] = DATA_SWD_CFG;
	SlaveAddr = IRPS5401_ADDR;
#else
	/* Page-2 for SW-C in ZCU111 */
	WriteBuffer[1] = DATA_SWC_CFG;
	SlaveAddr = IRPS5401_SWC_ADDR;
#endif
	Status = XIicPs_MasterSendPolled(&I2c0InstancePtr,
			WriteBuffer, 2, SlaveAddr);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle */
	while (XIicPs_BusIsBusy(&I2c0InstancePtr) > 0) {
		/*
		 * For MISRA C
		 * compliance
		 */
	}

	/* Operation Command For ON and OFF and Control Margining*/
	WriteBuffer[0] = CMD_OPERATION_CFG;
	WriteBuffer[1] = OPERATION_VAL;
	Status = XIicPs_MasterSendPolled(&I2c0InstancePtr,
			WriteBuffer, 2, SlaveAddr);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle */
	while (XIicPs_BusIsBusy(&I2c0InstancePtr) > 0) {
		/**
		 * For MISRA C
		 * compliance
		 */
	}

	/**
	 *  Sets the format for VOUT related commands. Linear mode,
	 * -8, -9, and -12 exponents supported. No LDO support
	 *
	 */
	WriteBuffer[0] = CMD_VOUT_MODE_CFG;
	WriteBuffer[1] = DATA_VOUT_MODE_VAL;
	Status = XIicPs_MasterSendPolled(&I2c0InstancePtr,
			WriteBuffer, 2, SlaveAddr);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle */
	while (XIicPs_BusIsBusy(&I2c0InstancePtr) > 0) {
		/**
		 * For MISRA C
		 * compliance
		 */
	}

	/**
	 * PMbus Command for VOUT_MAX Sets an upper limit on the output
	 * voltage the unit can command. Format according to VOUT_MODE
	 */
	WriteBuffer[0] = CMD_VOUT_MAX_CFG;
	WriteBuffer[1] = DATA_VOUT_MAX_VAL_L;
	WriteBuffer[2] = DATA_VOUT_MAX_VAL_H;
	Status = XIicPs_MasterSendPolled(&I2c0InstancePtr,
			WriteBuffer, 3, SlaveAddr);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle */
	while (XIicPs_BusIsBusy(&I2c0InstancePtr) > 0) {
		/**
		 * For MISRA C
		 * compliance
		 */
	}

	/**
	 * Lookup table for Vout_Cmd, Vout Margin High,Vout_OV_Warn_Limit,
	 * Vout_OV_Fault_Limit, Vout Margin_Low, Vout_UV_Warn_Limit
	 * Vout_UV_Fault_Limit based on Index which is similar to
	 * VadjSetting
	 */
	XVoutCommands LookupTable[] = {
		{ 0x33, 0x13, 0xCD,  0x15, 0x66, 0x16,
				0xCD, 0x10, 0x00, 0x10 },
		{ 0x33, 0x13, 0x9A, 0x15, 0x66, 0x16,
				0xCD, 0x10, 0x00, 0x10 },
		{ 0x00, 0x18, 0x66, 0x1A, 0x33, 0x1B,
				0x9A, 0x15, 0xCD, 0x14 },
		{ 0xCD, 0x1C, 0x33, 0x1F, 0x00, 0x20,
				0x66, 0x1A, 0x33, 0x1B },

		};

		VoutPtr = &LookupTable[VadjSetting];
		/**
		 * PMbus Command for Vout CMD
		 * This is a command that is used to set the
		 * output voltage when the OPERATION command
		 * is set to ON without margining
		 * Format according to VOUT_MODE
		 */

			WriteBuffer[0] = CMD_VOUT_CMD_CFG;
			WriteBuffer[1] = VoutPtr->VoutCmdL;
			WriteBuffer[2] = VoutPtr->VoutCmdH;
			Status = XIicPs_MasterSendPolled(&I2c0InstancePtr,
						WriteBuffer, 3, SlaveAddr);
			if (Status != XST_SUCCESS) {
				UStatus = XFSBL_ERROR_I2C_WRITE;
				XFsbl_Printf(DEBUG_GENERAL,
					"XFSBL_ERROR_I2C_WRITE\r\n");
				goto END;
			}

			/* Wait until bus is idle */
			while (XIicPs_BusIsBusy(&I2c0InstancePtr) > 0) {
				/**
				 * For MISRA C
				 * compliance
				 */
			}

			/**
			 * The VOUT_OV_WARN_LIMIT command
			 * sets threshold for the output voltage high warning.
			 * Masked until the unit reaches the programmed
			 * output voltage. Formatted according to the setting
			 * of the VOUT_MODE command.
			 */
			WriteBuffer[0] = CMD_VOUT_OV_WARN_LIMIT;
			WriteBuffer[1] = VoutPtr->VoutOvWarnL;
			WriteBuffer[2] = VoutPtr->VoutOvWarnH;
			Status = XIicPs_MasterSendPolled(&I2c0InstancePtr,
						WriteBuffer, 3, SlaveAddr);
			if (Status != XST_SUCCESS) {
				UStatus = XFSBL_ERROR_I2C_WRITE;
				XFsbl_Printf(DEBUG_GENERAL,
					"XFSBL_ERROR_I2C_WRITE\r\n");
				goto END;
			}

			/* Wait until bus is idle */
			while (XIicPs_BusIsBusy(&I2c0InstancePtr) > 0) {
				/**
				 * For MISRA C
				 * compliance
				 */
			}

			/**
			 *  The VOUT_OV_FAULT_LIMIT command
			 *  Sets threshold for the output voltage high fault.
			 *  Masked until the unit reaches the programmed
			 *  output voltage. Formatted according to the
			 *  setting of the VOUT_MODE command.
			 */
			WriteBuffer[0] = CMD_VOUT_OV_FAULT_LIMIT;
			WriteBuffer[1] = VoutPtr->VoutOvFaultL;
			WriteBuffer[2] = VoutPtr->VoutOvFaultH;
			Status = XIicPs_MasterSendPolled(&I2c0InstancePtr,
						WriteBuffer, 3, SlaveAddr);
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
			WriteBuffer[0] = CMD_VOUT_UV_WARN_LIMIT;
			WriteBuffer[1] = VoutPtr->VoutUvWarnL;
				WriteBuffer[2] = VoutPtr->VoutUvWarnH;
			Status = XIicPs_MasterSendPolled(&I2c0InstancePtr,
						WriteBuffer, 3, SlaveAddr);
			if (Status != XST_SUCCESS) {
				UStatus = XFSBL_ERROR_I2C_WRITE;
				XFsbl_Printf(DEBUG_GENERAL,
					"XFSBL_ERROR_I2C_WRITE\r\n");
				goto END;
			}

			/* Wait until bus is idle */
			while (XIicPs_BusIsBusy(&I2c0InstancePtr) > 0) {
				/**
				 * For MISRA C
				 * compliance
				 */
			}

			/**
			 * The VOUT_UV_FAULT_LIMIT command
			 * Sets threshold for the output voltage low fault.
			 * Masked until the unit reaches the programmed
			 * output voltage. Formatted according to the
			 * setting of the VOUT_MODE command.
			 */
			WriteBuffer[0] = CMD_VOUT_UV_FAULT_LIMIT;
			WriteBuffer[1] = VoutPtr->VoutUvFaultL;
			WriteBuffer[2] = VoutPtr->VoutUvFaultH;
			Status = XIicPs_MasterSendPolled(&I2c0InstancePtr,
						WriteBuffer, 3, SlaveAddr);
			if (Status != XST_SUCCESS) {
				UStatus = XFSBL_ERROR_I2C_WRITE;
				XFsbl_Printf(DEBUG_GENERAL,
					     "XFSBL_ERROR_I2C_WRITE\r\n");
				goto END;
			}

			/* Wait until bus is idle */
			while (XIicPs_BusIsBusy(&I2c0InstancePtr) > 0) {
				/**
				 * For MISRA C
				 * compliance
				 */
			}

#endif
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
	XIicPs I2c0InstancePtr;
	XIicPs_Config *I2c0CfgPtr;
	s32 Status;
	u32 UStatus;
#if defined(XPS_BOARD_ZCU102) || defined(XPS_BOARD_ZCU106)
	u8 WriteBuffer[BUF_LEN] = {0};
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

	Status = XIicPs_CfgInitialize(&I2c0InstancePtr, I2c0CfgPtr,
			I2c0CfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_INIT;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_INIT\r\n");
		goto END;
	}

	/* Set the IIC serial clock rate */
	Status = XIicPs_SetSClk(&I2c0InstancePtr, IIC_SCLK_RATE_IOEXP);
	if (Status != XST_SUCCESS) {
		UStatus  = XFSBL_ERROR_I2C_SET_SCLK;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_SET_SCLK\r\n");
		goto END;
	}

#if defined(XPS_BOARD_ZCU102) || defined(XPS_BOARD_ZCU106)
	/* Configure I/O pins as Output */
	WriteBuffer[0] = CMD_CFG_0_REG;
	WriteBuffer[1] = DATA_OUTPUT;
	Status = XIicPs_MasterSendPolled(&I2c0InstancePtr,
		WriteBuffer, 2, IOEXPANDER1_ADDR);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle to start another transfer */
	while (XIicPs_BusIsBusy(&I2c0InstancePtr)>0) {
		/*For MISRA C compliance*/
	}

	/*
	 * Deasserting I2C_MUX_RESETB
	 * And GEM3 Resetb
	 * Selecting lanes based on configuration
	 */
	WriteBuffer[0] = CMD_OUTPUT_0_REG;
	WriteBuffer[1] = DATA_COMMON_CFG;

#if defined(XPS_BOARD_ZCU102)
	ICMCfgLane[0] = XFsbl_In32(SERDES_ICM_CFG0) & SERDES_ICM_CFG0_L0_ICM_CFG_MASK;
	ICMCfgLane[1] = (XFsbl_In32(SERDES_ICM_CFG0) &
		SERDES_ICM_CFG0_L1_ICM_CFG_MASK) >> SERDES_ICM_CFG0_L1_ICM_CFG_SHIFT;
	ICMCfgLane[2] = XFsbl_In32(SERDES_ICM_CFG1) & (SERDES_ICM_CFG1_L2_ICM_CFG_MASK);
	ICMCfgLane[3] = (XFsbl_In32(SERDES_ICM_CFG1) &
		SERDES_ICM_CFG1_L3_ICM_CFG_MASK) >> SERDES_ICM_CFG1_L3_ICM_CFG_SHIFT;

	/* For ZCU102 board, check if GT combination is valid against the lane# */
	if (((ICMCfgLane[0] != ICM_CFG_VAL_PCIE)
			&& (ICMCfgLane[0] != ICM_CFG_VAL_DP)
			&& (ICMCfgLane[0] != ICM_CFG_VAL_PWRDN)) ||
		((ICMCfgLane[1] != ICM_CFG_VAL_PCIE)
			&& (ICMCfgLane[1] != ICM_CFG_VAL_DP)
			&& (ICMCfgLane[1] != ICM_CFG_VAL_PWRDN)) ||
		((ICMCfgLane[2] != ICM_CFG_VAL_PCIE)
			&& (ICMCfgLane[2] != ICM_CFG_VAL_USB)
			&& (ICMCfgLane[2] != ICM_CFG_VAL_PWRDN)) ||
		((ICMCfgLane[3] != ICM_CFG_VAL_PCIE)
			&& (ICMCfgLane[3] != ICM_CFG_VAL_SATA)
			&& (ICMCfgLane[3] != ICM_CFG_VAL_PWRDN))) {

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

	if (ICMCfgLane[0] == ICM_CFG_VAL_DP) {
		WriteBuffer[1] |= DATA_GT_L0_DP_CFG;
	}

	if (ICMCfgLane[1] == ICM_CFG_VAL_DP) {
		WriteBuffer[1] |= DATA_GT_L1_DP_CFG;
	}

	if (ICMCfgLane[2] == ICM_CFG_VAL_USB) {
		WriteBuffer[1] |= DATA_GT_L2_USB_CFG;
	}

	if (ICMCfgLane[3] == ICM_CFG_VAL_SATA) {
		WriteBuffer[1] |= DATA_GT_L3_SATA_CFG;
	}
#endif

	/* Send the Data */
	Status = XIicPs_MasterSendPolled(&I2c0InstancePtr,
			WriteBuffer, 2, IOEXPANDER1_ADDR);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle */
	while (XIicPs_BusIsBusy(&I2c0InstancePtr)>0) {
		/*For MISRA C compliance*/
	}
#endif


	Status = XFsbl_FMCEnable();
	if (Status != XST_SUCCESS) {
		UStatus = XSFBL_ERROR_FMC_ENABLE;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_FMC_ENABLE\r\n");
		goto END;
	}
	XFsbl_Printf(DEBUG_INFO, "Board Configuration successful\n\r");
	UStatus = XFSBL_SUCCESS;

END:

	return UStatus;

}

/*****************************************************************************/
/**
 * This function is used to provide Reset to USB Phy on ZCU102 board.
 *
 * @param none
 *
 * @return none
 *
 *****************************************************************************/
static void XFsbl_UsbPhyReset(void)
{

	/* USB PHY Reset */
	XFsbl_Out32(CRL_APB_BOOT_PIN_CTRL, CRL_APB_BOOTMODE_1_HI);
	(void)usleep(DELAY_1_US);
	XFsbl_Out32(CRL_APB_BOOT_PIN_CTRL, CRL_APB_BOOTMODE_1_LO);
	(void)usleep(DELAY_5_US);
	XFsbl_Out32(CRL_APB_BOOT_PIN_CTRL, CRL_APB_BOOTMODE_1_HI);

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
		|| defined(XPS_BOARD_ZCU104) || defined(XPS_BOARD_ZCU111)
	/* Program I2C to configure GT lanes */
	Status = XFsbl_BoardConfig();
	if (Status != XFSBL_SUCCESS) {
		goto END;
	}

	XFsbl_UsbPhyReset();
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
