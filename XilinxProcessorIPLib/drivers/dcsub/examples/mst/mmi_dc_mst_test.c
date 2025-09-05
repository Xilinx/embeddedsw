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

#include "mmi_dc_mst_test.h"
#include "mmi_dp_init.h"

/* Clk Wiz IP offsets */
#define XCLK_WIZ_CLKOUT1_1_OFFSET	0x340
#define XCLK_WIZ_CLKOUT1_2_OFFSET	0x344
#define XCLK_WIZ_CLKOUT2_1_OFFSET	0x348
#define XCLK_WIZ_CLKOUT2_2_OFFSET	0x34c
#define XCLK_WIZ_CLKOUT3_1_OFFSET	0x350
#define XCLK_WIZ_CLKOUT3_2_OFFSET	0x354
#define XCLK_WIZ_REG6_OFFSET		0x37c
#define XCLK_WIZ_REG7_OFFSET		0x384
#define XCLK_WIZ_REG8_OFFSET		0x394

/* 640x480 25.2 on 2X */
#define XCLK_WIZ_REG1			0x0
#define XCLK_WIZ_REG3			0x75f
#define XCLK_WIZ_REG4   		0xa75
#define XCLK_WIZ_CLKOUT1_1		0x0
#define XCLK_WIZ_CLKOUT1_2		0xa9c
#define XCLK_WIZ_CLKOUT2_1		0x0
#define XCLK_WIZ_CLKOUT2_2		0x20c
#define XCLK_WIZ_CLKOUT3_1		0x0
#define XCLK_WIZ_CLKOUT3_2		0x20c
#define XCLK_WIZ_REG6			0x0
#define XCLK_WIZ_REG7			0x0
#define XCLK_WIZ_REG13			0x6a0
#define XCLK_WIZ_REG8			0x320
#define XCLK_WIZ_REG15			0x20
#define XCLK_WIZ_REG16			0x20
#define XCLK_WIZ_RECONFIG		0x3

RunConfig RunCfgPtr;
XDcSub DcSub;
XDc Dc;
XMmiDp DpPsuPtr;

XVtc Vtc0;
XAvTpg AvTpg0;
XVtc Vtc1;
XAvTpg AvTpg1;
XVtc Vtc2;
XAvTpg AvTpg2;
XVtc Vtc3;
XAvTpg AvTpg3;

XAvTpg_VideoTiming TpgTiming;

/* Video Timings */
XDc_VideoTiming VidTiming_640_480 = { .HTotal = 800, .HSWidth = 96, .HRes = 640, .HStart = 144, .VTotal = 525, .VSWidth = 2, .VRes = 480, .VStart = 35};
XAvTpg_VideoTiming TpgTiming_640_480 = { .HTotal = 800, .HSWidth = 96, .HRes = 640, .HStart = 144, .VTotal = 525, .VSWidth = 2, .VRes = 480, .VStart = 35, .HBackPorch = 48, .HFrontPorch = 16, .VBackPorch = 33, .VFrontPorch = 10, .HRes_hack = 0, .HSWidth_hack = 0};

void EnableAvTpg()
{
	XAvTpg_WriteReg(XPG_AXI_GPIO1, 0x0, 0x00010001);
	XAvTpg_WriteReg(DPDC_ALPHA_BYPASS_EN, 0x8, 0x00000023);
	XAvTpg_WriteReg(DPDC_ALPHA_BYPASS_EN, 0x8, 0x00000013);
}

void EnableStream0()
{
	XAvTpg_WriteReg(AV_PATGEN_BASE0, 0x000, 0x1);
	XAvTpg_WriteReg(AV_PATGEN_BASE0, 0x400, 0x2);
	XVtc_WriteReg(V_TC_BASE0, 0x0000, 0x03f7ef06);
}

void EnableStream1()
{
	XAvTpg_WriteReg(AV_PATGEN_BASE1, 0x000, 0x1);
	XAvTpg_WriteReg(AV_PATGEN_BASE1, 0x400, 0x2);
	XVtc_WriteReg(V_TC_BASE1, 0x0000, 0x03f7ef06);
}

void EnableStream2()
{
	XAvTpg_WriteReg(AV_PATGEN_BASE2, 0x000, 0x1);
	XAvTpg_WriteReg(AV_PATGEN_BASE2, 0x400, 0x2);
	XVtc_WriteReg(V_TC_BASE2, 0x0000, 0x03f7ef06);
}

void EnableStream3()
{
	XAvTpg_WriteReg(AV_PATGEN_BASE3, 0x000, 0x1);
	XAvTpg_WriteReg(AV_PATGEN_BASE3, 0x400, 0x2);
	XVtc_WriteReg(V_TC_BASE3, 0x0000, 0x03f7ef06);
}

void InitBypassMode()
{
	XDc_WriteReg(DPDC_ALPHA_BYPASS_EN, 0x0, 0x00010000);
	XDc_WriteReg(XPG_AXI_GPIO1, 0x0, 0x00010000);
	XDc_WriteReg(DPDC_LV_AVTPG_PM, 0x0, 0x00000000 | (0x0 << 2) | 0x1);
	XDc_WriteReg(DPDC_ALPHA_GPIO, 0x8, 0xa5a5a5a5);

}

void InitClkWiz()
{
	XClk_Wiz InstancePtr;
	XClk_Wiz_Config CfgPtr;

	XClk_Wiz_CfgInitialize(&InstancePtr, &CfgPtr, CLK_WIZ_BASEADDR);

	XClk_Wiz_WriteReg(InstancePtr.Config.BaseAddr, XCLK_WIZ_REG1_OFFSET, XCLK_WIZ_REG1);
	XClk_Wiz_WriteReg(InstancePtr.Config.BaseAddr, XCLK_WIZ_REG3_OFFSET, XCLK_WIZ_REG3);
	XClk_Wiz_WriteReg(InstancePtr.Config.BaseAddr, XCLK_WIZ_REG4_OFFSET, XCLK_WIZ_REG4);
	XClk_Wiz_WriteReg(InstancePtr.Config.BaseAddr, XCLK_WIZ_CLKOUT1_1_OFFSET, XCLK_WIZ_CLKOUT1_1);
	XClk_Wiz_WriteReg(InstancePtr.Config.BaseAddr, XCLK_WIZ_CLKOUT1_2_OFFSET, XCLK_WIZ_CLKOUT1_2);
	XClk_Wiz_WriteReg(InstancePtr.Config.BaseAddr, XCLK_WIZ_CLKOUT2_1_OFFSET, XCLK_WIZ_CLKOUT2_1);
	XClk_Wiz_WriteReg(InstancePtr.Config.BaseAddr, XCLK_WIZ_CLKOUT2_2_OFFSET, XCLK_WIZ_CLKOUT2_2);
	XClk_Wiz_WriteReg(InstancePtr.Config.BaseAddr, XCLK_WIZ_CLKOUT3_1_OFFSET, XCLK_WIZ_CLKOUT3_1);
	XClk_Wiz_WriteReg(InstancePtr.Config.BaseAddr, XCLK_WIZ_CLKOUT3_2_OFFSET, XCLK_WIZ_CLKOUT3_2);
	XClk_Wiz_WriteReg(InstancePtr.Config.BaseAddr, XCLK_WIZ_REG6_OFFSET, XCLK_WIZ_REG6);
	XClk_Wiz_WriteReg(InstancePtr.Config.BaseAddr, XCLK_WIZ_REG13_OFFSET, XCLK_WIZ_REG13);
	XClk_Wiz_WriteReg(InstancePtr.Config.BaseAddr, XCLK_WIZ_REG7_OFFSET, XCLK_WIZ_REG7);
	XClk_Wiz_WriteReg(InstancePtr.Config.BaseAddr, XCLK_WIZ_REG8_OFFSET, XCLK_WIZ_REG8);
	XClk_Wiz_WriteReg(InstancePtr.Config.BaseAddr, XCLK_WIZ_REG15_OFFSET, XCLK_WIZ_REG15);
	XClk_Wiz_WriteReg(InstancePtr.Config.BaseAddr, XCLK_WIZ_REG16_OFFSET, XCLK_WIZ_REG16);
	XClk_Wiz_WriteReg(InstancePtr.Config.BaseAddr, XCLK_WIZ_RECONFIG_OFFSET, XCLK_WIZ_RECONFIG);

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

	VideoTiming->HRes_hack = VideoTiming->HRes / RunCfgPtr->PPC;
	VideoTiming->HSWidth_hack = VideoTiming->HSWidth / RunCfgPtr->PPC;

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

	InitClkWiz();

	/* Initialize DcSubsystem */
	Status = InitDcSubsystem(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("FAILED to Initialize DC Subsystem Ip\n");
		return Status;
	}
	xil_printf("DcSubsystem Initialization complete\n");

	InitBypassMode();

	/* Stream1 */
	InitAvTpgSubsystem(RunCfgPtr, RunCfgPtr->AvTpgPtr0);
	InitVtcSubsystem(RunCfgPtr, RunCfgPtr->VtcPtr0, V_TC_BASE0);
	EnableStream0();
	xil_printf("AvTpg Initialization for stream 1 complete\n");

	/* Stream2 */
	InitAvTpgSubsystem(RunCfgPtr, RunCfgPtr->AvTpgPtr1);
	InitVtcSubsystem(RunCfgPtr, RunCfgPtr->VtcPtr1, V_TC_BASE1);
	EnableStream1();
	xil_printf("AvTpg Initialization for stream 2 complete\n");

	/* Stream3 */
	InitAvTpgSubsystem(RunCfgPtr, RunCfgPtr->AvTpgPtr2);
	InitVtcSubsystem(RunCfgPtr, RunCfgPtr->VtcPtr2, V_TC_BASE2);
	EnableStream2();
	xil_printf("AvTpg Initialization for stream 3 complete\n");

	/* Stream4 */
	InitAvTpgSubsystem(RunCfgPtr, RunCfgPtr->AvTpgPtr3);
	InitVtcSubsystem(RunCfgPtr, RunCfgPtr->VtcPtr3, V_TC_BASE3);
	EnableStream3();
	xil_printf("AvTpg Initialization for stream 4 complete\n");

	EnableAvTpg();

	/* Initialize DpSubsystem */
	Status = InitDpPsuSubsystem(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("DpPsu14 Subsystem Initialization failed\n");
		return Status;
	}
	xil_printf("DpPsu14 Subsystem Initialization complete\n");

	return Status;
}

void InitDcSubPtr(RunConfig *RunCfgPtr)
{
	XDc *DcPtr = RunCfgPtr->DcSubPtr->DcPtr;
	DcPtr->Config.BaseAddr = DC_BASEADDR;
	DcPtr->VideoTiming = VidTiming_640_480;

	XDcSub_SetVidInterfaceMode(RunCfgPtr->DcSubPtr, XDC_VID_BYPASS);
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
	RunCfgPtr->VtcPtr1 = &Vtc1;
	RunCfgPtr->AvTpgPtr1 = &AvTpg1;
	RunCfgPtr->VtcPtr2 = &Vtc2;
	RunCfgPtr->AvTpgPtr2 = &AvTpg2;
	RunCfgPtr->VtcPtr3 = &Vtc3;
	RunCfgPtr->AvTpgPtr3 = &AvTpg3;

	RunCfgPtr->DcBaseAddr = DC_BASEADDR;
	RunCfgPtr->Width = 640;
	RunCfgPtr->Height = 480;
	RunCfgPtr->PPC = 0x1;

	/* Stream 0 */
	XAvTpg *AvTpgPtr0 = RunCfgPtr->AvTpgPtr0;
	AvTpgPtr0->Config.BaseAddr = AV_PATGEN_BASE0;
	AvTpgPtr0->Format = 0;
	AvTpgPtr0->DualPixelMode = 0;
	AvTpgPtr0->Bpc = 1;
	AvTpgPtr0->Pattern = 0x3;
	AvTpgPtr0->VideoTiming = &TpgTiming_640_480;

	/* Stream 1 */
	XAvTpg *AvTpgPtr1 = RunCfgPtr->AvTpgPtr1;
	AvTpgPtr1->Config.BaseAddr = AV_PATGEN_BASE1;
	AvTpgPtr1->Format = 0;
	AvTpgPtr1->DualPixelMode = 0;
	AvTpgPtr1->Bpc = 1;
	AvTpgPtr1->Pattern = 0x4;
	AvTpgPtr1->VideoTiming = &TpgTiming_640_480;

	/* Stream 2 */
	XAvTpg *AvTpgPtr2 = RunCfgPtr->AvTpgPtr2;
	AvTpgPtr2->Config.BaseAddr = AV_PATGEN_BASE2;
	AvTpgPtr2->Format = 0;
	AvTpgPtr2->DualPixelMode = 0;
	AvTpgPtr2->Bpc = 1;
	AvTpgPtr2->Pattern = 0x1;
	AvTpgPtr2->VideoTiming = &TpgTiming_640_480;

	/* Stream 3 */
	XAvTpg *AvTpgPtr3 = RunCfgPtr->AvTpgPtr3;
	AvTpgPtr3->Config.BaseAddr = AV_PATGEN_BASE3;
	AvTpgPtr3->Format = 0;
	AvTpgPtr3->DualPixelMode = 0;
	AvTpgPtr3->Bpc = 1;
	AvTpgPtr3->Pattern = 0x5;
	AvTpgPtr3->VideoTiming = &TpgTiming_640_480;

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

	Status = mmi_dc_mst_test(&RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("MMI_DC_LIVE_TEST failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("\nSuccessfully ran MMI_DC MST Mode Example\r\n");

	/* Do not exit application,
	   required for monitor display */
	while (1);

	return 0;

}
