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
* @file xv_hcresampler_l2.c
* @addtogroup v_hcresampler_v1_0
* @{
* @details
*
* The Horizontal Chroma Resampler Layer-2 Driver.
* The functions in this file provides an abstraction from the register peek/poke
* methodology by implementing most common use-case provided by the sub-core.
* See xv_hcresampler_l2.h for a detailed description of the layer-2 driver
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

/***************************** Include Files *********************************/
#include "xv_hcresampler_l2.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**************************** Local Global *******************************/

/************************** Function Prototypes ******************************/
static void hcrUpdateCoefficients(XV_hcresampler *pHCrsmplr, u32 coeff[2][4]);

/*****************************************************************************/
/**
 * This function starts the Chroma resampler core
 *
 * @param  InstancePtr is a pointer to the core instance to be worked on.
 *
 * @return None
 *
 *****************************************************************************/
void XV_HCrsmplStart(XV_hcresampler *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_hcresampler_EnableAutoRestart(InstancePtr);
  XV_hcresampler_Start(InstancePtr);
}

/*****************************************************************************/
/**
 * This function stops the Chroma resampler core
 *
 * @param  InstancePtr is a pointer to the core instance to be worked on.
 *
 * @return None
 *
 *****************************************************************************/
void XV_HCrsmplStop(XV_hcresampler *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_hcresampler_DisableAutoRestart(InstancePtr);
}

/*****************************************************************************/
/**
* This function configures the Chroma resampler active resolution
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  width is the active frame width
* @param  height is the active frame height
*
* @return None
*
******************************************************************************/
void XV_HCrsmplSetActiveSize(XV_hcresampler *InstancePtr,
                             u32             width,
                             u32             height)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_hcresampler_Set_HwReg_width(InstancePtr,  width);
  XV_hcresampler_Set_HwReg_height(InstancePtr, height);
}

/*****************************************************************************/
/**
* This function configures the Chroma resampler for the required format
* conversion
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  formatIn is the input chroma format
* @param  formatOut is required chroma format
*
* @return None
*
******************************************************************************/
void XV_HCrsmplSetFormat(XV_hcresampler   *InstancePtr,
                         XVidC_ColorFormat formatIn,
                         XVidC_ColorFormat formatOut)
{
  u32 K[2][4] = {{0}};

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);

  XV_hcresampler_Set_HwReg_input_video_format(InstancePtr,  formatIn);
  XV_hcresampler_Set_HwReg_output_video_format(InstancePtr, formatOut);

  if((formatIn == XVIDC_CSF_YCRCB_422) &&
     (formatOut == XVIDC_CSF_YCRCB_444))
  {
    K[0][0] = 0;
    K[0][1] = 0;
    K[0][2] = 4096;
    K[0][3] = 0;
    K[1][0] = 506;
    K[1][1] = 1542;
    K[1][2] = 1542;
    K[1][3] = 506;
  }
  else if((formatIn == XVIDC_CSF_YCRCB_444) &&
          (formatOut == XVIDC_CSF_YCRCB_422))

  {
    K[0][0] = 0;
    K[0][1] = 1024;
    K[0][2] = 2048;
    K[0][3] = 1024;
    K[1][0] = 0;
    K[1][1] = 0;
    K[1][2] = 0;
    K[1][3] = 0;
  }
  hcrUpdateCoefficients(InstancePtr, K);
}

/*****************************************************************************/
/**
* This function updates the core registers with computed coefficients for
* required conversion
*
* @param  pHCrsmplr is a pointer to the core instance to be worked on.
* @param  coeff is the array holding computed coefficients
*
* @return None
*
******************************************************************************/
static void hcrUpdateCoefficients(XV_hcresampler *pHCrsmplr, u32 coeff[2][4])
{
  XV_hcresampler_Set_HwReg_coefs_0_0(pHCrsmplr, coeff[0][0]);
  XV_hcresampler_Set_HwReg_coefs_0_1(pHCrsmplr, coeff[0][1]);
  XV_hcresampler_Set_HwReg_coefs_0_2(pHCrsmplr, coeff[0][2]);
  XV_hcresampler_Set_HwReg_coefs_0_3(pHCrsmplr, coeff[0][3]);
  XV_hcresampler_Set_HwReg_coefs_1_0(pHCrsmplr, coeff[1][0]);
  XV_hcresampler_Set_HwReg_coefs_1_1(pHCrsmplr, coeff[1][1]);
  XV_hcresampler_Set_HwReg_coefs_1_2(pHCrsmplr, coeff[1][2]);
  XV_hcresampler_Set_HwReg_coefs_1_3(pHCrsmplr, coeff[1][3]);
}

/*****************************************************************************/
/**
*
* This function prints Chroma Resampler status on the console
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_HCrsmplDbgReportStatus(XV_hcresampler *InstancePtr)
{
  XV_hcresampler *pHCrsmplr = InstancePtr;
  u32 done, idle, ready, ctrl;
  u32 vidfmtIn, vidfmtOut, height, width;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);

  xil_printf("\r\n\r\n----->H Chroma Resampler IP STATUS<----\r\n");

  done  = XV_hcresampler_IsDone(pHCrsmplr);
  idle  = XV_hcresampler_IsIdle(pHCrsmplr);
  ready = XV_hcresampler_IsReady(pHCrsmplr);
  ctrl  = XV_hcresampler_ReadReg(pHCrsmplr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_AP_CTRL);

  vidfmtIn  = XV_hcresampler_Get_HwReg_input_video_format(pHCrsmplr);
  vidfmtOut = XV_hcresampler_Get_HwReg_output_video_format(pHCrsmplr);
  height    = XV_hcresampler_Get_HwReg_height(pHCrsmplr);
  width     = XV_hcresampler_Get_HwReg_width(pHCrsmplr);

  xil_printf("IsDone:  %d\r\n", done);
  xil_printf("IsIdle:  %d\r\n", idle);
  xil_printf("IsReady: %d\r\n", ready);
  xil_printf("Ctrl:    0x%x\r\n\r\n", ctrl);

  xil_printf("Video Format In:  %d\r\n", vidfmtIn);
  xil_printf("Video Format Out: %d\r\n", vidfmtOut);
  xil_printf("Width:            %d\r\n", width);
  xil_printf("Height:           %d\r\n", height);
}
/** @} */
