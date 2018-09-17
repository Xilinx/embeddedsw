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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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



/*
	// Issue a Software Reset.
	Buffer[0] = 12; // DEV_CTL register address
	Buffer[1] = 0x59; // R12 -> RESETN_SW = 0, the rest is default.
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 2, XIIC_STOP);
	if (ByteCount != 2) {
		return XST_FAILURE;
	}

	// Release Software Reset.
	Buffer[0] = 12; // DEV_CTL register address
	Buffer[1] = 0xD9; // R12 -> RESETN_SW = 1, the rest is default.
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 2, XIIC_STOP);
	if (ByteCount != 2) {
		return XST_FAILURE;
	}

	// Set the default output modes
	// CK_OUT0/1 are not connected on the VFMC, so disable these outputs.
	// CK_OUT2/3 are configured as AC LVDS.
	// CK_OUT4/5/6/7 are configured as AC LVPECL.
	Buffer[0] = 31; // OUTCTL_0 register address
	Buffer[1] = 0x00; // R31 -> CK_OUT0 disabled
	Buffer[2] = 0x00; // R32 -> CK_OUT1 disabled
	Buffer[3] = 0x01; // R33 -> CK_OUT0/1 DIV Default
	Buffer[4] = 0x20; // R34 -> CK_OUT2 AC-LVDS 4mA
	Buffer[5] = 0x20; // R35 -> CK_OUT3 AC-LVDS 4mA
	Buffer[6] = 0x03; // R36 -> CK_OUT2/3 DIV Default
	Buffer[7] = 0x18; // R37 -> CK_OUT4 AC-LVPECL 8mA, source is PLL
	Buffer[8] = 0x02; // R38 -> CK_OUT4 DIV Default
	Buffer[9] = 0x18; // R39 -> CK_OUT5 AC-LVPECL 8mA, source is PLL
	Buffer[10] = 0x02; // R40 -> CK_OUT5 DIV Default
	Buffer[11] = 0x18; // R41 -> CK_OUT6 AC-LVPECL 8mA, source is PLL
	Buffer[12] = 0x05; // R42 -> CK_OUT6 DIV Default
	Buffer[13] = 0x18; // R43 -> CK_OUT7 AC-LVPECL 8mA, source is PLL
	Buffer[14] = 0x05; // R44 -> CK_OUT7 DIV Default
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 15, XIIC_STOP);
	if (ByteCount != 15) {
		return XST_FAILURE;
	}

	// Set the default input modes
	Buffer[0] = 29; // OSCCTL1 register address
	Buffer[1] = 0x0f; // R29 -> PRI/SEC internal 100-Ohm termination, PRI/SEC external AC coupling
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 2, XIIC_STOP);
	if (ByteCount != 2) {
		return XST_FAILURE;
	}

	// PRIREF and SECREF are configured as differential input.
	Buffer[0] = 50; // IPCLKSEL register address
	Buffer[1] = 0x50; // R50 -> PRIREF/SECREF Dif input, input mode -> automatic
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 2, XIIC_STOP);
	if (ByteCount != 2) {
		return XST_FAILURE;
	}

	// Disable the Secondary Reference Doubler
	Buffer[0] = 72; // SEC_CTRL register address
	Buffer[1] = 0x08; // R72 -> Secondary Reference Doubler disabled.
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 2, XIIC_STOP);
	if (ByteCount != 2) {
		return XST_FAILURE;
	}
*/
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
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 29, 0x03);

	/* Register 30 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 30, 0x3f);

	/* Register 31 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 31, 0x00);

	/* Register 32 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 32, 0x00);

	/* Register 34 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 34, 0x00);

	/* Register 35 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 35, 0x00);

	/* Register 37 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 37, 0x00);

	/* Register 39 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 39, 0x00);

	/* Register 41 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 41, 0x00);

	/* Register 43 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 43, 0x00);

	/* Register 50 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 50, 0xf6);

	/* Register 56 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 56, 0x01);

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
