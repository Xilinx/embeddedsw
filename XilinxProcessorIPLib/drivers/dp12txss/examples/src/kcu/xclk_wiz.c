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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xclk_wiz.c
*
* This file contains functions to configure Video Pattern Generator core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  YB    07/01/15 Initial release.
* </pre>
*
******************************************************************************/


#include "xclk_wiz.h"

#define DYNAMIC_INPUT_FREQ              200                                                                                                         /*Input frequency in MHz */
#define DYNAMIC_INPUT_FREQ_FACTOR       (DYNAMIC_INPUT_FREQ * 10000)

#define DYNAMIC_OUTPUT_FREQ          	100.00                                                                                   /*Output frequency in MHz */
#define DYNAMIC_OUTPUT_FREQFACTOR       (DYNAMIC_OUTPUT_FREQ * 10000)

#define CLK_WIZ_RECONFIG_OUTPUT         DYNAMIC_OUTPUT_FREQ
#define CLK_FRAC_EN 					1


/************************** Function Definitions ******************************/

int wait_for_lock()
{
                u32 count, error;
                count = error = 0;
                u32 rData = XDp_ReadReg(CLK_WIZ_BASE, 0x04);
                while(!   (rData & CLK_LOCK) )
                {
                                if(count == 10000)
                                {
                                                error++;
                                                break;
                                }
                                count++;
                                rData = XDp_ReadReg(CLK_WIZ_BASE, 0x04);
                }
                return error;
}
#if 1
#if (XPAR_XHDCP_NUM_INSTANCES > 0 \
			&& XPAR_DP_TX_HIER_DP_TX_SUBSYSTEM_0_DP_GT_DATAWIDTH == 2)
int wait_for_lock_tx()
{
                u32 count, error;
                count = error = 0;
                u32 rData = XDp_ReadReg(CLK_WIZ_BASE_TX, 0x04);
                while(!   (rData & CLK_LOCK) )
                {
                                if(count == 10000)
                                {
                                                error++;
                                                break;
                                }
                                count++;
                                rData = XDp_ReadReg(CLK_WIZ_BASE_TX, 0x04);
                }
                return error;
}
#endif

#if (XPAR_XHDCP_NUM_INSTANCES > 0 \
		&& XPAR_DP_RX_HIER_DP_RX_SUBSYSTEM_0_DP_GT_DATAWIDTH == 2)
int wait_for_lock_rx()
{
                u32 count, error;
                count = error = 0;
                u32 rData = XDp_ReadReg(CLK_WIZ_BASE_RX, 0x04);
                while(!   (rData & CLK_LOCK) )
                {
                                if(count == 10000)
                                {
                                                error++;
                                                break;
                                }
                                count++;
                                rData = XDp_ReadReg(CLK_WIZ_BASE_RX, 0x04);
                }
                return error;
}
#endif
#endif
void ComputeMandD(XDp *InstancePtr, u32 VidFreq)
{
//	XDp_TxLinkConfig *LinkConfig = &InstancePtr->TxInstance.LinkConfig;

	u32 RefFreq;
	u32 m, d, Div, Freq, Diff, Fvco;
	u32 Minerr = 10000;
	u32 MVal = 0;
	u32 DVal = 0;
	u32 DivVal = 0;
	u32 rData;

	RefFreq = 100000;

//	xil_printf ("org values %x\r\n", *(u32 *)(CLK_WIZ_BASE + 0x200));
//	xil_printf ("org values2 %x\r\n", *(u32 *)(CLK_WIZ_BASE + 0x208));

//	xil_printf("RefClk: %d , VidClk: %d \r\n", RefFreq, VidFreq);

	for (m = 20; m <= 64; m++) {
		for (d = 1; d <= 80; d++) {
			Fvco = RefFreq * m / d;

			if ( Fvco >= 600000 && Fvco <= 900000 ) {
				for (Div = 1; Div <= 128; Div++ ) {
					Freq = Fvco/Div;

					if (Freq >= VidFreq) {
						Diff = Freq - VidFreq;
					}
					else {
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
					}
					else if (Diff < Minerr) {
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

//	xil_printf("MVal = %d, DivVal = %d, DVal = %d \r\n", MVal, DivVal, DVal);

	/* Progamming the clocking wizard */
	u32 fail,error,count;
	fail = error = count = 0;

	fail = wait_for_lock();
	if(fail)
	{
		error++;
		xil_printf("\n ERROR: Clock is not locked for default frequency : ");
		xil_printf("0x%x\n\r",
				*(u32 *)(CLK_WIZ_BASE + 0x04)&CLK_LOCK);
	}
	else{
//		xil_printf("\n Clock is locked for default frequency : 0x%x\n\r",
//				*(u32 *)(CLK_WIZ_BASE + 0x04)&CLK_LOCK);
	}

	XDp_WriteReg(CLK_WIZ_BASE, 0x00, 0xA);

	rData = XDp_ReadReg(CLK_WIZ_BASE, 0x04);
	if( rData & CLK_LOCK)
	{
		error++;
		xil_printf("\n ERROR: Clock is locked : 0x%x \t expected 0x00\n\r",
				rData & CLK_LOCK);
	}

	for(count=0; count<2000; count++); /* Wait cycles after SW reset */
	fail = wait_for_lock();
	if(fail)
	{
		error++;
		xil_printf("\n ERROR: Clock is not locked after SW reset : ");
		xil_printf("0x%x \t Expected : 0x1\n\r",
				*(u32 *)(CLK_WIZ_BASE + 0x04)&CLK_LOCK);
	}

	/* Configuring Multiply and Divide values */
	XDp_WriteReg(CLK_WIZ_BASE, 0x200, (MVal<<8)|DVal);

	XDp_WriteReg(CLK_WIZ_BASE, 0x204, 0x00);

	XDp_WriteReg(CLK_WIZ_BASE, 0x208, DivVal);

	/* Load Clock Configuration Register values */
	XDp_WriteReg(CLK_WIZ_BASE, 0x25C, 0x07);

	if(*(u32 *)(CLK_WIZ_BASE + 0x04)&CLK_LOCK)
	rData = XDp_ReadReg(CLK_WIZ_BASE, 0x04);
	if(rData & CLK_LOCK)
	{
		error++;
		xil_printf("\n ERROR: Clock is locked : 0x%x \t expected 0x00\n\r",
				rData & CLK_LOCK);
	}
	/* Clock Configuration Registers are used for dynamic reconfiguration */
	XDp_WriteReg(CLK_WIZ_BASE, 0x25C, 0x02);


	fail = wait_for_lock();
	if(fail)
	{
		error++;
		xil_printf("\n ERROR: Clock is not locked : 0x%x \t Expected : 0x1\n\r",
				*(u32 *)(CLK_WIZ_BASE + 0x04)&CLK_LOCK);
	}
	else{

	}

//	xil_printf ("new values %x\r\n", *(u32 *)(CLK_WIZ_BASE + 0x200));
//	xil_printf ("new values2 %x\r\n", *(u32 *)(CLK_WIZ_BASE + 0x208));

}

#if 1
#if (XPAR_XHDCP_NUM_INSTANCES > 0 \
		&& XPAR_DP_TX_HIER_DP_TX_SUBSYSTEM_0_DP_GT_DATAWIDTH == 2)
void ComputeMandD_txlnk(u32 VidFreq, u16 Link_rate)
{
//	XDp_TxLinkConfig *LinkConfig = &InstancePtr->TxInstance.LinkConfig;

//	u32 RefFreq;
//	u32 Diff, Fvco;
//	u32 Minerr = 10000;
	u32 MVal = 0;
	u32 DVal = 0;
	u32 DivVal = 0;
	u32 rData;

	if (Link_rate == 0x14) {
//		RefFreq = 270000;
		MVal = 0xA;
		DVal = 0x2;
		DivVal = 0xA;
	} else if (Link_rate == 0xA) {
//		RefFreq = 135000;
		MVal = 0xA;
		DVal = 0x2;
		DivVal = 0xA;

	} else {
//		RefFreq = 81000;
		MVal = 0xA;
		DVal = 0x1;
		DivVal = 0x14;

	}


	/* Progamming the clocking wizard */
	u32 fail,error,count;
	fail = error = count = 0;

	fail = wait_for_lock_tx();
	if(fail)
	{
		error++;
	}
	else{
	}

	XDp_WriteReg(CLK_WIZ_BASE_TX, 0x00, 0xA);

	rData = XDp_ReadReg(CLK_WIZ_BASE_TX, 0x04);
	if( rData & CLK_LOCK)
	{
		error++;
//		xil_printf("\n ERROR: Clock is locked : 0x%x \t expected 0x00\n\r",
//				*(u32 *)(CLK_WIZ_BASE_TX + 0x04)&CLK_LOCK);
	}

	for(count=0; count<2000; count++);     /* Wait cycles after SW reset */
	fail = wait_for_lock_tx();
	if(fail)
	{
		error++;
		xil_printf("\n ERROR: Clock is not locked after SW reset : ");
		xil_printf("0x%x \t Expected : 0x1\n\r",
				*(u32 *)(CLK_WIZ_BASE_TX + 0x04)&CLK_LOCK);
	}

	/* Configuring Multiply and Divide values */
	XDp_WriteReg(CLK_WIZ_BASE_TX, 0x200, (MVal<<8)|DVal);

	XDp_WriteReg(CLK_WIZ_BASE_TX, 0x204, 0x00);

	XDp_WriteReg(CLK_WIZ_BASE_TX, 0x208, DivVal);

	/* Load Clock Configuration Register values */
	XDp_WriteReg(CLK_WIZ_BASE_TX, 0x25C, 0x07);

	rData = XDp_ReadReg(CLK_WIZ_BASE_TX, 0x04);
	if(rData & CLK_LOCK)
	{
		error++;
//		xil_printf("\n ERROR: Clock is locked : 0x%x \t expected 0x00\n\r",
//				*(u32 *)(CLK_WIZ_BASE_TX + 0x04)&CLK_LOCK);
	}
	/* Clock Configuration Registers are used for dynamic reconfiguration */
	XDp_WriteReg(CLK_WIZ_BASE_TX, 0x25C, 0x02);

	fail = wait_for_lock_tx();
	if(fail)
	{
		error++;
//		xil_printf("\n ERROR: Clock is not locked : 0x%x \t Expected : 0x1\n\r",
//				*(u32 *)(CLK_WIZ_BASE_TX + 0x04)&CLK_LOCK);
	}
	else{

	}

//	xil_printf ("new values %x\r\n", *(u32 *)(CLK_WIZ_BASE + 0x200));
//	xil_printf ("new values2 %x\r\n", *(u32 *)(CLK_WIZ_BASE + 0x208));

}
#endif

#if (XPAR_XHDCP_NUM_INSTANCES > 0 \
		&& XPAR_DP_RX_HIER_DP_RX_SUBSYSTEM_0_DP_GT_DATAWIDTH == 2)

void ComputeMandD_rxlnk(u32 VidFreq, u16 Link_rate)
{
//	XDp_TxLinkConfig *LinkConfig = &InstancePtr->TxInstance.LinkConfig;

//	u32 RefFreq;
//	u32 m, d, Div, Freq, Diff, Fvco;
//	u32 Minerr = 10000;
	u32 MVal = 0;
	u32 DVal = 0;
	u32 DivVal = 0;
	u32 rData;

//	RefFreq = 100000;
	if (Link_rate == 0x14) {
//		RefFreq = 270000;
		MVal = 0xA;
		DVal = 0x2;
		DivVal = 0xA;
//		xil_printf ("14\r\n");
	} else if (Link_rate == 0xA) {
//		RefFreq = 135000;
		MVal = 0xA;
		DVal = 0x2;
		DivVal = 0xA;

//		xil_printf ("10\r\n");
	} else {
//		RefFreq = 81000;
		MVal = 0xA;
		DVal = 0x1;
		DivVal = 0x14;

//		xil_printf ("6\r\n");
	}

//	xil_printf ("org values %x\r\n", *(u32 *)(CLK_WIZ_BASE + 0x200));
//	xil_printf ("org values2 %x\r\n", *(u32 *)(CLK_WIZ_BASE + 0x208));

//	xil_printf("RX RefClk: %d , VidClk: %d \r\n", RefFreq, VidFreq);

//	for (m = 20; m <= 64; m++) {
//		for (d = 1; d <= 80; d++) {
//			Fvco = RefFreq * m / d;
//
//			if ( Fvco >= 600000 && Fvco <= 900000 ) {
//				for (Div = 1; Div <= 128; Div++ ) {
//					Freq = Fvco/Div;
//
//					if (Freq >= VidFreq) {
//						Diff = Freq - VidFreq;
//					}
//					else {
//						Diff = VidFreq - Freq;
//					}
//
//					if (Diff == 0) {
//						MVal = m;
//						DVal = d;
//						DivVal = Div;
//						m = 257;
//						d = 257;
//						Div = 257;
//						Minerr = 0;
//					}
//					else if (Diff < Minerr) {
//						Minerr = Diff;
//						MVal = m;
//						DVal = d;
//						DivVal = Div;
//
//						if (Minerr < 100) {
//							m = 257;
//							d = 257;
//							Div = 257;
//						}
//					}
//				}
//			}
//		}
//	}

//	xil_printf("MVal = %d, DivVal = %d, DVal = %d \r\n", MVal, DivVal, DVal);

	/* Progamming the clocking wizard */
	u32 fail,error,count;
	fail = error = count = 0;

	fail = wait_for_lock_rx();
	if(fail)
	{
		error++;
	}
	else{
	}

	XDp_WriteReg(CLK_WIZ_BASE_RX, 0x00, 0x0A);
	rData = XDp_ReadReg(CLK_WIZ_BASE_RX, 0x04);
	if( rData & CLK_LOCK)
	{
		error++;
//		xil_printf("\n ERROR: Clock is locked : 0x%x \t expected 0x00\n\r",
//				*(u32 *)(CLK_WIZ_BASE_RX + 0x04)&CLK_LOCK);
	}

//	for(count=0; count<2000; count++);    /* Wait cycles after SW reset */
	fail = wait_for_lock_rx();
	if(fail)
	{
		error++;
	}

	/* Configuring Multiply and Divide values */
	XDp_WriteReg(CLK_WIZ_BASE_RX, 0x200, (MVal<<8)|DVal);
	XDp_WriteReg(CLK_WIZ_BASE_RX, 0x204, 0x00);

	XDp_WriteReg(CLK_WIZ_BASE_RX, 0x208, DivVal);

	/* Load Clock Configuration Register values */
	XDp_WriteReg(CLK_WIZ_BASE_RX, 0x25C, 0x07);
//xil_printf ("%d %d %d\r\n", MVal, DVal, DivVal);

	rData = XDp_ReadReg(CLK_WIZ_BASE_RX, 0x04);
	if( rData & CLK_LOCK)
	{
		error++;
//		xil_printf("\n ERROR: Clock is locked : 0x%x \t expected 0x00\n\r",
//				*(u32 *)(CLK_WIZ_BASE_RX + 0x04)&CLK_LOCK);
	}
	/* Clock Configuration Registers are used for dynamic reconfiguration */
	XDp_WriteReg(CLK_WIZ_BASE_RX, 0x25C, 0x02);

	fail = wait_for_lock_rx();
	if(fail)
	{
		error++;
//		xil_printf("\n ERROR: Clock is not locked : 0x%x \t Expected : 0x1\n\r",
//				*(u32 *)(CLK_WIZ_BASE_RX + 0x04)&CLK_LOCK);
	}
	else{
	}

//	xil_printf ("new values %x\r\n", *(u32 *)(CLK_WIZ_BASE + 0x200));
//	xil_printf ("new values2 %x\r\n", *(u32 *)(CLK_WIZ_BASE + 0x208));

}
#endif
#endif
