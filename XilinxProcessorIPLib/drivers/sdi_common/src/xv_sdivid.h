/*******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xv_sdivid.h
 * @addtogroup sdi_common Overview
 * @{
 * @details
 *
 * Contains common utility functions that are typically used by SDI IP
 * drivers and applications.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   jsr  07/17/17 Initial release.
 * 1.1   jsr  10/05/18 Moved 3GB specific video modes timing
 * 			parameters from video common library
 *			to SDI common driver
 * </pre>
 *
*******************************************************************************/

#ifndef XV_SDIVID_H_
/* Prevent circular inclusions by using protection macros. */
#define XV_SDIVID_H_

#ifdef __cplusplus
extern "C" {
#endif
/******************************* Include Files ********************************/

#include "xil_types.h"
#include "xvidc.h"

/************************** Constant Definitions ******************************/

//TODO: Incomplete list of video mode
/**
 * Payload and digital interface standards.
 */
typedef enum {
	XSDIVID_VM_483_576_SD = 0x81,
	XSDIVID_VM_720_HD = 0x84,
	XSDIVID_VM_1080_HD = 0x85,
	XSDIVID_VM_2X720_3GB = 0x88,
	XSDIVID_VM_1080_3GA = 0x89,
	XSDIVID_VM_2X720_3GB1 = 0x8B,
	XSDIVID_VM_1080_3GB = 0x8C,
	XSDIVID_VM_2160_6G = 0xC0,
	XSDIVID_VM_1080_6G = 0xC0,
	XSDIVID_VM_2160_12G = 0xCE
} XSdiVid_Standard;

/**
* SDI Modes
*/
typedef enum {
	XSDIVID_MODE_HD = 0,
	XSDIVID_MODE_SD = 1,
	XSDIVID_MODE_3GA = 2,
	XSDIVID_MODE_3GB = 3,
	XSDIVID_MODE_6G = 4,
	XSDIVID_MODE_12G = 5,
	XSDIVID_MODE_12GF = 6
} XSdiVid_TransMode;

/**
 * Channel Assignment.
 */
typedef enum {
	XSDIVID_BR_INTEGER = 0x0,
	XSDIVID_BR_FRACTIONAL = 0x1,
	XSDIVID_BR_UNKNOWN
} XSdiVid_BitRate;

/**
 * Channel Assignment.
 */
typedef enum {
	XSDIVID_CA_CH1 = 0x0,
	XSDIVID_CA_CH2 = 0x1,
	XSDIVID_CA_CH3 = 0x2,
	XSDIVID_CA_CH4 = 0x3,
	XSDIVID_CA_CH5 = 0x4,
	XSDIVID_CA_CH6 = 0x5,
	XSDIVID_CA_CH7 = 0x6,
	XSDIVID_CA_CH8 = 0x7,
	XSDIVID_CA_UNKNOWN
} XSdiVid_ChannelAssignment;

/**
 * End of Transfer Characteristics.
 */
typedef enum {
	XSDIVID_EOTF_SDRTV = 0x0,
	XSDIVID_EOTF_HLG = 0x1,
	XSDIVID_EOTF_SMPTE2084 = 0x2,
	XSDIVID_EOTF_UNKNOWN
} XSdiVid_Eotf;

/***************** Macros (Inline Functions) Definitions *********************/
#define XVIDC_SDICUSTOM_NUM_SUPPORTED 6

/****************************** Type Definitions ******************************/

//typedef struct {
//	XSdiVid_TransMode	TransMode;
//	XSdiVid_BitRate		IsFractional;
//} XSdiVid_Transport;

typedef struct {
  XSdiVid_TransMode	TMode;
  u8	ActiveStreams;
  u8	TScan;
  u8	TFamily;
  u8	TRate;
  u8	IsFractional;
  u8	IsLevelB3G;
} XSdiVid_Transport;

extern const XVidC_VideoTimingMode XVidC_SdiVidTimingModes[XVIDC_SDICUSTOM_NUM_SUPPORTED];
/*************************** Variable Declarations ****************************/

u8 GetPayloadColorFormat(XVidC_ColorFormat ColorFormat);
u32 GetPayloadColorDepth(XVidC_ColorDepth ColorDepth);
u32 GetPayloadFrameRate(XVidC_FrameRate FrameRate, XSdiVid_BitRate BitRate);
u32 GetPayloadIsInterlaced(XVidC_VideoFormat VideoFormat);
u32 GetPayloadAspectRatio(XVidC_AspectRatio AspectRatio);

#ifdef __cplusplus
}
#endif
#endif /* XV_SDIVID_H_ */
/** @} */
