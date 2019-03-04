/******************************************************************************
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
* @file xsem_cfr_init.c
*  Initialize XilSem CFRAME to known state, enable SEU interrupts,
* and start CFRAME scan checksum compute
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date        Changes
* ----  ----     --------    --------------------------------------------------
* 0.1   mw       06/26/2018  Initial creation
* 0.2   pc       09/10/2018  Updated cframe/cfu reg wr sequence to align with
*                            latest documentation
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsem_cfr_init.h"

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************
*
* This initialization function starts up SEM and CFRAME scan from a clean slate
* - clears SEM error/status registers (does not touch SEM cramerr registers)
* - (re-)runs CFRAME calibration and scan start
*
* @param  *ErrInfoPtr - pointer to error info
*
* @return XST_FAILURE - on Init fail
*     XST_SUCCESS - on Init success
*
* @note   None
*
******************************************************************************/
u32 XSem_CfrInit(void)
{
  u32 RetStatus = XST_FAILURE;
  u32 TempVal32 = 0;
  u32 TempSet32 = 0;
  u32 TempClr32 = 0;
  u32 sem_attrib_reg = 0;
  u32 sw_ecc_sel;

  /* define four registers to hold 128b of AXI read Data */
  u32 rdReg3; /* [127: 96] */
  u32 rdReg2; /* [ 95: 64] */
  u32 rdReg1; /* [ 63: 32] */
  u32 rdReg0; /* [ 31:  0] */

  /* Print start of formal customer log */
  XPlmi_Printf(DEBUG_GENERAL, "XSEM CFRInit start\n\r");

  /* TODO: align this err info structure with STLs */
  /*  Clear the Error info structure */
  /*  XSEM_CLEAR(ErrInfoPtr);        */

  /**********************************/
  /* Quick exit if we shouldn't run */
  /**********************************/

  /* TODO add some basic checks
   * - PLM integration question: if customer configs SEM to postpone init until
   *    after startup, does PLM have an ifdef to control call to SEM init?
   * - if SEM is in FATAL state, should we allow init to clear it? or just exit
   */

  /*********************************/
  /* Update SEM CFRAME Status Bits */
  /*********************************/

  /* Clear CRAM Status bits to 0 */
  /* Set CRAM_INIT_STATE and CRAM_FUNC_ACTIVE status bits to 1 */
  TempClr32 = (u32)(PMC_GLOBAL_SEM_STATUS_CRAM_INIT_DONE_MASK |
	             PMC_GLOBAL_SEM_STATUS_CRAM_FATAL_STATE_MASK |
	             PMC_GLOBAL_SEM_STATUS_CRAM_INJ_STATE_MASK |
	             PMC_GLOBAL_SEM_STATUS_CRAM_IDLE_STATE_MASK |
	             PMC_GLOBAL_SEM_STATUS_CRAM_DETECT_STATE_MASK |
	             PMC_GLOBAL_SEM_STATUS_CRAM_CLASS_STATE_MASK |
	             PMC_GLOBAL_SEM_STATUS_CRAM_CORR_STATE_MASK |
	             PMC_GLOBAL_SEM_STATUS_CRAM_OBS_STATE_MASK |
	             PMC_GLOBAL_SEM_STATUS_CRAM_SCANCHK_STATE_MASK |
	             PMC_GLOBAL_SEM_STATUS_CRAM_INIT_STATE_MASK |
	             PMC_GLOBAL_SEM_STATUS_CRAM_FUNC_ACTIVE_MASK);
  TempSet32 = (u32) (PMC_GLOBAL_SEM_STATUS_CRAM_INIT_STATE_MASK |
	             PMC_GLOBAL_SEM_STATUS_CRAM_FUNC_ACTIVE_MASK);
  TempVal32 = XSem_In32(PMC_GLOBAL_SEM_STATUS);
  TempVal32 = (TempVal32 & ~TempClr32) | TempSet32;
  XSem_Out32(PMC_GLOBAL_SEM_STATUS, TempVal32);

  /*********************************/
  /* Clear SEM CFRAME Error Bits   */
  /*********************************/

  /* Clear CRAM Error bits to 0 */
  TempClr32 = (u32)(PMC_GLOBAL_SEM_ERROR_CRAM_HRT_ERR_MASK |
	             PMC_GLOBAL_SEM_ERROR_CRAM_COR_ERR_CNT_MASK |
	             PMC_GLOBAL_SEM_ERROR_CRAM_RES_MASK |
	             PMC_GLOBAL_SEM_ERROR_CRAM_COR_ERR_MASK |
	             PMC_GLOBAL_SEM_ERROR_CRAM_CRC_ERR_MASK |
	             PMC_GLOBAL_SEM_ERROR_CRAM_UNCOR_ERR_MASK |
	             PMC_GLOBAL_SEM_ERROR_CRAM_INIT_ERR_MASK  );
  TempVal32 = XSem_In32(PMC_GLOBAL_SEM_ERROR);
  XSem_Out32(PMC_GLOBAL_SEM_ERROR, (TempVal32 & ~TempClr32));

  /* TODO: Clear CRAM ERRORCODE bits. Reg definition TBD */

  /**********************************/
  /* Clear SEM EMIO GPIO Error Bits */
  /**********************************/

  // TODO: clear GPIO

  /*********************************/
  /* Initial pre-calibration work  */
  /*********************************/
  /*
   *  Enable all rows.                   CFRAME_BCAST_REG/CMD/ROWON  = 1
   *  (temporary workaround) run HCLEAN. CFRAME_BCAST_REG/CMD/HCLEAN = 1
   *  Stop scan (in case it is on)       CFU_APB/CFU_CTL/SEU_GO = 0
   *  Enable interrupts for endofcalib   CFU_APB/CFU_IER/SEU_ENDOFCALIB = 1
   *                                     CFU_APB/CFU_ISR/SEU_ENDOFCALIB = 0
   *                                     PMC_GLOBAL/GICP3_IRQ_ENABLE/CFU = 1
   *                                     PMC_GLOBAL/GICP3_IRQ_STATUS  = 1
   *                                     PMC_GLOBAL/GICP_PMC_IRQ_ENABLE/src3 = 1
   *                                     PMC_GLOBAL/GICP_PMC_IRQ_STATUS  = 1
   *  Enable interrupts for seu          CFRAME_BCAST_REG/CFRM_IER/SEU*ERROR = 1
   *                                     CFRAME_BCAST_REG/CFRM_ISR/SEU*ERROR = 0
   **********************************/

   // Set CFRAME_BCAST_REG_CMD ROWON  (write 0x2 to CMD reg, all rows)
   XSem_Out128(CFRAME_BCAST_REG_CMD, 0x00000002, 0x00000000, 0x00000000,
				     0x00000000);

   // TODO: remove before release (PLM will do HCLEAN at startup)
   // Set CFRAME_BCAST_REG_CMD HCLEAN (write 0x7 to CMD reg, all rows)
   XSem_Out128(CFRAME_BCAST_REG_CMD, 0x00000007, 0x00000000, 0x00000000,
				     0x00000000);

   // Clear seu_go to stop scan (if running)
   XSem_Out32(CFU_APB_CFU_MASK, 0x00000002U);
   XSem_Out32(CFU_APB_CFU_CTL,  0x00000000U);

   //TODO: move this to another function that is run once at device startup
   // enable/clear SEU_ENDOFCALIB interrupt tree
   XSem_Out32(CFU_APB_CFU_IER, CFU_APB_CFU_IER_SEU_ENDOFCALIB_MASK);
   XSem_Out32(CFU_APB_CFU_ISR, CFU_APB_CFU_ISR_SEU_ENDOFCALIB_MASK);
   XSem_Out32(PMC_GLOBAL_GICP3_IRQ_ENABLE,
     PMC_GLOBAL_GICP3_IRQ_ENABLE_SRC24_MASK);
   XSem_Out32(PMC_GLOBAL_GICP3_IRQ_STATUS,
     PMC_GLOBAL_GICP3_IRQ_STATUS_SRC24_MASK);
   XSem_Out32(PMC_GLOBAL_GICP_PMC_IRQ_ENABLE,
     PMC_GLOBAL_GICP_PMC_IRQ_ENABLE_SRC3_MASK);
   XSem_Out32(PMC_GLOBAL_GICP_PMC_IRQ_STATUS,
     PMC_GLOBAL_GICP_PMC_IRQ_STATUS_SRC3_MASK);
/* TODO: for VNC, disable seu_endofcalib IRQ until interrupt handler is ready
 * TODO: if these interrupts are all enabled in xpmcfw_main by
 *        XPmcFw_SetUpInterruptSystem, then we can remove this tree enablement
 * XSem_Out32(PPU1_IOMODULE_IRQ_ENABLE, PPU1_IOMODULE_IRQ_ACK_PMC_GIC_IRQ_MASK);
 * XSem_Out32(PPU1_IOMODULE_IRQ_ACK, PPU1_IOMODULE_IRQ_ACK_PMC_GIC_IRQ_MASK);
 */

   // enable/clear SEU_CRC_ERROR and SEU_ECC_ERROR interrupts for all rows
   XSem_Out128(CFRAME_BCAST_REG_CFRM_IER, 0x00000003, 0x00000000, 0x00000000,
					  0x00000000);
   XSem_Out128(CFRAME_BCAST_REG_CFRM_ISR, 0x00000003, 0x00000000, 0x00000000,
					  0x00000000);

  /*********************************/
  /* Run SEU calibration           */
  /*********************************/
  /*  Set-up calibration: write SEUOPT    CFRAME_BCAST_REG/SEUOPT/
   *                                      HB_CNT = 0
   *                                      NOCRCC = 1 (leave as 1 all the time)
   *                                      RBCRC_ONEROW = 1
   *                                      SWECC_SEL = <user choice> default: 0
   *                                      SWCRC_SEL = 0
   *                                      RBCRC_EN = 1
   *  Send CRCC (re-calc golden readback CRC at init and after PR)
   *  Fake cframe read to avoid race cond CFRAME_0_REG/STATUS
   *  Kick off calibration/scan           CFU_APB/CFU_CTL/SEU_GO = 1
   *
   *********************************/

  TempVal32 = (XSem_In32(PMC_GLOBAL_SEM_CRAM_ATTRIB)) &&
	       PMC_GLOBAL_SEM_CRAM_ATTRIB_SWECC_SEL_MASK;
  if (TempVal32 == PMC_GLOBAL_SEM_CRAM_ATTRIB_SWECC_SEL_MASK) {
    // select SWECC calibration
    XSem_Out128(CFRAME_BCAST_REG_SEUOPT, 0x0000001D, 0x00000000, 0x00000000,
					 0x00000000);
  } else {
    // select HWECC calibration
    XSem_Out128(CFRAME_BCAST_REG_SEUOPT, 0x00000019, 0x00000000, 0x00000000,
					 0x00000000);
  }

  /* broadcast CRCC command (CMD/CRCC) */
  XSem_Out128(CFRAME_BCAST_REG_CMD, 0x00000006, 0x00000000, 0x00000000,
				    0x00000000);
  /* fake read after cframe and before cfu access. discard result */
  XSem_In128(CFRAME0_REG_STATUS, &rdReg0, &rdReg1, &rdReg2, &rdReg3);

  /* set SEU_GO  */
  XSem_Out32(CFU_APB_CFU_MASK, 0x00000002U);
  XSem_Out32(CFU_APB_CFU_CTL, 0x00000002U);


  /*********************************/
  /* Wait for SEU_ENDOFCALIB       */
  /*********************************/

  // TODO : replace this seu_endofcalib polling with interrupt handler
  do
  {
    TempVal32 = (XSem_In32(CFU_APB_CFU_ISR));
    XPlmi_Printf(DEBUG_DETAILED, "DEBUG: CFU_APB_CFU_ISR: Address: 0x%08x; \
      Data: 0x%08x\n", CFU_APB_CFU_ISR, TempVal32);
    TempVal32 &= ((u32)(CFU_APB_CFU_ISR_SEU_ENDOFCALIB_MASK));
  } while (TempVal32 == 0x00000000U);


  /***************************************************/
  /* Calibration done. Update SEM CFRAME Status Bits */
  /***************************************************/

  XSem_SetBit32(PMC_GLOBAL_SEM_STATUS,
    PMC_GLOBAL_SEM_STATUS_CRAM_INIT_DONE_MASK);
  XSem_ClrBit32(PMC_GLOBAL_SEM_STATUS,
    PMC_GLOBAL_SEM_STATUS_CRAM_INIT_STATE_MASK);
  XSem_SetBit32(PMC_GLOBAL_SEM_STATUS,
    PMC_GLOBAL_SEM_STATUS_CRAM_OBS_STATE_MASK);

  // Print end of formal customer log
  XPlmi_Printf(DEBUG_GENERAL, "XSEM CFRInit Done\n");
  RetStatus = XSEM_CFR_INIT_SEM_CRAM_SUCCESS;
  return RetStatus;

  }
