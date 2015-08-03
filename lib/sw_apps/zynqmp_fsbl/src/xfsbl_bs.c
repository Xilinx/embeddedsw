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
 *
 *******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xfsbl_bs.c
 *
 * This file contains the definitions of bitstream loading functions.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  ba   11/17/14 Initial release
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xfsbl_hw.h"
#ifdef XFSBL_BS
#include "xfsbl_bs.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/** This function does the necessary initialization of PCAP interface
 *
 * @param	None
 *
 * @return	error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
u32 XFsbl_PcapInit(void) {
	u32 RegVal;
	u32 Status = XFSBL_SUCCESS;

	/* Take PCAP out of Reset */
	RegVal = XFsbl_In32(CSU_PCAP_RESET);
	RegVal &= (~CSU_PCAP_RESET_RESET_MASK);
	XFsbl_Out32(CSU_PCAP_RESET, RegVal);

	/* Select PCAP mode and change PCAP to write mode */
	RegVal = CSU_PCAP_CTRL_PCAP_PR_MASK;
	XFsbl_Out32(CSU_PCAP_CTRL, RegVal);
	XFsbl_Out32(CSU_PCAP_RDWR, 0x0);

	Status = XFsbl_PowerUpIsland(PMU_GLOBAL_PWR_STATE_PL_MASK);

	if (Status != XFSBL_SUCCESS) {
		Status = XFSBL_ERROR_PL_POWER_UP;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_PL_POWER_UP\r\n");
			goto END;
	}

	/* Reset PL */
	XFsbl_Out32(CSU_PCAP_PROG, 0x0U);

	usleep(PL_RESET_PERIOD_IN_US);

	XFsbl_Out32(CSU_PCAP_PROG, CSU_PCAP_PROG_PCFG_PROG_B_MASK);

	/*
	 *  Wait for PL_init completion
	 *  Bypass this check in platforms not supporting PCAP interface
	 */
	if ((XFSBL_PLATFORM != XFSBL_PLATFORM_REMUS)
			&& (XFSBL_PLATFORM != XFSBL_PLATFORM_QEMU)) {
		RegVal = 0U;
		do {
			RegVal = XFsbl_In32(CSU_PCAP_STATUS) &
			CSU_PCAP_STATUS_PL_INIT_MASK;
		} while (RegVal != CSU_PCAP_STATUS_PL_INIT_MASK);
	} else {
		XFsbl_Printf(DEBUG_GENERAL,
				"PCAP interface is not supported in this platform \r\n");
	}
END:
	return Status;
}

/*****************************************************************************/
/** This function waits for PCAP transfer to complete
 *
 * @param	None
 *
 * @return	error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
static u32 XFsbl_PcapWaitForDone() {
	u32 RegVal;
	u32 Status = XFSBL_SUCCESS;

	do {
		RegVal = XFsbl_In32(CSU_PCAP_STATUS);
		RegVal = RegVal & CSU_PCAP_STATUS_PCAP_WR_IDLE_MASK;
	} while (RegVal != CSU_PCAP_STATUS_PCAP_WR_IDLE_MASK);

	return Status;
}

/*****************************************************************************/
/** This is the function to write data to PCAP interface
 *
 * @param	WrSize: Number of 32bit words that the DMA should write to
 *          the PCAP interface
 * @param   WrAddr: Linear memory space from where CSUDMA will read
 *	        the data to be written to PCAP interface
 *
 * @return	None
 *
 *****************************************************************************/
u32 XFsbl_WriteToPcap(u32 WrSize, u8 *WrAddr) {
	u32 RegVal;
	u32 Status = XFSBL_SUCCESS;

	/*
	 * Setup the  SSS, setup the PCAP to receive from DMA source
	 */
	RegVal = XFsbl_In32(CSU_CSU_SSS_CFG) & CSU_CSU_SSS_CFG_PCAP_SSS_MASK;
	RegVal = RegVal
			| (XFSBL_CSU_SSS_SRC_SRC_DMA << CSU_CSU_SSS_CFG_PCAP_SSS_SHIFT);
	XFsbl_Out32(CSU_CSU_SSS_CFG, RegVal);

	/* Setup the source DMA channel */
	XCsuDma_Transfer(&CsuDma, XCSUDMA_SRC_CHANNEL, (PTRSIZE) WrAddr, WrSize, 0);

	/* wait for the SRC_DMA to complete and the pcap to be IDLE */
	XCsuDma_WaitForDone(&CsuDma, XCSUDMA_SRC_CHANNEL);

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_SRC_CHANNEL, XCSUDMA_IXR_DONE_MASK);

	XFsbl_Printf(DEBUG_GENERAL, "DMA transfer done \r\n");
	Status = XFsbl_PcapWaitForDone();
	if (Status != XFSBL_SUCCESS) {
		goto END;
	}

	END: return Status;
}

/*****************************************************************************/
/**
 * This function waits for PL Done bit to be set or till timeout and resets
 * PCAP after this.
 *
 * @param	None
 *
 * @return	error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
u32 XFsbl_PLWaitForDone(void) {
	u32 Status = XFSBL_SUCCESS;
	u32 PollCount;
	u32 RegVal;

	PollCount = (PL_DONE_POLL_COUNT);
	while (PollCount) {
		/* Read PCAP Status register and check for PL_DONE bit */
		RegVal = XFsbl_In32(CSU_PCAP_STATUS);
		RegVal &= CSU_PCAP_STATUS_PL_DONE_MASK;
		if (RegVal == CSU_PCAP_STATUS_PL_DONE_MASK) {
			break;
		}
		PollCount--;
	}

	if (RegVal == CSU_PCAP_STATUS_PL_DONE_MASK) {
		XFsbl_Printf(DEBUG_GENERAL, "PL Configuration done successfully \r\n");
	} else {
		Status = XFSBL_ERROR_BITSTREAM_LOAD_FAIL;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_BITSTREAM_LOAD_FAIL\r\n");
		goto END;
	}

	/* Reset PCAP after data transfer */
	RegVal = XFsbl_In32(CSU_PCAP_RESET);
	RegVal = RegVal | CSU_PCAP_RESET_RESET_MASK;
	XFsbl_Out32(CSU_PCAP_RESET, RegVal);

	do {
		RegVal = XFsbl_In32(CSU_PCAP_RESET);
		RegVal = RegVal & CSU_PCAP_RESET_RESET_MASK;
	} while (RegVal != CSU_PCAP_RESET_RESET_MASK);

	END:
	return Status;
}

#endif
