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
* @file si_5344.c
*
* This file contains Si5344 related functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  MG     07/27/16 Initial release.
* </pre>
*
******************************************************************************/

#include "si_5344.h"
#include "si_5344_freqconfigs.h"
#include "xiic.h"

int SI_5344_SetFrequencyConfig(u32 I2CBaseAddress, u8 I2CSlaveAddress,
				u8 Freerun, u8 ConfigSelect)
{
//	u32 ByteCount = 0;
//	u16 RegisterAddress = 0;
//	u8 Value = 0;
//	u8 Buffer[2];
//	int Index = 0;
//	int NumRegs = 0;
//
//	SI_5344_FreqConfig* pFreqConfig;
//	xil_printf("SI: Set Config %d\n\r", ConfigSelect);
//	switch(ConfigSelect) {
//		case 0 :
//			pFreqConfig = &SI_5344_Locked_384x32k[0];
//			NumRegs = SI_5344_FREQCONFIG_LOCKED_NUM_REGS;
//		break;
//
//		case 1 :
//			pFreqConfig = &SI_5344_Locked_384x44k1[0];
//			NumRegs = SI_5344_FREQCONFIG_LOCKED_NUM_REGS;
//		break;
//
//		case 2 :
//			pFreqConfig = &SI_5344_Locked_384x48k[0];
//			NumRegs = SI_5344_FREQCONFIG_LOCKED_NUM_REGS;
//		break;
//
//		case 3 :
//			pFreqConfig = &SI_5344_Locked_192x88k2[0];
//			NumRegs = SI_5344_FREQCONFIG_LOCKED_NUM_REGS;
//		break;
//
//		case 4 :
//			pFreqConfig = &SI_5344_Locked_192x96k[0];
//			NumRegs = SI_5344_FREQCONFIG_LOCKED_NUM_REGS;
//		break;
//
//		case 6 :
//			pFreqConfig = &SI_5344_Locked_192x192k[0];
//			NumRegs = SI_5344_FREQCONFIG_LOCKED_NUM_REGS;
//		break;
//
//		case 7 :
//			pFreqConfig = &SI_5344_Locked_148M5_512x48k[0];
//			NumRegs = SI_5344_FREQCONFIG_LOCKED_NUM_REGS;
//		break;
//
//
//		default :
//			return XST_FAILURE;
//	}
//
//	for (Index = 0; Index < NumRegs; Index++) {
//		RegisterAddress = pFreqConfig->Address;
//		Value = pFreqConfig->Value;
//
//		Buffer[0] = 0x01; // Set page address
//		Buffer[1] = (RegisterAddress >> 8) & 0xFF;
//
//		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 2, XIIC_STOP);
//		if (ByteCount != 2)
//			return XST_FAILURE;
//
//		Buffer[0] = RegisterAddress & 0xFF;
//		Buffer[1] = Value;
//		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 2, XIIC_STOP);
//		if (ByteCount != 2)
//			return XST_FAILURE;
//
//		pFreqConfig++;
//	}
//
//
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets a register in the SI 5344  
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
int SI_5344_SetRegister(u32 I2CBaseAddress, u8 I2CSlaveAddress,
			u16 RegisterAddress, u8 Value)
{
	u32 ByteCount = 0;
	u8 Buffer[2];
	
	Buffer[0] = 0x01;	// Set page address
	Buffer[1] = (RegisterAddress >> 8) & 0xFF; 

	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 2, XIIC_STOP);

	if (ByteCount != 2) 
		return XST_FAILURE;

	Buffer[0] = RegisterAddress & 0xFF; 
	Buffer[1] = Value; 
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 2, XIIC_STOP);
	
	if (ByteCount != 2) 
		return XST_FAILURE;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function puts the SI 5344 into power down 
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
int SI_5344_PowerDown(u32 I2CBaseAddress, u8 I2CSlaveAddress)
{
	/* Register 1e */
	SI_5344_SetRegister(I2CBaseAddress, I2CSlaveAddress, 0x1e, 0x01);

	/* Register 112 */
	SI_5344_SetRegister(I2CBaseAddress, I2CSlaveAddress, 0x112, 0x01);

	/* Register 117 */
	SI_5344_SetRegister(I2CBaseAddress, I2CSlaveAddress, 0x117, 0x01);

	/* Register 126 */
	SI_5344_SetRegister(I2CBaseAddress, I2CSlaveAddress, 0x126, 0x01);

	/* Register 12b */
	SI_5344_SetRegister(I2CBaseAddress, I2CSlaveAddress, 0x12b, 0x01);

	/* Register 145 */
	SI_5344_SetRegister(I2CBaseAddress, I2CSlaveAddress, 0x145, 0x01);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function displays a registerdump of the SI 5344 device.
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return None
*
* @note None.
*
******************************************************************************/
void SI_5344_RegisterDump(u32 I2CBaseAddress, u8 I2CSlaveAddress)
{
	u32 ByteCount = 0;
	u8 Buffer[256];
	int i;
	int page;

	print("\n\r");
	print("-----------------------\n\r");
	print("- SI 5344 I2C dump:\n\r");
	print("-----------------------\n\r");
	
	for (page=0; page<3; page++) {
		xil_printf("\n\rpage %d\n\r", page);

		/* Page */
		Buffer[0] = 1;
		Buffer[1] = page;
		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
				      (u8*)Buffer, 2, XIIC_STOP);
		if (ByteCount != 2) {
			xil_printf("I2C write error: %d\n\r", ByteCount);
		}

		/* Register address */
		Buffer[0] = 0;
		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
				      (u8*)Buffer, 1, XIIC_REPEATED_START);
		if (ByteCount != 1) {
			xil_printf("I2C write error: %d\n\r", ByteCount);
		}

		/* Read page */
		ByteCount = XIic_Recv(I2CBaseAddress, I2CSlaveAddress,
				      (u8*)Buffer, 256, XIIC_STOP);
		if (ByteCount != 256) {
			xil_printf("I2C read error: %d\n\r", ByteCount);
		}

		xil_printf("      ");
		for (i=0; i<16; i++)
			xil_printf("+%01x ", i);

		xil_printf("\n\r      ");
		for (i=0; i<16; i++)
			xil_printf("---");
		
		for (i = 0; i < ByteCount; i++) {
			if ((i % 16) == 0) {
				xil_printf("\n\r%01x%02x : ", page, i);
			}
			xil_printf("%02x ", Buffer[i]);
		}
	}

	print("\n\r");
}
