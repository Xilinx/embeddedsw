/*******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xvid_clk_gen.c
*
* This file contains functions to generate video clock.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  ND	   09/02/20 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xparameters.h"
#include "xdptxss.h"
#include "xvphy.h"

#include "xclk_wiz.h"
#include "xclk_wiz_hw.h"

#ifndef versal
#define CLK_WIZ_BASE      				XPAR_CLK_WIZ_0_BASEADDR
#endif
#define CLK_LOCK                        1

//Following limits are for ZCU102 US+ device
//User to refer to DS and Switching char and update for
//their design
#define VCO_MAX 1600000
#define VCO_MIN 800000

#define M_MAX 128
#define M_MIN 2

#define D_MAX 106
#define D_MIN 1

#define DIV_MAX 128
#define DIV_MIN 1
/************************** Function Prototypes ******************************/
void ComputeMandD_vidGen(u32 VidFreq);
void Gen_vid_clk(XDp *InstancePtr, u8 Stream);

/*****************************************************************************/
/**
*
* This function waits for PLL lock
*
* @return	pass/fail result. If there is error, none-zero value will return
*
* @note		None.
*
******************************************************************************/
int wait_for_lock_vidGen(void)
{
	u32 count, error;
	count = error = 0;
	volatile u32 rdata=0;
	rdata = XClk_Wiz_ReadReg(CLK_WIZ_BASE, 0x04) & 1;
	while (!rdata) {
		if (count == 10000) {
			error++;
			break;
		}
		count++;
		rdata = XClk_Wiz_ReadReg(CLK_WIZ_BASE, 0x04) & 1;
	}
	return error;
}
/*****************************************************************************/
/**
*
* This function computes M and D value
*
* @param	Video frequency
*
* @note		None.
*
******************************************************************************/
void ComputeMandD_vidGen(u32 VidFreq){

	u32 RefFreq;
	u32 m, d, Div, Freq, Diff, Fvco;
	u32 Minerr = 10000;
	u32 MVal = 0;
	u32 DVal = 0;
	u32 DivVal = 0;
	u32 rdata=0;

#if XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTYE4
	RefFreq = 107000;
    for (m = 2; m <= 128; m++) {
            for (d = 1; d <= 106; d++) {
                    Fvco = RefFreq * m / d;
                    if ( Fvco >= 800000 && Fvco <= 1600000 ) {

#else
	RefFreq = 100000;
	for (m = 20; m <= 64; m++) {
		for (d = 1; d <= 80; d++) {
			Fvco = RefFreq * m / d;
			if ( Fvco >= 600000 && Fvco <= 900000 ) {
#endif

				for (Div = 1; Div <= 128; Div++ ) {
					Freq = Fvco/Div;

					if (Freq >= VidFreq) {
						Diff = Freq - VidFreq;
					} else {
						Diff = VidFreq - Freq;
					}

					if (Diff == 0) {
						MVal = m;
						DVal = d;
						DivVal = Div;
						m = 257;
						d = 257;
						Div = 257;
						Minerr = 0;
					} else if (Diff < Minerr) {
						Minerr = Diff;
						MVal = m;
						DVal = d;
						DivVal = Div;

						if (Minerr < 100) {
							m = 257;
							d = 257;
							Div = 257;
						}
					}
				}
			}
		}
	}

	/* Progamming the clocking wizard */
	u32 fail,error,count;
	fail = error = count = 0;

	fail = wait_for_lock_vidGen();
	if(fail) {
		error++;
		xil_printf("\n ERROR: Clock is not locked for "
				"default frequency : 0x%x\r\n",
				XClk_Wiz_ReadReg(CLK_WIZ_BASE, 0x04) & CLK_LOCK);
	}


	/* SW reset applied */
	XClk_Wiz_WriteReg(CLK_WIZ_BASE, 0x0, 0xA);

	rdata = XClk_Wiz_ReadReg(CLK_WIZ_BASE, 0x04) & CLK_LOCK;
	if (rdata) {
		error++;
		xil_printf("\n ERROR: Clock is locked : 0x%x \t "
				"expected 0x00\r\n",
				XClk_Wiz_ReadReg(CLK_WIZ_BASE, 0x04) & CLK_LOCK);
	}

	/* Wait cycles after SW reset */
	for(count = 0 ; count < 2000 ; count++);
	fail = wait_for_lock_vidGen();
	if (fail) {
		error++;
		xil_printf("\n ERROR: Clock is not locked after SW "
				"reset : 0x%x \t Expected : 0x1\r\n",
				XClk_Wiz_ReadReg(CLK_WIZ_BASE, 0x04) & CLK_LOCK);
	}

	/* Configuring Multiply and Divide values */
	XClk_Wiz_WriteReg(CLK_WIZ_BASE, 0x200, (MVal<<8) | DVal);
	XClk_Wiz_WriteReg(CLK_WIZ_BASE, 0x204, 0);
	XClk_Wiz_WriteReg(CLK_WIZ_BASE, 0x208, DivVal);

	/* Load Clock Configuration Register values */
	XClk_Wiz_WriteReg(CLK_WIZ_BASE, 0x25C, 0x07);

	rdata = XClk_Wiz_ReadReg(CLK_WIZ_BASE, 0x04) & CLK_LOCK;
	if (rdata) {
		error++;
		xil_printf("\n ERROR: Clock is locked : 0x%x "
				"\t expected 0x00\r\n",
				XClk_Wiz_ReadReg(CLK_WIZ_BASE, 0x04) & CLK_LOCK);
	}

	/* Clock Configuration Registers are used for dynamic reconfiguration */
	XClk_Wiz_WriteReg(CLK_WIZ_BASE, 0x25C, 0x02);

	fail = wait_for_lock_vidGen();
	if (fail) {
		error++;
		xil_printf("\n ERROR: Clock is not locked : 0x%x "
				"\t Expected : 0x1\r\n",
				XClk_Wiz_ReadReg(CLK_WIZ_BASE, 0x04) & CLK_LOCK);
	}

}



void Gen_vid_clk(XDp *InstancePtr, u8 Stream)
{
	u32 Count = 0;
	XDp_TxMainStreamAttributes *MsaConfig =
			&InstancePtr->TxInstance.MsaConfig[Stream - 1];
	//	xil_printf ("MSA pixel width is %d\r\n",MsaConfig->UserPixelWidth);
	//	xil_printf ("MSA pixel clock is %d\r\n",MsaConfig->PixelClockHz);
	ComputeMandD_vidGen(((MsaConfig->PixelClockHz/1000)/MsaConfig->UserPixelWidth) );

}
