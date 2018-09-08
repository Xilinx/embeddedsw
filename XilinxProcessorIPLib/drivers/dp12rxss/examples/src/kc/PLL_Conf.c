/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file PLL_Conf.c
*
* This file contains functions to configure Video Pattern Generator core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  KI    07/13/17 Initial release.
* </pre>
*
******************************************************************************/

#include "PLL_Conf.h"

//void PLL_RegWrite (XSpi *SPI_LMK04906 , u32 RegData , u32 RegNum);

int PLL_init_Seting(XSpi *SPI_LMK04906) {

    const u32 init_Setting_num [27][2] =
    {     //{ Data      , Addr }
		/* 00 */
		{ 0x0000000, 31 },
            { 0x0001000,  0 }, // Reset
		{ 0x0000009,  0 }, // CLKOut0_PD   = 0   ,Divider = 18 (2430/18=135)
		{ 0x0000009,  1 }, // CLKOut1_PD   = 0
//    		{ 0x4000012,  1 }, // CLKOut1_PD   = 1
		{ 0x4000000,  2 }, //    ***

		/* 05 */
		{ 0x4000001,  3 }, //    ***
            { 0x4000000,  4 }, //    ***
		{ 0x4000000,  5 }, // CLKOut5_PD   = 1 (Disable)
			// CLKOut0_TYPE = 0x01 (LVDS) , CLKOut0_ADLY = 0x00 (Analog Dealy 0)
		{ 0x0888000,  6 },
		{ 0x0888000,  7 }, //    ***

	    /* 10 */
			// CLKOut5_TYPE = 0x01 (LVDS) , CLKOut5_ADLY = 0x00 (Analog Dealy 0)
		{ 0x0888000,  8 },
		{ 0x2AAAAAA,  9 }, // === Defaulte ===
		{ 0x08A0200, 10 }, // EN_OSCout0 = 1 (Enable)
//    		{ 0x0820200, 10 }, // EN_OSCout0 = 0 (Disable)
		{ 0x02C0881, 11 }, // xxxx MODE = 5'b00000 Dual PLL, Internal VCO
			// LD_MUX = 3(PLL1 & 2 DLD) , LD_TYPE = 3(Output (Push-Pull)) ,
			// SYNC_PLL2_DLD = 0(Nomal),SYNC_PLL1_DLD = 0(Nomal),EN_TRACK = 1
			// (Enable) HOLDOVER_MODE = 2(Enable)
		{ 0x0D8600B, 12 },

		/* 15 */
			// HOLDOVER_MUX = 7(uWire Readback) ,
			// HOLDOVER_TYPE = Output(Push-Pull) ,
			// CLKin_SELECT_MODE = 0 (CLKin0 Manual)
		{ 0x1D80003, 13 },
			// CLKin1_BUF_TYPE = 1 (CMOS) , CLKin0_BUF_TYPE = 1 (CMOS)
		{ 0x0918000, 14 },
			// HOLDOVER_DLD_CNT = 512 , FORCE_HOLDOVER = 0(Disable)
		{ 0x0000000, 15 },
		{ 0x00AA820, 16 }, // XTAL_LVL = 0 (1.65 Vpp)
		{ 0x0000006, 24 }, // PLL1_WIND_SIZE = 3

		/* 20 */
		{ 0x0080030, 25 },
		{ 0x07D4000, 26 },
		{ 0x0800002, 27 }, // PLL1_R = 1
		{ 0x0008002, 28 }, // PLL2_R = 1 , PLL1_N = 1
		{ 0x000002D, 29 },

		/* 25 */
		{ 0x010002D, 30 }, // PLL2_N = 45
		{ 0x0000001, 31 }

    };
    int i;
    for(i = 0;i < 26 /*NIT_CMD_NUM*/;i++){
        LMK04906_RegWrite(SPI_LMK04906,init_Setting_num[i][0],
						init_Setting_num[i][1]);
    }
//    xil_printf("PLL Init End \n\r");
    return XST_SUCCESS;
}

void CLK81MHz_Out (XSpi *SPI_LMK04906 , u32 CLKout_Num){
	if(0 <= CLKout_Num && CLKout_Num <= 5){
		PLL_RegWrite(SPI_LMK04906,0x000001E, CLKout_Num);
	}
}

void CLK135MHz_Out (XSpi *SPI_LMK04906 , u32 CLKout_Num){
	if(0 <= CLKout_Num && CLKout_Num <= 5){
		PLL_RegWrite(SPI_LMK04906,0x0000012,CLKout_Num);
	}
}

void CLK162MHz_Out (XSpi *SPI_LMK04906 , u32 CLKout_Num){
	if(0 <= CLKout_Num && CLKout_Num <= 5){
		PLL_RegWrite(SPI_LMK04906,0x000000F,CLKout_Num);
	}
}
void CLK270MHz_Out (XSpi *SPI_LMK04906 , u32 CLKout_Num){
	if(0 <= CLKout_Num && CLKout_Num <= 5){
		PLL_RegWrite(SPI_LMK04906,0x0000009,CLKout_Num);
	}
}

void PLL_RegWrite (XSpi *SPI_LMK04906 , u32 RegData , u32 RegNum){
	LMK04906_RegWrite(SPI_LMK04906,0x0000000,31);
	LMK04906_RegWrite(SPI_LMK04906,RegData,RegNum);
	LMK04906_RegWrite(SPI_LMK04906,0x0000001,31);
}
