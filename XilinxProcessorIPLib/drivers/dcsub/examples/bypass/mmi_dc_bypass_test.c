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

#include "mmi_dc_bypass_test.h"
#include "mmi_dp_init.h"

#define CLK_LOCK			1

RunConfig RunCfgPtr;
XDcSub DcSub;
XDc Dc;
XVtc Vtc0;
XAvTpg AvTpg0;
XAvTpg_VideoTiming TpgTiming;
XMmiDp DpPsuPtr;

/* Video Timings */
XDc_VideoTiming VidTiming_640_480 = { .HTotal = 800, .HSWidth = 96, .HRes = 640, .HStart = 144, .VTotal = 525, .VSWidth = 2, .VRes = 480, .VStart = 35};
XAvTpg_VideoTiming TpgTiming_640_480 = { .HTotal = 800, .HSWidth = 96, .HRes = 640, .HStart = 144, .VTotal = 525, .VSWidth = 2, .VRes = 480, .VStart = 35, .HBackPorch = 48, .HFrontPorch = 16, .VBackPorch = 33, .VFrontPorch = 10, .HRes_hack = 0, .HSWidth_hack = 0};

void EnableAvTpg()
{
	XAvTpg_WriteReg(XPG_AXI_GPIO1, 0x0, 0x00010001);
	XAvTpg_WriteReg(DPDC_ALPHA_BYPASS_EN, 0x8, 0x00000023);
	XAvTpg_WriteReg(DPDC_ALPHA_BYPASS_EN, 0x8, 0x00000013);
}

void EnableStream()
{
	XAvTpg_WriteReg(AV_PATGEN_BASE, 0x000, 0x1);
	XAvTpg_WriteReg(AV_PATGEN_BASE, 0x400, 0x2);
	XVtc_WriteReg(V_TC_BASE, 0x0000, 0x03f7ef06);
}

void InitBypassMode()
{
	XDc_WriteReg(DPDC_ALPHA_BYPASS_EN, 0x0, 0x00010000);
	XDc_WriteReg(XPG_AXI_GPIO1 , 0x0, 0x00010000);
	XDc_WriteReg(DPDC_LV_AVTPG_PM, 0x0, 0x00000000 | (0x0 << 2) | 0x1);
	XDc_WriteReg(DPDC_ALPHA_GPIO, 0x8, 0xa5a5a5a5);

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

	/*
	 * Bypass mode is in quad pixel.
	 * So pl_dc_1x_clk is pixel clock = 25.175 MHz / 4.
	 * But clk wiz output is controlled by pl_dc_2x_clk port.
	 * So program clk wiz to (25.175 / 4) * 2 MHz i.e
	 * 12.5875 MHz.
	 *
	 * These values are taken from Vivado generated drp file for accuracy.
	 */
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x330, 0x1600);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x334, 0x2d2d);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x338, 0xba00);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x33c, 0x3c3c);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x340, 0xa00);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x344, 0x303);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x348, 0xa00);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x34c, 0x303);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x350, 0xa00);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x354, 0x303);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x378, 0x23);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x37c, 0x0);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x380, 0x400);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x384, 0x101);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x390, 0x0);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x394, 0x0);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x398, 0xe80);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x39c, 0x7e58);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x3a0, 0x7fe9);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x3a8, 0x18);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x3c0, 0x1);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x3cc, 0xa00);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x3d0, 0x303);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x3d4, 0xa00);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x3d8, 0x303);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x3dc, 0xa00);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x3e0, 0x303);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x3e4, 0x0);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x3e8, 0x0);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x3f0, 0x18);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x3f8, 0xf00);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x3fc, 0x3);
	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, 0x14, 0x3);

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
u32 InitAvTpgSubsystem(RunConfig *RunCfgPtr, XAvTpg *AvTpgPtr)
{
	XAvTpg_VideoTiming *VideoTiming = AvTpgPtr->VideoTiming;

	VideoTiming->HRes_hack = VideoTiming->HRes/RunCfgPtr->PPC;
	VideoTiming->HSWidth_hack = VideoTiming->HSWidth/RunCfgPtr->PPC;

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

	/* Program clock source selection */
	XDc_VidClkSelect(DcPtr);

	/* DC Video Frame Switch rate */
	XDc_SetVidFrameSwitch(DcPtr);

	/* DC Video Interface */
	XDc_SetVidInterfaceMode(DcPtr);

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

	InitBypassMode();

	InitAvTpgSubsystem(RunCfgPtr, RunCfgPtr->AvTpgPtr0);
	InitVtcSubsystem(RunCfgPtr, RunCfgPtr->VtcPtr0, V_TC_BASE);

	EnableStream();
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

	XDcSub_SetVidInterfaceMode(RunCfgPtr->DcSubPtr, XDC_VID_BYPASS);
	XDcSub_VidClkSelect(RunCfgPtr->DcSubPtr, 0, 0);
	XDcSub_SetVidFrameSwitch(RunCfgPtr->DcSubPtr, 0x3F);

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
	RunCfgPtr->AvTpgPtr0 = &AvTpg0;

	RunCfgPtr->DcBaseAddr = DC_BASEADDR;
	RunCfgPtr->Width = 640;
	RunCfgPtr->Height = 480;
	RunCfgPtr->PPC = 4;

	XAvTpg *AvTpgPtr0 = RunCfgPtr->AvTpgPtr0;
	AvTpgPtr0->Config.BaseAddr = AV_PATGEN_BASE;
	AvTpgPtr0->Format = 0;
	AvTpgPtr0->DualPixelMode = 0;
	AvTpgPtr0->Bpc = 1;
	AvTpgPtr0->Pattern = 0x3;
	AvTpgPtr0->VideoTiming = &TpgTiming_640_480;

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
u32 mmi_dc_mst_test(RunConfig *RunCfgPtr)
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

	xil_printf("------------------------------------------\r\n");
	xil_printf("----- MMI DC Bypass Example - v%d.%d -------\r\n",
		    APP_MAJ_VERSION, APP_MIN_VERSION);
        xil_printf("Build date - %s %s \r\n", __DATE__, __TIME__);
	xil_printf("------------------------------------------\r\n");

	Status = mmi_dc_mst_test(&RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("MMI_DC_LIVE_TEST failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("\nSuccessfully ran MMI DC Bypass Mode Example\r\n");

	/* Do not exit application,
	   required for monitor display */
	while (1);

	return 0;

}
