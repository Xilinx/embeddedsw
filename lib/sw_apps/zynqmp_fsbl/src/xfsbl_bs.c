/******************************************************************************
 *
 * Copyright (C) 2015 - 17 Xilinx, Inc.  All rights reserved.
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
 * 2.0   bv   12/05/16 Made compliance to MISRAC 2012 guidelines
 *                     Chunk for bitstream is been storing at bitstream_buffer
 *                     section
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
/* Global OCM buffer to store data chunks */
u8 ReadBuffer[READ_BUFFER_SIZE]__attribute__ ((aligned (64)))
				__attribute__((section (".bitstream_buffer")));

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
	u32 Status;
	u32 PlatInfo;
	(void)memset(ReadBuffer, 0U, sizeof(ReadBuffer));

	/* Take PCAP out of Reset */
	RegVal = XFsbl_In32(CSU_PCAP_RESET);
	RegVal &= (~CSU_PCAP_RESET_RESET_MASK);
	XFsbl_Out32(CSU_PCAP_RESET, RegVal);

	/* Select PCAP mode and change PCAP to write mode */
	RegVal = CSU_PCAP_CTRL_PCAP_PR_MASK;
	XFsbl_Out32(CSU_PCAP_CTRL, RegVal);
	XFsbl_Out32(CSU_PCAP_RDWR, 0x0);

	/**
	 * For 1.0 and 2.0 Silicon, PL should already be powered up
	 * (before MIO config). Hence, skip that step here.
	 */
	if (XGetPSVersion_Info() > (u32)XPS_VERSION_2) {
		Status = XFsbl_PowerUpIsland(PMU_GLOBAL_PWR_STATE_PL_MASK);

		if (Status != XFSBL_SUCCESS) {
			Status = XFSBL_ERROR_PL_POWER_UP;
			XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_PL_POWER_UP\r\n");
			goto END;
		}

		usleep(XFSBL_PL_PWRUP_WAIT_MICROSEC);

		Status = XFsbl_IsolationRestore(PMU_GLOBAL_REQ_ISO_INT_EN_PL_NONPCAP_MASK);

		if (Status != XFSBL_SUCCESS) {
			Status = XFSBL_ERROR_PMU_GLOBAL_REQ_ISO;
			XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_PMU_GLOBAL_REQ_ISO\r\n");
			goto END;
		}
	}

	/* Reset PL */
	XFsbl_Out32(CSU_PCAP_PROG, 0x0U);

	(void)usleep(PL_RESET_PERIOD_IN_US);

	XFsbl_Out32(CSU_PCAP_PROG, CSU_PCAP_PROG_PCFG_PROG_B_MASK);

	/*
	 *  Wait for PL_init completion
	 *  Bypass this check in platforms not supporting PCAP interface
	 */

	PlatInfo = XGet_Zynq_UltraMp_Platform_info();

	if ((PlatInfo != (u32)XPLAT_ZYNQ_ULTRA_MP)
			&& (PlatInfo != (u32)XPLAT_ZYNQ_ULTRA_MPQEMU)) {
		/* To confirm house cleaning of PL is started */
		do {
			RegVal = XFsbl_In32(CSU_PCAP_STATUS) &
				CSU_PCAP_STATUS_PL_CFG_RST_MASK;
		} while (RegVal != CSU_PCAP_STATUS_PL_CFG_RST_MASK);
		/* To confirm house cleaning of PL is done */
		do {
			RegVal = XFsbl_In32(CSU_PCAP_STATUS) &
			CSU_PCAP_STATUS_PL_INIT_MASK;
		} while (RegVal != CSU_PCAP_STATUS_PL_INIT_MASK);
	} else {
		XFsbl_Printf(DEBUG_GENERAL,
				"PCAP interface is not supported in this platform \r\n");
	}
	Status = XFSBL_SUCCESS;
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
u32 XFsbl_PcapWaitForDone(void) {
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
	u32 Status;

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
	XCsuDma_WaitForDone(&CsuDma, XCSUDMA_SRC_CHANNEL){}

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_SRC_CHANNEL, XCSUDMA_IXR_DONE_MASK);

	XFsbl_Printf(DEBUG_INFO, "DMA transfer done \r\n");
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
	u32 Status;
	u32 PollCount;
	u32 RegVal = 0U;

	PollCount = PL_DONE_POLL_COUNT;
	while (PollCount > 0U) {
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

	Status = XFSBL_SUCCESS;
	END:
	return Status;
}

/*****************************************************************************/
/** This is the function to download nonsebitstream to PL using chunking.
 *
 * @param	None
 *
 * @return	error status based on implemented functionality(SUCCESS by default)
 *
 *****************************************************************************/
#ifndef XFSBL_PS_DDR
u32 XFsbl_ChunkedBSTxfer(XFsblPs *FsblInstancePtr, u32 PartitionNum)
{
	u32 Status = XFSBL_SUCCESS;
	XFsblPs_PartitionHeader *PartitionHeader;
	u32 NumChunks = 0U;
	u32 RemainingBytes = 0U;
	u32 Index = 0U;
	u32 BitStreamSizeWord = 0U;
	u32 BitStreamSizeByte = 0U;
	u32 ImageOffset = 0U;
	u32 StartAddrByte = 0U;

	XFsbl_Printf(DEBUG_GENERAL,
		"Nonsecure Bitstream transfer in chunks to begin now\r\n");

	/**
	 * Assign the partition header to local variable
	 */
	PartitionHeader =
		&FsblInstancePtr->ImageHeader.PartitionHeader[PartitionNum];

	BitStreamSizeWord = PartitionHeader->UnEncryptedDataWordLength;
	ImageOffset = FsblInstancePtr->ImageOffsetAddress;

	StartAddrByte = ImageOffset + 4*(PartitionHeader->DataWordOffset);

	XFsbl_Printf(DEBUG_INFO,
			"Nonsecure Bitstream to be copied from %0x \r\n",
			StartAddrByte);

	/* Converting size in words to bytes */
	BitStreamSizeByte = BitStreamSizeWord*4;

	NumChunks = BitStreamSizeByte / READ_BUFFER_SIZE;
	RemainingBytes = BitStreamSizeByte % READ_BUFFER_SIZE;

	for(Index = 0; Index < NumChunks; Index++)
	{
		Status = FsblInstancePtr->DeviceOps.DeviceCopy(StartAddrByte,
				(PTRSIZE)ReadBuffer, READ_BUFFER_SIZE);

		if (XFSBL_SUCCESS != Status)
		{
			XFsbl_Printf(DEBUG_GENERAL,
				"Copy of chunk from flash to OCM failed \r\n");
			goto END;
		}

		Status = XFsbl_WriteToPcap((READ_BUFFER_SIZE/4), &ReadBuffer[0]);
		if (XFSBL_SUCCESS != Status)
		{
			goto END;
		}

		StartAddrByte += READ_BUFFER_SIZE;
	}

	if(RemainingBytes != 0U)
	{
		Status = FsblInstancePtr->DeviceOps.DeviceCopy(StartAddrByte,
					(PTRSIZE)ReadBuffer, RemainingBytes);
		if (XFSBL_SUCCESS != Status)
		{
			XFsbl_Printf(DEBUG_GENERAL,
				"Copy of chunk from flash to OCM failed \r\n");
			goto END;
		}

		Status = XFsbl_WriteToPcap((RemainingBytes/4), &ReadBuffer[0]);
	}

END:
	return Status;
}
#endif

#endif
/*****************************************************************************/
/**
 * This function checks for PL Done bit to be set and if set, resets
 * PCAP after this.
 *
 * @param	None
 *
 * @return	error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
u32 XFsbl_PLCheckForDone(void) {
	u32 Status;
	u32 RegVal = 0U;

	/* Read PCAP Status register and check for PL_DONE bit */
	RegVal = XFsbl_In32(CSU_PCAP_STATUS);
	RegVal &= CSU_PCAP_STATUS_PL_DONE_MASK;
	if (RegVal != CSU_PCAP_STATUS_PL_DONE_MASK) {
		Status = XFSBL_BITSTREAM_NOT_LOADED;
		goto END;
	}

	XFsbl_Printf(DEBUG_GENERAL, "PL Configuration done successfully \r\n");

	/* Reset PCAP after data transfer */
	RegVal = XFsbl_In32(CSU_PCAP_RESET);
	RegVal = RegVal | CSU_PCAP_RESET_RESET_MASK;
	XFsbl_Out32(CSU_PCAP_RESET, RegVal);

	do {
		RegVal = XFsbl_In32(CSU_PCAP_RESET);
		RegVal = RegVal & CSU_PCAP_RESET_RESET_MASK;
	} while (RegVal != CSU_PCAP_RESET_RESET_MASK);

	Status = XFSBL_SUCCESS;
	END:
	return Status;
}
