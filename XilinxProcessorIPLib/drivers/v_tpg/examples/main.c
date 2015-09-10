/*******************************************************************************
 *
 * Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
*******************************************************************************/
/*****************************************************************************/
/**
*
* @file main.c
*
* This file demonstrates the example usage of TPG IP available in catalogue
* Please refer v_tpg example design guide for details on HW setup
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  vyc   09/11/15   Initial Release

* </pre>
*
******************************************************************************/

#include "xparameters.h"
#include "microblaze_sleep.h"
#include "xv_tpg.h"
#include "xvtc.h"
#include "xvidc.h"

XV_tpg_Config		*tpg_Config;
XV_tpg				tpg;

XV_tpg_Config		*tpg1_Config;
XV_tpg				tpg1;

XVtc				vtc;
XVtc_Config			*vtc_Config;
XVtc_Timing			vtc_timing;

u32 volatile		*gpio_hlsIpReset;
u32 volatile		*gpio_videoLockMonitor;

#define VideoClockGen_WriteReg(RegOffset, Data) \
    Xil_Out32((XPAR_VIDEO_CLK_BASEADDR) + (RegOffset), (u32)(Data))
#define VideoClockGen_ReadReg(RegOffset) \
    Xil_In32((XPAR_VIDEO_CLK_BASEADDR) + (RegOffset))

int driverInit()
{
	int status;

	vtc_Config = XVtc_LookupConfig(XPAR_V_TC_0_DEVICE_ID);
	if(vtc_Config == NULL)
	{
		xil_printf("ERR:: VTC device not found\r\n");
		return(XST_DEVICE_NOT_FOUND);
	}
	status = XVtc_CfgInitialize(&vtc, vtc_Config, vtc_Config->BaseAddress);
	if(status != XST_SUCCESS)
	{
		xil_printf("ERR:: VTC Initialization failed %d\r\n", status);
		return(XST_FAILURE);
	}

	tpg_Config = XV_tpg_LookupConfig(XPAR_V_TPG_0_DEVICE_ID);
	if(tpg_Config == NULL)
	{
		xil_printf("ERR:: TPG device not found\r\n");
		return(XST_DEVICE_NOT_FOUND);
	}
	status = XV_tpg_CfgInitialize(&tpg, tpg_Config, tpg_Config->BaseAddress);
	if(status != XST_SUCCESS)
	{
		xil_printf("ERR:: TPG Initialization failed %d\r\n", status);
		return(XST_FAILURE);
	}

	tpg1_Config = XV_tpg_LookupConfig(XPAR_V_TPG_1_DEVICE_ID);
	if(tpg1_Config == NULL)
	{
		xil_printf("ERR:: TPG device not found\r\n");
		return(XST_DEVICE_NOT_FOUND);
	}
	status = XV_tpg_CfgInitialize(&tpg1, tpg1_Config, tpg1_Config->BaseAddress);
	if(status != XST_SUCCESS)
	{
		xil_printf("ERR:: TPG Initialization failed %d\r\n", status);
		return(XST_FAILURE);
	}

	return(XST_SUCCESS);
}

void videoIpConfig(XVidC_VideoMode videoMode)
{
	XVidC_VideoTiming const *timing = XVidC_GetTimingInfo(videoMode);

	XV_tpg_Set_height(&tpg, timing->VActive);
	XV_tpg_Set_width(&tpg, timing->HActive);
	XV_tpg_Set_colorFormat(&tpg, 0);
	XV_tpg_Set_bckgndId(&tpg, XTPG_BKGND_COLOR_BARS);
	XV_tpg_Set_ovrlayId(&tpg, 0);
	XV_tpg_WriteReg(tpg_Config->BaseAddress, XV_TPG_CTRL_ADDR_AP_CTRL, 0x81);

	XV_tpg_Set_height(&tpg1, timing->VActive);
	XV_tpg_Set_width(&tpg1, timing->HActive);
	XV_tpg_Set_colorFormat(&tpg1, 0);
	XV_tpg_Set_bckgndId(&tpg1, XTPG_BKGND_COLOR_BARS);
	XV_tpg_Set_ovrlayId(&tpg1, 0);
	XV_tpg_Set_enableInput(&tpg1, 1);
	XV_tpg_Set_passthruStartX(&tpg1, 0);
	XV_tpg_Set_passthruStartY(&tpg1, 0);
	XV_tpg_Set_passthruEndX(&tpg1, timing->HActive);
	XV_tpg_Set_passthruEndY(&tpg1, timing->VActive);
	XV_tpg_WriteReg(tpg1_Config->BaseAddress, XV_TPG_CTRL_ADDR_AP_CTRL, 0x81);

	vtc_timing.HActiveVideo  = timing->HActive/2;
	vtc_timing.HFrontPorch   = timing->HFrontPorch/2;
	vtc_timing.HSyncWidth    = timing->HSyncWidth/2;
	vtc_timing.HBackPorch    = timing->HBackPorch/2;
	vtc_timing.HSyncPolarity = timing->HSyncPolarity;
	vtc_timing.VActiveVideo  = timing->VActive;
	vtc_timing.V0FrontPorch  = timing->F0PVFrontPorch;
	vtc_timing.V0SyncWidth   = timing->F0PVSyncWidth;
	vtc_timing.V0BackPorch   = timing->F0PVBackPorch;
	vtc_timing.VSyncPolarity = timing->VSyncPolarity;
	XVtc_SetGeneratorTiming(&vtc, &vtc_timing);
	XVtc_Enable(&vtc);
	XVtc_EnableGenerator(&vtc);
	XVtc_RegUpdateEnable(&vtc);
}

int videoClockConfig(XVidC_VideoMode videoMode)
{
	u32 DIVCLK_DIVIDE = 4;
	u32 CLKFBOUT_MULT = 37;
	u32 CLKFBOUT_FRAC = 125;
	u32 CLKOUT0_DIVIDE;
	u32 CLKOUT0_FRAC;
	u32 clock_config_reg_0;
	u32 clock_config_reg_2;
	u32 timeout;
	u32 lock;


	if(videoMode == XVIDC_VM_1080_60_P)
	{
		CLKOUT0_FRAC = 500;
		CLKOUT0_DIVIDE = 12;
	}
	else if(videoMode == XVIDC_VM_UHD_30_P)
	{
		CLKOUT0_FRAC = 250;
		CLKOUT0_DIVIDE = 6;
	}
	else if(videoMode == XVIDC_VM_UHD_60_P)
	{
		CLKOUT0_FRAC = 125;
		CLKOUT0_DIVIDE = 3;
	}
	else
	{
		print("ERR:: Invalid video mode\r\n");
		return(XST_FAILURE);
	}

	clock_config_reg_0 = (1<<26) | (CLKFBOUT_FRAC<<16) | (CLKFBOUT_MULT<<8) | DIVCLK_DIVIDE;
	clock_config_reg_2 = (1<<18) | (CLKOUT0_FRAC<<8) | CLKOUT0_DIVIDE;

	VideoClockGen_WriteReg(0x200, clock_config_reg_0);
	VideoClockGen_WriteReg(0x208, clock_config_reg_2);

	MB_Sleep(100);

	lock = VideoClockGen_ReadReg(0x4) & 0x1;
	if(!lock) //check for lock
	{
		//Video Clock Generator not locked
		VideoClockGen_WriteReg(0x25C, 0x7);
		VideoClockGen_WriteReg(0x25C, 0x2);
		timeout = 100000;
		while(!lock)
		{
			lock = VideoClockGen_ReadReg(0x4) & 0x1;
			--timeout;
			if(!timeout)
			{
				print("ERR:: Video Clock Generator failed lock\r\n");
				return(XST_FAILURE);
			}
		}
	}
	print("Video Clock Generator locked\r\n");

	return(XST_SUCCESS);

}

void resetIp(void)
{
	*gpio_hlsIpReset = 0; //reset IPs

	MB_Sleep(100);

	*gpio_hlsIpReset = 1; // release reset

	MB_Sleep(100);

}

int main()
{
	int status;

	print("Start test\r\n");

	gpio_hlsIpReset = (u32*)XPAR_HLS_IP_RESET_BASEADDR;
	gpio_videoLockMonitor = (u32*)XPAR_VIDEO_LOCK_MONITOR_BASEADDR;

	status = driverInit();
	if(status != XST_SUCCESS)
	{
		return(XST_FAILURE);
	}

	resetIp();

	if(*gpio_videoLockMonitor)
	{
		print("ERR:: Video should not be locked\r\n");
		return(XST_FAILURE);
	}


	videoClockConfig(XVIDC_VM_1080_60_P);
	videoIpConfig(XVIDC_VM_1080_60_P);

	MB_Sleep(100);

	if(!gpio_videoLockMonitor)
	{
		print("ERR:: Video Lock failed for 1080P60\r\n");
		return(XST_FAILURE);
	}
	else
	{
		print("1080P60 passed\r\n");
	}

	resetIp();

	videoClockConfig(XVIDC_VM_UHD_30_P);
	videoIpConfig(XVIDC_VM_UHD_30_P);

	MB_Sleep(100);

	if(!gpio_videoLockMonitor)
	{
		print("ERR:: Video Lock failed for 4KP30\r\n");
		return(XST_FAILURE);
	}
	else
	{
		print("4KP30 passed\r\n");
	}

	resetIp();

	videoClockConfig(XVIDC_VM_UHD_60_P);
	videoIpConfig(XVIDC_VM_UHD_60_P);

	MB_Sleep(100);

	if(!gpio_videoLockMonitor)
	{
		print("ERR:: Video Lock failed for 4KP60\r\n");
		return(XST_FAILURE);
	}
	else
	{
		print("4KP60 passed\r\n");
	}

	print("TEST PASS\r\n");



	return 0;
}
