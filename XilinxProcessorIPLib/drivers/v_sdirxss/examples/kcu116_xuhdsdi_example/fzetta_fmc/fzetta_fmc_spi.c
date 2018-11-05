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
 * @file fzetta_fmc_spi.c
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
#include <stdio.h>
#include "xspi.h"
#include "fzetta_fmc_spi.h"

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

int fzetta_fmc_spi_init(u8 Dev_ID) {
	int Status;
	/*
	 * Initialize the SPI driver so that it is  ready to use.
	 */
   fzetta_fmc_Spi_ConfigPtr = XSpi_LookupConfig(Dev_ID);
	if (fzetta_fmc_Spi_ConfigPtr == NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	Status = XSpi_CfgInitialize(&fzetta_fmc_Spi, fzetta_fmc_Spi_ConfigPtr, fzetta_fmc_Spi_ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the SPI device as a master and in manual slave select mode such
	 * that the slave select signal does not toggle for every byte of a
	 * transfer, this must be done before the slave select is set.
	 */
	Status = XSpi_SetOptions(&fzetta_fmc_Spi, XSP_MASTER_OPTION | XSP_MANUAL_SSELECT_OPTION);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}


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

int fzetta_fmc_spi_devices_register_write(spi_slave_sel Slave_Sel, u8 RegAddr, u8 RegData)
{
   int match;
   int Status;
   static u8 ReadBuffer[2];
   static u8 WriteBuffer[2];

	// Select Slave device
	   Status = XSpi_SetSlaveSelect(&fzetta_fmc_Spi, Slave_Sel);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	// Start SPI IP
	Status = XSpi_Start(&fzetta_fmc_Spi);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	//Disable Interrupts after SPI Start as required by polling
	XSpi_IntrGlobalDisable(&fzetta_fmc_Spi),

   //Construct write buffer
   WriteBuffer[1] = SPI_WR | RegAddr; //ADDR MSB first to be sent
   WriteBuffer[0] = RegData;          //DATA //Last to be sent
   // Perform register write
   Status = XSpi_Transfer(&fzetta_fmc_Spi, WriteBuffer, NULL, 2);
   if (Status != XST_SUCCESS) {
	   return XST_FAILURE;
   }

   int verify = 0;
   if(verify){
	   WriteBuffer[1] = SPI_RD | RegAddr; //ADDR
	   WriteBuffer[0] = SPI_DUMMY;     	  //DATA
	   // Activate read transaction and send target address
	   Status = XSpi_Transfer(&fzetta_fmc_Spi, WriteBuffer, NULL, 2);
	   if (Status != XST_SUCCESS) {
		   return XST_FAILURE;
	   }
	   // Fetch register data in MISO
	   Status = XSpi_Transfer(&fzetta_fmc_Spi, WriteBuffer, ReadBuffer, 2);
	   if (Status != XST_SUCCESS) {
		   return XST_FAILURE;
	   }

	   // Check data
	   match = ((ReadBuffer[1] == (SPI_RD | RegAddr)) & (ReadBuffer[0] == RegData)) ? 1 : 0;
	   if (match != 1) {
		   return XST_FAILURE;
	   }
   }

   Status = XSpi_Stop(&fzetta_fmc_Spi);
   if (Status != XST_SUCCESS) {
	   return XST_FAILURE;
   }
   return XST_SUCCESS;
}


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

u8 fzetta_fmc_spi_devices_register_read(u32 Slave_Sel, u8 RegAddr)
{
   int Status;
   static u8 ReadBuffer[2];
   static u8 WriteBuffer[2];

	// Select Slave device
	Status = XSpi_SetSlaveSelect(&fzetta_fmc_Spi, Slave_Sel);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	// Start SPI IP
	Status = XSpi_Start(&fzetta_fmc_Spi);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	//Disable Interrupts after SPI Start as required by polling
	XSpi_IntrGlobalDisable(&fzetta_fmc_Spi),

    WriteBuffer[1] = SPI_RD | RegAddr; //ADDR
    WriteBuffer[0] = SPI_DUMMY;     	  //DATA
    // Activate read transaction and send target address
    Status = XSpi_Transfer(&fzetta_fmc_Spi, WriteBuffer, NULL, 2);
    if (Status != XST_SUCCESS) {
	    return XST_FAILURE;
    }
    // Fetch register data in MISO
    Status = XSpi_Transfer(&fzetta_fmc_Spi, WriteBuffer, ReadBuffer, 2);
    if (Status != XST_SUCCESS) {
	   return XST_FAILURE;
    }

    Status = XSpi_Stop(&fzetta_fmc_Spi);
    if (Status != XST_SUCCESS) {
	   return XST_FAILURE;
    }

   return ReadBuffer[0];
}
