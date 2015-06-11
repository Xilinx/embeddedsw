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
* @file xv_vscaler_l2.c
* @addtogroup v_vscaler_v1_0
* @{
* @details
*
* The Vertical Scaler Layer-2 Driver.
* The functions in this file provides an abstraction from the register peek/poke
* methodology by implementing most common use-case provided by the sub-core.
* See xv_vscaler_l2.h for a detailed description of the layer-2 driver
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

/***************************** Include Files *********************************/
#include <math.h>
#include "xv_vscaler_l2.h"

/************************** Constant Definitions *****************************/
#define PI                     (3.14159265358979)
#define STEP_PRECISION         (65536)  // 2^16
#define COEFF_PRECISION        (4096)   // 2^12
#define COEFF_QUANT            (4096)

/* Mask definitions for Low and high 16 bits in a 32 bit number */
#define XMASK_LOW_16BITS       (0x0000FFFF)
#define XMASK_HIGH_16BITS      (0xFFFF0000)

/**************************** Type Definitions *******************************/

/**************************** Local Global *******************************/
static float SincCoeffs[XV_VSCALER_MAX_V_PHASES][XV_VSCALER_MAX_V_TAPS];
static float TempCoeffs[XV_VSCALER_MAX_V_PHASES][XV_VSCALER_MAX_V_TAPS];
static float WinCoeffs[XV_VSCALER_MAX_V_PHASES][XV_VSCALER_MAX_V_TAPS];
static float NormCoeffs[XV_VSCALER_MAX_V_PHASES][XV_VSCALER_MAX_V_TAPS];

/************************** Function Prototypes ******************************/
static float hamming( int x, int taps);
static float sinc(float x);
static void XV_VScalerGetCoeff(XV_vscaler *pVsc,
                               XV_vscaler_l2 *pVscL2Data,
                               u32 HeightIn,
                               u32 HeightOut);
static void XV_VScalerSetCoeff(XV_vscaler *pVsc,
                               XV_vscaler_l2 *pVscL2Data);

/*****************************************************************************/
/**
* This function starts the vertical scaler core
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_VScalerStart(XV_vscaler *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_vscaler_EnableAutoRestart(InstancePtr);
  XV_vscaler_Start(InstancePtr);
}

/*****************************************************************************/
/**
* This function stops the vertical scaler core
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_VScalerStop(XV_vscaler *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_vscaler_DisableAutoRestart(InstancePtr);
}

/*****************************************************************************/
/**
* This function applies the hamming filter on specified pixel position
*
* @param  x is the pixel coordinate in horizontal direction
* @param  taps is the number of taps available to the scaler
*
* @return hamming filter result
*
******************************************************************************/
static float hamming( int x, int taps)
{
    //0.54 + 0.46 * cos(pi * x / filter_size); 0.54 - 0.46 * cos(2*pi * x / filter_size)
    return (float) (0.54 + (0.46*cos((PI*x)/(taps+1))));
}

/*****************************************************************************/
/**
* This function applies the SIN function to specified pixel position
*
* @param  x is the pixel coordinate in horizontal direction
*
* @return Sine function result
*
******************************************************************************/
static float sinc(float x)
{
    if (x==0)
       return 1;

    return (float) sin(x*PI)/(float)(x*PI);
}

/*****************************************************************************/
/**
* This function loads user defined filter coefficients in the scaler coefficient
* storage
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  pVscL2Data is a pointer to the core instance layer 2 data.
* @param  VCoeff is the user defined filter coefficients
******************************************************************************/
void XV_VscalerLoadUsrCoeffients(XV_vscaler *InstancePtr,
                                 XV_vscaler_l2 *pVscL2Data,
                                 u16 num_phases,
                                 u16 num_taps,
                                 const short *Coeff)
{
  int i,j, pad, offset;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(pVscL2Data != NULL);
  Xil_AssertVoid(num_taps <= InstancePtr->Config.NumTaps);
  Xil_AssertVoid(num_phases <= (1<<InstancePtr->Config.PhaseShift));

  //determine if coefficient needs padding (effective vs. max taps)
  pad = XV_VSCALER_MAX_V_TAPS - InstancePtr->Config.NumTaps;
  offset = ((pad) ? (pad>>1) : 0);

  //Load User defined coefficients into scaler coefficient table
  for (i = 0; i < num_phases; i++)
  {
    for (j=0; j<num_taps; ++j)
    {
      pVscL2Data->coeff[i][j+offset] = Coeff[i*num_taps+j];
    }
  }

  if(pad) //effective taps < max_taps
  {
    for (i = 0; i < num_phases; i++)
    {
      //pad left
      for (j = 0; j < offset; j++)
      {
        pVscL2Data->coeff[i][j] = 0;
      }
        //pad right
      for (j = (num_taps+offset); j < XV_VSCALER_MAX_V_TAPS; j++)
      {
        pVscL2Data->coeff[i][j] = 0;
      }
    }
  }

  /* Enable use of external coefficients */
  pVscL2Data->UseExtCoeff = TRUE;
}

/*****************************************************************************/
/**
* This function computes the filter coefficients based on scaling ratio and
* stores them into the layer 2 data storage
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  pVscL2Data is a pointer to the core instance layer 2 data.
* @param  HeightIn is the input frame height
* @param  HeightOut is the scaled frame height
*
* @return None
*
******************************************************************************/
static void XV_VScalerGetCoeff(XV_vscaler    *pVsc,
                               XV_vscaler_l2 *pVscL2Data,
                               u32            HeightIn,
                               u32            HeightOut)
{
  int num_phases = (1<<pVsc->Config.PhaseShift);
  int num_taps   = pVsc->Config.NumTaps;
  int center_tap = num_taps/2;
  int i,j, pad, offset;
  float x, fc;
  float sum[XV_VSCALER_MAX_V_PHASES];
  float cos_win[XV_VSCALER_MAX_V_TAPS];

  if(HeightIn < HeightOut)
  {
    fc = (float)HeightIn/(float)HeightOut;
  }
  else
  {
    fc = (float)HeightOut/(float)HeightIn;
  }

  //determine if coefficient needs padding (effective vs. max taps)
  pad = XV_VSCALER_MAX_V_TAPS - num_taps;
  offset = ((pad) ? (pad>>1) : 0);

  for(i=0; i<num_phases; ++i)
  {
    for(j=0; j<num_taps; ++j)
    {
      x = ((float) (j - center_tap)) + (((float)i)/(float)num_phases);
      TempCoeffs[i][j] = x;
      SincCoeffs[i][j] = sinc(fc*x);
    }
  }

  switch(pVscL2Data->FilterSel)
  {
    case XV_VFILT_LANCZOS:
      //Window is a sinc function instead of cosine function
      // if using lanczos2 or lanczos3 kernel
      // lanczos(x) = sinc(x) * sinc(x / filter_size);

      for (i = 0; i < num_phases; i++)
      {
          for (j = 0; j < num_taps; j++)
          {
              x = TempCoeffs[i][j];
              WinCoeffs[i][j] = SincCoeffs[i][j] * sinc((fc*x)/num_taps);
          }
      }
      break;

    case XV_VFILT_WINDOWED_SINC:
      for (j = 1; j <= num_taps; j++)
      {
        cos_win[j-1] = hamming(j, num_taps);
      }

      for (i = 0; i < num_phases; i++)
      {
          for (j = 0; j < num_taps; j++)
          {
              WinCoeffs[i][j] = SincCoeffs[i][j] * cos_win[j];
          }
      }
      break;
  }

  // normalize to unity and quantize
  for (i = 0; i < num_phases; i++)
  {
    sum[i] = 0;
    for (j = 0; j < num_taps; j++)
    {
      sum[i] += WinCoeffs[i][j];
    }
  }

  for (i = 0; i < num_phases; i++)
  {
    for (j = 0; j < num_taps; j++)
    {
      NormCoeffs[i][j] = WinCoeffs[i][j]/sum[i];
      pVscL2Data->coeff[i][j+offset] = (short) ((NormCoeffs[i][j] * COEFF_QUANT) + 0.5);
    }
  }

  if(pad) //effective taps < max_taps
  {
    for (i = 0; i < num_phases; i++)
    {
      //pad left
      for (j = 0; j < offset; j++)
      {
        pVscL2Data->coeff[i][j] = 0;
      }
        //pad right
      for (j = (num_taps+offset); j < XV_VSCALER_MAX_V_TAPS; j++)
      {
        pVscL2Data->coeff[i][j] = 0;
      }
    }
  }
}


/*****************************************************************************/
/**
* This function programs the computed filter coefficients and phase data into
* core registers
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  VCoeff is the array that holds computed coefficients
*
* @return None
*
* @Note  This version of driver does not make use of computed coefficients.
*        Pre-computed coefficients are stored in a local table which are used
*        to overwrite any computed coefficients before being programmed into
*        the core registers. Control flow still computes the coefficients to
*        maintain the sw latency for driver version which would eventually use
*        computed coefficients
******************************************************************************/
static void XV_VScalerSetCoeff(XV_vscaler *pVsc,
                               XV_vscaler_l2 *pVscL2Data)
{
  int num_phases = 1<<pVsc->Config.PhaseShift;
  int num_taps   = pVsc->Config.NumTaps/2;
  int val,i,j,offset,rdIndx;
  u32 baseAddr;

  offset = (XV_VSCALER_MAX_V_TAPS - pVsc->Config.NumTaps)/2;
  baseAddr = XV_vscaler_Get_HwReg_vfltCoeff_BaseAddress(pVsc);
  for (i=0; i < num_phases; i++)
  {
    for(j=0; j < num_taps; j++)
    {
       rdIndx = j*2+offset;
       val = (pVscL2Data->coeff[i][rdIndx+1] << 16) | (pVscL2Data->coeff[i][rdIndx] & XMASK_LOW_16BITS);
       Xil_Out32(baseAddr+((i*num_taps+j)*4), val);
    }
  }
}

/*****************************************************************************/
/**
* This function configures the scaler core registers with the specified
* configuration parameters of the axi stream
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  WidthIn is the input stream width
* @param  HeightIn is the input stream height
* @param  HeightOut is the output stream height
*
* @return None
*
******************************************************************************/
void XV_VScalerSetup(XV_vscaler  *InstancePtr,
                     XV_vscaler_l2 *pVscL2Data,
                     u32         WidthIn,
                     u32         HeightIn,
                     u32         HeightOut)
{
  u32 LineRate;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(pVscL2Data != NULL);

  if(InstancePtr->Config.ScalerType == XV_VSCALER_POLYPHASE)
  {
    if(!pVscL2Data->UseExtCoeff) //No predefined coefficients
    {
      /* If user has not selected any filter set default */
      if(pVscL2Data->FilterSel == 0)
      {
        XV_VScalerSetFilterType(pVscL2Data, XV_VFILT_LANCZOS);
      }

      /* Generate coefficients for vertical scaling ratio */
      XV_VScalerGetCoeff(InstancePtr,
                         pVscL2Data,
                         HeightIn,
                         HeightOut);
    }

    /* Program coefficients into the IP register bank */
    XV_VScalerSetCoeff(InstancePtr, pVscL2Data);
  }

  LineRate = (u32) ((float)((HeightIn * STEP_PRECISION) + (HeightOut/2))/(float)HeightOut);

  XV_vscaler_Set_HwReg_HeightIn(InstancePtr,   HeightIn);
  XV_vscaler_Set_HwReg_Width(InstancePtr,      WidthIn);
  XV_vscaler_Set_HwReg_HeightOut(InstancePtr,  HeightOut);
  XV_vscaler_Set_HwReg_LineRate(InstancePtr,   LineRate);
}

/*****************************************************************************/
/**
*
* This function prints V Scaler status on the console
*
* @param	InstancePtr is the instance pointer to the core instance.
*
* @return	None
*
******************************************************************************/
void XV_VScalerDbgReportStatus(XV_vscaler *InstancePtr)
{
  XV_vscaler *pVsc = InstancePtr;
  u32 done, idle, ready, ctrl;
  u32 widthin, heightin, heightout, linerate;
  u32 baseAddr, taps, phases;
  int val,i,j;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);

  xil_printf("\r\n\r\n----->V SCALER IP STATUS<----\r\n");
  done  = XV_vscaler_IsDone(pVsc);
  idle  = XV_vscaler_IsIdle(pVsc);
  ready = XV_vscaler_IsReady(pVsc);
  ctrl =  XV_vscaler_ReadReg(pVsc->Ctrl_BaseAddress, XV_VSCALER_CTRL_ADDR_AP_CTRL);


  heightin  = XV_vscaler_Get_HwReg_HeightIn(pVsc);
  widthin   = XV_vscaler_Get_HwReg_Width(pVsc);
  heightout = XV_vscaler_Get_HwReg_HeightOut(pVsc);
  linerate  = XV_vscaler_Get_HwReg_LineRate(pVsc);

  taps   = pVsc->Config.NumTaps/2;
  phases = (1<<pVsc->Config.PhaseShift);

  xil_printf("IsDone:  %d\r\n", done);
  xil_printf("IsIdle:  %d\r\n", idle);
  xil_printf("IsReady: %d\r\n", ready);
  xil_printf("Ctrl:    0x%x\r\n\r\n", ctrl);

  xil_printf("Scaler Type:     %d\r\n",pVsc->Config.ScalerType);
  xil_printf("Input Width:     %d\r\n",widthin);
  xil_printf("Input Height:    %d\r\n",heightin);
  xil_printf("Output Height:   %d\r\n",heightout);
  xil_printf("Line Rate:       %d\r\n",linerate);
  xil_printf("Num Phases:      %d\r\n",phases);
  xil_printf("Num Taps:        %d\r\n",taps*2);

  if(pVsc->Config.ScalerType == XV_VSCALER_POLYPHASE)
  {
    short lsb, msb;

    xil_printf("\r\nCoefficients:");

    baseAddr = XV_vscaler_Get_HwReg_vfltCoeff_BaseAddress(pVsc);
    for(i = 0; i < phases; i++)
    {
      xil_printf("\r\nPhase %2d: ",i);
      for(j=0; j< taps; j++)
      {
        val = Xil_In32(baseAddr+((i*taps+j)*4));

        //coefficients are 12-bits
        lsb = (short)(val & XMASK_LOW_16BITS);
        msb = (short)((val & XMASK_HIGH_16BITS)>>16);

        xil_printf("%5d %5d ", lsb, msb);
      }
    }
  }
}
/** @} */
