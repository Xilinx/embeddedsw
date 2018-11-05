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
 * @file fzetta_fmc_init_table.c
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
#include <string.h>
#include "xparameters.h"
#include "fzetta_fmc_gpio.h"
#include "fzetta_fmc_iic.h"
#include "fzetta_fmc_spi.h"
#include "fzetta_fmc_init_table.h"

/*****************************************************************************/
/**
 *
 * This function has FIdus FMC Initialization Table.
 *
 * @param	None.
 *
 * @return	None
 *
 * @note	None.
 *
 ******************************************************************************/

void fzetta_fmc_table_init(){


		//Generic FMC Device Initialization Table
		fzetta_fmc_reg fzetta_fmc_table_temp = {
		.gpio_dev_id = XPAR_GPIO_3_DEVICE_ID,
		.iic_dev_id  = XPAR_IIC_0_DEVICE_ID,
		.spi_dev_id  = XPAR_SPI_0_DEVICE_ID,
		.reglist =
		{   //Dev Type		Channel		Slave Select		RegAddr			RegData
			{ IIC_Dev,		DUMMY,		DUMMY,				0x00,			0x0E},	//148.5MHz (IN2-->OUT0); 148.35MHz (IN3-->OUT1)

			{ SPI_Dev,		0,			SPI_RCVR,			0x03,			0x0F},  //Set output_swing0 (0x03h 1:0)  --> 800mV (11b)
			{ SPI_Dev,		0,			SPI_RCLKR,			0x0C,			0x20},  //Adaptive broadband equalization enable
			{ SPI_Dev,		0,			SPI_RCLKR,			0x0D,			0x40},  //Adaptive boost equalization enable
			{ SPI_Dev,		0,			SPI_DRVR,			0x10,			0x1F},  //Set Input Eq      (0x10 2:0)   --> Max (111)   (set to 0x1F according to Maycom)
			{ SPI_Dev,		0,			SPI_DRVR,			0x20,			0x71},  //Mute   SDIO1      (0x20 bit:0) --> 1           (set to 0x71 according to Maycom)
			{ SPI_Dev,		0,			SPI_DRVR,			0x21,			0x82},  //Derate SDIO2      (0x21 bit:3) --> 1           (Derate and reverse Polarity)

			{ SPI_Dev,		1,			SPI_RCVR,			0x03,			0x0F},  //Set output_swing0 (0x03h 1:0)  --> 800mV (11b)
			{ SPI_Dev,		1,			SPI_RCLKR,			0x0C,			0x20},  //Adaptive broadband equalization enable
			{ SPI_Dev,		1,			SPI_RCLKR,			0x0D,			0x40},  //Adaptive boost equalization enable
			{ SPI_Dev,		1,			SPI_DRVR,			0x10,			0x1F},  //Set Input Eq      (0x10 2:0)   --> Max (111)   (set to 0x1F according to Maycom)
			{ SPI_Dev,		1,			SPI_DRVR,			0x20,			0x71},  //Mute   SDIO1      (0x20 bit:0) --> 1           (set to 0x71 according to Maycom)
			{ SPI_Dev,		1,			SPI_DRVR,			0x21,			0x82},  //Derate SDIO2      (0x21 bit:3) --> 1           (Derate and reverse Polarity)

			{ SPI_Dev,		2,			SPI_RCVR,			0x03,			0x0F},  //Set output_swing0 (0x03h 1:0)  --> 800mV (11b)
			{ SPI_Dev,		2,			SPI_RCLKR,			0x0C,			0x20},  //Adaptive broadband equalization enable
			{ SPI_Dev,		2,			SPI_RCLKR,			0x0D,			0x40},  //Adaptive boost equalization enable
			{ SPI_Dev,		2,			SPI_DRVR,			0x10,			0x1F},  //Set Input Eq      (0x10 2:0)   --> Max (111)   (set to 0x1F according to Maycom)
			{ SPI_Dev,		2,			SPI_DRVR,			0x20,			0x71},  //Mute   SDIO1      (0x20 bit:0) --> 1           (set to 0x71 according to Maycom)
			{ SPI_Dev,		2,			SPI_DRVR,			0x21,			0x82},  //Derate SDIO2      (0x21 bit:3) --> 1           (Derate and reverse Polarity)

			{ SPI_Dev,		3,			SPI_RCVR,			0x03,			0x0F},  //Set output_swing0 (0x03h 1:0)  --> 800mV (11b)
			{ SPI_Dev,		3,			SPI_RCLKR,			0x0C,			0x20},  //Adaptive broadband equalization enable
			{ SPI_Dev,		3,			SPI_RCLKR,			0x0D,			0x40},  //Adaptive boost equalization enable
			{ SPI_Dev,		3,			SPI_DRVR,			0x10,			0x1F},  //Set Input Eq      (0x10 2:0)   --> Max (111)   (set to 0x1F according to Maycom)
			{ SPI_Dev,		3,			SPI_DRVR,			0x20,			0x71},  //Mute   SDIO1      (0x20 bit:0) --> 1           (set to 0x71 according to Maycom)
			{ SPI_Dev,		3,			SPI_DRVR,			0x21,			0x82},  //Derate SDIO2     (0x21 bit:3) --> 1            (Derate and reverse Polarity)

		}
	   };
		memcpy(&fzetta_fmc_table, &fzetta_fmc_table_temp, sizeof(fzetta_fmc_reg));


		//Reclocker ID=80 Errata Initialization Table
		fzetta_fmc_reglist rclkr_errata_id_80_temp[2] =
			{   //Dev Type		Channel		Slave Select		RegAddr			RegData
					{ SPI_Dev,	DUMMY,		SPI_RCLKR,			0x43,			0x25},  //Errata from Maycom for M23145G-11P             (Reclocker�s VCO Out of Range)
					{ SPI_Dev,	DUMMY,		SPI_RCLKR,			0x11,			0x02},  //Errata from Maycom for M23145G-11P             (to perform an acquisition reset)
			};
		memcpy(&rclkr_errata_id_80, &rclkr_errata_id_80_temp, sizeof(rclkr_errata_id_80_temp));

		//Reclocker ID=81 Errata Initialization Table
		fzetta_fmc_reglist rclkr_errata_id_81_temp[5] =
			{   //Dev Type		Channel		Slave Select		RegAddr			RegData
					{ SPI_Dev,	DUMMY,		SPI_RCLKR,			0x22,			0x09},  //Errata from Maycom for M23145G-12P             (Reclocker�s VCO Out of Range)
					{ SPI_Dev,	DUMMY,		SPI_RCLKR,			0x32,			0x3F},  //Errata from Maycom for M23145G-12P             (Reclocker�s VCO Out of Range)
					{ SPI_Dev,	DUMMY,		SPI_RCLKR,			0x3F,			0x03},  //Errata from Maycom for M23145G-12P             (Reclocker�s VCO Out of Range)
					{ SPI_Dev,	DUMMY,		SPI_RCLKR,			0x42,			0x2C},  //Errata from Maycom for M23145G-12P             (Reclocker�s VCO Out of Range)
					{ SPI_Dev,	DUMMY,		SPI_RCLKR,			0x11,			0x02},  //Errata from Maycom for M23145G-12P             (to perform an acquisition reset)
			};
		memcpy(&rclkr_errata_id_81, &rclkr_errata_id_81_temp, sizeof(rclkr_errata_id_81_temp));

		//Reclocker ID=82 Errata Initialization Table
		fzetta_fmc_reglist rclkr_errata_id_82_temp[3] =
			{   //Dev Type		Channel		Slave Select		RegAddr			RegData
					{ SPI_Dev,	DUMMY,		SPI_RCLKR,			0x32,			0x3F},  //Errata from Maycom for M23145G-13P             (Reclocker�s VCO Out of Range)
					{ SPI_Dev,	DUMMY,		SPI_RCLKR,			0x3F,			0x03},  //Errata from Maycom for M23145G-13P             (Reclocker�s VCO Out of Range)
					{ SPI_Dev,	DUMMY,		SPI_RCLKR,			0x11,			0x02},  //Errata from Maycom for M23145G-13P             (to perform an acquisition reset)
			};
		memcpy(&rclkr_errata_id_82, &rclkr_errata_id_82_temp, sizeof(rclkr_errata_id_82_temp));

		//Reclocker ID=82 Errata Initialization Table
		fzetta_fmc_reglist rcvr_errata_id_01_temp[5] =
			{   //Dev Type		Channel		Slave Select		RegAddr			RegData
					{ SPI_Dev,	DUMMY,		SPI_RCVR,			0x22,			0x09},  //Errata from Maycom for M23544G-12P             (Reclocker�s VCO Out of Range)
					{ SPI_Dev,	DUMMY,		SPI_RCVR,			0x32,			0x3F},  //Errata from Maycom for M23544G-12P             (Reclocker�s VCO Out of Range)
					{ SPI_Dev,	DUMMY,		SPI_RCVR,			0x3F,			0x03},  //Errata from Maycom for M23544G-12P             (Reclocker�s VCO Out of Range)
					{ SPI_Dev,	DUMMY,		SPI_RCVR,			0x42,			0x2C},  //Errata from Maycom for M23544G-12P             (Reclocker�s VCO Out of Range)
					{ SPI_Dev,	DUMMY,		SPI_RCVR,			0x11,			0x02},  //Errata from Maycom for M23544G-12P             (to perform an acquisition reset)
			};
		memcpy(&rcvr_errata_id_01, &rcvr_errata_id_01_temp, sizeof(rcvr_errata_id_01_temp));

}
