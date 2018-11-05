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
* @file xv_vcresampler_l2.h
* @addtogroup v_vcresampler_v3_0
* @{
* @details
*
* This header file contains layer 2 API's of the vertical chroma resampler
* sub-core driver.The functions contained herein provides a high level
* implementation of features provided by the IP, abstracting away the register
* level details from the user
*
* <b>V Chroma Resampler IP Features </b>
*
* The vertical chroma resampler IP supports following features
* 	- convert YUV420 -> YUV422 and YUV422 -> YUV420
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
* communicate to the vcresampler core.
*
* Before using the layer-2 API's user must initialize the core by calling
* Layer-1 API XV_vcresampler_Initialize(). This function will look for a
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
* 1.00  rc   07/21/15   Initial Release
* 2.00  rco   11/05/15   Integrate layer-1 with layer-2
*
* </pre>
*
******************************************************************************/
#ifndef XV_VCHROMA_RESAMPLER_L2_H	   /* prevent circular inclusions */
#define XV_VCHROMA_RESAMPLER_L2_H    /* by using protection macros  */

#ifdef __cplusplus
extern "C" {
#endif

#include "xvidc.h"
#include "xv_vcresampler.h"

/************************** Constant Definitions *****************************/
 /** @name Hw Configuration
    * @{
    * The following constants define the vert. resampler HW MAX configuration
    *
    */
#define XV_VCRSMPLR_MAX_TAPS           (10)
#define XV_VCRSMPLR_MAX_PHASES         (2)

/**************************** Type Definitions *******************************/
 /**
  * This typedef enumerates the supported taps
  */
 typedef enum
 {
   XV_VCRSMPLR_TAPS_4  = 4,
   XV_VCRSMPLR_TAPS_6  = 6,
   XV_VCRSMPLR_TAPS_8  = 8,
   XV_VCRSMPLR_TAPS_10 = 10,
   XV_VCRSMPLR_NUM_SUPPORTED_TAPS_CONFIG = 4
 }XV_VCRESAMPLER_TAPS;

 /**
  * This typedef enumerates the conversion type
  */
 typedef enum
 {
   XV_VCRSMPLR_422_TO_420  = 0,
   XV_VCRSMPLR_420_TO_422 ,
   XV_VCRSMPLR_NUM_CONVERSIONS
 }XV_VCRESAMPLER_CONVERSION;

 /** This typedef enumerates the resampling algorithm
  *
  */
 typedef enum
 {
   XV_VCRSMPLR_TYPE_NEAREST_NEIGHBOR = 0,
   XV_VCRSMPLR_TYPE_FIXED_COEFFICIENT,
   XV_VCRSMPLR_TYPE_FIR
 }XV_VCRESAMPLER_TYPE;

 /**
  * V Chroma Resampler Layer 2 data. The user is required to allocate a
  * variable of this type for every V chroma resampler device in the system.
  * A pointer to a variable of this type is then passed to the driver API
  * functions.
  */
 typedef struct
 {
   XV_vcresampler Vcr;   /*<< Layer 1 instance */
   u16 UseExtCoeff;
   short coeff[XV_VCRSMPLR_NUM_CONVERSIONS][XV_VCRSMPLR_MAX_PHASES][XV_VCRSMPLR_MAX_TAPS];
 }XV_Vcresampler_l2;

/************************** Function Prototypes ******************************/
int XV_VcrsmplInitialize(XV_Vcresampler_l2 *InstancePtr, u16 DeviceId);
void XV_VCrsmplStart(XV_Vcresampler_l2 *InstancePtr);
void XV_VCrsmplStop(XV_Vcresampler_l2 *InstancePtr);
void XV_VCrsmplLoadDefaultCoeff(XV_Vcresampler_l2 *InstancePtr);
void XV_VCrsmplrLoadExtCoeff(XV_Vcresampler_l2 *InstancePtr,
                             u16 num_taps,
                             const short *Coeff);

void XV_VCrsmplSetActiveSize(XV_Vcresampler_l2 *InstancePtr,
                             u32 width,
                             u32 height);

void XV_VCrsmplSetFormat(XV_Vcresampler_l2 *InstancePtr,
                         XVidC_ColorFormat formatIn,
                         XVidC_ColorFormat formatOut);

void XV_VCrsmplDbgReportStatus(XV_Vcresampler_l2 *InstancePtr);

#ifdef __cplusplus
}
#endif
#endif
/** @} */
