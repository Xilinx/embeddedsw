/******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include <stdio.h>
#include <xil_exception.h>
#include <xil_cache.h>
#include <xil_printf.h>
#include "xil_io.h"
#include "xil_types.h"

#include "mmi_dc_live_test.h"
#include "mmi_dp_init.h"

#define CLK_LOCK			1

RunConfig RunCfgPtr;
XDcSub DcSub;
XDc Dc;
XVtc Vtc0;
XVtc Vtc1;
XAvTpg AvTpg0;
XAvTpg AvTpg1;
XAvTpg_VideoTiming TpgTiming;
XMmiDp DpPsuPtr;

/* SDTV Coeffs */
u32 CSCCoeff_RGB[] = { 0x1000, 0x0000, 0x0000, 0x0000, 0x1000, 0x0000, 0x0000, 0x0000, 0x1000};
u32 CSCOffset_RGB[] =  { 0x0000, 0x0000, 0x0000 };

/* Video Timings */
XDc_VideoTiming VidTiming_640_480 = { .HTotal = 800, .HSWidth = 96, .HRes = 640, .HStart = 144, .VTotal = 525, .VSWidth = 2, .VRes = 480, .VStart = 35};
XAvTpg_VideoTiming TpgTiming_640_480 = { .HTotal = 800, .HSWidth = 96, .HRes = 640, .HStart = 144, .VTotal = 525, .VSWidth = 2, .VRes = 480, .VStart = 35, .HBackPorch = 48, .HFrontPorch = 16, .VBackPorch = 33, .VFrontPorch = 10, .HRes_hack = 320, .HSWidth_hack = 48};

void EnableAvTpg()
{
	XAvTpg_WriteReg(XPG_AXI_GPIO1, 0x0, 0x00010001);
}

void EnableStream0()
{
	XAvTpg_WriteReg(AV_PATGEN_BASE, 0x000, 0x1);
	XAvTpg_WriteReg(AV_PATGEN_BASE, 0x400, 0x2);
	XVtc_WriteReg(V_TC_BASE, 0x0000, 0x03f7ef06);
}

void EnableStream1()
{

	XAvTpg_WriteReg(AV_PATGEN_BASE_1, 0x000, 0x1);
	XAvTpg_WriteReg(AV_PATGEN_BASE_1, 0x400, 0x2);
	XVtc_WriteReg(V_TC_BASE_1, 0x0000, 0x03f7ef06);
}

void InitLiveMode()
{
	XDc_WriteReg(XPG_AXI_GPIO1, 0x0, 0x00010000);
}

u32 XClk_WaitForLock(XClk_Wiz_Config *CfgPtr)
{
	u32 Count = 0;

	while (!XClk_Wiz_ReadReg(CfgPtr->BaseAddr, XCLK_WIZ_REG4_OFFSET) & CLK_LOCK) {
		if (Count == 10000) {
			return XST_FAILURE;
		}
		usleep(100);
		Count++;
	}
	return XST_SUCCESS;
}

u32 InitClkWiz()
{
	XClk_Wiz InstancePtr;
	XClk_Wiz_Config *CfgPtr;
	u32 Status = XST_SUCCESS;
	u32 Reg;
	u64 Rate;

	CfgPtr = XClk_Wiz_LookupConfig(CLK_WIZ_BASEADDR);
	if (!CfgPtr) {
		xil_printf("FAILED to lookup Clk Wizard IP\n");
		return XST_FAILURE;
	}

	Status = XClk_Wiz_CfgInitialize(&InstancePtr, CfgPtr, CfgPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("FAILED to cfg initialize Clk Wizard IP\n");
		return XST_FAILURE;
	}

	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, XCLK_WIZ_REG25_OFFSET, 0);

	Status = XClk_Wiz_SetRateHz(&InstancePtr, 25200000);
	if (Status != XST_SUCCESS) {
		xil_printf("FAILED to set rate of Clk Wizard IP\n");
		return XST_FAILURE;
	}

	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, XCLK_WIZ_RECONFIG_OFFSET,
						(XCLK_WIZ_RECONFIG_LOAD | XCLK_WIZ_RECONFIG_SADDR));

	Status = XClk_WaitForLock(CfgPtr);
	if (Status != XST_SUCCESS) {
		Reg = XClk_Wiz_ReadReg(CfgPtr->BaseAddr, XCLK_WIZ_REG4_OFFSET) & CLK_LOCK;
		xil_printf("\n ERROR: Clock is not locked : 0x%x \t Expected "\
				": 0x1\n\r", Reg);
	}
	XClk_Wiz_GetRate(&InstancePtr, 0, &Rate);
	Rate = Rate / XCLK_MHZ;
	xil_printf("\nCalculated Rate is %ld MHz and Expected is 25.2 MHz \n",  Rate);

	return Status;
}

/*****************************************************************************/
/**
*
* The purpose of this function is to initialize the VTC Subsystem
*
* @param        RunCfgPtr is a pointer to the application configuration structure.
*
* @return       None.
*
* @note         None.
*
*****************************************************************************/
u32 InitVtcSubsystem(RunConfig *RunCfgPtr, XVtc *Vtc, UINTPTR Addr)
{
	XVtc_Config CfgPtr;
	XVtc_Polarity Polarity;
	XVtc_SourceSelect SourceSelect;
	XVtc_Timing VideoTiming;
	XVtc_Signal Signal;
	XVtc_HoriOffsets Hoff;

	u32 PixPerClk = RunCfgPtr->PPC;

	XVtc_CfgInitialize(Vtc, &CfgPtr, Addr);

	/* Disable Generator */
	XVtc_Reset(Vtc);
	XVtc_DisableGenerator(Vtc);
	XVtc_Disable(Vtc);

	/* Set up source select */
	memset((void *)&SourceSelect, 0, sizeof(SourceSelect));

	/* 1 = Generator registers, 0 = Detector registers */
	SourceSelect.VChromaSrc = 1;
	SourceSelect.VActiveSrc = 1;
	SourceSelect.VBackPorchSrc = 1;
	SourceSelect.VSyncSrc = 1;
	SourceSelect.VFrontPorchSrc = 1;
	SourceSelect.VTotalSrc = 1;
	SourceSelect.HActiveSrc = 1;
	SourceSelect.HBackPorchSrc = 1;
	SourceSelect.HSyncSrc = 1;
	SourceSelect.HFrontPorchSrc = 1;
	SourceSelect.HTotalSrc = 1;

	XVtc_SetSource(Vtc, &SourceSelect);
	XVtc_ConvVideoMode2Timing(Vtc, XVTC_VMODE_VGA, &VideoTiming); //640x480

	if (PixPerClk == RunCfgPtr->PPC) {
		VideoTiming.HActiveVideo = VideoTiming.HActiveVideo / RunCfgPtr->PPC;
		VideoTiming.HFrontPorch = VideoTiming.HFrontPorch / RunCfgPtr->PPC;
		VideoTiming.HBackPorch = VideoTiming.HBackPorch / RunCfgPtr->PPC;
		VideoTiming.HSyncWidth = VideoTiming.HSyncWidth / RunCfgPtr->PPC;
	}

	XVtc_ConvTiming2Signal(Vtc, &VideoTiming, &Signal, &Hoff, &Polarity);

	/* Setup VTC Polarity Config */
	memset((void *)&Polarity, 0, sizeof(Polarity));
	Polarity.ActiveChromaPol = 1;
	Polarity.ActiveVideoPol  = 1;
	Polarity.VBlankPol       = 1;
	Polarity.VSyncPol        = 1;
	Polarity.HBlankPol       = 1;
	Polarity.HSyncPol        = 1;

	XVtc_SetPolarity(Vtc, &Polarity);
	XVtc_SetGenerator(Vtc, &Signal);
	XVtc_SetGeneratorHoriOffset(Vtc, &Hoff);

	XVtc_Enable(Vtc);
	XVtc_EnableGenerator(Vtc);
	XVtc_RegUpdateEnable(Vtc);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* The purpose of this function is to initialize the AVTPG Subsystem
*
* @param        RunCfgPtr is a pointer to the application configuration structure.
*
* @return       None.
*
* @note         None.
*
*****************************************************************************/
u32 InitAvTpgSubsystem(XAvTpg *AvTpgPtr)
{
	XAvTpg_SetVideoTiming(AvTpgPtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* The purpose of this function is to initialize the DC Subsystem (XDcDma,
* XDc)
*
* @param        RunCfgPtr is a pointer to the application configuration structure.
*
* @return       None.
*
* @note         None.
*
*****************************************************************************/
u32 InitDcSubsystem(RunConfig *RunCfgPtr)
{

	XDcSub *DcSubPtr = RunCfgPtr->DcSubPtr;
	XDc *DcPtr = DcSubPtr->DcPtr;

	/* Dc Subsystem Initialization */
	XDcSub_Initialize(DcSubPtr);
	/* Configure Video resolution */
	XDc_SetVideoTiming(DcPtr);
	/* Program clock source selection */
	XDc_VidClkSelect(DcPtr);
	/* DC Soft Reset */
	XDc_VideoSoftReset(DcPtr);
	/* DC Video Frame Switch rate */
	XDc_SetVidFrameSwitch(DcPtr);
	/* Configure all registers for Video Pipeline */
	XDcSub_ConfigureDcVideo(DcPtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* The purpose of this function is to configure all the platform blocks in use
*
* @param        RunCfgPtr is a pointer to the application configuration structure.
*
* @return       XST_SUCCESS if successful, else XST_FAILURE.
*
* @note         None.
*
*****************************************************************************/
u32 InitPlatform(RunConfig *RunCfgPtr)
{
	u32 Status = XST_SUCCESS;

	Status = InitClkWiz();
	if (Status != XST_SUCCESS) {
		xil_printf("FAILED to Initialize Clk Wizard IP\n");
		return Status;
	}

	/* Initialize DcSubsystem */
	Status = InitDcSubsystem(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("FAILED to Initialize DC Subsystem Ip\n");
		return Status;
	}

	xil_printf("DcSubsystem Initialization complete\n");
	InitLiveMode();

	InitAvTpgSubsystem(RunCfgPtr->AvTpgPtr0);
	InitVtcSubsystem(RunCfgPtr, RunCfgPtr->VtcPtr0, V_TC_BASE);

	InitAvTpgSubsystem(RunCfgPtr->AvTpgPtr1);
	InitVtcSubsystem(RunCfgPtr, RunCfgPtr->VtcPtr1, V_TC_BASE_1);

	EnableStream0();
	EnableStream1();
	EnableAvTpg();

	xil_printf("AvTpg Initialization complete\n");

	/* Initialize DpSubsystem */
	Status = InitDpPsuSubsystem(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("DpPsu14 Subsystem Initialization failed\n");
		return Status;
	}

	return Status;
}

void InitDcSubPtr(RunConfig *RunCfgPtr)
{
	XDc *DcPtr = RunCfgPtr->DcSubPtr->DcPtr;
	DcPtr->Config.BaseAddr = DC_BASEADDR;
	DcPtr->VideoTiming = VidTiming_640_480;

	XDcSub_SetVidInterfaceMode(RunCfgPtr->DcSubPtr, XDC_VID_FUNCTIONAL);
	XDcSub_SetVidStreamSrc(RunCfgPtr->DcSubPtr, XDC_VIDSTREAM1_LIVE, XDC_VIDSTREAM2_LIVE);
	XDcSub_SetInputAudioSelect(RunCfgPtr->DcSubPtr, XDC_AUDSTREAM_NONE);
	XDcSub_SetInputLiveStreamFormat(RunCfgPtr->DcSubPtr, RunCfgPtr->Stream1Format,
					RunCfgPtr->Stream2Format);
	XDcSub_SetOutputVideoFormat(RunCfgPtr->DcSubPtr, RunCfgPtr->OutStreamFormat);
	XDcSub_SetInputStreamLayerControl(RunCfgPtr->DcSubPtr, 0, 0);
	XDcSub_SetGlobalAlpha(RunCfgPtr->DcSubPtr, 0x1, 0xA5);
	XDcSub_SetStreamPixelScaling(RunCfgPtr->DcSubPtr, NULL, NULL);

	XDcSub_SetInputStreamCSC(RunCfgPtr->DcSubPtr, CSCCoeff_RGB, CSCOffset_RGB, CSCCoeff_RGB,
				 CSCOffset_RGB);
	XDcSub_SetOutputStreamCSC(RunCfgPtr->DcSubPtr, CSCCoeff_RGB, CSCOffset_RGB);
	XDcSub_SetAudioVideoClkSrc(RunCfgPtr->DcSubPtr);
	DcPtr->VideoClk = 0;
	DcPtr->AudioClk = 0;
	XDcSub_VidClkSelect(RunCfgPtr->DcSubPtr, 0, 0);
	XDcSub_SetVidFrameSwitch(RunCfgPtr->DcSubPtr, 0x3F);
	XDcSub_SetNonLiveLatency(RunCfgPtr->DcSubPtr, 0x00120138);

}

/*****************************************************************************/
/**
*
* The purpose of this function is to initialize the application configuration.
*
* @param        RunCfgPtr is a pointer to the application configuration structure.
*
* @return       None.
*
* @note         None.
*
*****************************************************************************/
u32 InitRunConfig(RunConfig *RunCfgPtr)
{

	u32 Status = XST_SUCCESS;

	/* All user configurable params */
	RunCfgPtr->DcSubPtr = &DcSub;
	RunCfgPtr->DcSubPtr->DcPtr = &Dc;
	RunCfgPtr->DpPsuPtr = &DpPsuPtr;
	RunCfgPtr->VtcPtr0 = &Vtc0;
	RunCfgPtr->VtcPtr1 = &Vtc1;
	RunCfgPtr->AvTpgPtr0 = &AvTpg0;
	RunCfgPtr->AvTpgPtr1 = &AvTpg1;

	RunCfgPtr->DcBaseAddr = DC_BASEADDR;
	RunCfgPtr->Width = 640;
	RunCfgPtr->Height = 480;
	RunCfgPtr->PPC = 0x2;

	RunCfgPtr->Stream1Format = RGB_8BPC;
	RunCfgPtr->Stream1Bpc = 3;
	RunCfgPtr->Stream2Format = RGB_8BPC;
	RunCfgPtr->Stream2Bpc = 3;
	RunCfgPtr->OutStreamFormat = RGB_8BPC;

	XAvTpg *AvTpgPtr0 = RunCfgPtr->AvTpgPtr0;
	AvTpgPtr0->Config.BaseAddr = AV_PATGEN_BASE;
	AvTpgPtr0->Format = 0;
	AvTpgPtr0->DualPixelMode = 1;
	AvTpgPtr0->Bpc = 1;
	AvTpgPtr0->Pattern = 0x1;
	AvTpgPtr0->VideoTiming = &TpgTiming_640_480;

	XAvTpg *AvTpgPtr1 = RunCfgPtr->AvTpgPtr1;
	AvTpgPtr1->Config.BaseAddr = AV_PATGEN_BASE_1;
	AvTpgPtr1->Format = 0;
	AvTpgPtr1->DualPixelMode = 1;
	AvTpgPtr1->Bpc = 1;
	AvTpgPtr1->Pattern = 0x3;
	AvTpgPtr1->VideoTiming = &TpgTiming_640_480;

	InitDcSubPtr(RunCfgPtr);
	return Status;
}

/*****************************************************************************/
/**
*
* The purpose of this function is to illustrate how to setup the platform
*
* @param        RunCfgPtr is a pointer to the application configuration structure.
*
* @return       XST_SUCCESS if successful, else XST_FAILURE.
*
* @note         None.
*
*****************************************************************************/
u32 mmi_dc_live_test(RunConfig *RunCfgPtr)
{

	u32 Status = XST_SUCCESS;

	/* Initialize the application configuration */
	Status = InitRunConfig(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("FAILED to get test configuration\n");
		return Status;
	}

	Status = InitPlatform(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("FAILED to Initialize Platform\n");
		return Status;
	}

	return Status;

}

int main()
{
	u32 Status = 0;

	Xil_DCacheDisable();
	Xil_ICacheDisable();

	Status = mmi_dc_live_test(&RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("MMI_DC_LIVE_TEST failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran MMI_DC Live Mode Example\r\n");

	/* Do not exit application,
	   required for monitor display */
	while (1);

	return 0;

}
