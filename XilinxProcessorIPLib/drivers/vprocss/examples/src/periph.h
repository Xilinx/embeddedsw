/******************************************************************************
*
* (c) Copyright 2014 - 2015 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information
* of Xilinx, Inc. and is protected under U.S. and
* international copyright and other intellectual property
* laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any
* rights to the materials distributed herewith. Except as
* otherwise provided in a valid license issued to you by
* Xilinx, and to the maximum extent permitted by applicable
* law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
* WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
* AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
* BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
* INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
* (2) Xilinx shall not be liable (whether in contract or tort,
* including negligence, or under any other theory of
* liability) for any loss or damage of any kind or nature
* related to, arising under or in connection with these
* materials, including for any direct, or any indirect,
* special, incidental, or consequential loss or damage
* (including loss of data, profits, goodwill, or any type of
* loss or damage suffered as a result of any action brought
* by a third party) even if such damage or loss was
* reasonably foreseeable or Xilinx had been advised of the
* possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-
* safe, or for use in any application requiring fail-safe
* performance, such as life-support or safety devices or
* systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any
* other applications that could lead to death, personal
* injury, or severe property or environmental damage
* (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and
* liability of any use of Xilinx products in Critical
* Applications, subject only to applicable laws and
* regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
* PART OF THIS FILE AT ALL TIMES.
*
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
#include "microblaze_sleep.h"

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
