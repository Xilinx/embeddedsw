/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 *****************************************************************************/

/*****************************************************************************/
/**
 *
 * @file main.c
 *
 * This file demonstrates the SDI Tx to SDI Tx loopback design for KCU116
 * board.
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
#include "xil_cache.h"
#include "xparameters.h"
#include "si5324drv.h"
#include "xuartlite_l.h"
#include "fzetta_fmc_ctlr.h"

/************************** Constant Definitions *****************************/
#ifndef SDT
#define GPIO_0_TX_MODE		XPAR_GPIO_0_BASEADDR
#define GPIO_0_RX_MODE		XPAR_GPIO_1_BASEADDR
#define GPIO_2_TX_FORMAT	XPAR_GPIO_2_BASEADDR
#define AUD_ATPG		XPAR_ATPG_BASEADDR
#define GT_RESET		XPAR_GPIO_3_BASEADDR
#else
#define GPIO_0_TX_MODE		XPAR_XGPIO_0_BASEADDR
#define GPIO_0_RX_MODE		XPAR_XGPIO_1_BASEADDR
#define GPIO_2_TX_FORMAT	XPAR_XGPIO_2_BASEADDR
#define AUD_ATPG		XPAR_ATPG_BASEADDR
#define GT_RESET		XPAR_XGPIO_3_BASEADDR
#endif
#define I2C_MUX_ADDR	0x74  /**< I2C Mux Address */
#define I2C_CLK_ADDR	0x69  /**< I2C Clk Address */
#define I2C_CLK_ADDR_570	0x5D  /**< I2C Clk Address for Si570*/

#define CARRIAGE_RETURN		0x0D
#define BACKSPACE		0x08
#define DELETE			0x7F

#define FREQ_148_5_MHz	(148500000)
#define FREQ_148_35_MHz	(148350000)
#define FREQ_148_43_MHz	(148438000)
#define FREQ_297_MHz	(297000000)

/* global variables */
unsigned char inchar;
unsigned char get_val[9];
char slv_type[9];
unsigned char IsValid = 0;
unsigned int State;
unsigned char IsMode = 0;
unsigned char IsSDReso = 0;
unsigned char IsHDReso = 0;
unsigned char Is3GReso = 0;
unsigned char IsHigherReso = 0;
unsigned char IsSDFPS = 0;
unsigned char IsFPS1 = 0;
unsigned char IsFPS2 = 0;
unsigned char IsFPS3 = 0;
unsigned char IsFPS4 = 0;


static int I2cMux(void);
static int I2cClk(u32 InFreq, u32 OutFreq);
static int I2cClk_SI5319(u32 InFreq, u32 OutFreq);
int Si570_SetClock(u32 IICBaseAddress, u8 IICAddress1, u32 RxRefClk);

fzetta_dev_type dev;
u8 channel;
spi_slave_sel slave_sel;
u8 regaddr;
u8 regdata;

/*****************************************************************************/
/**
 *
 * This function copies string based on spi slave selection.
 *
 * @param	StrOut Destination string.
 * @param	slave_sel The string to be copied.
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
void slave_StringCopy(char *StrOut, spi_slave_sel slave_sel) {
	switch (slave_sel) {
	case SPI_RCLKR:
		strcpy(StrOut, "SPI_RCLKR");
		break;

	case SPI_DRVR:
		strcpy(StrOut, "SPI_DRVR");
		break;

	case SPI_RCVR:
		strcpy(StrOut, "SPI_RCVR");
		break;

	default:
		strcpy(StrOut, "DUMMY");
		break;
	}
}



/*****************************************************************************/
/**
 *
 * This function setup SI5324 clock generator over IIC.
 *
 * @return	The number of bytes sent.
 *
 * @note	None.
 *
 ******************************************************************************/
static int I2cMux(void)
{
	u8 Buffer;
	int Status;

	xil_printf("Set i2c mux... ");

	Buffer = 0x18;
#ifndef SDT
	Status = XIic_Send((XPAR_IIC_0_BASEADDR),
				(I2C_MUX_ADDR),
				(u8 *)&Buffer,
				1,
				(XIIC_STOP));
#else
	Status = XIic_Send((XPAR_XIIC_0_BASEADDR),
			   (I2C_MUX_ADDR),
			   (u8 *)&Buffer,
			   1,
			   (XIIC_STOP));
#endif
	xil_printf("done\n\r");

	return Status;
}

/*****************************************************************************/
/**
 *
 * This function setup SI5324 clock generator either in free or locked mode.
 *
 * @param	InFreq specifies an input frequency for the si5324.
 * @param	OutFreq specifies the output frequency of si5324.
 *
 * @return	'XST_FAILURE' if error in programming external clock
 *			else 'XST_SUCCESS' if success
 *
 * @note	None.
 *
 ******************************************************************************/
static int I2cClk(u32 InFreq, u32 OutFreq)
{
	int Status;

	/* Free running mode */
	if (!InFreq) {

#ifndef SDT
		Status = Si5324_SetClock((XPAR_IIC_0_BASEADDR),
					(I2C_CLK_ADDR),
					(SI5324_CLKSRC_XTAL),
					(SI5324_XTAL_FREQ),
					OutFreq);
#else
		Status = Si5324_SetClock((XPAR_XIIC_0_BASEADDR),
					(I2C_CLK_ADDR),
					(SI5324_CLKSRC_XTAL),
					(SI5324_XTAL_FREQ),
					OutFreq);

#endif

		if (Status != (SI5324_SUCCESS)) {
			print("Error programming SI5324\n\r");
			return XST_FAILURE;
		}
	}

	/* Locked mode */
	else {
#ifndef SDT
		Status = Si5324_SetClock((XPAR_IIC_0_BASEADDR),
					(I2C_CLK_ADDR),
					(SI5324_CLKSRC_CLK1),
					InFreq,
					OutFreq);
#else
		Status = Si5324_SetClock((XPAR_XIIC_0_BASEADDR),
					(I2C_CLK_ADDR),
					(SI5324_CLKSRC_CLK1),
					InFreq,
					OutFreq);
#endif

		if (Status != (SI5324_SUCCESS)) {
			print("Error programming SI5324\n\r");
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function setup SI5319 clock generator either in free or locked mode.
 *
 * @param	InFreq specifies an input frequency for the si5319.
 * @param	OutFreq specifies the output frequency of si5319.
 *
 * @return	'XST_FAILURE' if error in programming external clock
 *			else 'XST_SUCCESS' if success
 *
 * @note	None.
 *
 ******************************************************************************/
static int I2cClk_SI5319(u32 InFreq, u32 OutFreq)
{
	int Status;

	/* Free running mode */
	if (!InFreq) {

		Status = Si5324_SetClock((0x800E0000),
					(0x68),
					(SI5324_CLKSRC_XTAL),
					(SI5324_XTAL_FREQ),
					OutFreq);

		if (Status != (SI5324_SUCCESS)) {
			print("Error programming SI5319\n\r");
			return XST_FAILURE;
		} else {
			print("Success programming SI5319\n\r");
		}
	}

	/* Locked mode */
	else {
		Status = Si5324_SetClock((0x800E0000),
					(0x68),
					(SI5324_CLKSRC_CLK1),
					InFreq,
					OutFreq);

		if (Status != (SI5324_SUCCESS)) {
			print("Error programming SI5319\n\r");
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}


/*****************************************************************************/
/**
 *
 * This function clears the screen.
 *
 * @return	None.
 *
 * @note	None
 *
 ******************************************************************************/
void cls(void) {
	/* Clear Sreen */
	xil_printf("%c\[2J", 27);
	/* Bring Cursor to 0,0 */
	xil_printf("%c\033[0;0H", 27);
}

/*****************************************************************************/
/**
 *
 * This function takes the input from user.
 *
 * @return	None.
 *
 * @note	None
 *
 ******************************************************************************/
void get_input2_val(void) {
	unsigned char inchar;
	int char_cnt = 0;
	do {
		inchar = inbyte();
		xil_printf("%c", inchar);
		get_val[char_cnt] = inchar;
		if (!((inchar == BACKSPACE) | (inchar == DELETE))) {
			char_cnt++;
		} else {
			char_cnt--;
		}
	} while ((char_cnt != 9) & (inchar != CARRIAGE_RETURN));
}

/*****************************************************************************/
/**
 *
 * This function display the user menu.
 *
 * @param	IsValid Returns from menu for a 0 value.
 *
 * @return	None.
 *
 * @note	None
 *
 ******************************************************************************/
void ctrl_main_menu(unsigned char IsValid) {
	if (IsValid == 0)
		return;
	xil_printf("\n\r");
	xil_printf("-----------------------\n\r");
	xil_printf("--  FIDUS Main Menu  --\n\r");
	xil_printf("-----------------------\n\r");
	xil_printf("\n\r");
	xil_printf(" Select option\n\r");
	xil_printf(" 1 = Re-Init \n\r");
	xil_printf(" 2 = IIC Dev Select \n\r");
	xil_printf(" 3 = SPI CH0 Select \n\r");
	xil_printf(" 4 = SPI CH1 Select \n\r");
	xil_printf(" 5 = SPI CH2 Select \n\r");
	xil_printf(" 6 = SPI CH3 Select \n\r");
	xil_printf(" 7 = Mode Select \n\r");
	xil_printf(" ? = help \n\r");
	xil_printf("------------------\n\r");
	xil_printf(">");
}

/*****************************************************************************/
/**
 *
 * This function displays spi slave selection menu.
 *
 * @param	IsValid Returns from menu for a 0 value.
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
void ctlr_spi_slv_select_menu(unsigned char IsValid) {
	if (IsValid == 0)
		return;
	xil_printf("\n\r");
	xil_printf("-----------------------\n\r");
	xil_printf("-- CH%d Slave Select  --\n\r", channel);
	xil_printf("-----------------------\n\r");
	xil_printf("\n\r");
	xil_printf(" Select option\n\r");
	xil_printf(" 1 = SPI Receiver \n\r");
	xil_printf(" 2 = SPI Re-Clocker \n\r");
	xil_printf(" 3 = SPI Driver \n\r");
	xil_printf(" m = Main Menu \n\r");
	xil_printf(" ? = help \n\r");
	xil_printf("------------------\n\r");
	xil_printf(">");
}

/*****************************************************************************/
/**
 *
 * This function has Menu to generate modes.
 *
 * @param	IsMode  Returns from menu for a 0 value.
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
void mode_menu(unsigned char IsMode) {
	if (IsMode == 0)
		return;
	xil_printf("\n\r");
	xil_printf("-----------------------\n\r");
	xil_printf("--  Mode Menu  --\n\r");
	xil_printf("-----------------------\n\r");
	xil_printf("\n\r");
	xil_printf(" Select option\n\r");
	xil_printf(" a = SD-SDI \n\r");
	xil_printf(" b = HD-SDI \n\r");
	xil_printf(" c = 3G-SDI Level A \n\r");
	xil_printf(" d = 3G-SDI Level B \n\r");
	xil_printf(" e = 6G-SDI \n\r");
	xil_printf(" f = 12G-SDI \n\r");
	xil_printf("------------------\n\r");
	xil_printf(">");
}

/*****************************************************************************/
/**
 *
 * This function has Menu for resolutions.
 *
 * @param	IsSDReso  Returns from menu for a 0 value.
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
void sd_reso_menu(unsigned char IsSDReso) {
	if (IsSDReso == 0)
		return;
	xil_printf("\n\r");
	xil_printf("-----------------------\n\r");
	xil_printf("--  Resolution Menu  --\n\r");
	xil_printf("-----------------------\n\r");
	xil_printf("\n\r");
	xil_printf(" Select option\n\r");
	xil_printf(" 1 = 525i59 \n\r");
	xil_printf(" 2 = 625i60 \n\r");
	xil_printf("------------------\n\r");
	xil_printf(">");
}

/*****************************************************************************/
/**
 *
 * This function has Menu for HD resolutions.
 *
 * @param	IsHDReso  Returns from menu for a 0 value.
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
void hd_reso_menu(unsigned char IsHDReso) {
	if (IsHDReso == 0)
		return;
	xil_printf("\n\r");
	xil_printf("-----------------------\n\r");
	xil_printf("--  Resolution Menu  --\n\r");
	xil_printf("-----------------------\n\r");
	xil_printf("\n\r");
	xil_printf(" Select option\n\r");
	xil_printf(" 1 = 720p \n\r");
	xil_printf(" 2 = 1080i \n\r");
	xil_printf(" 3 = 1080pSF \n\r");
	xil_printf(" 4 = 1080p \n\r");
	xil_printf("------------------\n\r");
	xil_printf(">");
}

/*****************************************************************************/
/**
 *
 * This function has Menu for 3G resolutions.
 *
 * @param	Is3GReso  Returns from menu for a 0 value.
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
void threeg_reso_menu(unsigned char Is3GReso) {
	if (Is3GReso == 0)
		return;
	xil_printf("\n\r");
	xil_printf("-----------------------\n\r");
	xil_printf("--  Resolution Menu  --\n\r");
	xil_printf("-----------------------\n\r");
	xil_printf("\n\r");
	xil_printf(" Select option\n\r");
	xil_printf(" 1 = 1080p \n\r");
	xil_printf("------------------\n\r");
	xil_printf(">");
}

/*****************************************************************************/
/**
 *
 * This function has Menu for higher resolutions.
 *
 * @param	IsHigerReso  Returns from menu for a 0 value.
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
void higher_reso_menu(unsigned char IsHigerReso) {
	if (IsHigherReso == 0)
		return;
	xil_printf("\n\r");
	xil_printf("-----------------------\n\r");
	xil_printf("--  Resolution Menu  --\n\r");
	xil_printf("-----------------------\n\r");
	xil_printf("\n\r");
	xil_printf(" Select option\n\r");
	xil_printf(" 1 = 2160p \n\r");
	xil_printf("------------------\n\r");
	xil_printf(">");
}

/*****************************************************************************/
/**
 *
 * This function has Menu for fps.
 *
 * @param	IsFPS1  Returns from menu for a 0 value.
 *
 * @return	None
 *
 * @note	HD-720p,1080i, 3G-A, 12G
 *
 ******************************************************************************/
void fps_1_menu(unsigned char IsFPS1)
{
	if (IsFPS1 == 0)
		return;
	xil_printf("\n\r");
	xil_printf("-----------------------\n\r");
	xil_printf("--  Frame Rate Menu  --\n\r");
	xil_printf("-----------------------\n\r");
	xil_printf("\n\r");
	xil_printf(" Select option\n\r");
	xil_printf(" 1 = 50 \n\r");
	xil_printf(" 2 = 59.94 \n\r");
	xil_printf(" 3 = 60 \n\r");
	xil_printf("------------------\n\r");
	xil_printf(">");
}

/*****************************************************************************/
/**
 *
 * This function has Menu for fps.
 *
 * @param	IsFPS2  Returns from menu for a 0 value.
 *
 * @return	None
 *
 * @note	HD-1080p
 *
 ******************************************************************************/
void fps_2_menu(unsigned char IsFPS2)
{
	if (IsFPS2 == 0)
		return;
	xil_printf("\n\r");
	xil_printf("-----------------------\n\r");
	xil_printf("--  Frame Rate Menu  --\n\r");
	xil_printf("-----------------------\n\r");
	xil_printf("\n\r");
	xil_printf(" Select option\n\r");
	xil_printf(" 1 = 23.98 \n\r");
	xil_printf(" 2 = 24 \n\r");
	xil_printf(" 3 = 25 \n\r");
	xil_printf(" 4 = 29.97 \n\r");
	xil_printf(" 5 = 30 \n\r");
	xil_printf("------------------\n\r");
	xil_printf(">");
}

/*****************************************************************************/
/**
 *
 * This function has Menu for fps.
 *
 * @param	IsFPS3  Returns from menu for a 0 value.
 *
 * @return	None
 *
 * @note	3G-B, 6G
 *
 ******************************************************************************/
void fps_3_menu(unsigned char IsFPS3)
{
	if (IsFPS3 == 0)
		return;
	xil_printf("\n\r");
	xil_printf("-----------------------\n\r");
	xil_printf("--  Frame Rate Menu  --\n\r");
	xil_printf("-----------------------\n\r");
	xil_printf("\n\r");
	xil_printf(" Select option\n\r");
	xil_printf(" 1 = 25 \n\r");
	xil_printf(" 2 = 29.97 \n\r");
	xil_printf(" 3 = 30 \n\r");
	xil_printf("------------------\n\r");
	xil_printf(">");
}

/*****************************************************************************/
/**
 *
 * This function has Menu for fps.
 *
 * @param	IsFPS4  Returns from menu for a 0 value.
 *
 * @return	None
 *
 * @note	HD-1080pSF
 *
 ******************************************************************************/
void fps_4_menu(unsigned char IsFPS4)
{
	if (IsFPS4 == 0)
		return;
	xil_printf("\n\r");
	xil_printf("-----------------------\n\r");
	xil_printf("--  Frame Rate Menu  --\n\r");
	xil_printf("-----------------------\n\r");
	xil_printf("\n\r");
	xil_printf(" Select option\n\r");
	xil_printf(" 1 = 23.98 \n\r");
	xil_printf(" 2 = 24 \n\r");
	xil_printf("------------------\n\r");
	xil_printf(">");
}

/*****************************************************************************/
/**
 *
 * This function reports audio status.
 *
 * @param	None
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
void report_audio_status(void)
{
	xil_printf("\n\r------------\r\nSDI Audio Status\n\r------------\r\n");
	xil_printf("\n\rAUDIO: Number of audio samples detected: = 0x%x\r\n",(*(u32 *) (AUD_ATPG +(0x48))));
	xil_printf("\n\rAUDIO: Number of audio samples missed: = 0x%x\r\n",(*(u32 *) (AUD_ATPG +(0x4C))));
}

/*****************************************************************************/
/**
 *
 * This function resets the audio test pattern generator.
 *
 * @param	None
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
void reset_audio_generator(void)
{
	/* Audio test pattern Generator Reset */
	Xil_Out32((UINTPTR) (AUD_ATPG), (u32) (0x00000000));
	sleep(1);
	Xil_Out32((UINTPTR) (AUD_ATPG), (u32) (0x00000003));
}


/*****************************************************************************/
/**
 *
 * This function resets the audio test pattern generator.
 *
 * @param	None
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
void GT_Reset(void)
{
	/* Audio test pattern Generator Reset */
	Xil_Out32((UINTPTR) (GT_RESET), (u32) (0x00000000));
	Xil_Out32((UINTPTR) (GT_RESET), (u32) (0x00000080));
}
/*****************************************************************************/
/**
 *
 * This function has Main control for Fidus FMC register configuration.
 *
 * @param	inchar Character input to show the main menu.
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
void ctrl_app(unsigned char inchar) {
	switch (State) {
	/* Main Menu */
	case 0:
		ctrl_main_menu(IsValid);
		switch (inchar) {
		case '1':
			fzetta_fmc_init();
			State = 0;
			IsValid = 1;
			break;
		/* IIC Dev */
		case '2':
			dev = IIC_Dev;
			channel = DUMMY;
			slave_sel = DUMMY;
			State = 2;
			IsValid = 1;
			break;
		/* SPI Dev Ch0 */
		case '3':
			dev = SPI_Dev;
			channel = 0;
			State = 1;
			IsValid = 1;
			break;
		/* SPI Dev Ch1 */
		case '4':
			dev = SPI_Dev;
			channel = 1;
			State = 1;
			IsValid = 1;
			break;
		/* SPI Dev Ch2 */
		case '5':
			dev = SPI_Dev;
			channel = 2;
			State = 1;
			IsValid = 1;
			break;
		/* SPI Dev Ch3 */
		case '6':
			dev = SPI_Dev;
			channel = 3;
			State = 1;
			IsValid = 1;
			break;
		/* Mode */
		case '7':
			State = 3;
			IsValid = 1;
			break;

		case '?':
			IsValid = 1;
			break;

		case CARRIAGE_RETURN:
			IsValid = 0;
			break;

		default:
			xil_printf("Invalid Entry!\n\r");
			IsValid = 0;
			break;
		}
	break;

	/* SPI Slave Select Channel */
	case 1:
		ctlr_spi_slv_select_menu(IsValid);
		switch (inchar) {
		/* SPI Receiver Select */
		case '1':
			slave_sel = SPI_RCVR;
			State = 2;
			IsValid = 1;
			break;
		/* SPI Re-Clocker Select */
		case '2':
			slave_sel = SPI_RCLKR;
			State = 2;
			IsValid = 1;
			break;

		case '3':
			slave_sel = SPI_DRVR;
			State = 2;
			IsValid = 1;
			break;

		case 'm':
		case 'M':
			State = 0;
			channel = 99;
			IsValid = 1;
			break;

		case CARRIAGE_RETURN:
			IsValid = 0;
			break;

		default:
			xil_printf("Invalid Entry!\n\r");
			IsValid = 0;
			break;
		}
	break;

	/* Register Read/Write */
	case 2:
		xil_printf("\nEnter E-Exit or Address: 0x");
		get_input2_val();
		/* Exit */
		if ((get_val[0] == 'E') || (get_val[0] == 'e')) {
			xil_printf("\n");
			State = (dev == IIC_Dev) ? 0 : 1;
			IsValid = 1;
			break;
		}

	/* Mode Menu */
	case 3:
		mode_menu(IsMode);
		inchar = inbyte();
		xil_printf("\n\rYour choice is: %c\n\r",inchar);
		switch (inchar) {
		/* SD */
		case 'a':
			sd_reso_menu(IsSDReso);
			inchar = inbyte();
			xil_printf("\n\rYour choice is: %c\n\r",inchar);
			xil_printf("\n\rPlease wait for 3 seconds\n\r");
			switch (inchar) {
			case '1':
				Xil_Out32((UINTPTR) (GPIO_0_TX_MODE), (u32) (0x00000001));
				Xil_Out32((UINTPTR) (GPIO_2_TX_FORMAT), (u32) (0x00000000));
				reset_audio_generator();
				sleep(3);
				xil_printf("\n\r------------\r\nSDI Video Status\n\r------------\r\n");
				if (*(u32 *) (GPIO_0_RX_MODE) == 0x1) {
					xil_printf("\n\rPASS: Rx mode received: 525i59 (SD-SDI)\n\r");
				} else {
					xil_printf("\n\rFAIL: Rx mode not matching 525i59 (SD-SDI) \n\r");
					xil_printf("\n\rInfo: Please check Rx VIO for more details \n\r");
				}
				report_audio_status();
				IsSDReso = 1;
				break;
			case '2':
				Xil_Out32((UINTPTR) (GPIO_0_TX_MODE), (u32) (0x00000001));
				Xil_Out32((UINTPTR) (GPIO_2_TX_FORMAT), (u32) (0x00000001));
				reset_audio_generator();
				sleep(3);
				xil_printf("\n\r------------\r\nSDI Video Status\n\r------------\r\n");
				if (*(u32 *) (GPIO_0_RX_MODE) == 0x1) {
					xil_printf("\n\rPASS: Rx mode received: 625i50 (SD-SDI)\n\r");
				} else {
					xil_printf("\n\rFAIL: Rx mode not matching 625i50 (SD-SDI) \n\r");
					xil_printf("\n\rInfo: Please check Rx VIO for more details \n\r");
				}
				report_audio_status();
				IsSDReso = 1;
				break;
			}
		IsMode = 1;
		break;
		/* HD */
		case 'b':
			hd_reso_menu(IsHDReso);
			inchar = inbyte();
			xil_printf("\n\rYour choice is: %c\n\r",inchar);
			switch(inchar){
			case'1':
				fps_1_menu(IsFPS1);
				inchar = inbyte();
				xil_printf("\n\rYour choice is: %c\n\r",inchar);
				xil_printf("\n\rPlease wait for 3 seconds\n\r");
				switch(inchar){
				case'1':
					Xil_Out32((UINTPTR) (GPIO_0_TX_MODE), (u32) (0x00000000));
					Xil_Out32((UINTPTR) (GPIO_2_TX_FORMAT), (u32) (0x00000000));
					reset_audio_generator();
					sleep(3);
					xil_printf("\n\r------------\r\nSDI Video Status\n\r------------\r\n");
					if (*(u32 *) (GPIO_0_RX_MODE) == 0x2) {
						xil_printf("\n\rPASS: Rx mode received: 720p50 (HD-SDI)\n\r");
					} else {
						xil_printf("\n\rFAIL: Rx mode not matching 720p50 (HD-SDI) \n\r");
						xil_printf("\n\rInfo: Please check Rx VIO for more details \n\r");
					}
					report_audio_status();
					IsFPS1 = 1;
					break;
				case'2':
					Xil_Out32((UINTPTR) (GPIO_0_TX_MODE), (u32) (0x00000008));
					Xil_Out32((UINTPTR) (GPIO_2_TX_FORMAT), (u32) (0x00000007));
					reset_audio_generator();
					sleep(3);
					xil_printf("\n\r------------\r\nSDI Video Status\n\r------------\r\n");
					if (*(u32 *) (GPIO_0_RX_MODE) == 0x42) {
						xil_printf("\n\rPASS: Rx mode received: 720p59.94 (HD-SDI)\n\r");
					} else {
						xil_printf("\n\rFAIL: Rx mode not matching 720p59.94 (HD-SDI) \n\r");
						xil_printf("\n\rInfo: Please check Rx VIO for more details \n\r");

					}
					report_audio_status();
					IsFPS1 = 1;
					break;
				case'3':
					Xil_Out32((UINTPTR) (GPIO_0_TX_MODE), (u32) (0x00000000));
					Xil_Out32((UINTPTR) (GPIO_2_TX_FORMAT), (u32) (0x00000007));
					reset_audio_generator();
					sleep(3);
					xil_printf("\n\r------------\r\nSDI Video Status\n\r------------\r\n");
					if (*(u32 *) (GPIO_0_RX_MODE) == 0x2) {
						xil_printf("\n\rPASS: Rx mode received: 720p60 (HD-SDI)\n\r");
					} else {
						xil_printf("\n\rFAIL: Rx mode not matching 720p60 (HD-SDI) \n\r");
						xil_printf("\n\rInfo: Please check Rx VIO for more details \n\r");
					}
					report_audio_status();
					IsFPS1 = 1;
					break;
				}
				IsHDReso = 1;
				break;
			case'2':
				fps_1_menu(IsFPS1);
				inchar = inbyte();
				xil_printf("\n\rYour choice is: %c\n\r",inchar);
				xil_printf("\n\rPlease wait for 3 seconds\n\r");
				switch(inchar){
				case'1':
					Xil_Out32((UINTPTR) (GPIO_0_TX_MODE), (u32) (0x00000000));
					Xil_Out32((UINTPTR) (GPIO_2_TX_FORMAT), (u32) (0x00000003));
					reset_audio_generator();
					sleep(3);
					xil_printf("\n\r------------\r\nSDI Video Status\n\r------------\r\n");
					if (*(u32 *) (GPIO_0_RX_MODE) == 0x2) {
						xil_printf("\n\rPASS: Rx mode received: 1080i50 (HD-SDI)\n\r");
					} else {
						xil_printf("\n\rFAIL: Rx mode not matching 1080i50 (HD-SDI) \n\r");
						xil_printf("\n\rInfo: Please check Rx VIO for more details \n\r");

					}
					report_audio_status();
					IsFPS1 = 1;
					break;
				case'2':
					Xil_Out32((UINTPTR) (GPIO_0_TX_MODE), (u32) (0x00000008));
					Xil_Out32((UINTPTR) (GPIO_2_TX_FORMAT), (u32) (0x00000002));
					reset_audio_generator();
					sleep(3);
					xil_printf("\n\r------------\r\nSDI Video Status\n\r------------\r\n");
					if (*(u32 *) (GPIO_0_RX_MODE) == 0x42) {
						xil_printf("\n\rPASS: Rx mode received: 1080i59.94 (HD-SDI)\n\r");
					} else {
						xil_printf("\n\rFAIL: Rx mode not matching 1080i59.94 (HD-SDI) \n\r");
						xil_printf("\n\rInfo: Please check Rx VIO for more details \n\r");
					}
					report_audio_status();
					IsFPS1 = 1;
					break;
				case'3':
					Xil_Out32((UINTPTR) (GPIO_0_TX_MODE), (u32) (0x00000000));
					Xil_Out32((UINTPTR) (GPIO_2_TX_FORMAT), (u32) (0x00000002));
					reset_audio_generator();
					sleep(3);
					xil_printf("\n\r------------\r\nSDI Video Status\n\r------------\r\n");
					if (*(u32 *) (GPIO_0_RX_MODE) == 0x2) {
						xil_printf("\n\rPASS: Rx mode received: 1080i60 (HD-SDI)\n\r");
					} else {
						xil_printf("\n\rFAIL: Rx mode not matching 1080i60 (HD-SDI) \n\r");
						xil_printf("\n\rInfo: Please check Rx VIO for more details \n\r");
					}
					report_audio_status();
					IsFPS1 = 1;
					break;
				}
				IsHDReso = 1;
				break;
			case'3':
				fps_4_menu(IsFPS4);
				inchar = inbyte();
				xil_printf("\n\rYour choice is: %c\n\r",inchar);
				xil_printf("\n\rPlease wait for 3 seconds\n\r");
				switch(inchar){
				case'1':
					Xil_Out32((UINTPTR) (GPIO_0_TX_MODE), (u32) (0x00000008));
					Xil_Out32((UINTPTR) (GPIO_2_TX_FORMAT), (u32) (0x00000001));
					reset_audio_generator();
					sleep(3);
					xil_printf("\n\r------------\r\nSDI Video Status\n\r------------\r\n");
					if (*(u32 *) (GPIO_0_RX_MODE) == 0x42) {
						xil_printf("\n\rPASS: Rx mode received: 1080pSF23.98 (HD-SDI)\n\r");
					} else {
						xil_printf("\n\rFAIL: Rx mode not matching 1080pSF23.98 (HD-SDI) \n\r");
						xil_printf("\n\rInfo: Please check Rx VIO for more details \n\r");
					}
					report_audio_status();
					IsFPS4 = 1;
					break;
				case'2':
					Xil_Out32((UINTPTR) (GPIO_0_TX_MODE), (u32) (0x00000000));
					Xil_Out32((UINTPTR) (GPIO_2_TX_FORMAT), (u32) (0x00000001));
					reset_audio_generator();
					sleep(3);
					xil_printf("\n\r------------\r\nSDI Video Status\n\r------------\r\n");
					if (*(u32 *) (GPIO_0_RX_MODE) == 0x2) {
						xil_printf("\n\rPASS: Rx mode received: 1080pSF24 (HD-SDI)\n\r");
					} else {
						xil_printf("\n\rFAIL: Rx mode not matching 1080pSF24 (HD-SDI) \n\r");
						xil_printf("\n\rInfo: Please check Rx VIO for more details \n\r");
					}
					report_audio_status();
					IsFPS4 = 1;
					break;
				}
				IsHDReso = 1;
				break;
			case'4':
				fps_2_menu(IsFPS2);
				inchar = inbyte();
				xil_printf("\n\rYour choice is: %c\n\r",inchar);
				xil_printf("\n\rPlease wait for 3 seconds\n\r");
				switch(inchar){
				case'1':
					Xil_Out32((UINTPTR) (GPIO_0_TX_MODE), (u32) (0x00000008));
					Xil_Out32((UINTPTR) (GPIO_2_TX_FORMAT), (u32) (0x00000006));
					reset_audio_generator();
					sleep(3);
					xil_printf("\n\r------------\r\nSDI Video Status\n\r------------\r\n");
					if (*(u32 *) (GPIO_0_RX_MODE) == 0x42) {
						xil_printf("\n\rPASS: Rx mode received: 1080p23.98 (HD-SDI)\n\r");
					} else {
						xil_printf("\n\rFAIL: Rx mode not matching 1080p23.98 (HD-SDI) \n\r");
						xil_printf("\n\rInfo: Please check Rx VIO for more details \n\r");
					}
					report_audio_status();
					IsFPS2 = 1;
					break;
				case'2':
					Xil_Out32((UINTPTR) (GPIO_0_TX_MODE), (u32) (0x00000000));
					Xil_Out32((UINTPTR) (GPIO_2_TX_FORMAT), (u32) (0x00000006));
					reset_audio_generator();
					sleep(3);
					xil_printf("\n\r------------\r\nSDI Video Status\n\r------------\r\n");
					if (*(u32 *) (GPIO_0_RX_MODE) == 0x2) {
						xil_printf("\n\rPASS: Rx mode received: 1080p24 (HD-SDI)\n\r");
					} else {
						xil_printf("\n\rFAIL: Rx mode not matching 1080p24 (HD-SDI) \n\r");
						xil_printf("\n\rInfo: Please check Rx VIO for more details \n\r");
					}
					report_audio_status();
					IsFPS2 = 1;
					break;
				case'3':
					Xil_Out32((UINTPTR) (GPIO_0_TX_MODE), (u32) (0x00000000));
					Xil_Out32((UINTPTR) (GPIO_2_TX_FORMAT), (u32) (0x00000005));
					reset_audio_generator();
					sleep(3);
					xil_printf("\n\r------------\r\nSDI Video Status\n\r------------\r\n");
					if (*(u32 *) (GPIO_0_RX_MODE) == 0x2) {
						xil_printf("\n\rPASS: Rx mode received: 1080p25 (HD-SDI)\n\r");
					} else {
						xil_printf("\n\rFAIL: Rx mode not matching 1080p25 (HD-SDI) \n\r");
						xil_printf("\n\rInfo: Please check Rx VIO for more details \n\r");
					}
					report_audio_status();
					IsFPS2 = 1;
					break;
				case'4':
					Xil_Out32((UINTPTR) (GPIO_0_TX_MODE), (u32) (0x00000008));
					Xil_Out32((UINTPTR) (GPIO_2_TX_FORMAT), (u32) (0x00000004));
					reset_audio_generator();
					sleep(3);
					xil_printf("\n\r------------\r\nSDI Video Status\n\r------------\r\n");
					if (*(u32 *) (GPIO_0_RX_MODE) == 0x42) {
						xil_printf("\n\rPASS: Rx mode received: 1080p29.97 (HD-SDI)\n\r");
					} else {
						xil_printf("\n\rFAIL: Rx mode not matching 1080p29.97 (HD-SDI) \n\r");
						xil_printf("\n\rInfo: Please check Rx VIO for more details \n\r");
					}
					report_audio_status();
					IsFPS2 = 1;
					break;
				case'5':
					Xil_Out32((UINTPTR) (GPIO_0_TX_MODE), (u32) (0x00000000));
					Xil_Out32((UINTPTR) (GPIO_2_TX_FORMAT), (u32) (0x00000004));
					reset_audio_generator();
					sleep(3);
					xil_printf("\n\r------------\r\nSDI Video Status\n\r------------\r\n");
					if (*(u32 *) (GPIO_0_RX_MODE) == 0x2) {
						xil_printf("\n\rPASS: Rx mode received: 1080p30 (HD-SDI)\n\r");
					} else {
						xil_printf("\n\rFAIL: Rx mode not matching 1080p30 (HD-SDI) \n\r");
						xil_printf("\n\rInfo: Please check Rx VIO for more details \n\r");
					}
					report_audio_status();
					IsFPS2 = 1;
					break;
				}
				IsHDReso = 1;
				break;
			}
			IsMode = 1;
			break;
		/* 3G-A */
		case 'c':
			threeg_reso_menu(Is3GReso);
			inchar = inbyte();
			xil_printf("\n\rYour choice is: %c\n\r", inchar);
			switch(inchar){
			case'1':
				fps_1_menu(IsFPS1);
				inchar = inbyte();
				xil_printf("\n\rYour choice is: %c\n\r",inchar);
				xil_printf("\n\rPlease wait for 3 seconds\n\r");
				switch(inchar){
				case'1':
					Xil_Out32((UINTPTR) (GPIO_0_TX_MODE), (u32) (0x00000002));
					Xil_Out32((UINTPTR) (GPIO_2_TX_FORMAT), (u32) (0x00000005));
					reset_audio_generator();
					sleep(3);
					xil_printf("\n\r------------\r\nSDI Video Status\n\r------------\r\n");
					if (*(u32 *) (GPIO_0_RX_MODE) == 0x4) {
						xil_printf("\n\rPASS: Rx mode received: 1080p50 (3G-SDI Level A)\n\r");
					} else {
						xil_printf("\n\rFAIL: Rx mode not matching 1080p50 (3G-SDI Level A) \n\r");
						xil_printf("\n\rInfo: Please check Rx VIO for more details \n\r");
					}
					report_audio_status();
					IsFPS1 = 1;
					break;
				case'2':
					Xil_Out32((UINTPTR) (GPIO_0_TX_MODE), (u32) (0x0000000A));
					Xil_Out32((UINTPTR) (GPIO_2_TX_FORMAT), (u32) (0x00000004));
					reset_audio_generator();
					sleep(3);
					xil_printf("\n\r------------\r\nSDI Video Status\n\r------------\r\n");
					if (*(u32 *) (GPIO_0_RX_MODE) == 0x44) {
						xil_printf("\n\rPASS: Rx mode received: 1080p59.94 (3G-SDI Level A)\n\r");
					} else {
						xil_printf("\n\rFAIL: Rx mode not matching 1080p59.94 (3G-SDI Level A) \n\r");
						xil_printf("\n\rInfo: Please check Rx VIO for more details \n\r");
					}
					report_audio_status();
					IsFPS1 = 1;
					break;
				case'3':
					Xil_Out32((UINTPTR) (GPIO_0_TX_MODE), (u32) (0x00000002));
					Xil_Out32((UINTPTR) (GPIO_2_TX_FORMAT), (u32) (0x00000004));
					reset_audio_generator();
					sleep(3);
					xil_printf("\n\r------------\r\nSDI Video Status\n\r------------\r\n");
					if (*(u32 *) (GPIO_0_RX_MODE) == 0x4) {
						xil_printf("\n\rPASS: Rx mode received: 1080p60 (3G-SDI Level A)\n\r");
					} else {
						xil_printf("\n\rFAIL: Rx mode not matching 1080p60 (3G-SDI Level A) \n\r");
						xil_printf("\n\rInfo: Please check Rx VIO for more details \n\r");
					}
					report_audio_status();
					IsFPS1 = 1;
					break;
				}
				Is3GReso = 1;
				break;
			}
			IsMode = 1;
			break;
		/* 3G-B */
		case 'd':
			threeg_reso_menu(Is3GReso);
			inchar = inbyte();
			xil_printf("\n\rYour choice is: %c\n\r",inchar);
			switch(inchar){
			case'1':
				fps_3_menu(IsFPS3);
				inchar = inbyte();
				xil_printf("\n\rYour choice is: %c\n\r",inchar);
				xil_printf("\n\rPlease wait for 3 seconds\n\r");
				switch(inchar){
				case'1':
					Xil_Out32((UINTPTR) (GPIO_0_TX_MODE), (u32) (0x00000003));
					Xil_Out32((UINTPTR) (GPIO_2_TX_FORMAT), (u32) (0x00000005));
					reset_audio_generator();
					sleep(3);
					xil_printf("\n\r------------\r\nSDI Video Status\n\r------------\r\n");
					if (*(u32 *) (GPIO_0_RX_MODE) == 0x24) {
						xil_printf("\n\rPASS: Rx mode received: 1080p25 (3G-SDI Level B)\n\r");
					} else {
						xil_printf("\n\rFAIL: Rx mode not matching 1080p25 (3G-SDI Level B) \n\r");
						xil_printf("\n\rInfo: Please check Rx VIO for more details \n\r");
					}
					report_audio_status();
					IsFPS3 = 1;
					break;
				case'2':
					Xil_Out32((UINTPTR) (GPIO_0_TX_MODE), (u32) (0x0000000B));
					Xil_Out32((UINTPTR) (GPIO_2_TX_FORMAT), (u32) (0x00000004));
					reset_audio_generator();
					sleep(3);
					xil_printf("\n\r------------\r\nSDI Video Status\n\r------------\r\n");
					if (*(u32 *) (GPIO_0_RX_MODE) == 0x64) {
						xil_printf("\n\rPASS: Rx mode received: 1080p29.97 (3G-SDI Level B)\n\r");
					} else {
						xil_printf("\n\rFAIL: Rx mode not matching 1080p29.97 (3G-SDI Level B) \n\r");
						xil_printf("\n\rInfo: Please check Rx VIO for more details \n\r");
					}
					report_audio_status();
					IsFPS3 = 1;
					break;
				case'3':
					Xil_Out32((UINTPTR) (GPIO_0_TX_MODE), (u32) (0x00000003));
					Xil_Out32((UINTPTR) (GPIO_2_TX_FORMAT), (u32) (0x00000004));
					reset_audio_generator();
					sleep(3);
					xil_printf("\n\r------------\r\nSDI Video Status\n\r------------\r\n");
					if (*(u32 *) (GPIO_0_RX_MODE) == 0x24) {
						xil_printf("\n\rPASS: Rx mode received: 1080p30 (3G-SDI Level B)\n\r");
					} else {
						xil_printf("\n\rFAIL: Rx mode not matching 1080p30 (3G-SDI Level B) \n\r");
						xil_printf("\n\rInfo: Please check Rx VIO for more details \n\r");
					}
					report_audio_status();
					IsFPS3 = 1;
					break;
				}
				Is3GReso = 1;
				break;
			}
			IsMode = 1;
			break;
		/* 6G */
		case 'e':
			higher_reso_menu(IsHigherReso);
			inchar = inbyte();
			xil_printf("\n\rYour choice is: %c\n\r",inchar);
			switch(inchar){
			case'1':
				fps_3_menu(IsFPS3);
				inchar = inbyte();
				xil_printf("\n\rYour choice is: %c\n\r",inchar);
				xil_printf("\n\rPlease wait for 3 seconds\n\r");
				switch(inchar){
				case'1':
					Xil_Out32((UINTPTR) (GPIO_0_TX_MODE), (u32) (0x00000004));
					Xil_Out32((UINTPTR) (GPIO_2_TX_FORMAT), (u32) (0x00000005));
					reset_audio_generator();
					sleep(3);
					xil_printf("\n\r------------\r\nSDI Video Status\n\r------------\r\n");
					if (*(u32 *) (GPIO_0_RX_MODE) == 0x8) {
						xil_printf("\n\rPASS: Rx mode received: 2160p25 (6G-SDI)\n\r");
					} else {
						xil_printf("\n\rFAIL: Rx mode not matching 2160p25 (6G-SDI) \n\r");
						xil_printf("\n\rInfo: Please check Rx VIO for more details \n\r");
					}
					report_audio_status();
					IsFPS3 = 1;
					break;
				case'2':
					Xil_Out32((UINTPTR) (GPIO_0_TX_MODE), (u32) (0x0000000C));
					Xil_Out32((UINTPTR) (GPIO_2_TX_FORMAT), (u32) (0x00000004));
					reset_audio_generator();
					sleep(3);
					xil_printf("\n\r------------\r\nSDI Video Status\n\r------------\r\n");
					if (*(u32 *) (GPIO_0_RX_MODE) == 0x48) {
						xil_printf("\n\rPASS: Rx mode received: 2160p29.97 (6G-SDI)\n\r");
					} else {
						xil_printf("\n\rFAIL: Rx mode not matching 2160p29.97 (6G-SDI) \n\r");
						xil_printf("\n\rInfo: Please check Rx VIO for more details \n\r");
					}
					report_audio_status();
					IsFPS3 = 1;
					break;
				case'3':
					Xil_Out32((UINTPTR) (GPIO_0_TX_MODE), (u32) (0x00000004));
					Xil_Out32((UINTPTR) (GPIO_2_TX_FORMAT), (u32) (0x00000004));
					reset_audio_generator();
					sleep(3);
					xil_printf("\n\r------------\r\nSDI Video Status\n\r------------\r\n");
					if (*(u32 *) (GPIO_0_RX_MODE) == 0x8) {
						xil_printf("\n\rPASS: Rx mode received: 2160p30 (6G-SDI)\n\r");
					} else {
						xil_printf("\n\rFAIL: Rx mode not matching 2160p30 (6G-SDI) \n\r");
						xil_printf("\n\rInfo: Please check Rx VIO for more details \n\r");
					}
					report_audio_status();
					IsFPS3 = 1;
					break;
				}
				IsHigherReso = 1;
				break;
			}
			IsMode = 1;
			break;
		/* 12G */
		case 'f':
			higher_reso_menu(IsHigherReso);
			inchar = inbyte();
			xil_printf("\n\rYour choice is: %c\n\r",inchar);
			switch(inchar){
			case'1':
				fps_1_menu(IsFPS1);
				inchar = inbyte();
				xil_printf("\n\rYour choice is: %c\n\r",inchar);
				xil_printf("\n\rPlease wait for 3 seconds\n\r");
				switch(inchar){
				case'1':
					Xil_Out32((UINTPTR) (GPIO_0_TX_MODE), (u32) (0x00000005));
					Xil_Out32((UINTPTR) (GPIO_2_TX_FORMAT), (u32) (0x00000005));
					reset_audio_generator();
					sleep(3);
					xil_printf("\n\r------------\r\nSDI Video Status\n\r------------\r\n");
					if (*(u32 *) (GPIO_0_RX_MODE) == 0x10) {
						xil_printf("\n\rPASS: Rx mode received: 2160p50 (12G-SDI)\n\r");
					} else {
						xil_printf("\n\rFAIL: Rx mode not matching 2160p50 (12G-SDI) \n\r");
						xil_printf("\n\rInfo: Please check Rx VIO for more details \n\r");
					}
					report_audio_status();
					IsFPS1 = 1;
					break;
				case'2':
					Xil_Out32((UINTPTR) (GPIO_0_TX_MODE), (u32) (0x0000000D));
					Xil_Out32((UINTPTR) (GPIO_2_TX_FORMAT), (u32) (0x00000004));
					reset_audio_generator();
					sleep(3);
					xil_printf("\n\r------------\r\nSDI Video Status\n\r------------\r\n");
					if (*(u32 *) (GPIO_0_RX_MODE) == 0x50) {
						xil_printf("\n\rPASS: Rx mode received: 2160p59.94 (12G-SDI)\n\r");
					} else {
						xil_printf("\n\rFAIL: Rx mode not matching 2160p59.94 (12G-SDI) \n\r");
						xil_printf("\n\rInfo: Please check Rx VIO for more details \n\r");
					}
					report_audio_status();
					IsFPS1 = 1;
					break;
				case'3':
					Xil_Out32((UINTPTR) (GPIO_0_TX_MODE), (u32) (0x00000005));
					Xil_Out32((UINTPTR) (GPIO_2_TX_FORMAT), (u32) (0x00000004));
					reset_audio_generator();
					sleep(3);
					xil_printf("\n\r------------\r\nSDI Video Status\n\r------------\r\n");
					if (*(u32 *) (GPIO_0_RX_MODE) == 0x10) {
						xil_printf("\n\rPASS: Rx mode received: 2160p60 (12G-SDI)\n\r");
					} else {
						xil_printf("\n\rFAIL: Rx mode not matching 2160p60 (12G-SDI) \n\r");
						xil_printf("\n\rInfo: Please check Rx VIO for more details \n\r");
					}
					report_audio_status();
					IsFPS1 = 1;
					break;
				}
				IsHigherReso = 1;
				break;
			}
			IsMode = 1;
			break;

		case CARRIAGE_RETURN:
			IsMode = 0;
			break;

		default:
			xil_printf("Invalid Entry! Try again.\n\r");
			IsMode = 0;
			break;
		}
	break;

	case 4:
		regaddr = (unsigned int) strtoul(get_val, NULL, 16);
		xil_printf("\nEnter R-Read or Data: 0x");
		get_input2_val();
		slave_StringCopy(&slv_type[0], slave_sel);
		if ((get_val[0] == 'R') || (get_val[0] == 'r')) { /* READ */
			if (dev == IIC_Dev) {
				xil_printf("\nCannot Read Registers from IIC Device!\n\r");
			} else {
				xil_printf(
						"\n\nREAD  Dev: %s Ch: %d Slave: %s Addr: 0x%02x Data: 0x%02x\n\r",
						dev ? "SPI" : "IIC", channel, slv_type, regaddr,
						fzetta_fmc_register_read(dev, channel, slave_sel,
								regaddr));
			}
		} else { /* WRITE */
			regdata = (unsigned int) strtoul(get_val, NULL, 16);
			xil_printf(
					"\n\nWRITE Dev: %s Ch: %d Slave: %s Addr: 0x%02x Data: 0x%02x\n\r",
					dev ? "SPI" : "IIC", channel, slv_type, regaddr, regdata);
			fzetta_fmc_register_write(&dev, &channel, &slave_sel, &regaddr,
					&regdata);
		}

		State = 2;
		IsValid = 0;
	break;

	case CARRIAGE_RETURN:
		IsValid = 0;
	break;

	default:
		xil_printf("Invalid Entry!\n\r");
		State = 0;
		channel = 99;
		IsValid = 0;
	break;
	}
}

/*****************************************************************************/
/**
 *
 * This function  Fetch input from UART then callback voip_contrl_app to
 * process the selction.
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
void get_ctrl_app_input(void) {
	if (!XUartLite_IsReceiveEmpty(STDIN_BASEADDRESS)) {
		inchar = (u8) XUartLite_ReadReg(STDIN_BASEADDRESS, XUL_RX_FIFO_OFFSET);
		xil_printf("%c\n\r", inchar);
		ctrl_app(inchar);
		ctrl_app(CARRIAGE_RETURN);
	}
}

/*****************************************************************************/
/**
 *
 * Main function to call example with SDI RX and SDI TX drivers for KCU116 board
 *
 * @return
 *		- XST_SUCCESS if SDI example was successfully.
 *		- XST_FAILURE if SDI example failed.
 *
 * @note	None
 *
 ******************************************************************************/
int main() {

	IsMode = 1;
	IsValid = 1;
	IsSDReso = 1;
	IsHDReso = 1;
	Is3GReso = 1;
	IsHigherReso = 1;
	IsSDFPS = 1;
	IsFPS1 = 1;
	IsFPS2 = 1;
	IsFPS3 = 1;
	IsFPS4 = 1;
	State = 0;
	Xil_ICacheEnable();
	Xil_DCacheEnable();
	cls();
	/* Setting path for Si570 chip */
	I2cMux();
#ifndef SDT
	/* si570 configuration of 148.5MHz */
	Si570_SetClock(XPAR_IIC_0_BASEADDR, I2C_CLK_ADDR_570, FREQ_148_35_MHz);
#else
	Si570_SetClock(XPAR_XIIC_0_BASEADDR, I2C_CLK_ADDR_570, FREQ_148_35_MHz);
#endif
	I2cClk(FREQ_148_43_MHz, FREQ_297_MHz);
	sleep(1);
	fzetta_fmc_init();
	Xil_Out32((UINTPTR) (AUD_ATPG), (u32) (0x00000000));
	Xil_Out32((UINTPTR) (AUD_ATPG+ (0x04)), (u32) (0x00000823));
	Xil_Out32((UINTPTR) (AUD_ATPG+ (0x08)), (u32) (0x04040207));
	Xil_Out32((UINTPTR) (AUD_ATPG), (u32) (0x00000003));
	ctrl_app(CARRIAGE_RETURN);
	while (1) {
		get_ctrl_app_input();
	}

	Xil_DCacheDisable();
	Xil_ICacheDisable();

	return 0;
}
