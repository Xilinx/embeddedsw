/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
*******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xilfpga_pcap.c
 *
 * This file contains the definitions of bitstream loading functions.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   Nava  08/06/16 Initial release
 * 1.1   Nava  16/11/16 Added PL power-up sequence.
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_io.h"
#include "xil_types.h"
#include "xcsudma.h"
#include "sleep.h"
#include "xil_printf.h"
#include "xilfpga_pcap.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/
#ifdef __MICROBLAZE__
#define XPBR_SERV_EXT_PWRUPPLD		119
#define XPBR_SERV_EXT_PLNONPCAPISO	162
#define XPBR_SERV_EXT_TBL_MAX		256
#endif

#ifdef XPAR_NUM_FABRIC_RESETS
#define FPGA_NUM_FABRIC_RESETS	XPAR_NUM_FABRIC_RESETS
#else
#define FPGA_NUM_FABRIC_RESETS	1
#endif

#define MAX_REG_BITS	31

/**************************** Type Definitions *******************************/
#ifdef __MICROBLAZE__
typedef u32 (*XpbrServHndlr_t) (void);
#endif
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static u32 XFpga_PcapWaitForDone();
static u32 XFpga_WriteToPcap(u32 WrSize, u32 WrAddrHigh, u32 WrAddrLow);
static u32 XFpga_PcapInit(void);
static u32 XFpga_CsuDmaInit();
static u32 XFpga_PLWaitForDone(void);
static u32 XFpga_PowerUpPl(void);
static u32 XFpga_IsolationRestore(void);
static u32 XFpga_PsPlGpioReset(u32 TotalResets);
#ifdef __MICROBLAZE__
extern const XpbrServHndlr_t XpbrServHndlrTbl[XPBR_SERV_EXT_TBL_MAX];
#endif
/************************** Variable Definitions *****************************/
XCsuDma CsuDma;

/*****************************************************************************/

/*****************************************************************************/
/** This function does the calls the necessary PCAP interfaces based on flags.
 *
 *@param	WrAddrHigh: Higher 32-bit Linear memory space from where CSUDMA
 *		will read the data to be written to PCAP interface
 *
 *@param        WrAddrLow: Lower 32-bit Linear memory space from where CSUDMA
 *              will read the data to be written to PCAP interface
 *
 *@param        WrSize: Number of 32bit words that the DMA should write to
 *              the PCAP interface
 *
 *@param	flags:
 *		BIT(0) - Bit-stream type.
 *			 0 - Full Bit-stream.
 *			 1 - Partial Bit-stream.
 *		BIT(1) - Authentication.
 *			 1 - Enable.
 *		 	 0 - Disable.
 *		BIT(2) - Encryption.
 *			 1 - Enable.
 *			 0 - Disable.
 * NOTE -
 *	The current implementation supports only Full Bit-stream.
 *
 *@return	error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
u32 XFpga_PL_BitSream_Load (u32 WrAddrHigh, u32 WrAddrLow,
				u32 WrSize, u32 flags )
{
	u32 Status = XFPGA_SUCCESS;

	/* Power-Up PL */
	Status = XFpga_PowerUpPl();
	if (Status != XFPGA_SUCCESS) {
		xil_printf("XFPGA_ERROR_PL_POWER_UP\r\n");
		Status = XFPGA_ERROR_PL_POWER_UP;
		goto END;
	}

	/* PS PL Isolation Restore */
	Status = XFpga_IsolationRestore();
	if (Status != XFPGA_SUCCESS) {
		xil_printf("XFPGA_ERROR_PL_ISOLATION\r\n");
		Status = XFPGA_ERROR_PL_ISOLATION;
		goto END;
	}

	Status = XFpga_PcapInit();
	if(Status != XFPGA_SUCCESS) {
		xil_printf("FPGA Init fail\n");
		goto END;
	}
	Status = XFpga_WriteToPcap(WrSize, WrAddrHigh, WrAddrLow);
	if(Status != XFPGA_SUCCESS) {
		xil_printf("FPGA fail to write Bit-stream into PL\n");
		goto END;
	}
	Status = XFpga_PLWaitForDone();
	if(Status != XFPGA_SUCCESS) {
		xil_printf("FPGA fail to get the done status\n");
		goto END;
	}

	/* Power-Up PL */
	Status = XFpga_PowerUpPl();
	if (Status != XFPGA_SUCCESS) {
		xil_printf("XFPGA_ERROR_PL_POWER_UP\r\n");
		Status = XFPGA_ERROR_PL_POWER_UP;
		goto END;
	}

	/* PS-PL reset */
	XFpga_PsPlGpioReset(FPGA_NUM_FABRIC_RESETS);

	END:
	return Status;
}
/*****************************************************************************/
/** This function does the necessary initialization of PCAP interface
 *
 * @param	None
 *
 * @return	error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
static u32 XFpga_PcapInit(void) {
	u32 RegVal;
	u32 PollCount;
	u32 Status = XFPGA_SUCCESS;


	/* Take PCAP out of Reset */
	RegVal = Xil_In32(CSU_PCAP_RESET);
	RegVal &= (~CSU_PCAP_RESET_RESET_MASK);
	Xil_Out32(CSU_PCAP_RESET, RegVal);

	/* Select PCAP mode and change PCAP to write mode */
	RegVal = CSU_PCAP_CTRL_PCAP_PR_MASK;
	Xil_Out32(CSU_PCAP_CTRL, RegVal);
	Xil_Out32(CSU_PCAP_RDWR, 0x0);

	/* Reset PL */
	Xil_Out32(CSU_PCAP_PROG, 0x0U);

	usleep(PL_RESET_PERIOD_IN_US);

	Xil_Out32(CSU_PCAP_PROG, CSU_PCAP_PROG_PCFG_PROG_B_MASK);
	/*
	 *  Wait for PL_init completion
	 */
	RegVal = 0U;
	PollCount = (PL_DONE_POLL_COUNT);
	while (PollCount) {
		RegVal = Xil_In32(CSU_PCAP_STATUS) &
		CSU_PCAP_STATUS_PL_INIT_MASK;
		if (RegVal == CSU_PCAP_STATUS_PL_INIT_MASK)
			break;
		PollCount--;
	}
	Status = XFpga_CsuDmaInit();
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
static u32 XFpga_PcapWaitForDone() {
	u32 RegVal = 0U;
	u32 PollCount;
	u32 Status = XFPGA_SUCCESS;

	PollCount = (PL_DONE_POLL_COUNT);
	while(PollCount) {
		RegVal = Xil_In32(CSU_PCAP_STATUS);
		RegVal = RegVal & CSU_PCAP_STATUS_PCAP_WR_IDLE_MASK;
		if (RegVal == CSU_PCAP_STATUS_PCAP_WR_IDLE_MASK)
			break;
		PollCount--;
	}
	return Status;
}

/*****************************************************************************/
/** This is the function to write data to PCAP interface
 *
 * @param	WrSize: Number of 32bit words that the DMA should write to
 *         	the PCAP interface
 *
 * @param       WrAddrHigh: Higher 32-bit Linear memory space from where CSUDMA
 *              will read the data to be written to PCAP interfacae
 *
 * @param       WrAddrLow: Lower 32-bit Linear memory space from where CSUDMA
 *              will read the data to be written to PCAP interface
 *
 * @return	None
 *
 *****************************************************************************/
static u32 XFpga_WriteToPcap(u32 WrSize, u32 WrAddrHigh, u32 WrAddrLow) {
	u32 RegVal;
	u32 Status = XFPGA_SUCCESS;
	u64 WrAddr;

	WrAddr = ((u64)WrAddrHigh << 32)|WrAddrLow;
	/*
	 * Setup the  SSS, setup the PCAP to receive from DMA source
	 */
	RegVal = Xil_In32(CSU_CSU_SSS_CFG) & CSU_CSU_SSS_CFG_PCAP_SSS_MASK;
	RegVal = RegVal
			| (XFPGA_CSU_SSS_SRC_SRC_DMA << CSU_CSU_SSS_CFG_PCAP_SSS_SHIFT);
	Xil_Out32(CSU_CSU_SSS_CFG, RegVal);

	/* Setup the source DMA channel */
	XCsuDma_Transfer(&CsuDma, XCSUDMA_SRC_CHANNEL, WrAddr, WrSize, 0);

	/* wait for the SRC_DMA to complete and the pcap to be IDLE */
	XCsuDma_WaitForDone(&CsuDma, XCSUDMA_SRC_CHANNEL);

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_SRC_CHANNEL, XCSUDMA_IXR_DONE_MASK);

	Status = XFpga_PcapWaitForDone();

	return Status;
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
static u32 XFpga_PLWaitForDone(void) {
	u32 Status = XFPGA_SUCCESS;
	u32 PollCount;
	u32 RegVal = 0U;

	PollCount = (PL_DONE_POLL_COUNT);
	while (PollCount) {
		/* Read PCAP Status register and check for PL_DONE bit */
		RegVal = Xil_In32(CSU_PCAP_STATUS);
		RegVal &= CSU_PCAP_STATUS_PL_DONE_MASK;
		if (RegVal == CSU_PCAP_STATUS_PL_DONE_MASK) {
			break;
		}
		PollCount--;
	}

	if (RegVal != CSU_PCAP_STATUS_PL_DONE_MASK) {
		Status = XFPGA_ERROR_BITSTREAM_LOAD_FAIL;
		goto END;
	}

	/* Reset PCAP after data transfer */
	RegVal = Xil_In32(CSU_PCAP_RESET);
	RegVal = RegVal | CSU_PCAP_RESET_RESET_MASK;
	Xil_Out32(CSU_PCAP_RESET, RegVal);

	PollCount = (PL_DONE_POLL_COUNT);
	RegVal = 0U;
	while(PollCount) {
		RegVal = Xil_In32(CSU_PCAP_RESET);
		RegVal = RegVal & CSU_PCAP_RESET_RESET_MASK;
		if (RegVal == CSU_PCAP_RESET_RESET_MASK)
			break;
		PollCount--;
	}

	END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to initialize the DMA driver
 *
 * @param	None
 *
 * @return	error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
static u32 XFpga_CsuDmaInit()
{
	u32 Status = XFPGA_SUCCESS;
	XCsuDma_Config * CsuDmaConfig;

	CsuDmaConfig = XCsuDma_LookupConfig(0);
	if (NULL == CsuDmaConfig) {
		Status = XFPGA_ERROR_CSUDMA_INIT_FAIL;
		goto END;
	}

	Status = XCsuDma_CfgInitialize(&CsuDma, CsuDmaConfig,
			CsuDmaConfig->BaseAddress);
	if (Status != XFPGA_SUCCESS) {
		Status = XFPGA_ERROR_CSUDMA_INIT_FAIL;
		goto END;
	}
END:
	return Status;
}
/*****************************************************************************/
/**
 * This function is used to power-up the PL
 *
 * @param	None
 *
 * @return	error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
static u32 XFpga_PowerUpPl(void) {

	u32 Status = XFPGA_SUCCESS;

#ifdef __MICROBLAZE__
	Status = XpbrServHndlrTbl[XPBR_SERV_EXT_PWRUPPLD]();
#else

	u32 RegVal;
	u32 PollCount;


	Xil_Out32(PMU_GLOBAL_PWRUP_EN, PMU_GLOBAL_PWR_PL_MASK);
	Xil_Out32(PMU_GLOBAL_PWRUP_TRIG, PMU_GLOBAL_PWR_PL_MASK);
	PollCount = (PL_DONE_POLL_COUNT);
	do {
		RegVal = Xil_In32(PMU_GLOBAL_PWRUP_STATUS) &
					PMU_GLOBAL_PWR_PL_MASK;
		PollCount--;
	} while ((RegVal != 0) && PollCount);

	if (PollCount == 0)
		Status = XFPGA_ERROR_PL_POWER_UP;
#endif
	return Status;

}
/**
*
* This function is used to request isolation restore, through PMU
*
* @param	Mask of the entries for which isolation is to be restored
*
* @return	XFSBL_SUCCESS (for now always returns this)
*
* @note		None.
*
****************************************************************************/
static u32 XFpga_IsolationRestore()
{
	u32 Status = XFPGA_SUCCESS;

#ifdef __MICROBLAZE__
	u32 Reg;
	Status = XpbrServHndlrTbl[XPBR_SERV_EXT_PLNONPCAPISO]();
#else

	u32 PollCount;
	u32 RegVal;


	/* Isolation request enable */
	Xil_Out32(PMU_GLOBAL_ISO_INT_EN, PMU_GLOBAL_PWR_PL_MASK);

	/* Trigger Isolation request */
	Xil_Out32(PMU_GLOBAL_ISO_TRIG, PMU_GLOBAL_PWR_PL_MASK);

	/* Poll for Isolation complete */
	PollCount = (PL_DONE_POLL_COUNT);
	do {
		RegVal = Xil_In32(PMU_GLOBAL_ISO_STATUS) & PMU_GLOBAL_PWR_PL_MASK;
		PollCount--;
	} while ((RegVal != 0) && PollCount);

	if (PollCount == 0)
		Status = XFPGA_ERROR_PL_ISOLATION;
#endif
	return Status;
}
/**
*
* This function is used to reset the PL from PS EMIO pins
*
* @param	TotalResets
*
* @return	XFSBL_SUCCESS (for now always returns this)
*
* @note		None.
*
****************************************************************************/
static u32 XFpga_PsPlGpioReset(u32 TotalResets) {
	u32 Status = XFPGA_SUCCESS;
	u32 RegVal = 0;
	u32 MaskVal;

	/* Set EMIO Direction */
	RegVal = Xil_In32(GPIO_DIRM_5_EMIO) |
		~(~0 << TotalResets) << (MAX_REG_BITS + 1 - TotalResets);
	Xil_Out32(GPIO_DIRM_5_EMIO, RegVal);

	/*Assert the EMIO with the required Mask */
	MaskVal = ~(~0 << TotalResets) << (MAX_REG_BITS/2 + 1 - TotalResets) | 0xFFFF0000;
	RegVal = MaskVal & ~(~(~0 << TotalResets) << (MAX_REG_BITS + 1 - TotalResets));
	Xil_Out32(GPIO_MASK_DATA_5_MSW,RegVal);
	usleep(1000);

	/*De-assert the EMIO with the required Mask */
	RegVal = ~(~(~0 << TotalResets) << (MAX_REG_BITS + 1 - TotalResets)) & 0xFFFF0000;
	Xil_Out32(GPIO_MASK_DATA_5_MSW, RegVal);
	usleep(1000);

	/*Assert the EMIO with the required Mask */
	MaskVal = ~(~0 << TotalResets) << (MAX_REG_BITS/2 + 1 - TotalResets) | 0xFFFF0000;
	RegVal = MaskVal & ~(~(~0 << TotalResets) << (MAX_REG_BITS + 1 - TotalResets));
	Xil_Out32(GPIO_MASK_DATA_5_MSW,RegVal);
	usleep(1000);

	return Status;
}

/*****************************************************************************/
/** This function  provides the STATUS of PCAP interface
 *
 * @param	None
 *
 * @return	Status of the PCAP interface.
 *
 *****************************************************************************/
u32 XFpga_PcapStatus() {

	return Xil_In32(CSU_PCAP_STATUS);
}
