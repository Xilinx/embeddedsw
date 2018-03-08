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
 *****************************************************************************/
/*****************************************************************************/
/**
 *
 * @file fzetta_fmc_spi.h
 *
 * FMC configuration file
 *
 * This file configures the FMC card for KCU116 SDI Tx to SDI Rx loopback
 * design
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date       Changes
 * ---- ---- ---------- --------------------------------------------------
 * 1.0  jsr   03/07/2018 Initial version
 * </pre>
 *
 ******************************************************************************/
#ifndef FZETTA_FMC_SPI_H_
#define FZETTA_FMC_SPI_H_

#include <stdio.h>
#include "xspi.h"

XSpi_Config *fzetta_fmc_Spi_ConfigPtr;	/* Pointer to Configuration data */
XSpi fzetta_fmc_Spi; /* The instance of the SPI device */

typedef enum{
	SPI_RCLKR = 0x01, //Reclocker Device ID
	SPI_DRVR  = 0x02, //SDI Driver Device ID
	SPI_RCVR  = 0x04, //Equalizer Device ID
	DUMMY     = 0xFF  //Dummy
}spi_slave_sel;

#define SPI_WR       0x00 // Register Write
#define SPI_RD       0x80 // Register Read
#define SPI_DUMMY    0xFF // Dummy
/*****************************************************************************/
/**
 *
 * This function Initialize SPI IP and its corresponding drivers and configptr instances
 *
 * @param	Dev_ID  Device ID.
 *
 * @return	XST_SUCCESS if initialization is successful else XST_FAILURE
 *
 * @note	None.
 *
 ******************************************************************************/
int fzetta_fmc_spi_init(u8 Dev_ID);
/*****************************************************************************/
/**
 *
 * This function Fidus Zetta FMC Macom SPI Devices Register Write
 * Driver:M23428; EQ:M23554; Reclocker:M23145
 *
 * Register Write SPI Sequence:
 * 		Normal 2 byte SPI sequence with first MSB = 0
 * 		Byte 1: [7]=0 (wr) + [6:0]=Register Address
 * 		Byte 2: [7:0]= Register Data
 *
 * @param	spi_slave_sel  SPI Slave Slection.
 * @param	Slave_sel      Slave Selection.
 * @param       RegAddr        Register Address
 * @param       RegData        Register data
 *
 * @return	XST_SUCCESS if register write is successful else XST_FAILURE
 *
 * @note	None.
 *
 ******************************************************************************/

int fzetta_fmc_spi_devices_register_write(spi_slave_sel Slave_Sel, u8 RegAddr, u8 RegData);
/*****************************************************************************/
/**
 *
 * This function Fidus Zetta FMC Macom SPI Devices Register Write
 * Driver:M23428; EQ:M23554; Reclocker:M23145
 *
 * Register Read SPI Sequence:
 * 		Register Latching:
 * 		2 byte SPI write sequence  with first MSB = 1
 * 		Byte 1: [7]=1 (rd) + [6:0]=Register Address
 * 		Byte 2: [7:0]= Dummy Data (0xFF)
 *
 * 		Data Acquisition: (from MISO)
 * 		2 byte SPI write sequence  with first MSB = 1
 * 		Byte 1: [7]=1 (rd) + [6:0]=Register Address
 * 		Byte 2: [7:0]= Register Data
 *
 * @param	Slave_Sel      SPI Slave Selection.
 * @param       RegAddr        Register Address
 *
 * @return	XST_SUCCESS if register read is successful else XST_FAILURE
 *
 * @note	None.
 *
 ******************************************************************************/

u8 fzetta_fmc_spi_devices_register_read(u32 Slave_Sel, u8 RegAddr);

#endif /* FZETTA_FMC_SPI_H_ */
