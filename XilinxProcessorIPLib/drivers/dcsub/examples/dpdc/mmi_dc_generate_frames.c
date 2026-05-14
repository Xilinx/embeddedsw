/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file mmi_dc_generate_frames.c
*
* This file contains frame buffer pattern generation implementations for all
* 80 non-live video formats as per DC pixel data format specification
*
******************************************************************************/

#include "mmi_dc_generate_frames.h"
#include "xil_printf.h"
#include "xdc.h"

/* SMPTE color bar values in RGB (R, G, B) */
static const u8 SMPTE_RGB[7][3] = {
	{255, 255, 255}, /* White */
	{255, 255, 0},   /* Yellow */
	{0, 255, 255},   /* Cyan */
	{0, 255, 0},     /* Green */
	{255, 0, 255},   /* Magenta */
	{255, 0, 0},     /* Red */
	{0, 0, 255}      /* Blue */
};

/* SMPTE color bar values in YUV (Y, Cb, Cr) */
static const u8 SMPTE_YUV[7][3] = {
	{235, 128, 128}, /* White */
	{210, 16, 146},  /* Yellow */
	{170, 166, 16},  /* Cyan */
	{145, 54, 34},   /* Green */
	{106, 202, 222}, /* Magenta */
	{81, 90, 240},   /* Red */
	{41, 240, 110}   /* Blue */
};

/* SMPTE color bar values for 10-bit formats (scaled from 8-bit) */
static const u16 SMPTE_YUV_10BPC[7][3] = {
	{940, 512, 512},   /* White (235*4, 128*4, 128*4) */
	{840, 64, 584},    /* Yellow (210*4, 16*4, 146*4) */
	{680, 664, 64},    /* Cyan (170*4, 166*4, 16*4) */
	{580, 216, 136},   /* Green (145*4, 54*4, 34*4) */
	{424, 808, 888},   /* Magenta (106*4, 202*4, 222*4) */
	{324, 360, 960},   /* Red (81*4, 90*4, 240*4) */
	{164, 960, 440}    /* Blue (41*4, 240*4, 110*4) */
};

/* SMPTE color bar values for 12-bit formats (scaled from 8-bit) */
static const u16 SMPTE_YUV_12BPC[7][3] = {
	{3760, 2048, 2048}, /* White (235*16, 128*16, 128*16) */
	{3360, 256, 2336},  /* Yellow (210*16, 16*16, 146*16) */
	{2720, 2656, 256},  /* Cyan (170*16, 166*16, 16*16) */
	{2320, 864, 544},   /* Green (145*16, 54*16, 34*16) */
	{1696, 3232, 3552}, /* Magenta (106*16, 202*16, 222*16) */
	{1296, 1440, 3840}, /* Red (81*16, 90*16, 240*16) */
	{656, 3840, 1760}   /* Blue (41*16, 240*16, 110*16) */
};

/* SMPTE grayscale bar values (Y-only) */
static const u8 SMPTE_GRAY_8BPC[7] = {235, 210, 170, 145, 106, 81, 41};
static const u16 SMPTE_GRAY_10BPC[7] = {940, 840, 680, 580, 424, 324, 164};
static const u16 SMPTE_GRAY_12BPC[7] = {3760, 3360, 2720, 2320, 1696, 1296, 656};

/*****************************************************************************/
/**
*
* This function generates frame buffer data based on video format
*
* @param    RunCfgPtr - Pointer to RunConfig structure
*
* @return   None
*
* @note     Generates pixel data according to DMA pixel data format spec

*           V1 buffer (Stream1): SMPTE vertical bars (left-to-right)
*           V4 buffer (Stream2): SMPTE horizontal bars (top-to-bottom)
*           All 80 non-live video formats supported
*
******************************************************************************/
void XDpDc_GenerateFrames(RunConfig *RunCfgPtr)
{
	XDc_VideoFormat fmt = RunCfgPtr->Stream1Format;
	u32 stream2_rgba_alpha = 0xFF000000U;
	u32 height = RunCfgPtr->Height;
	u32 width = RunCfgPtr->Width;
	/* For V1 vertical bars */
	u32 bar_width = width / 7;
	/* For V4 horizontal bars */
	u32 bar_height = height / 7;

	/*
	 * In mixed mode, make Stream2 (non-live) semi-transparent for RGBA paths
	 * so Stream1 live content remains visible through the blend.
	 */
	if (RunCfgPtr->presentationmode == XDCSUB_PPTMODE_MIXED) {
		stream2_rgba_alpha = 0x40000000U;
	}

	xil_printf("\r\n=== Video Frame Generation ===\r\n");
	xil_printf("  Format: %d\r\n", fmt);
	xil_printf("  Resolution: %dx%d\r\n", width, height);
	xil_printf("  V1 (Stream1): Vertical SMPTE bars\r\n");
	xil_printf("  V4 (Stream2): Horizontal SMPTE bars\r\n");

	switch (fmt) {
		/* ===== YUV 4:2:2 Interleaved Formats ===== */

	case CbY0CrY1: /* Format 0 - Cb[7:0]Y(n)[7:0]Cr[7:0]Y(n+1)[7:0] */
		for (u32 y = 0; y < height; y++) {
			for (u32 x = 0; x < width; x += 2) {
				u32 bar_v1 = (x / bar_width >= 7) ? 6 : (x / bar_width);
				u32 bar_v2 = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u32 offset = (y * width + x) * 2;
				u32 val_v1 = SMPTE_YUV[bar_v1][1] | (SMPTE_YUV[bar_v1][0] << 8) |
					     (SMPTE_YUV[bar_v1][2] << 16) | (SMPTE_YUV[bar_v1][0] << 24);
				u32 val_v2 = SMPTE_YUV[bar_v2][1] | (SMPTE_YUV[bar_v2][0] << 8) |
					     (SMPTE_YUV[bar_v2][2] << 16) | (SMPTE_YUV[bar_v2][0] << 24);
				XDc_WriteReg(IN_BUFFER_0_ADDR_V1, offset, val_v1);
				XDc_WriteReg(IN_BUFFER_0_ADDR_V4, offset, val_v2);
			}
		}
		break;

	case CrY0CbY1: /* Format 1 - Cr[7:0]Y(n)[7:0]Cb(n)[7:0]Y(n+1)[7:0] */
		for (u32 y = 0; y < height; y++) {
			for (u32 x = 0; x < width; x += 2) {
				u32 bar_v1 = (x / bar_width >= 7) ? 6 : (x / bar_width);
				u32 bar_v2 = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u32 offset = (y * width + x) * 2;
				u32 val_v1 = SMPTE_YUV[bar_v1][2] | (SMPTE_YUV[bar_v1][0] << 8) |
					     (SMPTE_YUV[bar_v1][1] << 16) | (SMPTE_YUV[bar_v1][0] << 24);
				u32 val_v2 = SMPTE_YUV[bar_v2][2] | (SMPTE_YUV[bar_v2][0] << 8) |
					     (SMPTE_YUV[bar_v2][1] << 16) | (SMPTE_YUV[bar_v2][0] << 24);
				XDc_WriteReg(IN_BUFFER_0_ADDR_V1, offset, val_v1);
				XDc_WriteReg(IN_BUFFER_0_ADDR_V4, offset, val_v2);
			}
		}
		break;

	case Y0CrY1Cb: /* Format 2 - Y(n)[7:0]Cr[7:0]Y(n+1)[7:0]Cb[7:0] */
		for (u32 y = 0; y < height; y++) {
			for (u32 x = 0; x < width; x += 2) {
				u32 bar_v1 = (x / bar_width >= 7) ? 6 : (x / bar_width);
				u32 bar_v2 = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u32 offset = (y * width + x) * 2;
				u32 val_v1 = SMPTE_YUV[bar_v1][0] | (SMPTE_YUV[bar_v1][2] << 8) |
					     (SMPTE_YUV[bar_v1][0] << 16) | (SMPTE_YUV[bar_v1][1] << 24);
				u32 val_v2 = SMPTE_YUV[bar_v2][0] | (SMPTE_YUV[bar_v2][2] << 8) |
					     (SMPTE_YUV[bar_v2][0] << 16) | (SMPTE_YUV[bar_v2][1] << 24);
				XDc_WriteReg(IN_BUFFER_0_ADDR_V1, offset, val_v1);
				XDc_WriteReg(IN_BUFFER_0_ADDR_V4, offset, val_v2);
			}
		}
		break;

	case Y0CbY1Cr: /* Format 3 - Y(n)[7:0]Cb(n)[7:0]Y(n+1)[7:0]Cr(n)[7:0] */
		for (u32 y = 0; y < height; y++) {
			for (u32 x = 0; x < width; x += 2) {
				u32 bar_v1 = (x / bar_width >= 7) ? 6 : (x / bar_width);
				u32 bar_v2 = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u32 offset = (y * width + x) * 2;
				u32 val_v1 = SMPTE_YUV[bar_v1][0] | (SMPTE_YUV[bar_v1][1] << 8) |
					     (SMPTE_YUV[bar_v1][0] << 16) | (SMPTE_YUV[bar_v1][2] << 24);
				u32 val_v2 = SMPTE_YUV[bar_v2][0] | (SMPTE_YUV[bar_v2][1] << 8) |
					     (SMPTE_YUV[bar_v2][0] << 16) | (SMPTE_YUV[bar_v2][2] << 24);
				XDc_WriteReg(IN_BUFFER_0_ADDR_V1, offset, val_v1);
				XDc_WriteReg(IN_BUFFER_0_ADDR_V4, offset, val_v2);
			}
		}
		break;

		/* ===== YUV 4:2:2 Planar Format ===== */

		case YV16: /* Format 4 - Y/Cb/Cr Planar 4:2:2 8-bit (V2=Cb, V3=Cr, chroma half-width) */
		{
			u32 y_line = width;
			u32 y_stride = y_line;
			if (y_stride % 256 != 0)
				y_stride = (y_stride / 256 + 1) * 256;
			u32 c_line = width / 2;
			u32 c_stride = c_line;
			if (c_stride % 256 != 0)
				c_stride = (c_stride / 256 + 1) * 256;

			for (u32 y = 0; y < height; y++) {
				u32 y_off = y * y_stride;
				u32 c_off = y * c_stride;
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);

				/* Y plane: full width */
				for (u32 x = 0; x < width; x += 4) {
					u32 b0 = ((x+0)/bar_width >= 7) ? 6 : ((x+0)/bar_width);
					u32 b1 = ((x+1)/bar_width >= 7) ? 6 : ((x+1)/bar_width);
					u32 b2 = ((x+2)/bar_width >= 7) ? 6 : ((x+2)/bar_width);
					u32 b3 = ((x+3)/bar_width >= 7) ? 6 : ((x+3)/bar_width);
					u32 y_word = SMPTE_YUV[b0][0] | (SMPTE_YUV[b1][0]<<8) |
						     (SMPTE_YUV[b2][0]<<16) | (SMPTE_YUV[b3][0]<<24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, y_off + x, y_word);
					u32 yh = SMPTE_YUV[bh][0];
					u32 yw2 = yh | (yh<<8) | (yh<<16) | (yh<<24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, y_off + x, yw2);
				}

				/* Cb/Cr planes: half width (one chroma sample per 2 luma samples) */
				for (u32 x = 0; x < width / 2; x += 4) {
					u32 lx = x * 2;
					u32 b0 = ((lx+0)/bar_width >= 7) ? 6 : ((lx+0)/bar_width);
					u32 b1 = ((lx+2)/bar_width >= 7) ? 6 : ((lx+2)/bar_width);
					u32 b2 = ((lx+4)/bar_width >= 7) ? 6 : ((lx+4)/bar_width);
					u32 b3 = ((lx+6)/bar_width >= 7) ? 6 : ((lx+6)/bar_width);
					u32 cb_word = SMPTE_YUV[b0][1] | (SMPTE_YUV[b1][1]<<8) |
						      (SMPTE_YUV[b2][1]<<16) | (SMPTE_YUV[b3][1]<<24);
					u32 cr_word = SMPTE_YUV[b0][2] | (SMPTE_YUV[b1][2]<<8) |
						      (SMPTE_YUV[b2][2]<<16) | (SMPTE_YUV[b3][2]<<24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V2, c_off + x, cb_word);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V3, c_off + x, cr_word);
					u32 cbh = SMPTE_YUV[bh][1];
					u32 crh = SMPTE_YUV[bh][2];
					u32 cbw2 = cbh | (cbh<<8) | (cbh<<16) | (cbh<<24);
					u32 crw2 = crh | (crh<<8) | (crh<<16) | (crh<<24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V5, c_off + x, cbw2);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V6, c_off + x, crw2);
				}
			}
		}
			break;

		/* ===== YUV 4:4:4 Planar Formats ===== */

		case YV24: /* Format 5 - Y/Cb/Cr Planar 4:4:4 8-bit (V2=Cb, V3=Cr per CSC matrix) */
		{
			u32 line_bytes = width;
			u32 stride_bytes = line_bytes;
			if (stride_bytes % 256 != 0) {
				stride_bytes = (stride_bytes / 256 + 1) * 256;
			}

			for (u32 y = 0; y < height; y++) {
				u32 line_off = y * stride_bytes;
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u32 yh  = SMPTE_YUV[bh][0];
				u32 cbh = SMPTE_YUV[bh][1];
				u32 crh = SMPTE_YUV[bh][2];
				u32 y_word2  = yh  | (yh  << 8) | (yh  << 16) | (yh  << 24);
				u32 cb_word2 = cbh | (cbh << 8) | (cbh << 16) | (cbh << 24);
				u32 cr_word2 = crh | (crh << 8) | (crh << 16) | (crh << 24);

				for (u32 x = 0; x < width; x += 4) {
					u32 b0 = ((x+0)/bar_width >= 7) ? 6 : ((x+0)/bar_width);
					u32 b1 = ((x+1)/bar_width >= 7) ? 6 : ((x+1)/bar_width);
					u32 b2 = ((x+2)/bar_width >= 7) ? 6 : ((x+2)/bar_width);
					u32 b3 = ((x+3)/bar_width >= 7) ? 6 : ((x+3)/bar_width);

					u32 y_word  = SMPTE_YUV[b0][0] | (SMPTE_YUV[b1][0]<<8) |
						      (SMPTE_YUV[b2][0]<<16) | (SMPTE_YUV[b3][0]<<24);
					u32 cb_word = SMPTE_YUV[b0][1] | (SMPTE_YUV[b1][1]<<8) |
						      (SMPTE_YUV[b2][1]<<16) | (SMPTE_YUV[b3][1]<<24);
					u32 cr_word = SMPTE_YUV[b0][2] | (SMPTE_YUV[b1][2]<<8) |
						      (SMPTE_YUV[b2][2]<<16) | (SMPTE_YUV[b3][2]<<24);

					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, line_off + x, y_word);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V2, line_off + x, cb_word);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V3, line_off + x, cr_word);

					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, line_off + x, y_word2);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V5, line_off + x, cb_word2);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V6, line_off + x, cr_word2);
				}
			}
		}
			break;

		/* ===== YUV 4:2:2 Semi-Planar Formats ===== */

		case YV16Ci: /* Format 6 - YV16CI: V1=Y, V2=CrCb (semi-planar) */
		{
			/* BPC=1: 640 Y samples = 640 bytes */
			u32 line_bytes = width;
			u32 stride_bytes = line_bytes;
			if (stride_bytes % 256 != 0) {
				stride_bytes = (stride_bytes / 256 + 1) * 256;
			}
			for (u32 y = 0; y < height; y++) {
				u32 line_off = y * stride_bytes;
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u32 yh = SMPTE_YUV[bh][0];
				u32 y_hword = yh | (yh << 8) | (yh << 16) | (yh << 24);
				u32 c_hword = SMPTE_YUV[bh][2] | (SMPTE_YUV[bh][1] << 8) |
					      (SMPTE_YUV[bh][2] << 16) | (SMPTE_YUV[bh][1] << 24);
				for (u32 x = 0; x + 4 <= width; x += 4) {
					u32 b0 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 b1 = ((x + 1) / bar_width >= 7) ? 6 : ((x + 1) / bar_width);
					u32 b2 = ((x + 2) / bar_width >= 7) ? 6 : ((x + 2) / bar_width);
					u32 b3 = ((x + 3) / bar_width >= 7) ? 6 : ((x + 3) / bar_width);
					u32 y_word = SMPTE_YUV[b0][0] | (SMPTE_YUV[b1][0] << 8) |
						     (SMPTE_YUV[b2][0] << 16) | (SMPTE_YUV[b3][0] << 24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, line_off + x, y_word);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, line_off + x, y_hword);

					u32 cb0 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 cb1 = ((x + 2) / bar_width >= 7) ? 6 : ((x + 2) / bar_width);
					u32 c_word = SMPTE_YUV[cb0][2] | (SMPTE_YUV[cb0][1] << 8) |
						     (SMPTE_YUV[cb1][2] << 16) | (SMPTE_YUV[cb1][1] << 24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V2, line_off + x, c_word);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V5, line_off + x, c_hword);
				}
			}
		}
			break;

		case MONOCHROME: /* Format 7 - Y only 8-bit (32-bit packed writes) */
		{
			/* BPC=1 */
			u32 line_bytes = width;
			u32 stride_bytes = line_bytes;
			if (stride_bytes % 256 != 0) {
				stride_bytes = (stride_bytes / 256 + 1) * 256;
			}
			for (u32 y = 0; y < height; y++) {
				u32 line_off = y * stride_bytes;
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u32 yh = SMPTE_GRAY_8BPC[bh];
				u32 y_word2 = yh | (yh << 8) | (yh << 16) | (yh << 24);
				for (u32 x = 0; x < width; x += 4) {
					u32 b;
					b = ((x + 0) / bar_width >= 7) ? 6 : ((x + 0) / bar_width);
					u8 g0 = SMPTE_GRAY_8BPC[b];
					b = ((x + 1) / bar_width >= 7) ? 6 : ((x + 1) / bar_width);
					u8 g1 = SMPTE_GRAY_8BPC[b];
					b = ((x + 2) / bar_width >= 7) ? 6 : ((x + 2) / bar_width);
					u8 g2 = SMPTE_GRAY_8BPC[b];
					b = ((x + 3) / bar_width >= 7) ? 6 : ((x + 3) / bar_width);
					u8 g3 = SMPTE_GRAY_8BPC[b];
					u32 y_word = g0 | (g1 << 8) | (g2 << 16) | (g3 << 24);

					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, line_off + x, y_word);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, line_off + x, y_word2);
				}
			}
		}
			break;

		case YV16Ci2: /* Format 8 - YV16CI2: V1=Y, V2=CbCr (semi-planar, swapped chroma) */
		{
			/* BPC=1: 640 Y samples = 640 bytes */
			u32 line_bytes = width;
			u32 stride_bytes = line_bytes;
			if (stride_bytes % 256 != 0) {
				stride_bytes = (stride_bytes / 256 + 1) * 256;
			}
			for (u32 y = 0; y < height; y++) {
				u32 line_off = y * stride_bytes;
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u32 yh = SMPTE_YUV[bh][0];
				u32 y_hword = yh | (yh << 8) | (yh << 16) | (yh << 24);
				u32 c_hword = SMPTE_YUV[bh][1] | (SMPTE_YUV[bh][2] << 8) |
					      (SMPTE_YUV[bh][1] << 16) | (SMPTE_YUV[bh][2] << 24);
				for (u32 x = 0; x + 4 <= width; x += 4) {
					u32 b0 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 b1 = ((x + 1) / bar_width >= 7) ? 6 : ((x + 1) / bar_width);
					u32 b2 = ((x + 2) / bar_width >= 7) ? 6 : ((x + 2) / bar_width);
					u32 b3 = ((x + 3) / bar_width >= 7) ? 6 : ((x + 3) / bar_width);
					u32 y_word = SMPTE_YUV[b0][0] | (SMPTE_YUV[b1][0] << 8) |
						     (SMPTE_YUV[b2][0] << 16) | (SMPTE_YUV[b3][0] << 24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, line_off + x, y_word);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, line_off + x, y_hword);

					u32 cb0 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 cb1 = ((x + 2) / bar_width >= 7) ? 6 : ((x + 2) / bar_width);
					u32 c_word = SMPTE_YUV[cb0][1] | (SMPTE_YUV[cb0][2] << 8) |
						     (SMPTE_YUV[cb1][1] << 16) | (SMPTE_YUV[cb1][2] << 24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V2, line_off + x, c_word);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V5, line_off + x, c_hword);
				}
			}
		}
			break;

		case YUV444: /* Format 9 - YCbCr 4:4:4 Interleaved 8-bit: Cb[7:0]Cr[7:0]Y[7:0] */
		{
			u32 line_bytes = width * 3;
			u32 stride_bytes = line_bytes;
			if (stride_bytes % 256 != 0) {
				stride_bytes = (stride_bytes / 256 + 1) * 256;
			}
			for (u32 y = 0; y < height; y++) {
				u32 line_off = y * stride_bytes;
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u32 hw0 = SMPTE_YUV[bh][1] | (SMPTE_YUV[bh][2] << 8) |
					  (SMPTE_YUV[bh][0] << 16) | (SMPTE_YUV[bh][1] << 24);
				u32 hw1 = SMPTE_YUV[bh][2] | (SMPTE_YUV[bh][0] << 8) |
					  (SMPTE_YUV[bh][1] << 16) | (SMPTE_YUV[bh][2] << 24);
				u32 hw2 = SMPTE_YUV[bh][0] | (SMPTE_YUV[bh][1] << 8) |
					  (SMPTE_YUV[bh][2] << 16) | (SMPTE_YUV[bh][0] << 24);
				for (u32 x = 0; x + 4 <= width; x += 4) {
					u32 b0 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 b1 = ((x + 1) / bar_width >= 7) ? 6 : ((x + 1) / bar_width);
					u32 b2 = ((x + 2) / bar_width >= 7) ? 6 : ((x + 2) / bar_width);
					u32 b3 = ((x + 3) / bar_width >= 7) ? 6 : ((x + 3) / bar_width);
					u32 off = line_off + x * 3;

					u32 w0 = SMPTE_YUV[b0][1] | (SMPTE_YUV[b0][2] << 8) |
						 (SMPTE_YUV[b0][0] << 16) | (SMPTE_YUV[b1][1] << 24);
					u32 w1 = SMPTE_YUV[b1][2] | (SMPTE_YUV[b1][0] << 8) |
						 (SMPTE_YUV[b2][1] << 16) | (SMPTE_YUV[b2][2] << 24);
					u32 w2 = SMPTE_YUV[b2][0] | (SMPTE_YUV[b3][1] << 8) |
						 (SMPTE_YUV[b3][2] << 16) | (SMPTE_YUV[b3][0] << 24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, off, w0);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, off + 4, w1);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, off + 8, w2);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, off, hw0);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, off + 4, hw1);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, off + 8, hw2);
				}
			}
		}
			break;

		case RGBA8880: /* Format 11 - 24-bit RGB packed (BPC=3, like RGB888) */
		case RGB888: /* Format 10 - RGB888: B[7:0]G[7:0]R[7:0] packed */
			for (u32 y = 0; y < height; y++) {
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u32 hw0 = SMPTE_RGB[bh][2] | (SMPTE_RGB[bh][1] << 8) |
					  (SMPTE_RGB[bh][0] << 16) | (SMPTE_RGB[bh][2] << 24);
				u32 hw1 = SMPTE_RGB[bh][1] | (SMPTE_RGB[bh][0] << 8) |
					  (SMPTE_RGB[bh][2] << 16) | (SMPTE_RGB[bh][1] << 24);
				u32 hw2 = SMPTE_RGB[bh][0] | (SMPTE_RGB[bh][2] << 8) |
					  (SMPTE_RGB[bh][1] << 16) | (SMPTE_RGB[bh][0] << 24);
				for (u32 x = 0; x + 4 <= width; x += 4) {
					u32 b0 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 b1 = ((x + 1) / bar_width >= 7) ? 6 : ((x + 1) / bar_width);
					u32 b2 = ((x + 2) / bar_width >= 7) ? 6 : ((x + 2) / bar_width);
					u32 b3 = ((x + 3) / bar_width >= 7) ? 6 : ((x + 3) / bar_width);
					u32 off = (y * width + x) * 3;

					u32 w0 = SMPTE_RGB[b0][2] | (SMPTE_RGB[b0][1] << 8) |
						 (SMPTE_RGB[b0][0] << 16) | (SMPTE_RGB[b1][2] << 24);
					u32 w1 = SMPTE_RGB[b1][1] | (SMPTE_RGB[b1][0] << 8) |
						 (SMPTE_RGB[b2][2] << 16) | (SMPTE_RGB[b2][1] << 24);
					u32 w2 = SMPTE_RGB[b2][0] | (SMPTE_RGB[b3][2] << 8) |
						 (SMPTE_RGB[b3][1] << 16) | (SMPTE_RGB[b3][0] << 24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, off, w0);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, off + 4, w1);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, off + 8, w2);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, off, hw0);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, off + 4, hw1);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, off + 8, hw2);
				}
			}
			break;

		case RGB888_10BPC: /* Format 12 - RGB 10-bit packed in 32-bit: B[9:0]G[9:0]R[9:0] */
		{
			u32 stride_bytes = width * 4;
			if (stride_bytes % 256 != 0)
				stride_bytes = (stride_bytes / 256 + 1) * 256;
			for (u32 y = 0; y < height; y++) {
				u32 line_off = y * stride_bytes;
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				for (u32 x = 0; x < width; x++) {
					u32 bv = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 c1 = (SMPTE_RGB[bv][2] * 4) |
						 ((SMPTE_RGB[bv][1] * 4) << 10) |
						 ((SMPTE_RGB[bv][0] * 4) << 20);
					u32 c2 = (SMPTE_RGB[bh][2] * 4) |
						 ((SMPTE_RGB[bh][1] * 4) << 10) |
						 ((SMPTE_RGB[bh][0] * 4) << 20);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, line_off + x * 4, c1);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, line_off + x * 4, c2);
				}
			}
		}
			break;

		case YUV444_10BPC: /* Format 13 - YCbCr 4:4:4 10-bit packed: Cb[9:0]Cr[9:0]Y[9:0] */
		{
			u32 stride_bytes = width * 4;
			if (stride_bytes % 256 != 0)
				stride_bytes = (stride_bytes / 256 + 1) * 256;
			for (u32 y = 0; y < height; y++) {
				u32 line_off = y * stride_bytes;
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				for (u32 x = 0; x < width; x++) {
					u32 bv = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 c1 = SMPTE_YUV_10BPC[bv][1] |
						 (SMPTE_YUV_10BPC[bv][2] << 10) |
						 (SMPTE_YUV_10BPC[bv][0] << 20);
					u32 c2 = SMPTE_YUV_10BPC[bh][1] |
						 (SMPTE_YUV_10BPC[bh][2] << 10) |
						 (SMPTE_YUV_10BPC[bh][0] << 20);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, line_off + x * 4, c1);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, line_off + x * 4, c2);
				}
			}
		}
			break;

		case YV16Ci2_10BPC: /* Format 14 - 30-bit packed: V1=Y, V2=CbCr (Cb first) */
		case YV16Ci_10BPC: /* Format 15 - 30-bit packed: V1=Y, V2=CrCb (Cr first) */
		{
			int first_comp = (fmt == YV16Ci_10BPC) ? 2 : 1;
			int second_comp = (fmt == YV16Ci_10BPC) ? 1 : 2;

			u32 line_size = (width * 4) / 3;
			if (line_size % 16 != 0)
				line_size = ((line_size / 16) + 1) * 16;
			u32 stride_bytes = ((line_size + 255) / 256) * 256;

			for (u32 y = 0; y < height; y++) {
				u32 line_off = y * stride_bytes;
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);

				/* Y plane: 3 x 10-bit samples per 32-bit word */
				u32 woff = 0;
				for (u32 x = 0; x < width; x += 3) {
					u32 b0 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 s0 = SMPTE_YUV_10BPC[b0][0];
					u32 s1 = 0, s2 = 0;
					if (x + 1 < width) {
						u32 b1 = ((x+1) / bar_width >= 7) ? 6 : ((x+1) / bar_width);
						s1 = SMPTE_YUV_10BPC[b1][0];
					}
					if (x + 2 < width) {
						u32 b2 = ((x+2) / bar_width >= 7) ? 6 : ((x+2) / bar_width);
						s2 = SMPTE_YUV_10BPC[b2][0];
					}
					u32 yw = (s0 & 0x3FF) | ((s1 & 0x3FF) << 10) | ((s2 & 0x3FF) << 20);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, line_off + woff, yw);

					u32 yh = SMPTE_YUV_10BPC[bh][0];
					u32 ywh = (yh & 0x3FF) | ((yh & 0x3FF) << 10) | ((yh & 0x3FF) << 20);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, line_off + woff, ywh);
					woff += 4;
				}

				/* CbCr plane: 3 interleaved chroma samples per 32-bit word
				 * 640 total samples (320 pairs): Cb/Cr alternating */
				woff = 0;
				u32 total_chroma = width;
				for (u32 si = 0; si < total_chroma; si += 3) {
					u32 v0 = 0, v1 = 0, v2 = 0;
					u32 pair, lx, bv;

					pair = si / 2;
					lx = pair * 2;
					bv = (lx / bar_width >= 7) ? 6 : (lx / bar_width);
					v0 = (si % 2 == 0) ? SMPTE_YUV_10BPC[bv][first_comp]
							    : SMPTE_YUV_10BPC[bv][second_comp];

					if (si + 1 < total_chroma) {
						pair = (si + 1) / 2;
						lx = pair * 2;
						bv = (lx / bar_width >= 7) ? 6 : (lx / bar_width);
						v1 = ((si + 1) % 2 == 0) ? SMPTE_YUV_10BPC[bv][first_comp]
									  : SMPTE_YUV_10BPC[bv][second_comp];
					}
					if (si + 2 < total_chroma) {
						pair = (si + 2) / 2;
						lx = pair * 2;
						bv = (lx / bar_width >= 7) ? 6 : (lx / bar_width);
						v2 = ((si + 2) % 2 == 0) ? SMPTE_YUV_10BPC[bv][first_comp]
									  : SMPTE_YUV_10BPC[bv][second_comp];
					}

					u32 cw = (v0 & 0x3FF) | ((v1 & 0x3FF) << 10) | ((v2 & 0x3FF) << 20);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V2, line_off + woff, cw);

					u32 h0 = (si % 2 == 0) ? SMPTE_YUV_10BPC[bh][first_comp]
								: SMPTE_YUV_10BPC[bh][second_comp];
					u32 h1 = ((si + 1) % 2 == 0) ? SMPTE_YUV_10BPC[bh][first_comp]
								      : SMPTE_YUV_10BPC[bh][second_comp];
					u32 h2 = ((si + 2) % 2 == 0) ? SMPTE_YUV_10BPC[bh][first_comp]
								      : SMPTE_YUV_10BPC[bh][second_comp];
					u32 cwh = (h0 & 0x3FF) | ((h1 & 0x3FF) << 10) | ((h2 & 0x3FF) << 20);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V5, line_off + woff, cwh);

					woff += 4;
				}
			}
		}
			break;

		case YV16_10BPC: /* Format 16 - Planar 4:2:2 10-bit: V1=Y, V2=Cb, V3=Cr (chroma half-width) */
		{
			u32 y_stride = width * 2;
			if (y_stride % 256 != 0)
				y_stride = (y_stride / 256 + 1) * 256;
			u32 c_line = (width / 2) * 2;
			u32 c_stride = c_line;
			if (c_stride % 256 != 0)
				c_stride = (c_stride / 256 + 1) * 256;

			for (u32 y = 0; y < height; y++) {
				u32 y_off = y * y_stride;
				u32 c_off = y * c_stride;
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);

				for (u32 x = 0; x < width; x += 2) {
					u32 bv0 = ((x+0)/bar_width >= 7) ? 6 : ((x+0)/bar_width);
					u32 bv1 = ((x+1)/bar_width >= 7) ? 6 : ((x+1)/bar_width);
					u32 yw = SMPTE_YUV_10BPC[bv0][0] | (SMPTE_YUV_10BPC[bv1][0] << 16);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, y_off + x * 2, yw);
					u32 yw2 = SMPTE_YUV_10BPC[bh][0] | (SMPTE_YUV_10BPC[bh][0] << 16);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, y_off + x * 2, yw2);
				}

				for (u32 x = 0; x < width / 2; x += 2) {
					u32 lx = x * 2;
					u32 b0 = ((lx+0)/bar_width >= 7) ? 6 : ((lx+0)/bar_width);
					u32 b1 = ((lx+2)/bar_width >= 7) ? 6 : ((lx+2)/bar_width);
					u32 cbw = SMPTE_YUV_10BPC[b0][1] | (SMPTE_YUV_10BPC[b1][1] << 16);
					u32 crw = SMPTE_YUV_10BPC[b0][2] | (SMPTE_YUV_10BPC[b1][2] << 16);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V2, c_off + x * 2, cbw);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V3, c_off + x * 2, crw);
					u32 cbw2 = SMPTE_YUV_10BPC[bh][1] | (SMPTE_YUV_10BPC[bh][1] << 16);
					u32 crw2 = SMPTE_YUV_10BPC[bh][2] | (SMPTE_YUV_10BPC[bh][2] << 16);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V5, c_off + x * 2, cbw2);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V6, c_off + x * 2, crw2);
				}
			}
		}
			break;

		case YV24_10BPC: /* Format 17 - Planar 4:4:4 10-bit: V1=Y, V2=Cb, V3=Cr (full-width) */
		{
			u32 plane_stride = width * 2;
			if (plane_stride % 256 != 0)
				plane_stride = (plane_stride / 256 + 1) * 256;

			for (u32 y = 0; y < height; y++) {
				u32 line_off = y * plane_stride;
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				for (u32 x = 0; x < width; x += 2) {
					u32 bv0 = ((x+0)/bar_width >= 7) ? 6 : ((x+0)/bar_width);
					u32 bv1 = ((x+1)/bar_width >= 7) ? 6 : ((x+1)/bar_width);
					u32 off = line_off + x * 2;

					u32 yw  = SMPTE_YUV_10BPC[bv0][0] | (SMPTE_YUV_10BPC[bv1][0] << 16);
					u32 cbw = SMPTE_YUV_10BPC[bv0][1] | (SMPTE_YUV_10BPC[bv1][1] << 16);
					u32 crw = SMPTE_YUV_10BPC[bv0][2] | (SMPTE_YUV_10BPC[bv1][2] << 16);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, off, yw);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V2, off, cbw);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V3, off, crw);

					u32 yw2  = SMPTE_YUV_10BPC[bh][0] | (SMPTE_YUV_10BPC[bh][0] << 16);
					u32 cbw2 = SMPTE_YUV_10BPC[bh][1] | (SMPTE_YUV_10BPC[bh][1] << 16);
					u32 crw2 = SMPTE_YUV_10BPC[bh][2] | (SMPTE_YUV_10BPC[bh][2] << 16);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, off, yw2);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V5, off, cbw2);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V6, off, crw2);
				}
			}
		}
			break;

		case MONOCHROME_10BPC: /* Format 18 - Y only 10-bit (32-bit packed, 2 samples per write) */
		{
			u32 stride_bytes = width * 2;
			if (stride_bytes % 256 != 0)
				stride_bytes = (stride_bytes / 256 + 1) * 256;
			for (u32 y = 0; y < height; y++) {
				u32 line_off = y * stride_bytes;
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u32 yh = SMPTE_GRAY_10BPC[bh];
				u32 yh_word = (yh & 0xFFFF) | ((yh & 0xFFFF) << 16);
				for (u32 x = 0; x < width; x += 2) {
					u32 bv0 = ((x + 0) / bar_width >= 7) ? 6 : ((x + 0) / bar_width);
					u32 bv1 = ((x + 1) / bar_width >= 7) ? 6 : ((x + 1) / bar_width);
					u32 yw = (SMPTE_GRAY_10BPC[bv0] & 0xFFFF) |
						 ((SMPTE_GRAY_10BPC[bv1] & 0xFFFF) << 16);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, line_off + x * 2, yw);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, line_off + x * 2, yh_word);
				}
			}
		}
			break;

		/* ===== YUV 4:2:0 Formats ===== */

		case YV16_420: /* Format 19 - Planar 4:2:0 8-bit: V1=Y, V2=Cb, V3=Cr (chroma half-w, half-h) */
		{
			u32 y_stride = width;
			if (y_stride % 256 != 0)
				y_stride = (y_stride / 256 + 1) * 256;
			u32 c_line = width / 2;
			u32 c_stride = c_line;
			if (c_stride % 256 != 0)
				c_stride = (c_stride / 256 + 1) * 256;

			for (u32 y = 0; y < height; y++) {
				u32 y_off = y * y_stride;
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				for (u32 x = 0; x < width; x += 4) {
					u32 b0 = ((x+0)/bar_width >= 7) ? 6 : ((x+0)/bar_width);
					u32 b1 = ((x+1)/bar_width >= 7) ? 6 : ((x+1)/bar_width);
					u32 b2 = ((x+2)/bar_width >= 7) ? 6 : ((x+2)/bar_width);
					u32 b3 = ((x+3)/bar_width >= 7) ? 6 : ((x+3)/bar_width);
					u32 yw = SMPTE_YUV[b0][0] | (SMPTE_YUV[b1][0]<<8) |
						 (SMPTE_YUV[b2][0]<<16) | (SMPTE_YUV[b3][0]<<24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, y_off + x, yw);
					u32 yh = SMPTE_YUV[bh][0];
					u32 yw2 = yh | (yh<<8) | (yh<<16) | (yh<<24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, y_off + x, yw2);
				}
			}
			for (u32 y = 0; y < height / 2; y++) {
				u32 c_off = y * c_stride;
				u32 bh = ((y * 2) / bar_height >= 7) ? 6 : ((y * 2) / bar_height);
				for (u32 x = 0; x < width / 2; x += 4) {
					u32 lx = x * 2;
					u32 b0 = ((lx+0)/bar_width >= 7) ? 6 : ((lx+0)/bar_width);
					u32 b1 = ((lx+2)/bar_width >= 7) ? 6 : ((lx+2)/bar_width);
					u32 b2 = ((lx+4)/bar_width >= 7) ? 6 : ((lx+4)/bar_width);
					u32 b3 = ((lx+6)/bar_width >= 7) ? 6 : ((lx+6)/bar_width);
					u32 cbw = SMPTE_YUV[b0][1] | (SMPTE_YUV[b1][1]<<8) |
						  (SMPTE_YUV[b2][1]<<16) | (SMPTE_YUV[b3][1]<<24);
					u32 crw = SMPTE_YUV[b0][2] | (SMPTE_YUV[b1][2]<<8) |
						  (SMPTE_YUV[b2][2]<<16) | (SMPTE_YUV[b3][2]<<24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V2, c_off + x, cbw);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V3, c_off + x, crw);
					u32 cbh = SMPTE_YUV[bh][1];
					u32 crh = SMPTE_YUV[bh][2];
					u32 cbw2 = cbh | (cbh<<8) | (cbh<<16) | (cbh<<24);
					u32 crw2 = crh | (crh<<8) | (crh<<16) | (crh<<24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V5, c_off + x, cbw2);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V6, c_off + x, crw2);
				}
			}
		}
			break;

		case YV16Ci_420: /* Format 20 - SemiPlanar 4:2:0 8-bit: V1=Y, V2=CrCb */
		case YV16Ci2_420: /* Format 21 - SemiPlanar 4:2:0 8-bit: V1=Y, V2=CbCr (CbFirst=TRUE) */
		{
			u32 y_stride = width;
			if (y_stride % 256 != 0)
				y_stride = (y_stride / 256 + 1) * 256;
			u32 c_stride = width;
			if (c_stride % 256 != 0)
				c_stride = (c_stride / 256 + 1) * 256;
			u8 cbfirst = (fmt == YV16Ci2_420) ? 1 : 0;
			u32 ci1 = cbfirst ? 1 : 2;
			u32 ci2 = cbfirst ? 2 : 1;

			for (u32 y = 0; y < height; y++) {
				u32 y_off = y * y_stride;
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				for (u32 x = 0; x < width; x += 4) {
					u32 b0 = ((x+0)/bar_width >= 7) ? 6 : ((x+0)/bar_width);
					u32 b1 = ((x+1)/bar_width >= 7) ? 6 : ((x+1)/bar_width);
					u32 b2 = ((x+2)/bar_width >= 7) ? 6 : ((x+2)/bar_width);
					u32 b3 = ((x+3)/bar_width >= 7) ? 6 : ((x+3)/bar_width);
					u32 yw = SMPTE_YUV[b0][0] | (SMPTE_YUV[b1][0]<<8) |
						 (SMPTE_YUV[b2][0]<<16) | (SMPTE_YUV[b3][0]<<24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, y_off + x, yw);
					u32 yh = SMPTE_YUV[bh][0];
					u32 yw2 = yh | (yh<<8) | (yh<<16) | (yh<<24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, y_off + x, yw2);
				}
			}
			for (u32 y = 0; y < height / 2; y++) {
				u32 c_off = y * c_stride;
				u32 bh = ((y * 2) / bar_height >= 7) ? 6 : ((y * 2) / bar_height);
				for (u32 x = 0; x < width; x += 4) {
					u32 b0 = ((x+0)/bar_width >= 7) ? 6 : ((x+0)/bar_width);
					u32 b2 = ((x+2)/bar_width >= 7) ? 6 : ((x+2)/bar_width);
					u32 cw = SMPTE_YUV[b0][ci1] | (SMPTE_YUV[b0][ci2]<<8) |
						 (SMPTE_YUV[b2][ci1]<<16) | (SMPTE_YUV[b2][ci2]<<24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V2, c_off + x, cw);
					u32 cw2 = SMPTE_YUV[bh][ci1] | (SMPTE_YUV[bh][ci2]<<8) |
						  (SMPTE_YUV[bh][ci1]<<16) | (SMPTE_YUV[bh][ci2]<<24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V5, c_off + x, cw2);
				}
			}
		}
			break;

		case YV16_420_10BPC: /* Format 22 - Planar 4:2:0 10-bit: V1=Y, V2=Cb, V3=Cr (chroma half-w, half-h) */
		{
			u32 y_stride = width * 2;
			if (y_stride % 256 != 0)
				y_stride = (y_stride / 256 + 1) * 256;
			u32 c_line = (width / 2) * 2;
			u32 c_stride = c_line;
			if (c_stride % 256 != 0)
				c_stride = (c_stride / 256 + 1) * 256;

			for (u32 y = 0; y < height; y++) {
				u32 y_off = y * y_stride;
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				for (u32 x = 0; x < width; x += 2) {
					u32 bv0 = ((x+0)/bar_width >= 7) ? 6 : ((x+0)/bar_width);
					u32 bv1 = ((x+1)/bar_width >= 7) ? 6 : ((x+1)/bar_width);
					u32 yw = SMPTE_YUV_10BPC[bv0][0] | (SMPTE_YUV_10BPC[bv1][0] << 16);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, y_off + x * 2, yw);
					u32 yw2 = SMPTE_YUV_10BPC[bh][0] | (SMPTE_YUV_10BPC[bh][0] << 16);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, y_off + x * 2, yw2);
				}
			}
			for (u32 y = 0; y < height / 2; y++) {
				u32 c_off = y * c_stride;
				u32 bh = ((y * 2) / bar_height >= 7) ? 6 : ((y * 2) / bar_height);
				for (u32 x = 0; x < width / 2; x += 2) {
					u32 lx = x * 2;
					u32 b0 = ((lx+0)/bar_width >= 7) ? 6 : ((lx+0)/bar_width);
					u32 b1 = ((lx+2)/bar_width >= 7) ? 6 : ((lx+2)/bar_width);
					u32 cbw = SMPTE_YUV_10BPC[b0][1] | (SMPTE_YUV_10BPC[b1][1] << 16);
					u32 crw = SMPTE_YUV_10BPC[b0][2] | (SMPTE_YUV_10BPC[b1][2] << 16);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V2, c_off + x * 2, cbw);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V3, c_off + x * 2, crw);
					u32 cbw2 = SMPTE_YUV_10BPC[bh][1] | (SMPTE_YUV_10BPC[bh][1] << 16);
					u32 crw2 = SMPTE_YUV_10BPC[bh][2] | (SMPTE_YUV_10BPC[bh][2] << 16);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V5, c_off + x * 2, cbw2);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V6, c_off + x * 2, crw2);
				}
			}
		}
			break;

		case YV16Ci_420_10BPC: /* Format 23 - SemiPlanar 4:2:0 10-bit: V1=Y, V2=CrCb */
		case YV16Ci2_420_10BPC: /* Format 24 - SemiPlanar 4:2:0 10-bit: V1=Y, V2=CbCr (CbFirst=TRUE) */
		{
			u32 y_stride = width * 2;
			if (y_stride % 256 != 0)
				y_stride = (y_stride / 256 + 1) * 256;
			u32 c_stride = width * 2;
			if (c_stride % 256 != 0)
				c_stride = (c_stride / 256 + 1) * 256;
			u8 cbfirst = (fmt == YV16Ci2_420_10BPC) ? 1 : 0;
			u32 ci1 = cbfirst ? 1 : 2;
			u32 ci2 = cbfirst ? 2 : 1;

			for (u32 y = 0; y < height; y++) {
				u32 y_off = y * y_stride;
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				for (u32 x = 0; x < width; x += 2) {
					u32 bv0 = ((x+0)/bar_width >= 7) ? 6 : ((x+0)/bar_width);
					u32 bv1 = ((x+1)/bar_width >= 7) ? 6 : ((x+1)/bar_width);
					u32 yw = SMPTE_YUV_10BPC[bv0][0] | (SMPTE_YUV_10BPC[bv1][0] << 16);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, y_off + x * 2, yw);
					u32 yw2 = SMPTE_YUV_10BPC[bh][0] | (SMPTE_YUV_10BPC[bh][0] << 16);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, y_off + x * 2, yw2);
				}
			}
			for (u32 y = 0; y < height / 2; y++) {
				u32 c_off = y * c_stride;
				u32 bh = ((y * 2) / bar_height >= 7) ? 6 : ((y * 2) / bar_height);
				for (u32 x = 0; x < width; x += 2) {
					u32 bv0 = ((x+0)/bar_width >= 7) ? 6 : ((x+0)/bar_width);
					u32 cw = (SMPTE_YUV_10BPC[bv0][ci1] & 0xFFFF) |
						 ((SMPTE_YUV_10BPC[bv0][ci2] & 0xFFFF) << 16);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V2, c_off + x * 2, cw);
					u32 cw2 = (SMPTE_YUV_10BPC[bh][ci1] & 0xFFFF) |
						  ((SMPTE_YUV_10BPC[bh][ci2] & 0xFFFF) << 16);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V5, c_off + x * 2, cw2);
				}
			}
		}
			break;

		/* ===== RGBA Formats ===== */

		case RGBA8888: /* Format 32 - RGBA8888: A[7:0]B[7:0]G[7:0]R[7:0] */
			for (u32 y = 0; y < height; y++) {
				for (u32 x = 0; x < width; x++) {
					u32 pixel_idx = y * width + x;
					u32 bar_v1 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 bar_v2 = (y / bar_height >= 7) ? 6 : (y / bar_height);

					u32 color_v1 = 0xFF000000 | (SMPTE_RGB[bar_v1][2] << 16) | (SMPTE_RGB[bar_v1][1] << 8) | SMPTE_RGB[bar_v1][0];
					u32 color_v2 = stream2_rgba_alpha | (SMPTE_RGB[bar_v2][2] << 16) | (SMPTE_RGB[bar_v2][1] << 8) | SMPTE_RGB[bar_v2][0];

					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 4, color_v1);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 4, color_v2);
				}
			}
			break;

		case RGBA5551: /* Format 36 - RGBA5551: A[0]B[4:0]G[4:0]R[4:0] */
		{
			const u16 smpte_colors[7] = {
				0xFFFF, 0xFFE0, 0x87FF, 0x83E0, 0xFC1F, 0xF800, 0x801F
			};
			for (u32 y = 0; y < height; y++) {
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u32 h_word = (u32)smpte_colors[bh] | ((u32)smpte_colors[bh] << 16);
				for (u32 x = 0; x + 2 <= width; x += 2) {
					u32 b0 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 b1 = ((x + 1) / bar_width >= 7) ? 6 : ((x + 1) / bar_width);
					u32 off = (y * width + x) * 2;
					u32 v_word = (u32)smpte_colors[b0] | ((u32)smpte_colors[b1] << 16);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, off, v_word);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, off, h_word);
				}
			}
		}
			break;

		case RGBA4444: /* Format 37 - RGBA4444: A[3:0]B[3:0]G[3:0]R[3:0] */
		{
			const u16 smpte_colors[7] = {
				0xFFFF, 0xFFF0, 0xF0FF, 0xF0F0, 0xFF0F, 0xFF00, 0xF00F
			};
			for (u32 y = 0; y < height; y++) {
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u32 h_word = (u32)smpte_colors[bh] | ((u32)smpte_colors[bh] << 16);
				for (u32 x = 0; x + 2 <= width; x += 2) {
					u32 b0 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 b1 = ((x + 1) / bar_width >= 7) ? 6 : ((x + 1) / bar_width);
					u32 off = (y * width + x) * 2;
					u32 v_word = (u32)smpte_colors[b0] | ((u32)smpte_colors[b1] << 16);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, off, v_word);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, off, h_word);
				}
			}
		}
			break;

		case RGB565: /* Format 38 - RGB565: R[4:0]G[5:0]B[4:0] */
		{
			const u16 smpte_colors[7] = {
				0xFFFF, 0xFFE0, 0x07FF, 0x07E0, 0xF81F, 0xF800, 0x001F
			};
			for (u32 y = 0; y < height; y++) {
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u32 h_word = (u32)smpte_colors[bh] | ((u32)smpte_colors[bh] << 16);
				for (u32 x = 0; x + 2 <= width; x += 2) {
					u32 b0 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 b1 = ((x + 1) / bar_width >= 7) ? 6 : ((x + 1) / bar_width);
					u32 off = (y * width + x) * 2;
					u32 v_word = (u32)smpte_colors[b0] | ((u32)smpte_colors[b1] << 16);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, off, v_word);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, off, h_word);
				}
			}
		}
			break;

		case RGB888_GFX: /* Format 64 - RGB888dc: 32-bit container B[7:0]G[7:0]R[7:0] */
		case RGB888dc:
			for (u32 y = 0; y < height; y++) {
				for (u32 x = 0; x < width; x++) {
					u32 pixel_idx = y * width + x;
					u32 bar_v1 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 bar_v2 = (y / bar_height >= 7) ? 6 : (y / bar_height);

					u32 color_v1 = (SMPTE_RGB[bar_v1][2] << 16) | (SMPTE_RGB[bar_v1][1] << 8) | SMPTE_RGB[bar_v1][0];
					u32 color_v2 = (SMPTE_RGB[bar_v2][2] << 16) | (SMPTE_RGB[bar_v2][1] << 8) | SMPTE_RGB[bar_v2][0];

					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 4, color_v1);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 4, color_v2);
				}
			}
			break;

		/* ===== 10-bit RGB Formats in 64-bit containers ===== */

		case RGB888dcm_10BPC: /* Format 65 - RGB888dcm 10-bit MSB aligned */
		case RGB888dcl_10BPC: /* Format 66 - RGB888dcl 10-bit LSB aligned */
		{
			u8 msb_align = (fmt == RGB888dcm_10BPC) ? 1 : 0;
			for (u32 y = 0; y < height; y++) {
				for (u32 x = 0; x < width; x++) {
					u32 pixel_idx = y * width + x;
					u32 bar_v1 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 bar_v2 = (y / bar_height >= 7) ? 6 : (y / bar_height);

					/* Scale 8-bit to 10-bit */
					u16 r_v1 = SMPTE_RGB[bar_v1][0] * 4;
					u16 g_v1 = SMPTE_RGB[bar_v1][1] * 4;
					u16 b_v1 = SMPTE_RGB[bar_v1][2] * 4;
					u16 r_v2 = SMPTE_RGB[bar_v2][0] * 4;
					u16 g_v2 = SMPTE_RGB[bar_v2][1] * 4;
					u16 b_v2 = SMPTE_RGB[bar_v2][2] * 4;

					if (msb_align) {
						/* MSB aligned - shift left by 6 bits (for 16-bit storage) */
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8, (u16)(b_v1 << 6));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 2, (u16)(g_v1 << 6));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 4, (u16)(r_v1 << 6));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8, (u16)(b_v2 << 6));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 2, (u16)(g_v2 << 6));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 4, (u16)(r_v2 << 6));
					} else {
						/* LSB aligned */
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8, b_v1);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 2, g_v1);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 4, r_v1);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8, b_v2);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 2, g_v2);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 4, r_v2);
					}
				}
			}
		}
			break;

		/* ===== 12-bit RGB Formats in 64-bit containers ===== */

		case RGB888dcm_12BPC: /* Format 67 - RGB888dcm 12-bit MSB aligned */
		case RGB888dcl_12BPC: /* Format 68 - RGB888dcl 12-bit LSB aligned */
		{
			u8 msb_align = (fmt == RGB888dcm_12BPC) ? 1 : 0;
			for (u32 y = 0; y < height; y++) {
				for (u32 x = 0; x < width; x++) {
					u32 pixel_idx = y * width + x;
					u32 bar_v1 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 bar_v2 = (y / bar_height >= 7) ? 6 : (y / bar_height);

					/* Scale 8-bit to 12-bit */
					u16 r_v1 = SMPTE_RGB[bar_v1][0] * 16;
					u16 g_v1 = SMPTE_RGB[bar_v1][1] * 16;
					u16 b_v1 = SMPTE_RGB[bar_v1][2] * 16;
					u16 r_v2 = SMPTE_RGB[bar_v2][0] * 16;
					u16 g_v2 = SMPTE_RGB[bar_v2][1] * 16;
					u16 b_v2 = SMPTE_RGB[bar_v2][2] * 16;

					if (msb_align) {
						/* MSB aligned - shift left by 4 bits */
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8, (u16)(b_v1 << 4));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 2, (u16)(g_v1 << 4));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 4, (u16)(r_v1 << 4));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8, (u16)(b_v2 << 4));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 2, (u16)(g_v2 << 4));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 4, (u16)(r_v2 << 4));
					} else {
						/* LSB aligned */
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8, b_v1);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 2, g_v1);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 4, r_v1);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8, b_v2);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 2, g_v2);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 4, r_v2);
					}
				}
			}
		}
			break;

		case RGB10A2g: /* Format 71 - RGB10A2g: 2-bit alpha, 10-bit RGB */
			for (u32 y = 0; y < height; y++) {
				for (u32 x = 0; x < width; x++) {
					u32 pixel_idx = y * width + x;
					u32 bar_v1 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 bar_v2 = (y / bar_height >= 7) ? 6 : (y / bar_height);

					/* Pack: A[1:0]B[9:0]G[9:0]R[9:0] */
					u32 color_v1 = (3 << 30) | ((SMPTE_RGB[bar_v1][2] * 4) << 20) |
					               ((SMPTE_RGB[bar_v1][1] * 4) << 10) | (SMPTE_RGB[bar_v1][0] * 4);
					u32 color_v2 = (3 << 30) | ((SMPTE_RGB[bar_v2][2] * 4) << 20) |
					               ((SMPTE_RGB[bar_v2][1] * 4) << 10) | (SMPTE_RGB[bar_v2][0] * 4);

					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 4, color_v1);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 4, color_v2);
				}
			}
			break;

		/* ===== 10-bit RGBA Formats ===== */

		case RGBA8888dcm_10BPC: /* Format 72 - RGBA8888dcm 10-bit MSB */
		case RGBA8888dcl_10BPC: /* Format 73 - RGBA8888dcl 10-bit LSB */
		{
			u8 msb_align = (fmt == RGBA8888dcm_10BPC) ? 1 : 0;
			for (u32 y = 0; y < height; y++) {
				for (u32 x = 0; x < width; x++) {
					u32 pixel_idx = y * width + x;
					u32 bar_v1 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 bar_v2 = (y / bar_height >= 7) ? 6 : (y / bar_height);

					u16 r_v1 = SMPTE_RGB[bar_v1][0] * 4;
					u16 g_v1 = SMPTE_RGB[bar_v1][1] * 4;
					u16 b_v1 = SMPTE_RGB[bar_v1][2] * 4;
					u16 r_v2 = SMPTE_RGB[bar_v2][0] * 4;
					u16 g_v2 = SMPTE_RGB[bar_v2][1] * 4;
					u16 b_v2 = SMPTE_RGB[bar_v2][2] * 4;

					if (msb_align) {
						/* Alpha */
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8, (u16)(0xFF << 6));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 2, (u16)(b_v1 << 6));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 4, (u16)(g_v1 << 6));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 6, (u16)(r_v1 << 6));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8, (u16)(0xFF << 6));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 2, (u16)(b_v2 << 6));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 4, (u16)(g_v2 << 6));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 6, (u16)(r_v2 << 6));
					} else {
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8, 0xFF);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 2, b_v1);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 4, g_v1);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 6, r_v1);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8, 0xFF);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 2, b_v2);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 4, g_v2);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 6, r_v2);
					}
				}
			}
		}
			break;

		/* ===== 12-bit RGBA Formats ===== */

		case RGBA8888dcm_12BPC: /* Format 74 - RGBA8888dcm 12-bit MSB */
		case RGBA8888dcl_12BPC: /* Format 75 - RGBA8888dcl 12-bit LSB */
		{
			u8 msb_align = (fmt == RGBA8888dcm_12BPC) ? 1 : 0;
			for (u32 y = 0; y < height; y++) {
				for (u32 x = 0; x < width; x++) {
					u32 pixel_idx = y * width + x;
					u32 bar_v1 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 bar_v2 = (y / bar_height >= 7) ? 6 : (y / bar_height);

					u16 r_v1 = SMPTE_RGB[bar_v1][0] * 16;
					u16 g_v1 = SMPTE_RGB[bar_v1][1] * 16;
					u16 b_v1 = SMPTE_RGB[bar_v1][2] * 16;
					u16 r_v2 = SMPTE_RGB[bar_v2][0] * 16;
					u16 g_v2 = SMPTE_RGB[bar_v2][1] * 16;
					u16 b_v2 = SMPTE_RGB[bar_v2][2] * 16;

					if (msb_align) {
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8, (u16)(0xFF << 4));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 2, (u16)(b_v1 << 4));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 4, (u16)(g_v1 << 4));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 6, (u16)(r_v1 << 4));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8, (u16)(0xFF << 4));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 2, (u16)(b_v2 << 4));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 4, (u16)(g_v2 << 4));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 6, (u16)(r_v2 << 4));
					} else {
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8, 0xFF);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 2, b_v1);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 4, g_v1);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 6, r_v1);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8, 0xFF);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 2, b_v2);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 4, g_v2);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 6, r_v2);
					}
				}
			}
		}
			break;

		/* ===== YUV 4:4:4 Interleaved GPU/DC Formats ===== */

		case YUV444g: /* Format 78 - YUV444g (GPU): Cr[7:0]Cb[7:0]Y[7:0] */
		case YUV444dc: /* Format 79 - YUV444dc (container): Cr[7:0]Cb[7:0]Y[7:0] */
			for (u32 y = 0; y < height; y++) {
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u32 hw0 = SMPTE_YUV[bh][2] | (SMPTE_YUV[bh][1] << 8) |
					  (SMPTE_YUV[bh][0] << 16) | (SMPTE_YUV[bh][2] << 24);
				u32 hw1 = SMPTE_YUV[bh][1] | (SMPTE_YUV[bh][0] << 8) |
					  (SMPTE_YUV[bh][2] << 16) | (SMPTE_YUV[bh][1] << 24);
				u32 hw2 = SMPTE_YUV[bh][0] | (SMPTE_YUV[bh][2] << 8) |
					  (SMPTE_YUV[bh][1] << 16) | (SMPTE_YUV[bh][0] << 24);
				for (u32 x = 0; x + 4 <= width; x += 4) {
					u32 b0 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 b1 = ((x + 1) / bar_width >= 7) ? 6 : ((x + 1) / bar_width);
					u32 b2 = ((x + 2) / bar_width >= 7) ? 6 : ((x + 2) / bar_width);
					u32 b3 = ((x + 3) / bar_width >= 7) ? 6 : ((x + 3) / bar_width);
					u32 off = (y * width + x) * 3;

					u32 w0 = SMPTE_YUV[b0][2] | (SMPTE_YUV[b0][1] << 8) |
						 (SMPTE_YUV[b0][0] << 16) | (SMPTE_YUV[b1][2] << 24);
					u32 w1 = SMPTE_YUV[b1][1] | (SMPTE_YUV[b1][0] << 8) |
						 (SMPTE_YUV[b2][2] << 16) | (SMPTE_YUV[b2][1] << 24);
					u32 w2 = SMPTE_YUV[b2][0] | (SMPTE_YUV[b3][2] << 8) |
						 (SMPTE_YUV[b3][1] << 16) | (SMPTE_YUV[b3][0] << 24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, off, w0);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, off + 4, w1);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, off + 8, w2);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, off, hw0);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, off + 4, hw1);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, off + 8, hw2);
				}
			}
			break;

		/* ===== YUV 4:4:4 10-bit Interleaved Formats ===== */

		case YUV444g_10BPC: /* Format 80 - YUV444g 10-bit (GPU) */
		case YUV444dcm_10BPC: /* Format 81 - YUV444dcm 10-bit MSB */
		case YUV444dcl_10BPC: /* Format 82 - YUV444dcl 10-bit LSB */
		{
			u8 msb_align = (fmt == YUV444dcm_10BPC) ? 1 : 0;
			u8 lsb_align = (fmt == YUV444dcl_10BPC) ? 1 : 0;

			for (u32 y = 0; y < height; y++) {
				for (u32 x = 0; x < width; x++) {
					u32 pixel_idx = y * width + x;
					u32 bar_v1 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 bar_v2 = (y / bar_height >= 7) ? 6 : (y / bar_height);

					if (msb_align) {
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8, (u16)(SMPTE_YUV_10BPC[bar_v1][2] << 6));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 2, (u16)(SMPTE_YUV_10BPC[bar_v1][1] << 6));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 4, (u16)(SMPTE_YUV_10BPC[bar_v1][0] << 6));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8, (u16)(SMPTE_YUV_10BPC[bar_v2][2] << 6));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 2, (u16)(SMPTE_YUV_10BPC[bar_v2][1] << 6));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 4, (u16)(SMPTE_YUV_10BPC[bar_v2][0] << 6));
					} else if (lsb_align) {
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8, SMPTE_YUV_10BPC[bar_v1][2]);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 2, SMPTE_YUV_10BPC[bar_v1][1]);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 4, SMPTE_YUV_10BPC[bar_v1][0]);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8, SMPTE_YUV_10BPC[bar_v2][2]);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 2, SMPTE_YUV_10BPC[bar_v2][1]);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 4, SMPTE_YUV_10BPC[bar_v2][0]);
					} else {
						/* GPU format - store in compact form */
						u32 color_v1 = SMPTE_YUV_10BPC[bar_v1][2] | (SMPTE_YUV_10BPC[bar_v1][1] << 10) |
						               (SMPTE_YUV_10BPC[bar_v1][0] << 20);
						u32 color_v2 = SMPTE_YUV_10BPC[bar_v2][2] | (SMPTE_YUV_10BPC[bar_v2][1] << 10) |
						               (SMPTE_YUV_10BPC[bar_v2][0] << 20);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 4, color_v1);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 4, color_v2);
					}
				}
			}
		}
			break;

		/* ===== YUV 4:4:4 12-bit Interleaved Formats ===== */

		case YUV444dcm_12BPC: /* Format 83 - YUV444dcm 12-bit MSB */
		case YUV444dcl_12BPC: /* Format 84 - YUV444dcl 12-bit LSB */
		{
			u8 msb_align = (fmt == YUV444dcm_12BPC) ? 1 : 0;

			for (u32 y = 0; y < height; y++) {
				for (u32 x = 0; x < width; x++) {
					u32 pixel_idx = y * width + x;
					u32 bar_v1 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 bar_v2 = (y / bar_height >= 7) ? 6 : (y / bar_height);

					if (msb_align) {
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8, (u16)(SMPTE_YUV_12BPC[bar_v1][2] << 4));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 2, (u16)(SMPTE_YUV_12BPC[bar_v1][1] << 4));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 4, (u16)(SMPTE_YUV_12BPC[bar_v1][0] << 4));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8, (u16)(SMPTE_YUV_12BPC[bar_v2][2] << 4));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 2, (u16)(SMPTE_YUV_12BPC[bar_v2][1] << 4));
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 4, (u16)(SMPTE_YUV_12BPC[bar_v2][0] << 4));
					} else {
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8, SMPTE_YUV_12BPC[bar_v1][2]);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 2, SMPTE_YUV_12BPC[bar_v1][1]);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 8 + 4, SMPTE_YUV_12BPC[bar_v1][0]);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8, SMPTE_YUV_12BPC[bar_v2][2]);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 2, SMPTE_YUV_12BPC[bar_v2][1]);
						XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 8 + 4, SMPTE_YUV_12BPC[bar_v2][0]);
					}
				}
			}
		}
			break;

		case YUVA444g: /* Format 87 - YUVA4444g: 8-bit Alpha + YUV */
			for (u32 y = 0; y < height; y++) {
				for (u32 x = 0; x < width; x++) {
					u32 pixel_idx = y * width + x;
					u32 bar_v1 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 bar_v2 = (y / bar_height >= 7) ? 6 : (y / bar_height);

					u32 color_v1 = 0xFF000000 | (SMPTE_YUV[bar_v1][2] << 16) |
					               (SMPTE_YUV[bar_v1][1] << 8) | SMPTE_YUV[bar_v1][0];
					u32 color_v2 = 0xFF000000 | (SMPTE_YUV[bar_v2][2] << 16) |
					               (SMPTE_YUV[bar_v2][1] << 8) | SMPTE_YUV[bar_v2][0];

					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 4, color_v1);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 4, color_v2);
				}
			}
			break;

		/* ===== YV24 Planar 10-bit/12-bit Formats ===== */

		case YV24dcm_10BPC: /* Format 89 - Planar 10-bit MSB: V1=Y, V2=Cb, V3=Cr */
		case YV24dcl_10BPC: /* Format 90 - Planar 10-bit LSB: V1=Y, V2=Cb, V3=Cr */
		{
			u8 msb_align = (fmt == YV24dcm_10BPC) ? 1 : 0;
			u32 shift = msb_align ? 6 : 0;

			for (u32 y = 0; y < height; y++) {
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u16 hy = (u16)(SMPTE_YUV_10BPC[bh][0] << shift);
				u16 hcb = (u16)(SMPTE_YUV_10BPC[bh][1] << shift);
				u16 hcr = (u16)(SMPTE_YUV_10BPC[bh][2] << shift);
				u32 hy_w = (u32)hy | ((u32)hy << 16);
				u32 hcb_w = (u32)hcb | ((u32)hcb << 16);
				u32 hcr_w = (u32)hcr | ((u32)hcr << 16);
				for (u32 x = 0; x + 2 <= width; x += 2) {
					u32 b0 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 b1 = ((x + 1) / bar_width >= 7) ? 6 : ((x + 1) / bar_width);
					u32 off = (y * width + x) * 2;
					u16 y0 = (u16)(SMPTE_YUV_10BPC[b0][0] << shift);
					u16 y1 = (u16)(SMPTE_YUV_10BPC[b1][0] << shift);
					u16 cb0 = (u16)(SMPTE_YUV_10BPC[b0][1] << shift);
					u16 cb1 = (u16)(SMPTE_YUV_10BPC[b1][1] << shift);
					u16 cr0 = (u16)(SMPTE_YUV_10BPC[b0][2] << shift);
					u16 cr1 = (u16)(SMPTE_YUV_10BPC[b1][2] << shift);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, off, (u32)y0 | ((u32)y1 << 16));
					XDc_WriteReg(IN_BUFFER_0_ADDR_V2, off, (u32)cb0 | ((u32)cb1 << 16));
					XDc_WriteReg(IN_BUFFER_0_ADDR_V3, off, (u32)cr0 | ((u32)cr1 << 16));
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, off, hy_w);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V5, off, hcb_w);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V6, off, hcr_w);
				}
			}
		}
			break;

		case YV24dcm_12BPC: /* Format 91 - Planar 12-bit MSB: V1=Y, V2=Cb, V3=Cr */
		case YV24dcl_12BPC: /* Format 92 - Planar 12-bit LSB: V1=Y, V2=Cb, V3=Cr */
		{
			u8 msb_align = (fmt == YV24dcm_12BPC) ? 1 : 0;
			u32 shift = msb_align ? 4 : 0;

			for (u32 y = 0; y < height; y++) {
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u16 hy = (u16)(SMPTE_YUV_12BPC[bh][0] << shift);
				u16 hcb = (u16)(SMPTE_YUV_12BPC[bh][1] << shift);
				u16 hcr = (u16)(SMPTE_YUV_12BPC[bh][2] << shift);
				u32 hy_w = (u32)hy | ((u32)hy << 16);
				u32 hcb_w = (u32)hcb | ((u32)hcb << 16);
				u32 hcr_w = (u32)hcr | ((u32)hcr << 16);
				for (u32 x = 0; x + 2 <= width; x += 2) {
					u32 b0 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 b1 = ((x + 1) / bar_width >= 7) ? 6 : ((x + 1) / bar_width);
					u32 off = (y * width + x) * 2;
					u16 y0 = (u16)(SMPTE_YUV_12BPC[b0][0] << shift);
					u16 y1 = (u16)(SMPTE_YUV_12BPC[b1][0] << shift);
					u16 cb0 = (u16)(SMPTE_YUV_12BPC[b0][1] << shift);
					u16 cb1 = (u16)(SMPTE_YUV_12BPC[b1][1] << shift);
					u16 cr0 = (u16)(SMPTE_YUV_12BPC[b0][2] << shift);
					u16 cr1 = (u16)(SMPTE_YUV_12BPC[b1][2] << shift);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, off, (u32)y0 | ((u32)y1 << 16));
					XDc_WriteReg(IN_BUFFER_0_ADDR_V2, off, (u32)cb0 | ((u32)cb1 << 16));
					XDc_WriteReg(IN_BUFFER_0_ADDR_V3, off, (u32)cr0 | ((u32)cr1 << 16));
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, off, hy_w);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V5, off, hcb_w);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V6, off, hcr_w);
				}
			}
		}
			break;

		/* ===== YV16CI 10-bit/12-bit Semi-Planar 4:2:2 Formats ===== */

		case YV16CIdcm_10BPC: /* Format 95 - V1=Y(10b MSB), V2=CbCr semi-planar 4:2:2 */
		case YV16CIdcl_10BPC: /* Format 96 - V1=Y(10b LSB), V2=CbCr semi-planar 4:2:2 */
		case YV16CI2dcm_10BPC: /* Format 97 - V1=Y(10b MSB), V2=CrCb semi-planar 4:2:2 */
		case YV16CI2dcl_10BPC: /* Format 98 - V1=Y(10b LSB), V2=CrCb semi-planar 4:2:2 */
		{
			u8 msb_align = (fmt == YV16CIdcm_10BPC || fmt == YV16CI2dcm_10BPC) ? 1 : 0;
			u8 swap_chroma = (fmt == YV16CI2dcm_10BPC || fmt == YV16CI2dcl_10BPC) ? 1 : 0;
			u32 shift = msb_align ? 6 : 0;
			u32 ci1 = swap_chroma ? 2 : 1;
			u32 ci2 = swap_chroma ? 1 : 2;

			for (u32 y = 0; y < height; y++) {
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u16 hy = (u16)(SMPTE_YUV_10BPC[bh][0] << shift);
				u32 hy_w = (u32)hy | ((u32)hy << 16);
				u16 hc1 = (u16)(SMPTE_YUV_10BPC[bh][ci1] << shift);
				u16 hc2 = (u16)(SMPTE_YUV_10BPC[bh][ci2] << shift);
				u32 hc_w = (u32)hc1 | ((u32)hc2 << 16);
				for (u32 x = 0; x + 2 <= width; x += 2) {
					u32 b0 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 b1 = ((x + 1) / bar_width >= 7) ? 6 : ((x + 1) / bar_width);
					u32 off = (y * width + x) * 2;
					u16 s0 = (u16)(SMPTE_YUV_10BPC[b0][0] << shift);
					u16 s1 = (u16)(SMPTE_YUV_10BPC[b1][0] << shift);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, off, (u32)s0 | ((u32)s1 << 16));
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, off, hy_w);

					u32 chroma_idx = y * (width / 2) + (x / 2);
					u16 cv1 = (u16)(SMPTE_YUV_10BPC[b0][ci1] << shift);
					u16 cv2 = (u16)(SMPTE_YUV_10BPC[b0][ci2] << shift);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V2, chroma_idx * 4, (u32)cv1 | ((u32)cv2 << 16));
					XDc_WriteReg(IN_BUFFER_0_ADDR_V5, chroma_idx * 4, hc_w);
				}
			}
		}
			break;

		case YV16CIdcm_12BPC: /* Format 99 - V1=Y(12b MSB), V2=CbCr semi-planar 4:2:2 */
		case YV16CIdcl_12BPC: /* Format 100 - V1=Y(12b LSB), V2=CbCr semi-planar 4:2:2 */
		case YV16CI2dcm_12BPC: /* Format 101 - V1=Y(12b MSB), V2=CrCb semi-planar 4:2:2 */
		case YV16CI2dcl_12BPC: /* Format 102 - V1=Y(12b LSB), V2=CrCb semi-planar 4:2:2 */
		{
			u8 msb_align = (fmt == YV16CIdcm_12BPC || fmt == YV16CI2dcm_12BPC) ? 1 : 0;
			u8 swap_chroma = (fmt == YV16CI2dcm_12BPC || fmt == YV16CI2dcl_12BPC) ? 1 : 0;
			u32 shift = msb_align ? 4 : 0;
			u32 ci1 = swap_chroma ? 2 : 1;
			u32 ci2 = swap_chroma ? 1 : 2;

			for (u32 y = 0; y < height; y++) {
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u16 hy = (u16)(SMPTE_YUV_12BPC[bh][0] << shift);
				u32 hy_w = (u32)hy | ((u32)hy << 16);
				u16 hc1 = (u16)(SMPTE_YUV_12BPC[bh][ci1] << shift);
				u16 hc2 = (u16)(SMPTE_YUV_12BPC[bh][ci2] << shift);
				u32 hc_w = (u32)hc1 | ((u32)hc2 << 16);
				for (u32 x = 0; x + 2 <= width; x += 2) {
					u32 b0 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 b1 = ((x + 1) / bar_width >= 7) ? 6 : ((x + 1) / bar_width);
					u32 off = (y * width + x) * 2;
					u16 s0 = (u16)(SMPTE_YUV_12BPC[b0][0] << shift);
					u16 s1 = (u16)(SMPTE_YUV_12BPC[b1][0] << shift);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, off, (u32)s0 | ((u32)s1 << 16));
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, off, hy_w);

					u32 chroma_idx = y * (width / 2) + (x / 2);
					u16 cv1 = (u16)(SMPTE_YUV_12BPC[b0][ci1] << shift);
					u16 cv2 = (u16)(SMPTE_YUV_12BPC[b0][ci2] << shift);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V2, chroma_idx * 4, (u32)cv1 | ((u32)cv2 << 16));
					XDc_WriteReg(IN_BUFFER_0_ADDR_V5, chroma_idx * 4, hc_w);
				}
			}
		}
			break;

		/* ===== YV16CI_420 10-bit/12-bit Semi-Planar 4:2:0 Formats ===== */

		case YV16CIdcm_420_10BPC: /* Format 107 - V1=Y(10b MSB), V2=CbCr semi-planar 4:2:0 */
		case YV16CIdcl_420_10BPC: /* Format 108 - V1=Y(10b LSB), V2=CbCr semi-planar 4:2:0 */
		case YV16CI2dcm_420_10BPC: /* Format 109 - V1=Y(10b MSB), V2=CrCb semi-planar 4:2:0 */
		case YV16CI2dcl_420_10BPC: /* Format 110 - V1=Y(10b LSB), V2=CrCb semi-planar 4:2:0 */
		{
			u8 msb_align = (fmt == YV16CIdcm_420_10BPC || fmt == YV16CI2dcm_420_10BPC) ? 1 : 0;
			u8 swap_chroma = (fmt == YV16CI2dcm_420_10BPC || fmt == YV16CI2dcl_420_10BPC) ? 1 : 0;
			u32 shift = msb_align ? 6 : 0;
			u32 ci1 = swap_chroma ? 2 : 1;
			u32 ci2 = swap_chroma ? 1 : 2;

			for (u32 y = 0; y < height; y++) {
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u16 hy = (u16)(SMPTE_YUV_10BPC[bh][0] << shift);
				u32 hy_w = (u32)hy | ((u32)hy << 16);
				for (u32 x = 0; x + 2 <= width; x += 2) {
					u32 b0 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 b1 = ((x + 1) / bar_width >= 7) ? 6 : ((x + 1) / bar_width);
					u32 off = (y * width + x) * 2;
					u16 s0 = (u16)(SMPTE_YUV_10BPC[b0][0] << shift);
					u16 s1 = (u16)(SMPTE_YUV_10BPC[b1][0] << shift);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, off, (u32)s0 | ((u32)s1 << 16));
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, off, hy_w);
				}
			}
			for (u32 y = 0; y < height; y += 2) {
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u16 hc1 = (u16)(SMPTE_YUV_10BPC[bh][ci1] << shift);
				u16 hc2 = (u16)(SMPTE_YUV_10BPC[bh][ci2] << shift);
				u32 hc_w = (u32)hc1 | ((u32)hc2 << 16);
				for (u32 x = 0; x < width; x += 2) {
					u32 bar_v1 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 chroma_idx = (y / 2) * (width / 2) + (x / 2);
					u16 cv1 = (u16)(SMPTE_YUV_10BPC[bar_v1][ci1] << shift);
					u16 cv2 = (u16)(SMPTE_YUV_10BPC[bar_v1][ci2] << shift);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V2, chroma_idx * 4, (u32)cv1 | ((u32)cv2 << 16));
					XDc_WriteReg(IN_BUFFER_0_ADDR_V5, chroma_idx * 4, hc_w);
				}
			}
		}
			break;

		case YV16CIdcm_420_12BPC: /* Format 111 - V1=Y(12b MSB), V2=CbCr semi-planar 4:2:0 */
		case YV16CIdcl_420_12BPC: /* Format 112 - V1=Y(12b LSB), V2=CbCr semi-planar 4:2:0 */
		case YV16CI2dcm_420_12BPC: /* Format 113 - V1=Y(12b MSB), V2=CrCb semi-planar 4:2:0 */
		case YV16CI2dcl_420_12BPC: /* Format 114 - V1=Y(12b LSB), V2=CrCb semi-planar 4:2:0 */
		{
			u8 msb_align = (fmt == YV16CIdcm_420_12BPC || fmt == YV16CI2dcm_420_12BPC) ? 1 : 0;
			u8 swap_chroma = (fmt == YV16CI2dcm_420_12BPC || fmt == YV16CI2dcl_420_12BPC) ? 1 : 0;
			u32 shift = msb_align ? 4 : 0;
			u32 ci1 = swap_chroma ? 2 : 1;
			u32 ci2 = swap_chroma ? 1 : 2;

			for (u32 y = 0; y < height; y++) {
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u16 hy = (u16)(SMPTE_YUV_12BPC[bh][0] << shift);
				u32 hy_w = (u32)hy | ((u32)hy << 16);
				for (u32 x = 0; x + 2 <= width; x += 2) {
					u32 b0 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 b1 = ((x + 1) / bar_width >= 7) ? 6 : ((x + 1) / bar_width);
					u32 off = (y * width + x) * 2;
					u16 s0 = (u16)(SMPTE_YUV_12BPC[b0][0] << shift);
					u16 s1 = (u16)(SMPTE_YUV_12BPC[b1][0] << shift);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, off, (u32)s0 | ((u32)s1 << 16));
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, off, hy_w);
				}
			}
			for (u32 y = 0; y < height; y += 2) {
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u16 hc1 = (u16)(SMPTE_YUV_12BPC[bh][ci1] << shift);
				u16 hc2 = (u16)(SMPTE_YUV_12BPC[bh][ci2] << shift);
				u32 hc_w = (u32)hc1 | ((u32)hc2 << 16);
				for (u32 x = 0; x < width; x += 2) {
					u32 bar_v1 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 chroma_idx = (y / 2) * (width / 2) + (x / 2);
					u16 cv1 = (u16)(SMPTE_YUV_12BPC[bar_v1][ci1] << shift);
					u16 cv2 = (u16)(SMPTE_YUV_12BPC[bar_v1][ci2] << shift);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V2, chroma_idx * 4, (u32)cv1 | ((u32)cv2 << 16));
					XDc_WriteReg(IN_BUFFER_0_ADDR_V5, chroma_idx * 4, hc_w);
				}
			}
		}
			break;

		/* ===== Y-only (Monochrome) 10-bit/12-bit Formats ===== */

		case Ydc_ONLY: /* Format 117 - Ydc_ONLY 8-bit */
			for (u32 y = 0; y < height; y++) {
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u32 gh = SMPTE_GRAY_8BPC[bh];
				u32 y_hword = gh | (gh << 8) | (gh << 16) | (gh << 24);
				u32 y_base = y * width;
				for (u32 x = 0; x + 4 <= width; x += 4) {
					u32 b0 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 b1 = ((x + 1) / bar_width >= 7) ? 6 : ((x + 1) / bar_width);
					u32 b2 = ((x + 2) / bar_width >= 7) ? 6 : ((x + 2) / bar_width);
					u32 b3 = ((x + 3) / bar_width >= 7) ? 6 : ((x + 3) / bar_width);
					u32 y_word = SMPTE_GRAY_8BPC[b0] | (SMPTE_GRAY_8BPC[b1] << 8) |
						     (SMPTE_GRAY_8BPC[b2] << 16) | (SMPTE_GRAY_8BPC[b3] << 24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, y_base + x, y_word);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, y_base + x, y_hword);
				}
			}
			break;

		case Ydcm_ONLY_10BPC: /* Format 118 - Ydcm_ONLY 10-bit MSB */
		case Ydcl_ONLY_10BPC: /* Format 119 - Ydcl_ONLY 10-bit LSB */
		{
			u32 shift = (fmt == Ydcm_ONLY_10BPC) ? 6 : 0;
			for (u32 y = 0; y < height; y++) {
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u16 hg = (u16)(SMPTE_GRAY_10BPC[bh] << shift);
				u32 h_word = (u32)hg | ((u32)hg << 16);
				for (u32 x = 0; x + 2 <= width; x += 2) {
					u32 b0 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 b1 = ((x + 1) / bar_width >= 7) ? 6 : ((x + 1) / bar_width);
					u32 off = (y * width + x) * 2;
					u16 s0 = (u16)(SMPTE_GRAY_10BPC[b0] << shift);
					u16 s1 = (u16)(SMPTE_GRAY_10BPC[b1] << shift);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, off, (u32)s0 | ((u32)s1 << 16));
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, off, h_word);
				}
			}
		}
			break;

		case Ydcm_ONLY_12BPC: /* Format 120 - Ydcm_ONLY 12-bit MSB */
		case Ydcl_ONLY_12BPC: /* Format 121 - Ydcl_ONLY 12-bit LSB */
		{
			u32 shift = (fmt == Ydcm_ONLY_12BPC) ? 4 : 0;
			for (u32 y = 0; y < height; y++) {
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u16 hg = (u16)(SMPTE_GRAY_12BPC[bh] << shift);
				u32 h_word = (u32)hg | ((u32)hg << 16);
				for (u32 x = 0; x + 2 <= width; x += 2) {
					u32 b0 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 b1 = ((x + 1) / bar_width >= 7) ? 6 : ((x + 1) / bar_width);
					u32 off = (y * width + x) * 2;
					u16 s0 = (u16)(SMPTE_GRAY_12BPC[b0] << shift);
					u16 s1 = (u16)(SMPTE_GRAY_12BPC[b1] << shift);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, off, (u32)s0 | ((u32)s1 << 16));
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, off, h_word);
				}
			}
		}
			break;

		/* ===== Additional Format Aliases and Variants ===== */

		case YV24dc: /* Format 88 - Planar 4:4:4 8-bit DC: V1=Y, V2=Cb, V3=Cr */
		{
			for (u32 y = 0; y < height; y++) {
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u32 yh = SMPTE_YUV[bh][0];
				u32 cbh = SMPTE_YUV[bh][1];
				u32 crh = SMPTE_YUV[bh][2];
				u32 y_hword = yh | (yh << 8) | (yh << 16) | (yh << 24);
				u32 cb_hword = cbh | (cbh << 8) | (cbh << 16) | (cbh << 24);
				u32 cr_hword = crh | (crh << 8) | (crh << 16) | (crh << 24);
				u32 base = y * width;
				for (u32 x = 0; x + 4 <= width; x += 4) {
					u32 b0 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 b1 = ((x + 1) / bar_width >= 7) ? 6 : ((x + 1) / bar_width);
					u32 b2 = ((x + 2) / bar_width >= 7) ? 6 : ((x + 2) / bar_width);
					u32 b3 = ((x + 3) / bar_width >= 7) ? 6 : ((x + 3) / bar_width);
					u32 yw = SMPTE_YUV[b0][0] | (SMPTE_YUV[b1][0] << 8) |
						 (SMPTE_YUV[b2][0] << 16) | (SMPTE_YUV[b3][0] << 24);
					u32 cbw = SMPTE_YUV[b0][1] | (SMPTE_YUV[b1][1] << 8) |
						  (SMPTE_YUV[b2][1] << 16) | (SMPTE_YUV[b3][1] << 24);
					u32 crw = SMPTE_YUV[b0][2] | (SMPTE_YUV[b1][2] << 8) |
						  (SMPTE_YUV[b2][2] << 16) | (SMPTE_YUV[b3][2] << 24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, base + x, yw);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V2, base + x, cbw);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V3, base + x, crw);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, base + x, y_hword);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V5, base + x, cb_hword);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V6, base + x, cr_hword);
				}
			}
		}
			break;

		case YV16CIdc: /* Format 93 - SemiPlanar 4:2:2 8-bit DC: V1=Y, V2=CbCr */
		{
			for (u32 y = 0; y < height; y++) {
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u32 yh = SMPTE_YUV[bh][0];
				u32 y_hword = yh | (yh << 8) | (yh << 16) | (yh << 24);
				u32 c_hword = SMPTE_YUV[bh][1] | (SMPTE_YUV[bh][2] << 8) |
					      (SMPTE_YUV[bh][1] << 16) | (SMPTE_YUV[bh][2] << 24);
				u32 y_base = y * width;
				u32 c_base = y * (width / 2) * 2;
				for (u32 x = 0; x + 4 <= width; x += 4) {
					u32 b0 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 b1 = ((x + 1) / bar_width >= 7) ? 6 : ((x + 1) / bar_width);
					u32 b2 = ((x + 2) / bar_width >= 7) ? 6 : ((x + 2) / bar_width);
					u32 b3 = ((x + 3) / bar_width >= 7) ? 6 : ((x + 3) / bar_width);
					u32 y_word = SMPTE_YUV[b0][0] | (SMPTE_YUV[b1][0] << 8) |
						     (SMPTE_YUV[b2][0] << 16) | (SMPTE_YUV[b3][0] << 24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, y_base + x, y_word);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, y_base + x, y_hword);

					u32 cb0 = b0;
					u32 cb1 = b2;
					u32 c_word = SMPTE_YUV[cb0][1] | (SMPTE_YUV[cb0][2] << 8) |
						     (SMPTE_YUV[cb1][1] << 16) | (SMPTE_YUV[cb1][2] << 24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V2, c_base + x, c_word);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V5, c_base + x, c_hword);
				}
			}
		}
			break;

		case YV16CI2dc: /* Format 94 - SemiPlanar 4:2:2 8-bit DC: V1=Y, V2=CrCb */
		{
			for (u32 y = 0; y < height; y++) {
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u32 yh = SMPTE_YUV[bh][0];
				u32 y_hword = yh | (yh << 8) | (yh << 16) | (yh << 24);
				u32 c_hword = SMPTE_YUV[bh][2] | (SMPTE_YUV[bh][1] << 8) |
					      (SMPTE_YUV[bh][2] << 16) | (SMPTE_YUV[bh][1] << 24);
				u32 y_base = y * width;
				u32 c_base = y * (width / 2) * 2;
				for (u32 x = 0; x + 4 <= width; x += 4) {
					u32 b0 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 b1 = ((x + 1) / bar_width >= 7) ? 6 : ((x + 1) / bar_width);
					u32 b2 = ((x + 2) / bar_width >= 7) ? 6 : ((x + 2) / bar_width);
					u32 b3 = ((x + 3) / bar_width >= 7) ? 6 : ((x + 3) / bar_width);
					u32 y_word = SMPTE_YUV[b0][0] | (SMPTE_YUV[b1][0] << 8) |
						     (SMPTE_YUV[b2][0] << 16) | (SMPTE_YUV[b3][0] << 24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, y_base + x, y_word);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, y_base + x, y_hword);

					u32 cb0 = b0;
					u32 cb1 = b2;
					u32 c_word = SMPTE_YUV[cb0][2] | (SMPTE_YUV[cb0][1] << 8) |
						     (SMPTE_YUV[cb1][2] << 16) | (SMPTE_YUV[cb1][1] << 24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V2, c_base + x, c_word);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V5, c_base + x, c_hword);
				}
			}
		}
			break;

		case YV16CIdc_420: /* Format 105 - SemiPlanar 4:2:0 8-bit DC: V1=Y, V2=CbCr */
		{
			for (u32 y = 0; y < height; y++) {
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u32 yh = SMPTE_YUV[bh][0];
				u32 y_hword = yh | (yh << 8) | (yh << 16) | (yh << 24);
				u32 y_base = y * width;
				for (u32 x = 0; x + 4 <= width; x += 4) {
					u32 b0 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 b1 = ((x + 1) / bar_width >= 7) ? 6 : ((x + 1) / bar_width);
					u32 b2 = ((x + 2) / bar_width >= 7) ? 6 : ((x + 2) / bar_width);
					u32 b3 = ((x + 3) / bar_width >= 7) ? 6 : ((x + 3) / bar_width);
					u32 y_word = SMPTE_YUV[b0][0] | (SMPTE_YUV[b1][0] << 8) |
						     (SMPTE_YUV[b2][0] << 16) | (SMPTE_YUV[b3][0] << 24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, y_base + x, y_word);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, y_base + x, y_hword);
				}
			}
			for (u32 y = 0; y < height; y += 2) {
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u32 c_hword = SMPTE_YUV[bh][1] | (SMPTE_YUV[bh][2] << 8) |
					      (SMPTE_YUV[bh][1] << 16) | (SMPTE_YUV[bh][2] << 24);
				u32 c_base = (y / 2) * (width / 2) * 2;
				for (u32 x = 0; x + 4 <= width; x += 4) {
					u32 cb0 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 cb1 = ((x + 2) / bar_width >= 7) ? 6 : ((x + 2) / bar_width);
					u32 c_word = SMPTE_YUV[cb0][1] | (SMPTE_YUV[cb0][2] << 8) |
						     (SMPTE_YUV[cb1][1] << 16) | (SMPTE_YUV[cb1][2] << 24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V2, c_base + x, c_word);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V5, c_base + x, c_hword);
				}
			}
		}
			break;

		case YV16CI2dc_420: /* Format 106 - SemiPlanar 4:2:0 8-bit DC: V1=Y, V2=CrCb */
		{
			for (u32 y = 0; y < height; y++) {
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u32 yh = SMPTE_YUV[bh][0];
				u32 y_hword = yh | (yh << 8) | (yh << 16) | (yh << 24);
				u32 y_base = y * width;
				for (u32 x = 0; x + 4 <= width; x += 4) {
					u32 b0 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 b1 = ((x + 1) / bar_width >= 7) ? 6 : ((x + 1) / bar_width);
					u32 b2 = ((x + 2) / bar_width >= 7) ? 6 : ((x + 2) / bar_width);
					u32 b3 = ((x + 3) / bar_width >= 7) ? 6 : ((x + 3) / bar_width);
					u32 y_word = SMPTE_YUV[b0][0] | (SMPTE_YUV[b1][0] << 8) |
						     (SMPTE_YUV[b2][0] << 16) | (SMPTE_YUV[b3][0] << 24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, y_base + x, y_word);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, y_base + x, y_hword);
				}
			}
			for (u32 y = 0; y < height; y += 2) {
				u32 bh = (y / bar_height >= 7) ? 6 : (y / bar_height);
				u32 c_hword = SMPTE_YUV[bh][2] | (SMPTE_YUV[bh][1] << 8) |
					      (SMPTE_YUV[bh][2] << 16) | (SMPTE_YUV[bh][1] << 24);
				u32 c_base = (y / 2) * (width / 2) * 2;
				for (u32 x = 0; x + 4 <= width; x += 4) {
					u32 cb0 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 cb1 = ((x + 2) / bar_width >= 7) ? 6 : ((x + 2) / bar_width);
					u32 c_word = SMPTE_YUV[cb0][2] | (SMPTE_YUV[cb0][1] << 8) |
						     (SMPTE_YUV[cb1][2] << 16) | (SMPTE_YUV[cb1][1] << 24);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V2, c_base + x, c_word);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V5, c_base + x, c_hword);
				}
			}
		}
			break;

		case ABGR8888: /* Format 33 - ABGR8888 variant */
			/* Fall through */
		case BGR888: /* Format 73 - BGR888 variant */
			/* Fall through */
		default:
			xil_printf("  WARNING: Format %d not fully implemented, using basic approximation\r\n", fmt);
			/* Use RGBA8888 as fallback */
			for (u32 y = 0; y < height; y++) {
				for (u32 x = 0; x < width; x++) {
					u32 pixel_idx = y * width + x;
					u32 bar_v1 = (x / bar_width >= 7) ? 6 : (x / bar_width);
					u32 bar_v2 = (y / bar_height >= 7) ? 6 : (y / bar_height);

					u32 color_v1 = 0xFF000000 | (SMPTE_RGB[bar_v1][2] << 16) | (SMPTE_RGB[bar_v1][1] << 8) | SMPTE_RGB[bar_v1][0];
					u32 color_v2 = stream2_rgba_alpha | (SMPTE_RGB[bar_v2][2] << 16) | (SMPTE_RGB[bar_v2][1] << 8) | SMPTE_RGB[bar_v2][0];

					XDc_WriteReg(IN_BUFFER_0_ADDR_V1, pixel_idx * 4, color_v1);
					XDc_WriteReg(IN_BUFFER_0_ADDR_V4, pixel_idx * 4, color_v2);
				}
			}
			break;
	}

	xil_printf("  Video frames generated successfully\r\n");

	/* Debug: readback first 2 bytes per plane from V1-V3 and V4-V6 */
	const XDc_VideoAttribute *attr = XDc_GetNonLiveVideoAttribute(fmt);
	if (attr != NULL && attr->IsRGB == FALSE) {
		u32 b0, b1;

		xil_printf("  [DBG] Stream1 readback (first 2 bytes per plane):\r\n");
		b0 = XDc_ReadReg(IN_BUFFER_0_ADDR_V1, 0);
		b1 = XDc_ReadReg(IN_BUFFER_0_ADDR_V1, 1);
		xil_printf("    V1: 0x%02x 0x%02x\r\n", b0 & 0xFF, b1 & 0xFF);

		if (attr->Mode == SemiPlanar || attr->Mode == Planar || attr->Mode == Tiled) {
			b0 = XDc_ReadReg(IN_BUFFER_0_ADDR_V2, 0);
			b1 = XDc_ReadReg(IN_BUFFER_0_ADDR_V2, 1);
			xil_printf("    V2: 0x%02x 0x%02x\r\n", b0 & 0xFF, b1 & 0xFF);
		}
		if (attr->Mode == Planar || attr->Mode == Tiled) {
			b0 = XDc_ReadReg(IN_BUFFER_0_ADDR_V3, 0);
			b1 = XDc_ReadReg(IN_BUFFER_0_ADDR_V3, 1);
			xil_printf("    V3: 0x%02x 0x%02x\r\n", b0 & 0xFF, b1 & 0xFF);
		}

		xil_printf("  [DBG] Stream2 readback (first 2 bytes per plane):\r\n");
		b0 = XDc_ReadReg(IN_BUFFER_0_ADDR_V4, 0);
		b1 = XDc_ReadReg(IN_BUFFER_0_ADDR_V4, 1);
		xil_printf("    V4: 0x%02x 0x%02x\r\n", b0 & 0xFF, b1 & 0xFF);

		if (attr->Mode == SemiPlanar || attr->Mode == Planar || attr->Mode == Tiled) {
			b0 = XDc_ReadReg(IN_BUFFER_0_ADDR_V5, 0);
			b1 = XDc_ReadReg(IN_BUFFER_0_ADDR_V5, 1);
			xil_printf("    V5: 0x%02x 0x%02x\r\n", b0 & 0xFF, b1 & 0xFF);
		}
		if (attr->Mode == Planar || attr->Mode == Tiled) {
			b0 = XDc_ReadReg(IN_BUFFER_0_ADDR_V6, 0);
			b1 = XDc_ReadReg(IN_BUFFER_0_ADDR_V6, 1);
			xil_printf("    V6: 0x%02x 0x%02x\r\n", b0 & 0xFF, b1 & 0xFF);
		}
	}

	xil_printf("\r\n");
}
