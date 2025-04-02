/******************************************************************************
*
* Copyright (C) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
*
******************************************************************************/

// Test Name - mmi_dc_nonlive_test
#include <stdio.h>
#include <stdlib.h>

#include <xil_exception.h>
#include <xil_printf.h>
#include <xil_cache.h>
#include <xil_types.h>
#include <sleep.h>
#include <xparameters.h>

#include "mmi_dc_nonlive_test.h"
#include "mmi_dp_init.h"

/************************** Constant Definitions *****************************/

#define UPDATE_ENABLE_SHIFT             8U
#define IGNORE_DONE_SHIFT               9U
#define LAST_DESC_SHIFT                 10U /* 16(ID) +  10 */
#define LAST_DESC_FRAME_SHIFT           11U

#define XDCDMA_IEN_VSYNC_INT_MASK	0x0000000C

RunConfig RunCfg;
XDcSub DcSub;
XDcDma DcDma;
XDc Dc;
XV_frmbufwr FrmbufWr;
XScuGic IntrPtr;
FrameInfo OutFbPtr;
FrameInfo Video1FbPtr;
FrameInfo Video2FbPtr;

XMmiDp DpPsuPtr;

XDcDma_Descriptor *Desc1 = (XDcDma_Descriptor *)0x33500;
XDcDma_Descriptor *Desc2 = (XDcDma_Descriptor *)0x33600;

/* SDTV Coeffs */
u32 In_CSCCoeff_RGB[] = { 0x1000, 0x0000, 0x0000, 0x0000, 0x1000, 0x0000, 0x0000, 0x0000, 0x1000};
u32 In_CSCOffset_RGB[] =  { 0x0000, 0x0000, 0x0000 };

XDc_VideoTiming VidTiming_640_480 = { .HTotal = 800, .HSWidth = 96, .HRes = 640, .HStart = 144, .VTotal = 525, .VSWidth = 2, .VRes = 480, .VStart = 35};

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
		XDc_WriteReg(IN_BUFFER_0_ADDR_V1, 0x0 + (i * 12), 0x000000FF);
		XDc_WriteReg(IN_BUFFER_0_ADDR_V1, 0x4 + (i * 12), 0x000000FF);
		XDc_WriteReg(IN_BUFFER_0_ADDR_V1, 0x8 + (i * 12), 0x000000FF);

		XDc_WriteReg(IN_BUFFER_0_ADDR_V2, 0x0 + (i * 12), 0x000000FF);
		XDc_WriteReg(IN_BUFFER_0_ADDR_V2, 0x4 + (i * 12), 0x000000FF);
		XDc_WriteReg(IN_BUFFER_0_ADDR_V2, 0x8 + (i * 12), 0x000000FF);

		XDc_WriteReg(OUTPUT_BUFFER_ADDR, 0x0 + (i * 12), 0xA5A5A5A5);
		XDc_WriteReg(OUTPUT_BUFFER_ADDR, 0x4 + (i * 12), 0xA5A5A5A5);
		XDc_WriteReg(OUTPUT_BUFFER_ADDR, 0x8 + (i * 12), 0xA5A5A5A5);

	}

	xil_printf("POPBUFF done\n");
}

void InitFrames(RunConfig *RunCfgPtr)
{
	FrameInfo *Out_FbInfo;
	FrameInfo *V2_FbInfo;
	FrameInfo *V1_FbInfo;

	Out_FbInfo  = RunCfgPtr->Out_FbInfo;

	Out_FbInfo->Address = OUTPUT_BUFFER_ADDR;
	Out_FbInfo->VideoFormat = RGB888;
	Out_FbInfo->Width = RunCfgPtr->Width;
	Out_FbInfo->Height = RunCfgPtr->Height;
	Out_FbInfo->Bpc = 5;
	GenerateFrameInfoAttribute(Out_FbInfo);

	V2_FbInfo = RunCfgPtr->V2_FbInfo;
	V2_FbInfo->Address = IN_BUFFER_0_ADDR_V2;
	V2_FbInfo->Width = RunCfgPtr->Width;
	V2_FbInfo->Height = RunCfgPtr->Height;
	V2_FbInfo->VideoFormat = RunCfgPtr->Stream1Format;
	V2_FbInfo->Bpc = RunCfgPtr->Stream1Bpc;
	GenerateFrameInfoAttribute(V2_FbInfo);

	V1_FbInfo = RunCfgPtr->V1_FbInfo;
	V1_FbInfo->Address = IN_BUFFER_0_ADDR_V1;
	V1_FbInfo->Width = RunCfgPtr->Width;
	V1_FbInfo->Height = RunCfgPtr->Height;
	V1_FbInfo->VideoFormat = RunCfgPtr->Stream1Format;
	V1_FbInfo->Bpc = RunCfgPtr->Stream1Bpc;
	GenerateFrameInfoAttribute(V1_FbInfo);

	/* Create Descriptors */
	CreateDescriptors(RunCfgPtr, RunCfgPtr->Desc2, RunCfgPtr->V2_FbInfo);
	CreateDescriptors(RunCfgPtr, RunCfgPtr->Desc1, RunCfgPtr->V1_FbInfo);

	GenerateFrames();

}

void InitDcSubPtr(RunConfig *RunCfgPtr)
{

	XDc *DcPtr = RunCfgPtr->DcSubPtr->DcPtr;
	DcPtr->Config.BaseAddr = DC_BASEADDR;
	DcPtr->VideoTiming = VidTiming_640_480;

	XDcSub_SetVidInterfaceMode(RunCfgPtr->DcSubPtr, XDC_VID_FUNCTIONAL);
	XDcSub_SetVidStreamSrc(RunCfgPtr->DcSubPtr, XDC_VIDSTREAM1_NONLIVE, XDC_VIDSTREAM2_NONLIVE);
	XDcSub_SetInputAudioSelect(RunCfgPtr->DcSubPtr, XDC_AUDSTREAM_NONE);
	XDcSub_SetInputNonLiveVideoFormat(RunCfgPtr->DcSubPtr, RunCfgPtr->Stream1Format,
					  RunCfgPtr->Stream2Format);
	XDcSub_SetOutputVideoFormat(RunCfgPtr->DcSubPtr, RunCfgPtr->OutStreamFormat);
	XDcSub_SetInputStreamLayerControl(RunCfgPtr->DcSubPtr, 0, 0);
	XDcSub_SetGlobalAlpha(RunCfgPtr->DcSubPtr, ALPHA_ENABLE, 0x0);
	XDcSub_SetStreamPixelScaling(RunCfgPtr->DcSubPtr, NULL, NULL);

	XDcSub_SetInputStreamCSC(RunCfgPtr->DcSubPtr, In_CSCCoeff_RGB, In_CSCOffset_RGB, In_CSCCoeff_RGB,
				 In_CSCOffset_RGB);
	XDcSub_SetOutputStreamCSC(RunCfgPtr->DcSubPtr, In_CSCCoeff_RGB, In_CSCOffset_RGB);
	XDcSub_SetAudioVideoClkSrc(RunCfgPtr->DcSubPtr);
	DcPtr->VideoClk = 0;
	DcPtr->AudioClk = 0;
	XDcSub_EnableStream1Buffers(RunCfgPtr->DcSubPtr, 1, 15);
	XDcSub_EnableStream2Buffers(RunCfgPtr->DcSubPtr, 1, 15);
	XDcSub_VidClkSelect(RunCfgPtr->DcSubPtr, 0, 0);
	XDcSub_SetVidFrameSwitch(RunCfgPtr->DcSubPtr, 0x3F);

}

/*****************************************************************************/
/**
*
* The purpose of this function is to configure framebuffer IP.
*
* @param        RunCfgPtr is a pointer to the application configuration structure.
*
* @return       None.
*
* @note         None.
*
*****************************************************************************/
u32 ConfigFbWr(RunConfig *RunCfgPtr)
{
	XV_frmbufwr_Config CfgPtr;
	XV_frmbufwr *FrmbufPtr;
	FrameInfo *FbInfo;

	UINTPTR FB_BASEADDR = 0xB0590000;

	FrmbufPtr = RunCfgPtr->FrmbufPtr;
	FbInfo = RunCfgPtr->Out_FbInfo;

	XV_frmbufwr_CfgInitialize(FrmbufPtr, &CfgPtr, FB_BASEADDR);

	XDc_WriteReg(RunCfgPtr->FbBaseAddr, 0x0, 0); //control register
	XDc_WriteReg(RunCfgPtr->FbBaseAddr, 0x4, 1); //Interrupt Enable
	XDc_WriteReg(RunCfgPtr->FbBaseAddr, 0x8, 3);
	XDc_WriteReg(RunCfgPtr->FbBaseAddr, 0xC, 0);

	XV_frmbufwr_Set_HwReg_width(FrmbufPtr, FbInfo->Width);
	XV_frmbufwr_Set_HwReg_height(FrmbufPtr, FbInfo->Height);
	XV_frmbufwr_Set_HwReg_stride(FrmbufPtr, FbInfo->Width * 5);
	XV_frmbufwr_Set_HwReg_video_format(FrmbufPtr, 30); // Packed RGB - 30
	XV_frmbufwr_Set_HwReg_frm_buffer_V(FrmbufPtr, FbInfo->Address);
	XV_frmbufwr_Start(FrmbufPtr);

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
	XDc_VideoStream1 VideoSrc1;
	XDc_VideoStream2 VideoSrc2;
	XDcSub *DcSubPtr = RunCfgPtr->DcSubPtr;
	XDc *DcPtr = DcSubPtr->DcPtr;
	XDcDma *DmaPtr = DcSubPtr->DmaPtr;

	VideoSrc1 = DcPtr->AVMode.VideoSrc1;
	VideoSrc2 = DcPtr->AVMode.VideoSrc2;

	XDcSub_Initialize(DcSubPtr);
	XDcDma_CfgInitialize(DmaPtr, DCDMA_BASEADDR);
	XDcDma_WriteProtDisable(DmaPtr);
	XDc_VidClkSelect(DcPtr);
	XDc_VideoSoftReset(DcPtr);
	XDc_SetVidFrameSwitch(DcPtr);
	XDc_SetVideoTiming(DcPtr);
	XDcSub_ConfigureDcVideo(DcPtr);

	XDcDma_InterruptEnable(DmaPtr, XDCDMA_IEN_VSYNC_INT_MASK);

	DmaPtr->Video.Channel[XDCDMA_VIDEO_CHANNEL0].Current = RunCfgPtr->Desc1;
	DmaPtr->Gfx.Channel[XDCDMA_GRAPHICS_CHANNEL3].Current = RunCfgPtr->Desc2;

	DmaPtr->Video.VideoInfo = XDc_GetNonLiveVideoAttribute(RunCfgPtr->Stream1Format);
	DmaPtr->Gfx.VideoInfo = XDc_GetNonLiveVideoAttribute(RunCfgPtr->Stream2Format);

	uint8_t vsync_int = 0;
	while (1) {
		int val =  XDcDma_ReadReg(DCDMA_BASEADDR, 0x70);
		if ((val & (XDCDMA_ISR_VSYNC1_INT_MASK | XDCDMA_ISR_VSYNC2_INT_MASK)) != 0U) {
			vsync_int++;
		}

		if (vsync_int > 2) {
			break ;
		}

	}

	DmaPtr->Video.Video_TriggerStatus = XDCDMA_TRIGGER_EN;
	DmaPtr->Gfx.Graphics_TriggerStatus = XDCDMA_TRIGGER_EN;
	XDcDma_VSyncHandler(DmaPtr);

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

	if (RunCfgPtr->DpTxEnable) {
		xil_printf("Enabling Output to DisplayPort\n");
		/* Initialize DpSubsystem */
		Status = InitDpPsuSubsystem(RunCfgPtr);
		if (Status != XST_SUCCESS) {
			xil_printf("DpPsu14 Subsystem Initialization failed\n");
			return Status;
		}

	}

	/* Initialize DcSubsystem */
	Status = InitDcSubsystem(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("FAILED to Initialize DC Subsystem Ip\n");
		return Status;
	}

	/* Configure Framebuffer */
	Status = ConfigFbWr(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("FAILED to Initialize FrmbufWr Ip\n");
		return Status;
	}

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
	RunCfgPtr->FrmbufPtr = &FrmbufWr;
	RunCfgPtr->IntrPtr = &IntrPtr;
	RunCfgPtr->Out_FbInfo = &OutFbPtr;
	RunCfgPtr->V1_FbInfo = &Video1FbPtr;
	RunCfgPtr->V2_FbInfo = &Video2FbPtr;
	RunCfgPtr->DpPsuPtr = &DpPsuPtr;
	RunCfgPtr->Desc1 = Desc1;
	RunCfgPtr->Desc2 = Desc2;

	// BaseAddress
	RunCfgPtr->DcBaseAddr = DC_BASEADDR;
	RunCfgPtr->DcDmaBaseAddr = DCDMA_BASEADDR;
	RunCfgPtr->FbBaseAddr = PL_VID_S0P0_FRMBUFWR0;

	RunCfgPtr->Width = 640;
	RunCfgPtr->Height = 480;

	RunCfgPtr->Stream1Format = RGBA8888;
	RunCfgPtr->Stream1Bpc = 4;
	RunCfgPtr->Stream2Format = RGBA8888;
	RunCfgPtr->Stream2Bpc = 4;

	RunCfgPtr->CursorEnable = CB_DISABLE;
	RunCfgPtr->OutStreamFormat = RGB_8BPC;

	/* Dp */
	RunCfgPtr->DpTxEnable = 1;

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

int main()
{
	u32 Status;

	Xil_DCacheDisable();
	Xil_ICacheDisable();

	Status = mmi_dc_nonlive_test(&RunCfg);
	if (Status != XST_SUCCESS) {
		xil_printf("MMI_DC_NONLIVE_TEST failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran MMI_DC_NONLIVE_TEST\r\n");

	return 0;

}
