/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_letterbox_l2.h
* @addtogroup v_letterbox_v2_2
* @{
* @details
*
* This header file contains layer 2 API's of the letter box sub-core driver.
* The functions contained herein provides a high level implementation of
* features provided by the IP, abstracting away the register level details from
* the user
*
* <b>Letter Box IP Features </b>
*
* This Letterbox IP supports following features
* 	- Apply black bars (horizontal or vertical) on input stream
* 	- Programmable background colors
* 	- up to 16bits color depth
*	- 1, 2 or 4 pixel per clock processing
*	- Input stream up to 4k2k 60Hz
*
* <b>Dependency</b>
*
* This driver makes use of the video enumerations and data types defined in the
* Xilinx Video Common Driver (video_common_vX.x) and as such the common driver
* must be included as dependency to compile this driver

* <b>Initialization & Configuration</b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate to the lbox core.
*
* Before using the layer-2 API's user must initialize the core by calling
* Layer-1 API XV_letterbox_Initialize(). This function will look for a
* configuration structure for the device and initialize it to defined HW
* settings. After initialization Layer-2 API's can be used to configure
* the core. It is recommended user always make use of Layer-2 API to
* interact with this core.
* Advanced users always have the capability to directly interact with the IP
* core using Layer-1 API's that perform low level register peek/poke.
*
* <b> Interrupts </b>
*
* This driver does not have any interrupts
*
* <b> Virtual Memory </b>
*
* This driver supports Virtual Memory. The RTOS is responsible for calculating
* the correct device base address in Virtual Memory space.
*
* <b> Threads </b>
*
* This driver is not thread safe. Any needs for threads or thread mutual
* exclusion must be satisfied by the layer above this driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rco   07/21/15   Initial Release
* 2.00  rco   11/05/15   Integrate layer-1 with layer-2
*
* </pre>
*
******************************************************************************/
#ifndef XV_LETTERBOX_L2_H		 /* prevent circular inclusions */
#define XV_LETTERBOX_L2_H		 /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xvidc.h"
#include "xv_letterbox.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
/**
 * This typedef contains the different background colors available
 */
typedef enum
{
  XLBOX_BKGND_BLACK = 0,
  XLBOX_BKGND_WHITE,
  XLBOX_BKGND_RED,
  XLBOX_BKGND_GREEN,
  XLBOX_BKGND_BLUE,
  XLBOX_BKGND_LAST
}XLboxColorId;

/**
 * Letterbox Layer 2 data. The user is required to allocate a variable
 * of this type for every V Scaler device in the system. A pointer to a
 * variable of this type is then passed to the driver API functions.
 */
typedef struct
{
  XV_letterbox Lbox; /*<< Layer 1 instance */

}XV_Lbox_l2;

/************************** Function Prototypes ******************************/
int XV_LBoxInitialize(XV_Lbox_l2 *InstancePtr, u16 DeviceId);
void XV_LBoxStart(XV_Lbox_l2 *InstancePtr);
void XV_LBoxStop(XV_Lbox_l2 *InstancePtr);

void XV_LBoxSetActiveWin(XV_Lbox_l2 *InstancePtr,
                         XVidC_VideoWindow *ActiveWindow,
                         u32 FrameWidth,
                         u32 FrameHeight);

void XV_LboxSetBackgroundColor(XV_Lbox_l2    *InstancePtr,
                               XLboxColorId  ColorId,
                               XVidC_ColorFormat cfmt,
                               XVidC_ColorDepth  bpc);

void XV_LBoxDbgReportStatus(XV_Lbox_l2 *InstancePtr);

#ifdef __cplusplus
}
#endif
#endif
/** @} */
