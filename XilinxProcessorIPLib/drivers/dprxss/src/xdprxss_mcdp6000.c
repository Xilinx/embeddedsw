/******************************************************************************
*
* Copyright (C) 2017 - 2018 Xilinx, Inc. All rights reserved.
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
* @file mcdp6000.c
* @addtogroup dprxss_v5_0
* @{
*
* This file contains a set of functions to configure the MCDP6000.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- ----------------------------------------------------------
* 1.00 Kei 01/23/18 Initial release.
* </pre>
*
******************************************************************************/


/***************************** Include Files *********************************/
#include "xdprxss_mcdp6000.h"

#include "xiic.h"

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function reads a single 32b word from the MCDP6000 device
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
* @param RegisterAddress is the 16-bit register address.
*
* @return
*    - The read 32b data.
*
* @note None.
*
******************************************************************************/
u32 XDpRxSs_MCDP6000_GetRegister(u32 I2CBaseAddress, u8 I2CSlaveAddress,
		u16 RegisterAddress)
{
	u32 ByteCount = 0;
	u8 Buffer[4];
	u32 Data = 0;
	u8 Retry = 0;
	int i = 0;

	while (1) {

		/* Maximum retries */
		if (Retry == 255) {
			break;
		}

		/* Set Address -> Little Endian */
		Buffer[0] = RegisterAddress & 0xff;
		Buffer[1] = (RegisterAddress >> 8) & 0xff;
		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
				      (u8*)Buffer, 2, XIIC_REPEATED_START);

		if (ByteCount != 2) {
			Retry++;
			continue;
		}

		/* Read data */
		ByteCount = XIic_Recv(I2CBaseAddress, I2CSlaveAddress,
				      (u8*)Buffer, 4, XIIC_STOP);
		if (ByteCount != 4) {
			Retry++;
			continue;
		}

		for(i = 0; i < 4; i++) {
			Data |= (Buffer[i] << (i*8));
		}
		break;
	}

	return Data;
}

/*****************************************************************************/
/**
*
* This function writes a single 32b word to the MCDP6000 device
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
* @param RegisterAddress is the 16-bit register address.
* @param Value is the 32b word to write
*
* @return
*    - XST_SUCCESS Register write was successful.
*    - XST_FAILURE I2C write error.
*
* @note None.
*
******************************************************************************/
int XDpRxSs_MCDP6000_SetRegister(u32 I2CBaseAddress, u8 I2CSlaveAddress,
			 u16 RegisterAddress, u32 Value)
{
	u32 ByteCount = 0;
	u8 Buffer[6];
	u8 Retry = 0;

	/* Set Address -> Little Endian */
	Buffer[0] = RegisterAddress & 0xff;
	Buffer[1] = (RegisterAddress >> 8) & 0xff;

	/* Write data -> Little Endian */
	Buffer[2] = Value & 0xff;
	Buffer[3] = (Value >>  8) & 0xff;
	Buffer[4] = (Value >> 16) & 0xff;
	Buffer[5] = (Value >> 24) & 0xff;

	while (1) {
		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
				      (u8*)Buffer, 6, XIIC_STOP);
		if (ByteCount != 6) {
			Retry++;

			/* Maximum retries */
			if (Retry == 255) {
				return XST_FAILURE;
			}
		}
		else {
			return XST_SUCCESS;
		}
	}
}

/*****************************************************************************/
/**
*
* This function modifies a single 32b word from the MCDP6000 device
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
* @param RegisterAddress is the 16-bit register address.
* @param Value is the 32b word to write
* @param Mask is the 32b mask
*
* @return
*    - XST_SUCCESS Modification was successful.
*    - XST_FAILURE I2C write error.
*
* @note None.
*
******************************************************************************/
int XDpRxSs_MCDP6000_ModifyRegister(u32 I2CBaseAddress, u8 I2CSlaveAddress,
			    u16 RegisterAddress, u32 Value, u32 Mask)
{
	u32 Data;
	int Result;

	/* Read data */
	Data = XDpRxSs_MCDP6000_GetRegister(I2CBaseAddress, I2CSlaveAddress,
				    RegisterAddress);

	/* Clear masked bits */
	Data &= ~Mask;

	/* Update */
	Data |= (Value & Mask);

	/* Write data */
	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      RegisterAddress, Data);

	return Result;
}

/*****************************************************************************/
/**
*
* This function initializes the MCDP6000 device with default values
* for DP use with the Video FMC.
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
int XDpRxSs_MCDP6000_DpInit(u32 I2CBaseAddress, u8 I2CSlaveAddress)
{
	int Result;

	/* According to AppNote DP Retimer Use Case rev 1.0.0, June 6th, 2017
	 * Table 1, pg7; Exit from disabled state to DP 4 lane with normal
	 * plug orientation. The data lanes need to be swapped so use the
	 * inverted plug orientation instead.
	 */
	/*AUX Setting to add latency for data forwarding*/
	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0350, 0x0000001F);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}


	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0504, 0x0000705E);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x278C, 0x00000190);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}


	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x010C, 0x0F0F2D24);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0504, 0x0000715E);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0504, 0x0000705E);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x2614, 0x1A070F0F);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x01A0, 0xCC884444);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x01C0, 0x2C00A81E);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x01D0, 0x0000C360);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0178, 0x13471480);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Setting MC to be fully transparent mode */
	/* Need to set bit10 for Xilinx fixed length mode. */
	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0908, 0x0C00);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0B00, 0x0000);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0B04, 0x0000);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x090C, 0x02020000);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Result;
}

/*****************************************************************************/
/**
*
* This function initializes the MCDP6000 device with default values
* for IBERT use with the Video FMC.
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
int XDpRxSs_MCDP6000_IbertInit(u32 I2CBaseAddress, u8 I2CSlaveAddress)
{
	int Result;

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0504, 0x0000704E);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x278C, 0x00000190);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x010C, 0x0F0F2D24);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x01A0, 0xCC884444);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x01D0, 0x0000B360);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0504, 0x0000714E);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0504, 0x0000704E);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x01C0, 0x2C00A81E);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0178, 0x13471480);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0604, 0x0000F004);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x067C, 0x00000002);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0630, 0x0000041E);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0350, 0x0000004F);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0150, 0x00180000);

	return Result;
}


/*****************************************************************************/
/**
*
* This function requests a reset of the DP path of the MCDP6000 device
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Reset request was successful.
*    - XST_FAILURE I2C error.
*
* @note None.
*
******************************************************************************/
int XDpRxSs_MCDP6000_ResetDpPath(u32 I2CBaseAddress, u8 I2CSlaveAddress)
{
	int Result;


	/* Set Reset bits : This is actually getting out from DP mode.
	 * Eventually reset the chip. */
	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0504, 0x715E);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Clear Reset bits  */
	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0504, 0x705E);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Result;
}

/*****************************************************************************/
/**
*
* This function enables the PRBS7 output of the MCDP6000 device
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Enable request was successful.
*    - XST_FAILURE I2C error.
*
* @note None.
*
******************************************************************************/
int XDpRxSs_MCDP6000_EnablePrbs7_Tx(u32 I2CBaseAddress, u8 I2CSlaveAddress)
{
	int Result;

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x024C, 0xAAAA9A05);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0150, 0x00100000);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0604, 0x0000F004);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0630, 0x0000041E);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0668, 0x00000001);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0674, 0x00000001);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0680, 0x00070000);

	return Result;
}

/*****************************************************************************/
/**
*
* This function enables the PRBS7 counter mode in MC Rx path
* Used in DP PHY compliance mode
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Enable request was successful.
*    - XST_FAILURE I2C error.
*
* @note None.
*
******************************************************************************/
int XDpRxSs_MCDP6000_EnablePrbs7_Rx(u32 I2CBaseAddress, u8 I2CSlaveAddress)
{
	int Result;
	unsigned int ReadVal;

	/* Enable PRBS Mode */
	ReadVal = XDpRxSs_MCDP6000_GetRegister(I2CBaseAddress, I2CSlaveAddress,
				       0x0614);
	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0614, (ReadVal | 0x800));
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Result;
}

/*****************************************************************************/
/**
*
* This function Disables the PRBS7 counter mode in MC Rx path
* Used in DP PHY compliance mode
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Enable request was successful.
*    - XST_FAILURE I2C error.
*
* @note None.
*
******************************************************************************/
int XDpRxSs_MCDP6000_DisablePrbs7_Rx(u32 I2CBaseAddress, u8 I2CSlaveAddress)
{
	int Result;
	unsigned int ReadVal;

	/* Disable PRBS Mode */
	ReadVal = XDpRxSs_MCDP6000_GetRegister(I2CBaseAddress, I2CSlaveAddress,
				       0x0614);
	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0614, (ReadVal & ~0xFFFFF7FF));
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Result;
}

/*****************************************************************************/
/**
*
* This function enables symbol counter
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Enable request was successful.
*    - XST_FAILURE I2C error.
*
* @note None.
*
******************************************************************************/
int XDpRxSs_MCDP6000_EnableCounter(u32 I2CBaseAddress, u8 I2CSlaveAddress)
{
	int Result;
	unsigned int ReadVal;

	/* Enable Symbol Counter Always*/
	ReadVal = XDpRxSs_MCDP6000_GetRegister(I2CBaseAddress, I2CSlaveAddress,
				       0x061c);
	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x061c, (ReadVal | 0x1));
	return Result;
}

/*****************************************************************************/
/**
*
* This function clears symbol counter
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Enable request was successful.
*    - XST_FAILURE I2C error.
*
* @note None.
*
******************************************************************************/
int XDpRxSs_MCDP6000_ClearCounter(u32 I2CBaseAddress, u8 I2CSlaveAddress)
{
	int Result;
	unsigned int ReadVal;

	/* Enable Symbol Counter Always*/
	ReadVal = XDpRxSs_MCDP6000_GetRegister(I2CBaseAddress, I2CSlaveAddress,
				       0x061c);
	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x061c, (ReadVal & 0xFFFFFFFE));
	return Result;
}

/*****************************************************************************/
/**
*
* This function reads error counters for all lanes
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Enable request was successful.
*    - XST_FAILURE I2C error.
*
* @note None.
*
******************************************************************************/
int XDpRxSs_MCDP6000_Read_ErrorCounters(u32 I2CBaseAddress, u8 I2CSlaveAddress)
{
	int Result;
	unsigned int ReadVal;

	/* Set lane count to 0 in [2:1] */
	ReadVal = XDpRxSs_MCDP6000_GetRegister(I2CBaseAddress, I2CSlaveAddress,
				       0x061C);
	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x061C, (ReadVal & 0xFFFFFFF9));
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Read error counter */
	ReadVal = XDpRxSs_MCDP6000_GetRegister(I2CBaseAddress, I2CSlaveAddress,
				       0x0620);
	xil_printf("MCDP Error Counter (Lane0): %0x%x \n\r",ReadVal);

	/* Set lane count to 1 in [2:1] */
	ReadVal = XDpRxSs_MCDP6000_GetRegister(I2CBaseAddress, I2CSlaveAddress,
				       0x061C);
	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x061C, (ReadVal | 0x2));
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Read error counter */
	ReadVal = XDpRxSs_MCDP6000_GetRegister(I2CBaseAddress, I2CSlaveAddress,
				       0x0620);
	xil_printf("MCDP Error Counter (Lane1): %0x%x \n\r",ReadVal);

	/* Set lane count to 2 in [2:1] */
	ReadVal = XDpRxSs_MCDP6000_GetRegister(I2CBaseAddress, I2CSlaveAddress,
				       0x061C);
	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x061C, ReadVal | 0x4);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Read error counter */
	ReadVal = XDpRxSs_MCDP6000_GetRegister(I2CBaseAddress, I2CSlaveAddress,
				       0x0620);
	xil_printf("MCDP Error Counter (Lane2): %0x%x \n\r",ReadVal);

	/* Set lane count to 3 in [2:1] */
	ReadVal = XDpRxSs_MCDP6000_GetRegister(I2CBaseAddress, I2CSlaveAddress,
				        0x061C);
	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x061C, ReadVal | 0x6);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Read error counter */
	ReadVal = XDpRxSs_MCDP6000_GetRegister(I2CBaseAddress, I2CSlaveAddress,
				       0x0620);
	xil_printf("MCDP Error Counter (Lane3): %0x%x \n\r",ReadVal);

	return Result;
}

/*****************************************************************************/
/**
*
* This function displays a registerdump of the MCDP6000 device.
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return None
*
* @note None.
*
******************************************************************************/
void XDpRxSs_MCDP6000_RegisterDump(u32 I2CBaseAddress, u8 I2CSlaveAddress)
{
	u32 Data;
	u32 i;

	xil_printf("\n\r");
	xil_printf("---------------------\n\r");
	xil_printf("- MCDP6000 I2C dump:\n\r");
	xil_printf("---------------------\n\r");

	for (i = 0x200; i < 0xC00; i+=4) {
		if ((i % 4) == 0) {
			xil_printf("\n\r%04x : ", i);
		}
		Data = XDpRxSs_MCDP6000_GetRegister(I2CBaseAddress, I2CSlaveAddress, i);
		xil_printf("%08x ", Data);
	}

	for (i = 0x1200; i < 0x1600; i+=4) {
		if ((i % 4) == 0) {
			xil_printf("\n\r%04x : ", i);
		}
		Data = XDpRxSs_MCDP6000_GetRegister(I2CBaseAddress, I2CSlaveAddress, i);
		xil_printf("%08x ", Data);
	}

	for (i = 0x2200; i < 0x2600; i+=4) {
		if ((i % 4) == 0) {
			xil_printf("\n\r%04x : ", i);
		}
		Data = XDpRxSs_MCDP6000_GetRegister(I2CBaseAddress, I2CSlaveAddress, i);
		xil_printf("%08x ", Data);
	}

	for (i = 0x2700; i < 0x2800; i+=4) {
		if ((i % 4) == 0) {
			xil_printf("\n\r%04x : ", i);
		}
		Data = XDpRxSs_MCDP6000_GetRegister(I2CBaseAddress, I2CSlaveAddress, i);
		xil_printf("%08x ", Data);
	}
	xil_printf("\n\r");
}

/*****************************************************************************/
/**
*
* This function sets the transparent mode of the MCDP6000 device.
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS transparent mode was set.
*    - XST_FAILURE setting transparent mode failed.
*
* @note None.
*
******************************************************************************/
int XDpRxSs_MCDP6000_TransparentMode(u32 I2CBaseAddress, u8 I2CSlaveAddress){
	int Result;

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				       0x0908, 0x00000800);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				       0x090C, 0x00020000);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Result;
}

/*****************************************************************************/
/**
*
* This function changes the bandwidth of the MCDP6000 device.
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS BW change successful.
*    - XST_FAILURE BW change failed.
*
* @note None.
*
******************************************************************************/
int XDpRxSs_MCDP6000_BWchange(u32 I2CBaseAddress, u8 I2CSlaveAddress)
{
	int Result;

	/* Set Reset bits (bits 10) */
	Result = XDpRxSs_MCDP6000_ModifyRegister(I2CBaseAddress, I2CSlaveAddress,
					 0x0504, 0x400, 0x400);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Clear Reset bits (bits 10) */
	Result = XDpRxSs_MCDP6000_ModifyRegister(I2CBaseAddress, I2CSlaveAddress,
					 0x0504, 0x000, 0x400);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Result;
}

/*****************************************************************************/
/**
*
* This function sets the access lane register of the MCDP6000 device.
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS access lane set successful.
*    - XST_FAILURE setting access lane failed.
*
* @note None.
*
******************************************************************************/
int XDpRxSs_MCDP6000_AccessLaneSet(u32 I2CBaseAddress, u8 I2CSlaveAddress)
{
	int Result;

	/* Set Reset bits (bits 0) */
	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0150, 0x01);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Clear Reset bits (bits 0) */
	Result = XDpRxSs_MCDP6000_SetRegister(I2CBaseAddress, I2CSlaveAddress,
				      0x0150, 0x00);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Result;
}
/** @} */
