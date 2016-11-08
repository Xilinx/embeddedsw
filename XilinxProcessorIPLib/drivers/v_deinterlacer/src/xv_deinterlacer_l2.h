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
* @file xv_deinterlacer_l2.h
* @addtogroup v_deinterlacer_v5_0
* @{
* @details
*
* This header file contains layer 2 API's of the deint sub-core driver.
* The functions contained herein provides a high level implementation of features
* provided by the IP, abstracting away the register level details from
* the user
*
* <b>Deinterlacer IP Features </b>
*
* Currently only 1080i input is supported
*
* <b>Dependency</b>
*
* This driver makes use of the video enumerations and data types defined in the
* Xilinx Video Common Driver (video_common_vX.x) and as such the common driver
* must be included as dependency to compile this driver
*
* <b>Initialization & Configuration</b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate to the deint core.
*
* Before using the layer-2 API's user must initialize the core by calling
* Layer-1 API XV_deinterlacer_Initialize(). This function will look for a
* configuration structure for the device and initialize it to defined HW
* settings. After initialization Layer-2 API's can be used to configure
* the core. It is recommended user always make use of Layer-2 API to interact
* with core.
* Advanced users always have the capability to directly interact with the
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
* 5.00  rco   07/21/15   Initial Release
* 6.00  rco   11/05/15   Integrate layer-1 with layer-2
*       dmc   02/25/16   add public routine XV_DeintWaitForIdle()
* 6.1   rco   11/07/16   Fix for c++ compile
*
* </pre>
*
******************************************************************************/
#ifndef XV_DEINTERALCER_L2_H          /* prevent circular inclusions */
#define XV_DEINTERALCER_L2_H          /* by using protection macros  */

#ifdef __cplusplus
extern "C" {
#endif

#include "xvidc.h"
#include "xv_deinterlacer.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**
 * Deinterlacer Layer 2 data. The user is required to allocate a variable
 * of this type for every V Scaler device in the system. A pointer to a
 * variable of this type is then passed to the driver API functions.
 */
typedef struct
{
  XV_deinterlacer Deint; /*<< Layer 1 instance */

}XV_Deint_l2;

/************************** Function Prototypes ******************************/
int XV_DeintInitialize(XV_Deint_l2 *InstancePtr, u16 DeviceId);
void XV_DeintStart(XV_Deint_l2 *InstancePtr);
void XV_DeintStop(XV_Deint_l2 *InstancePtr);
int XV_DeintWaitForIdle(XV_Deint_l2 *InstancePtr);
void XV_DeintSetFieldBuffers(XV_Deint_l2   *InstancePtr,
							 u32 memAddr,
							 XVidC_ColorFormat cformat);

void XV_DeintDbgReportStatus(XV_Deint_l2 *InstancePtr);

#ifdef __cplusplus
}
#endif
#endif
/** @} */
