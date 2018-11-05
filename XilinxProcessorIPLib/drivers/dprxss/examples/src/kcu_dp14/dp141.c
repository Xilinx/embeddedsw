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
* @file dp141.c
*
* This file contains dp141 related functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  Kei    08/12/17 Initial release.
* </pre>
*
******************************************************************************/

#include "xbasic_types.h"
#include "xiic.h"
#include "xiic_l.h"

#include "xparameters.h"
#include "videofmc_defs.h"

#define ch0 0x02
#define ch1 0x05
#define ch2 0x08
#define ch3 0x0B

#define TX_GAIN 	0x08
#define EQ_DC_GAIN 	0x04
#define RX_GAIN_1	0x02
#define RX_GAIN_0	0x01

u8 i2c_read_dp141(u32 I2CBaseAddress, u8 I2CSlaveAddress, u16 RegisterAddress)
{
	u32 ByteCount = 0;
	u8 Buffer[1];
	u8 Data;
	u8 Retry = 0;
	u8 Exit;


	Exit = FALSE;
	Data = 0;

	do {
		/* Set Address */
		Buffer[0] = RegisterAddress & 0xff;
		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
				(u8*)Buffer, 1, XIIC_REPEATED_START);

		if (ByteCount != 1) {
			Retry++;

			/* Maximum retries */
			if (Retry == 255) {
				Exit = TRUE;
			}
		} else {
			/* Read data */
			ByteCount = XIic_Recv(I2CBaseAddress, I2CSlaveAddress,
					(u8*)Buffer, 1, XIIC_STOP);
				Data = Buffer[0];
				Exit = TRUE;
		}
	} while (!Exit);

	return Data;
}


int i2c_write_dp141(u32 I2CBaseAddress, u8 I2CSlaveAddress, 
		    u16 RegisterAddress, u8 Value)
{
	u32 ByteCount = 0;
	u8 Buffer[2];
	u8 Retry = 0;

	/* Write data */
	Buffer[0] = RegisterAddress & 0xff;
	Buffer[1] = Value;

	while (1) {
		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
				(u8*)Buffer, 3, XIIC_STOP);

		if (ByteCount != 2) {
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

void read_DP141()
{
	u8 Data;
	int i =0;

	for(i=0; i<0xD; i++){
		Data = i2c_read_dp141( XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, i);
		xil_printf("%x : %02x \r\n",i, Data);
	}

}

void DP141_init(u32 I2CBaseAddress, u8 I2CSlaveAddress)
{
	u8 eq_level = 7;
	u8 write_data = TX_GAIN | (eq_level << 4);
	i2c_write_dp141(I2CBaseAddress, I2CSlaveAddress, ch0, write_data);
	i2c_write_dp141(I2CBaseAddress, I2CSlaveAddress, ch1, write_data);
	i2c_write_dp141(I2CBaseAddress, I2CSlaveAddress, ch2, write_data);
	i2c_write_dp141(I2CBaseAddress, I2CSlaveAddress, ch3, write_data);
}
