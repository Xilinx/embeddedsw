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
* @file xv_csc_l2.h
* @addtogroup v_csc_v1_0
* @{
* @details
*
* This header file contains layer 2 API's of the csc sub-core driver.
* The functions contained herein provides a high level implementation of features
* provided by the IP, abstracting away the register level details from
* the user
*
* <b>Color Space Converter IP Features </b>
*
* The CSC IP supports following features
*	- Set a Demo Window (user can select a sub-frame where above features
*	  will have effect)
*	- Supports resolution up to 4k2k 60Hz
*	- up to 16 bits color depth
*	- 1, 2 or 4 pixel per clock processing
*
* The Layer 2 driver of Color Space Conversion core offers following features
*	- Set/Get Brightness, contrast, saturation
*	- Set/Get Gain for R/G/B channel
*	- Set/Get Input/Output Color Standard (BT601, BT709, BT2020)
*	- Set/Get Input/Output Color Format (RGB, YUV444, YUV422)
*	- All settings are translated between user range (0-100) and IP supported
*	  range
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
* communicate to the CSC core.
*
* Before using the layer-2 API's user must initialize the core by calling
* Layer-1 API XV_csc_Initialize(). This function will look for a configuration
* structure for the device and initialize it to defined HW settings. After
* initialization Layer-2 API's can be used to configure the core. It is
* recommended user always make use of Layer-2 API to interact with the core
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
* 1.00  rco   07/21/15   Initial Release

* </pre>
*
******************************************************************************/
#ifndef XV_CSC_L2_H            /* prevent circular inclusions */
#define XV_CSC_L2_H            /* by using protection macros  */

#ifdef __cplusplus
extern "C" {
#endif

#include "xvidc.h"
#include "xv_csc.h"


/****************************** Type Definitions ******************************/
/**
 * CSC Layer 2 Register Map. Each instance of the csc core will have it's own
 * register map
 */
typedef enum
{
   CSC_FW_REG_ColStart = 0,
   CSC_FW_REG_ColEnd,
   CSC_FW_REG_RowStart,
   CSC_FW_REG_RowEnd,
   CSC_FW_REG_K11,
   CSC_FW_REG_K12,
   CSC_FW_REG_K13,
   CSC_FW_REG_K21,
   CSC_FW_REG_K22,
   CSC_FW_REG_K23,
   CSC_FW_REG_K31,
   CSC_FW_REG_K32,
   CSC_FW_REG_K33,
   CSC_FW_REG_ROffset,
   CSC_FW_REG_GOffset,
   CSC_FW_REG_BOffset,
   CSC_FW_REG_ClampMin,
   CSC_FW_REG_ClipMax,
   CSC_FW_REG_K11_2,
   CSC_FW_REG_K12_2,
   CSC_FW_REG_K13_2,
   CSC_FW_REG_K21_2,
   CSC_FW_REG_K22_2,
   CSC_FW_REG_K23_2,
   CSC_FW_REG_K31_2,
   CSC_FW_REG_K32_2,
   CSC_FW_REG_K33_2,
   CSC_FW_REG_ROffset_2,
   CSC_FW_REG_GOffset_2,
   CSC_FW_REG_BOffset_2,
   CSC_FW_REG_ClampMin_2,
   CSC_FW_REG_ClipMax_2,
   CSC_FW_NUM_REGS
}XV_CSC_FW_REG_MMAP;

/**
 * This typedef contains the layer 2 register map for a given
 * instance of the csc core.
 */
typedef struct
{
  XVidC_ColorFormat ColorFormatIn;
  XVidC_ColorFormat ColorFormatOut;
  XVidC_ColorStd StandardIn;
  XVidC_ColorStd StandardOut;
  XVidC_ColorRange OutputRange;
  XVidC_ColorDepth ColorDepth;
  s32 Brightness;
  s32 Contrast;
  s32 Saturation;
  s32 RedGain;
  s32 GreenGain;
  s32 BlueGain;
  s32 Brightness_active;
  s32 Contrast_active;
  s32 Saturation_active;
  s32 RedGain_active;
  s32 GreenGain_active;
  s32 BlueGain_active;
  s32 K_active[3][4];

  s32 regMap[CSC_FW_NUM_REGS];
}XV_csc_L2Reg;

/************************** Macros Definitions *******************************/
/*****************************************************************************/
/**
* This macro sets color depth for CSC core
*
* @param  pCscFwReg is a pointer to csc layer 2 fw register map
* @param  val is the requested color depth
*
* @return None
*
******************************************************************************/
#define XV_CscSetColorDepth(pCscFwReg, val)   ((pCscFwReg)->ColorDepth = val)

/*****************************************************************************/
/**
* This macro returns current brightness setting by reading layer 2 fw register
* map. It also translates between hw register value and user view
*
* @param  pCscFwReg is pointer to csc fw register associated with csc core
*         instance
*
* @return current user view value (0-100)
*
******************************************************************************/
#define XV_CscGetBrightness(pCscFwReg)        (((pCscFwReg)->Brightness-20)/2)

/*****************************************************************************/
/**
* This macro returns current contrast setting by reading layer 2 fw register
* map. It also translates between hw register value and user view
*
* @param  pCscFwReg is pointer to csc fw register associated with csc core
*         instance
*
* @return current user view value (0-100)
*
******************************************************************************/
#define XV_CscGetContrast(pCscFwReg)          (((pCscFwReg)->Contrast+200)/4)

/*****************************************************************************/
/**
* This macro returns current saturation setting by reading layer 2 fw register
* map. It also translates between hw register value and user view
*
* @param  pCscFwReg is pointer to csc fw register associated with csc core
*         instance
*
* @return current user view value (0-100)
*
******************************************************************************/
#define XV_CscGetSaturation(pCscFwReg)        (((pCscFwReg)->Saturation/2))

/*****************************************************************************/
/**
* This macro returns current red gain setting by reading layer 2 fw register
* map. It also translates between hw register value and user view
*
* @param  pCscFwReg is pointer to csc fw register associated with csc core
*         instance
*
* @return current user view value (0-100)
*
******************************************************************************/
#define XV_CscGetRedGain(pCscFwReg)           (((pCscFwReg)->RedGain-20)/2)

/*****************************************************************************/
/**
* This macro returns current green gain setting by reading layer 2 fw register
* map. It also translates between hw register value and user view
*
* @param  pCscFwReg is pointer to csc fw register associated with csc core
*         instance
*
* @return current user view value (0-100)
*
******************************************************************************/
#define XV_CscGetGreenGain(pCscFwReg)         (((pCscFwReg)->GreenGain-20)/2)

/*****************************************************************************/
/**
* This macro returns current blue gain setting by reading layer 2 fw register
* map. It also translates between hw register value and user view
*
* @param  pCscFwReg is pointer to csc fw register associated with csc core
*         instance
*
* @return current user view value (0-100)
*
******************************************************************************/
#define XV_CscGetBlueGain(pCscFwReg)          (((pCscFwReg)->BlueGain-20)/2)

/*****************************************************************************/
/**
* This macro returns current set input color format by reading layer 2 fw
* register map.
*
* @param  pCscFwReg is pointer to csc fw register associated with csc core
*         instance
*
* @return Current set input color format
*		- XVIDC_CSF_RGB
*		- XVIDC_CSF_YCRCB_444
*		- XVIDC_CSF_YCRCB_422
*
******************************************************************************/
#define XV_CscGetColorFormatIn(pCscFwReg)     ((pCscFwReg)->ColorFormatIn)

/*****************************************************************************/
/**
* This macro returns current set output color format by reading layer 2 fw
* register map.
*
* @param  pCscFwReg is pointer to csc fw register associated with csc core
*         instance
*
* @return Current set output color format
*		- XVIDC_CSF_RGB
*		- XVIDC_CSF_YCRCB_444
*		- XVIDC_CSF_YCRCB_422
*
******************************************************************************/
#define XV_CscGetColorFormatOut(pCscFwReg)    ((pCscFwReg)->ColorFormatOut)

/*****************************************************************************/
/**
* This macro returns current set input color standard by reading layer 2 fw
* register map.
*
* @param  pCscFwReg is pointer to csc fw register associated with csc core
*         instance
*
* @return Current set input color standard
*		- XVIDC_BT_2020
*		- XVIDC_BT_709
*		- XVIDC_BT_601
*
******************************************************************************/
#define XV_CscGetColorStdIn(pCscFwReg)        ((pCscFwReg)->StandardIn)

/*****************************************************************************/
/**
* This macro returns current set output color standard by reading layer 2 fw
* register map.
*
* @param  pCscFwReg is pointer to csc fw register associated with csc core
*         instance
*
* @return Current set output color standard
*		- XVIDC_BT_2020
*		- XVIDC_BT_709
*		- XVIDC_BT_601
*
*****************************************************************************/
#define XV_CscGetColorStdOut(pCscFwReg)       ((pCscFwReg)->StandardOut)

/*****************************************************************************/
/**
* This macro returns current set output range by reading layer 2 fw register
* map.
*
* @param  pCscFwReg is pointer to csc fw register associated with csc core
*         instance
*
* @return Current set output range
*		- XVIDC_CR_16_235
*		- XVIDC_CR_16_240
*		- XVIDC_CR_0_255
*
******************************************************************************/
#define XV_CscGetOutputRange(pCscFwReg)       ((pCscFwReg)->OutputRange)

/************************** Function Prototypes ******************************/
void XV_CscStart(XV_csc *InstancePtr);
void XV_CscStop(XV_csc *InstancePtr);
void XV_CscSetActiveSize(XV_csc *InstancePtr,
                         u32    width,
                         u32    height);
void XV_CscSetDemoWindow(XV_csc *InstancePtr, XVidC_VideoWindow *ActiveWindow);

void XV_CscSetColorspace(XV_csc *InstancePtr,
                         XV_csc_L2Reg  *pCscFwReg,
                         XVidC_ColorFormat cfmtIn,
                         XVidC_ColorFormat cfmtOut,
                         XVidC_ColorStd    cstdIn,
                         XVidC_ColorStd    cstdOut,
                         XVidC_ColorRange  cRangeOut
                         );

void XV_CscInitPowerOnDefault(XV_csc_L2Reg *pCscFwReg);
void XV_CscSetBrightness(XV_csc *InstancePtr,
                         XV_csc_L2Reg *pCscFwReg,
                         s32 val);
void XV_CscSetContrast(XV_csc *InstancePtr,
                        XV_csc_L2Reg *pCscFwReg,
                        s32 val);
void XV_CscSetSaturation(XV_csc *InstancePtr,
                         XV_csc_L2Reg *pCscFwReg,
                         s32 val);
void XV_CscSetRedGain(XV_csc *InstancePtr,
                      XV_csc_L2Reg *pCscFwReg,
                      s32 val);
void XV_CscSetGreenGain(XV_csc *InstancePtr,
                        XV_csc_L2Reg *pCscFwReg,
                        s32 val);
void XV_CscSetBlueGain(XV_csc *InstancePtr,
                       XV_csc_L2Reg *pCscFwReg,
                       s32 val);
void XV_CscDbgReportStatus(XV_csc *InstancePtr);

#ifdef __cplusplus
}
#endif
#endif
/** @} */
