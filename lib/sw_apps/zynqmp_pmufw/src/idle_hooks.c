/*
 * Copyright (C) 2014 - 2015 Xilinx, Inc.  All rights reserved.
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
 */

/**
 * Implementation for the custom idling of of individual peripheral node.
 */

#include <sleep.h>
#include "pm_node_idle.h"
#include "pm_common.h"

#define MAX_TIMEOUT 0x1FFFFFFF

#if defined(XPAR_PSU_TTC_0_DEVICE_ID) || \
	defined(XPAR_PSU_TTC_1_DEVICE_ID) || \
	defined(XPAR_PSU_TTC_2_DEVICE_ID) || \
	defined(XPAR_PSU_TTC_3_DEVICE_ID)

 /**
  * NodeTtcIdle() - Custom code to idle the TTC
  *
  * @BaseAddress: TTC base address
  */
void NodeTtcIdle(u32 BaseAddress)
{
	/*Stop the TTC timer */
	u32 Val = Xil_In32(BaseAddress + XTTCPS_CNT_CNTRL_OFFSET);
	Xil_Out32(BaseAddress + XTTCPS_CNT_CNTRL_OFFSET,(Val | ~XTTCPS_CNT_CNTRL_DIS_MASK));

	/*
	 * Reset the counter control register
	 */
	XTtcPs_WriteReg(BaseAddress, XTTCPS_CNT_CNTRL_OFFSET,  XTTCPS_CNT_CNTRL_RESET_VALUE);

	/*
	 * Clear counters interval values
	 */
	XTtcPs_WriteReg(BaseAddress, XTTCPS_INTERVAL_VAL_OFFSET, 0x0);

	/*
	 * Clear counters Match values
	 */
	XTtcPs_WriteReg(BaseAddress, XTTCPS_MATCH_0_OFFSET, 0x0);
	XTtcPs_WriteReg(BaseAddress, XTTCPS_MATCH_1_OFFSET, 0x0);
	XTtcPs_WriteReg(BaseAddress, XTTCPS_MATCH_2_OFFSET, 0x0);

	/*
	 * Disable counter's interrupts
	 */
	XTtcPs_WriteReg(BaseAddress, XTTCPS_IER_OFFSET, 0x0);

	/*
	 * Clear interrupts (status) for all the counters [clronrd]
	 */
	XTtcPs_ReadReg(BaseAddress, XTTCPS_ISR_OFFSET);
}
#endif

#if defined(XPAR_PSU_SD_0_DEVICE_ID) || \
	defined(XPAR_PSU_SD_1_DEVICE_ID)

#define IOU_SLCR_BASE  		0xFF180000
#define IOU_SD_CTRL_OFFSET	0x00000310

#define SD_SLEEP_TIME   1000		/* in ms */
#define EMMC_RESET_TIME 1			/* in ms */

#define SD0_EMMC_SEL_MASK	(0x1 << 0)
#define SD1_EMMC_SEL_MASK	(0x1 << 15)

/**
 * NodeSdioIdle() - Custom code to idle the SDIO
 *
 * @BaseAddress: SDIO base address
 */

void NodeSdioIdle(u32 BaseAddress)
{
	u16 EmmcStatus;
	u8 Val;
	u32 StatusReg;
	u32 PresentStateReg;
	u32 Timeout = MAX_TIMEOUT;
	u32 SdpsActive = (XSDPS_PSR_INHIBIT_CMD_MASK | XSDPS_PSR_INHIBIT_DAT_MASK
													| XSDPS_PSR_DAT_ACTIVE_MASK);

	PresentStateReg = XSdPs_ReadReg8(BaseAddress, XSDPS_PRES_STATE_OFFSET);
	/* Check for Card Present */
	if (PresentStateReg & XSDPS_PSR_CARD_INSRT_MASK) {
		/* Check for SD idle */
		do {
			StatusReg = XSdPs_ReadReg8(BaseAddress, XSDPS_PRES_STATE_OFFSET);
		} while ((StatusReg & SdpsActive) && Timeout--);
	}
	if (Timeout == 0) {
		PmDbg("SD was still not idle\n");
	}

	/* Reset the eMMC card */
	EmmcStatus = Xil_In32(IOU_SLCR_BASE + IOU_SD_CTRL_OFFSET);

#ifdef XPAR_PSU_SD_0_DEVICE_ID
	if ((BaseAddress == XPAR_PSU_SD_0_BASEADDR)
			&& (EmmcStatus & SD0_EMMC_SEL_MASK)) {
		Val = XSdPs_ReadReg8(BaseAddress, XSDPS_POWER_CTRL_OFFSET);
		XSdPs_WriteReg8(BaseAddress, XSDPS_POWER_CTRL_OFFSET,
				Val | XSDPS_PC_EMMC_HW_RST_MASK);
		usleep(1000 * EMMC_RESET_TIME);
		Val = XSdPs_ReadReg8(BaseAddress, XSDPS_POWER_CTRL_OFFSET);
		XSdPs_WriteReg8(BaseAddress, XSDPS_POWER_CTRL_OFFSET,
				Val & ~XSDPS_PC_EMMC_HW_RST_MASK);
	}
#endif

#ifdef XPAR_PSU_SD_1_DEVICE_ID
	if ((BaseAddress == XPAR_PSU_SD_1_BASEADDR)
			&& (EmmcStatus & SD1_EMMC_SEL_MASK)) {
		Val = XSdPs_ReadReg8(BaseAddress, XSDPS_POWER_CTRL_OFFSET);
		XSdPs_WriteReg8(BaseAddress, XSDPS_POWER_CTRL_OFFSET,
				Val | XSDPS_PC_EMMC_HW_RST_MASK);
		usleep(1000 * EMMC_RESET_TIME);
		Val = XSdPs_ReadReg8(BaseAddress, XSDPS_POWER_CTRL_OFFSET);
		XSdPs_WriteReg8(BaseAddress, XSDPS_POWER_CTRL_OFFSET,
				Val & ~XSDPS_PC_EMMC_HW_RST_MASK);
	}
#endif
	/* Disable bus power */
	XSdPs_WriteReg8(BaseAddress, XSDPS_POWER_CTRL_OFFSET, 0);
	usleep(1000 * SD_SLEEP_TIME);

	/* "Software reset for all" is initiated */
	XSdPs_WriteReg8(BaseAddress, XSDPS_SW_RST_OFFSET, XSDPS_SWRST_ALL_MASK);

	Timeout = MAX_TIMEOUT;
	/* Proceed with initialization only after reset is complete */
	Val = XSdPs_ReadReg8(BaseAddress, XSDPS_SW_RST_OFFSET);
	while (((Val & XSDPS_SWRST_ALL_MASK) != 0U) && Timeout--) {
		Val = XSdPs_ReadReg8(BaseAddress, XSDPS_SW_RST_OFFSET);
	}
	if (Timeout == 0) {
		PmDbg("SD was still not reset\n");
	}
}

#endif


#if defined(XPAR_PSU_I2C_0_DEVICE_ID) || \
	defined(XPAR_PSU_I2C_1_DEVICE_ID)

/**
 * NodeI2cIdle() - Custom code to idle the I2c
 *
 * @BaseAddress: I2c base address
 */

void NodeI2cIdle(u32 BaseAddress)
{
	u32 StatusReg;
	u32 Timeout = MAX_TIMEOUT;

	/* Check whether the I2C bus is busy */
	do {
		StatusReg = XIicPs_ReadReg(BaseAddress,
					   XIICPS_SR_OFFSET);
	} while (((StatusReg & XIICPS_SR_BA_MASK) != 0x0U) && Timeout--);
	if (Timeout == 0) {
		PmDbg("i2c was still not idle\n");
	}
}
#endif


#if defined(XPAR_PSU_ETHERNET_0_DEVICE_ID) || \
	defined(XPAR_PSU_ETHERNET_1_DEVICE_ID) || \
	defined(XPAR_PSU_ETHERNET_2_DEVICE_ID) || \
	defined(XPAR_PSU_ETHERNET_3_DEVICE_ID)

/**
 * NodeGemIdle() - Custom code to idle the GEM
 *
 * @BaseAddress: GEM base address
 */

void NodeGemIdle(u32 BaseAddress)
{
	u32 Reg;
	u32 Timeout = MAX_TIMEOUT;

	/* Make sure MDIO is in IDLE state */
	do {
		Reg = XEmacPs_ReadReg(BaseAddress, XEMACPS_NWSR_OFFSET);
	} while ((!(Reg & XEMACPS_NWSR_MDIOIDLE_MASK)) && Timeout--);

	if (Timeout == 0) {
		PmDbg("gem was still not idle\n");
	}

	/* stop all transactions of the Ethernet */
	/* Disable all interrupts */
	XEmacPs_WriteReg(BaseAddress, XEMACPS_IDR_OFFSET,
			   XEMACPS_IXR_ALL_MASK);

	/* Disable the receiver & transmitter */
	Reg = XEmacPs_ReadReg(BaseAddress, XEMACPS_NWCTRL_OFFSET);
	Reg &= (u32)(~XEMACPS_NWCTRL_RXEN_MASK);
	Reg &= (u32)(~XEMACPS_NWCTRL_TXEN_MASK);
	XEmacPs_WriteReg(BaseAddress, XEMACPS_NWCTRL_OFFSET, Reg);

}
#endif


#ifdef XPAR_PSU_QSPI_0_DEVICE_ID

/**
 * NodeQspiIdle() - Custom code to idle the QSPI
 *
 * @BaseAddress: QSPI base address
 */

void NodeQspiIdle(u32 BaseAddress)
{
	u32 StatusReg;
	u32 Timeout = MAX_TIMEOUT;
	/*
	 * Wait for the transfer to finish by polling Tx fifo status.
	 */
	do {
		StatusReg = XQspiPsu_ReadReg(BaseAddress, XQSPIPSU_XFER_STS_OFFSET);
	} while ((StatusReg != 0) && Timeout--);

	if (Timeout == 0) {
		PmDbg("QSPI was still not idle\n");
	}
}

#endif
