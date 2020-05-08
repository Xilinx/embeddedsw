/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*
	DP159

	Written by Marco Groeneveld

	History
	-------
	v1.0 - Initial release
	v1.1 - Updated DP159 setting to automatic redriver to retimer
	       for HDMI 1.4 data rates
	v1.2 - Added Reset to I2C controller before issuing new transaction
	v1.3 - Changed printf usage to xil_printf
	v1.4 - Update vswing setting to recommened values to pass compliance
	v1.5 - Update the register setting sequence to write 0x0A the last
           to set APPLY_RXTX_CHANGES
*/

#include "dp159.h"
#include "sleep.h"
#include "xiic.h"

#define DP159_VERBOSE			0
#define DP159_ZOMBIE 			0
#define DP159_ES				1

#define I2C_DP159_ZOMBIE_ADDR 	0x2C
#define I2C_DP159_ES_ADDR 		0x5E

// I2C DP159 check
// This routine checks if any data can be read from the DP159
u8 i2c_dp159_chk(u8 dev) {
	u32 r;
	u8 buf[1];

	// DP159 ES
	if (dev == DP159_ES) {
		r = XIic_Recv(XPAR_IIC_0_BASEADDR, I2C_DP159_ES_ADDR, (u8 *)buf, 1, XIIC_STOP);
	}
	else
		r = XIic_Recv(XPAR_IIC_0_BASEADDR, I2C_DP159_ZOMBIE_ADDR, (u8 *)buf, 1, XIIC_STOP);

	// When a device is found, it returns one byte
	if (r == 1)
	  return XST_SUCCESS;
	else
	  return XST_FAILURE;
}

// I2C DP159 write
u32 i2c_dp159_write(u8 dev, u8 addr, u8 dat)
{
  u32 r;
  u8 buf[2];

  buf[0] = addr;
  buf[1] = dat;

  // DP159 ES
  if (dev == DP159_ES) {
	  r = XIic_Send(XPAR_IIC_0_BASEADDR, I2C_DP159_ES_ADDR, (u8 *)buf, 2, XIIC_STOP);
  }

  // Zombie
  else {
	  r = XIic_Send(XPAR_IIC_0_BASEADDR, I2C_DP159_ZOMBIE_ADDR, (u8 *)buf, 2, XIIC_STOP);
  }

  if (r == 2)
	  return XST_SUCCESS;
  else
	  return XST_FAILURE;
}

// I2C DP159 read
u8 i2c_dp159_read(u8 dev, u8 addr)
{
  u32 r;
  u8 buf[2];

  buf[0] = addr;

  // DP159 ES
  if (dev == DP159_ES) {
	  r = XIic_Send(XPAR_IIC_0_BASEADDR, I2C_DP159_ES_ADDR, (u8 *)buf, 1, XII_REPEATED_START_OPTION);
	  r = XIic_Recv(XPAR_IIC_0_BASEADDR, I2C_DP159_ES_ADDR, (u8 *)buf, 1, XIIC_STOP);
  }

  // Zombie
  else {
	  r = XIic_Send(XPAR_IIC_0_BASEADDR, I2C_DP159_ZOMBIE_ADDR, (u8 *)buf, 1, XII_REPEATED_START_OPTION);
	  r = XIic_Recv(XPAR_IIC_0_BASEADDR, I2C_DP159_ZOMBIE_ADDR, (u8 *)buf, 1, XIIC_STOP);
  }

  if (r == 1)
	return buf[0];
  else
	return 0;
}

// I2C DP159 dump
// This routine dumps the DP159 ES registers
void i2c_dp159_dump(void)
{
  u32 r;
  u8 i;
  u8 buf[32];

  buf[0] = 0x0;
  xil_printf("DP159 register dump\r\n");
  r = XIic_Send(XPAR_IIC_0_BASEADDR, I2C_DP159_ES_ADDR, (u8 *)buf, 1, XII_REPEATED_START_OPTION);

  r = XIic_Recv(XPAR_IIC_0_BASEADDR, I2C_DP159_ES_ADDR, (u8 *)buf, 32, XIIC_STOP);
  for (i = 0; i< 0x20; i++) {
	  xil_printf("(%d) ADDR: %0x DATA: %0x\r\n", r, i, buf[i]);
  }
 }

// DP159
#ifndef versal
u32 i2c_dp159(XVphy *VphyPtr, u8 QuadId, u64 TxLineRate)
#else
u32 i2c_dp159(XHdmiphy1 *Hdmiphy1Ptr, u8 QuadId, u64 TxLineRate)
#endif
{
  u32 r;
  u8 mode;

  // Reset I2C controller before issuing new transaction. This is required to
  // recover the IIC controller in case a previous transaction is pending.
  XIic_WriteReg(XPAR_IIC_0_BASEADDR, XIIC_RESETR_OFFSET,
			XIIC_RESET_MASK);

  // Select mode
  // HDMI 2.0
  if ((TxLineRate / (1000000)) > 3400)
	  mode = 2;

  // HDMI 1.4 > 1.2 Gbps
  else if ((TxLineRate / (1000000)) > 1200)
	  mode = 1;

  // HDMI 1.4 < 1.2 Gbps
  else
	  mode = 0;

  // Check if the DP159 is a zombie device
  r = i2c_dp159_chk(DP159_ZOMBIE);

  if (r == XST_SUCCESS) {
	  if (DP159_VERBOSE)
		  xil_printf("Program DP159 ZOMBIE... \r\n");
	switch(mode) {
		case 0 : // HDMI 1.4 (250Mbps - 1.2Gbps)
			// Select page 1
			r = i2c_dp159_write(DP159_ZOMBIE, 0xff, 0x01);

			// PLL_FBDIV is 280
			r = i2c_dp159_write(DP159_ZOMBIE, 0x04, 0x80);
			r = i2c_dp159_write(DP159_ZOMBIE, 0x05, 0x02);

			// PLL_PREDIV is 2
			r = i2c_dp159_write(DP159_ZOMBIE, 0x08, 0x02);

			// CDR_CONFIG[4:0]
			r = i2c_dp159_write(DP159_ZOMBIE, 0x0e, 0x10);

			// CP_CURRENT
			r = i2c_dp159_write(DP159_ZOMBIE, 0x01, 0x81);
			usleep(10000);

			// Enable Bandgap
			r = i2c_dp159_write(DP159_ZOMBIE, 0x00, 0x02);
			usleep(10000);

			// Enable PLL
			r = i2c_dp159_write(DP159_ZOMBIE, 0x00, 0x03);

			// Enable TX
			r = i2c_dp159_write(DP159_ZOMBIE, 0x10, 0x0f);

			// HDMI_TWPST1
			r = i2c_dp159_write(DP159_ZOMBIE, 0x14, 0x10);

			// DP_TWPST1
			r = i2c_dp159_write(DP159_ZOMBIE, 0x16, 0x10);

			// DP_TWPST2
			r = i2c_dp159_write(DP159_ZOMBIE, 0x17, 0x00);

			// Slew CTRL
			r = i2c_dp159_write(DP159_ZOMBIE, 0x12, 0x28);

			// FIR_UPD
			r = i2c_dp159_write(DP159_ZOMBIE, 0x13, 0x0f);
			r = i2c_dp159_write(DP159_ZOMBIE, 0x13, 0x00);

			// TX_RATE
			r = i2c_dp159_write(DP159_ZOMBIE, 0x11, 0xC0);

			// Enable receivers
			r = i2c_dp159_write(DP159_ZOMBIE, 0x30, 0x0f);

			// PD_RXINT
			r = i2c_dp159_write(DP159_ZOMBIE, 0x32, 0x00);

			// RX_RATE
			r = i2c_dp159_write(DP159_ZOMBIE, 0x31, 0xC0);

			// Disable offset correction
			r = i2c_dp159_write(DP159_ZOMBIE, 0x34, 0x00);

			// Change default of CDR_STL
			r = i2c_dp159_write(DP159_ZOMBIE, 0x3c, 0x04);

			// Change default of CDR_SO_TR
			r = i2c_dp159_write(DP159_ZOMBIE, 0x3D, 0x06);

			// EQFTC
			r = i2c_dp159_write(DP159_ZOMBIE, 0x4D, 0x38);

			// Enable Adaptive EQ
			r = i2c_dp159_write(DP159_ZOMBIE, 0x4c, 0x03);

			// Select page 0
			r = i2c_dp159_write(DP159_ZOMBIE, 0xff, 0x00);

			// Gate HPD_SNK
			r = i2c_dp159_write(DP159_ZOMBIE, 0x09, 0x01);

			// Set GPIO
			r = i2c_dp159_write(DP159_ZOMBIE, 0xe0, 0x01);

			// Un gate HPD_SNK
			r = i2c_dp159_write(DP159_ZOMBIE, 0x09, 0x00);
			return XST_SUCCESS;
			break;

		case 1 : // HDMI 1.4 (1.2Gbps - 3Gbps)
			// Select page 1
			r = i2c_dp159_write(DP159_ZOMBIE, 0xff, 0x01);

			// PLL_FBDIV is 140
			r = i2c_dp159_write(DP159_ZOMBIE, 0x04, 0x40);
			r = i2c_dp159_write(DP159_ZOMBIE, 0x05, 0x01);

			// PLL_PREDIV is 4
			r = i2c_dp159_write(DP159_ZOMBIE, 0x08, 0x04);

			// CDR_CONFIG[4:0]
			r = i2c_dp159_write(DP159_ZOMBIE, 0x0e, 0x10);

			// CP_CURRENT
			r = i2c_dp159_write(DP159_ZOMBIE, 0x01, 0x81);
			usleep(10000);
			// Enable Bandgap
			r = i2c_dp159_write(DP159_ZOMBIE, 0x00, 0x02);
			usleep(10000);

			// Enable PLL
			r = i2c_dp159_write(DP159_ZOMBIE, 0x00, 0x03);

			// Enable TX
			r = i2c_dp159_write(DP159_ZOMBIE, 0x10, 0x0f);

			// HDMI_TWPST1
			r = i2c_dp159_write(DP159_ZOMBIE, 0x14, 0x10);

			// DP_TWPST1
			r = i2c_dp159_write(DP159_ZOMBIE, 0x16, 0x10);

			// DP_TWPST2
			r = i2c_dp159_write(DP159_ZOMBIE, 0x17, 0x00);

			// Slew CTRL
			r = i2c_dp159_write(DP159_ZOMBIE, 0x12, 0x28);

			// FIR_UPD
			r = i2c_dp159_write(DP159_ZOMBIE, 0x13, 0x0f);
			r = i2c_dp159_write(DP159_ZOMBIE, 0x13, 0x00);

			// TX_RATE
			r = i2c_dp159_write(DP159_ZOMBIE, 0x11, 0x70);

			// Enable receivers
			r = i2c_dp159_write(DP159_ZOMBIE, 0x30, 0x0f);

			// PD_RXINT
			r = i2c_dp159_write(DP159_ZOMBIE, 0x32, 0x00);

			// RX_RATE
			r = i2c_dp159_write(DP159_ZOMBIE, 0x31, 0x40);

			// Disable offset correction
			r = i2c_dp159_write(DP159_ZOMBIE, 0x34, 0x00);

			// Change default of CDR_STL
			r = i2c_dp159_write(DP159_ZOMBIE, 0x3c, 0x04);

			// Change default of CDR_SO_TR
			r = i2c_dp159_write(DP159_ZOMBIE, 0x3D, 0x06);

			// EQFTC
			r = i2c_dp159_write(DP159_ZOMBIE, 0x4D, 0x28);

			// Enable Adaptive EQ
			r = i2c_dp159_write(DP159_ZOMBIE, 0x4c, 0x03);

			// Select page 0
			r = i2c_dp159_write(DP159_ZOMBIE, 0xff, 0x00);

			// Gate HPD_SNK
			r = i2c_dp159_write(DP159_ZOMBIE, 0x09, 0x01);

			// Set GPIO
			r = i2c_dp159_write(DP159_ZOMBIE, 0xe0, 0x01);

			// Un gate HPD_SNK
			r = i2c_dp159_write(DP159_ZOMBIE, 0x09, 0x00);
			return XST_SUCCESS;
			break;

		case 2 : // HDMI 2.0 (3.4Gbps - 6 Gbps)
			  if (DP159_VERBOSE)
				  xil_printf("Program zombie HDMI 2.0\r\n");

			 // Select page 1
			r = i2c_dp159_write(DP159_ZOMBIE, 0xff, 0x01);

			// PLL_FBDIV is 280
			r = i2c_dp159_write(DP159_ZOMBIE, 0x04, 0x80);
			r = i2c_dp159_write(DP159_ZOMBIE, 0x05, 0x02);

			// PLL_PREDIV is 4
			r = i2c_dp159_write(DP159_ZOMBIE, 0x08, 0x04);

			// CDR_CONFIG[4:0]
			r = i2c_dp159_write(DP159_ZOMBIE, 0x0e, 0x10);

			// CP_CURRENT
			r = i2c_dp159_write(DP159_ZOMBIE, 0x01, 0x81);
			usleep(10000);

			// Enable Bandgap
			r = i2c_dp159_write(DP159_ZOMBIE, 0x00, 0x02);
			usleep(10000);

			// Enable PLL
			r = i2c_dp159_write(DP159_ZOMBIE, 0x00, 0x03);

			// Enable TX
			r = i2c_dp159_write(DP159_ZOMBIE, 0x10, 0x0f);

			// HDMI_TWPST1
			r = i2c_dp159_write(DP159_ZOMBIE, 0x14, 0x10);

			// DP_TWPST1
			r = i2c_dp159_write(DP159_ZOMBIE, 0x16, 0x10);

			// DP_TWPST2
			r = i2c_dp159_write(DP159_ZOMBIE, 0x17, 0x00);

			// Slew CTRL
			r = i2c_dp159_write(DP159_ZOMBIE, 0x12, 0x28);

			// FIR_UPD
			r = i2c_dp159_write(DP159_ZOMBIE, 0x13, 0x0f);
			r = i2c_dp159_write(DP159_ZOMBIE, 0x13, 0x00);

			// TX_RATE
			r = i2c_dp159_write(DP159_ZOMBIE, 0x11, 0x30);

			// Enable receivers
			r = i2c_dp159_write(DP159_ZOMBIE, 0x30, 0x0f);

			// PD_RXINT
			r = i2c_dp159_write(DP159_ZOMBIE, 0x32, 0x00);

			// RX_RATE
			r = i2c_dp159_write(DP159_ZOMBIE, 0x31, 0x00);

			// Disable offset correction
			r = i2c_dp159_write(DP159_ZOMBIE, 0x34, 0x00);

			// Change default of CDR_STL
			r = i2c_dp159_write(DP159_ZOMBIE, 0x3c, 0x04);

			// Change default of CDR_SO_TR
			r = i2c_dp159_write(DP159_ZOMBIE, 0x3D, 0x06);

			// EQFTC
			r = i2c_dp159_write(DP159_ZOMBIE, 0x4D, 0x18);

			// Enable Adaptive EQ
			r = i2c_dp159_write(DP159_ZOMBIE, 0x4c, 0x03);

			// Select page 0
			r = i2c_dp159_write(DP159_ZOMBIE, 0xff, 0x00);

			// Gate HPD_SNK
			r = i2c_dp159_write(DP159_ZOMBIE, 0x09, 0x01);

			// Set GPIO
			r = i2c_dp159_write(DP159_ZOMBIE, 0xe0, 0x01);

			// Un gate HPD_SNK
			r = i2c_dp159_write(DP159_ZOMBIE, 0x09, 0x00);
			return XST_SUCCESS;
			break;

		}
  }

  else {
	  // Check if the DP159 is a ES device
	  r = i2c_dp159_chk(DP159_ES);

	  if (r == XST_SUCCESS) {
		  //xil_printf("Program DP159 ES... \r\n");
		  r = i2c_dp159_write(DP159_ES, 0x09, 0x06);

		  // HDMI 2.0
		  if ((TxLineRate / (1000000)) > 3400) {
			  if (DP159_VERBOSE)
				  xil_printf("DP159 HDMI 2.0\r\n");
			  r = i2c_dp159_write(DP159_ES, 0x0B, 0x9a);    // SLEW_CTL = Reg0Bh[7:6] = 10
			                                                // TX_TERM_CTL = Reg0Bh[4:3] = 11
#ifndef versal
			  r = i2c_dp159_write(DP159_ES, 0x0C, 0x49);    // VSWING_DATA & VSWING_CLK to +14% = Reg0Ch[7:2] = 100100
			                                                // PRE_SEL = Reg0Ch[1:0] = 01 (labeled HDMI_TWPST)

#else
			  r = i2c_dp159_write(DP159_ES, 0x0C, 0x1C); //44);    // VSWING_DATA to +14% & VSWING_CLK to +7% = Reg0Ch[7:2] = 010001
			                                                // PRE_SEL = Reg0Ch[1:0] = 00 (labeled HDMI_TWPST)
#endif
			  r = i2c_dp159_write(DP159_ES, 0x0D, 0x00);
			  r = i2c_dp159_write(DP159_ES, 0x0A, 0x36);	// Automatic retimer for HDMI 2.0
		  }

		  // HDMI 1.4 (> 2 Gbps)
		  else if ((TxLineRate / (1000000)) > 2000) {
			  if (DP159_VERBOSE)
				  xil_printf("DP159 HDMI 1.4 (> 2 Gbps)\r\n");
			  r = i2c_dp159_write(DP159_ES, 0x0B, 0x88);    // SLEW_CTL = Reg0Bh[7:6] = 10
			                                                // TX_TERM_CTL = Reg0Bh[4:3] = 01
#ifndef versal
			  r = i2c_dp159_write(DP159_ES, 0x0C, 0x48);	// VSWING_DATA & VSWING_CLK to +14% = Reg0Ch[7:2] = 010010
			                                                // PRE_SEL = Reg0Ch[1:0] = 00 (labeled HDMI_TWPST)
#else
			  r = i2c_dp159_write(DP159_ES, 0x0C, 0x00);	// VSWING_DATA & VSWING_CLK to +00% = Reg0Ch[7:2] = 000000
			                                                // PRE_SEL = Reg0Ch[1:0] = 00 (labeled HDMI_TWPST)
#endif
			  r = i2c_dp159_write(DP159_ES, 0x0D, 0x00);
			  r = i2c_dp159_write(DP159_ES, 0x0A, 0x35);	// Automatic redriver to retimer crossover at 1.0 Gbps
		  }
		  // HDMI 1.4 (< 2 Gbps)
		  else {
			  if (DP159_VERBOSE)
				  xil_printf("DP159 HDMI 1.4 (< 2 Gbps)\r\n");
			  r = i2c_dp159_write(DP159_ES, 0x0B, 0x80);    // SLEW_CTL = Reg0Bh[7:6] = 10
			                                                // TX_TERM_CTL = Reg0Bh[4:3] = 00
#ifndef versal
			  r = i2c_dp159_write(DP159_ES, 0x0C, 0x48);	// VSWING_DATA & VSWING_CLK to +14% = Reg0Ch[7:2] = 010010
			                                                // PRE_SEL = Reg0Ch[1:0] = 00 (labeled HDMI_TWPST)
#else
			  r = i2c_dp159_write(DP159_ES, 0x0C, 0x00);	// VSWING_DATA & VSWING_CLK to +00% = Reg0Ch[7:2] = 000000
			                                                // PRE_SEL = Reg0Ch[1:0] = 00 (labeled HDMI_TWPST)
#endif
			  r = i2c_dp159_write(DP159_ES, 0x0D, 0x00);
			  r = i2c_dp159_write(DP159_ES, 0x0A, 0x35);	// Automatic redriver to retimer crossover at 1.0 Gbps
		}
		return XST_SUCCESS;
	  }

	  else {
		  xil_printf("No DP159 device found!\r\n");
		  return XST_FAILURE;
	  }
  }
  return XST_FAILURE;
}
