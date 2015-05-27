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
* @file xv_deinterlacer_l2.c
*
* The deint Layer-2 Driver. The functions in this file provides an abstraction
* from the register peek/poke methodology by implementing most common use-case
* provided by the sub-core. See xv_deinterlacer_l2.h for a detailed description
* of the layer-2 driver
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
#include "xv_deinterlacer_l2.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**************************** Local Global *******************************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
* This function starts the deinterlacer core
*
* @param  InstancePtr is a pointer to the core instance to be worked on
*
* @return None
*
******************************************************************************/
void XV_DeintStart(XV_deinterlacer *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_deinterlacer_EnableAutoRestart(InstancePtr);
  XV_deinterlacer_Start(InstancePtr);
}

/*****************************************************************************/
/**
* This function stops the deinterlacer core
*
* @param  InstancePtr is a pointer to the core instance to be worked on
*
* @return None
*
******************************************************************************/
void XV_DeintStop(XV_deinterlacer *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_deinterlacer_DisableAutoRestart(InstancePtr);
}

/*****************************************************************************/
/**
* This function sets the deinterlacer cores RD/WR field buffers addresses
* and color space
*
* @param  InstancePtr is a pointer to the core instance to be worked on
* @param  memAddr is the buffer address in DDR for RD/WR clients
* @param  cformat is the input stream color format
*
* @return None
*
******************************************************************************/
void XV_DeintSetFieldBuffers(XV_deinterlacer   *InstancePtr,
							  u32               memAddr,
							  XVidC_ColorFormat cformat)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_deinterlacer_Set_read_fb(InstancePtr, memAddr);
  XV_deinterlacer_Set_write_fb(InstancePtr, memAddr);
  XV_deinterlacer_Set_colorFormat(InstancePtr, cformat);
}

/*****************************************************************************/
/**
*
* This function prints Deinterlacer status on console
*
* @param	InstancePtr is the instance pointer to the IP instance.
*
* @return	None
*
******************************************************************************/
void XV_DeintDbgReportStatus(XV_deinterlacer *InstancePtr)
{
  XV_deinterlacer *pDeint = InstancePtr;
  u32 done, idle, ready, ctrl;
  u32 rfb, wfb, colformat, algo;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);

  xil_printf("\r\n\r\n----->Deinterlacer IP STATUS<----\r\n");

  done  = XV_deinterlacer_IsDone(pDeint);
  idle  = XV_deinterlacer_IsIdle(pDeint);
  ready = XV_deinterlacer_IsReady(pDeint);
  ctrl  = XV_deinterlacer_ReadReg(pDeint->Axilites_BaseAddress, XV_DEINTERLACER_AXILITES_ADDR_AP_CTRL);

  rfb  = XV_deinterlacer_Get_read_fb(pDeint);
  wfb  = XV_deinterlacer_Get_write_fb(pDeint);
  colformat = XV_deinterlacer_Get_colorFormat(pDeint);
  algo      = XV_deinterlacer_Get_algo(pDeint);

  xil_printf("IsDone:  %d\r\n",     done);
  xil_printf("IsIdle:  %d\r\n",     idle);
  xil_printf("IsReady: %d\r\n",     ready);
  xil_printf("Ctrl:    0x%x\r\n\r\n", ctrl);

  xil_printf("Read Frame Buffer:  0x%x\r\n", rfb);
  xil_printf("Write Frame Buffer: 0x%x\r\n", wfb);
  xil_printf("Color Format:       %d\r\n", colformat);
  xil_printf("Algo Selected:      %d\r\n", algo);
}
