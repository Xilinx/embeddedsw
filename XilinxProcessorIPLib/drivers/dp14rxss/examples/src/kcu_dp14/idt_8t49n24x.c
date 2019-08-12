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
* @file idt_8t49n24x.c
* @addtogroup IDT_8T49N24x
* @{
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* X.XX  XX     YY/MM/DD
* 1.00  RHe    16/06/20 Initial release. A ported version from the
*                       8T49N28X-FrequencyProgrammingGuide-register-
*                       calculations.py script
* 2.00  MG     16/08/15 Major update
* 2.10  MG     16/09/05 Added LOS variable
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "idt_8t49n24x.h"

#include "xiic.h"
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "microblaze_sleep.h"
#include <stdlib.h>
#include <math.h>

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
static u8 IDT_8T49N24x_GetRegister(u32 I2CBaseAddress, u8 I2CSlaveAddress,
				u16 RegisterAddress);
static int IDT_8T49N24x_SetRegister(u32 I2CBaseAddress, u8 I2CSlaveAddress,
				u16 RegisterAddress, u8 Value);
static int IDT_8T49N24x_ModifyRegister(u32 I2CBaseAddress, u8 I2CSlaveAddress,
				u16 RegisterAddress, u8 Value, u8 Mask);
static int IDT_8T49N24x_CheckDeviceID(u32 I2CBaseAddress, u8 I2CSlaveAddress);
//static int IDT_8T49N24x_GPIO(u32 I2CBaseAddress, u8 I2CSlaveAddress);
//static int IDT_8T49N24x_OutputDriver(u32 I2CBaseAddress, u8 I2CSlaveAddress,
//				u8 Output, u8 Enable);
//static int IDT_8T49N24x_LockMode(u32 I2CBaseAddress, u8 I2CSlaveAddress);
static int IDT_8T49N24x_InputMonitorControl(u32 I2CBaseAddress,
				u8 I2CSlaveAddress, u32 Value, u8 Input);
static int IDT_8T49N24x_PreDivider(u32 I2CBaseAddress, u8 I2CSlaveAddress,
				u32 Value, u8 Input);
static int IDT_8T49N24x_M1Feedback(u32 I2CBaseAddress, u8 I2CSlaveAddress,
				u32 Value, u8 Input);
static int IDT_8T49N24x_DSMInteger(u32 I2CBaseAddress, u8 I2CSlaveAddress,
				u16 Value);
static int IDT_8T49N24x_DSMFractional(u32 I2CBaseAddress, u8 I2CSlaveAddress,
				u32 Value);
static int IDT_8T49N24x_OutputDividerInteger(u32 I2CBaseAddress,
				u8 I2CSlaveAddress, u32 Value, u8 Output);
static int IDT_8T49N24x_OutputDividerFractional(u32 I2CBaseAddress,
				u8 I2CSlaveAddress, u32 Value, u8 Output);
static int IDT_8T49N24x_Mode(u32 I2CBaseAddress, u8 I2CSlaveAddress,
				u8 Synthesizer);
//static int IDT_8T49N24x_SelectInputReference(u32 I2CBaseAddress,
//				u8 I2CSlaveAddress, u8 Input);
static int IDT_8T49N24x_GetIntDivTable(int FOut, int *DivTable, u8 Bypass);
static int IDT_8T49N24x_CalculateSettings(int FIn, int FOut,
				IDT_8T49N24x_Settings* RegSettings);
static int IDT_8T49N24x_Enable(u32 I2CBaseAddress, u8 I2CSlaveAddress,
				u8 Enable);
static int IDT_8T49N24x_ReferenceInput(u32 I2CBaseAddress, u8 I2CSlaveAddress,
				u8 Input, u8 Enable);
//static int IDT_8T49N24x_Configure(u32 I2CBaseAddress, u8 I2CSlaveAddress);
static int IDT_8T49N24x_Configure_JA(u32 I2CBaseAddress, u8 I2CSlaveAddress);
/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function reads a single byte to the IDT 8T49N24x  
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Initialization was successful.
*    - XST_FAILURE I2C write error.
*
* @note None.
*
******************************************************************************/
static u8 IDT_8T49N24x_GetRegister(u32 I2CBaseAddress, u8 I2CSlaveAddress,
			u16 RegisterAddress)
{
	u32 ByteCount = 0;
	u8 Buffer[2];
	u8 Data;
	u8 Retry = 0;
	u8 Exit;

	Exit = FALSE;
	Data = 0;

	do {
		/* Set Address */
		Buffer[0] = (RegisterAddress >> 8); 
		Buffer[1] = RegisterAddress & 0xff; 
		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
				(u8*)Buffer, 2, XIIC_REPEATED_START);

		if (ByteCount != 2) {
			Retry++;
			
			/* Maximum retries */
			if (Retry == 255) {
				Exit = TRUE;
			}
		} else {
			/* Read data */
			ByteCount = XIic_Recv(I2CBaseAddress, I2CSlaveAddress,
						(u8*)Buffer, 1, XIIC_STOP);
			if (ByteCount != 1) {
				Exit = FALSE;
			} else {
				Data = Buffer[0];
				Exit = TRUE;
			}
		}
	} while (!Exit);

	return Data;
}

/*****************************************************************************/
/**
*
* This function send a single byte to the IDT 8T49N24x  
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Initialization was successful.
*    - XST_FAILURE I2C write error.
*
* @note None.
*
******************************************************************************/
static int IDT_8T49N24x_SetRegister(u32 I2CBaseAddress, u8 I2CSlaveAddress,
			u16 RegisterAddress, u8 Value)
{
	u32 ByteCount = 0;
	u8 Buffer[3];
	u8 Retry = 0;

	/* Write data */
	Buffer[0] = (RegisterAddress >> 8); 
	Buffer[1] = RegisterAddress & 0xff; 
	Buffer[2] = Value;
	
	while (1) {
		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
					(u8*)Buffer, 3, XIIC_STOP);
		if (ByteCount != 3) {
			Retry++;

			/* Maximum retries */
			if (Retry == 255) {
				return XST_FAILURE;
			}
		} else {
			return XST_SUCCESS;
		}
	}  
}

/*****************************************************************************/
/**
*
* This function modifies a single byte to the IDT 8T49N24x  
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Initialization was successful.
*    - XST_FAILURE I2C write error.
*
* @note None.
*
******************************************************************************/
static int IDT_8T49N24x_ModifyRegister(u32 I2CBaseAddress, u8 I2CSlaveAddress,
				u16 RegisterAddress, u8 Value, u8 Mask)
{
	u8 Data;
	int Result;

	/* Read data */
	Data = IDT_8T49N24x_GetRegister(I2CBaseAddress, I2CSlaveAddress,
					RegisterAddress);

	/* Clear masked bits  */
	Data &= ~Mask; 

	/* Update */
	Data |= (Value & Mask);

	/* Write data */
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					RegisterAddress, Data);

	return Result;
}

/*****************************************************************************/
/**
*
* This function checks the IDT 8T49N24x device ID  
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Initialization was successful.
*    - XST_FAILURE I2C write error.
*
* @note None.
*
******************************************************************************/
static int IDT_8T49N24x_CheckDeviceID(u32 I2CBaseAddress, u8 I2CSlaveAddress)
{
	u16 DeviceId;
	u8 Data;

	/* Get DEV_ID[15:12] */
	Data = IDT_8T49N24x_GetRegister(I2CBaseAddress, I2CSlaveAddress,
							0x0002);

	/* Mask */
	Data &= 0x0f;

	/* Copy */
	DeviceId = Data;

	/* Shift */
	DeviceId <<= 8;

	/* Get DEV_ID[11:4] */
	Data = IDT_8T49N24x_GetRegister(I2CBaseAddress, I2CSlaveAddress,
							0x0003);

	/* Copy */
	DeviceId |= Data;

	/* Shift */
	DeviceId <<= 4;

	/* Get DEV_ID[3:0] */
	Data = IDT_8T49N24x_GetRegister(I2CBaseAddress, I2CSlaveAddress,
							0x0004);

	/* Copy */
	DeviceId |= (Data >> 4);

	/* Check */
	if (DeviceId == 0x0606) {
		return XST_SUCCESS;
	} else {
		return XST_FAILURE;
	}
}

/* Get valid Integer output divider values */
static int IDT_8T49N24x_GetIntDivTable(int FOut, int *DivTable, u8 Bypass)
{
	int NS1_Options[4] = {1,4,5,6};
	int index;
	int NS2Min = 1;
	int NS2Max = 1;
	
	int NS2Temp;
	int OutDivTemp;
	u32 VCOTemp;
	
	int OutDivMin = (int)ceil(IDT_8T49N24X_FVCO_MIN/FOut);
	int OutDivMax = (int)floor(IDT_8T49N24X_FVCO_MAX/FOut);
	
	int Count = 0;
	int *DivTablePtr = DivTable;
	
	if (Bypass == TRUE) {
		index = 0;
	} else {
		index = 1;
	}
	
	for (int i = index; i < (sizeof(NS1_Options)/sizeof(int)); i++) {
		if ((NS1_Options[i] == OutDivMin) || 
		    (NS1_Options[i] == OutDivMax)) {
			/* This is for the case where 
			 * we want to bypass NS2.
			 * */
			NS2Min = 0;
			NS2Max = 0;
		}
	}
	
	if (NS2Min == 1) {
		/* If this test passes, then we know 
		 * we're not in the bypass case */
		NS2Min = (int)ceil(OutDivMin / NS1_Options[3] / 2); /* the last element in the list */
		NS2Max = (int)floor(OutDivMax / NS1_Options[index] / 2);
		if (NS2Max == 0)
			/* Because we're rounding-down for the max, 
			 * we may end-up with it being 0, 
			 * In which case we need to make it 1.
			 * */
			NS2Max = 1; 
	}
	 
	NS2Temp = NS2Min;
	
	while (NS2Temp <= NS2Max) {
		for (int i = index ; 
		     i < (sizeof(NS1_Options)/sizeof(int)) ;
		     i++) {
			if (NS2Temp == 0) {
				OutDivTemp = NS1_Options[i];
			} else {
				OutDivTemp = NS1_Options[i] * NS2Temp * 2;
			}
			
			VCOTemp = FOut * OutDivTemp;
			
			if ((VCOTemp <= IDT_8T49N24X_FVCO_MAX) &&
			    (VCOTemp >= IDT_8T49N24X_FVCO_MIN)) {
				*DivTablePtr = OutDivTemp;
				Count++;
				DivTablePtr++;
			}
		}
		NS2Temp++;
	}
	
	return Count;
}

/*****************************************************************************/
/**
*
* This function calculates the settings  
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Initialization was successful.
*    - XST_FAILURE I2C write error.
*
* @note None.
*
******************************************************************************/
static int IDT_8T49N24x_CalculateSettings(int FIn, int FOut, 
				IDT_8T49N24x_Settings* RegSettings)
{
	int DivTable[20];
	int DivTableCount;
	
	int MaxDiv = 0;
	double FVCO;
	int NS1_RegSettings;
	int NS2_RegSettings;
	
	int NS1Ratio;
	int NS2Ratio;
	
	double FracDiv;
	
	double UpperFBDiv;
	int DSMInt_RegSettings;
	int DSMFrac_RegSettings;
	double Ratio;
	
	int LOS;

	/* Get the valid integer dividers */
	DivTableCount = IDT_8T49N24x_GetIntDivTable(FOut, DivTable, FALSE);
	
	/* Find the highest divider */
	for (int i = 0 ; i < DivTableCount ; i++) {
		if (MaxDiv < DivTable[i]) {
			MaxDiv = DivTable[i];
		}
	}
	
	FVCO = (double)FOut * MaxDiv;
	
	/* **********************************************
	* INTEGER DIVIDER: Determine NS1 register setting
	* **********************************************/
	
	/* Only use the divide-by-1 option for really small divide ratios
	 * note that this option will never be on the list for 
	 * the Q0 - Q3 dividers
	 * */
	if (MaxDiv < 4) { 
		
	}
	
	/* Make sure we can divide the ratio by 4 in NS1 and
	 * by 1 or an even number in NS2 */
	if ((MaxDiv == 4) || (MaxDiv % 8 == 0)) { 
		NS1_RegSettings = 2; /* Divide by 4 register selection */
	}
	
	/* Make sure we can divide the ratio by 5 in NS1 and
	 * by 1 or an even number in NS2 */
	if ((MaxDiv == 5) || (MaxDiv % 10 == 0)) {
		NS1_RegSettings = 0; /* Divide by 5 register selection */
	}
	
	/* Make sure we can divide the ratio by 6 in NS1 and
	 * by 1 or an even number in NS2 */
	if ((MaxDiv == 6) || (MaxDiv % 12 == 0)) {
		NS1_RegSettings = 1; /* Divide by 6 register setting */
	}
	
	/* **********************************************
	 * INTEGER DIVIDER: Determine NS2 register setting
	 * *********************************************/
	
	switch (NS1_RegSettings) {
		case (0) :
			NS1Ratio = 5;
		break;
		
		case (1) :
			NS1Ratio = 6;
		break;
		
		case (2) :
			NS1Ratio = 4;
		break;
		
		case (3) : /* This is the bypass (divide-by-1) option */
			NS1Ratio = 1;
		break;
		
		default :
			NS1Ratio = 6;
		break;
	}
	
	NS2Ratio = (int)floor(MaxDiv / NS1Ratio);
	
	NS2_RegSettings = (int)floor(NS2Ratio/2);
	
	/* **********************************************
	* FRACTIONAL DIVIDER:
	* **********************************************/
	FracDiv = FVCO/FOut;
	
	u32 N_Q2 = 0;
	u32 NFRAC_Q2 = 0;

	double frac_numerator = round(((FracDiv / 2.0) -
				(int)(FracDiv / 2.0)) * pow(2,28));

	/* This is the case where the fractional portion is 0.
	 * Due to precision limitations, sometimes fractional portion
	 * of the Effective divider gets rounded to 1.
	 * This checks for that condition.
	 * */
	if ((frac_numerator >= 268435456) || 
	    ((FracDiv/2.0) == (int)(FracDiv/2.0))) {
		N_Q2 = (int)round(FracDiv / 2.0);
		NFRAC_Q2 = 0;
	} else { 
		/* This is the case where the fractional portion is not 0. */
		N_Q2 = (int)floor(FracDiv / 2.0);
		NFRAC_Q2 = (int)frac_numerator;
	}

	/* ***********************************************
	* Calculate the Upper Loop Feedback divider setting
	* ***********************************************/
	
	UpperFBDiv = (double)(FVCO) / (2*IDT_8T49N24X_XTAL_FREQ);
	DSMInt_RegSettings = (int)floor(UpperFBDiv);

	DSMFrac_RegSettings = (int)round((UpperFBDiv - floor(UpperFBDiv))*pow(2,21));
	
	/* **********************************************
	* Calculate the Lower Loop Feedback divider and Input Divider
	* **********************************************/
	
	Ratio = FVCO/FIn;
	
	int M1 = 0;
	int PMin = (int)FIn/IDT_8T49N24X_FPD_MAX;
//	int M1Min = (int)(FVCO/IDT_8T49N24X_FPD_MAX); /* This M1 divider sets the input PFD frequency at 128KHz, the set max */
	
	int M1_default = 0;
	int P_default = 0;
	int error_tmp = 999999;
	int error = 99999999;

	int count = 0;

	/* Start from lowest divider and iterate until 0 error 
	 * is found or the divider limit is exhausted.
	 * Keep the setting with the lowest error
	 * */
	for (int i = PMin; i <= IDT_8T49N24X_P_MAX; i++) {
		M1 = (int)round(i*Ratio);
		count++;
		if (M1 < IDT_8T49N24X_M_MAX) {
			error_tmp = (int)(Ratio*1e9 - (M1*1e9 / i));

			if (abs(error_tmp) < error || error_tmp == 0) {
				error = abs(error_tmp);
				M1_default = M1;
				P_default = i;

				if (error_tmp == 0)
					break;
			}
		}
		else {
			break;
		}
	}
	
	/* Calculate LOS */
	LOS = FVCO / 8 / FIn; 
	LOS = LOS + 3;
	if (LOS < 6)
		LOS = 6;

	/* Copy registers */
	RegSettings->NS1_Qx = NS1_RegSettings;
	RegSettings->NS2_Qx = NS2_RegSettings;
	
	RegSettings->N_Qx = N_Q2;
	RegSettings->NFRAC_Qx = NFRAC_Q2;

	RegSettings->DSM_INT = DSMInt_RegSettings;
	RegSettings->DSM_FRAC = DSMFrac_RegSettings;
	RegSettings->M1_x = M1_default;
	RegSettings->PRE_x = P_default;
	RegSettings->LOS_x = LOS;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function calculates and sets the IDT 8TN49N24x device with the 
* given clock configuration.
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
* @param FIn specifies the input frequency.
* @param FOut specifies the output frequency.
* @param FreeRun specifies if the operation mode is locked/synthesizer mode.
*    - TRUE Synthesizer mode (Fout only)
*    - FALSE Locked mode (Fout locked to Fin)
*
* @return
*    - XST_SUCCESS Initialization was successful.
*    - XST_FAILURE I2C write error or incorrect parameters detected.
*
* @note 
*
******************************************************************************/
int IDT_8T49N24x_SetClock(u32 I2CBaseAddress, u8 I2CSlaveAddress,
				int FIn, int FOut, u8 FreeRun)
{
	int Result;

	if ((FIn < IDT_8T49N24X_FIN_MIN) && (FIn > IDT_8T49N24X_FIN_MAX)) {
		return XST_FAILURE;
	}
	
	if ((FOut < IDT_8T49N24X_FOUT_MIN) && (FOut > IDT_8T49N24X_FOUT_MAX)) {
		return XST_FAILURE;
	}
	
	IDT_8T49N24x_Settings RegSettings;

	/* Calculate settings	 */
	IDT_8T49N24x_CalculateSettings(FIn, FOut, &RegSettings);

	/* Disable DPLL and APLL calibration */
	Result = IDT_8T49N24x_Enable(I2CBaseAddress, I2CSlaveAddress, FALSE);

	usleep(300000);

	/* Mode */
	if (FreeRun == TRUE) {
		/* Disable reference clock input 0 */
		Result = IDT_8T49N24x_ReferenceInput(I2CBaseAddress,
						I2CSlaveAddress, 0, FALSE);

		/* Disable reference clock input 1 */
		Result = IDT_8T49N24x_ReferenceInput(I2CBaseAddress,
						I2CSlaveAddress, 1, FALSE);

		/* Set synthesizer mode */
		Result = IDT_8T49N24x_Mode(I2CBaseAddress,
					   I2CSlaveAddress, TRUE);
	} else {
		/* Enable reference clock input 0 */
		Result = IDT_8T49N24x_ReferenceInput(I2CBaseAddress,
						I2CSlaveAddress, 0, TRUE);

		/* Disable reference clock input 1 */
		Result = IDT_8T49N24x_ReferenceInput(I2CBaseAddress,
						I2CSlaveAddress, 1, FALSE);

		/* Set jitter attentuator mode */
		Result = IDT_8T49N24x_Mode(I2CBaseAddress,
					I2CSlaveAddress, FALSE);
	}

	/* Pre-divider Input 0 */
	Result = IDT_8T49N24x_PreDivider(I2CBaseAddress, I2CSlaveAddress,
					 RegSettings.PRE_x, 0);

	/* Pre-divider Input 1 */
	Result = IDT_8T49N24x_PreDivider(I2CBaseAddress, I2CSlaveAddress,
					 RegSettings.PRE_x, 1);

	/* M1 feedback Input 0 */
	Result = IDT_8T49N24x_M1Feedback(I2CBaseAddress, I2CSlaveAddress,
					 RegSettings.M1_x, 0);

	/* M1 feedback Input 1 */
	Result = IDT_8T49N24x_M1Feedback(I2CBaseAddress, I2CSlaveAddress,
					 RegSettings.M1_x, 1);

	/* DSM integer */
	Result = IDT_8T49N24x_DSMInteger(I2CBaseAddress, I2CSlaveAddress,
					 RegSettings.DSM_INT);

	/* DSM fractional */
	Result = IDT_8T49N24x_DSMFractional(I2CBaseAddress, I2CSlaveAddress,
					    RegSettings.DSM_FRAC);

	/* Output divider integer output 2 */
	Result = IDT_8T49N24x_OutputDividerInteger(I2CBaseAddress,
						   I2CSlaveAddress,
						   RegSettings.N_Qx, 2);

	/* Output divider integer output 3 */
	Result = IDT_8T49N24x_OutputDividerInteger(I2CBaseAddress,
						   I2CSlaveAddress,
						   RegSettings.N_Qx, 3);

	/* Output divider fractional output 2 */
	Result = IDT_8T49N24x_OutputDividerFractional(I2CBaseAddress,
						      I2CSlaveAddress,
						      RegSettings.NFRAC_Qx, 2);

	/* Output divider fractional output 3 */
	Result = IDT_8T49N24x_OutputDividerFractional(I2CBaseAddress,
						      I2CSlaveAddress,
						      RegSettings.NFRAC_Qx, 3);

	/* Input monitor control 0 */
	Result = IDT_8T49N24x_InputMonitorControl(I2CBaseAddress,
						  I2CSlaveAddress,
						  RegSettings.LOS_x, 0);

	/* Input monitor control 1 */
	Result = IDT_8T49N24x_InputMonitorControl(I2CBaseAddress,
						  I2CSlaveAddress,
						  RegSettings.LOS_x, 1);

	/* Enable DPLL and APLL calibration */
	Result = IDT_8T49N24x_Enable(I2CBaseAddress, I2CSlaveAddress, TRUE);

	return Result;
}

/*****************************************************************************/
/**
*
* This function set the input monitor control
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS 
*    - XST_FAILURE 
*
* @note None.
*
******************************************************************************/
static int IDT_8T49N24x_InputMonitorControl(u32 I2CBaseAddress,
				u8 I2CSlaveAddress, u32 Value, u8 Input)
{
	int Result;
	u8 Data;
	u16 Address;

	if (Input == 1)
		Address = 0x0074;
	else
		Address = 0x0071;

	/* LOSx[16] */
	Data = (Value >> 16) & 0x1; 
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  Address, Data);

	/* LOSx[15:8] */
	Data = (Value >> 8); 
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  Address+1, Data);

	/* LOSx[7:0] */
	Data = (Value & 0xff); 
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  Address+2, Data);

	return Result;
}

/*****************************************************************************/
/**
*
* This function set the pre-divider
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS 
*    - XST_FAILURE 
*
* @note None.
*
******************************************************************************/
static int IDT_8T49N24x_PreDivider(u32 I2CBaseAddress, u8 I2CSlaveAddress,
					u32 Value, u8 Input)
{
	int Result;
	u8 Data;
	u16 Address;

	if (Input == 1)
		Address = 0x000e;
	else
		Address = 0x000b;

	/* PREx[20:16] */
	Data = (Value >> 16) & 0x1f; 
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  Address, Data);

	/* PREx[15:8] */
	Data = (Value >> 8); 
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  Address+1, Data);

	/* PREx[7:0] */
	Data = (Value & 0xff); 
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  Address+2, Data);

	return Result;
}

/*****************************************************************************/
/**
*
* This function set the M1 feedback
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS 
*    - XST_FAILURE 
*
* @note None.
*
******************************************************************************/
static int IDT_8T49N24x_M1Feedback(u32 I2CBaseAddress, u8 I2CSlaveAddress,
					u32 Value, u8 Input)
{
	int Result;
	u8 Data;
	u16 Address;

	if (Input == 1)
		Address = 0x0011;
	else
		Address = 0x0014;

	/* M1x[23:16] */
	Data = (Value >> 16); 
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  Address, Data);

	/* M1x[15:8] */
	Data = (Value >> 8); 
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  Address+1, Data);

	/* M1x[7:0] */
	Data = (Value & 0xff); 
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  Address+2, Data);

	return Result;
}

/*****************************************************************************/
/**
*
* This function set the DSM integer
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS 
*    - XST_FAILURE 
*
* @note None.
*
******************************************************************************/
static int IDT_8T49N24x_DSMInteger(u32 I2CBaseAddress, u8 I2CSlaveAddress,
					u16 Value)
{
	int Result;
	u8 Data;

	/* DSM_INT[8] */
	Data = (Value >> 8) & 0x01; 
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  0x0025, Data);

	/* DSM_INT[7:0] */
	Data = (Value & 0xff); 
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  0x0026, Data);

	return Result;
}

/*****************************************************************************/
/**
*
* This function set the DSM fractional
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS 
*    - XST_FAILURE 
*
* @note None.
*
******************************************************************************/
static int IDT_8T49N24x_DSMFractional(u32 I2CBaseAddress, u8 I2CSlaveAddress,
						u32 Value)
{
	int Result;
	u8 Data;

	/* DSM_FRAC[20:16] */
	Data = (Value >> 16) & 0x1f; 
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  0x0028, Data);

	/* DSM_FRAC[15:8] */
	Data = (Value >> 8); 
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  0x0029, Data);

	/* DSM_FRAC[7:0] */
	Data = (Value & 0xff); 
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  0x002a, Data);

	return Result;
}

/*****************************************************************************/
/**
*
* This function set the Output divider integer
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS 
*    - XST_FAILURE 
*
* @note None.
*
******************************************************************************/
static int IDT_8T49N24x_OutputDividerInteger(u32 I2CBaseAddress,
			u8 I2CSlaveAddress, u32 Value, u8 Output)
{
	int Result;
	u8 Data;
	u16 Address;

	switch (Output) {
		case 0 : Address = 0x003f;
			break;
		case 1 : Address = 0x0042;
			break;
		case 2 : Address = 0x0045;
			break;
		case 3 : Address = 0x0048;
			break;
	}
	
	/* N_Qm[17:16] */
	Data = (Value >> 16) & 0x03; 
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  Address, Data);

	/* N_Qm[15:8] */
	Data = (Value >> 8); 
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  Address+1, Data);

	/* N_Qm[7:0] */
	Data = (Value & 0xff); 
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  Address+2, Data);

	return Result;
}

/*****************************************************************************/
/**
*
* This function set the Output divider fractional
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS 
*    - XST_FAILURE 
*
* @note None.
*
******************************************************************************/
static int IDT_8T49N24x_OutputDividerFractional(u32 I2CBaseAddress,
			u8 I2CSlaveAddress, u32 Value, u8 Output)
{
	int Result;
	u8 Data;
	u16 Address;

	switch (Output) {
		case 0 : Address = 0x0000;	// ???
			break;
		case 1 : Address = 0x0057;
			break;
		case 2 : Address = 0x005b;
			break;
		case 3 : Address = 0x005f;
			break;
	}
	
	/* NFRAC_Qm[27:24] */
	Data = (Value >> 24) & 0x0f; 
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  Address, Data);

	/* NFRAC_Qm[23:16] */
	Data = (Value >> 16); 
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  Address+1, Data);

	/* NFRAC_Qm[15:8] */
	Data = (Value >> 8); 
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  Address+2, Data);

	/* NFRAC_Qm[7:0] */
	Data = (Value & 0xff);
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  Address+3, Data);

	return Result;
}

/*****************************************************************************/
/**
*
* This function sets the mode (jitter attenuator / synthesizer)
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS 
*    - XST_FAILURE 
*
* @note None.
*
******************************************************************************/
static int IDT_8T49N24x_Mode(u32 I2CBaseAddress, u8 I2CSlaveAddress,
					u8 Synthesizer)
{
	int Result;
	u8 Value;
	u8 Mask;

	/* Digital PLL */
	/* State[1:0] */
	if (Synthesizer) {
		Value = 0x01;		/* Force FREERUN */
		Value |= (1<<4);	/* Disable reference input 0 */
		Value |= (1<<5);	/* Disable reference input 1 */
	} else {
		Value = 0x00;		/* Run automatically */
		Value |= (1<<5);	/* Disable reference input 1 */
	}

	Mask = 0x33;
	Result = IDT_8T49N24x_ModifyRegister(I2CBaseAddress, I2CSlaveAddress,
					     0x000a, Value, Mask);

	/* Analog PLL */
	/* SYN_MODE */
	if (Synthesizer) {
		Value = (1<<3);		/* Synthesizer mode */
	} else {
		Value = 0x00;		/* Jitter attenuator mode */
	}

	Mask = (1<<3);
	Result = IDT_8T49N24x_ModifyRegister(I2CBaseAddress, I2CSlaveAddress,
					     0x0069, Value, Mask);

	return Result;
}
/*****************************************************************************/
/**
*
* This function sets disables / enables the reference inputs
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS 
*    - XST_FAILURE 
*
* @note None.
*
******************************************************************************/
static int IDT_8T49N24x_ReferenceInput(u32 I2CBaseAddress, u8 I2CSlaveAddress,
					u8 Input, u8 Enable)
{
	int Result;
	u8 Value;
	u8 Mask;
	u8 Shift;

	if (Input == 1) {
		Shift = 5;
	} else {
		Shift = 4;
	}

	if (Enable) {
		Value = 0x00;		/* Enable */
	} else {
		Value = (1<<Shift);	/* Disable reference input  */
	}
	Mask = (1<<Shift);
	Result = IDT_8T49N24x_ModifyRegister(I2CBaseAddress, I2CSlaveAddress,
					     0x000a, Value, Mask);

	return Result;
}

/*****************************************************************************/
/**
*
* This function enables/disables the DPLL and APLL calibration
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS 
*    - XST_FAILURE 
*
* @note None.
*
******************************************************************************/
static int IDT_8T49N24x_Enable(u32 I2CBaseAddress, u8 I2CSlaveAddress,
					u8 Enable)
{
	int Result;
	u8 Value;
	u8 Mask;

	if (Enable) {
		/* Digital PLL enabled - Calibration
		 * logic for Analog PLL enabled */
		Value = 0x00;
	} else {
		/* Digital PLL disabled - Calibration
		 * logic for Analog PLL disabled */
		Value = 0x05;
	}

	Mask = 0x05;
	Result = IDT_8T49N24x_ModifyRegister(I2CBaseAddress, I2CSlaveAddress,
					     0x0070, Value, Mask);

	return Result;
}

/*****************************************************************************/
/**
*
* This function set the GPIO outputs
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS 
*    - XST_FAILURE 
*
* @note None.
*
******************************************************************************/
//static int IDT_8T49N24x_GPIO(u32 I2CBaseAddress, u8 I2CSlaveAddress)
//{
//	int Result;
//
//	/* GPIO_DIR
//	 * GPIO3 is output
//	 * Rest of the pins are input
//	 * */
//	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
//					  0x0030, 0x08);
//
//	/* GPIxSEL[2] */
//	Result |= IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
//					  0x0031, 0x00);
//
//	/* GPIxSEL[1] */
//	Result |= IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
//					  0x0032, 0x00);
//
//	/* GPIxSEL[0] */
//	Result |= IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
//					  0x0033, 0x00);
//
//	/* GPOxSEL[2] */
//	Result |= IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
//					  0x0034, 0x00);
//
//	/* GPOxSEL[1] */
//	Result |= IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
//					  0x0035, 0x00);
//
//	/* GPOxSEL[0] */
//	Result |= IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
//					  0x0036, 0x08);
//
//	if (Result != XST_SUCCESS)
//		return XST_FAILURE;
//	else
//		return XST_SUCCESS;
//}

/*****************************************************************************/
/**
*
* This function set the input reference
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS 
*    - XST_FAILURE 
*
* @note None.
*
******************************************************************************/
//static int IDT_8T49N24x_SelectInputReference(u32 I2CBaseAddress,
//					u8 I2CSlaveAddress, u8 Input)
//{
//	int Result;
//	u8 Value;
//	u8 Mask;
//	u8 Shift;
//
//	Shift = 5;
//
//	if (Input == 1) {
//		/* Clock 1 */
//		Value = (0x05 << Shift);
//	} else {
//		/* Clock 0 */
//		Value = (0x04 << Shift);
//	}
//
//	Mask = 0x07 << Shift;
//
//	Result = IDT_8T49N24x_ModifyRegister(I2CBaseAddress, I2CSlaveAddress,
//					     0x0008, Value, Mask);
//
//	return Result;
//}

/*****************************************************************************/
/**
*
* This function set the output drivers
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS 
*    - XST_FAILURE 
*
* @note None.
*
******************************************************************************/
//static int IDT_8T49N24x_OutputDriver(u32 I2CBaseAddress, u8 I2CSlaveAddress,
//					u8 Output, u8 Enable)
//{
//	int Result;
//	u8 Value;
//	u8 Mask;
//	u16 Address;
//	u8 Shift;
//
//	/* OUTEN */
//	Mask = (1 << Output);
//	if (Enable)
//		Value = Mask;
//	else
//		Value = 0x00;
//
//	Result = IDT_8T49N24x_ModifyRegister(I2CBaseAddress, I2CSlaveAddress,
//					     0x0039, Value, Mask);
//
//	/* OUTMODE */
//	switch (Output) {
//		case 0 :
//			Address = 0x003e;
//			Shift = 1;
//			break;
//
//		case 1 :
//			Address = 0x003e;
//			Shift = 5;
//			break;
//
//		case 2 :
//			Address = 0x003d;
//			Shift = 1;
//			break;
//
//		case 3 :
//			Address = 0x003d;
//			Shift = 5;
//			break;
//	}
//
//	if (Enable)
//		Value = 0x02;	/* LVDS */
//	else
//		Value = 0x00;	/* High-impedance */
//
//	Value <<= Shift;
//	Mask = (0x07 << Shift);
//
//	Result |= IDT_8T49N24x_ModifyRegister(I2CBaseAddress, I2CSlaveAddress,
//					      Address, Value, Mask);
//
//	/* Qm_DIS */
//	if (!Enable)
//		Value = (1 << Output);
//	else
//		Value = 0;
//
//	Mask = (1 << Output);
//
//	Result |= IDT_8T49N24x_ModifyRegister(I2CBaseAddress, I2CSlaveAddress,
//					      0x006f, Value, Mask);
//
//	if (Result != XST_SUCCESS)
//		return XST_FAILURE;
//	else
//		return XST_SUCCESS;
//}

/*****************************************************************************/
/**
*
* This function sets the lock mode
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS 
*    - XST_FAILURE 
*
* @note None.
*
******************************************************************************/
//static int IDT_8T49N24x_LockMode(u32 I2CBaseAddress, u8 I2CSlaveAddress)
//{
//	int Result;
//	u8 Value;
//	u8 Mask;
//
//	Value = 0x02;
//	Mask = 0x02;
//	Result = IDT_8T49N24x_ModifyRegister(I2CBaseAddress, I2CSlaveAddress,
//					     0x006c, Value, Mask);
//
//	return Result;
//}

int IDT_8T49N24x_SetGPOut(u32 I2CBaseAddress, u8 I2CSlaveAddress,
				u8 PortID, u8 Set)
{
	if (PortID > 3) {
		print("Invalid port ID\n\r");
		return XST_FAILURE;
	}
	
	u32 ByteCount = 0;
	u8 Data = 0;
	u8 Buffer[3];
	
	Buffer[0] = 0x00; /* MSB RegAddr */
	Buffer[1] = 0x38; /* LSB RegAddr */
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer,
			      2, XIIC_REPEATED_START);
	if (ByteCount != 2) {
		return XST_FAILURE;
	}

	ByteCount = XIic_Recv(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer,
			      1, XIIC_STOP);
	if (ByteCount != 1) {
		return XST_FAILURE;
	}
	Data = Buffer[0];
	
	if (Set == TRUE) {
		Data |= (1<<PortID);
	} else {
		Data &= ~(1<<PortID);
	}
	
	Buffer[0] = 0x00; /* MSB RegAddr */
	Buffer[1] = 0x38; /* LSB RegAddr */
	Buffer[2] = Data;
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer,
			      3, XIIC_STOP);
	if (ByteCount != 3) {
		return XST_FAILURE;
	}
	
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function initializes the IDT 8TN49N24x with default values
* for use with the Video FMC.
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Initialization was successful.
*    - XST_FAILURE I2C write error.
*
* @note None.
*
******************************************************************************/
int IDT_8T49N24x_Init(u32 I2CBaseAddress, u8 I2CSlaveAddress)
{
	int Result;

	/* Check device ID */
	Result = IDT_8T49N24x_CheckDeviceID(I2CBaseAddress, I2CSlaveAddress);

	if (Result == XST_SUCCESS) {

		/* Disable DPLL and APLL calibration
		 * The i2c interface is clocked by the APLL.
		 * During the PLL parameters update, the i2c might 
		 * become unresponsive.
		 * To prevent this, the DPLL and APLL calibration are 
		 * disabled during the i2c transactions
		 * */
		Result = IDT_8T49N24x_Enable(I2CBaseAddress, I2CSlaveAddress,
					     FALSE);

		/* Configure device */
		Result = IDT_8T49N24x_Configure_JA(I2CBaseAddress,
						   I2CSlaveAddress);

		/* Enable DPLL and APLL calibration */
		Result = IDT_8T49N24x_Enable(I2CBaseAddress, I2CSlaveAddress,
					     TRUE);

		return Result;
	}

	return XST_FAILURE;
}

/*****************************************************************************/
/**
*
* This function the IDT 8TN49N24x device with the data from the configuration table.
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return None
*
* @note None.
*
******************************************************************************/
//static int IDT_8T49N24x_Configure(u32 I2CBaseAddress, u8 I2CSlaveAddress)
//{
//	int Result;
//	u32 Index;
//	xil_printf("freerun\n\r");
//
//	/* The configuration is started from address 0x08 */
//	for (Index = 8 ; Index < sizeof(IDT_8T49N24x_Config_Syn) ; Index++) {
//		/* Skip address 0x70
//		 * Address 0x70 enables the DPLL and APLL calibration */
//		if (Index != 0x070) {
//			Result = IDT_8T49N24x_SetRegister(I2CBaseAddress,
//						I2CSlaveAddress, Index,
//						IDT_8T49N24x_Config_Syn[Index]);
//		}
//	}
//	return Result;
//}

static int IDT_8T49N24x_Configure_JA(u32 I2CBaseAddress, u8 I2CSlaveAddress)
{
	int Result;
	u32 Index;

	/* The configuration is started from address 0x08a */
	for (Index = 8 ; Index < sizeof(IDT_8T49N24x_Config_JA) ; Index++) {
		/* Skip address 0x70
		 * Address 0x70 enables the DPLL and APLL calibration */
		if (Index != 0x070) {
			Result = IDT_8T49N24x_SetRegister(I2CBaseAddress,
						I2CSlaveAddress, Index,
						IDT_8T49N24x_Config_JA[Index]);
		}
	}
	return Result;
}

/*****************************************************************************/
/**
*
* This function displays a registerdump of the IDT 8TN49N24x device.
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return None
*
* @note None.
*
******************************************************************************/
void IDT_8T49N24x_RegisterDump(u32 I2CBaseAddress, u8 I2CSlaveAddress)
{
	u8 Data;
	u32 i;
	int Result;

	xil_printf("Register dump\n\r");

	Result = IDT_8T49N24x_CheckDeviceID(I2CBaseAddress, I2CSlaveAddress);

	if (Result == XST_SUCCESS) {
		print("\n\r");
		print("---------------------\n\r");
		print("- IDT8T49N241 I2C dump:\n\r");
		print("---------------------\n\r");

		xil_printf("     ");
		for (i = 0 ; i < 16 ; i++)
			xil_printf("+%01x ", i);

		xil_printf("\n\r     ");
		for (i = 0 ; i < 16 ; i++)
			xil_printf("---");
		
		for (i = 0 ; i < 256 ; i++) {
			if ((i % 16) == 0) {
				xil_printf("\n\r%02x : ", i);
			}
			Data = IDT_8T49N24x_GetRegister(I2CBaseAddress,
							I2CSlaveAddress, i);
			xil_printf("%02x ", Data);
		}
		
		xil_printf("\n\r");
	} else {
		xil_printf("IDT 8T49N241 not found!\n\r");
	}
}

/** @} */
