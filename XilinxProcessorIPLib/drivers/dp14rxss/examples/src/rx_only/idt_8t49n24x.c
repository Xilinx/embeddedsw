/******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
*                       8T49N28X-FrequencyProgrammingGuide-register-calculations.py script
* 2.00  MG     16/08/15 Major update
* </pre>
*
******************************************************************************/

#define new

#ifdef new
/***************************** Include Files *********************************/
#include "idt_8t49n24x.h"

#include "xiic.h"

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#include <stdlib.h>
#include "math.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

int ceil_func(double x);
int floor_func(double x);
int round_func(double number);

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
			/* Read data. */
			ByteCount = XIic_Recv(I2CBaseAddress, I2CSlaveAddress,
						(u8*)Buffer, 1, XIIC_STOP);
			if (ByteCount != 1) {
				Exit = FALSE;
				Exit = TRUE;
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

	/* Clear masked bits */
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
	if (DeviceId == 0x0606)
		return XST_SUCCESS;
	else
		return XST_FAILURE;
}

/* Get valid Integer output divider values. */
static int IDT_8T49N24x_GetIntDivTable(int FOut, int *DivTable, u8 Bypass)
{
	int NS1_Options[4] = {1,4,5,6};
	int index;
	int NS2Min = 1;
	int NS2Max = 1;

	int NS2Temp;
	int OutDivTemp;
	u32 VCOTemp;
	int i;

	int OutDivMin = (int)ceil_func(IDT_8T49N24X_FVCO_MIN/FOut);
	int OutDivMax = (int)floor_func(IDT_8T49N24X_FVCO_MAX/FOut);

	int Count = 0;
	int *DivTablePtr = DivTable;

	if (Bypass == TRUE) {
		index = 0;
	} else {
		index = 1;
	}

	for (i = index; i < (sizeof(NS1_Options) / sizeof(int)); i++) {
		if ((NS1_Options[i] == OutDivMin) ||
		    (NS1_Options[i] == OutDivMax)) {
			/* This is for the case where we want to bypass NS2. */
			NS2Min = 0;
			NS2Max = 0;
		}
	}

	if (NS2Min == 1) {
		/* if this test passes, then we know
		 * we're not in the bypass case */

		/* The last element in the list */
		NS2Min = (int)ceil_func(OutDivMin / NS1_Options[3] / 2);
		NS2Max = (int)floor_func(OutDivMax / NS1_Options[index] / 2);

		if (NS2Max == 0) {
			/* Because we're rounding-down for the max,
			 * we may end-up with it being 0,
			 * in which case we need to make it 1
			 * */
			NS2Max = 1;
		}
	}

	NS2Temp = NS2Min;

	while (NS2Temp <= NS2Max) {
		for (i = index; i < (sizeof(NS1_Options) / sizeof(int)); i++) {
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
	int i;

	/* Get the valid integer dividers. */
	DivTableCount = IDT_8T49N24x_GetIntDivTable(FOut, DivTable, FALSE);

	/* Find the highest divider */
	for (i = 0; i < DivTableCount; i++) {
		if (MaxDiv < DivTable[i]) {
			MaxDiv = DivTable[i];
		}
	}

	FVCO = (double)FOut*MaxDiv;

	/* *************************************************
	 * INTEGER DIVIDER: Determine NS1 register setting *
	 * ************************************************/

	/* Only use the divide-by-1 option for
	 * really small divide ratios
	 * Note that this option will never be on
	 * the list for the Q0 - Q3 dividers
	 * */
	if (MaxDiv < 4) {

	}

	/* Make sure we can divide the ratio by 4 in NS1 and
	 * by 1 or an even number in NS2
	 * */
	if ((MaxDiv == 4) ||
	    (MaxDiv % 8 == 0)) {
		NS1_RegSettings = 2; // Divide by 4 register selection
	}

	/* Make sure we can divide the ratio by 5 in NS1 and by 1
	 * or an even number in NS2
	 * */
	if ((MaxDiv == 5) ||
	    (MaxDiv % 10 == 0)) {
		NS1_RegSettings = 0; // Divide by 5 register selection
	}

	/* Make sure we can divide the ratio by 6 in NS1 and by 1
	 * or an even number in NS2
	 * */
	if ((MaxDiv == 6) ||
	    (MaxDiv % 12 == 0)) {
		NS1_RegSettings = 1; // Divide by 6 register setting
	}

	/* *************************************************
	 * INTEGER DIVIDER: Determine NS2 register setting *
	 * ************************************************/

	switch (NS1_RegSettings) {
	case (0):
		NS1Ratio = 5;
		break;

	case (1):
		NS1Ratio = 6;
		break;

	case (2):
		NS1Ratio = 4;
		break;

	case (3): /* This is the bypass (divide-by-1) option. */
		NS1Ratio = 1;
		break;

	default:
		NS1Ratio = 6;
		break;
	}

	NS2Ratio = (int)floor_func(MaxDiv / NS1Ratio);

	NS2_RegSettings = (int)floor_func(NS2Ratio/2);

	/* ********************
	 * FRACTIONAL DIVIDER *
	 * *******************/
	FracDiv = FVCO/FOut;

	u32 N_Q2 = 0;
	u32 NFRAC_Q2 = 0;

	double frac_numerator = round_func(((FracDiv / 2.0) -
					    (int)(FracDiv / 2.0)) *
					   pow(2,28));

	/* This is the case where the fractional portion is 0. */
	/* Due to precision limitations, sometimes fractional
	 * portion of the Effective divider gets rounded to 1.
	 * This checks for that condition
	 * */
	if ((frac_numerator >= 268435456) ||
	    ((FracDiv/2.0) == (int)(FracDiv/2.0))) {
		N_Q2 = (int)round_func(FracDiv / 2.0);
		NFRAC_Q2 = 0;
	} else {
		/* This is the case where the
		 * fractional portion is not 0. */
		N_Q2 = (int)floor_func(FracDiv / 2.0);
		NFRAC_Q2 = (int)frac_numerator;
	}

	/* ***************************************************
	 * Calculate the Upper Loop Feedback divider setting *
	 * **************************************************/

	UpperFBDiv = (double)(FVCO) / (2 * IDT_8T49N24X_XTAL_FREQ);
	DSMInt_RegSettings = (int)floor_func(UpperFBDiv);

	DSMFrac_RegSettings = (int)round_func((UpperFBDiv -
					       floor_func(UpperFBDiv)) *
					      pow(2,21));

	/* ***************************************************
	 * Calculate the Lower Loop Feedback divider and     *
	 * Input Divider                                     *
	 * **************************************************/

	Ratio = FVCO/FIn;

	int M1 = 0;
	int PMin = (int)FIn / IDT_8T49N24X_FPD_MAX;

	/* This M1 divider sets the input PFD
	 * frequency at 128KHz, the set max
	 * */
	//int M1Min = (int)(FVCO / IDT_8T49N24X_FPD_MAX);

	int M1_default = 0;
	int P_default = 0;
	int error_tmp = 999999;
	int error = 99999999;

	int count = 0;

	/* Start from lowest divider and iterate until 0 error is found
	 * or the divider limit is exhausted.
	 * Keep the setting with the lowest error
	 * */
	for (i = PMin; i <= IDT_8T49N24X_P_MAX; i++) {
		M1 = (int)round_func(i*Ratio);
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
		} else {
			break;
		}
	}

	RegSettings->NS1_Qx = NS1_RegSettings;
	RegSettings->NS2_Qx = NS2_RegSettings;

	RegSettings->N_Qx = N_Q2;
	RegSettings->NFRAC_Qx = NFRAC_Q2;

	RegSettings->DSM_INT = DSMInt_RegSettings;
	RegSettings->DSM_FRAC = DSMFrac_RegSettings;
	RegSettings->M1_x = M1_default;
	RegSettings->PRE_x = P_default;

/*
	RegSettings->NS1_Qx = NS1_RegSettings;
	RegSettings->NS2_Qx = NS2_RegSettings;

	RegSettings->N_Qx = 8;
	RegSettings->NFRAC_Qx = 0;

	RegSettings->DSM_INT = 44;
	RegSettings->DSM_FRAC = 1153434;
	RegSettings->M1_x = 27840;
	RegSettings->PRE_x = 1740;

	xil_printf("NS1 : %0d\n\r", NS1_RegSettings);
	xil_printf("NS2 : %0d\n\r", NS2_RegSettings);
	xil_printf("N_Qx : %0d\n\r", RegSettings->N_Qx);
	xil_printf("NFRAC_Qx : %0d\n\r", RegSettings->NFRAC_Qx);
	xil_printf("DSM_INT : %0d\n\r", RegSettings->DSM_INT);
	xil_printf("DSM_FRAC : %0d\n\r", RegSettings->DSM_FRAC);
	xil_printf("M1_x : %0d\n\r", RegSettings->M1_x);
	xil_printf("PRE_x : %0d\n\r", RegSettings->PRE_x);
*/

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

	if ((FIn < IDT_8T49N24X_FIN_MIN) &&
	    (FIn > IDT_8T49N24X_FIN_MAX)) {
		return XST_FAILURE;
	}

	if ((FOut < IDT_8T49N24X_FOUT_MIN) &&
	    (FOut > IDT_8T49N24X_FOUT_MAX)) {
		return XST_FAILURE;
	}

	IDT_8T49N24x_Settings RegSettings;

	/* Calculate settings */
	IDT_8T49N24x_CalculateSettings(FIn, FOut, &RegSettings);

	/* Disable DPLL and APLL calibration */
	Result = IDT_8T49N24x_Enable(I2CBaseAddress, I2CSlaveAddress, FALSE);

#if DELAY
	usleep(500000);
#else
	usleep(300000);
#endif
/*
	if (!FreeRun) {

		// Configure device
		Result = IDT_8T49N24x_Configure_JA(I2CBaseAddress, I2CSlaveAddress);

	// Enable DPLL and APLL calibration
	Result = IDT_8T49N24x_Enable(I2CBaseAddress, I2CSlaveAddress, TRUE);
}
	else {
		// Configure device
//		Result = IDT_8T49N24x_Configure(I2CBaseAddress, I2CSlaveAddress);
*/
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

	/* Pre-divider Input 0. */
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

	/* Enable DPLL and APLL calibration */
	Result = IDT_8T49N24x_Enable(I2CBaseAddress, I2CSlaveAddress, TRUE);
//}

	return Result;
}

/*
int IDT_8T49N24x_SetClock(u32 I2CBaseAddress, u8 I2CSlaveAddress, int FIn, int FOut, u8 FreeRun)
{
	u32 ByteCount = 0;
	u8 Data = 0;
	u8 Buffer[16];

	if ((FIn < IDT_8T49N24X_FIN_MIN) &&
	   (FIn > IDT_8T49N24X_FIN_MAX)) {
		return XST_FAILURE;
	}

	if ((FOut < IDT_8T49N24X_FOUT_MIN) &&
		(FOut > IDT_8T49N24X_FOUT_MAX)) {
		return XST_FAILURE;
	}


	IDT_8T49N24x_Settings RegSettings;

	IDT_8T49N24x_CalculateSettings(FIn, FOut, &RegSettings);

	// TODO: Add support for independent inputs
	// Write Pre-Divider values for the Input Reference
	Buffer[0] = 0x00; // MSB
	Buffer[1] = 0x0B; // LSB
	// PRE0
	Data = ((RegSettings.PRE_x >> 16) & 0x1F); // PRE0[20:16]
	Buffer[2] = Data;
	Data = ((RegSettings.PRE_x >> 8) & 0xFF); // PRE0[15:8]
	Buffer[3] = Data;
	Data = (RegSettings.PRE_x & 0xFF); // PRE0[7:0]
	Buffer[4] = Data;
	// PRE1
	Data = ((RegSettings.PRE_x >> 16) & 0x1F); // PRE1[20:16]
	Buffer[5] = Data;
	Data = ((RegSettings.PRE_x >> 8) & 0xFF); // PRE1[15:8]
	Buffer[6] = Data;
	Data = (RegSettings.PRE_x & 0xFF); // PRE1[7:0]
	Buffer[7] = Data;
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 8, XIIC_STOP);
	if (ByteCount != 8) {
		return XST_FAILURE;
	}

	// Write M1 Feedback Divider Ratio for Input Reference
	Buffer[0] = 0x00; // MSB
	Buffer[1] = 0x11; // LSB
	// M1_0
	Data = ((RegSettings.M1_x >> 16) & 0xFF); // M1_0[23:16]
	Buffer[2] = Data;
	Data = ((RegSettings.M1_x >> 8) & 0xFF); // M1_0[15:8]
	Buffer[3] = Data;
	Data = (RegSettings.M1_x & 0xFF); // M1_0[7:0]
	Buffer[4] = Data;
	// M1_1
	Data = ((RegSettings.M1_x >> 16) & 0x1F); // M1_1[23:16]
	Buffer[5] = Data;
	Data = ((RegSettings.M1_x >> 8) & 0xFF); // M1_1[15:8]
	Buffer[6] = Data;
	Data = (RegSettings.M1_x & 0xFF); // M1_1[7:0]
	Buffer[7] = Data;
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 8, XIIC_STOP);
	if (ByteCount != 8) {
		return XST_FAILURE;
	}

	// Write DPLL Feedback Configuration
	Buffer[0] = 0x00; // MSB
	Buffer[1] = 0x25; // LSB
	// DSM_INT
	Data = ((RegSettings.DSM_INT >> 8) & 0x1); // DSM_INT[8]
	Buffer[2] = Data;
	Data = (RegSettings.DSM_INT & 0xFF); // DSM_INT[7:0]
	Buffer[3] = Data;
	Buffer[4] = 0; // RSVD
	// DSM_FRAC
	Data = ((RegSettings.DSM_FRAC >> 16) & 0x1F); // DSM_FRAC[20:16]
	Buffer[5] = Data;
	Data = ((RegSettings.DSM_FRAC >> 8) & 0xFF); // DSM_FRAC[15:8]
	Buffer[6] = Data;
	Data = (RegSettings.DSM_FRAC & 0xFF); // DSM_FRAC[7:0]
	Buffer[7] = Data;
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 8, XIIC_STOP);
	if (ByteCount != 8) {
		return XST_FAILURE;
	}

	// Write Output Divider Control
	// Integer portion
	Buffer[0] = 0x00; // MSB
	Buffer[1] = 0x45; // LSB
	// N_Q2
	Data = ((RegSettings.N_Qx >> 16) & 0x3); // N_Q2[17:16]
	Buffer[2] = Data;
	Data = ((RegSettings.N_Qx >> 8) & 0xFF); // N_Q2[15:8]
	Buffer[3] = Data;
	Data = (RegSettings.N_Qx & 0xFF); // N_Q2[7:0]
	Buffer[4] = Data;
	// N_Q3
	Data = ((RegSettings.N_Qx >> 16) & 0x3); // N_Q3[17:16]
	Buffer[5] = Data;
	Data = ((RegSettings.N_Qx >> 8) & 0xFF); // N_Q3[15:8]
	Buffer[6] = Data;
	Data = (RegSettings.N_Qx & 0xFF); // N_Q3[7:0]
	Buffer[7] = Data;
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 8, XIIC_STOP);
	if (ByteCount != 8) {
		return XST_FAILURE;
	}

	// Fractional portion
	Buffer[0] = 0x00; // MSB
	Buffer[1] = 0x5B; // LSB
	// NFRAC_Q2
	Data = ((RegSettings.NFRAC_Qx >> 24) & 0xF); // NFRAC_Q2[27:24]
	Buffer[2] = Data;
	Data = ((RegSettings.NFRAC_Qx >> 16) & 0xFF); // NFRAC_Q2[23:16]
	Buffer[3] = Data;
	Data = ((RegSettings.NFRAC_Qx >> 8) & 0xFF); // NFRAC_Q2[15:8]
	Buffer[4] = Data;
	Data = (RegSettings.NFRAC_Qx & 0xFF); // NFRAC_Q2[7:0]
	Buffer[5] = Data;
	// NFRAC_Q3
	Data = ((RegSettings.NFRAC_Qx >> 24) & 0xF); // NFRAC_Q3[27:24]
	Buffer[6] = Data;
	Data = ((RegSettings.NFRAC_Qx >> 16) & 0xFF); // NFRAC_Q3[23:16]
	Buffer[7] = Data;
	Data = ((RegSettings.NFRAC_Qx >> 8) & 0xFF); // NFRAC_Q3[15:8]
	Buffer[8] = Data;
	Data = (RegSettings.NFRAC_Qx & 0xFF); // NFRAC_Q3[7:0]
	Buffer[9] = Data;
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 10, XIIC_STOP);
	if (ByteCount != 10) {
		return XST_FAILURE;
	}

	// Analog PLL Control
	Buffer[0] = 0x00; // MSB
	Buffer[1] = 0x69; // LSB
	if (FreeRun == TRUE) {
		Buffer[2] = 0x0A; // SYN_MODE = 1, rest is default
	}
	else {
		Buffer[2] = 0x02; // SYN_MODE = 0, rest is default
	}
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 3, XIIC_STOP);
	if (ByteCount != 3) {
		return XST_FAILURE;
	}

	// Set operating mode
	Buffer[0] = 0x00; // MSB
	Buffer[1] = 0x0A; // LSB
	if (FreeRun == TRUE) {
		Buffer[2] = 0x31; // STATE = Force FREERUN and disable input references
	}
	else {
		Buffer[2] = 0x02; // STATE = Force NORMAL, rest is default
	}
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 3, XIIC_STOP);
	if (ByteCount != 3) {
		return XST_FAILURE;
	}

	Buffer[0] = 0x00; // MSB RegAddr
	Buffer[1] = 0x6D; // LSB RegAddr
	if (FreeRun == TRUE) {
		// Set Input Reference in power down state
		Buffer[2] = 0x3;
	}
	else {
		Buffer[2] = 0x0;
	}
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 3, XIIC_STOP);
	if (ByteCount != 3) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
*/

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
					  Address + 1, Data);

	/* PREx[7:0] */
	Data = (Value & 0xff);
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  Address + 2, Data);

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
					  Address + 1, Data);

	/* M1x[7:0] */
	Data = (Value & 0xff);
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  Address + 2, Data);

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
static int IDT_8T49N24x_DSMInteger(u32 I2CBaseAddress,
				u8 I2CSlaveAddress, u16 Value)
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
static int IDT_8T49N24x_DSMFractional(u32 I2CBaseAddress,
				u8 I2CSlaveAddress, u32 Value)
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
	case 0:
		Address = 0x003f;
		break;
	case 1:
		Address = 0x0042;
		break;
	case 2:
		Address = 0x0045;
		break;
	case 3:
		Address = 0x0048;
		break;
	}

	/* N_Qm[17:16] */
	Data = (Value >> 16) & 0x03;
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  Address, Data);

	/* N_Qm[15:8] */
	Data = (Value >> 8);
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  Address + 1, Data);

	/* N_Qm[7:0] */
	Data = (Value & 0xff);
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  Address + 2, Data);

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
	case 0:
		Address = 0x0000;
		break;
	case 1:
		Address = 0x0057;
		break;
	case 2:
		Address = 0x005b;
		break;
	case 3:
		Address = 0x005f;
		break;
	}

	/* NFRAC_Qm[27:24] */
	Data = (Value >> 24) & 0x0f;
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  Address, Data);

	/* NFRAC_Qm[23:16] */
	Data = (Value >> 16);
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  Address + 1, Data);

	/* NFRAC_Qm[15:8] */
	Data = (Value >> 8);
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  Address + 2, Data);

	/* NFRAC_Qm[7:0] */
	Data = (Value & 0xff);
	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					  Address + 3, Data);

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
		Value = 0x01;		// Force FREERUN
		Value |= (1<<4);	// Disable reference input 0
		Value |= (1<<5);	// Disable reference input 1
	} else {
		Value = 0x00;		// Run automatically
		Value |= (1<<5);	// Disable reference input 1
	}

	Mask = 0x33;
	Result = IDT_8T49N24x_ModifyRegister(I2CBaseAddress, I2CSlaveAddress,
					     0x000a, Value, Mask);

	/* Analog PLL */
	/* SYN_MODE */
	if (Synthesizer) {
		Value = (1<<3);		// Synthesizer mode
	} else {
		Value = 0x00;		// Jitter attenuator mode
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

	/* Enable */
	if (Enable) {
		Value = 0x00;		// Enable
	} else {
		 /* Disable */
		Value = (1<<Shift);	// Disable reference input
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
	//int Result;
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
	IDT_8T49N24x_ModifyRegister(I2CBaseAddress, I2CSlaveAddress,
					     0x0070, Value, Mask);
	return 0;
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
//	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
//					  0x0031, 0x00);
//
//	/* GPIxSEL[1] */
//	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
//					  0x0032, 0x00);
//
//	/* GPIxSEL[0] */
//	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
//					  0x0033, 0x00);
//
//	/* GPOxSEL[2] */
//	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
//					  0x0034, 0x00);
//
//	/* GPOxSEL[1] */
//	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
//					  0x0035, 0x00);
//
//	/* GPOxSEL[0] */
//	Result = IDT_8T49N24x_SetRegister(I2CBaseAddress, I2CSlaveAddress,
//					  0x0036, 0x08);
//
//	return XST_SUCCESS;
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
//				u8 I2CSlaveAddress, u8 Input)
//{
//	int Result;
//	u8 Value;
//	u8 Mask;
//	u8 Shift;
//
//	Shift = 5;
//
//	/* Clock 1 */
//	if (Input == 1) {
//		Value = (0x05 << Shift);
//	} else {
//		/* Clock 0 */
//		Value = (0x04 << Shift);
//	}
//	Mask = 0x07 << Shift;
//
//	Result = IDT_8T49N24x_ModifyRegister(I2CBaseAddress, I2CSlaveAddress,
//					     0x0008, Value, Mask);
//
//	return XST_SUCCESS;
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
//				u8 Output, u8 Enable)
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
//	case 0:
//		Address = 0x003e;
//		Shift = 1;
//		break;
//
//	case 1:
//		Address = 0x003e;
//		Shift = 5;
//		break;
//
//	case 2:
//		Address = 0x003d;
//		Shift = 1;
//		break;
//
//	case 3:
//		Address = 0x003d;
//		Shift = 5;
//		break;
//	}
//
//	if (Enable)
//		Value = 0x02;	// LVDS
//	else
//		Value = 0x00;	// High-impedance
//
//	Value <<= Shift;
//	Mask = (0x07 << Shift);
//
//	Result = IDT_8T49N24x_ModifyRegister(I2CBaseAddress, I2CSlaveAddress,
//					     Address, Value, Mask);
//
//	/* Qm_DIS */
//	if (!Enable)
//		Value = (1 << Output);
//	else
//		Value = 0;
//
//	Mask = (1 << Output);
//
//	Result = IDT_8T49N24x_ModifyRegister(I2CBaseAddress, I2CSlaveAddress,
//					     0x006f, Value, Mask);
//
//	return XST_SUCCESS;
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
//	return XST_SUCCESS;
//}

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
	int Result=0;

	/* Check device ID. */
//	Result = IDT_8T49N24x_CheckDeviceID(I2CBaseAddress, I2CSlaveAddress);


	if (Result == XST_SUCCESS) {

		/* Disable DPLL and APLL calibration
		 * The i2c interface is clocked by the APLL.
		 * During the PLL parameters update, the i2c
		 * might become unresponsive.
		 * To prevent this, the DPLL and APLL calibration
		 * are disabled during the i2c transactions
		 * */
		//MB_Sleep(300);
		usleep(300);
		Result = IDT_8T49N24x_Enable(I2CBaseAddress,
					     I2CSlaveAddress, FALSE);
		//MB_Sleep(300);
		usleep(300);

		/* Configure device */
		Result = IDT_8T49N24x_Configure_JA(I2CBaseAddress,
						   I2CSlaveAddress);
		//MB_Sleep(300);
		usleep(300);

		/* Enable DPLL and APLL calibration */
		Result = IDT_8T49N24x_Enable(I2CBaseAddress,
					     I2CSlaveAddress, TRUE);
		//MB_Sleep(300);
		usleep(300);
///*
//
//		/* Set GPIO */
//		Result = IDT_8T49N24x_GPIO(I2CBaseAddress, I2CSlaveAddress);
//
//		/* Disable output 0 */
//		Result = IDT_8T49N24x_OutputDriver(I2CBaseAddress,
//						   I2CSlaveAddress,
//						   0, FALSE);
//
//		/* Disable output 1 */
//		Result = IDT_8T49N24x_OutputDriver(I2CBaseAddress,
//						   I2CSlaveAddress,
//						   1, FALSE);
//
//		// Enable output 2
//		Result = IDT_8T49N24x_OutputDriver(I2CBaseAddress,
//						   I2CSlaveAddress,
//						   2, TRUE);
//
//		// Enable output 3
//		Result = IDT_8T49N24x_OutputDriver(I2CBaseAddress,
//						   I2CSlaveAddress,
//						   3, TRUE);
//
//		// Lock mode
//		Result = IDT_8T49N24x_LockMode(I2CBaseAddress, I2CSlaveAddress);
//
//		// Select clock input 0
//		Result = IDT_8T49N24x_SelectInputReference(I2CBaseAddress,
//							   I2CSlaveAddress, 0);
//
//		return XST_SUCCESS;
//
//*/
		return Result;
	}

	return XST_FAILURE;
}

/*
int IDT_8T49N24x_Init(u32 I2CBaseAddress, u8 I2CSlaveAddress)
{
	u32 ByteCount = 0;
	u8 Data = 0;
	u8 Buffer[10];

	// Set Offset address to 0x0002
	Buffer[0] = 0x00; // MSB RegAddr
	Buffer[1] = 0x02; // LSB RegAddr
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 2, XIIC_REPEATED_START);
	if (ByteCount != 2) {
		return XST_FAILURE;
	}
	ByteCount = XIic_Recv(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 4, XIIC_STOP);
	if (ByteCount != 4) {
		return XST_FAILURE;
	}

	//TODO
	// Check device ID and rev ID

	// Startup Control
	Buffer[0] = 0x00; // MSB RegAddr
	Buffer[1] = 0x00; // LSB RegAddr
	Buffer[2] = 0x09; //
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 3, XIIC_STOP);
	if (ByteCount != 3) {
		return XST_FAILURE;
	}

	// Configure the GPIO as General Purpose inputs/outputs
	Buffer[0] = 0x00; // MSB RegAddr
	Buffer[1] = 0x30; // LSB RegAddr
	Buffer[2] = 0x08; // GPIO_DIR -> GPIO3 is output, the rest is input
	Buffer[3] = 0x00; // GPIxSEL[2]
	Buffer[4] = 0x00; // GPIxSEL[1]
	Buffer[5] = 0x00; // GPIxSEL[0]
	Buffer[6] = 0x00; // GPOxSEL[2]
	Buffer[7] = 0x00; // GPOxSEL[1]
	Buffer[8] = 0x08; // GPOxSEL[0]
	Buffer[9] = 0x00; // RSVD
	Buffer[10] = 0x00; // GPO[3:0]
	ByteCount = XIic_Send(XPAR_IIC_0_BASEADDR, 0x7C,
			      (u8*)Buffer, 11, XIIC_STOP);
	if (ByteCount != 11) {
		xil_printf("I2C write error: %d\n\r", ByteCount);
	}

	// Set the default output mode to LVDS
	Buffer[0] = 0x00; // MSB RegAddr
	Buffer[1] = 0x3D; // LSB RegAddr

	Data = 0;
	Data |= (0x2 << 5); // OUTMODE3 -> LVDS
	Data |= (0x2 << 1); // OUTMODE2 -> LVDS
	Buffer[2] = Data;

	Data = 0;
	Data |= (0x2 << 5); // OUTMODE1 -> LVDS
	Data |= (0x2 << 1); // OUTMODE0 -> LVDS
	Buffer[3] = Data;
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 4, XIIC_STOP);
	if (ByteCount != 4) {
		return XST_FAILURE;
	}

	// Enable all output buffers
	Buffer[0] = 0x00; // MSB RegAddr
	Buffer[1] = 0x39; // LSB RegAddr
	Buffer[2] = 0x0F; // OUTEN[3:0]
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 3, XIIC_STOP);
	if (ByteCount != 3) {
		return XST_FAILURE;
	}

	Buffer[0] = 0x00; // MSB RegAddr
	Buffer[1] = 0x6C; // LSB RegAddr
	Buffer[2] = 0x02; // LCKMODE = 1, rest is default
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 3, XIIC_STOP);
	if (ByteCount != 3) {
		return XST_FAILURE;
	}

	// Only Outputs Q2 and Q3 are used.
	// Set Q0 and Q1 in power down state.
	Buffer[0] = 0x00; // MSB RegAddr
	Buffer[1] = 0x6F; // LSB RegAddr
	Buffer[2] = 0x03; // Qx_DIS[3:0]
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 3, XIIC_STOP);
	if (ByteCount != 3) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
*/

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
//		 * Address 0x70 enables the DPLL and APLL calibration
//		 * */
//		if (Index != 0x070) {
//			Result = IDT_8T49N24x_SetRegister(I2CBaseAddress,
//							  I2CSlaveAddress,
//							  Index,
//					IDT_8T49N24x_Config_Syn[Index]);
//		}
//	}
//	return Result;
//}

static int IDT_8T49N24x_Configure_JA(u32 I2CBaseAddress, u8 I2CSlaveAddress)
{
	int Result;
	u32 Index;

	/* The configuration is started from address 0x08 */
	for (Index = 8 ; Index < sizeof(IDT_8T49N24x_Config_JA) ; Index++) {
		/* Skip address 0x70
		 * Address 0x70 enables the DPLL and APLL calibration
		 * */
		if (Index != 0x070) {
			Result = IDT_8T49N24x_SetRegister(I2CBaseAddress,
							  I2CSlaveAddress,
							  Index,
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
		for (i=0; i<16; i++)
			xil_printf("+%01x ", i);

		xil_printf("\n\r     ");
		for (i=0; i<16; i++)
			xil_printf("---");

		for (i = 0; i < 256; i++) {
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
#else

/*****************************************************************************/

/***************************** Include Files *********************************/
#include "idt_8t49n24x.h"

#include "xiic.h"

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xparameters.h"

#include <stdlib.h>
#include <math.h>

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/* Get valid Integer output divider values. */
int IDT_8T49N24x_GetIntDivTable(int FOut, int *DivTable, u8 Bypass)
{
	int i;
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

	for (i = index; i < (sizeof(NS1_Options) / sizeof(int)); i++) {
		if ((NS1_Options[i] == OutDivMin) ||
		    (NS1_Options[i] == OutDivMax)) {
			/* This is for the case where we want to bypass NS2. */
			NS2Min = 0;
			NS2Max = 0;
		}
	}

	if (NS2Min == 1) {
		/* If this test passes, then we know
		 * we're not in the bypass case. */
		/* the last element in the list */
		NS2Min = (int)ceil(OutDivMin / NS1_Options[3] / 2);
		NS2Max = (int)floor(OutDivMax / NS1_Options[index] / 2);
		if (NS2Max == 0) {
			/* Because we're rounding-down for the max,
			 * we may end-up with it being 0,
			 * In which case we need to make it 1
			 * */
			NS2Max = 1;
		}
	}

	NS2Temp = NS2Min;

	while (NS2Temp <= NS2Max) {
		for (i = index; i < (sizeof(NS1_Options) / sizeof(int)); i++) {
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

int IDT_8T49N24x_GetSettings(int FIn, int FOut,
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
	int i;

	/* Get the valid integer dividers */
	DivTableCount = IDT_8T49N24x_GetIntDivTable(FOut, DivTable, FALSE);

	/* Find the highest divider */
	for (i = 0; i < DivTableCount; i++) {
		if (MaxDiv < DivTable[i]) {
			MaxDiv = DivTable[i];
		}
	}

	FVCO = (double)FOut*MaxDiv;

	/* *************************************************
	 * INTEGER DIVIDER: Determine NS1 register setting *
	 * ************************************************/

	/* Only use the divide-by-1 option for really small
	 * divide ratios, note that this option will never be
	 * on the list for the Q0 - Q3 dividers.
	 * */
	if (MaxDiv < 4) {

	}

	/* Make sure we can divide the ratio by 4 in NS1 and
	 * by 1 or an even number in NS2
	 * */
	if ((MaxDiv == 4) ||
	    (MaxDiv % 8 == 0)) {
		NS1_RegSettings = 2; // Divide by 4 register selection
	}

	/* Make sure we can divide the ratio by 5 in NS1 and
	 * by 1 or an even number in NS2
	 * */
	if ((MaxDiv == 5) ||
	    (MaxDiv % 10 == 0)) {
		NS1_RegSettings = 0; // Divide by 5 register selection
	}

	/* Make sure we can divide the ratio by 6 in NS1 and
	 * by 1 or an even number in NS2
	 * */
	if ((MaxDiv == 6) ||
	    (MaxDiv % 12 == 0)) {
		NS1_RegSettings = 1; // Divide by 6 register setting
	}

	/* *************************************************
	 * INTEGER DIVIDER: Determine NS2 register setting *
	 * ************************************************/
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

	/* ********************************************
	 * FRACTIONAL DIVIDER:                        *
	 * *******************************************/
	FracDiv = FVCO/FOut;

	u32 N_Q2 = 0;
	u32 NFRAC_Q2 = 0;

	double frac_numerator = round(((FracDiv / 2.0) -
				       (int)(FracDiv / 2.0)) *
				      pow(2 , 28));

	/* This is the case where the fractional portion is 0.
	 * Due to precision limitations, sometimes fractional
	 * portion of the Effective divider gets rounded to 1.
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

	/* ***************************************************
	 * Calculate the Upper Loop Feedback divider setting *
	 * **************************************************/

	UpperFBDiv = (double)(FVCO) / (2 * IDT_8T49N24X_XTAL_FREQ);
	DSMInt_RegSettings = (int)floor(UpperFBDiv);

	DSMFrac_RegSettings = (int)round((UpperFBDiv -
					  floor(UpperFBDiv)) *
					 pow(2 , 21));

	/* ***************************************************
	 * Calculate the Lower Loop Feedback divider and     *
	 * Input Divider.                                    *
	 * **************************************************/
	Ratio = FVCO/FIn;

	int M1 = 0;
	int PMin = (int)FIn / IDT_8T49N24X_FPD_MAX;
	/* This M1 divider sets the input PFD frequency
	 * at 128KHz, the set max. */
	int M1Min = (int)(FVCO / IDT_8T49N24X_FPD_MAX);

	int M1_default;
	int P_default;
	int error_tmp = 999999;
	int error = 99999999;

	int count = 0;

	/* Start from lowest divider and iterate until 0 error
	 * is found or the divider limit is exhausted.
	 * Keep the setting with the lowest error.
	 * */
	for (i = PMin; i <= IDT_8T49N24X_P_MAX; i++) {
		M1 = (int)round(i * Ratio);
		count++;
		if (M1 < IDT_8T49N24X_M_MAX) {
			error_tmp = (int)(Ratio * 1e9 - (M1 * 1e9 / i));

			if (abs(error_tmp) < error || error_tmp == 0) {
				error = abs(error_tmp);
				M1_default = M1;
				P_default = i;

				if (error_tmp == 0)
					break;
			}
		} else {
			break;
		}
	}

	RegSettings->NS1_Qx = NS1_RegSettings;
	RegSettings->NS2_Qx = NS2_RegSettings;

	RegSettings->N_Qx = N_Q2;
	RegSettings->NFRAC_Qx = NFRAC_Q2;

	RegSettings->DSM_INT = DSMInt_RegSettings;
	RegSettings->DSM_FRAC = DSMFrac_RegSettings;
	RegSettings->M1_x = M1_default;
	RegSettings->PRE_x = P_default;

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
	u32 ByteCount = 0;
	u8 Data = 0;
	u8 Buffer[16];

	if ((FIn < IDT_8T49N24X_FIN_MIN) &&
	    (FIn > IDT_8T49N24X_FIN_MAX)) {
		return XST_FAILURE;
	}

	if ((FOut < IDT_8T49N24X_FOUT_MIN) &&
	    (FOut > IDT_8T49N24X_FOUT_MAX)) {
		return XST_FAILURE;
	}

	IDT_8T49N24x_Settings RegSettings;

	IDT_8T49N24x_GetSettings(FIn, FOut, &RegSettings);

	/* TODO: Add support for independent inputs
	 * Write Pre-Divider values for the Input Reference
	 * */
	Buffer[0] = 0x00; // MSB
	Buffer[1] = 0x0B; // LSB
	/* PRE0 */
	Data = ((RegSettings.PRE_x >> 16) & 0x1F); // PRE0[20:16]
	Buffer[2] = Data;
	Data = ((RegSettings.PRE_x >> 8) & 0xFF); // PRE0[15:8]
	Buffer[3] = Data;
	Data = (RegSettings.PRE_x & 0xFF); // PRE0[7:0]
	Buffer[4] = Data;
	/* PRE1 */
	Data = ((RegSettings.PRE_x >> 16) & 0x1F); // PRE1[20:16]
	Buffer[5] = Data;
	Data = ((RegSettings.PRE_x >> 8) & 0xFF); // PRE1[15:8]
	Buffer[6] = Data;
	Data = (RegSettings.PRE_x & 0xFF); // PRE1[7:0]
	Buffer[7] = Data;
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 8, XIIC_STOP);
	if (ByteCount != 8) {
		return XST_FAILURE;
	}

	/* Write M1 Feedback Divider Ratio for Input Reference. */
	Buffer[0] = 0x00; // MSB
	Buffer[1] = 0x11; // LSB
	/* M1_0 */
	Data = ((RegSettings.M1_x >> 16) & 0xFF); // M1_0[23:16]
	Buffer[2] = Data;
	Data = ((RegSettings.M1_x >> 8) & 0xFF); // M1_0[15:8]
	Buffer[3] = Data;
	Data = (RegSettings.M1_x & 0xFF); // M1_0[7:0]
	Buffer[4] = Data;
	/* M1_1 */
	Data = ((RegSettings.M1_x >> 16) & 0x1F); // M1_1[23:16]
	Buffer[5] = Data;
	Data = ((RegSettings.M1_x >> 8) & 0xFF); // M1_1[15:8]
	Buffer[6] = Data;
	Data = (RegSettings.M1_x & 0xFF); // M1_1[7:0]
	Buffer[7] = Data;
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 8, XIIC_STOP);
	if (ByteCount != 8) {
		return XST_FAILURE;
	}

	/* Write DPLL Feedback Configuration. */
	Buffer[0] = 0x00; // MSB
	Buffer[1] = 0x25; // LSB
	/* DSM_INT. */
	Data = ((RegSettings.DSM_INT >> 8) & 0x1); // DSM_INT[8]
	Buffer[2] = Data;
	Data = (RegSettings.DSM_INT & 0xFF); // DSM_INT[7:0]
	Buffer[3] = Data;
	Buffer[4] = 0; // RSVD
	/* DSM_FRAC. */
	Data = ((RegSettings.DSM_FRAC >> 16) & 0x1F); // DSM_FRAC[20:16]
	Buffer[5] = Data;
	Data = ((RegSettings.DSM_FRAC >> 8) & 0xFF); // DSM_FRAC[15:8]
	Buffer[6] = Data;
	Data = (RegSettings.DSM_FRAC & 0xFF); // DSM_FRAC[7:0]
	Buffer[7] = Data;
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 8, XIIC_STOP);
	if (ByteCount != 8) {
		return XST_FAILURE;
	}

	/* Write Output Divider Control */
	/* Integer portion */
	Buffer[0] = 0x00; // MSB
	Buffer[1] = 0x45; // LSB
	/* N_Q2 */
	Data = ((RegSettings.N_Qx >> 16) & 0x3); // N_Q2[17:16]
	Buffer[2] = Data;
	Data = ((RegSettings.N_Qx >> 8) & 0xFF); // N_Q2[15:8]
	Buffer[3] = Data;
	Data = (RegSettings.N_Qx & 0xFF); // N_Q2[7:0]
	Buffer[4] = Data;
	/* N_Q3 */
	Data = ((RegSettings.N_Qx >> 16) & 0x3); // N_Q3[17:16]
	Buffer[5] = Data;
	Data = ((RegSettings.N_Qx >> 8) & 0xFF); // N_Q3[15:8]
	Buffer[6] = Data;
	Data = (RegSettings.N_Qx & 0xFF); // N_Q3[7:0]
	Buffer[7] = Data;
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 8, XIIC_STOP);
	if (ByteCount != 8) {
		return XST_FAILURE;
	}

	/* Fractional portion */
	Buffer[0] = 0x00; // MSB
	Buffer[1] = 0x5B; // LSB
	/* NFRAC_Q2 */
	Data = ((RegSettings.NFRAC_Qx >> 24) & 0xF); // NFRAC_Q2[27:24]
	Buffer[2] = Data;
	Data = ((RegSettings.NFRAC_Qx >> 16) & 0xFF); // NFRAC_Q2[23:16]
	Buffer[3] = Data;
	Data = ((RegSettings.NFRAC_Qx >> 8) & 0xFF); // NFRAC_Q2[15:8]
	Buffer[4] = Data;
	Data = (RegSettings.NFRAC_Qx & 0xFF); // NFRAC_Q2[7:0]
	Buffer[5] = Data;
	/* NFRAC_Q3 */
	Data = ((RegSettings.NFRAC_Qx >> 24) & 0xF); // NFRAC_Q3[27:24]
	Buffer[6] = Data;
	Data = ((RegSettings.NFRAC_Qx >> 16) & 0xFF); // NFRAC_Q3[23:16]
	Buffer[7] = Data;
	Data = ((RegSettings.NFRAC_Qx >> 8) & 0xFF); // NFRAC_Q3[15:8]
	Buffer[8] = Data;
	Data = (RegSettings.NFRAC_Qx & 0xFF); // NFRAC_Q3[7:0]
	Buffer[9] = Data;
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 10, XIIC_STOP);
	if (ByteCount != 10) {
		return XST_FAILURE;
	}

	/* Analog PLL Control */
	Buffer[0] = 0x00; // MSB
	Buffer[1] = 0x69; // LSB
	if (FreeRun == TRUE) {
		Buffer[2] = 0x0A; // SYN_MODE = 1, rest is default
	} else {
		Buffer[2] = 0x02; // SYN_MODE = 0, rest is default
	}
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 3, XIIC_STOP);
	if (ByteCount != 3) {
		return XST_FAILURE;
	}

	/* Set operating mode */
	Buffer[0] = 0x00; // MSB
	Buffer[1] = 0x0A; // LSB
	if (FreeRun == TRUE) {
		Buffer[2] = 0x31; // STATE = Force FREERUN and disable input references
	} else {
		Buffer[2] = 0x02; // STATE = Force NORMAL, rest is default
	}
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 3, XIIC_STOP);
	if (ByteCount != 3) {
		return XST_FAILURE;
	}

	Buffer[0] = 0x00; // MSB RegAddr
	Buffer[1] = 0x6D; // LSB RegAddr
	if (FreeRun == TRUE) {
		/* Set Input Reference in power down state */
		Buffer[2] = 0x3;
	} else {
		Buffer[2] = 0x0;
	}
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 3, XIIC_STOP);
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
	u32 ByteCount = 0;
	u8 Data = 0;
	u8 Buffer[10];

	/* Set Offset address to 0x0002 */
	Buffer[0] = 0x00; // MSB RegAddr
	Buffer[1] = 0x02; // LSB RegAddr
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 2, XIIC_REPEATED_START);
	if (ByteCount != 2) {
		return XST_FAILURE;
	}
	ByteCount = XIic_Recv(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 4, XIIC_STOP);
	if (ByteCount != 4) {
		return XST_FAILURE;
	}

	/* Check device ID and rev ID */

	/* Startup Control */
	Buffer[0] = 0x00; // MSB RegAddr
	Buffer[1] = 0x00; // LSB RegAddr
	Buffer[2] = 0x09; //
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 3, XIIC_STOP);
	if (ByteCount != 3) {
		return XST_FAILURE;
	}

	/* Configure the GPIO as General Purpose inputs/outputs. */
	Buffer[0] = 0x00; // MSB RegAddr
	Buffer[1] = 0x30; // LSB RegAddr
	Buffer[2] = 0x08; // GPIO_DIR -> GPIO3 is output, the rest is input
	Buffer[3] = 0x00; // GPIxSEL[2]
	Buffer[4] = 0x00; // GPIxSEL[1]
	Buffer[5] = 0x00; // GPIxSEL[0]
	Buffer[6] = 0x00; // GPOxSEL[2]
	Buffer[7] = 0x00; // GPOxSEL[1]
	Buffer[8] = 0x08; // GPOxSEL[0]
	Buffer[9] = 0x00; // RSVD
	Buffer[10] = 0x00; // GPO[3:0]
	ByteCount = XIic_Send(XPAR_IIC_0_BASEADDR, 0x7C,
			      (u8*)Buffer, 11, XIIC_STOP);
	if (ByteCount != 11) {
		xil_printf("I2C write error: %d\n\r", ByteCount);
	}

	/* Set the default output mode to LVDS. */
	Buffer[0] = 0x00; // MSB RegAddr
	Buffer[1] = 0x3D; // LSB RegAddr

	Data = 0;
	Data |= (0x2 << 5); // OUTMODE3 -> LVDS
	Data |= (0x2 << 1); // OUTMODE2 -> LVDS
	Buffer[2] = Data;

	Data = 0;
	Data |= (0x2 << 5); // OUTMODE1 -> LVDS
	Data |= (0x2 << 1); // OUTMODE0 -> LVDS
	Buffer[3] = Data;
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 4, XIIC_STOP);
	if (ByteCount != 4) {
		return XST_FAILURE;
	}

	/* Enable all output buffers */
	Buffer[0] = 0x00; // MSB RegAddr
	Buffer[1] = 0x39; // LSB RegAddr
	Buffer[2] = 0x0F; // OUTEN[3:0]
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 3, XIIC_STOP);
	if (ByteCount != 3) {
		return XST_FAILURE;
	}

	Buffer[0] = 0x00; // MSB RegAddr
	Buffer[1] = 0x6C; // LSB RegAddr
	Buffer[2] = 0x02; // LCKMODE = 1, rest is default
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 3, XIIC_STOP);
	if (ByteCount != 3) {
		return XST_FAILURE;
	}

	/* Only Outputs Q2 and Q3 are used.
	 * Set Q0 and Q1 in power down state.
	 * */
	Buffer[0] = 0x00; // MSB RegAddr
	Buffer[1] = 0x6F; // LSB RegAddr
	Buffer[2] = 0x03; // Qx_DIS[3:0]
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 3, XIIC_STOP);
	if (ByteCount != 3) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets the specified GPO of the IDT 8TN49N24x with default.
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
* @param PortID specifies the port to update. Valid values are 0 to 3.
* @param Set specifies the value for the port.
*    - TRUE Port is driven high
*    - FALSE Port is driver low
*
* @return
*    - XST_SUCCESS Initialization was successful.
*    - XST_FAILURE I2C write error or incorrect parameters detected.
*
* @note
*   The specified port must be configured as a general purpose
*   output prior to calling this function.
*
******************************************************************************/
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

	Buffer[0] = 0x00; // MSB RegAddr
	Buffer[1] = 0x38; // LSB RegAddr
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 2, XIIC_REPEATED_START);
	if (ByteCount != 2) {
		return XST_FAILURE;
	}
	ByteCount = XIic_Recv(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 1, XIIC_STOP);
	if (ByteCount != 1) {
		return XST_FAILURE;
	}

	Data = Buffer[0];

	if (Set == TRUE) {
		Data |= (1<<PortID);
	} else {
		Data &= ~(1<<PortID);
	}

	Buffer[0] = 0x00; // MSB RegAddr
	Buffer[1] = 0x38; // LSB RegAddr
	Buffer[2] = Data;
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 3, XIIC_STOP);
	if (ByteCount != 3) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
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
	u32 ByteCount = 0;
	u8 Buffer[256];
	int i;

	print("\n\r");
	print("---------------------\n\r");
	print("- IDT8T49N241 I2C dump:\n\r");
	print("---------------------\n\r");
	Buffer[0] = 0;
	Buffer[1] = 0;
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 2, XIIC_REPEATED_START);
	if (ByteCount != 2) {
		xil_printf("I2C write error: %d\n\r", ByteCount);
	}
	ByteCount = XIic_Recv(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 0x79, XIIC_STOP);
	if (ByteCount != 0x79) {
		xil_printf("I2C read error: %d\n\r", ByteCount);
	}

	for (i = 0; i < ByteCount; i++) {
		xil_printf("%02x : %02x\n\r", i, Buffer[i]);
	}
	print("\n\r");
}

/** @} */
#endif

int ceil_func(double x){
	return (int)( x < 0.0 ? x : x+0.9 );
}

int floor_func(double x){
	return (int)( x < 0.0 ? x-0.9 : x );
}

int round_func(double number)
{
    return (number >= 0) ? (int)(number + 0.5) : (int)(number - 0.5);
}
