/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*
	TMDS181

	Written by Marco Groeneveld

	History
	-------
	v1.0 - Initial release
*/

#include "tmds181.h"
#include "sleep.h"
#include "xiic.h"

#define TMDS181_VERBOSE			0
#define TMDS181_ZOMBIE 			0
#define TMDS181_ES				1

#define I2C_TMDS181_ZOMBIE_ADDR 	0x5C
#define I2C_TMDS181_ES_ADDR 		0x5C

// I2C TMDS181 check
// This routine checks if any data can be read from the TMDS181
u8 i2c_tmds181_chk(u8 dev) {
	u32 r;
	u8 buf[1];

	// TMDS181 ES
	if (dev == TMDS181_ES) {
		r = XIic_Recv(XPAR_IIC_0_BASEADDR, I2C_TMDS181_ES_ADDR, (u8 *)&buf, 1, XIIC_STOP);
	}
	else
		r = XIic_Recv(XPAR_IIC_0_BASEADDR, I2C_TMDS181_ZOMBIE_ADDR, (u8 *)&buf, 1, XIIC_STOP);

	// When a device is found, it returns one byte
	if (r == 1)
	  return XST_SUCCESS;
	else
	  return XST_FAILURE;
}

// I2C TMDS181 write
u32 i2c_tmds181_write(u8 dev, u8 addr, u8 dat)
{
  u32 r;
  u8 buf[2];

  buf[0] = addr;
  buf[1] = dat;

  // TMDS181 ES
  if (dev == TMDS181_ES) {
	  r = XIic_Send(XPAR_IIC_0_BASEADDR, I2C_TMDS181_ES_ADDR, (u8 *)&buf, 2, XIIC_STOP);
  }

  // Zombie
  else {
	  r = XIic_Send(XPAR_IIC_0_BASEADDR, I2C_TMDS181_ZOMBIE_ADDR, (u8 *)&buf, 2, XIIC_STOP);
  }

  if (r == 2)
	  return XST_SUCCESS;
  else
	  return XST_FAILURE;
}

// I2C TMDS181 read
u8 i2c_tmds181_read(u8 dev, u8 addr)
{
  u32 r;
  u8 buf[2];

  buf[0] = addr;

  // TMDS181 ES
  if (dev == TMDS181_ES) {
	  r = XIic_Send(XPAR_IIC_0_BASEADDR, I2C_TMDS181_ES_ADDR, (u8 *)&buf, 1, XII_REPEATED_START_OPTION);
	  r = XIic_Recv(XPAR_IIC_0_BASEADDR, I2C_TMDS181_ES_ADDR, (u8 *)&buf, 1, XIIC_STOP);
  }

  // Zombie
  else {
	  r = XIic_Send(XPAR_IIC_0_BASEADDR, I2C_TMDS181_ZOMBIE_ADDR, (u8 *)&buf, 1, XII_REPEATED_START_OPTION);
	  r = XIic_Recv(XPAR_IIC_0_BASEADDR, I2C_TMDS181_ZOMBIE_ADDR, (u8 *)&buf, 1, XIIC_STOP);
  }

  if (r == 1)
	return buf[0];
  else
	return 0;
}

// I2C TMDS181 dump
// This routine dumps the TMDS181 ES registers
void i2c_tmds181_dump(void)
{
  u32 r;
  u8 i;
  u8 buf[32];

  buf[0] = 0x0;
  xil_printf("TMDS181 register dump\r\n");
  r = XIic_Send(XPAR_IIC_0_BASEADDR, I2C_TMDS181_ES_ADDR, (u8 *)&buf, 1, XII_REPEATED_START_OPTION);

  r = XIic_Recv(XPAR_IIC_0_BASEADDR, I2C_TMDS181_ES_ADDR, (u8 *)&buf, 32, XIIC_STOP);
  for (i = 0; i<= 0x20; i++) {
	  xil_printf("(%d) ADDR: %0x DATA: %0x\r\n", r, i, buf[i]);
  }
 }
