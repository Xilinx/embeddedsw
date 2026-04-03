/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file mmi_dc_cursor.c
*
* This file contains cursor blend functionality implementation
*
******************************************************************************/

#include "mmi_dc_cursor.h"
#include "mmi_dc_setup_frames.h"
#include "xil_printf.h"

/*****************************************************************************/
/**
*
* This function initializes the cursor frame buffer
*
* @param    RunCfgPtr - Pointer to RunConfig structure
*
* @return   None
*
* @note     Sets up cursor frame buffer with RGBA4444 format
*
******************************************************************************/
void XDpDc_InitCursorFrameBuffer(RunConfig *RunCfgPtr)
{
	FrameInfo *Cursor_FbInfo;

	Cursor_FbInfo = RunCfgPtr->Cursor_FbInfo;
	Cursor_FbInfo->Address = CURSOR_BUFFER_ADDR;
	/* Cursor size is always 128x128 per hardware spec */
	Cursor_FbInfo->Width = 128;
	Cursor_FbInfo->Height = 128;
	Cursor_FbInfo->VideoFormat = RGBA4444;
	Cursor_FbInfo->Bpc = 2;
	XDpDc_GenerateFrameInfoAttribute(Cursor_FbInfo);

	xil_printf("  - Cursor frame buffer initialized: 128x128 (hardware fixed) @ 0x%llx\r\n",
		   (unsigned long long)Cursor_FbInfo->Address);
}

/*****************************************************************************/
/**
*
* This function fills the cursor buffer with a red circle pattern
*
* @param    RunCfgPtr - Pointer to RunConfig structure
*
* @return   None
*
* @note     RGBA4444 format: 16-bit per pixel, bit layout from MSB to LSB:
*           Bits 15-12: Alpha[3:0]
*           Bits 11-8:  Blue[3:0]
*           Bits 7-4:   Green[3:0]
*           Bits 3-0:   Red[3:0]
*

*           Red (opaque) = 0xF00F (A=F, B=0, G=0, R=F)
*           Transparent = 0x0000 (all zeros)
*
******************************************************************************/
void XDpDc_FillCursorBuffer(RunConfig *RunCfgPtr)
{
	(void)RunCfgPtr;
	xil_printf("\r\n=== Cursor Buffer Setup ===\r\n");
	xil_printf("  - Format: RGBA4444\r\n");
	xil_printf("  - Pattern: Red circle on transparent background\r\n");

	/* Hardware cursor is always 128x128 per spec */
	u32 width = 128;
	u32 height = 128;

	/* Draw a circle in the center - radius 50 pixels */
	u32 center_x = width / 2;
	u32 center_y = height / 2;
	/* 50 pixel radius for 100x100 visible cursor */
	u32 radius = 50;

	u16 red_opaque = 0xF00F;
	u16 transparent = 0x0000;
	u32 radius_squared = radius * radius;
	u32 red_count = 0;
	u32 transparent_count = 0;

	/*
	 * Pack two RGBA4444 pixels into each 32-bit write at 4-byte-aligned
	 * offsets.  Pixels are stored little-endian: pixel_even in bits [15:0],
	 * pixel_odd in bits [31:16].
	 */
	for (u32 y = 0; y < height; y++) {
		for (u32 x = 0; x < width; x += 2) {
			u16 p0, p1;
			s32 dx, dy;
			u32 d2;

			dx = (s32)x - (s32)center_x;
			dy = (s32)y - (s32)center_y;
			d2 = (u32)(dx * dx + dy * dy);
			if (d2 <= radius_squared) {
				p0 = red_opaque;
				red_count++;
			} else {
				p0 = transparent;
				transparent_count++;
			}

			dx = (s32)(x + 1) - (s32)center_x;
			d2 = (u32)(dx * dx + dy * dy);
			if (d2 <= radius_squared) {
				p1 = red_opaque;
				red_count++;
			} else {
				p1 = transparent;
				transparent_count++;
			}

			u32 word = ((u32)p1 << 16) | (u32)p0;
			u32 offset = (y * width + x) * 2;
			XDc_WriteReg(CURSOR_BUFFER_ADDR, offset, word);
		}
	}

	xil_printf("  - Hardware cursor size: 128x128 (fixed)\r\n");
	xil_printf("  - Visible circle: radius=%d pixels (center at 64,64)\r\n", radius);
	xil_printf("  - Red pixels (opaque): %d\r\n", red_count);
	xil_printf("  - Transparent pixels (alpha=0): %d\r\n", transparent_count);
	xil_printf("  - Cursor buffer generation complete\r\n\r\n");
}

/*****************************************************************************/
/**
*
* This function configures cursor blend parameters using DCsub APIs
*
* @param    RunCfgPtr - Pointer to RunConfig structure
*
* @return   None
*
* @note     Uses XDcSub_SetCursorBlend and XDcSub_SetSdpCursorBuffers APIs
*           to configure cursor parameters. Must be called BEFORE hardware init.

*           Hardware registers are written during XDcSub_ConfigureDcVideo()
*
******************************************************************************/
void XDpDc_ConfigureCursorBlend(RunConfig *RunCfgPtr)
{
	u32 Status;
	static XDc_Cursor CursorConfig;

	/* Populate cursor configuration from RunConfig */
	CursorConfig.CoordX = RunCfgPtr->CursorCoordX;
	CursorConfig.CoordY = RunCfgPtr->CursorCoordY;
	CursorConfig.SizeX = RunCfgPtr->CursorSizeX;
	CursorConfig.SizeY = RunCfgPtr->CursorSizeY;
	CursorConfig.CursorAttribute = (XDc_VideoAttribute *)XDc_GetNonLiveVideoAttribute(RGBA4444);

	/* Use DCsub API to configure cursor blend */
	Status = XDcSub_SetCursorBlend(RunCfgPtr->DcSubPtr, CB_ENABLE, &CursorConfig);
	if (Status != XST_SUCCESS) {
		xil_printf("ERROR: Failed to configure cursor blend\r\n");
		return;
	}

	/* Use DCsub API to configure SDP channel for cursor */
	Status = XDcSub_SetSdpCursorBuffers(RunCfgPtr->DcSubPtr, 0x1, 0x3);
	if (Status != XST_SUCCESS) {
		xil_printf("ERROR: Failed to configure SDP cursor buffers\r\n");
		return;
	}

	xil_printf("  - Cursor blend configured: Position(%d,%d) Size(%dx%d)\r\n",
		   RunCfgPtr->CursorCoordX, RunCfgPtr->CursorCoordY,
		   RunCfgPtr->CursorSizeX, RunCfgPtr->CursorSizeY);
	xil_printf("  - SDP channel configured: Enable=0x1, BurstLen=0x3\r\n");
}

/*****************************************************************************/
/**
*
* This function configures cursor DMA channel
*
* @param    RunCfgPtr - Pointer to RunConfig structure
*
* @return   None
*
* @note     Sets up SDP DMA channel for cursor data transfer
*
******************************************************************************/
void XDpDc_ConfigureCursorDMA(RunConfig *RunCfgPtr)
{
	XDcDma *DmaPtr = RunCfgPtr->DcSubPtr->DmaPtr;

	DmaPtr->SDP.Channel.Current = RunCfgPtr->Desc7;
	DmaPtr->SDP.SDP_TriggerStatus = XDCDMA_TRIGGER_EN;

	xil_printf("  - Cursor DMA configured\r\n");
}
