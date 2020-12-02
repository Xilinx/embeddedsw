/*******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
* 1.10  rco   10/05/15   Update to support multiple PPC configurations
* 8.0   ms    01/23/17   Modified xil_printf statement in main function to
*                        ensure that "Successfully ran" and "Failed" strings
*                        are available in all examples. This is a fix for
*                        CR-965028.
* </pre>
*
******************************************************************************/

#include "xparameters.h"
#include "sleep.h"
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
    Xil_Out32((XPAR_VIDEO_CLK_WIZ_BASEADDR) + (RegOffset), (u32)(Data))
#define VideoClockGen_ReadReg(RegOffset) \
    Xil_In32((XPAR_VIDEO_CLK_WIZ_BASEADDR) + (RegOffset))

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
	u16 PixelsPerClk;

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

	PixelsPerClk = tpg1.Config.PixPerClk;

	vtc_timing.HActiveVideo  = timing->HActive/PixelsPerClk;
	vtc_timing.HFrontPorch   = timing->HFrontPorch/PixelsPerClk;
	vtc_timing.HSyncWidth    = timing->HSyncWidth/PixelsPerClk;
	vtc_timing.HBackPorch    = timing->HBackPorch/PixelsPerClk;
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
	u16 PixelsPerClk, mode_index;

    const int ClkOut_Frac[3][XVIDC_PPC_NUM_SUPPORTED] =
    { {250, 500, 0  , 0}, //1080p
      {125, 250, 500, 0}, //4K30
      {0,   125, 250, 500}  //4K60
    };
    const int ClkOut_Div[3][XVIDC_PPC_NUM_SUPPORTED] =
    { {6, 12, 25, 50}, //1080p
      {3, 6 , 12, 25}, //4K30
      {0, 3 , 6 , 12}  //4K60
    };

    /* Validate TPG Parameters */
    Xil_AssertNonvoid((tpg1.Config.PixPerClk == XVIDC_PPC_1) ||
                      (tpg1.Config.PixPerClk == XVIDC_PPC_2) ||
					  (tpg1.Config.PixPerClk == XVIDC_PPC_4) ||
                      (tpg1.Config.PixPerClk == XVIDC_PPC_8));


    mode_index = ((videoMode ==  XVIDC_VM_1080_60_P) ? 0 :
                  (videoMode ==  XVIDC_VM_UHD_30_P)  ? 1 :
                  (videoMode ==  XVIDC_VM_UHD_60_P)  ? 2 : 3);

    if(mode_index > 2)
    {
      xil_printf("ERR:: Video Mode %s not supported\r\n", XVidC_GetVideoModeStr(videoMode));
      return(XST_FAILURE);
    }

    //map PPC to array index
    PixelsPerClk = ((tpg1.Config.PixPerClk == XVIDC_PPC_8)? 3 : tpg1.Config.PixPerClk>>1);
    CLKOUT0_FRAC   =  ClkOut_Frac[mode_index][PixelsPerClk];
    CLKOUT0_DIVIDE =  ClkOut_Div[mode_index][PixelsPerClk];

	clock_config_reg_0 = (1<<26) | (CLKFBOUT_FRAC<<16) | (CLKFBOUT_MULT<<8) | DIVCLK_DIVIDE;
	clock_config_reg_2 = (1<<18) | (CLKOUT0_FRAC<<8) | CLKOUT0_DIVIDE;

	VideoClockGen_WriteReg(0x200, clock_config_reg_0);
	VideoClockGen_WriteReg(0x208, clock_config_reg_2);

	usleep(300000);

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
				xil_printf("ERR:: Video Clock Generator failed lock\r\n");
				return(XST_FAILURE);
			}
		}
	}
	xil_printf("Video Clock Generator locked\r\n");

	return(XST_SUCCESS);

}

void resetIp(void)
{
	*gpio_hlsIpReset = 0; //reset IPs

	usleep(300000);

	*gpio_hlsIpReset = 1; // release reset

	usleep(300000);

}

int main()
{
	int status;
	XVidC_VideoMode TestMode;

	xil_printf("Start test\r\n");

	gpio_hlsIpReset = (u32*)XPAR_HLS_IP_RESET_BASEADDR;
	gpio_videoLockMonitor = (u32*)XPAR_VIDEO_LOCK_MONITOR_BASEADDR;

	status = driverInit();
	if(status != XST_SUCCESS) {
		return(XST_FAILURE);
	}

	resetIp();

	if(*gpio_videoLockMonitor) {
		xil_printf("ERR:: Video should not be locked\r\n");
		return(XST_FAILURE);
	}


	TestMode = XVIDC_VM_1080_60_P;
	xil_printf("\r\nTest: %s\r\n", XVidC_GetVideoModeStr(TestMode));
	status = videoClockConfig(TestMode);
	if(status != XST_SUCCESS) {
		return(XST_FAILURE);
	}
	videoIpConfig(TestMode);

	usleep(300000);

	if(!(*gpio_videoLockMonitor)) {
		xil_printf("ERR:: Video Lock failed for 1080P60\r\n");
		return(XST_FAILURE);
	}
	else {
		xil_printf("1080P60 passed\r\n");
	}

	resetIp();

	TestMode = XVIDC_VM_UHD_30_P;
	xil_printf("\r\nTest: %s\r\n", XVidC_GetVideoModeStr(TestMode));
	status = videoClockConfig(TestMode);
	if(status != XST_SUCCESS){
		return(XST_FAILURE);
	}
	videoIpConfig(TestMode);

	usleep(300000);

	if(!(*gpio_videoLockMonitor)) {
		xil_printf("ERR:: Video Lock failed for 4KP30\r\n");
		return(XST_FAILURE);
	}
	else {
		xil_printf("4KP30 passed\r\n\r\n");
	}

    /* Run 4k60 Test if supported by HW
     * Check if TPG is configured for 2/4/8 Pixels/Clock
     * Required to support 4K60
     */
    if((tpg1.Config.PixPerClk == XVIDC_PPC_2) ||
       (tpg1.Config.PixPerClk == XVIDC_PPC_4) ||
       (tpg1.Config.PixPerClk == XVIDC_PPC_8)) {

      resetIp();

      TestMode = XVIDC_VM_UHD_60_P;
	  xil_printf("\r\nTest: %s\r\n", XVidC_GetVideoModeStr(TestMode));
	  status = videoClockConfig(TestMode);
	  if(status != XST_SUCCESS) {
		return(XST_FAILURE);
	  }
	  videoIpConfig(TestMode);

	  usleep(300000);

	  if(!(*gpio_videoLockMonitor)) {
		xil_printf("ERR:: Video Lock failed for 4KP60\r\n");
		return(XST_FAILURE);
	  }
	  else {
		xil_printf("4KP60 passed\r\n\r\n");
	  }
    }
	xil_printf("Successfully ran Example\r\n");

	return 0;
}
