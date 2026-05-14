/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdcsub_g.c
*
* This file contains a configuration table that specifies the configuration
* of DC Subsystem devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who     Date      Changes
* ---- ---     --------  --------------------------------------------------
* 1.00 vsa     05/01/26  Initial version
*
* </pre>
*
******************************************************************************/

/******************************* Include Files ********************************/

#include "xdcsub.h"

/**
  * The configuration table for devices
  */

XDcSub_Config XDcSub_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"amd,mmi-dc-1.0", /* compatible */
		0xedd00000, /* reg */
		"dp", /* reg-names */
		0x40b3, /* interrupts */
		0xe2000000, /* interrupt-parent */
		0x0, /* ports */
		"DC_Functional", /* xlnx,dc-operating-mode */
		"Live", /* xlnx,dc-presentation-mode */
		"Both", /* xlnx,dc-live-video-select */
		"Audio_&_Video", /* xlnx,dc-live-video01-mode */
		"Video_only", /* xlnx,dc-live-video02-mode */
		0x1, /* xlnx,dc-live-video-alpha-en */
		0x1, /* xlnx,dc-live-video-sdp-en */
		0x0, /* xlnx,dc-streams */
		0x0, /* xlnx,dc-stream0-mode */
		0x0, /* xlnx,dc-stream0-pixel-mode */
		0x0, /* xlnx,dc-stream0-sdp-en */
		0x0, /* xlnx,dc-stream1-mode */
		0x0, /* xlnx,dc-stream1-pixel-mode */
		0x0, /* xlnx,dc-stream1-sdp-en */
		0x0, /* xlnx,dc-stream2-mode */
		0x0, /* xlnx,dc-stream2-pixel-mode */
		0x0, /* xlnx,dc-stream2-sdp-en */
		0x0, /* xlnx,dc-stream3-mode */
		0x0, /* xlnx,dc-stream3-pixel-mode */
		0x0 /* xlnx,dc-stream3-sdp-en */
	},
	{
		 NULL
	}
};
