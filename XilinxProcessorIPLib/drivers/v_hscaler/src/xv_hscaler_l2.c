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
* @addtogroup v_hscaler_v1_0
* @{
* @details
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
* 1.00  rco   07/21/15   Initial Release

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xv_hscaler_l2.h"

/************************** Constant Definitions *****************************/
#define STEP_PRECISION         (65536)  // 2^16

/* Mask definitions for Low and high 16 bits in a 32 bit number */
#define XHSC_MASK_LOW_16BITS       (0x0000FFFF)
#define XHSC_MASK_HIGH_16BITS      (0xFFFF0000)


/**************************** Type Definitions *******************************/

/**************************** Local Global *******************************/
const int STEP_PRECISION_SHIFT = 16;
const u64 XHSC_MASK_LOW_32BITS = ((u64)1<<32)-1;

const short XV_hscaler_fixedcoeff_taps6[XV_HSCALER_MAX_H_PHASES][XV_HSCALER_TAPS_6];
const short XV_hscaler_fixedcoeff_taps8[XV_HSCALER_MAX_H_PHASES][XV_HSCALER_TAPS_8];
const short XV_hscaler_fixedcoeff_taps10[XV_HSCALER_MAX_H_PHASES][XV_HSCALER_TAPS_10];
const short XV_hscaler_fixedcoeff_taps12[XV_HSCALER_MAX_H_PHASES][XV_HSCALER_TAPS_12];

/************************** Function Prototypes ******************************/
static void XV_HScalerSelectCoeff(XV_hscaler *InstancePtr,
		                          XV_hscaler_l2 *HscL2DataPtr,
		                          u32 WidthIn,
		                          u32 WidthOut);
static void CalculatePhases(XV_hscaler *pHsc,
                            XV_hscaler_l2 *HscL2DataPtr,
                            u32 WidthIn,
                            u32 WidthOut,
                            u32 PixelRate);

static void XV_HScalerSetCoeff(XV_hscaler *pHsc,
                               XV_hscaler_l2 *HscL2DataPtr);

static void XV_HScalerSetPhase(XV_hscaler *pHsc,
                               XV_hscaler_l2 *HscL2DataPtr);

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
* This function determines the internal coeffiecient table to be used based on
* scaling ratio and loads the filter coefficients in the scaler coefficient
* storage.
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  HscL2DataPtr is a pointer to the core instance layer 2 data.
* @param  WidthIn is the input stream width
* @param  Widthout is the output stream width
*
* @return None
*
******************************************************************************/
static void XV_HScalerSelectCoeff(XV_hscaler *InstancePtr,
		                          XV_hscaler_l2 *HscL2DataPtr,
		                          u32 WidthIn,
		                          u32 WidthOut)
{
  const short *coeff;
  u16 numTaps, numPhases;
  u16 ScalingRatio;
  u16 IsScaleDown;

  /*
   * validate input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(HscL2DataPtr != NULL);

  numPhases = (1<<InstancePtr->Config.PhaseShift);

  IsScaleDown = (WidthOut < WidthIn);

  /* Scale Down Mode will use dynamic filter selection logic
   * Scale Up Mode (including 1:1) will always use 6 tap filter
   */
  if(IsScaleDown)
  {
    ScalingRatio = ((WidthIn * 10)/WidthOut);

    switch(InstancePtr->Config.NumTaps)
    {
      case XV_HSCALER_TAPS_6:
	       coeff = &XV_hscaler_fixedcoeff_taps6[0][0];
		   numTaps = XV_HSCALER_TAPS_6;
		   break;

      case XV_HSCALER_TAPS_8:
	   if(ScalingRatio > 15) //>1.5
	   {
		 coeff = &XV_hscaler_fixedcoeff_taps8[0][0];
		 numTaps = XV_HSCALER_TAPS_8;
	   }
	   else //<=1.5
	   {
	         coeff = &XV_hscaler_fixedcoeff_taps6[0][0];
		 numTaps = XV_HSCALER_TAPS_6;
	   }
		   break;

      case XV_HSCALER_TAPS_10:
	       if(ScalingRatio > 25) //2.5
	       {
		     coeff = &XV_hscaler_fixedcoeff_taps10[0][0];
		 numTaps = XV_HSCALER_TAPS_10;
	       }
	       else if(ScalingRatio > 15) // 1.6 < ratio <= 2.5
	       {
	         coeff = &XV_hscaler_fixedcoeff_taps8[0][0];
		 numTaps = XV_HSCALER_TAPS_8;
	       }
	       else // <= 1.5
	       {
	         coeff = &XV_hscaler_fixedcoeff_taps6[0][0];
		 numTaps = XV_HSCALER_TAPS_6;
	       }
           break;

      case XV_HSCALER_TAPS_12:
	       if(ScalingRatio > 35) //> 3.5
	       {
		     coeff = &XV_hscaler_fixedcoeff_taps12[0][0];
		 numTaps = XV_HSCALER_TAPS_12;
	       }
	       else if(ScalingRatio > 25) //2.6 < Ratio <= 3.5
	       {
	         coeff = &XV_hscaler_fixedcoeff_taps10[0][0];
		 numTaps = XV_HSCALER_TAPS_10;
	       }
	       else if(ScalingRatio > 15) //1.6 < Ratio <= 2.5
	       {
	         coeff = &XV_hscaler_fixedcoeff_taps8[0][0];
		 numTaps = XV_HSCALER_TAPS_8;
	       }
	       else // <=1.5
	       {
	         coeff = &XV_hscaler_fixedcoeff_taps6[0][0];
		 numTaps = XV_HSCALER_TAPS_6;
	       }
		   break;

	  default:
		  xil_printf("ERR: H-Scaler %d Taps Not Supported",numTaps);
		  return;
	}
  }
  else //Scale Up
  {
    coeff = &XV_hscaler_fixedcoeff_taps6[0][0];
	numTaps = XV_HSCALER_TAPS_6;
  }

  XV_HScalerLoadExtCoeff(InstancePtr,
			             HscL2DataPtr,
		                 numPhases,
		                 numTaps,
		                 coeff);

  /* Disable use of external coefficients */
  HscL2DataPtr->UseExtCoeff = FALSE;
}

/*****************************************************************************/
/**
* This function loads user defined filter coefficients in the scaler coefficient
* storage
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  HscL2DataPtr is a pointer to the core instance layer 2 data.
* @param  num_phases is the number of phases in coefficient table
* @param  num_taps is the number of taps in coefficient table
* @param  Coeff is a pointer to user defined filter coefficients table
*
* @return None
*
******************************************************************************/
void XV_HScalerLoadExtCoeff(XV_hscaler *InstancePtr,
                            XV_hscaler_l2 *HscL2DataPtr,
                            u16 num_phases,
                            u16 num_taps,
                            const short *Coeff)
{
  int i,j, pad, offset;

  /*
   * validate input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(HscL2DataPtr != NULL);
  Xil_AssertVoid(num_taps <= InstancePtr->Config.NumTaps);
  Xil_AssertVoid(num_phases == (1<<InstancePtr->Config.PhaseShift));
  Xil_AssertVoid(Coeff != NULL);

  switch(num_taps)
  {
    case XV_HSCALER_TAPS_6:
    case XV_HSCALER_TAPS_8:
    case XV_HSCALER_TAPS_10:
    case XV_HSCALER_TAPS_12:
	break;

    default:
	xil_printf("\r\nERR: H Scaler %d TAPS not supported. (Select from 8/10/12/16)\r\n");
	return;
  }

  //determine if coefficient needs padding (effective vs. max taps)
  pad = XV_HSCALER_MAX_H_TAPS - num_taps;
  offset = ((pad) ? (pad>>1) : 0);

  //Load User defined coefficients into scaler coefficient table
  for (i = 0; i < num_phases; i++)
  {
    for (j=0; j<num_taps; ++j)
    {
      HscL2DataPtr->coeff[i][j+offset] = Coeff[i*num_taps+j];
    }
  }

  if(pad) //effective taps < max_taps
  {
    for (i = 0; i < num_phases; i++)
    {
      //pad left
      for (j = 0; j < offset; j++)
      {
        HscL2DataPtr->coeff[i][j] = 0;
      }
        //pad right
      for (j = (num_taps+offset); j < XV_HSCALER_MAX_H_TAPS; j++)
      {
        HscL2DataPtr->coeff[i][j] = 0;
      }
    }
  }

  /* Enable use of external coefficients */
  HscL2DataPtr->UseExtCoeff = TRUE;
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
static void CalculatePhases(XV_hscaler *pHsc,
                            XV_hscaler_l2 *HscL2DataPtr,
                            u32 WidthIn,
                            u32 WidthOut,
                            u32 PixelRate)
{
    int loopWidth;
    int x,s;
    int offset = 0;
    int xWritePos = 0;
    u64 OutputWriteEn;
    int GetNewPix;
    u64 PhaseH;
    u64 arrayIdx;
    int xReadPos = 0;
    int nrRds = 0;
    int nrRdsClck = 0;
    int MaxPhases = (1<<pHsc->Config.PhaseShift);
    loopWidth = ((WidthIn > WidthOut) ? WidthIn + (pHsc->Config.PixPerClk-1)
                                      : WidthOut  +(pHsc->Config.PixPerClk-1))/pHsc->Config.PixPerClk;


    arrayIdx = 0;
    for (x=0; x<loopWidth; x++)
    {
        HscL2DataPtr->phasesH[x] = 0;
        nrRdsClck = 0;
        for (s=0; s<pHsc->Config.PixPerClk; s++)
        {
            PhaseH = (offset>>(STEP_PRECISION_SHIFT-pHsc->Config.PhaseShift)) & (MaxPhases-1);//(HSC_PHASES-1);
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

            if(pHsc->Config.PixPerClk == XVIDC_PPC_4)
            {
              HscL2DataPtr->phasesH[x] = HscL2DataPtr->phasesH[x] | (PhaseH << (s*10));
              HscL2DataPtr->phasesH[x] = HscL2DataPtr->phasesH[x] | (arrayIdx << 6 + (s*10));
              HscL2DataPtr->phasesH[x] = HscL2DataPtr->phasesH[x] | (OutputWriteEn << 9 + (s*10));
            }
            else
            {
              HscL2DataPtr->phasesH[x] = HscL2DataPtr->phasesH[x] | (PhaseH << (s*9));
              HscL2DataPtr->phasesH[x] = HscL2DataPtr->phasesH[x] | (arrayIdx << 6 + (s*9));
              HscL2DataPtr->phasesH[x] = HscL2DataPtr->phasesH[x] | (OutputWriteEn << 8 + (s*9));
            }

            if (GetNewPix) nrRdsClck++;
        }
        if (arrayIdx>=pHsc->Config.PixPerClk)
            arrayIdx &= (pHsc->Config.PixPerClk-1);

        nrRds += nrRdsClck;
        if (nrRds >= pHsc->Config.PixPerClk)
            nrRds -= pHsc->Config.PixPerClk;
    }
}

/*****************************************************************************/
/**
* This function programs the phase data into core registers
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  HscL2DataPtr is a pointer to the core instance layer 2 data.
*
* @return None
*
* @Note  This version of driver does not make use of computed coefficients.
*        User must load the coefficients, using the provided API, before
*        scaler can be used
******************************************************************************/
static void XV_HScalerSetPhase(XV_hscaler *pHsc,
                               XV_hscaler_l2 *HscL2DataPtr)
{
  u32 baseAddr, loopWidth;

  //program phases
  baseAddr = XV_hscaler_Get_HwReg_phasesH_V_BaseAddress(pHsc);
  loopWidth = pHsc->Config.MaxWidth/pHsc->Config.PixPerClk;
  switch(pHsc->Config.PixPerClk)
  {
    case XVIDC_PPC_1:
            {
              u32 val, lsb, msb, index, i;

              /* PhaseH is 64bits but only lower 16b of each entry is valid
               * Form 32b word with 16bit LSB from 2 consecutive entries
               * Need 1 32b write to get 2 entries into IP registers
               * (i is array loc and index is address offset)
               */
              index = 0;
              for(i=0; i < loopWidth; i+=2)
              {
                lsb = (u32)(HscL2DataPtr->phasesH[i]   & (u64)XHSC_MASK_LOW_16BITS);
                msb = (u32)(HscL2DataPtr->phasesH[i+1] & (u64)XHSC_MASK_LOW_16BITS);
                val = (msb<<16 | lsb);
                Xil_Out32(baseAddr+(index*4), val);
                ++index;
              }
            }
            break;

    case XVIDC_PPC_2:
            {
              u32 val, i;

              /* PhaseH is 64bits but only lower 32b of each entry is valid
               * Need 1 32b write to get each entry into IP registers
               */
              for(i=0; i < loopWidth; ++i)
              {
                val = (u32)(HscL2DataPtr->phasesH[i] & XHSC_MASK_LOW_32BITS);
                Xil_Out32(baseAddr+(i*4), val);
              }
            }
            break;

    case XVIDC_PPC_4:
            {
              u32 lsb, msb, index, offset, i;
			  u64 phaseHData;

              /* PhaseH is 64bits and each entry has valid 32b MSB & LSB
               * Need 2 32b writes to get each entry into IP registers
               * (index is array loc and offset is address offset)
               */
              index = 0;
			  offset = 0;
              for(i=0; i < loopWidth; ++i)
              {
			    phaseHData = HscL2DataPtr->phasesH[index];
                lsb = (u32)(phaseHData & XHSC_MASK_LOW_32BITS);
                msb = (u32)((phaseHData>>32) & XHSC_MASK_LOW_32BITS);
                Xil_Out32(baseAddr+(offset*4), lsb);
                Xil_Out32(baseAddr+((offset+1)*4), msb);
                ++index;
				offset += 2;
              }
            }
            break;

    default:
           break;
  }
}


/*****************************************************************************/
/**
* This function programs the filter coefficients and phase data into core
* registers
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  HscL2DataPtr is a pointer to the core instance layer 2 data.
*
* @return None
*
* @Note  This version of driver does not make use of computed coefficients.
*        User must load the coefficients, using the provided API, before
*        scaler can be used
******************************************************************************/
static void XV_HScalerSetCoeff(XV_hscaler *pHsc,
                               XV_hscaler_l2 *HscL2DataPtr)
{
  int num_phases = 1<<pHsc->Config.PhaseShift;
  int num_taps   = pHsc->Config.NumTaps/2;
  int val,i,j,offset,rdIndx;
  u32 baseAddr;

  offset = (XV_HSCALER_MAX_H_TAPS - pHsc->Config.NumTaps)/2;
  baseAddr = XV_hscaler_Get_HwReg_hfltCoeff_BaseAddress(pHsc);
  for (i = 0; i < num_phases; i++)
  {
    for(j=0; j < num_taps; j++)
    {
       rdIndx = j*2+offset;
       val = (HscL2DataPtr->coeff[i][rdIndx+1] << 16) | (HscL2DataPtr->coeff[i][rdIndx] & XHSC_MASK_LOW_16BITS);
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
* @param  HscL2DataPtr is a pointer to the core instance layer 2 data.
* @param  HeightIn is the input stream height
* @param  WidthIn is the input stream width
* @param  WidthOut is the output stream width
* @param  cformat is the input stream color format
*
* @return None
*
******************************************************************************/
void XV_HScalerSetup(XV_hscaler  *InstancePtr,
                     XV_hscaler_l2 *HscL2DataPtr,
                     u32 HeightIn,
                     u32 WidthIn,
                     u32 WidthOut,
                     u32 cformat)
{
  u32 PixelRate;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(HscL2DataPtr != NULL);
  Xil_AssertVoid((HeightIn>0) && (HeightIn<=InstancePtr->Config.MaxHeight));
  Xil_AssertVoid((WidthIn>0) && (WidthIn<=InstancePtr->Config.MaxWidth));
  Xil_AssertVoid((WidthOut>0) && (WidthOut<=InstancePtr->Config.MaxWidth));
  Xil_AssertVoid((InstancePtr->Config.PixPerClk >= XVIDC_PPC_1) &&
                 (InstancePtr->Config.PixPerClk <= XVIDC_PPC_4));

  PixelRate = (WidthIn * STEP_PRECISION)/WidthOut;

  if(InstancePtr->Config.ScalerType == XV_HSCALER_POLYPHASE)
  {
    if(!HscL2DataPtr->UseExtCoeff)  //No user defined coefficients
    {
      /* Determine coefficient table to use */
      XV_HScalerSelectCoeff(InstancePtr,
		                HscL2DataPtr,
		                WidthIn,
		                WidthOut);
    }
    /* Program generated coefficients into the IP register bank */
    XV_HScalerSetCoeff(InstancePtr, HscL2DataPtr);
  }

  /* Compute Phase for 1 line */
  CalculatePhases(InstancePtr, HscL2DataPtr, WidthIn, WidthOut, PixelRate);

  /* Program computed Phase into the IP register bank */
  XV_HScalerSetPhase(InstancePtr, HscL2DataPtr);

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
  u32 baseAddr, taps, phases;
  int val,i,j;
  const char *ScalerTypeStr[] = {"Bilinear", "Bicubic", "Polyphase"};

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);

  xil_printf("\r\n\r\n----->H SCALER IP STATUS<----\r\n");
  done  = XV_hscaler_IsDone(pHsc);
  idle  = XV_hscaler_IsIdle(pHsc);
  ready = XV_hscaler_IsReady(pHsc);
  ctrl =  XV_hscaler_ReadReg(pHsc->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_AP_CTRL);

  heightin = XV_hscaler_Get_HwReg_Height(pHsc);
  widthin  = XV_hscaler_Get_HwReg_WidthIn(pHsc);
  widthout = XV_hscaler_Get_HwReg_WidthOut(pHsc);
  cformat  = XV_hscaler_Get_HwReg_ColorMode(pHsc);
  pixrate  = XV_hscaler_Get_HwReg_PixelRate(pHsc);

  taps   = pHsc->Config.NumTaps/2;
  phases = (1<<pHsc->Config.PhaseShift);
  xil_printf("IsDone:  %d\r\n", done);
  xil_printf("IsIdle:  %d\r\n", idle);
  xil_printf("IsReady: %d\r\n", ready);
  xil_printf("Ctrl:    0x%x\r\n\r\n", ctrl);

  if(pHsc->Config.ScalerType <= XV_HSCALER_POLYPHASE)
  {
    xil_printf("Scaler Type:     %s\r\n",ScalerTypeStr[pHsc->Config.ScalerType]);
  }
  else
  {
    xil_printf("Scaler Type:     Unknown\r\n");
  }
  xil_printf("Input Height:    %d\r\n",heightin);
  xil_printf("Input Width:     %d\r\n",widthin);
  xil_printf("Output Width:    %d\r\n",widthout);
  xil_printf("Color Format:    %d\r\n",cformat);
  xil_printf("Pixel Rate:      %d\r\n",pixrate);
  xil_printf("Num Phases:      %d\r\n",phases);

  if(pHsc->Config.ScalerType == XV_HSCALER_POLYPHASE)
  {
    short lsb, msb;

    xil_printf("Num Taps:        %d\r\n",taps*2);
    xil_printf("\r\nCoefficients:");

    baseAddr = XV_hscaler_Get_HwReg_hfltCoeff_BaseAddress(pHsc);
    for(i = 0; i < phases; i++)
    {
      xil_printf("\r\nPhase %2d: ",i);
      for(j=0; j< taps; j++)
      {
        val = Xil_In32(baseAddr+((i*taps+j)*4));

        //coefficients are 12-bits
        lsb = (short)(val & XHSC_MASK_LOW_16BITS);
        msb = (short)((val & XHSC_MASK_HIGH_16BITS)>>16);

        xil_printf("%5d %5d ", lsb, msb);
      }
    }
  }
}
/** @} */
