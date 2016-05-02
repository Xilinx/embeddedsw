/******************************************************************************
 *
 * Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xv_hscaler_l2.h
* @addtogroup v_hscaler_v1_0
* @{
* @details
*
* This header file contains layer 2 API's of the horizontal scaler sub-core
* driver.The functions contained herein provides a high level implementation of
* features provided by the IP, abstracting away the register level details from
* the user
*
* <b>H Scaler IP Features </b>
*
* This H-Scaler IP supports following features
* 	- 3 Channel Scaler with RGB, YUV444 and optional YUV422, and YUV420 support
* 	- Scale horizontally to 4K line at 60Hz
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
* communicate to the hscaler core.
*
* Before using the layer-2 API's user must initialize the core by calling
* Layer-1 API XV_hscaler_Initialize(). This function will look for a
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
*       dmc   12/17/15   Add macro to query the Is422Enabled flag that was
*                        added to the XV_hscaler_Config structure
* 3.0   mpe   04/28/16   Added optional color format conversion handling
* </pre>
*
******************************************************************************/
#ifndef XV_HSCALER_L2_H        /* prevent circular inclusions */
#define XV_HSCALER_L2_H        /* by using protection macros  */

#ifdef __cplusplus
extern "C" {
#endif

#include "xvidc.h"
#include "xv_hscaler.h"

/************************** Constant Definitions *****************************/
/** @name Hw Configuration
 * @{
 * The following constants define the scaler HW MAX configuration
 */
#define XV_HSCALER_MAX_H_TAPS           (12)
#define XV_HSCALER_MAX_H_PHASES         (64)
#define XV_HSCALER_MAX_LINE_WIDTH       (3840)

/**************************** Type Definitions *******************************/
/**
 * This typedef enumerates the Scaler Type
 */
typedef enum
{
  XV_HSCALER_BILINEAR = 0,
  XV_HSCALER_BICUBIC,
  XV_HSCALER_POLYPHASE
}XV_HSCALER_TYPE;

/**
 * This typedef enumerates the supported taps
 */
typedef enum
{
  XV_HSCALER_TAPS_6  = 6,
  XV_HSCALER_TAPS_8  = 8,
  XV_HSCALER_TAPS_10 = 10,
  XV_HSCALER_TAPS_12 = 12
}XV_HSCALER_TAPS;

/**
 * H Scaler Layer 2 data. The user is required to allocate a variable
 * of this type for every H Scaler device in the system. A pointer to a
 * variable of this type is then passed to the driver API functions.
 */
typedef struct
{
  XV_hscaler Hsc; /*<< Layer 1 instance */
  u8 UseExtCoeff;
  short coeff[XV_HSCALER_MAX_H_PHASES][XV_HSCALER_MAX_H_TAPS];
  u64 phasesH[XV_HSCALER_MAX_LINE_WIDTH];
}XV_Hscaler_l2;

/************************** Macros Definitions *******************************/

/*****************************************************************************/
/**
 * This macro checks if Hscaler instance is enabled for 4:2:2 processing
 *
 * @param  InstancePtr is pointer to Hscaler core layer 2
 *
 * @return Returns 1 if condition is TRUE or 0 if FALSE
 *
 *****************************************************************************/
#define XV_HscalerIs422Enabled(InstancePtr) \
   ((InstancePtr)->Hsc.Config.Is422Enabled)

/*****************************************************************************/
/**
 * This macro checks if Hscaler instance is enabled for 4:2:0 processing
 *
 * @param  InstancePtr is pointer to Hscaler core layer 2
 *
 * @return Returns 1 if condition is TRUE or 0 if FALSE
 *
 *****************************************************************************/
#define XV_HscalerIs420Enabled(InstancePtr) \
   ((InstancePtr)->Hsc.Config.Is420Enabled)

/*****************************************************************************/
/**
 * This macro checks if Hscaler instance is enabled for color space conversion
 *
 * @param  InstancePtr is pointer to Hscaler core layer 2
 *
 * @return Returns 1 if condition is TRUE or 0 if FALSE
 *
 *****************************************************************************/
#define XV_HscalerIsCscEnabled(InstancePtr) \
   ((InstancePtr)->Hsc.Config.IsCscEnabled)

/************************** Function Prototypes ******************************/
int XV_HScalerInitialize(XV_Hscaler_l2 *InstancePtr, u16 DeviceId);
void XV_HScalerStart(XV_Hscaler_l2 *InstancePtr);
void XV_HScalerStop(XV_Hscaler_l2 *InstancePtr);
void XV_HScalerLoadExtCoeff(XV_Hscaler_l2 *InstancePtr,
                            u16 num_phases,
                            u16 num_taps,
                            const short *Coeff);
int XV_HScalerSetup(XV_Hscaler_l2  *InstancePtr,
                     u32 HeightIn,
                     u32 WidthIn,
                     u32 WidthOut,
                     u32 cformat,
                     u32 cformatOut);
int XV_HScalerValidateConfig(XV_Hscaler_l2 *InstancePtr,
                             u32 ColorFormatIn,
                             u32 ColorFormatOut);
void XV_HScalerDbgReportStatus(XV_Hscaler_l2 *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
/** @} */
