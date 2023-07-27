/******************************************************************************
* Copyright (C) 2014 - 2022 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file periph.h
*
* This is header for resource file that will initialize all system
* level peripherals
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date   Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rc   07/07/14 First release
* 2.00  dmc  12/02/15 Removed UART driver instance
*            01/25/16 Support new GPIO instance to reset HLS IP inside the VPSS
*
* </pre>
*
******************************************************************************/
#ifndef XPERIPH_H		 /* prevent circular inclusions */
#define XPERIPH_H		 /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "string.h"
#include "xvidc.h"
#include "xv_tpg.h"
#include "xgpio.h"
#include "xvtc.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/

/************************** Structure Definitions *****************************/
typedef struct
{
  u16 Width;
  u16 Height;
  XVidC_ColorFormat ColorFmt;
  u16 IsInterlaced;
  u16 Pattern;
}XPeriph_TpgConfig;

/**
 * System Peripheral configuration structure.
 * Each device should have a configuration structure associated
 */

typedef struct
{
  XV_tpg    *TpgPtr;
  XVtc      *VtcPtr;
  XGpio     *VidLockMonitorPtr;
  XGpio     *HlsIpResetPtr;
  XPeriph_TpgConfig TpgConfig;
}XPeriph;

/************************** Constant Definitions *******************************/
#define XPER_GPIO_CHANNEL_1 1
#define XPER_GPIO_CHANNEL_2 2


/************************** Macros Definitions *******************************/

/*****************************************************************************/
/**
 * This macro sets the TPG color format
 *
 * @param  pPeriph is pointer to the peripheral Instance
 * @param  ColorFormat  is the new color format
 * @return none
 *
 *****************************************************************************/
#define XPeriph_SetTPGColorFormat(pPeriph, ColorFormat) \
	                      ((pPeriph)->TpgConfig.ColorFmt = ColorFormat)

/*****************************************************************************/
/**
 * This macro sets the TPG pattern
 *
 * @param  pPeriph is pointer to the peripheral Instance
 * @param  pattern  is the new pattern id
 * @return none
 *
 *****************************************************************************/
#define XPeriph_SetTPGPattern(pPeriph, Pattern) \
	                      ((pPeriph)->TpgConfig.Pattern = Pattern)

/*****************************************************************************/
/**
 * This macro sets TPG active width
 *
 * @param  pPeriph is pointer to the peripheral Instance
 * @param  width  is the new active width
 * @return none
 *
 *****************************************************************************/
#define XPeriph_SetTPGWidth(pPeriph, width) \
	                      ((pPeriph)->TpgConfig.Width = width)

/*****************************************************************************/
/**
 * This macro sets TPG active height
 *
 * @param  pPeriph is pointer to the peripheral Instance
 * @param  height  is the new active height
 * @return none
 *
 *****************************************************************************/
#define XPeriph_SetTPGHeight(pPeriph, height) \
	                      ((pPeriph)->TpgConfig.Height = height)

/*****************************************************************************/
/**
 * This macro sets TPG Interlaced Mode
 *
 * @param  pPeriph is pointer to the peripheral Instance
 * @param  mode  is the interlace mode T/F
 * @return none
 *
 *****************************************************************************/
#define XPeriph_SetTPGInterlacedMode(pPeriph, mode) \
	                      ((pPeriph)->TpgConfig.IsInterlaced = mode)

/*****************************************************************************/
/**
 * This macro reads GPIO to check video lock status
 *
 * @param  pPeriph is pointer to the peripheral Instance
 * @return T/F
 *
 *****************************************************************************/
#define XPeriph_IsVideoLocked(pPeriph) \
	                     (XGpio_DiscreteRead((pPeriph)->VidLockMonitorPtr, \
                          XPER_GPIO_CHANNEL_1))

/************************** Exported APIs ************************************/
int XPeriph_PowerOnInit(XPeriph *InstancePtr);
void XPeriph_ResetHlsIp(XPeriph *InstancePtr);
void XPeriph_ReportDeviceInfo(XPeriph *InstancePtr);
void XPeriph_ConfigTpg(XPeriph *InstancePtr);
void XPeriph_ConfigVtc(XPeriph *InstancePtr,
		               XVidC_VideoStream *StreamPtr,
		               u32 PixPerClk);
void XPeriph_DisableTpg(XPeriph *InstancePtr);
void XPeriph_SetTpgParams(XPeriph *InstancePtr,
		                  u16 width,
		                  u16 height,
		                  XVidC_ColorFormat Cformat,
		                  u16 Pattern,
		                  u16 IsInterlaced);

void XPeriph_TpgDbgReportStatus(XPeriph *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
