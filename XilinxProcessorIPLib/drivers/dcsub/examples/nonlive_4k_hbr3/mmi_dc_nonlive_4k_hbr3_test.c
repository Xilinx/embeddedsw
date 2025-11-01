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

#include "mmi_dc_nonlive_test.h"
#include "mmi_dp_init.h"

/************************** Constant Definitions *****************************/

#define CLK_LOCK			1
#define PL_AUD_CLK_MULT			512
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
FrameInfo Video1FbPtr;
FrameInfo Video2FbPtr;

FrameInfo AudFbInfo[MAX_AUDIO_CHANNELS];

typedef struct {
    unsigned char r, g, b, a;
} RGBA;


XDcDma_Descriptor *Desc1 = (XDcDma_Descriptor *)0x33500;
XDcDma_Descriptor *Desc2 = (XDcDma_Descriptor *)0x33600;

XDcDma_Descriptor *AudDesc[MAX_AUDIO_CHANNELS] = {
	(XDcDma_Descriptor *)0x08000000,
	(XDcDma_Descriptor *)0x08000100,
	(XDcDma_Descriptor *)0x08000200,
	(XDcDma_Descriptor *)0x08000300,
	(XDcDma_Descriptor *)0x08000400,
	(XDcDma_Descriptor *)0x08000500,
	(XDcDma_Descriptor *)0x08000600,
	(XDcDma_Descriptor *)0x08000700,
};


/* SDTV Coeffs */
u32 CSCCoeff_RGB[] = { 0x1000, 0x0000, 0x0000, 0x0000, 0x1000, 0x0000, 0x0000, 0x0000, 0x1000};
u32 CSCOffset_RGB[] =  { 0x0000, 0x0000, 0x0000 };

XDc_VideoTiming VidTiming_3840_2160 = { .HTotal = 4400, .HSWidth = 88, .HRes = 3840,
		.HStart = 384, .VTotal = 2250, .VSWidth = 10, .VRes = 2160, .VStart = 82};

void GenerateFrameInfoAttribute(FrameInfo *Frame)
{
	u32 Width = Frame->Width;
	u32 Height = Frame->Height;
	u32 Bpp = Frame->Bpp;
	u32 temp;

	Frame->Size = Width * Height * Bpp;
	Frame->LineSize = Width * Bpp;

	temp = Frame->LineSize;
	if (temp % 256 != 0 ) {
		temp = (temp / 256 + 1 ) * 256;
	}

	Frame->Stride = temp / 16;

}

void CreateDescriptors(RunConfig *RunCfgPtr, XDcDma_Descriptor *XDesc, FrameInfo *FBInfo, XDcDma_Descriptor *NextDesc)
{
	u64 DescAddr;

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

	if(NextDesc == NULL)
		DescAddr = (u64)XDesc;
	else
		DescAddr = (u64)NextDesc;

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

void InitFrames(RunConfig *RunCfgPtr)
{
	u8 i;
	FrameInfo *V2_FbInfo;
	FrameInfo *V1_FbInfo;

	V2_FbInfo = RunCfgPtr->V2_FbInfo;
	V2_FbInfo->Address = IN_BUFFER_0_ADDR_V2;
	V2_FbInfo->Width = RunCfgPtr->Width;
	V2_FbInfo->Height = RunCfgPtr->Height;
	V2_FbInfo->VideoFormat = RunCfgPtr->Stream1Format;
	V2_FbInfo->Bpp = RunCfgPtr->Stream1Bpp;
	GenerateFrameInfoAttribute(V2_FbInfo);

	V1_FbInfo = RunCfgPtr->V1_FbInfo;
	V1_FbInfo->Address = IN_BUFFER_0_ADDR_V1;
	V1_FbInfo->Width = RunCfgPtr->Width;
	V1_FbInfo->Height = RunCfgPtr->Height;
	V1_FbInfo->VideoFormat = RunCfgPtr->Stream1Format;
	V1_FbInfo->Bpp = RunCfgPtr->Stream1Bpp;
	GenerateFrameInfoAttribute(V1_FbInfo);

	/* Create Descriptors */
	CreateDescriptors(RunCfgPtr, RunCfgPtr->Desc2, RunCfgPtr->V2_FbInfo, NULL);
	CreateDescriptors(RunCfgPtr, RunCfgPtr->Desc1, RunCfgPtr->V1_FbInfo, NULL);

	if(RunCfgPtr->AudioEnable)
	{
		for (i = 0; i < MAX_AUDIO_CHANNELS; i++) {
			AudFbInfo[i].Address = CH6_BUFFER_ADDR_0 + (i * 0x30000);
			AudFbInfo[i].Size = 0x30000;
			if (i < (MAX_AUDIO_CHANNELS - 1))
				CreateDescriptors(RunCfgPtr, AudDesc[i], &AudFbInfo[i], AudDesc[i + 1]);
			else
				CreateDescriptors(RunCfgPtr, AudDesc[7], &AudFbInfo[i], AudDesc[0]);
		}
	}

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

u32 InitClkWiz(RunConfig *RunCfgPtr)
{
	XClk_Wiz InstancePtr, AudInstancePtr;
	XClk_Wiz_Config *CfgPtr, *AudCfgPtr;
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

	Status = XClk_Wiz_SetRateHz(&InstancePtr, 594000000);
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
	xil_printf("\nCalculated Rate is %ld MHz and Expected is 594 MHz \n",  Rate);

	if(RunCfgPtr->AudioEnable)
	{

		AudCfgPtr = XClk_Wiz_LookupConfig(CLK_WIZ_AUD_BASEADDR);
		if (!AudCfgPtr) {
			xil_printf("FAILED to lookup audio Clk Wizard IP\n");
			return XST_FAILURE;
		}

		Status = XClk_Wiz_CfgInitialize(&AudInstancePtr, AudCfgPtr, AudCfgPtr->BaseAddr);
		if (Status != XST_SUCCESS) {
			xil_printf("FAILED to cfg initialize Clk Wizard IP\n");
			return XST_FAILURE;
		}

		XClk_Wiz_WriteReg(AudCfgPtr->BaseAddr, XCLK_WIZ_REG25_OFFSET, 0);

		Status = XClk_Wiz_SetRateHz(&AudInstancePtr, PL_AUD_CLK_MULT * 48000);
		if (Status != XST_SUCCESS) {
			xil_printf("FAILED to set rate of Audio Clk Wizard IP\n");
			return XST_FAILURE;
		}

		XClk_Wiz_WriteReg(AudCfgPtr->BaseAddr, XCLK_WIZ_RECONFIG_OFFSET,
							(XCLK_WIZ_RECONFIG_LOAD | XCLK_WIZ_RECONFIG_SADDR));

		Status = XClk_WaitForLock(AudCfgPtr);
		if (Status != XST_SUCCESS) {
			Reg = XClk_Wiz_ReadReg(AudCfgPtr->BaseAddr, XCLK_WIZ_REG4_OFFSET) & CLK_LOCK;
			xil_printf("\n ERROR: Clock is not locked : 0x%x \t Expected "\
					": 0x1\n\r", Reg);
		}

	}
	return Status;
}

void InitDcSubPtr(RunConfig *RunCfgPtr)
{
	XDc *DcPtr = RunCfgPtr->DcSubPtr->DcPtr;
	DcPtr->Config.BaseAddr = DC_BASEADDR;
	DcPtr->VideoTiming = VidTiming_3840_2160;

	XDcSub_SetVidInterfaceMode(RunCfgPtr->DcSubPtr, XDC_VID_FUNCTIONAL);
	XDcSub_SetVidStreamSrc(RunCfgPtr->DcSubPtr, XDC_VIDSTREAM1_NONLIVE, XDC_VIDSTREAM2_NONLIVE);
	XDcSub_SetInputAudioSelect(RunCfgPtr->DcSubPtr, XDC_AUDSTREAM_NONE);
	XDcSub_SetInputNonLiveVideoFormat(RunCfgPtr->DcSubPtr, RunCfgPtr->Stream1Format,
					  RunCfgPtr->Stream2Format);
	XDcSub_SetOutputVideoFormat(RunCfgPtr->DcSubPtr, RunCfgPtr->OutStreamFormat);
	XDcSub_SetInputStreamLayerControl(RunCfgPtr->DcSubPtr, 0, 0);
	XDcSub_SetGlobalAlpha(RunCfgPtr->DcSubPtr, ALPHA_ENABLE, 0x1);
	XDcSub_SetStreamPixelScaling(RunCfgPtr->DcSubPtr, NULL, NULL);

	XDcSub_SetInputStreamCSC(RunCfgPtr->DcSubPtr, CSCCoeff_RGB, CSCOffset_RGB, CSCCoeff_RGB,
				 CSCOffset_RGB);
	XDcSub_SetOutputStreamCSC(RunCfgPtr->DcSubPtr, CSCCoeff_RGB, CSCOffset_RGB);
	XDcSub_SetAudioVideoClkSrc(RunCfgPtr->DcSubPtr);
	DcPtr->VideoClk = 0;
	DcPtr->AudioClk = 0;
	XDcSub_EnableStream1Buffers(RunCfgPtr->DcSubPtr, 1, 15);
	XDcSub_EnableStream2Buffers(RunCfgPtr->DcSubPtr, 1, 15);
	XDcSub_VidClkSelect(RunCfgPtr->DcSubPtr, 0, 0);
	XDcSub_SetVidFrameSwitch(RunCfgPtr->DcSubPtr, 0x3F);
	XDcSub_SetNonLiveLatency(RunCfgPtr->DcSubPtr, 0x00120138);

	if(RunCfgPtr->AudioEnable)
	{
		XDcSub_SetAudInterfaceMode(RunCfgPtr->DcSubPtr,XDC_AUD_FUNCTIONAL);
		XDcSub_SetInputAudioSelect(RunCfgPtr->DcSubPtr, XDC_AUDSTREAM_NONLIVE);
		XDcSub_EnableAudio(RunCfgPtr->DcSubPtr);
		XDcSub_AudClkSelect(RunCfgPtr->DcSubPtr,0);

		/* default values expected as per spec(non-segmented) */
		XDcSub_AudioChannelSelect(RunCfgPtr->DcSubPtr, 7, 0x1FF);
		XDcSub_EnableAudioBuffer(RunCfgPtr->DcSubPtr, 1, 15);
	}

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

	if(RunCfgPtr->AudioEnable)
	{
		XDc_AudioSoftReset(DcPtr);
		XDc_AudClkSelect(DcPtr);
	}

	XDc_SetVideoTiming(DcPtr);
	XDcSub_ConfigureDcVideo(DcPtr);

	/* Video DMA */
	DmaPtr->Video.Channel[XDCDMA_VIDEO_CHANNEL0].Current = RunCfgPtr->Desc1;
	DmaPtr->Gfx.Channel[XDCDMA_GRAPHICS_CHANNEL3].Current = RunCfgPtr->Desc2;

	DmaPtr->Video.VideoInfo = XDc_GetNonLiveVideoAttribute(RunCfgPtr->Stream1Format);
	DmaPtr->Gfx.VideoInfo = XDc_GetNonLiveVideoAttribute(RunCfgPtr->Stream2Format);

	DmaPtr->Video.Video_TriggerStatus = XDCDMA_TRIGGER_EN;
	DmaPtr->Gfx.Graphics_TriggerStatus = XDCDMA_TRIGGER_EN;

	/* Audio DMA */
	DmaPtr->Audio.Channel.Current = AudDesc[0];
	DmaPtr->Audio.Audio_TriggerStatus = XDCDMA_TRIGGER_EN;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* The purpose of this function is to setup call back functions for the DP
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

	/* Configure Video/Audio Clock */
	Status = InitClkWiz(RunCfgPtr);
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
	RunCfgPtr->V1_FbInfo = &Video1FbPtr;
	RunCfgPtr->V2_FbInfo = &Video2FbPtr;
	RunCfgPtr->DpPsuPtr = &DpPsuPtr;
	RunCfgPtr->Desc1 = Desc1;
	RunCfgPtr->Desc2 = Desc2;

	RunCfgPtr->DcBaseAddr = DC_BASEADDR;
	RunCfgPtr->DcDmaBaseAddr = DCDMA_BASEADDR;

	RunCfgPtr->Width = WIDTH;
	RunCfgPtr->Height = HEIGHT;

	RunCfgPtr->Stream1Format = RGBA8888;
	RunCfgPtr->Stream1Bpp = 4;
	RunCfgPtr->Stream2Format = RGBA8888;
	RunCfgPtr->Stream2Bpp = 4;

	RunCfgPtr->CursorEnable = CB_DISABLE;
	RunCfgPtr->OutStreamFormat = RGB_8BPC;

	/* Enable/Disable Audio */
	RunCfgPtr->AudioEnable = 1;

	InitFrames(RunCfgPtr);
	InitDcSubPtr(RunCfgPtr);

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
u32 mmi_dc_nonlive_test(RunConfig *RunCfgPtr)
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

/*****************************************************************************/
/**
*
* The purpose of this function is to generate an image & populate the input buffers
* according to the specified width and height.
*
* @param        image is a pointer to the RGBA color component structure.
*
* @return       None.
*
* @note         None.
*
*****************************************************************************/

void generate_demo_scene(RGBA *image)
{
	int cx = WIDTH / 2;
	int cy = HEIGHT / 2;
	int radius = HEIGHT / 4;
	int x, y, idx, dx, dy;
	unsigned char r, g, b;

	for (y = 0; y < HEIGHT; y++) {
		for (x = 0; x < WIDTH; x++) {
			idx = y * WIDTH + x;
			/* Background gradient (sky blue to dark blue) */
			r = (unsigned char)(x * 255 / WIDTH);
			g = (unsigned char)(y * 255 / HEIGHT);
			b = 200;

			/* Draw a red circle in the center */
			dx = x - cx;
			dy = y - cy;
			if (dx * dx + dy * dy < radius * radius) {
				r = 255;
				g = 50;
				b = 50;
			}
			image[idx].r = r;
			image[idx].g = g;
			image[idx].b = b;
			image[idx].a = 255;
		}
	}
}


int main()
{
	u32 Status;
	RGBA *buf1 = IN_BUFFER_0_ADDR_V1;
	RGBA *buf2 = IN_BUFFER_0_ADDR_V2;

	Xil_DCacheDisable();
	Xil_ICacheDisable();

	Status = mmi_dc_nonlive_test(&RunCfg);
	if (Status != XST_SUCCESS) {
		xil_printf("MMI_DC_NONLIVE_TEST failed\r\n");
		return XST_FAILURE;
	}

	generate_demo_scene(buf1);
	xil_printf("Loaded Input Buffer-1\r\n");
	generate_demo_scene(buf2);
	xil_printf("Loaded Input Buffer-2\r\n");

	xil_printf("Successfully ran MMI_DC_NONLIVE_TEST\r\n");

	/* Do not exit application, required for monitor display */
	while(1);

	return 0;

}
