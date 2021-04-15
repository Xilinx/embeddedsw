/******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
