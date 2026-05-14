/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xavpg.c
 *
 * This file contains AV Pattern Generator helper core configuration
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver  Who Date      Changes
 * ---- --- --------  --------------------------------------------------
 * 1.00 vsa 05/01/26  Initial version
 *
 * </pre>
 *
 ******************************************************************************/

#include "xavpg.h"
#include "mmi_dpdc_example.h"

/*****************************************************************************/
/**
 *
 * This function configures the AV Pattern Generator with the specified
 * resolution, format, and pattern settings.
 *
 * @param    BaseAddr - Base address of the AV Pattern Generator
 * @param    avpgcfg - Pointer to the AV Pattern Generator run configuration
 * @param    width - Horizontal resolution in pixels
 * @param    height - Vertical resolution in pixels
 *
 * @return   None
 *
 * @note     The pattern generator is disabled before reconfiguration.
 *
 ******************************************************************************/
void XAvpgSetConfig(UINTPTR BaseAddr, AvpgRunConfig *avpgcfg, u32 width,
		u32 height)
{
	u32 val;

	XAvpg_WriteReg(BaseAddr, XAV_PATGEN_EN, 0);
	XAvpg_WriteReg(BaseAddr, XAV_PATGEN_VRES, height);
	XAvpg_WriteReg(BaseAddr, XAV_PATGEN_HRES, width / (avpgcfg->ppc ? XAVPATGEN_PPC_QUAD : XAVPATGEN_PPC_DUAL));

	val = avpgcfg->pix_fmt << XAVPATGEN_PIXFMT_SHIFT |
		avpgcfg->colorimetry << XAVPATGEN_COLORIMETRY_SHIFT;

	switch (avpgcfg->bpc) {
		case 8:
			val |= (XAVPATGEN_BPC_8 << XAVPATGEN_BPC_SHIFT);
			break;
		case 10:
			val |= (XAVPATGEN_BPC_10 << XAVPATGEN_BPC_SHIFT);
			break;
		case 12:
			val |= (XAVPATGEN_BPC_12 << XAVPATGEN_BPC_SHIFT);
			break;
	}

	XAvpg_WriteReg(BaseAddr, XAV_PATGEN_FORMAT_BPC, val);

	val = avpgcfg->pattern;
	if (avpgcfg->ppc)
		/* Quad pixels per clock */
		val |= XAVPATGEN_PPC_QUAD_MASK;
	else
		/* Dual pixels per clock */
		val |= XAVPATGEN_PPC_DUAL_MASK;

	XAvpg_WriteReg(BaseAddr, XAV_PATGEN_MODE_PATTERN, val);

}
