/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file mmi_dc_partial_plane.c
*
* This file contains partial plane blend configuration functionality for the
* Display Controller. Partial plane blending allows displaying a portion of
* a video stream at a specified location with configurable offset.
*
******************************************************************************/

#include <xil_printf.h>
#include "mmi_dc_partial_plane.h"
#include "mmi_dc_nonlive_test.h"

/*****************************************************************************/
/**
*
* This function configures partial plane blend for Stream1 or Stream2 using
* DCsub APIs
*
* @param    RunCfgPtr - Pointer to RunConfig structure
*
* @return   None
*
* @note     Partial plane blending allows displaying a rectangular region
*           from a video stream at a specific position. Configuration includes:
*           - CoordX, CoordY: Position of the partial plane on the output
*           - SizeX, SizeY: Dimensions of the partial plane region
*           - OffsetX, OffsetY: Offset within the source stream
*
*           This function uses the XDcSub_SetStreamPartialBlend API to
*           configure partial plane blend parameters from RunConfig.
*
*           Note: XDcSub_SetStreamPartialBlend stores a POINTER to the coords
*           structure, so we use static storage to ensure it remains valid.
*
******************************************************************************/
void XDpDc_ConfigurePartialPlaneBlend(RunConfig *RunCfgPtr)
{
	u32 Status;
	static XDc_PartialBlend Stream1Coords;
	static XDc_PartialBlend Stream2Coords;

	xil_printf("\r\n=== Partial Plane Blend Configuration ===\r\n");

	/* Check if partial plane blend is enabled */
	if (RunCfgPtr->Stream1PbEnable == PB_ENABLE) {
		xil_printf("  - Stream1 Partial Plane: ENABLED\r\n");
		xil_printf("  - Stream2 Partial Plane: DISABLED\r\n");

		/* Populate Stream1 partial plane coordinates from RunConfig */
		Stream1Coords.CoordX = RunCfgPtr->PpbCoordX;
		Stream1Coords.CoordY = RunCfgPtr->PpbCoordY;
		Stream1Coords.SizeX = RunCfgPtr->PpbSizeX;
		Stream1Coords.SizeY = RunCfgPtr->PpbSizeY;
		Stream1Coords.OffsetX = RunCfgPtr->PpbOffsetX;
		Stream1Coords.OffsetY = RunCfgPtr->PpbOffsetY;

		/* Use DCsub API to configure Stream1 partial plane blend */
		Status = XDcSub_SetStreamPartialBlend(RunCfgPtr->DcSubPtr,
		                                      PB_ENABLE, &Stream1Coords,
		                                      PB_DISABLE, NULL);

		if (Status != XST_SUCCESS) {
			xil_printf("ERROR: Failed to configure Stream1 partial plane blend\r\n");
			return;
		}

		xil_printf("  - Position: (%d, %d)\r\n", Stream1Coords.CoordX, Stream1Coords.CoordY);
		xil_printf("  - Size: %dx%d\r\n", Stream1Coords.SizeX, Stream1Coords.SizeY);
		xil_printf("  - Offset: (%d, %d)\r\n", Stream1Coords.OffsetX, Stream1Coords.OffsetY);

	} else if (RunCfgPtr->Stream2PbEnable == PB_ENABLE) {
		xil_printf("  - Stream1 Partial Plane: DISABLED\r\n");
		xil_printf("  - Stream2 Partial Plane: ENABLED\r\n");

		/* Populate Stream2 partial plane coordinates from RunConfig */
		Stream2Coords.CoordX = RunCfgPtr->PpbCoordX;
		Stream2Coords.CoordY = RunCfgPtr->PpbCoordY;
		Stream2Coords.SizeX = RunCfgPtr->PpbSizeX;
		Stream2Coords.SizeY = RunCfgPtr->PpbSizeY;
		Stream2Coords.OffsetX = RunCfgPtr->PpbOffsetX;
		Stream2Coords.OffsetY = RunCfgPtr->PpbOffsetY;

		/* Use DCsub API to configure Stream2 partial plane blend */
		Status = XDcSub_SetStreamPartialBlend(RunCfgPtr->DcSubPtr,
		                                      PB_DISABLE, NULL,
		                                      PB_ENABLE, &Stream2Coords);

		if (Status != XST_SUCCESS) {
			xil_printf("ERROR: Failed to configure Stream2 partial plane blend\r\n");
			return;
		}

		xil_printf("  - Position: (%d, %d)\r\n", Stream2Coords.CoordX, Stream2Coords.CoordY);
		xil_printf("  - Size: %dx%d\r\n", Stream2Coords.SizeX, Stream2Coords.SizeY);
		xil_printf("  - Offset: (%d, %d)\r\n", Stream2Coords.OffsetX, Stream2Coords.OffsetY);

	} else {
		xil_printf("  - Partial Plane Blend: DISABLED\r\n");

		/* Use DCsub API to disable partial plane blend for both streams */
		Status = XDcSub_SetStreamPartialBlend(RunCfgPtr->DcSubPtr,
		                                      PB_DISABLE, NULL,
		                                      PB_DISABLE, NULL);

		if (Status != XST_SUCCESS) {
			xil_printf("ERROR: Failed to disable partial plane blend\r\n");
			return;
		}
	}

	xil_printf("  - Partial plane blend configured successfully\r\n\r\n");
}
