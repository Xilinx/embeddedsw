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
* @file xv_vscaler_l2.h
* @addtogroup v_vscaler_v1_0
* @{
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
* Current implementation of "computed coefficients" is non-entrant. If multiple
* instances of the core are used in the design, user application has to make sure
* that scale configuration is an atomic operation and cannot be interrupted in
* middle to context switch to a different instance of the core
* However fixed coefficient version of the code (default) is safe to be used with
* multiple instances of the core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rc   05/01/15   Initial Release

* </pre>
*
******************************************************************************/
#ifndef XV_VSCALER_L2_H 	 /* prevent circular inclusions */
#define XV_VSCALER_L2_H		 /* by using protection macros  */

#ifdef __cplusplus
extern "C" {
#endif

#include "xv_vscaler.h"

/************************** Constant Definitions *****************************/
 /** @name Hw Configuration
  * @{
  * The following constants define the scaler HW configuration
  * TODO:
  * Below defined Parameters are static configuration of H Scaler IP
  * The tool needs to export these parameters to SDK and the driver will
  * be populated with these option settings. i.e. these settings could be
  * accessible via instance pointer
  *
  */
 #define XV_VSCALER_MAX_V_TAPS           (10)
 #define XV_VSCALER_MAX_V_PHASES         (64)

/**************************** Type Definitions *******************************/
/**
 * This typedef contains the Scaler Type
 */
typedef enum
{
  XV_VSCALER_BILINEAR = 0,
  XV_VSCALER_BICUBIC,
  XV_VSCALER_POLYPHASE
}XV_VSCALER_TYPE;

/**
 * This typedef contains the type of filters available for scaling operation
 */
typedef enum
{
  XV_VFILT_LANCZOS = 0,
  XV_VFILT_WINDOWED_SINC
}XV_VFILTER_ID;


/**
 * V Scaler Layer 2 data. The user is required to allocate a variable
 * of this type for every V Scaler device in the system. A pointer to a
 * variable of this type is then passed to the driver API functions.
 */
typedef struct
{
  u8 UseExtCoeff;
  XV_VFILTER_ID FilterSel;
  short coeff[XV_VSCALER_MAX_V_PHASES][XV_VSCALER_MAX_V_TAPS];
}XV_vscaler_l2;

/************************** Macros Definitions *******************************/
/*****************************************************************************/
/**
 * This macro selects the filter used for generating coefficients.
 * Applicable only for Ployphase Scaler Type
 *
 * @param  pVscL2Data is pointer to the V Scaler Layer 2 structure instance
 * @param  value is the filter type
 *
 * @return None
 *
 *****************************************************************************/
#define XV_VScalerSetFilterType(pVscL2Data, value)  \
                         ((pVscL2Data)->FilterSel = value)

/************************** Function Prototypes ******************************/
void XV_VScalerStart(XV_vscaler *InstancePtr);
void XV_VScalerStop(XV_vscaler *InstancePtr);
void XV_VscalerLoadUsrCoeffients(XV_vscaler *InstancePtr,
                                 XV_vscaler_l2 *pVscL2Data,
                                 u16 num_phases,
                                 u16 num_taps,
                                 const short *Coeff);
void XV_VScalerSetup(XV_vscaler *InstancePtr,
                     XV_vscaler_l2 *pVscL2Data,
                     u32 WidthIn,
                     u32 HeightIn,
                     u32 HeightOut);

void XV_VScalerDbgReportStatus(XV_vscaler *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
/** @} */
