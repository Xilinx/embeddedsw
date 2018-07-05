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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xv_hcresampler_l2.h
* @addtogroup v_hcresampler_v3_0
* @{
* @details
*
* This header file contains layer 2 API's of the horizontal chroma resampler
* sub-core driver.The functions contained herein provides a high level
* implementation of features provided by the IP, abstracting away the register
* level details from the user
*
* <b>H Chroma Resampler IP Features </b>
*
* The horizontal chroma resampler IP supports following features
* 	- convert YUV422 -> YUV444 and YUV444 -> YUV422
*	- Supports resolution up to 4k2k 60Hz
*	- up to 16 bits color depth
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
* communicate to the hcresampler core.
*
* Before using the layer-2 API's user must initialize the core by calling
* Layer-1 API XV_hcresampler_Initialize(). This function will look for a
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
#ifndef XV_HCHROMA_RESAMPLER_L2_H	   /* prevent circular inclusions */
#define XV_HCHROMA_RESAMPLER_L2_H    /* by using protection macros  */

#ifdef __cplusplus
extern "C" {
#endif

#include "xvidc.h"
#include "xv_hcresampler.h"

/************************** Constant Definitions *****************************/
 /** @name Hw Configuration
   * @{
   * The following constants define the horiz. resampler HW MAX configuration
   *
   */
#define XV_HCRSMPLR_MAX_TAPS           (10)
#define XV_HCRSMPLR_MAX_PHASES         (2)

/**************************** Type Definitions *******************************/
 /**
  * This typedef enumerates the supported taps
  */
 typedef enum
 {
   XV_HCRSMPLR_TAPS_4  = 4,
   XV_HCRSMPLR_TAPS_6  = 6,
   XV_HCRSMPLR_TAPS_8  = 8,
   XV_HCRSMPLR_TAPS_10 = 10,
   XV_HCRSMPLR_NUM_SUPPORTED_TAPS_CONFIG = 4
 }XV_HCRESAMPLER_TAPS;

 /**
  * This typedef enumerates the conversion type
  */
 typedef enum
 {
   XV_HCRSMPLR_444_TO_422  = 0,
   XV_HCRSMPLR_422_TO_444 ,
   XV_HCRSMPLR_NUM_CONVERSIONS
 }XV_HCRESAMPLER_CONVERSION;

 /** This typedef enumerates the resampling algorithm
  *
  */
 typedef enum
 {
   XV_HCRSMPLR_TYPE_NEAREST_NEIGHBOR = 0,
   XV_HCRSMPLR_TYPE_FIXED_COEFFICIENT,
   XV_HCRSMPLR_TYPE_FIR
 }XV_HCRESAMPLER_TYPE;

 /**
  * H Chroma Resampler Layer 2 data. The user is required to allocate a
  * variable of this type for every H chroma resampler device in the system.
  * A pointer to a variable of this type is then passed to the driver API
  * functions.
  */
 typedef struct
 {
   XV_hcresampler Hcr;      /*<< Layer 1 instance */
   u16 UseExtCoeff;
   short coeff[XV_HCRSMPLR_NUM_CONVERSIONS][XV_HCRSMPLR_MAX_PHASES][XV_HCRSMPLR_MAX_TAPS];
 }XV_Hcresampler_l2;

/************************** Function Prototypes ******************************/
int XV_HcrsmplInitialize(XV_Hcresampler_l2 *InstancePtr, u16 DeviceId);
void XV_HCrsmplStart(XV_Hcresampler_l2 *InstancePtr);
void XV_HCrsmplStop(XV_Hcresampler_l2 *InstancePtr);
void XV_HCrsmplLoadDefaultCoeff(XV_Hcresampler_l2 *InstancePtr);
void XV_HCrsmplrLoadExtCoeff(XV_Hcresampler_l2 *InstancePtr,
                             u16 num_taps,
                             const short *Coeff);

void XV_HCrsmplSetActiveSize(XV_Hcresampler_l2 *InstancePtr,
                             u32 width,
                             u32 height);

void XV_HCrsmplSetFormat(XV_Hcresampler_l2 *InstancePtr,
                         XVidC_ColorFormat formatIn,
                         XVidC_ColorFormat formatOut);

void XV_HCrsmplDbgReportStatus(XV_Hcresampler_l2 *InstancePtr);

#ifdef __cplusplus
}
#endif
#endif
/** @} */
