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
* @file xv_hscaler_l2.c
*
* The Horizontal Scaler Layer-2 Driver.
* The functions in this file provides an abstraction from the register peek/poke
* methodology by implementing most common use-case provided by the sub-core.
* See xv_hscaler_l2.h for a detailed description of the layer-2 driver
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
#include "xv_hscaler_l2.h"

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
static float SincCoeffs[XV_HSCALER_MAX_H_PHASES][XV_HSCALER_MAX_H_TAPS];
static float TempCoeffs[XV_HSCALER_MAX_H_PHASES][XV_HSCALER_MAX_H_TAPS];
static float WinCoeffs[XV_HSCALER_MAX_H_PHASES][XV_HSCALER_MAX_H_TAPS];
static float NormCoeffs[XV_HSCALER_MAX_H_PHASES][XV_HSCALER_MAX_H_TAPS];

int HSC_SAMPLES_PER_CLOCK = 2;
int HSC_MAX_WIDTH = 4096;
int STEP_PRECISION_SHIFT = 16;
int HSC_PHASE_SHIFT = 6;

/************************** Function Prototypes ******************************/
static float hamming( int x, int taps);
static float sinc(float x);
static void CalculatePhases(XV_hscaler_l2 *pHscL2Data,
                            u32 WidthIn,
                            u32 WidthOut,
                            u32 PixelRate);
static void XV_HScalerGetCoeff(XV_hscaler *InstancePtr,
                               XV_hscaler_l2 *pHscL2Data,
                               u32 WidthIn,
                               u32 WidthOut,
                               u32 PixPerClk);
static void XV_HScalerSetCoeff(XV_hscaler *InstancePtr,
                               XV_hscaler_l2 *pHscL2Data);

/*****************************************************************************/
/**
* This function starts the horizontal scaler core
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_HScalerStart(XV_hscaler *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_hscaler_EnableAutoRestart(InstancePtr);
  XV_hscaler_Start(InstancePtr);
}

/*****************************************************************************/
/**
* This function stops the horizontal scaler core
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_HScalerStop(XV_hscaler *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_hscaler_DisableAutoRestart(InstancePtr);
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
* This function calculates the phases for 1 line. Same phase info is used for
* full frame
*
* @param  WidthIn is the input frame width
* @param  WidthOut is the scaled frame width
* @param  PixelRate is the number of pixels per clock being processed
*
* @return None
*
******************************************************************************/
static void CalculatePhases(XV_hscaler_l2 *pHscL2Data,
                            u32 WidthIn,
                            u32 WidthOut,
                            u32 PixelRate)
{
    int loopWidth;
    loopWidth = ((WidthIn > WidthOut) ? WidthIn +(HSC_SAMPLES_PER_CLOCK-1) : WidthOut  +(HSC_SAMPLES_PER_CLOCK-1))/HSC_SAMPLES_PER_CLOCK;

    int x, s;
    int offset = 0;
    int xWritePos = 0;
    int OutputWriteEn;
    int GetNewPix;
    int PhaseH;
    int arrayIdx;
    int xReadPos = 0;
    int nrRds = 0;
    int nrRdsClck = 0;

    arrayIdx = 0;
    for (x=0; x<loopWidth; x++)
    {
        pHscL2Data->phasesH[x] = 0;
        nrRdsClck = 0;
        for (s=0; s<HSC_SAMPLES_PER_CLOCK; s++)
        {
            PhaseH = (offset>>(STEP_PRECISION_SHIFT-HSC_PHASE_SHIFT)) & (XV_HSCALER_MAX_H_PHASES-1);//(HSC_PHASES-1);
            GetNewPix = 0;
            OutputWriteEn = 0;
            if ((offset >> STEP_PRECISION_SHIFT) != 0)
            {
                // read a new input sample
                GetNewPix = 1;
                offset = offset - (1<<STEP_PRECISION_SHIFT);
                OutputWriteEn = 0;
                arrayIdx++;
                xReadPos++;
            }

            if (((offset >> STEP_PRECISION_SHIFT) == 0) && (xWritePos< WidthOut))
            {
                // produce a new output sample
                offset += PixelRate;
                OutputWriteEn = 1;
                xWritePos++;
            }
            //printf("x %5d, offset %5d, phase %5d, arrayIdx %5d, readpos %5d writepos %5d  rden %3d wren %3d\n", (int)x*HSC_SAMPLES_PER_CLOCK+s, offset, (int)PhaseH, (int)arrayIdx, (int)xReadPos, xWritePos, GetNewPix, OutputWriteEn);
            pHscL2Data->phasesH[x] = pHscL2Data->phasesH[x] | (PhaseH << (s*9));
            pHscL2Data->phasesH[x] = pHscL2Data->phasesH[x] | (arrayIdx << (6 + (s*9)));
            pHscL2Data->phasesH[x] = pHscL2Data->phasesH[x] | (OutputWriteEn << (8 + (s*9)));

            if (GetNewPix) nrRdsClck++;
        }
        if (arrayIdx>=HSC_SAMPLES_PER_CLOCK) arrayIdx &= (HSC_SAMPLES_PER_CLOCK-1);

        //printf("%d nrRds per clock %d left hanging\n", nrRdsClck, nrRds);
        nrRds += nrRdsClck;
        if (nrRds>=HSC_SAMPLES_PER_CLOCK)
        {
            nrRds -= HSC_SAMPLES_PER_CLOCK;
            //printf("getting %d new samples\n", HSC_SAMPLES_PER_CLOCK);
        }
    }
}

/*****************************************************************************/
/**
* This function loads user defined filter coefficients in the scaler coefficient
* storage
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  pHscL2Data is a pointer to the core instance layer 2 data.
* @param  HCoeff is the user defined filter coefficients
******************************************************************************/
void XV_HscalerLoadUsrCoeffients(XV_hscaler *InstancePtr,
                                 XV_hscaler_l2 *pHscL2Data,
                                 const short HCoeff[XV_HSCALER_MAX_H_PHASES][XV_HSCALER_MAX_H_TAPS])
{
  int i,j,k, pad, offset;
  int num_phases = XV_HSCALER_MAX_H_PHASES;
  int num_taps   = pHscL2Data->EffectiveTaps;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(pHscL2Data != NULL);
  Xil_AssertVoid((pHscL2Data->EffectiveTaps > 0) &&
                 (pHscL2Data->EffectiveTaps <= XV_HSCALER_MAX_H_TAPS));

  //determine if coefficient needs padding (effective vs. max taps)
  pad = XV_HSCALER_MAX_H_TAPS - num_taps;
  offset = ((pad) ? (pad>>1) : 0);

  //Load User defined coefficients into scaler coefficient table
  for (i = 0; i < num_phases; i++)
  {
    for (k=0,j=offset; j<num_taps; ++j,++k)
    {
      pHscL2Data->coeff[i][j] = HCoeff[i][k];
    }
  }

  if(pad) //effective taps < max_taps
  {
    for (i = 0; i < num_phases; i++)
    {
      //pad left
      for (j = 0; j < offset; j++)
      {
        pHscL2Data->coeff[i][j] = 0;
      }
        //pad right
      for (j = (num_taps+offset); j < XV_HSCALER_MAX_H_TAPS; j++)
      {
        pHscL2Data->coeff[i][j] = 0;
      }
    }
  }
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
static void XV_HScalerGetCoeff(XV_hscaler *InstancePtr,
                               XV_hscaler_l2 *pHscL2Data,
                               u32 WidthIn,
                               u32 WidthOut,
                               u32 PixPerClk)
{
  int num_phases = XV_HSCALER_MAX_H_PHASES;
  int num_taps   = pHscL2Data->EffectiveTaps;
  int center_tap = num_taps/2;
  int i,j, pad, offset;
  float x, fc;
  float sum[XV_HSCALER_MAX_H_PHASES];
  float cos_win[XV_HSCALER_MAX_H_TAPS];

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid((pHscL2Data->EffectiveTaps > 0) &&
                  (pHscL2Data->EffectiveTaps <= XV_HSCALER_MAX_H_TAPS));

  if(WidthIn < WidthOut)
  {
    fc = (float)WidthIn/(float)WidthOut;
  }
  else
  {
    fc = (float)WidthOut/(float)WidthIn;
  }

  //determine if coefficient needs padding (effective vs. max taps)
  pad = XV_HSCALER_MAX_H_TAPS - num_taps;
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

  switch(pHscL2Data->FilterSel)
  {
    case XV_HFILT_LANCZOS:
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

    case XV_HFILT_WINDOWED_SINC:
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
    for (j = offset; j < num_taps; j++)
    {
      NormCoeffs[i][j] = WinCoeffs[i][j]/sum[i];
      pHscL2Data->coeff[i][j] = (short) ((NormCoeffs[i][j] * COEFF_QUANT) + 0.5);
    }
  }

  if(pad) //effective taps < max_taps
  {
    for (i = 0; i < num_phases; i++)
    {
        //pad left
      for (j = 0; j < offset; j++)
      {
        pHscL2Data->coeff[i][j] = 0;
      }
        //pad right
      for (j = (num_taps+offset); j < XV_HSCALER_MAX_H_TAPS; j++)
      {
        pHscL2Data->coeff[i][j] = 0;
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
* @param  HCoeff is the array that holds computed coefficients
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
static void XV_HScalerSetCoeff(XV_hscaler *InstancePtr,
                               XV_hscaler_l2 *pHscL2Data)
{
  int num_phases = XV_HSCALER_MAX_H_PHASES;
  int num_taps   = XV_HSCALER_MAX_H_TAPS/2;
  int val,i,j;
  u32 baseAddr;

  baseAddr = XV_hscaler_Get_HwReg_hfltCoeff_BaseAddress(InstancePtr);
  for (i = 0; i < num_phases; i++)
  {
    for(j=0; j< num_taps; j++)
    {
       val = (pHscL2Data->coeff[i][(j*2)+1] << 16) | (pHscL2Data->coeff[i][j*2] & XMASK_LOW_16BITS);
       Xil_Out32(baseAddr+((i*num_taps+j)*4), val);
    }
  }

  //program phases
  baseAddr = XV_hscaler_Get_HwReg_phasesH_V_BaseAddress(InstancePtr);
  for (i = 0; i < (HSC_MAX_WIDTH/HSC_SAMPLES_PER_CLOCK); i++)
  {
     Xil_Out32(baseAddr+(i*4), pHscL2Data->phasesH[i]);
  }
}

/*****************************************************************************/
/**
* This function configures the scaler core registers with the specified
* configuration parameters of the axi stream
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  HeightIn is the input stream height
* @param  WidthIn is the input stream width
* @param  WidthOut is the output stream width
* @param  cformat is the input stream color format
*
* @return None
*
******************************************************************************/
void XV_HScalerSetup(XV_hscaler  *InstancePtr,
                     XV_hscaler_l2 *pHscL2Data,
                     u32 HeightIn,
                     u32 WidthIn,
                     u32 WidthOut,
                     u32 PixPerClk,
                     u32 cformat)
{
  u32 PixelRate;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(pHscL2Data != NULL);

  PixelRate = (u32) ((float)((WidthIn * STEP_PRECISION) + (WidthOut/2))/(float)WidthOut);

  /* Compute Phase for 1 line */
  CalculatePhases(pHscL2Data, WidthIn, WidthOut, PixelRate);

  if(pHscL2Data->ScalerType == XV_HSCALER_POLYPHASE)
  {
    if(!pHscL2Data->UseExtCoeff)  //No predefined coefficients
    {
      /* Generate coefficients for horizontal scaling ratio */
      XV_HScalerGetCoeff(InstancePtr,
                         pHscL2Data,
                         WidthIn,
                         WidthOut,
                         PixPerClk);
    }

    /* Program generated coefficients into the IP register bank */
    XV_HScalerSetCoeff(InstancePtr, pHscL2Data);
  }

  XV_hscaler_Set_HwReg_Height(InstancePtr,     HeightIn);
  XV_hscaler_Set_HwReg_WidthIn(InstancePtr,    WidthIn);
  XV_hscaler_Set_HwReg_WidthOut(InstancePtr,   WidthOut);
  XV_hscaler_Set_HwReg_ColorMode(InstancePtr,  cformat);
  XV_hscaler_Set_HwReg_PixelRate(InstancePtr,  PixelRate);
}


/*****************************************************************************/
/**
*
* This function prints H Scaler status on the console
*
* @param	InstancePtr is the instance pointer to the core instance.
*
* @return	None
*
******************************************************************************/
void XV_HScalerDbgReportStatus(XV_hscaler *InstancePtr)
{
  XV_hscaler *pHsc = InstancePtr;
  u32 done, idle, ready, ctrl;
  u32 widthin, widthout, heightin, pixrate, cformat;
  u32 type = 3;  //hard-coded to polyphase for now
  u32 baseAddr, taps, phases;
  int val,i,j;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);

  xil_printf("\r\n\r\n----->H SCALER IP STATUS<----\r\n");
  done  = XV_hscaler_IsDone(pHsc);
  idle  = XV_hscaler_IsIdle(pHsc);
  ready = XV_hscaler_IsReady(pHsc);
  ctrl =  XV_hscaler_ReadReg(pHsc->Ctrl_BaseAddress, XV_HSCALER_CTRL_ADDR_AP_CTRL);

  heightin = XV_hscaler_Get_HwReg_Height(pHsc);
  widthin  = XV_hscaler_Get_HwReg_WidthIn(pHsc);
  widthout = XV_hscaler_Get_HwReg_WidthOut(pHsc);
// type     = XV_hscaler_Get_Hwreg_scaletype_v(pHsc);
  cformat  = XV_hscaler_Get_HwReg_ColorMode(pHsc);
  pixrate  = XV_hscaler_Get_HwReg_PixelRate(pHsc);

  taps   = XV_HSCALER_MAX_H_TAPS/2;
  phases = XV_HSCALER_MAX_H_PHASES;
  xil_printf("IsDone:  %d\r\n", done);
  xil_printf("IsIdle:  %d\r\n", idle);
  xil_printf("IsReady: %d\r\n", ready);
  xil_printf("Ctrl:    0x%x\r\n\r\n", ctrl);

  xil_printf("Input Height:    %d\r\n",heightin);
  xil_printf("Input Width:     %d\r\n",widthin);
  xil_printf("Output Width:    %d\r\n",widthout);
// xil_printf("Scaler Type:     %d\r\n",type);
  xil_printf("Color Format:    %d\r\n",cformat);
  xil_printf("Pixel Rate:      %d\r\n",pixrate);
  xil_printf("Num Phases:      %d\r\n",phases);
  xil_printf("Num Taps:        %d\r\n",taps*2);

  if(type == 3)
  {
    short lsb, msb;

    xil_printf("\r\nCoefficients:");

    baseAddr = XV_hscaler_Get_HwReg_hfltCoeff_BaseAddress(pHsc);
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
