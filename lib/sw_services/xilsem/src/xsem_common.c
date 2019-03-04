/*****************************************************************************
* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/**
* @file xsem_common.c
*  This file contains routines that are common for all SEM components
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date        Changes
* ----  ----     --------    --------------------------------------------------
* 0.1   mw       06/26/2018  Initial creation
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsem_common.h"

/************************** Variable Definitions *****************************/
static u32 XSem_CxtInitDone = 0U;

/************************** Function Prototypes  *****************************/

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* SEM interface for delay functionality with granularity of usecs.
*
* @param  Delay : Delay to be waited for, in usecs.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XSem_WaitForUsecs(u32 Delay)
{
  u32 CycsPerUsec;
  u32 CxtFreqHz;
  u64 StartCycs;
  u64 CurrCycs;
  u64 DiffCycs;

  CxtFreqHz = XSem_In32(XSEM_CXT_REG_CNTRFREQ);
  CycsPerUsec = CxtFreqHz / (1000U*1000U);
  DiffCycs = ((u64)CycsPerUsec * (u64)Delay);

#ifndef __MICROBLAZE__
  StartCycs = XSem_In64(XSEM_CXT_REG_CNTRLOW);
  CurrCycs = StartCycs;
  while((CurrCycs - StartCycs) <= DiffCycs) {
    CurrCycs = XSem_In64(XSEM_CXT_REG_CNTRLOW);
  }
#else
  StartCycs = (u64)XSem_In32(XSEM_CXT_REG_CNTRLOW);
  /* Check if a wrap could happen */
  if((0xFFFFFFFFU - StartCycs) < DiffCycs) {
    /* Wait for wrap to happen i.e., LSB word to be 0 */
    do {
      CurrCycs = (u64)XSem_In32(XSEM_CXT_REG_CNTRLOW);
    } while(CurrCycs >= StartCycs);

    /* Now wait for the extra cycles after wrap */
    DiffCycs -= (0xFFFFFFFFU - StartCycs);
    StartCycs = 0U;
  }

  CurrCycs = StartCycs;
  while((CurrCycs - StartCycs) <= DiffCycs) {
    CurrCycs = (u64)XSem_In32(XSEM_CXT_REG_CNTRLOW);
  }
#endif
}

/*****************************************************************************/
/**
*
* Function to initialize the CXT timestamp generator (or IOU timestamp counter)
* to start the timer in free running mode.
*
* @param  None.
*
* @return XST_SUCCESS if successful, else XST_FAILURE.
*
* @note   None.
*
******************************************************************************/
u32 XSem_CxtsgenInit(void)
{
  u32 Val;
  u32 Idx;
  u32 Status = XST_FAILURE;

  if(XSem_CxtInitDone == 0U) {
    XSem_CxtInitDone = 0x5AFEU;
  }
  else if(XSem_CxtInitDone == 0x5AFEU){
    XPlmi_Printf(DEBUG_INFO, "CXT timer already initialized\r\n");
    return XST_SUCCESS;
  }

  XPlmi_Printf(DEBUG_INFO, "CXT Timer Init Started\r\n");
  /*
   * Taking Time stamp Generator out of reset. Clear bit-20 of the
   * RST_LPD_IOU2
   */
  Val = XSem_In32(XSEM_REG_RST_LPD_IOU2);
  XSem_Out32(XSEM_REG_RST_LPD_IOU2, (Val & 0xFFEFFFFFU));
  for(Idx = 0U;Idx < XSEM_CXTINIT_WAIT_LOOP; Idx++); /* Wait loop */

  /* Set divisor to 15 and enable clock in TIMESTAMP_REF_CTRL reg */
  XSem_Out32(XSEM_REG_TS_REF_CTRL, ((1U << 24U) |
          (XSEM_CXTINIT_CLKDIV << 8U) | XSEM_CXTINIT_PLL_SRC));
  for(Idx = 0U;Idx < XSEM_CXTINIT_WAIT_LOOP; Idx++); /* Wait loop */

  /* Writet to the BASE_FREQUENCY_ID_REG register */
  XSem_Out32(XSEM_CXT_REG_CNTRFREQ, XSEM_CXTINIT_FREQ_HZ);
  XPlmi_Printf(DEBUG_INFO, "CXT freq in number of ticks per second = %x\r\n",
                    XSem_In32(XSEM_CXT_REG_CNTRFREQ));

  /* Enable the counter */
  XSem_Out32(XSEM_CXT_REG_CNTRCTRL, 0x1U);
  for(Idx = 0U;Idx < XSEM_CXTINIT_WAIT_LOOP; Idx++); /* Wait loop */

  Val = XSem_In32(XSEM_CXT_REG_CNTRCTRL);
  if((Val & 0x1U) == 0U) {
    XPlmi_Printf(DEBUG_INFO, "CXTSGEN Timer not started\r\n");
  }
  else {
    Status = XST_SUCCESS;
    XPlmi_Printf(DEBUG_INFO, "CXTSGEN timer init done successfully\r\n");
  }
  return Status;
}

/*****************************************************************************/
/**
*
* Function to clear a particular bit in a register.
*
* @param  Address of register, Bit location.
*
* @return VOID
*
* @note   AND the register value and the bit location. Then XOR the result
*         with the original register value to clear the bit.
*
******************************************************************************/
void XSem_ClrBit32(u32 Address, u32 BitLoc)
{
  u32 temp = 0U;
  u32 regVal = 0U;

  regVal = XSem_In32(Address);
  temp   =  regVal & BitLoc;
  regVal ^= temp;
  XSem_Out32(Address, regVal);

  return;
}

/*****************************************************************************/
/**
*
* Function to clear a particular bit in 32bit os data.
*
* @param  Data, Bit location.
*
* @return VOID
*
* @note   AND the data value and the bit location. Then XOR the result
*         with the original register value to clear the bit.
*
******************************************************************************/
u32 XSem_ClrVal32(u32 Data, u32 BitLoc)
{
  u32 temp = 0U;
  u32 tempData = Data;

  temp   =  Data & BitLoc;
  tempData ^= temp;

  return tempData;
}

/*****************************************************************************/
/**
*
* Function to set a particular bit in a register.
*
* @param  Address of register, Bit location.
*
* @return VOID
*
* @note   OR the register value and the bit location.
*
******************************************************************************/
void XSem_SetBit32(u32 Address, u32 BitLoc)
{
  u32 regVal = 0U;

  regVal = XSem_In32(Address);

  regVal   |=  BitLoc;
  XSem_Out32(Address, regVal);

  return;
}

/*****************************************************************************/
/**
*
* Function to set a particular bit in a register.
*
* @param  Address of register, Bit location.
*
* @return VOID
*
* @note   OR the register value and the bit location.
*
******************************************************************************/
void XSem_FatalState(void)
{
  u32 TempMask = 0U;

  XPlmi_Printf(DEBUG_INFO, "Entering SEM Fatal State\n");

  TempMask =  PMC_GLOBAL_SEM_STATUS_CRAM_FUNC_ACTIVE_MASK ;
  XSem_ClrBit32(PMC_GLOBAL_SEM_STATUS, TempMask);

  TempMask =  PMC_GLOBAL_SEM_STATUS_CRAM_FATAL_STATE_MASK;
  XSem_SetBit32(PMC_GLOBAL_SEM_STATUS, TempMask);

  while(1){
  }
}

/*****************************************************************************/
/**
*
* Function to read a 128b value using 32b Microblaze
*
* @param  Address of register, Pointer for each of the 4 32bit words to read
*
* @return VOID
*
* @note   None.
*
******************************************************************************/
void XSem_In128(u32 Addr, u32 *RegRd0, u32 *RegRd1, u32 *RegRd2, u32 *RegRd3)
{
   XPlmi_Printf(DEBUG_INFO, "DEBUG: Reading Reg Addr: 0x%08x\n", Addr);

   *RegRd0 = XSem_In32(Addr    );
   *RegRd1 = XSem_In32(Addr+0x4);
   *RegRd2 = XSem_In32(Addr+0x8);
   *RegRd3 = XSem_In32(Addr+0xC);

}

/*****************************************************************************/
/**
*
* Function to write a 128b value using 32b Microblaze
*
* @param  Address of register, data for each of the 4 32bit words to write
*
* @return VOID
*
* @note   None.
*
******************************************************************************/
void XSem_Out128(u32 Addr, u32 RegWr0, u32 RegWr1, u32 RegWr2, u32 RegWr3)
{
  XSem_Out32(Addr    , RegWr0);
  XSem_Out32(Addr+0x4, RegWr1);
  XSem_Out32(Addr+0x8, RegWr2);
  XSem_Out32(Addr+0xC, RegWr3);
}


/*****************************************************************************/
/**
 * This function dumps the registers which can help debugging
 *
 * @param
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XSem_DumpRegisters()
{
	XPlmi_Printf(DEBUG_INFO, "====SEM Registers=====\n\r");

	XPlmi_Printf(DEBUG_INFO, "CFU_ISR (Bit0:seu_endofcalib) 0x%08x\n\r",
		      Xil_In32(CFU_APB_CFU_ISR));
	XPlmi_Printf(DEBUG_INFO, "SEM_CRAM_ATTRIB: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_CRAM_ATTRIB));
	XPlmi_Printf(DEBUG_INFO, "SEM_NPI_ATTRIB: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_NPI_ATTRIB));
	XPlmi_Printf(DEBUG_INFO, "SEM_NPI_HDR_ADDR_0: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_NPI_HDR_ADDR_0));
	XPlmi_Printf(DEBUG_INFO, "SEM_NPI_HDR_ADDR_1: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_NPI_HDR_ADDR_1));
	XPlmi_Printf(DEBUG_INFO, "SEM_CLASSIFICATION_0: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_CLASSIFICATION_0));
	XPlmi_Printf(DEBUG_INFO, "SEM_CLASSIFICATION_1: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_CLASSIFICATION_1));
	XPlmi_Printf(DEBUG_INFO, "SEM_STATUS: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_STATUS));
	XPlmi_Printf(DEBUG_INFO, "SEM_ERROR_REG: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_ERROR));
	XPlmi_Printf(DEBUG_INFO, "SEM_CMD_REG0: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_CMD_REG0));
	XPlmi_Printf(DEBUG_INFO, "SEM_CMD_REG0_EXT: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_CMD_REG0_EXT));
	XPlmi_Printf(DEBUG_INFO, "SEM_CMD_REG1: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_CMD_REG1));
	XPlmi_Printf(DEBUG_INFO, "SEM_CMD_REG1_EXT: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_CMD_REG1_EXT));
	XPlmi_Printf(DEBUG_INFO, "SEM_CMD_REG2: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_CMD_REG2));
	XPlmi_Printf(DEBUG_INFO, "SEM_CMD_REG2_EXT: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_CMD_REG2_EXT));
	XPlmi_Printf(DEBUG_INFO, "SEM_CMD_REG3: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_CMD_REG3));
	XPlmi_Printf(DEBUG_INFO, "SEM_CMD_REG3_EXT: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_CMD_REG3_EXT));
	XPlmi_Printf(DEBUG_INFO, "SEM_CRAMERR_ADDRL0: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_CRAMERR_ADDRL0));
	XPlmi_Printf(DEBUG_INFO, "SEM_CRAMERR_ADDRH0: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_CRAMERR_ADDRH0));
	XPlmi_Printf(DEBUG_INFO, "SEM_CRAMERR_ADDRL1: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_CRAMERR_ADDRL1));
	XPlmi_Printf(DEBUG_INFO, "SEM_CRAMERR_ADDRH1: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_CRAMERR_ADDRH1));
	XPlmi_Printf(DEBUG_INFO, "SEM_CRAMERR_ADDRL2: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_CRAMERR_ADDRL2));
	XPlmi_Printf(DEBUG_INFO, "SEM_CRAMERR_ADDRH2: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_CRAMERR_ADDRH2));
	XPlmi_Printf(DEBUG_INFO, "SEM_CRAMERR_ADDRL3: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_CRAMERR_ADDRL3));
	XPlmi_Printf(DEBUG_INFO, "SEM_CRAMERR_ADDRH3: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_CRAMERR_ADDRH3));
	XPlmi_Printf(DEBUG_INFO, "SEM_RESERVED_0: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_RESERVED_0));
	XPlmi_Printf(DEBUG_INFO, "SEM_RESERVED_1: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_RESERVED_1));
	XPlmi_Printf(DEBUG_INFO, "SEM_RESERVED_2: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_RESERVED_2));
	XPlmi_Printf(DEBUG_INFO, "SEM_RESERVED_3: 0x%08x\n\r",
		      XSem_In32(PMC_GLOBAL_SEM_RESERVED_3));

	XPlmi_Printf(DEBUG_INFO, "====end SEM Registers====\n\r");
}

/*****************************************************************************/
/**
* This debug function prints out SEM pmc_global registers
*
* @param  None
*
* @note   None
*
******************************************************************************/
void XSem_debug_print()
{
  u32 TempReg32 = 0;

  XPlmi_Printf(DEBUG_INFO, "------------------------------------\n");

  TempReg32 = XSem_In32(PMC_GLOBAL_SEM_STATUS);
  XPlmi_Printf(DEBUG_INFO, ":: DEBUG: SEM STATUS: 0x%08x\n", TempReg32);

  TempReg32 = XSem_In32(PMC_GLOBAL_SEM_ERROR);
  XPlmi_Printf(DEBUG_INFO, ":: DEBUG: SEM ERROR: 0x%08x\n", TempReg32);

  TempReg32 = XSem_In32(PMC_GLOBAL_SEM_CRAM_ATTRIB);
  XPlmi_Printf(DEBUG_INFO, ":: DEBUG: SEM CRAM ATTRIB: 0x%08x\n", TempReg32);

  XPlmi_Printf(DEBUG_INFO, "------------------------------------\n");

}

/*****************************************************************************/
/**
* This debug function prints out CFRAME registers
*
* @param  None
*
* @note   None
*
******************************************************************************/
void XSem_cframe_debug_print()
{
  u32 TempReg32 = 0;
  /* define four registers to hold 128b of AXI read Data */
  u32 Cfr_rdReg3; /* [127: 96] */
  u32 Cfr_rdReg2; /* [ 95: 64] */
  u32 Cfr_rdReg1; /* [ 63: 32] */
  u32 Cfr_rdReg0; /* [ 31:  0] */

  XPlmi_Printf(DEBUG_INFO, "------------------------------------\n");

  XSem_In128(CFRAME0_REG_SEUOPT, &Cfr_rdReg0, &Cfr_rdReg1, &Cfr_rdReg2,
				 &Cfr_rdReg3);
  XPlmi_Printf(DEBUG_INFO, "DEBUG: CFRAME0_REG_SEUOPT 0: 0x%08x\n", Cfr_rdReg0);
  XPlmi_Printf(DEBUG_INFO, "DEBUG: CFRAME0_REG_SEUOPT 1: 0x%08x\n", Cfr_rdReg1);
  XPlmi_Printf(DEBUG_INFO, "DEBUG: CFRAME0_REG_SEUOPT 2: 0x%08x\n", Cfr_rdReg2);
  XPlmi_Printf(DEBUG_INFO, "DEBUG: CFRAME0_REG_SEUOPT 3: 0x%08x\n", Cfr_rdReg3);

  TempReg32 = (XSem_In32(CFU_APB_CFU_STATUS));
  XPlmi_Printf(DEBUG_INFO, "DEBUG: CFU_APB_CFU_STATUS: 0x%08x\n", TempReg32);

  XSem_In128(CFRAME0_REG_STATUS, &Cfr_rdReg0, &Cfr_rdReg1, &Cfr_rdReg2,
				 &Cfr_rdReg3);
  XPlmi_Printf(DEBUG_INFO, "DEBUG: CFRAME0_REG_STATUS reg3: 0x%08x, "
			   "reg2: 0x%08x, reg1: 0x%08x, reg0: 0x%08x \n", \
			   Cfr_rdReg3, Cfr_rdReg2, Cfr_rdReg1, Cfr_rdReg0);

  XSem_In128(CFRAME0_REG_STATUS+XCFRAME_FRAME_OFFSET, &Cfr_rdReg0, &Cfr_rdReg1,
						      &Cfr_rdReg2, &Cfr_rdReg3);
  XPlmi_Printf(DEBUG_INFO, "DEBUG: CFRAME1_REG_STATUS reg3: 0x%08x, "
			   "reg2: 0x%08x, reg1: 0x%08x, reg0: 0x%08x \n", \
		  Cfr_rdReg3, Cfr_rdReg2, Cfr_rdReg1, Cfr_rdReg0);

  XPlmi_Printf(DEBUG_INFO, "------------------------------------\n");

}

/*****************************************************************************/
/**
 * This function dumps the interrupt registers which can help debugging
 *
 * @param
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XSem_DumpIntRegisters()
{
	  /* define four registers to hold 128b of AXI read Data */
	  u32 Cfr_rdReg3; /* [127: 96] */
	  u32 Cfr_rdReg2; /* [ 95: 64] */
	  u32 Cfr_rdReg1; /* [ 63: 32] */
	  u32 Cfr_rdReg0; /* [ 31:  0] */

	XPlmi_Printf(DEBUG_INFO, "====CFU Register Dump============\n\r");

	XPlmi_Printf(DEBUG_INFO, "CFU_APB CFU_ISR (0:endofcalib) 0x%08x\n\r",
		      Xil_In32(CFU_APB_CFU_ISR));
	XPlmi_Printf(DEBUG_INFO, "CFU_APB CFU_IMR (0:endofcalib) 0x%08x\n\r",
		      Xil_In32(CFU_APB_CFU_IMR));
	XPlmi_Printf(DEBUG_INFO, "------------------------------------\n");
	XPlmi_Printf(DEBUG_INFO, "PMC_ERR1 Status (7:CFRAME,6:CFU): 0x%08x\n\r",
		      Xil_In32(PMC_GLOBAL_PMC_ERR1_STATUS));
	XPlmi_Printf(DEBUG_INFO, "GICP3 IRQ Status (25: CFRAME SEU, "
				 "24: CFU): 0x%08x\n\r",
				 Xil_In32(PMC_GLOBAL_GICP3_IRQ_STATUS));

	XPlmi_Printf(DEBUG_INFO, "====SEU Register Dump============\n\r");
	XSem_In128(CFRAME0_REG_CFRM_ISR, &Cfr_rdReg0, &Cfr_rdReg1, &Cfr_rdReg2,
					 &Cfr_rdReg3);
	XPlmi_Printf(DEBUG_INFO, "DEBUG: CFRAME0_REG_CFRM_ISR "
	  "(1: seu_crc_error, 0:seu_ecc_error) reg3: 0x%08x, reg2: 0x%08x, "
	  "reg1: 0x%08x, reg0: 0x%08x \n", Cfr_rdReg3, Cfr_rdReg2, Cfr_rdReg1,
	  Cfr_rdReg0);
	XSem_In128(CFRAME0_REG_CFRM_ISR+XCFRAME_FRAME_OFFSET, &Cfr_rdReg0,
	  &Cfr_rdReg1, &Cfr_rdReg2, &Cfr_rdReg3);
	XPlmi_Printf(DEBUG_INFO, "DEBUG: CFRAME1_REG_CFRM_ISR "
	  "(1: seu_crc_error, 0:seu_ecc_error) reg3: 0x%08x, reg2: 0x%08x, "
	  "reg1: 0x%08x, reg0: 0x%08x \n", Cfr_rdReg3, Cfr_rdReg2, Cfr_rdReg1,
	  Cfr_rdReg0);
	XPlmi_Printf(DEBUG_INFO, "CFU_APB CFU_IMR (0:endofcalib) 0x%08x\n\r",
		      Xil_In32(CFU_APB_CFU_IMR));
	XPlmi_Printf(DEBUG_INFO, "------------------------------------\n");
	XPlmi_Printf(DEBUG_INFO, "PMC_ERR1 Status (7:CFRAME,6:CFU): 0x%08x\n\r",
		      Xil_In32(PMC_GLOBAL_PMC_ERR1_STATUS));
	XPlmi_Printf(DEBUG_INFO, "PMC_ERR2 Status (19:CFRAME_SEU_ECC, "
	  "18:CFRAME_SEU_CRC): 0x%08x\n\r",
	  Xil_In32(PMC_GLOBAL_PMC_ERR2_STATUS));
	XPlmi_Printf(DEBUG_INFO, "GICP3 IRQ Status (25: CFRAME SEU, 24: CFU): "
	  "0x%08x\n\r", Xil_In32(PMC_GLOBAL_GICP3_IRQ_STATUS));

	XPlmi_Printf(DEBUG_INFO, "====Register Dump end============\n\r");
}
