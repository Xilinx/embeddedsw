/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
