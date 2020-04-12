/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_vscaler_l2.h
* @addtogroup v_vscaler_v3_1
* @{
* @details
*
* This header file contains layer 2 API's of the verrtical scaler sub-core
* driver.The functions contained herein provides a high level implementation of
* features provided by the IP, abstracting away the register level details from
* the user
*
* <b>V Scaler IP Features </b>
*
* This V-Scaler IP supports following features
* 	- 3 Channel Scaler
* 	- Scale vertically to 2K lines at 60Hz
* 	- up to 16bits color depth
*	- 1, 2 or 4 pixel per clock processing
*
* <b>Dependency</b>
*
* This driver makes use of the video enumerations and data types defined in the
* Xilinx Video Common Driver (video_common_vX.x) and as such the common driver
* must be included as dependency to compile this driver

* <b>Initialization & Configuration</b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate to the vscaler core.
*
* Before using the layer-2 API's user must initialize the core by calling
* Layer-1 API XV_vscaler_Initialize(). This function will look for a
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
* <b>Limitations</b>
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rco   07/21/15   Initial Release
* 2.00  rco   11/05/15   Integrate layer-1 with layer-2
* 3.0   mpe   04/28/16   Added optional color format conversion handling
* 3.1   vsa   04/07/20   Improve quality with new coefficients
*
* </pre>
*
******************************************************************************/
#ifndef XV_VSCALER_L2_H 	 /* prevent circular inclusions */
#define XV_VSCALER_L2_H		 /* by using protection macros  */

#ifdef __cplusplus
extern "C" {
#endif

#include "xvidc.h"
#include "xv_vscaler.h"

/************************** Constant Definitions *****************************/
 /** @name Hw Configuration
  * @{
  * The following constants define the scaler HW MAX configuration
  *
  */
 #define XV_VSCALER_MAX_V_TAPS           (12)
 #define XV_VSCALER_MAX_V_PHASES         (64)

/**************************** Type Definitions *******************************/
/**
 * This typedef eumerates the Scaler Type
 */
typedef enum
{
  XV_VSCALER_BILINEAR = 0,
  XV_VSCALER_BICUBIC,
  XV_VSCALER_POLYPHASE
}XV_VSCALER_TYPE;

/**
 * This typedef enumerates the supported taps
 */
typedef enum
{
  XV_VSCALER_TAPS_6  = 6,
  XV_VSCALER_TAPS_8  = 8,
  XV_VSCALER_TAPS_10 = 10,
  XV_VSCALER_TAPS_12 = 12
}XV_VSCALER_TAPS;

/**
 * V Scaler Layer 2 data. The user is required to allocate a variable
 * of this type for every V Scaler device in the system. A pointer to a
 * variable of this type is then passed to the driver API functions.
 */
typedef struct
{
  XV_vscaler Vsc; /*<< Layer 1 instance */
  u8 UseExtCoeff;
  short coeff[XV_VSCALER_MAX_V_PHASES][XV_VSCALER_MAX_V_TAPS];
}XV_Vscaler_l2;

/************************** Macros Definitions *******************************/

/*****************************************************************************/
/**
 * This macro checks if Vscaler instance is enabled for 4:2:0 processing
 *
 * @param  InstancePtr is pointer to Vscaler core layer 2
 *
 * @return Returns 1 if condition is TRUE or 0 if FALSE
 *
 *****************************************************************************/
#define XV_VscalerIs420Enabled(InstancePtr) \
   ((InstancePtr)->Vsc.Config.Is420Enabled)

/************************** Function Prototypes ******************************/
int XV_VScalerInitialize(XV_Vscaler_l2 *InstancePtr, u16 DeviceId);
void XV_VScalerStart(XV_Vscaler_l2 *InstancePtr);
void XV_VScalerStop(XV_Vscaler_l2 *InstancePtr);
void XV_VScalerLoadExtCoeff(XV_Vscaler_l2 *InstancePtr,
                            u16 num_phases,
                            u16 num_taps,
                            const short *Coeff);
int XV_VScalerSetup(XV_Vscaler_l2  *InstancePtr,
                    u32 WidthIn,
                    u32 HeightIn,
                    u32 HeightOut,
                    u32 ColorFormat);
void XV_VScalerDbgReportStatus(XV_Vscaler_l2 *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
/** @} */
