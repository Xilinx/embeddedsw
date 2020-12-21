#include "xil_types.h"
#include "xparameters.h"
#include "xdptxss.h"

#include "xclk_wiz.h"

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

#ifdef versal
extern XClk_Wiz ClkWiz_Dynamic;
extern XClk_Wiz_Config *CfgPtr_Dynamic;
#endif
/************************** Function Prototypes ******************************/
void ComputeMandD(u32 VidFreq);
void Gen_vid_clk(XDp *InstancePtr, u8 Stream);

/*****************************************************************************/

#ifndef versal
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
int wait_for_lock(void)
{
	u32 count, error;
	count = error = 0;
	volatile u32 rdata=0;
	rdata = XClk_Wiz_ReadReg(CLK_WIZ_BASE, 0x04) & 1;

	while(!rdata){
		if(count == 10000){
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
void ComputeMandD(u32 VidFreq){

	u32 RefFreq;
	u32 m, d, Div, Freq, Diff, Fvco;
	u32 Minerr = 1000;
	u32 MVal = 0;
	u32 DVal = 0;
	u32 DivVal = 0;
	u32 rdata=0;

	RefFreq = 100000;

	for (m = M_MIN; m <= M_MAX; m++) {
		for (d = D_MIN; d <= D_MAX; d++) {
			Fvco = RefFreq * m / d;

			if ( Fvco >= VCO_MIN && Fvco <= VCO_MAX ) {
				for (Div = DIV_MIN; Div <= DIV_MAX; Div++ ) {
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

	/* Progamming the clocking wizard */
	u32 fail,error,count;
	fail = error = count = 0;

	fail = wait_for_lock();
	if(fail)
	{
		error++;

		xil_printf(
			"\n ERROR: Clock is not locked for default frequency : 0x%x\r\n",
			XClk_Wiz_ReadReg(CLK_WIZ_BASE, 0x04) & CLK_LOCK);
	}


/* SW reset applied */
	XClk_Wiz_WriteReg(CLK_WIZ_BASE, 0x0, 0xA);

	for(count=0; count<2000; count++);      /* Wait cycles after SW reset */
	fail = wait_for_lock();
	if(fail)
	{
		error++;
	xil_printf(
			"\n ERROR: Clock is not locked after SW reset : 0x%x \t Expected : 0x1\r\n",
			XClk_Wiz_ReadReg(CLK_WIZ_BASE, 0x04) & CLK_LOCK);
	}

	/* Configuring Multiply and Divide values */
	XClk_Wiz_WriteReg(CLK_WIZ_BASE, 0x200, (MVal<<8)|DVal);
	XClk_Wiz_WriteReg(CLK_WIZ_BASE, 0x204, 0);

	XClk_Wiz_WriteReg(CLK_WIZ_BASE, 0x208, DivVal);

	/* Load Clock Configuration Register values */
	XClk_Wiz_WriteReg(CLK_WIZ_BASE, 0x25C, 0x07);

	rdata = XClk_Wiz_ReadReg(CLK_WIZ_BASE, 0x04) & CLK_LOCK;

	if(rdata){
		error++;
		xil_printf("\n ERROR: Clock is locked : 0x%x \t expected 0x00\r\n",
				XClk_Wiz_ReadReg(CLK_WIZ_BASE, 0x04) & CLK_LOCK);
	}
	/* Clock Configuration Registers are used for dynamic reconfiguration */
	XClk_Wiz_WriteReg(CLK_WIZ_BASE, 0x25C, 0x02);

	fail = wait_for_lock();
	if(fail)
	{
		error++;
				xil_printf("\n ERROR: Clock is not locked : 0x%x \t Expected : 0x1\r\n",
				XClk_Wiz_ReadReg(CLK_WIZ_BASE, 0x04) & CLK_LOCK);
	}
}
#endif

void Gen_vid_clk(XDp *InstancePtr, u8 Stream)
{
	u32 Count = 0;
	XDp_TxMainStreamAttributes *MsaConfig =
			&InstancePtr->TxInstance.MsaConfig[Stream - 1];
#ifndef versal
	ComputeMandD(((MsaConfig->PixelClockHz/1000)/MsaConfig->UserPixelWidth) );
#else
	XClk_Wiz_WriteReg(ClkWiz_Dynamic.Config.BaseAddr, XCLK_WIZ_REG25_OFFSET, 0x0);
	XClk_Wiz_SetRateHz(&ClkWiz_Dynamic, ((MsaConfig->PixelClockHz)/MsaConfig->UserPixelWidth));
	XClk_Wiz_WriteReg(ClkWiz_Dynamic.Config.BaseAddr, XCLK_WIZ_RECONFIG_OFFSET, 0x3);
	while(!((XClk_Wiz_ReadReg(ClkWiz_Dynamic.Config.BaseAddr, 0x4)) & CLK_LOCK)) {
		if(Count == 10000) {
				break;
		}
		usleep(100);
		Count++;
	}

	if (Count == 10000) {
		xil_printf ("Clk_wizard failed to lock\r\n");
	}

#endif
}
