/******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <xil_exception.h>
#include <xil_printf.h>
#include <xil_cache.h>
#include <xil_types.h>
#include <sleep.h>

#include "mmi_dc_mixed_test.h"
#include "mmi_dp_init.h"

/************************** Constant Definitions *****************************/

#define CLK_LOCK			1
#define UPDATE_ENABLE_SHIFT             8U
#define IGNORE_DONE_SHIFT               9U
#define LAST_DESC_SHIFT                 10U /* 16(ID) +  10 */
#define LAST_DESC_FRAME_SHIFT           11U

#define XDCDMA_IEN_VSYNC_INT_MASK	0x0000000C
#define XDCDMA_INTR_ID			179
#define XDCDMA_INTR_PARENT		0xe2000000

RunConfig RunCfg;
XDcSub DcSub;
XDcDma DcDma;
XDc Dc;
XMmiDp DpPsuPtr;

FrameInfo Video2FbPtr;

XVtc Vtc0;
XAvTpg AvTpg0;
XAvTpg_VideoTiming TpgTiming;

XDcDma_Descriptor *Desc2 = (XDcDma_Descriptor *)0x33600;

/* SDTV Coeffs */
u32 In_CSCCoeff_RGB[] = { 0x1000, 0x0000, 0x0000, 0x0000, 0x1000, 0x0000, 0x0000, 0x0000, 0x1000};
u32 In_CSCOffset_RGB[] =  { 0x0000, 0x0000, 0x0000 };

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

void InitLiveMode()
{
	XDc_WriteReg(XPG_AXI_GPIO1, 0x0, 0x00010000);
}

u32 InitAvTpgSubsystem(XAvTpg *AvTpgPtr)
{
	XAvTpg_SetVideoTiming(AvTpgPtr);

	return XST_SUCCESS;
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
u32 InitVtc0Subsystem(RunConfig *RunCfgPtr)
{
	XVtc_Config CfgPtr;
	XVtc_Polarity Polarity;
	XVtc_SourceSelect SourceSelect;
	XVtc_Timing VideoTiming;
	XVtc_Signal Signal;
	XVtc_HoriOffsets Hoff;

	XVtc *Vtc = RunCfgPtr->VtcPtr0;
	u32 PixPerClk = RunCfgPtr->PPC;

	XVtc_CfgInitialize(Vtc, &CfgPtr, V_TC_BASE);

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

void GenerateFrameInfoAttribute(FrameInfo *Frame)
{
	u32 Width = Frame->Width;
	u32 Height = Frame->Height;
	u32 Bpc = Frame->Bpc;

	Frame->Size = Width * Height * Bpc;
	Frame->LineSize = Width * Bpc;

	u32 temp = Frame->LineSize;
	if (temp % 256 != 0 ) {
		temp = (temp / 256 + 1 ) * 256;
	}

	Frame->Stride = temp / 16;

}

void CreateDescriptors(RunConfig *RunCfgPtr, XDcDma_Descriptor *XDesc, FrameInfo *FBInfo)
{
	u8 Last_Desc_Frame = 1;
	u8 Last_Desc = 0;
	u8 Update_Enable = 1;
	u8 Ignore_Done = 1;

	XDcDma *DcDmaPtr = RunCfgPtr->DcSubPtr->DmaPtr;
	XDcDma_DescInit(XDesc);

	XDesc->Id = XDcDma_GetNewDescId(DcDmaPtr);

	XDesc->Control_Lo |= ((Update_Enable) << UPDATE_ENABLE_SHIFT );
	XDesc->Control_Lo |= ((Ignore_Done) << IGNORE_DONE_SHIFT );
	XDesc->Control_Lo |= ((Last_Desc) << LAST_DESC_SHIFT );
	XDesc->Control_Lo |= ((Last_Desc_Frame) << LAST_DESC_FRAME_SHIFT );

	u64 DataSize = (u64)(FBInfo->Size);
	XDesc->Data_Size_Lo = (DataSize & 0xFFFF);
	XDesc->Data_Size_Hi = ((DataSize & 0xFFFF0000) >> 16);

	u64 FBAddr = (u64)FBInfo->Address;
	XDesc->Src_Addr_Lo = FBAddr & 0xFFFF;
	XDesc->Src_Addr_Hi = (FBAddr >> 16);

	u64 DescAddr = (u64)XDesc;
	XDesc->Next_Desc_Addr_Lo = LOWER_32_BITS(DescAddr);
	XDesc->Next_Desc_Addr_Hi = (DescAddr) >> 32;

	if (RunCfgPtr->CursorEnable == CB_DISABLE) {
		XDesc->Line_Size = FBInfo->LineSize;
		XDesc->Line_Stride = FBInfo->Stride;
	} else {
		XDesc->Target_Addr = 1;
	}

	XDesc->Intr_Enable = 1;

}

void GenerateFrames()
{
	xil_printf("POPBUFF RGBA_8BPC\n");

	for (int i = 0; i < 102400; i++) {
		XDc_WriteReg(IN_BUFFER_0_ADDR_V2, 0x0 + (i * 12), 0x000000FF);
		XDc_WriteReg(IN_BUFFER_0_ADDR_V2, 0x4 + (i * 12), 0x000000FF);
		XDc_WriteReg(IN_BUFFER_0_ADDR_V2, 0x8 + (i * 12), 0x000000FF);
	}

	xil_printf("POPBUFF done\n");
}

void InitFrames(RunConfig *RunCfgPtr)
{
	FrameInfo *V2_FbInfo;

	V2_FbInfo = RunCfgPtr->V2_FbInfo;
	V2_FbInfo->Address = IN_BUFFER_0_ADDR_V2;
	V2_FbInfo->Width = RunCfgPtr->Width;
	V2_FbInfo->Height = RunCfgPtr->Height;
	V2_FbInfo->VideoFormat = RunCfgPtr->Stream1Format;
	V2_FbInfo->Bpc = RunCfgPtr->Stream1Bpc;
	GenerateFrameInfoAttribute(V2_FbInfo);

	/* Create Descriptors */
	CreateDescriptors(RunCfgPtr, RunCfgPtr->Desc2, RunCfgPtr->V2_FbInfo);

	GenerateFrames();

}

void InitDcSubPtr(RunConfig *RunCfgPtr)
{

	XDc *DcPtr = RunCfgPtr->DcSubPtr->DcPtr;
	DcPtr->Config.BaseAddr = DC_BASEADDR;
	DcPtr->VideoTiming = VidTiming_640_480;

	XDcSub_SetVidInterfaceMode(RunCfgPtr->DcSubPtr, XDC_VID_FUNCTIONAL);
	XDcSub_SetVidStreamSrc(RunCfgPtr->DcSubPtr, XDC_VIDSTREAM1_LIVE, XDC_VIDSTREAM2_NONLIVE);
	XDcSub_SetInputAudioSelect(RunCfgPtr->DcSubPtr, XDC_AUDSTREAM_NONE);
	XDcSub_SetInputNonLiveVideoFormat(RunCfgPtr->DcSubPtr, RunCfgPtr->Stream1Format,
					  RunCfgPtr->Stream2Format);
	XDcSub_SetInputLiveStreamFormat(RunCfgPtr->DcSubPtr, RunCfgPtr->Stream1Format,
					RunCfgPtr->Stream2Format);
	XDcSub_SetOutputVideoFormat(RunCfgPtr->DcSubPtr, RunCfgPtr->OutStreamFormat);
	XDcSub_SetInputStreamLayerControl(RunCfgPtr->DcSubPtr, 0, 0);
	XDcSub_SetGlobalAlpha(RunCfgPtr->DcSubPtr, 0x1, 127);
	XDcSub_SetStreamPixelScaling(RunCfgPtr->DcSubPtr, NULL, NULL);

	XDcSub_SetInputStreamCSC(RunCfgPtr->DcSubPtr, In_CSCCoeff_RGB, In_CSCOffset_RGB, In_CSCCoeff_RGB,
				 In_CSCOffset_RGB);
	XDcSub_SetOutputStreamCSC(RunCfgPtr->DcSubPtr, In_CSCCoeff_RGB, In_CSCOffset_RGB);
	XDcSub_SetAudioVideoClkSrc(RunCfgPtr->DcSubPtr);
	DcPtr->VideoClk = 0;
	DcPtr->AudioClk = 0;
	XDcSub_EnableStream2Buffers(RunCfgPtr->DcSubPtr, 1, 15);
	XDcSub_VidClkSelect(RunCfgPtr->DcSubPtr, 0, 0);
	XDcSub_SetVidFrameSwitch(RunCfgPtr->DcSubPtr, 0x3F);
	XDcSub_SetNonLiveLatency(RunCfgPtr->DcSubPtr, 0x00120138);

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
	XDcDma *DmaPtr = DcSubPtr->DmaPtr;

	XDcSub_Initialize(DcSubPtr);
	XDcDma_CfgInitialize(DmaPtr, DCDMA_BASEADDR);
	XDcDma_WriteProtDisable(DmaPtr);
	XDc_VidClkSelect(DcPtr);
	XDc_VideoSoftReset(DcPtr);
	XDc_SetVidFrameSwitch(DcPtr);
	XDc_SetVideoTiming(DcPtr);
	XDcSub_ConfigureDcVideo(DcPtr);

	DmaPtr->Gfx.Channel[XDCDMA_GRAPHICS_CHANNEL3].Current = RunCfgPtr->Desc2;
	DmaPtr->Gfx.VideoInfo = XDc_GetNonLiveVideoAttribute(RunCfgPtr->Stream2Format);
	DmaPtr->Gfx.Graphics_TriggerStatus = XDCDMA_TRIGGER_EN;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* The purpose of this function is to setup call back functions for the DcDMA
* controller interrupts.
*
* @param	RunCfgPtr is a pointer to the application configuration structure.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void SetupInterrupts(RunConfig *RunCfgPtr)
{
	XDcDma *DmaPtr = RunCfgPtr->DcSubPtr->DmaPtr;

	XSetupInterruptSystem(DmaPtr, &XDcDma_InterruptHandler, XDCDMA_INTR_ID,
			      XDCDMA_INTR_PARENT, XINTERRUPT_DEFAULT_PRIORITY);
	XDcDma_InterruptEnable(DmaPtr, XDCDMA_IEN_VSYNC_INT_MASK);
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

	InitLiveMode();
	InitAvTpgSubsystem(RunCfgPtr->AvTpgPtr0);
	InitVtc0Subsystem(RunCfgPtr);
	EnableStream0();
	EnableAvTpg();

	xil_printf("Enabling Output to DisplayPort\n");
	/* Initialize DpSubsystem */
	Status = InitDpPsuSubsystem(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("DpPsu14 Subsystem Initialization failed\n");
		return Status;
	}

	SetupInterrupts(RunCfgPtr);

	return Status;
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
	RunCfgPtr->DcSubPtr->DmaPtr = &DcDma;
	RunCfgPtr->V2_FbInfo = &Video2FbPtr;
	RunCfgPtr->DpPsuPtr = &DpPsuPtr;
	RunCfgPtr->Desc2 = Desc2;

	// BaseAddress
	RunCfgPtr->DcBaseAddr = DC_BASEADDR;
	RunCfgPtr->DcDmaBaseAddr = DCDMA_BASEADDR;

	RunCfgPtr->Width = 640;
	RunCfgPtr->Height = 480;
	RunCfgPtr->PPC = 2;

	RunCfgPtr->Stream1Format = RGB_8BPC;
	RunCfgPtr->Stream1Bpc = 3;
	RunCfgPtr->Stream2Format = RGBA8888;
	RunCfgPtr->Stream2Bpc = 4;

	RunCfgPtr->CursorEnable = CB_DISABLE;
	RunCfgPtr->OutStreamFormat = RGB_8BPC;

	RunCfgPtr->VtcPtr0 = &Vtc0;
	RunCfgPtr->AvTpgPtr0 = &AvTpg0;

	InitFrames(RunCfgPtr);
	InitDcSubPtr(RunCfgPtr);

	XAvTpg *AvTpgPtr0 = RunCfgPtr->AvTpgPtr0;

	AvTpgPtr0->Config.BaseAddr = AV_PATGEN_BASE;
	AvTpgPtr0->Format = 0;
	AvTpgPtr0->DualPixelMode = 1;
	AvTpgPtr0->Bpc = 1;
	AvTpgPtr0->Pattern = 0x1;
	AvTpgPtr0->VideoTiming = &TpgTiming_640_480;

	return Status;

}

/*****************************************************************************/
/**
*
* The purpose of this function is to configure the test params and the platform
* driver & Xdc driver to set DC Mode
*
* @param        RunCfgPtr is a pointer to the application configuration structure.
*
* @return       XST_SUCCESS if successful, else XST_FAILURE.
*
* @note         None.
*
*****************************************************************************/
u32 mmi_dc_mixed_test(RunConfig *RunCfgPtr)
{

	u32 Status = XST_SUCCESS;

	/* Initialize the application configuration */
	Status = InitRunConfig(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("FAILED to get test configuration\n");
		return Status;
	}

	/* Platform Initialization */
	Status = InitPlatform(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("FAILED to Initialize Platform\n");
		return Status;
	}

	return Status;

}

int main()
{
	u32 Status;

	Xil_DCacheDisable();
	Xil_ICacheDisable();

	Status = mmi_dc_mixed_test(&RunCfg);
	if (Status != XST_SUCCESS) {
		xil_printf("MMI_DC_MIXED_TEST failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran MMI_DC_MIXED_TEST\r\n");

	/* Do not exit application,
	   required for monitor display */
	while (1);


	return 0;

}
