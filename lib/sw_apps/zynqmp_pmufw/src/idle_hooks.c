/*
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


/**
 * Implementation for the custom idling of of individual peripheral node.
 */

#include <sleep.h>
#include "pm_node_idle.h"
#include "pm_common.h"
#include "xpfw_default.h"

#ifdef ENABLE_NODE_IDLING

#define MAX_TIMEOUT 	10000		// Max Retry count

#if defined(XPMU_XTTCPS_0)  || \
	defined(XPMU_XTTCPS_3)  || \
	defined(XPMU_XTTCPS_6)  || \
	defined(XPMU_XTTCPS_9)

 /**
  * NodeTtcIdle() - Custom code to idle the TTC
  *
  * @BaseAddress: TTC base address
  */
void NodeTtcIdle(u32 FirstBaseAddress)
{
	u32 BaseAddress;

	for (BaseAddress = FirstBaseAddress; BaseAddress < FirstBaseAddress +
	     0xC; BaseAddress += 4) {
		u32 Val;

		/*Stop the TTC timer */
		Val = Xil_In32(BaseAddress + XTTCPS_CNT_CNTRL_OFFSET);
		Xil_Out32(BaseAddress + XTTCPS_CNT_CNTRL_OFFSET, (Val |
			  ~XTTCPS_CNT_CNTRL_DIS_MASK));

		/*
		 * Reset the counter control register
		 */
		XTtcPs_WriteReg(BaseAddress, XTTCPS_CNT_CNTRL_OFFSET,
				XTTCPS_CNT_CNTRL_RESET_VALUE);

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
}
#endif

#if defined(XPMU_SD_0) || \
	defined(XPMU_SD_1)

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
		} while ((StatusReg & SdpsActive) && --Timeout);
	}
	if (Timeout == 0) {
		PmWarn("sd not idle\r\n");
	}

	/* Reset the eMMC card */
	EmmcStatus = Xil_In32(IOU_SLCR_BASE + IOU_SD_CTRL_OFFSET);

#ifdef XPMU_SD_0
	if ((BaseAddress == XPMU_SD_0_BASEADDR)
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

#ifdef XPMU_SD_1
	if ((BaseAddress == XPMU_SD_1_BASEADDR)
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
	while (((Val & XSDPS_SWRST_ALL_MASK) != 0U) && --Timeout) {
		Val = XSdPs_ReadReg8(BaseAddress, XSDPS_SW_RST_OFFSET);
	}
	if (Timeout == 0) {
		PmWarn("sd not reset\r\n");
	}
}

#endif


#if defined(XPMU_I2C_0) || \
	defined(XPMU_I2C_1)

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
	} while (((StatusReg & XIICPS_SR_BA_MASK) != 0x0U) && --Timeout);
	if (Timeout == 0) {
		PmWarn("i2c not idle\r\n");
	}
}
#endif


#if defined(XPMU_ETHERNET_0) || \
	defined(XPMU_ETHERNET_1) || \
	defined(XPMU_ETHERNET_2) || \
	defined(XPMU_ETHERNET_3)

#define DMA_STOP_TIMEOUT		(500)
#define REG_POLL_STATUS_DELAY		(3000) /* in us */

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
	} while ((!(Reg & XEMACPS_NWSR_MDIOIDLE_MASK)) && --Timeout);

	if (Timeout == 0) {
		PmWarn("gem not idle\r\n");
	}

	/* Disable all interrupts */
	XEmacPs_WriteReg(BaseAddress, XEMACPS_IDR_OFFSET, XEMACPS_IXR_ALL_MASK);

	/* Halt the TX traffic */
	Reg = XEmacPs_ReadReg(BaseAddress, XEMACPS_NWCTRL_OFFSET);
	Reg |= XEMACPS_NWCTRL_HALTTX_MASK;
	XEmacPs_WriteReg(BaseAddress, XEMACPS_NWCTRL_OFFSET, Reg);

	/* Do not accept RX frames addressed to broadcast address */
	Reg = XEmacPs_ReadReg(BaseAddress, XEMACPS_NWCFG_OFFSET);
	Reg |= XEMACPS_NWCFG_BCASTDI_MASK;	/* No Broadcast bit */
	XEmacPs_WriteReg(BaseAddress, XEMACPS_NWCFG_OFFSET, Reg);

	/* Disable specific address filtering and thereby stop accept other RX frames */
	XEmacPs_WriteReg(BaseAddress, XEMACPS_LADDR1L_OFFSET, 0x0);
	XEmacPs_WriteReg(BaseAddress, XEMACPS_LADDR2L_OFFSET, 0x0);
	XEmacPs_WriteReg(BaseAddress, XEMACPS_LADDR3L_OFFSET, 0x0);
	XEmacPs_WriteReg(BaseAddress, XEMACPS_LADDR4L_OFFSET, 0x0);

	/*
	 * Check for the DMA stop. Poll on the current bd ptr and make sure it
	 * doesn't move for considerable time.
	 */
	/* Wait for TX DMA to stop */
	Timeout = DMA_STOP_TIMEOUT; /* Worst case timeout */
	do {
		Reg = XEmacPs_ReadReg(BaseAddress, XEMACPS_TXQBASE_OFFSET);
		usleep(REG_POLL_STATUS_DELAY);
	} while ((Reg != XEmacPs_ReadReg(BaseAddress, XEMACPS_TXQBASE_OFFSET)) && --Timeout);

	if (0 == Timeout) {
		PmWarn("GEM TX Not Idled\r\n");
	}

	/* Wait for RX DMA to stop */
	Timeout = DMA_STOP_TIMEOUT; /* Worst case timeout */
	do {
		Reg = XEmacPs_ReadReg(BaseAddress, XEMACPS_RXQBASE_OFFSET);
		usleep(REG_POLL_STATUS_DELAY);
	} while ((Reg != XEmacPs_ReadReg(BaseAddress, XEMACPS_RXQBASE_OFFSET)) && --Timeout);

	if (0 == Timeout) {
		PmWarn("GEM RX Not Idled\r\n");
	}

	/* Disable the receiver & transmitter */
	Reg = XEmacPs_ReadReg(BaseAddress, XEMACPS_NWCTRL_OFFSET);
	Reg &= (u32)(~XEMACPS_NWCTRL_RXEN_MASK);
	Reg &= (u32)(~XEMACPS_NWCTRL_TXEN_MASK);
	XEmacPs_WriteReg(BaseAddress, XEMACPS_NWCTRL_OFFSET, Reg);
}
#endif


#ifdef XPAR_XQSPIPSU_0_DEVICE_ID

/**
 * NodeQspiIdle() - Custom code to idle the QSPI
 *
 * @BaseAddress: QSPI base address
 */

void NodeQspiIdle(u32 BaseAddress)
{
	volatile u32 StatusReg;
	/*
	 * If QSPI is enabled, pause Tx and Rx DMA.
	 */
	StatusReg = XQspiPsu_ReadReg(BaseAddress, XQSPIPSU_EN_OFFSET);
	if (StatusReg & XQSPIPSU_EN_MASK) {
		StatusReg = XQspiPsu_ReadReg(BaseAddress, XQSPIPSU_QSPIDMA_DST_CTRL_OFFSET);
		StatusReg |= (XQSPIPSU_QSPIDMA_DST_CTRL_PAUSE_STRM_MASK |
			      XQSPIPSU_QSPIDMA_DST_CTRL_PAUSE_MEM_MASK);
		XQspiPsu_WriteReg(BaseAddress, XQSPIPSU_QSPIDMA_DST_CTRL_OFFSET, StatusReg);
	}
}

#endif

#if defined(XPMU_USB_0) || defined(XPMU_USB_1)

#define XUSBPSU_EP_PARAM_INDEX	3
#define XUSBPSU_GSTS_OP_MODE	0x3

/**
 * NodeUsbIdle() - Code to idle the Usb
 *
 * @BaseAddress: USB base address
 */

void NodeUsbIdle(u32 BaseAddress)
{
	u32 regVal;
	u32 LocalTimeout = MAX_TIMEOUT;
	u8 PhyEpNum, EpNums;
	u32 Cmd, RscIdx;

	regVal = Xil_In32(BaseAddress + XUSBPSU_GSTS);

	/* Check if USB is in device mode */
	if ((regVal & XUSBPSU_GSTS_OP_MODE) == 0) {
		/* Read number of endpoints */
		regVal = Xil_In32(BaseAddress + ((u32)XUSBPSU_GHWPARAMS0_OFFSET
				  + ((u32)XUSBPSU_EP_PARAM_INDEX * 4)));
		EpNums = XUSBPSU_NUM_EPS(regVal);

		/* Stop transfers */
		for (PhyEpNum = 0; PhyEpNum <= EpNums; PhyEpNum++) {
			LocalTimeout = MAX_TIMEOUT;

			/* Issue EndTransfer command */
			RscIdx = XUSBPSU_DEPCMD_GET_RSC_IDX(Xil_In32(BaseAddress
						+ XUSBPSU_DEPCMD(PhyEpNum)));
			if (RscIdx == 0) {
				continue;
			}

			Cmd = XUSBPSU_DEPCMD_ENDTRANSFER;
			Cmd |= XUSBPSU_DEPCMD_CMDIOC;
			Cmd |= XUSBPSU_DEPCMD_PARAM(RscIdx);

			Xil_Out32(BaseAddress +  XUSBPSU_DEPCMDPAR0(PhyEpNum),
					0x00);
			Xil_Out32(BaseAddress +  XUSBPSU_DEPCMDPAR1(PhyEpNum),
					0x00);
			Xil_Out32(BaseAddress +  XUSBPSU_DEPCMDPAR2(PhyEpNum),
					0x00);
			Xil_Out32(BaseAddress +  XUSBPSU_DEPCMD(PhyEpNum),
					Cmd | XUSBPSU_DEPCMD_CMDACT);

			/* Check end of transfer */
			do {
				regVal = Xil_In32(BaseAddress +
						  XUSBPSU_DEPCMD(PhyEpNum));
			} while ((regVal & XUSBPSU_DEPCMD_CMDACT) &&
				 --LocalTimeout);

			if (LocalTimeout == 0U)
				PmWarn("Endpoint transfer not completed\r\n");
		}

		/* Disable endpoints */
		for (PhyEpNum = 0; PhyEpNum <= EpNums; PhyEpNum++) {
			regVal = Xil_In32(BaseAddress + XUSBPSU_DALEPENA);
			regVal &= ~XUSBPSU_DALEPENA_EP(PhyEpNum);
			Xil_Out32(BaseAddress + XUSBPSU_DALEPENA, regVal);
		}

		/* Stop USB device controller */
		regVal = Xil_In32(BaseAddress + XUSBPSU_DCTL);
		regVal &= ~XUSBPSU_DCTL_RUN_STOP;
		Xil_Out32(BaseAddress + XUSBPSU_DCTL, regVal);

		/* Check for USB */
		LocalTimeout = MAX_TIMEOUT;

		do {
			regVal = Xil_In32(BaseAddress + XUSBPSU_DSTS);
		} while (!(regVal & XUSBPSU_DSTS_DEVCTRLHLT) && --LocalTimeout);

		if (LocalTimeout == 0U)
			PmWarn("USB device controller not stopped\r\n");
	}

	/* Clear event buffer */
	Xil_Out32(BaseAddress +  XUSBPSU_GEVNTADRLO(0U), 0U);
	Xil_Out32(BaseAddress +  XUSBPSU_GEVNTADRHI(0U), 0U);
	Xil_Out32(BaseAddress +  XUSBPSU_GEVNTSIZ(0U),
		  (u32)XUSBPSU_GEVNTSIZ_INTMASK | XUSBPSU_GEVNTSIZ_SIZE(0U));
	Xil_Out32(BaseAddress +  XUSBPSU_GEVNTCOUNT(0U), 0U);
}
#endif

#ifdef XPAR_XDPPSU_0_DEVICE_ID

#ifdef XPAR_XDPDMA_0_DEVICE_ID

#define XDPDMA_CH_OFFSET	0X100
#define XDPDMA_NUM_CHANNEL	6U	/* Number of channels */
#define XDPDMA_CH_CNTL_ENABLE	BIT(0)
#define XDPDMA_CH_CNTL_PAUSE	BIT(1)
#define XDPDMA_EINTR_ALL	0xffffffffU

/**
 * XDpDmaStopChannels - Stop DPDMA channels
 *
 * Stop the channel with the following sequence: 1. Pause, 2. Wait until the
 * number of outstanding transactions to go to 0, 3. Disable the channel.
 */

static void XDpDmaStopChannels(void)
{
	u8 channel = 0;
	u32 regVal = 0, LocalTimeout;

	for (channel = 0; channel < XDPDMA_NUM_CHANNEL; channel++) {
		/* Pause the channel */
		XDpDma_ReadModifyWrite(XPAR_XDPDMA_0_BASEADDR, XDPDMA_CH0_CNTL
				       + XDPDMA_CH_OFFSET * channel,
				       XDPDMA_CH_CNTL_PAUSE,
				       XDPDMA_CH_CNTL_PAUSE_MASK);

		LocalTimeout = MAX_TIMEOUT;

		/* Wait until the outstanding transactions number to go to 0 */
		do {
			regVal = XDpDma_ReadReg(XPAR_XDPDMA_0_BASEADDR,
						XDPDMA_CH0_STATUS +
						XDPDMA_CH_OFFSET * channel);
		} while ((regVal & XDPDMA_CH_STATUS_OTRAN_CNT_MASK) &&
			  --LocalTimeout);
		if (!LocalTimeout) {
			PmWarn("DP DMA not ready to stop\r\n");
			continue;
		}

		/* Disable channels */
		XDpDma_ReadModifyWrite(XPAR_XDPDMA_0_BASEADDR, XDPDMA_CH0_CNTL
				       + XDPDMA_CH_OFFSET * channel,
				       ~XDPDMA_CH_CNTL_ENABLE,
				       XDPDMA_CH_CNTL_EN_MASK);
	}

	/* Disable all DPDMA interrupts */
	XDpDma_WriteReg(XPAR_XDPDMA_0_BASEADDR, XDPDMA_IDS, ~0);
}
#endif

/**
 * NodeDpIdle() - Custom code to idle the DP
 *
 * @BaseAddress: DP base address
 */

void NodeDpIdle(u32 BaseAddress)
{
#ifdef XPAR_XDPDMA_0_DEVICE_ID
	/* Stop all dpdma channels */
	XDpDmaStopChannels();
#endif

	/* Disable main stream attributes */
	Xil_Out32(BaseAddress + XDPPSU_ENABLE, 0x0);
}
#endif

#ifdef XPAR_PSU_SATA_S_AXI_BASEADDR

#define SATA_HOST_CTL			0x04
#define SATA_HOST_IRQ_EN		(1 << 1)
#define SATA_HOST_AHCI_EN		(1U << 31U)

#define SATA_AHCI_PORT0_CTRL_OFFSET	0x100
#define SATA_AHCI_PORT1_CTRL_OFFSET	0x180

#define SATA_AHCI_PORT_PXCMD		0x18
#define SATA_AHCI_PORT_PXCMD_ST		1
#define SATA_AHCI_PORT_PXCMD_CR		(1 << 15)

/**
 * NodeSataIdle() - Custom code to idle the SATA
 *
 * @BaseAddress: SATA base address
 */

void NodeSataIdle(u32 BaseAddress)
{
	u32 regVal, timeOut = MAX_TIMEOUT;

	/* Disable Interrupt */
	regVal = Xil_In32(BaseAddress + SATA_HOST_CTL);
	regVal &= ~SATA_HOST_IRQ_EN;
	Xil_Out32(BaseAddress + SATA_HOST_CTL, regVal);

	/* Stop HBA engine for PORT0 and PORT1 */
	regVal = Xil_In32(BaseAddress + SATA_AHCI_PORT0_CTRL_OFFSET +
			  SATA_AHCI_PORT_PXCMD);
	regVal &= ~SATA_AHCI_PORT_PXCMD_ST;
	Xil_Out32(BaseAddress + SATA_AHCI_PORT0_CTRL_OFFSET +
		  SATA_AHCI_PORT_PXCMD, regVal);

	regVal = Xil_In32(BaseAddress + SATA_AHCI_PORT1_CTRL_OFFSET +
			  SATA_AHCI_PORT_PXCMD);
	regVal &= ~SATA_AHCI_PORT_PXCMD_ST;
	Xil_Out32(BaseAddress + SATA_AHCI_PORT1_CTRL_OFFSET +
		  SATA_AHCI_PORT_PXCMD, regVal);

	/* Wait for command list DMA engine to stop running */

	do {
		regVal = Xil_In32(BaseAddress + SATA_AHCI_PORT0_CTRL_OFFSET +
				  SATA_AHCI_PORT_PXCMD);
	} while ((regVal & SATA_AHCI_PORT_PXCMD_CR) && --timeOut);

	if (!timeOut) {
		PmWarn("Command list DMA engine not stopped for PORT0\r\n");
	}

	timeOut = MAX_TIMEOUT;

	do {
		regVal = Xil_In32(BaseAddress + SATA_AHCI_PORT1_CTRL_OFFSET +
				  SATA_AHCI_PORT_PXCMD);
	} while ((regVal & SATA_AHCI_PORT_PXCMD_CR) && --timeOut);

	if (!timeOut) {
		PmWarn("Command list DMA engine not stopped for PORT1\r\n");
	}

	/* Disable AHCI */
	regVal = Xil_In32(BaseAddress + SATA_HOST_CTL);
	regVal &= ~SATA_HOST_AHCI_EN;
	Xil_Out32(BaseAddress + SATA_HOST_CTL, regVal);
	Xil_In32(BaseAddress + SATA_HOST_CTL); /* flush */
}
#endif

#if defined(XPMU_ZDMA_8) || \
	defined(XPMU_ZDMA_0)

/**
 * NodeZdmaIdle() - Custom code to idle the ZDMA (GDMA and ADMA)
 *
 * @BaseAddress: ZDMA base address of the first channel
 */

void NodeZdmaIdle(u32 BaseAddress)
{
	u8 channel = 0;
	volatile u32 regVal = 0;
	u32 LocalTimeout;

	for (channel = 0; channel < XZDMA_NUM_CHANNEL; channel++) {
		/*
		 * Idle each of the 8 channels
		 */

		/* Disable / stop the channel the channel */
		XZDma_WriteReg(BaseAddress, (XZDMA_CH_CTRL2_OFFSET),
				(XZDMA_CH_CTRL2_DIS_MASK));

		/*
		 * wait till transfers are not completed or halted
		 */
		LocalTimeout = MAX_TIMEOUT; /* todo: not right to use max timeout. do calibrate*/

		do {
			regVal = XZDma_ReadReg(BaseAddress, XZDMA_CH_STS_OFFSET) & XZDMA_STS_BUSY_MASK;
		}while (regVal && LocalTimeout --);

		/* Disables and clear all interrupts */
		XZDma_WriteReg(BaseAddress, XZDMA_CH_IDS_OFFSET, XZDMA_IXR_ALL_INTR_MASK);

		XZDma_WriteReg( BaseAddress, XZDMA_CH_ISR_OFFSET,
				(XZDma_ReadReg(BaseAddress, XZDMA_CH_ISR_OFFSET) & XZDMA_IXR_ALL_INTR_MASK));

		/* Reset all the configurations */
		XZDma_WriteReg(BaseAddress, XZDMA_CH_CTRL0_OFFSET, XZDMA_CTRL0_RESET_VALUE);
		XZDma_WriteReg(BaseAddress, XZDMA_CH_CTRL1_OFFSET, XZDMA_CTRL1_RESET_VALUE);
		XZDma_WriteReg(BaseAddress, XZDMA_CH_DATA_ATTR_OFFSET, XZDMA_DATA_ATTR_RESET_VALUE);
		XZDma_WriteReg(BaseAddress,	XZDMA_CH_DSCR_ATTR_OFFSET, XZDMA_DSCR_ATTR_RESET_VALUE);

		/* Clears total byte transferred */
		XZDma_WriteReg(BaseAddress,	XZDMA_CH_TOTAL_BYTE_OFFSET,
					XZDma_ReadReg(BaseAddress, XZDMA_CH_TOTAL_BYTE_OFFSET));

		/* Read interrupt counts to clear it on both source and destination channels*/
		XZDma_ReadReg(BaseAddress, XZDMA_CH_IRQ_SRC_ACCT_OFFSET);
		XZDma_ReadReg(BaseAddress, XZDMA_CH_IRQ_DST_ACCT_OFFSET);

		/*
		 * Reset the channel's coherent attributes.
		 */
		XZDma_WriteReg(BaseAddress, XZDMA_CH_DSCR_ATTR_OFFSET, 0x0);
		XZDma_WriteReg(BaseAddress, XZDMA_CH_SRC_DSCR_WORD3_OFFSET, 0x0);
		XZDma_WriteReg(BaseAddress, XZDMA_CH_DST_DSCR_WORD3_OFFSET, 0x0);

		BaseAddress += XZDMA_CH_OFFSET;
	}
}
#endif /* ZDMA */

#if defined(XPMU_CAN_0) || \
	defined(XPMU_CAN_1)
void NodeCanIdle(u32 BaseAddress)
{
	volatile u32 StatusReg;
	u32 LocalTimeout = MAX_TIMEOUT;

	/*Disable CAN */
	XCanPs_WriteReg(BaseAddress, XCANPS_SRR_OFFSET, 0U);
	do {
		StatusReg = XCanPs_ReadReg(BaseAddress, XCANPS_SR_OFFSET);
	} while (!(StatusReg & XCANPS_SR_CONFIG_MASK) && LocalTimeout--);
}
#endif /* CAN */

#if defined(XPAR_XNANDPSU_0_DEVICE_ID)
void NodeNandIdle(u32 BaseAddress)
{
	volatile u32 StatusReg;
	u32 LocalTimeout = MAX_TIMEOUT;

	/* Wait for transfer to complete if any */
	do {
		StatusReg =  XNandPsu_ReadReg(BaseAddress,
				XNANDPSU_INTR_STS_OFFSET);
	} while ((StatusReg & XNANDPSU_INTR_STS_TRANS_COMP_STS_EN_MASK) && LocalTimeout--);

	/* Disable the Interrupts */
	XNandPsu_WriteReg(BaseAddress,
			  XNANDPSU_INTR_STS_EN_OFFSET, 0U);
}
#endif /* NAND */

#ifdef XPAR_PSU_GPU_S_AXI_BASEADDR
#define XGPU_PP_CTRL_MGMT_REG_OFFSET		0x100C
#define XGPU_PP_SOFT_RESET_MASK			(1U << 7)
#define XGPU_PP_INT_RAWSTAT_REG_OFFSET		0x1020
#define XGPU_PP_RESET_COMPLETED_MASK		(1U << 12)

void NodeGpuPPIdle(u32 BaseAddress)
{
	volatile u32 StatusReg;
	u32 LocalTimeout = MAX_TIMEOUT;

	XPfw_RMW32(BaseAddress + XGPU_PP_CTRL_MGMT_REG_OFFSET,
		   XGPU_PP_SOFT_RESET_MASK,
		   XGPU_PP_SOFT_RESET_MASK);

	do {
		StatusReg = Xil_In32(BaseAddress +
				     XGPU_PP_INT_RAWSTAT_REG_OFFSET);
	} while (!(StatusReg & XGPU_PP_RESET_COMPLETED_MASK) &&
		 LocalTimeout--);
}

void NodeGpuIdle(u32 BaseAddress)
{
	NodeGpuPPIdle(BaseAddress + GPU_PP_0_OFFSET);
	NodeGpuPPIdle(BaseAddress + GPU_PP_1_OFFSET);
}
#endif /* GPU */
#endif /* ENABLE_NODE_IDLING */
