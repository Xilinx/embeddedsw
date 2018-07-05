/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hscaler_l2.c
* @addtogroup v_hscaler_v3_3
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
* 2.00  rco   11/05/15   Integrate layer-1 with layer-2
*       dmc   12/17/15   Updated the XV_HScalerDbgReportStatus routine
* 3.0   mpe   04/28/16   Added optional color format conversion handling
* 3.1   rco   11/01/16   Fixed bug in config validation API, wherein hi/lo
*                        check should be made only if input is not RGB
*       rco   02/09/17   Fix c++ compilation warnings
*	jsr   09/07/18 Fix for 64-bit driver support
* 3.3   vsa   04/07/20   Improve quality with better coefficient tables
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "xv_hscaler_l2.h"

/************************** Constant Definitions *****************************/
#define STEP_PRECISION         (65536)  // 2^16

/* Mask definitions for Low and high 16 bits in a 32 bit number */
#define XHSC_MASK_LOW_16BITS       (0x0000FFFF)
#define XHSC_MASK_HIGH_16BITS      (0xFFFF0000)
#define XHSC_MASK_LOW_20BITS	   (0x000FFFFF)
#define XHSC_MASK_LOW_12BITS	   (0x00000FFF)

/**************************** Type Definitions *******************************/

/**************************** Local Global *******************************/
static const int STEP_PRECISION_SHIFT = 16;
static const u64 XHSC_MASK_LOW_32BITS = ((u64)1<<32)-1;

extern const short XV_hscaler_Lanczos2_taps6[XV_HSCALER_MAX_H_PHASES][XV_HSCALER_TAPS_6];
extern const short XV_hscaler_fixedcoeff_taps6_ScalingRatio1p2[XV_HSCALER_MAX_H_PHASES][XV_HSCALER_TAPS_6];
extern const short XV_hscaler_fixedcoeff_taps6_ScalingRatio2[XV_HSCALER_MAX_H_PHASES][XV_HSCALER_TAPS_6];
extern const short XV_hscaler_fixedcoeff_taps6_ScalingRatio3[XV_HSCALER_MAX_H_PHASES][XV_HSCALER_TAPS_6];
extern const short XV_hscaler_fixedcoeff_taps6_ScalingRatio4[XV_HSCALER_MAX_H_PHASES][XV_HSCALER_TAPS_6];
extern const short XV_hscaler_fixedcoeff_taps8_ScalingRatio2[XV_HSCALER_MAX_H_PHASES][XV_HSCALER_TAPS_8];
extern const short XV_hscaler_fixedcoeff_taps8_ScalingRatio3[XV_HSCALER_MAX_H_PHASES][XV_HSCALER_TAPS_8];
extern const short XV_hscaler_fixedcoeff_taps8_ScalingRatio4[XV_HSCALER_MAX_H_PHASES][XV_HSCALER_TAPS_8];
extern const short XV_hscaler_fixedcoeff_taps10_ScalingRatio3[XV_HSCALER_MAX_H_PHASES][XV_HSCALER_TAPS_10];
extern const short XV_hscaler_fixedcoeff_taps10_ScalingRatio4[XV_HSCALER_MAX_H_PHASES][XV_HSCALER_TAPS_10];
extern const short XV_hscaler_fixedcoeff_taps12_ScalingRatio4[XV_HSCALER_MAX_H_PHASES][XV_HSCALER_TAPS_12];

/************************** Function Prototypes ******************************/
static void XV_HScalerSelectCoeff(XV_Hscaler_l2 *InstancePtr,
                                  u32 WidthIn,
                                  u32 WidthOut);
static void CalculatePhases(XV_Hscaler_l2 *HscPtr,
                            u32 WidthIn,
                            u32 WidthOut,
                            u32 PixelRate);

static void XV_HScalerSetCoeff(XV_Hscaler_l2 *HscPtr);
static void XV_HScalerSetPhase(XV_Hscaler_l2 *HscPtr);

/*****************************************************************************/
/**
* This function initializes the core instance
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  DeviceId is instance id of the core
*
* @return XST_SUCCESS if device is found and initialized
*         XST_DEVICE_NOT_FOUND if device is not found
*
******************************************************************************/
int XV_HScalerInitialize(XV_Hscaler_l2 *InstancePtr, u16 DeviceId)
{
  int Status;
  Xil_AssertNonvoid(InstancePtr != NULL);

  /* Setup the instance */
  memset(InstancePtr, 0, sizeof(XV_Hscaler_l2));
  Status = XV_hscaler_Initialize(&InstancePtr->Hsc, DeviceId);

  return(Status);
}

/*****************************************************************************/
/**
* This function starts the horizontal scaler core
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_HScalerStart(XV_Hscaler_l2 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_hscaler_EnableAutoRestart(&InstancePtr->Hsc);
  XV_hscaler_Start(&InstancePtr->Hsc);
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
void XV_HScalerStop(XV_Hscaler_l2 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_hscaler_DisableAutoRestart(&InstancePtr->Hsc);
}

/*****************************************************************************/
/**
* This function determines the internal coeffiecient table to be used based on
* scaling ratio and loads the filter coefficients in the scaler coefficient
* storage.
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  WidthIn is the input stream width
* @param  Widthout is the output stream width
*
* @return None
*
******************************************************************************/
static void XV_HScalerSelectCoeff(XV_Hscaler_l2 *InstancePtr,
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

  numPhases = (1<<InstancePtr->Hsc.Config.PhaseShift);

  IsScaleDown = (WidthOut < WidthIn);

  /* Scale Down Mode will use dynamic filter selection logic
   * Scale Up Mode (including 1:1) will always use 6 tap filter
   */
  if(IsScaleDown)
  {
    ScalingRatio = ((WidthIn * 10)/WidthOut);

    switch(InstancePtr->Hsc.Config.NumTaps)
    {
      case XV_HSCALER_TAPS_6:
		if (ScalingRatio > 35) {// > 3.5
			coeff = &XV_hscaler_fixedcoeff_taps6_ScalingRatio4[0][0];
			numTaps = XV_HSCALER_TAPS_6;
		} else if (ScalingRatio > 25) {//2.6 < Ratio <= 3.5
			coeff = &XV_hscaler_fixedcoeff_taps6_ScalingRatio3[0][0];
			numTaps = XV_HSCALER_TAPS_6;
		} else if (ScalingRatio > 15) {//1.6 < Ratio <= 2.5
			coeff = &XV_hscaler_fixedcoeff_taps6_ScalingRatio2[0][0];
			numTaps = XV_HSCALER_TAPS_6;
		} else {// <= 1.5
			coeff = &XV_hscaler_fixedcoeff_taps6_ScalingRatio1p2[0][0];
			numTaps = XV_HSCALER_TAPS_6;
		}
           break;

      case XV_HSCALER_TAPS_8:
		if (ScalingRatio > 35) {// > 3.5
			coeff = &XV_hscaler_fixedcoeff_taps8_ScalingRatio4[0][0];
			numTaps = XV_HSCALER_TAPS_8;
		} else if (ScalingRatio > 25) {//2.6 < Ratio <= 3.5
			coeff = &XV_hscaler_fixedcoeff_taps8_ScalingRatio3[0][0];
			numTaps = XV_HSCALER_TAPS_8;
		} else if (ScalingRatio > 15) {//1.6 < Ratio <= 2.5
			coeff = &XV_hscaler_fixedcoeff_taps8_ScalingRatio2[0][0];
			numTaps = XV_HSCALER_TAPS_8;
		} else {// <= 1.5
			coeff = &XV_hscaler_fixedcoeff_taps6_ScalingRatio1p2[0][0];
			numTaps = XV_HSCALER_TAPS_6;
		}
           break;

      case XV_HSCALER_TAPS_10:
		if (ScalingRatio > 35) {// > 3.5
			coeff = &XV_hscaler_fixedcoeff_taps10_ScalingRatio4[0][0];
			numTaps = XV_HSCALER_TAPS_10;
		} else if (ScalingRatio > 25) {//2.6 < Ratio <= 3.5
			coeff = &XV_hscaler_fixedcoeff_taps10_ScalingRatio3[0][0];
			numTaps = XV_HSCALER_TAPS_10;
		} else if (ScalingRatio > 15) {//1.6 < Ratio <= 2.5
			coeff = &XV_hscaler_fixedcoeff_taps8_ScalingRatio2[0][0];
			numTaps = XV_HSCALER_TAPS_8;
		} else {// <= 1.5
			coeff = &XV_hscaler_fixedcoeff_taps6_ScalingRatio1p2[0][0];
			numTaps = XV_HSCALER_TAPS_6;
		}
           break;

      case XV_HSCALER_TAPS_12:
		if (ScalingRatio > 35) {// > 3.5
			coeff = &XV_hscaler_fixedcoeff_taps12_ScalingRatio4[0][0];
			numTaps = XV_HSCALER_TAPS_12;
		} else if (ScalingRatio > 25) {//2.6 < Ratio <= 3.5
			coeff = &XV_hscaler_fixedcoeff_taps10_ScalingRatio3[0][0];
			numTaps = XV_HSCALER_TAPS_10;
		} else if (ScalingRatio > 15) {//1.6 < Ratio <= 2.5
			coeff = &XV_hscaler_fixedcoeff_taps8_ScalingRatio2[0][0];
			numTaps = XV_HSCALER_TAPS_8;
		} else {// <= 1.5
			coeff = &XV_hscaler_fixedcoeff_taps6_ScalingRatio1p2[0][0];
			numTaps = XV_HSCALER_TAPS_6;
		}
           break;

      default:
          return;
    }
  }
  else //Scale Up
  {
	coeff = &XV_hscaler_Lanczos2_taps6[0][0];
	numTaps = XV_HSCALER_TAPS_6;
  }

  XV_HScalerLoadExtCoeff(InstancePtr,
                         numPhases,
                         numTaps,
                         coeff);

  /* Disable use of external coefficients */
  InstancePtr->UseExtCoeff = FALSE;
}

/*****************************************************************************/
/**
* This function loads user defined filter coefficients in the scaler coefficient
* storage
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  num_phases is the number of phases in coefficient table
* @param  num_taps is the number of taps in coefficient table
* @param  Coeff is a pointer to user defined filter coefficients table
*
* @return None
*
******************************************************************************/
void XV_HScalerLoadExtCoeff(XV_Hscaler_l2 *InstancePtr,
                            u16 num_phases,
                            u16 num_taps,
                            const short *Coeff)
{
  int i,j, pad, offset;

  /*
   * validate input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(num_taps <= InstancePtr->Hsc.Config.NumTaps);
  Xil_AssertVoid(num_phases == (1<<InstancePtr->Hsc.Config.PhaseShift));
  Xil_AssertVoid(Coeff != NULL);

  switch(num_taps)
  {
    case XV_HSCALER_TAPS_6:
    case XV_HSCALER_TAPS_8:
    case XV_HSCALER_TAPS_10:
    case XV_HSCALER_TAPS_12:
         break;

    default:
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
      InstancePtr->coeff[i][j+offset] = Coeff[i*num_taps+j];
    }
  }

  if(pad) //effective taps < max_taps
  {
    for (i = 0; i < num_phases; i++)
    {
      //pad left
      for (j = 0; j < offset; j++)
      {
        InstancePtr->coeff[i][j] = 0;
      }
        //pad right
      for (j = (num_taps+offset); j < XV_HSCALER_MAX_H_TAPS; j++)
      {
        InstancePtr->coeff[i][j] = 0;
      }
    }
  }

  /* Enable use of external coefficients */
  InstancePtr->UseExtCoeff = TRUE;
}

/*****************************************************************************/
/**
* This function calculates the phases for 1 line. Same phase info is used for
* full frame
*
* @param  HscPtr is a pointer to the core instance to be worked on.
* @param  WidthIn is the input frame width
* @param  WidthOut is the scaled frame width
* @param  PixelRate is the number of pixels per clock being processed
*
* @return None
*
******************************************************************************/
static void CalculatePhases(XV_Hscaler_l2 *HscPtr,
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
    int MaxPhases = (1<<HscPtr->Hsc.Config.PhaseShift);
    loopWidth = ((WidthIn > WidthOut) ? WidthIn + (HscPtr->Hsc.Config.PixPerClk-1)
                                      : WidthOut  +(HscPtr->Hsc.Config.PixPerClk-1))/HscPtr->Hsc.Config.PixPerClk;


    arrayIdx = 0;
    for (x=0; x<loopWidth; x++)
    {
        HscPtr->phasesH[x] = 0;
	HscPtr->phasesH_H[x] = 0;
	nrRdsClck = 0;
        for (s=0; s<HscPtr->Hsc.Config.PixPerClk; s++)
        {
            PhaseH = (offset>>(STEP_PRECISION_SHIFT-HscPtr->Hsc.Config.PhaseShift)) & (MaxPhases-1);//(HSC_PHASES-1);
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

            if (((offset >> STEP_PRECISION_SHIFT) == 0) && (xWritePos< (int)WidthOut))
            {
                // produce a new output sample
                offset += PixelRate;
                OutputWriteEn = 1;
                xWritePos++;
            }

	    if (HscPtr->Hsc.Config.PixPerClk == XVIDC_PPC_8)
	    {
		    if (s < 4 ) {
			    HscPtr->phasesH[x] |= (PhaseH << (s*11));
			    HscPtr->phasesH[x] |= (arrayIdx << (6 + (s*11)));
			    HscPtr->phasesH[x] |= (OutputWriteEn << (10 + (s*11)));
		    } else {
			    HscPtr->phasesH_H[x] |= (PhaseH << ((s-4)*11));
			    HscPtr->phasesH_H[x] |= (arrayIdx << (6 + ((s-4)*11)));
			    HscPtr->phasesH_H[x] |= (OutputWriteEn << (10 + ((s-4)*11)));
		    }
	    } else if (HscPtr->Hsc.Config.PixPerClk == XVIDC_PPC_4) {
              HscPtr->phasesH[x] = HscPtr->phasesH[x] | (PhaseH << (s*10));
              HscPtr->phasesH[x] = HscPtr->phasesH[x] | (arrayIdx << (6 + (s*10)));
              HscPtr->phasesH[x] = HscPtr->phasesH[x] | (OutputWriteEn << (9 + (s*10)));
            }
            else
            {
              HscPtr->phasesH[x] = HscPtr->phasesH[x] | (PhaseH << (s*9));
              HscPtr->phasesH[x] = HscPtr->phasesH[x] | (arrayIdx << (6 + (s*9)));
              HscPtr->phasesH[x] = HscPtr->phasesH[x] | (OutputWriteEn << (8 + (s*9)));
            }

            if (GetNewPix) nrRdsClck++;
        }
        if (arrayIdx>=HscPtr->Hsc.Config.PixPerClk)
            arrayIdx &= (HscPtr->Hsc.Config.PixPerClk-1);

        nrRds += nrRdsClck;
        if (nrRds >= HscPtr->Hsc.Config.PixPerClk)
            nrRds -= HscPtr->Hsc.Config.PixPerClk;
    }
}

/*****************************************************************************/
/**
* This function programs the phase data into core registers
*
* @param  HscPtr is a pointer to the core instance to be worked on.
*
* @return None
*
* @Note  This version of driver does not make use of computed coefficients.
*        User must load the coefficients, using the provided API, before
*        scaler can be used
******************************************************************************/
static void XV_HScalerSetPhase(XV_Hscaler_l2 *HscPtr)
{
  u32 loopWidth;
  UINTPTR baseAddr;
  //program phases
  baseAddr = XV_hscaler_Get_HwReg_phasesH_V_BaseAddress(&HscPtr->Hsc);
  loopWidth = HscPtr->Hsc.Config.MaxWidth/HscPtr->Hsc.Config.PixPerClk;
  switch(HscPtr->Hsc.Config.PixPerClk)
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
                lsb = (u32)(HscPtr->phasesH[i]   & (u64)XHSC_MASK_LOW_16BITS);
                msb = (u32)(HscPtr->phasesH[i+1] & (u64)XHSC_MASK_LOW_16BITS);
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
                val = (u32)(HscPtr->phasesH[i] & XHSC_MASK_LOW_32BITS);
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
                phaseHData = HscPtr->phasesH[index];
                lsb = (u32)(phaseHData & XHSC_MASK_LOW_32BITS);
                msb = (u32)((phaseHData>>32) & XHSC_MASK_LOW_32BITS);
                Xil_Out32(baseAddr+(offset*4), lsb);
                Xil_Out32(baseAddr+((offset+1)*4), msb);
                ++index;
                offset += 2;
              }
            }
            break;
    case XVIDC_PPC_8:
	    {
		u32 bits_0_31, bits_32_63, bits_64_95;
		u32 index, offset, i;
		u64 phaseHData, phaseHData_H;
		/*
		 * PhaseH and PhaseH_H are 64bits and each entry has valid 44
		 * bits. PhaseH has lower 44 bits and PhaseH_H has higer 44 bits
		 * Need to form 3 32b writes form the total 88 bits, and
		 * write each 32bits into IP registers.
		 * (index is array loc and offset is address offset)
		 */
		index = 0;
		offset = 0;
		for(i=0; i < loopWidth; i++) {
			bits_0_31 = 0;
			bits_32_63 = 0;
			bits_64_95 = 0;
			phaseHData = HscPtr->phasesH[index];
			phaseHData_H = HscPtr->phasesH_H[index];

			bits_0_31 = (u32)(phaseHData & XHSC_MASK_LOW_32BITS);
			bits_32_63 = (u32)((phaseHData>>32) & XHSC_MASK_LOW_32BITS);
			bits_32_63 |= ((u32)((phaseHData_H & XHSC_MASK_LOW_20BITS)) << 12);
			bits_64_95 = (((u32)(phaseHData_H & XHSC_MASK_LOW_32BITS)) >> 20);
			bits_64_95 |= (((u32)(phaseHData_H>>32) & XHSC_MASK_LOW_12BITS) << 12);
			Xil_Out32(baseAddr+(offset*4), bits_0_31);
			Xil_Out32(baseAddr+((offset+1)*4), bits_32_63);
			Xil_Out32(baseAddr+((offset+2)*4), bits_64_95);
			/*(offset+3)*4 register is reserved,so increment offset by 4*/
			offset += 4;
			index++;
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
* @param  HscPtr is a pointer to the core instance to be worked on.
*
* @return None
*
* @Note  This version of driver does not make use of computed coefficients.
*        User must load the coefficients, using the provided API, before
*        scaler can be used
******************************************************************************/
static void XV_HScalerSetCoeff(XV_Hscaler_l2 *HscPtr)
{
  int num_phases = 1<<HscPtr->Hsc.Config.PhaseShift;
  int num_taps   = HscPtr->Hsc.Config.NumTaps/2;
  int val,i,j,offset,rdIndx;
  UINTPTR baseAddr;

  offset = (XV_HSCALER_MAX_H_TAPS - HscPtr->Hsc.Config.NumTaps)/2;
  baseAddr = XV_hscaler_Get_HwReg_hfltCoeff_BaseAddress(&HscPtr->Hsc);
  for (i = 0; i < num_phases; i++)
  {
    for(j=0; j < num_taps; j++)
    {
       rdIndx = j*2+offset;
       val = (HscPtr->coeff[i][rdIndx+1] << 16) | (HscPtr->coeff[i][rdIndx] & XHSC_MASK_LOW_16BITS);
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
* @param  HeightIn is the input stream height
* @param  WidthIn is the input stream width
* @param  WidthOut is the output stream width
* @param  ColorFormatIn is the input stream color format
* @param  ColorFormatOut is the output stream color format
*
* @return XST_SUCCESS if the requested setup parameters are valid
*         XST_FAILURE otherwise
*
******************************************************************************/
int XV_HScalerSetup(XV_Hscaler_l2  *InstancePtr,
                     u32 HeightIn,
                     u32 WidthIn,
                     u32 WidthOut,
                     u32 ColorFormatIn,
                     u32 ColorFormatOut)
{
  u32 PixelRate;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid((HeightIn>0) && (HeightIn<=InstancePtr->Hsc.Config.MaxHeight));
  Xil_AssertNonvoid((WidthIn>0) && (WidthIn<=InstancePtr->Hsc.Config.MaxWidth));
  Xil_AssertNonvoid((WidthOut>0) && (WidthOut<=InstancePtr->Hsc.Config.MaxWidth));
  Xil_AssertNonvoid((InstancePtr->Hsc.Config.PixPerClk >= XVIDC_PPC_1) &&
		  (InstancePtr->Hsc.Config.PixPerClk <= XVIDC_PPC_8));

  if(!XV_HScalerValidateConfig(InstancePtr, ColorFormatIn, ColorFormatOut))
  {
    return XST_FAILURE;
  }

  PixelRate = (WidthIn * STEP_PRECISION)/WidthOut;

  if(InstancePtr->Hsc.Config.ScalerType == XV_HSCALER_POLYPHASE)
  {
    if(!InstancePtr->UseExtCoeff)  //No user defined coefficients
    {
      /* Determine coefficient table to use */
      XV_HScalerSelectCoeff(InstancePtr, WidthIn, WidthOut);
    }
    /* Program generated coefficients into the IP register bank */
    XV_HScalerSetCoeff(InstancePtr);
  }

  /* Compute Phase for 1 line */
  CalculatePhases(InstancePtr, WidthIn, WidthOut, PixelRate);

  /* Program computed Phase into the IP register bank */
  XV_HScalerSetPhase(InstancePtr);

  XV_hscaler_Set_HwReg_Height(&InstancePtr->Hsc,        HeightIn);
  XV_hscaler_Set_HwReg_WidthIn(&InstancePtr->Hsc,       WidthIn);
  XV_hscaler_Set_HwReg_WidthOut(&InstancePtr->Hsc,      WidthOut);
  XV_hscaler_Set_HwReg_ColorMode(&InstancePtr->Hsc,     ColorFormatIn);
  XV_hscaler_Set_HwReg_ColorModeOut(&InstancePtr->Hsc,  ColorFormatOut);
  XV_hscaler_Set_HwReg_PixelRate(&InstancePtr->Hsc,     PixelRate);

  return XST_SUCCESS;
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
void XV_HScalerDbgReportStatus(XV_Hscaler_l2 *InstancePtr)
{
  XV_hscaler *HscPtr = &InstancePtr->Hsc;
  u32 done, idle, ready, ctrl;
  u32 widthin, widthout, height, pixrate, cformatin, cformatOut;
  u16 allow422, allow420, allowCsc;
  u32 taps, phases, i, j;
  UINTPTR baseAddr;
  int val;
  const char *ScalerTypeStr[] = {"Bilinear", "Bicubic", "Polyphase"};

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);

  done  = XV_hscaler_IsDone(HscPtr);
  idle  = XV_hscaler_IsIdle(HscPtr);
  ready = XV_hscaler_IsReady(HscPtr);
  ctrl =  XV_hscaler_ReadReg(HscPtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_AP_CTRL);

  height     = XV_hscaler_Get_HwReg_Height(HscPtr);
  widthin    = XV_hscaler_Get_HwReg_WidthIn(HscPtr);
  widthout   = XV_hscaler_Get_HwReg_WidthOut(HscPtr);
  cformatin  = XV_hscaler_Get_HwReg_ColorMode(HscPtr);
  cformatOut = XV_hscaler_Get_HwReg_ColorModeOut(HscPtr);
  pixrate    = XV_hscaler_Get_HwReg_PixelRate(HscPtr);
  allow422   = XV_HscalerIs422Enabled(InstancePtr);
  allow420   = XV_HscalerIs420Enabled(InstancePtr);
  allowCsc   = XV_HscalerIsCscEnabled(InstancePtr);

  taps   = HscPtr->Config.NumTaps/2;
  phases = (1<<HscPtr->Config.PhaseShift);

  xil_printf("\r\n\r\n----->H SCALER IP STATUS<----\r\n");
  xil_printf("IsDone:  %d\r\n", done);
  xil_printf("IsIdle:  %d\r\n", idle);
  xil_printf("IsReady: %d\r\n", ready);
  xil_printf("Ctrl:    0x%x\r\n\r\n", ctrl);

  if(HscPtr->Config.ScalerType <= XV_HSCALER_POLYPHASE) {
    xil_printf("Scaler Type:        %s\r\n",
      ScalerTypeStr[HscPtr->Config.ScalerType]);
  } else {
    xil_printf("Scaler Type:        Unknown\r\n");
  }
  xil_printf("Input&Output Height:    %d\r\n",height);
  xil_printf("Input Width:            %d\r\n",widthin);
  xil_printf("Output Width:           %d\r\n\r\n",widthout);

  xil_printf("4:2:2 processing:       %s\r\n", allow422?"Enabled":"Disabled");
  xil_printf("4:2:0 processing:       %s\r\n", allow420?"Enabled":"Disabled");
  xil_printf("Color space conversion: %s\r\n", allowCsc?"Enabled":"Disabled");
  xil_printf("Input Color Format:     %s\r\n",
		  XVidC_GetColorFormatStr((XVidC_ColorFormat)cformatin));
  xil_printf("Output Color Format:    %s\r\n",
		  XVidC_GetColorFormatStr((XVidC_ColorFormat)cformatOut));
  xil_printf("Pixel Rate:             %d\r\n\r\n",pixrate);

  xil_printf("Num Phases:          %d\r\n",phases);
  if(HscPtr->Config.ScalerType == XV_HSCALER_POLYPHASE)
  {
    short lsb, msb;

    xil_printf("Num Taps:            %d\r\n",taps*2);
    xil_printf("\r\nCoefficients:");

    baseAddr = XV_hscaler_Get_HwReg_hfltCoeff_BaseAddress(HscPtr);
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

/*****************************************************************************/
/**
* This function checks if the given input and output color formats
* are valid configuration parameters for this instance
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  cformat is the input stream color format
* @param  cformatOut is the output stream color format
*
* @return TRUE if the requested setup parameters are valid
*         FALSE otherwise
*
******************************************************************************/
int XV_HScalerValidateConfig(XV_Hscaler_l2 *InstancePtr,
                             u32 cformatIn,
                             u32 cformatOut)
{
  u32 lo, hi;

  if(cformatIn==XVIDC_CSF_YCRCB_422 && !XV_HscalerIs422Enabled(InstancePtr)) {
    return(FALSE);
  }

  if(cformatIn==XVIDC_CSF_RGB && !XV_HscalerIsCscEnabled(InstancePtr) && cformatOut!=XVIDC_CSF_RGB) {
    return(FALSE);
  }

  if(cformatIn != XVIDC_CSF_RGB) {
    lo = (XV_HscalerIsCscEnabled(InstancePtr)) ? XVIDC_CSF_RGB : XVIDC_CSF_YCRCB_444;
    hi = (XV_HscalerIs420Enabled(InstancePtr)) ? XVIDC_CSF_YCRCB_420 : ((XV_HscalerIs422Enabled(InstancePtr)) ? XVIDC_CSF_YCRCB_422 : XVIDC_CSF_YCRCB_444);
    if (cformatOut<lo || cformatOut>hi )
    {
      return(FALSE);
    }
  }
  return(TRUE);
}

/** @} */
