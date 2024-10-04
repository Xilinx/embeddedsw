/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2023-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file ti_lmk03318.c
* @addtogroup TI_LMK03318
* @{
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* X.XX  XX     YY/MM/DD
* 1.00  RHe    16/06/20 Initial release.
* </pre>
*
******************************************************************************/

#include "ti_lmk03318.h"
#include "xiic.h"

/*****************************************************************************/
/**
*
* This function send a single byte to the TI LMK03318
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
static int TI_LMK03318_SetRegister(u32 I2CBaseAddress, u8 I2CSlaveAddress,
				u8 RegisterAddress, u8 Value)
{
	u32 ByteCount = 0;
	u8 Buffer[2];

	Buffer[0] = RegisterAddress;
	Buffer[1] = Value;
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 2, XIIC_STOP);

	if (ByteCount != 2) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function send a single byte to the TI LMK03318
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
static u8 TI_LMK03318_GetRegister(u32 I2CBaseAddress, u8 I2CSlaveAddress,
				u8 RegisterAddress)
{
	u8 Buffer[2];

	Buffer[0] = RegisterAddress;
	XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 1, XIIC_REPEATED_START);
	XIic_Recv(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 1, XIIC_STOP);

	return Buffer[0];
}

/*****************************************************************************/
/**
*
* This function modifies a single byte to the TI LMK03318
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
static int TI_LMK03318_ModifyRegister(u32 I2CBaseAddress, u8 I2CSlaveAddress,
				u16 RegisterAddress, u8 Value, u8 Mask)
{
	u8 Data;
	int Result;

	/* Read data */
	Data = TI_LMK03318_GetRegister(I2CBaseAddress, I2CSlaveAddress,
				       RegisterAddress);

	/* Clear masked bits */
	Data &= ~Mask;

	/* Update */
	Data |= (Value & Mask);

	/* Write data */
	Result = TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					 RegisterAddress, Data);

	return Result;
}

int TI_LMK03318_SetClock(u32 I2CBaseAddress, u8 I2CSlaveAddress,
				int FIn, int FOut, u8 FreeRun)
{
	return XST_FAILURE;
}

/*****************************************************************************/
/**
*
* This function configures the TI LMK03318 to route the specified input port
* to the specified output port bypassing the PLL.
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
* @param InPortID specifies the input port. Valid values are 0 to 1.
* @param OutPortID specifies the output port. Valid values are 4 to 7.
*
* @return
*    - XST_SUCCESS Initialization was successful.
*    - XST_FAILURE I2C write error or incorrect parameters detected.
*
* @note
*
******************************************************************************/
int TI_LMK03318_EnableBypass(u32 I2CBaseAddress, u8 I2CSlaveAddress,
			u8 InPortID, u8 OutPortID)
{
	if (InPortID > 1) {
		print("Invalid input port ID\n\r");
		return XST_FAILURE;
	}

	if (OutPortID < 4 || OutPortID > 7) {
		print("Invalid output port ID\n\r");
		return XST_FAILURE;
	}

	u32 ByteCount = 0;
	u8 Data = 0;
	u8 Buffer[32];
	u8 RegisterAddress = 37 + (OutPortID - 4) * 2; // OUTCTL_x register address

	Buffer[0] = RegisterAddress;
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 1, XIIC_REPEATED_START);
	if (ByteCount != 1) {
		xil_printf("I2C write error: %d\n\r", ByteCount);
		return XST_FAILURE;
	}
	ByteCount = XIic_Recv(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 1, XIIC_STOP);
	if (ByteCount != 1) {
		xil_printf("I2C read error: %d\n\r", ByteCount);
		return XST_FAILURE;
	}
	Data = Buffer[0];
	/* Clear the Clock Source Mux Control field */
	Data &= ~(0x3<<6);

	/* Set the Clock Source Mux Control field */
	Data |= ((InPortID+2)<<6);

	Buffer[0] = RegisterAddress;
	Buffer[1] = Data;
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 2, XIIC_STOP);
	if (ByteCount != 2) {
		xil_printf("I2C write error: %d\n\r", ByteCount);
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function initializes the TI LMK03318 with default values
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
int TI_LMK03318_Init(u32 I2CBaseAddress, u8 I2CSlaveAddress)
{
//	u32 ByteCount = 0;
//	u8 Data = 0;
//	u8 Buffer[32];
	int Result;

	/* Register 29 */
	/* Set input reference clock features */
	Result = TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					 29, 0x0f);

	/* Register 50 */
	/* Set input buffer */
	Result = TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					 50, 0x50);

	/* Register 30 */
	/* Power down register */
	Result = TI_LMK03318_ModifyRegister(I2CBaseAddress, I2CSlaveAddress,
					    30, 0x00, 0x54);

	/* Register 37 */
	/* Set Q4 output */
	Result = TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					 37, 0x92);

	/* Register 41 */
	/* Set Q6 output */
	Result = TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress,
					 41, 0x92);

	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function puts the TI LMK03318 into sleep
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
int TI_LMK03318_PowerDown(u32 I2CBaseAddress, u8 I2CSlaveAddress)
{

	/* Register 29 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 29, 0x8F); //03);
	/* Register 50 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 50, 0x50);

	/* Register 56 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 56, 0x01);


	/* Register 30 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 30, 0x23);//3f);

	/* Register 31 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 31, 0x00);

	/* Register 32 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 32, 0x00);

	/* Register 34 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 34, 0x00);

	/* Register 35 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 35, 0x00);

	/* Register 37 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 37, 0x92);//00);

	/* Register 39 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 39, 0xD2);//00);

	/* Register 41 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 41, 0x92);//00);

	/* Register 43 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 43, 0x00);


	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function displays a registerdump of the TI LMK03318 device.
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return None
*
* @note None.
*
******************************************************************************/
void TI_LMK03318_RegisterDump(u32 I2CBaseAddress, u8 I2CSlaveAddress)
{
	u32 ByteCount = 0;
	u8 Buffer[256];
	int i;

	print("\n\r");
	print("-----------------------\n\r");
	print("- TI LMK03381 I2C dump:\n\r");
	print("-----------------------\n\r");
	Buffer[0] = 0;
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 1, XIIC_REPEATED_START);
	if (ByteCount != 1) {
		xil_printf("I2C write error: %d\n\r", ByteCount);
	}
	ByteCount = XIic_Recv(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 145, XIIC_STOP);
	if (ByteCount != 145) {
		xil_printf("I2C read error: %d\n\r", ByteCount);
	}

	xil_printf("      ");
	for (i = 0 ; i < 10 ; i++)
		xil_printf("+%01d ", i);

	xil_printf("\n\r      ");
	for (i = 0 ; i < 10 ; i++)
		xil_printf("---");

	for (i = 0 ; i < ByteCount ; i++) {
		if ((i % 10) == 0) {
			xil_printf("\n\r%02d : ", i);
		}
		xil_printf("%02x ", Buffer[i]);
	}

	print("\n\r");
}
