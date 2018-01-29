/******************************************************************************
*
* Copyright (C) 2017 - 2018 Xilinx, Inc. All rights reserved.
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
* @file mcdp6000.c
* @addtogroup dprxss_v5_0
* @{
*
* This is the header file contains macros, enum, structure and function
* prototypes for MCDP6000.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- ----------------------------------------------------------
* 1.00 Kei 01/23/18 Initial release.
* </pre>
*
******************************************************************************/
#ifndef XDPRXSS_MCDP6000_H_
/* Prevent circular inclusions by using protection macros. */
#define XDPRXSS_MCDP6000_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xvidc.h"
#include "xparameters.h"
#include "xil_types.h"

#ifdef XPAR_XIIC_NUM_INSTANCES
#include "xiic.h"
#endif /* End of XPAR_XIIC_NUM_INSTANCES */

/************************** Constant Definitions *****************************/

#define XDPRXSS_MCDP6000_IIC_SLAVE          0x14    /**< MCDP6000 slave device
 	 	 	 	 	 	      */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
u32 XDpRxSs_MCDP6000_GetRegister(u32 I2CBaseAddress, u8 I2CSlaveAddress,
			 u16 RegisterAddress);
int XDpRxSs_MCDP6000_SetRegister(u32 I2CBaseAddress, u8 I2CSlaveAddress,
			 u16 RegisterAddress, u32 Value);
int XDpRxSs_MCDP6000_ModifyRegister(u32 I2CBaseAddress, u8 I2CSlaveAddress,
			    u16 RegisterAddress, u32 Value, u32 Mask);

int XDpRxSs_MCDP6000_DpInit(u32 I2CBaseAddress, u8 I2CSlaveAddress);
int XDpRxSs_MCDP6000_IbertInit(u32 I2CBaseAddress, u8 I2CSlaveAddress);
int XDpRxSs_MCDP6000_ResetDpPath(u32 I2CBaseAddress, u8 I2CSlaveAddress);

int XDpRxSs_MCDP6000_EnablePrbs7_Tx(u32 I2CBaseAddress, u8 I2CSlaveAddress);
int XDpRxSs_MCDP6000_EnablePrbs7_Rx(u32 I2CBaseAddress, u8 I2CSlaveAddress);
int XDpRxSs_MCDP6000_DisablePrbs7_Rx(u32 I2CBaseAddress, u8 I2CSlaveAddress);
int XDpRxSs_MCDP6000_Read_ErrorCounters(u32 I2CBaseAddress, u8 I2CSlaveAddress);
int XDpRxSs_MCDP6000_EnableCounter(u32 I2CBaseAddress, u8 I2CSlaveAddress);
int XDpRxSs_MCDP6000_ClearCounter(u32 I2CBaseAddress, u8 I2CSlaveAddress);

void XDpRxSs_MCDP6000_RegisterDump(u32 I2CBaseAddress, u8 I2CSlaveAddress);
int XDpRxSs_MCDP6000_TransparentMode(u32 I2CBaseAddress, u8 I2CSlaveAddress);
int XDpRxSs_MCDP6000_BWchange(u32 I2CBaseAddress, u8 I2CSlaveAddress);
int XDpRxSs_MCDP6000_AccessLaneSet(u32 I2CBaseAddress, u8 I2CSlaveAddress);

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif
#endif /* End of protection macro */
/** @} */
