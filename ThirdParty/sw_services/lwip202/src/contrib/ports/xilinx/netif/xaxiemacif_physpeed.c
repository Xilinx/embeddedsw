/*
 * Copyright (c) 2007-2008, Advanced Micro Devices, Inc.
 *               All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in
 *      the documentation and/or other materials provided with the
 *      distribution.
 *    * Neither the name of Advanced Micro Devices, Inc. nor the names
 *      of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written
 *      permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Some portions copyright (c) 2010-2017 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include "netif/xaxiemacif.h"
#include "lwipopts.h"
#include "sleep.h"
#include "xemac_ieee_reg.h"

#define PHY_R0_ISOLATE  						0x0400
#define PHY_DETECT_REG  						1
#define PHY_IDENTIFIER_1_REG					2
#define PHY_IDENTIFIER_2_REG					3
#define PHY_DETECT_MASK 						0x1808
#define PHY_MARVELL_IDENTIFIER					0x0141
#define PHY_TI_IDENTIFIER					    0x2000

/* Marvel PHY flags */
#define MARVEL_PHY_IDENTIFIER 					0x141
#define MARVEL_PHY_MODEL_NUM_MASK				0x3F0
#define MARVEL_PHY_88E1111_MODEL				0xC0
#define MARVEL_PHY_88E1116R_MODEL				0x240
#define PHY_88E1111_RGMII_RX_CLOCK_DELAYED_MASK	0x0080

/* TI PHY Flags */
#define TI_PHY_DETECT_MASK 						0x796D
#define TI_PHY_IDENTIFIER 						0x2000
#define TI_PHY_DP83867_MODEL					0xA231
#define DP83867_RGMII_CLOCK_DELAY_CTRL_MASK		0x0003
#define DP83867_RGMII_TX_CLOCK_DELAY_MASK		0x0030
#define DP83867_RGMII_RX_CLOCK_DELAY_MASK		0x0003

/* TI DP83867 PHY Registers */
#define DP83867_R32_RGMIICTL1					0x32
#define DP83867_R86_RGMIIDCTL					0x86

#define TI_PHY_REGCR			0xD
#define TI_PHY_ADDDR			0xE
#define TI_PHY_PHYCTRL			0x10
#define TI_PHY_CFGR2			0x14
#define TI_PHY_SGMIITYPE		0xD3
#define TI_PHY_CFGR2_SGMII_AUTONEG_EN	0x0080
#define TI_PHY_SGMIICLK_EN		0x4000
#define TI_PHY_REGCR_DEVAD_EN		0x001F
#define TI_PHY_REGCR_DEVAD_DATAEN	0x4000
#define TI_PHY_CFGR2_MASK		0x003F
#define TI_PHY_REGCFG4			0x31
#define TI_PHY_REGCR_DATA		0x401F
#define TI_PHY_CFG4RESVDBIT7		0x80
#define TI_PHY_CFG4RESVDBIT8		0x100
#define TI_PHY_CFG4_AUTONEG_TIMER	0x60

#define TI_PHY_CFG2_SPEEDOPT_10EN          0x0040
#define TI_PHY_CFG2_SGMII_AUTONEGEN        0x0080
#define TI_PHY_CFG2_SPEEDOPT_ENH           0x0100
#define TI_PHY_CFG2_SPEEDOPT_CNT           0x0800
#define TI_PHY_CFG2_SPEEDOPT_INTLOW        0x2000

#define TI_PHY_CR_SGMII_EN		0x0800

/* Loop counters to check for reset done
 */
#define RESET_TIMEOUT							0xFFFF
#define AUTO_NEG_TIMEOUT 						0x00FFFFFF

#if XPAR_GIGE_PCS_PMA_1000BASEX_CORE_PRESENT == 1 || \
	XPAR_GIGE_PCS_PMA_SGMII_CORE_PRESENT == 1
#define PCM_PMA_CORE_PRESENT
#else
#undef PCM_PMA_CORE_PRESENT
#endif

#ifdef PCM_PMA_CORE_PRESENT
#define IEEE_CTRL_RESET                         0x9140
#define IEEE_CTRL_ISOLATE_DISABLE               0xFBFF
#endif

#define PHY_XILINX_PCS_PMA_ID1			0x0174
#define PHY_XILINX_PCS_PMA_ID2			0x0C00

extern u32_t phyaddrforemac;

static void __attribute__ ((noinline)) AxiEthernetUtilPhyDelay(unsigned int Seconds);

static int detect_phy(XAxiEthernet *xaxiemacp)
{
	u16 phy_reg;
	u16 phy_id;
	u32 phy_addr;

	for (phy_addr = 31; phy_addr > 0; phy_addr--) {
		XAxiEthernet_PhyRead(xaxiemacp, phy_addr, PHY_DETECT_REG,
								&phy_reg);

		if ((phy_reg != 0xFFFF) &&
			((phy_reg & PHY_DETECT_MASK) == PHY_DETECT_MASK)) {
			/* Found a valid PHY address */
			LWIP_DEBUGF(NETIF_DEBUG, ("XAxiEthernet detect_phy: PHY detected at address %d.\r\n", phy_addr));
			LWIP_DEBUGF(NETIF_DEBUG, ("XAxiEthernet detect_phy: PHY detected.\r\n"));
			XAxiEthernet_PhyRead(xaxiemacp, phy_addr, PHY_IDENTIFIER_1_REG,
										&phy_reg);
			if ((phy_reg != PHY_MARVELL_IDENTIFIER) &&
                (phy_reg != TI_PHY_IDENTIFIER)){
				xil_printf("WARNING: Not a Marvell or TI Ethernet PHY. Please verify the initialization sequence\r\n");
			}
			phyaddrforemac = phy_addr;
			return phy_addr;
		}

		XAxiEthernet_PhyRead(xaxiemacp, phy_addr, PHY_IDENTIFIER_1_REG,
				&phy_id);

		if (phy_id == PHY_XILINX_PCS_PMA_ID1) {
			XAxiEthernet_PhyRead(xaxiemacp, phy_addr, PHY_IDENTIFIER_2_REG,
					&phy_id);
			if (phy_id == PHY_XILINX_PCS_PMA_ID2) {
				/* Found a valid PHY address */
				LWIP_DEBUGF(NETIF_DEBUG, ("XAxiEthernet detect_phy: PHY detected at address %d.\r\n",
							phy_addr));
				phyaddrforemac = phy_addr;
				return phy_addr;
			}
		}
	}

	LWIP_DEBUGF(NETIF_DEBUG, ("XAxiEthernet detect_phy: No PHY detected.  Assuming a PHY at address 0\r\n"));

        /* default to zero */
	return 0;
}

void XAxiEthernet_PhyReadExtended(XAxiEthernet *InstancePtr, u32 PhyAddress,
		u32 RegisterNum, u16 *PhyDataPtr)
{
	XAxiEthernet_PhyWrite(InstancePtr, PhyAddress,
			IEEE_MMD_ACCESS_CONTROL_REG, IEEE_MMD_ACCESS_CTRL_DEVAD_MASK);

	XAxiEthernet_PhyWrite(InstancePtr, PhyAddress,
			IEEE_MMD_ACCESS_ADDRESS_DATA_REG, RegisterNum);

	XAxiEthernet_PhyWrite(InstancePtr, PhyAddress,
			IEEE_MMD_ACCESS_CONTROL_REG, IEEE_MMD_ACCESS_CTRL_NOPIDEVAD_MASK);

	XAxiEthernet_PhyRead(InstancePtr, PhyAddress,
			IEEE_MMD_ACCESS_ADDRESS_DATA_REG, PhyDataPtr);

}

void XAxiEthernet_PhyWriteExtended(XAxiEthernet *InstancePtr, u32 PhyAddress,
		u32 RegisterNum, u16 PhyDataPtr)
{
	XAxiEthernet_PhyWrite(InstancePtr, PhyAddress,
			IEEE_MMD_ACCESS_CONTROL_REG, IEEE_MMD_ACCESS_CTRL_DEVAD_MASK);

	XAxiEthernet_PhyWrite(InstancePtr, PhyAddress,
			IEEE_MMD_ACCESS_ADDRESS_DATA_REG, RegisterNum);

	XAxiEthernet_PhyWrite(InstancePtr, PhyAddress,
			IEEE_MMD_ACCESS_CONTROL_REG, IEEE_MMD_ACCESS_CTRL_NOPIDEVAD_MASK);

	XAxiEthernet_PhyWrite(InstancePtr, PhyAddress,
			IEEE_MMD_ACCESS_ADDRESS_DATA_REG, PhyDataPtr);

}

unsigned int get_phy_negotiated_speed (XAxiEthernet *xaxiemacp, u32 phy_addr)
{
	u16 control;
	u16 status;
	u16 partner_capabilities;
	u16 partner_capabilities_1000;
	u16 phylinkspeed;
	u16 temp;

#ifdef XPAR_GIGE_PCS_PMA_1000BASEX_CORE_PRESENT
	phy_addr = XPAR_PCSPMA_1000BASEX_PHYADDR;
#elif XPAR_GIGE_PCS_PMA_SGMII_CORE_PRESENT
	phy_addr = XPAR_PCSPMA_SGMII_PHYADDR;
#endif

	xil_printf("Start PHY autonegotiation \r\n");
	XAxiEthernet_PhyRead(xaxiemacp, phy_addr, IEEE_CONTROL_REG_OFFSET,
																	&control);

	control |= IEEE_CTRL_AUTONEGOTIATE_ENABLE;
	control |= IEEE_STAT_AUTONEGOTIATE_RESTART;
#ifdef PCM_PMA_CORE_PRESENT
    control &= IEEE_CTRL_ISOLATE_DISABLE;
#endif

	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, IEEE_CONTROL_REG_OFFSET,
														control);
#ifdef PCM_PMA_CORE_PRESENT
	XAxiEthernet_PhyRead(xaxiemacp, phy_addr, IEEE_STATUS_REG_OFFSET, &status);
	xil_printf("Waiting for PHY to  complete autonegotiation \r\n");
	while ( !(status & IEEE_STAT_AUTONEGOTIATE_COMPLETE) ) {
		AxiEthernetUtilPhyDelay(1);
		XAxiEthernet_PhyRead(xaxiemacp, phy_addr, IEEE_STATUS_REG_OFFSET,
									&status);

	}

	xil_printf("Autonegotiation complete \r\n");

#if XPAR_GIGE_PCS_PMA_1000BASEX_CORE_PRESENT == 1
	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, IEEE_PAGE_ADDRESS_REGISTER, 1);
	XAxiEthernet_PhyRead(xaxiemacp, phy_addr, IEEE_PARTNER_ABILITIES_1_REG_OFFSET, &temp);
	if ((temp & 0x0020) == 0x0020) {
		XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, IEEE_PAGE_ADDRESS_REGISTER, 0);
		return 1000;
	}
	else {
		XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, IEEE_PAGE_ADDRESS_REGISTER, 0);
		xil_printf("Link error, temp = %x\r\n", temp);
		return 0;
	}
#elif XPAR_GIGE_PCS_PMA_SGMII_CORE_PRESENT == 1
	xil_printf("Waiting for Link to be up; Polling for SGMII core Reg \r\n");
	XAxiEthernet_PhyRead(xaxiemacp, phy_addr, IEEE_PARTNER_ABILITIES_1_REG_OFFSET, &temp);
	while(!(temp & 0x8000)) {
		XAxiEthernet_PhyRead(xaxiemacp, phy_addr, IEEE_PARTNER_ABILITIES_1_REG_OFFSET, &temp);
	}
	if((temp & 0x0C00) == 0x0800) {
		return 1000;
	}
	else if((temp & 0x0C00) == 0x0400) {
		return 100;
	}
	else if((temp & 0x0C00) == 0x0000) {
		return 10;
	} else {
		xil_printf("get_IEEE_phy_speed(): Invalid speed bit value, Deafulting to Speed = 10 Mbps\r\n");
		XAxiEthernet_PhyRead(xaxiemacp, phy_addr, IEEE_CONTROL_REG_OFFSET, &temp);
		XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, IEEE_CONTROL_REG_OFFSET, 0x0100);
		return 10;
	}
#endif
#endif

	/* Read PHY control and status registers is successful. */
	XAxiEthernet_PhyRead(xaxiemacp, phy_addr, IEEE_CONTROL_REG_OFFSET,
														&control);
	XAxiEthernet_PhyRead(xaxiemacp, phy_addr, IEEE_STATUS_REG_OFFSET,
														&status);
	if ((control & IEEE_CTRL_AUTONEGOTIATE_ENABLE) && (status &
					IEEE_STAT_AUTONEGOTIATE_CAPABLE)) {
		xil_printf("Waiting for PHY to complete autonegotiation.\r\n");
		while ( !(status & IEEE_STAT_AUTONEGOTIATE_COMPLETE) ) {
							XAxiEthernet_PhyRead(xaxiemacp, phy_addr,
									IEEE_STATUS_REG_OFFSET,
									&status);
	    }

		xil_printf("autonegotiation complete \r\n");

		XAxiEthernet_PhyRead(xaxiemacp, phy_addr,
										IEEE_PARTNER_ABILITIES_1_REG_OFFSET,
										&partner_capabilities);
		if (status & IEEE_STAT_1GBPS_EXTENSIONS) {
			XAxiEthernet_PhyRead(xaxiemacp, phy_addr,
					IEEE_PARTNER_ABILITIES_3_REG_OFFSET,
					&partner_capabilities_1000);
			if (partner_capabilities_1000 &
					IEEE_AN3_ABILITY_MASK_1GBPS)
				return 1000;
		}

		if (partner_capabilities & IEEE_AN1_ABILITY_MASK_100MBPS)
			return 100;
		if (partner_capabilities & IEEE_AN1_ABILITY_MASK_10MBPS)
			return 10;

		xil_printf("%s: unknown PHY link speed, setting TEMAC speed to be 10 Mbps\r\n",
				__FUNCTION__);
		return 10;
	} else {
		/* Update TEMAC speed accordingly */
		if (status & IEEE_STAT_1GBPS_EXTENSIONS) {

			/* Get commanded link speed */
			phylinkspeed = control &
				IEEE_CTRL_1GBPS_LINKSPEED_MASK;

			switch (phylinkspeed) {
				case (IEEE_CTRL_LINKSPEED_1000M):
					return 1000;
				case (IEEE_CTRL_LINKSPEED_100M):
					return 100;
				case (IEEE_CTRL_LINKSPEED_10M):
					return 10;
				default:
					xil_printf("%s: unknown PHY link speed (%d), setting TEMAC speed to be 10 Mbps\r\n",
						__FUNCTION__, phylinkspeed);
					return 10;
			}
		} else {
			return (control & IEEE_CTRL_LINKSPEED_MASK) ? 100 : 10;
		}
	}
}

unsigned int get_phy_speed_TI_DP83867(XAxiEthernet *xaxiemacp, u32 phy_addr)
{
	u16 phy_val;
	u16 control;

	xil_printf("Start PHY autonegotiation \r\n");

	/* Changing the PHY RX and TX DELAY settings. */
	XAxiEthernet_PhyReadExtended(xaxiemacp, phy_addr, DP83867_R32_RGMIICTL1, &phy_val);
	phy_val |= DP83867_RGMII_CLOCK_DELAY_CTRL_MASK;
	XAxiEthernet_PhyWriteExtended(xaxiemacp, phy_addr, DP83867_R32_RGMIICTL1, phy_val);

	XAxiEthernet_PhyReadExtended(xaxiemacp, phy_addr, DP83867_R86_RGMIIDCTL, &phy_val);
	phy_val &= 0xFF00;
	phy_val |= DP83867_RGMII_TX_CLOCK_DELAY_MASK;
	phy_val |= DP83867_RGMII_RX_CLOCK_DELAY_MASK;
	XAxiEthernet_PhyWriteExtended(xaxiemacp, phy_addr, DP83867_R86_RGMIIDCTL, phy_val);

	/* Set advertised speeds for 10/100/1000Mbps modes. */
	XAxiEthernet_PhyRead(xaxiemacp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, &control);
	control |= IEEE_ASYMMETRIC_PAUSE_MASK;
	control |= IEEE_PAUSE_MASK;
	control |= ADVERTISE_100;
	control |= ADVERTISE_10;
	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, control);

	XAxiEthernet_PhyRead(xaxiemacp, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET, &control);
	control |= ADVERTISE_1000;
	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET, control);

	return get_phy_negotiated_speed(xaxiemacp, phy_addr);
}

unsigned int get_phy_speed_TI_DP83867_SGMII(XAxiEthernet *xaxiemacp, u32 phy_addr)
{
	u16 control;
	u16 temp;
	u16 phyregtemp;

	xil_printf("Start TI PHY autonegotiation \r\n");

	/* Enable SGMII Clock */
	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, TI_PHY_REGCR,
			      TI_PHY_REGCR_DEVAD_EN);
	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, TI_PHY_ADDDR,
			      TI_PHY_SGMIITYPE);
	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, TI_PHY_REGCR,
			      TI_PHY_REGCR_DEVAD_EN | TI_PHY_REGCR_DEVAD_DATAEN);
	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, TI_PHY_ADDDR,
			      TI_PHY_SGMIICLK_EN);

	XAxiEthernet_PhyRead(xaxiemacp, phy_addr, IEEE_CONTROL_REG_OFFSET,
			     &control);
	control |= (IEEE_CTRL_AUTONEGOTIATE_ENABLE | IEEE_CTRL_LINKSPEED_1000M |
		    IEEE_CTRL_FULL_DUPLEX);
	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, IEEE_CONTROL_REG_OFFSET,
			      control);

	XAxiEthernet_PhyRead(xaxiemacp, phy_addr, TI_PHY_CFGR2, &control);
	control &= TI_PHY_CFGR2_MASK;
	control |= (TI_PHY_CFG2_SPEEDOPT_10EN   |
		    TI_PHY_CFG2_SGMII_AUTONEGEN |
		    TI_PHY_CFG2_SPEEDOPT_ENH    |
		    TI_PHY_CFG2_SPEEDOPT_CNT    |
		    TI_PHY_CFG2_SPEEDOPT_INTLOW);
	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, TI_PHY_CFGR2, control);

	/* Disable RGMII */
	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, TI_PHY_REGCR,
			      TI_PHY_REGCR_DEVAD_EN);
	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, TI_PHY_ADDDR,
			      DP83867_R32_RGMIICTL1);
	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, TI_PHY_REGCR,
			      TI_PHY_REGCR_DEVAD_EN | TI_PHY_REGCR_DEVAD_DATAEN);
	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, TI_PHY_ADDDR, 0);

	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, TI_PHY_PHYCTRL,
			      TI_PHY_CR_SGMII_EN);

	xil_printf("Waiting for Link to be up \r\n");
	XAxiEthernet_PhyRead(xaxiemacp, phy_addr,
			     IEEE_PARTNER_ABILITIES_1_REG_OFFSET, &temp);
	while(!(temp & 0x4000)) {
		XAxiEthernet_PhyRead(xaxiemacp, phy_addr,
				IEEE_PARTNER_ABILITIES_1_REG_OFFSET, &temp);
	}
	xil_printf("Auto negotiation completed for TI PHY\n\r");

	/* SW workaround for unstable link when RX_CTRL is not STRAP MODE 3 or 4 */
	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, TI_PHY_REGCR, TI_PHY_REGCR_DEVAD_EN);
	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, TI_PHY_ADDDR, TI_PHY_REGCFG4);
	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, TI_PHY_REGCR, TI_PHY_REGCR_DATA);
	XAxiEthernet_PhyRead(xaxiemacp, phy_addr, TI_PHY_ADDDR, (u16_t *)&phyregtemp);
	phyregtemp &= ~(TI_PHY_CFG4RESVDBIT7);
	phyregtemp |= TI_PHY_CFG4RESVDBIT8;
	phyregtemp &= ~(TI_PHY_CFG4_AUTONEG_TIMER);
	phyregtemp |= TI_PHY_CFG4_AUTONEG_TIMER;
	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, TI_PHY_REGCR, TI_PHY_REGCR_DEVAD_EN);
	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, TI_PHY_ADDDR, TI_PHY_REGCFG4);
	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, TI_PHY_REGCR, TI_PHY_REGCR_DATA);
	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, TI_PHY_ADDDR, phyregtemp);

	return get_phy_negotiated_speed(xaxiemacp, phy_addr);
}

unsigned int get_phy_speed_88E1116R(XAxiEthernet *xaxiemacp, u32 phy_addr)
{
	u16 phy_val;
	u16 control;
	u16 status;
	u16 partner_capabilities;

	xil_printf("Start PHY autonegotiation \r\n");

	XAxiEthernet_PhyWrite(xaxiemacp,phy_addr, IEEE_PAGE_ADDRESS_REGISTER, 2);
	XAxiEthernet_PhyRead(xaxiemacp, phy_addr, IEEE_CONTROL_REG_MAC, &control);
	control |= IEEE_RGMII_TXRX_CLOCK_DELAYED_MASK;
	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, IEEE_CONTROL_REG_MAC, control);

	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, IEEE_PAGE_ADDRESS_REGISTER, 0);

	XAxiEthernet_PhyRead(xaxiemacp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, &control);
	control |= IEEE_ASYMMETRIC_PAUSE_MASK;
	control |= IEEE_PAUSE_MASK;
	control |= ADVERTISE_100;
	control |= ADVERTISE_10;
	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, control);

	XAxiEthernet_PhyRead(xaxiemacp, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET,
				&control);
	control |= ADVERTISE_1000;
	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET,
				control);

	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, IEEE_PAGE_ADDRESS_REGISTER, 0);
	XAxiEthernet_PhyRead(xaxiemacp, phy_addr, IEEE_COPPER_SPECIFIC_CONTROL_REG,
				&control);
	control |= (7 << 12);	/* max number of gigabit atphy_valts */
	control |= (1 << 11);	/* enable downshift */
	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, IEEE_COPPER_SPECIFIC_CONTROL_REG,
				control);

	XAxiEthernet_PhyRead(xaxiemacp, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
	control |= IEEE_CTRL_AUTONEGOTIATE_ENABLE;
	control |= IEEE_STAT_AUTONEGOTIATE_RESTART;
	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, IEEE_CONTROL_REG_OFFSET, control);

	XAxiEthernet_PhyRead(xaxiemacp, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
	control |= IEEE_CTRL_RESET_MASK;
	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, IEEE_CONTROL_REG_OFFSET, control);
	while (1) {
		XAxiEthernet_PhyRead(xaxiemacp, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
		if (control & IEEE_CTRL_RESET_MASK)
			continue;
		else
			break;
	}

	xil_printf("Waiting for PHY to complete autonegotiation.\r\n");

	XAxiEthernet_PhyRead(xaxiemacp, phy_addr, IEEE_STATUS_REG_OFFSET, &status);
	while ( !(status & IEEE_STAT_AUTONEGOTIATE_COMPLETE) ) {
		AxiEthernetUtilPhyDelay(1);
		XAxiEthernet_PhyRead(xaxiemacp, phy_addr, IEEE_COPPER_SPECIFIC_STATUS_REG_2,
							&phy_val);
		if (phy_val & IEEE_AUTONEG_ERROR_MASK) {
			xil_printf("Auto negotiation error \r\n");
		}
		XAxiEthernet_PhyRead(xaxiemacp, phy_addr, IEEE_STATUS_REG_OFFSET,
					&status);
	}

	xil_printf("autonegotiation complete \r\n");

	XAxiEthernet_PhyRead(xaxiemacp, phy_addr, IEEE_SPECIFIC_STATUS_REG,
				&partner_capabilities);
	if ( ((partner_capabilities >> 14) & 3) == 2)/* 1000Mbps */
		return 1000;
	else if ( ((partner_capabilities >> 14) & 3) == 1)/* 100Mbps */
		return 100;
	else					/* 10Mbps */
		return 10;
}


unsigned int get_phy_speed_88E1111 (XAxiEthernet *xaxiemacp, u32 phy_addr)
{
	u16 control;
	int TimeOut;
	u16 phy_val;

	if (XAxiEthernet_GetPhysicalInterface(xaxiemacp) ==
											XAE_PHY_TYPE_RGMII_2_0) {
		XAxiEthernet_PhyRead(xaxiemacp, phy_addr,
						IEEE_EXT_PHY_SPECIFIC_CONTROL_REG, &phy_val);
		phy_val |= PHY_88E1111_RGMII_RX_CLOCK_DELAYED_MASK;
		XAxiEthernet_PhyWrite(xaxiemacp, phy_addr,
						IEEE_EXT_PHY_SPECIFIC_CONTROL_REG, phy_val);

		XAxiEthernet_PhyRead(xaxiemacp, phy_addr, IEEE_CONTROL_REG_OFFSET,
													&control);
		XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, IEEE_CONTROL_REG_OFFSET,
	                                        control | IEEE_CTRL_RESET_MASK);

		TimeOut = RESET_TIMEOUT;
		while (TimeOut) {
				XAxiEthernet_PhyRead(xaxiemacp, phy_addr,
									IEEE_CONTROL_REG_OFFSET, &control);
			if (!(control & IEEE_CTRL_RESET_MASK))
				break;
			TimeOut -= 1;
		}

		if (!TimeOut) {
			xil_printf("%s: Phy Reset failed\n\r", __FUNCTION__);
			return 0;
		}
	}

	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET,
															ADVERTISE_1000);
	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG,
														ADVERTISE_100_AND_10);

	return get_phy_negotiated_speed(xaxiemacp, phy_addr);
}

unsigned get_IEEE_phy_speed(XAxiEthernet *xaxiemacp)
{
	u16 phy_identifier;
	u16 phy_model;
	u8 phytype;

#ifdef XPAR_AXIETHERNET_0_BASEADDR
	u32 phy_addr = detect_phy(xaxiemacp);

	/* Get the PHY Identifier and Model number */
	XAxiEthernet_PhyRead(xaxiemacp, phy_addr, PHY_IDENTIFIER_1_REG, &phy_identifier);
	XAxiEthernet_PhyRead(xaxiemacp, phy_addr, PHY_IDENTIFIER_2_REG, &phy_model);

/* Depending upon what manufacturer PHY is connected, a different mask is
 * needed to determine the specific model number of the PHY. */
	if (phy_identifier == MARVEL_PHY_IDENTIFIER) {
		phy_model = phy_model & MARVEL_PHY_MODEL_NUM_MASK;

		if (phy_model == MARVEL_PHY_88E1116R_MODEL) {
			return get_phy_speed_88E1116R(xaxiemacp, phy_addr);
		} else if (phy_model == MARVEL_PHY_88E1111_MODEL) {
			return get_phy_speed_88E1111(xaxiemacp, phy_addr);
		}
	} else if (phy_identifier == TI_PHY_IDENTIFIER) {
		phy_model = phy_model & TI_PHY_DP83867_MODEL;
		phytype = XAxiEthernet_GetPhysicalInterface(xaxiemacp);

		if (phy_model == TI_PHY_DP83867_MODEL && phytype == XAE_PHY_TYPE_SGMII) {
			return get_phy_speed_TI_DP83867_SGMII(xaxiemacp, phy_addr);
		}

		if (phy_model == TI_PHY_DP83867_MODEL) {
			return get_phy_speed_TI_DP83867(xaxiemacp, phy_addr);
		}
	}
	else {
	    LWIP_DEBUGF(NETIF_DEBUG, ("XAxiEthernet get_IEEE_phy_speed: Detected PHY with unknown identifier/model.\r\n"));
	}
#endif
#ifdef PCM_PMA_CORE_PRESENT
	return get_phy_negotiated_speed(xaxiemacp, phy_addr);
#endif
}

unsigned configure_IEEE_phy_speed(XAxiEthernet *xaxiemacp, unsigned speed)
{
	u16 control;
	u32 phy_addr = detect_phy(xaxiemacp);
	u16 phy_val;

	if (XAxiEthernet_GetPhysicalInterface(xaxiemacp) ==
				XAE_PHY_TYPE_RGMII_2_0) {
		/* Setting Tx and Rx Delays for RGMII mode */
		XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, IEEE_PAGE_ADDRESS_REGISTER, 0x2);

		XAxiEthernet_PhyRead(xaxiemacp, phy_addr, IEEE_CONTROL_REG_MAC, &phy_val);
		phy_val |= IEEE_RGMII_TXRX_CLOCK_DELAYED_MASK;
		XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, IEEE_CONTROL_REG_MAC, phy_val);

		XAxiEthernet_PhyWrite(xaxiemacp, phy_addr, IEEE_PAGE_ADDRESS_REGISTER, 0x0);
	}

	XAxiEthernet_PhyRead(xaxiemacp, phy_addr,
				IEEE_CONTROL_REG_OFFSET,
				&control);
	control &= ~IEEE_CTRL_LINKSPEED_1000M;
	control &= ~IEEE_CTRL_LINKSPEED_100M;
	control &= ~IEEE_CTRL_LINKSPEED_10M;

	if (speed == 1000) {
		control |= IEEE_CTRL_LINKSPEED_1000M;
	}

	else if (speed == 100) {
		control |= IEEE_CTRL_LINKSPEED_100M;
		/* Dont advertise PHY speed of 1000 Mbps */
		XAxiEthernet_PhyWrite(xaxiemacp, phy_addr,
					IEEE_1000_ADVERTISE_REG_OFFSET,
					0);
		/* Dont advertise PHY speed of 10 Mbps */
		XAxiEthernet_PhyWrite(xaxiemacp, phy_addr,
				IEEE_AUTONEGO_ADVERTISE_REG,
				ADVERTISE_100);

	}
	else if (speed == 10) {
		control |= IEEE_CTRL_LINKSPEED_10M;
		/* Dont advertise PHY speed of 1000 Mbps */
		XAxiEthernet_PhyWrite(xaxiemacp, phy_addr,
				IEEE_1000_ADVERTISE_REG_OFFSET,
					0);
		/* Dont advertise PHY speed of 100 Mbps */
		XAxiEthernet_PhyWrite(xaxiemacp, phy_addr,
				IEEE_AUTONEGO_ADVERTISE_REG,
				ADVERTISE_10);
	}

	XAxiEthernet_PhyWrite(xaxiemacp, phy_addr,
				IEEE_CONTROL_REG_OFFSET,
				control | IEEE_CTRL_RESET_MASK);

	if (XAxiEthernet_GetPhysicalInterface(xaxiemacp) ==
			XAE_PHY_TYPE_SGMII) {
		control &= (~PHY_R0_ISOLATE);
		XAxiEthernet_PhyWrite(xaxiemacp,
				XPAR_AXIETHERNET_0_PHYADDR,
				IEEE_CONTROL_REG_OFFSET,
				control | IEEE_CTRL_AUTONEGOTIATE_ENABLE);
	}

	{
		volatile int wait;
		for (wait=0; wait < 100000; wait++);
		for (wait=0; wait < 100000; wait++);
	}
	return 0;
}

unsigned phy_setup_axiemac (XAxiEthernet *xaxiemacp)
{
	unsigned link_speed = 1000;

	if (XAxiEthernet_GetPhysicalInterface(xaxiemacp) ==
						XAE_PHY_TYPE_RGMII_1_3) {
		; /* Add PHY initialization code for RGMII 1.3 */
	} else if (XAxiEthernet_GetPhysicalInterface(xaxiemacp) ==
						XAE_PHY_TYPE_RGMII_2_0) {
		; /* Add PHY initialization code for RGMII 2.0 */
	} else if (XAxiEthernet_GetPhysicalInterface(xaxiemacp) ==
						XAE_PHY_TYPE_SGMII) {
#ifdef  CONFIG_LINKSPEED_AUTODETECT
		u32 phy_wr_data = IEEE_CTRL_AUTONEGOTIATE_ENABLE |
					IEEE_CTRL_LINKSPEED_1000M;
		phy_wr_data &= (~PHY_R0_ISOLATE);

		XAxiEthernet_PhyWrite(xaxiemacp,
				XPAR_AXIETHERNET_0_PHYADDR,
				IEEE_CONTROL_REG_OFFSET,
				phy_wr_data);
#endif
	} else if (XAxiEthernet_GetPhysicalInterface(xaxiemacp) ==
						XAE_PHY_TYPE_1000BASE_X) {
		; /* Add PHY initialization code for 1000 Base-X */
	}
/* set PHY <--> MAC data clock */
#ifdef  CONFIG_LINKSPEED_AUTODETECT
	link_speed = get_IEEE_phy_speed(xaxiemacp);
	xil_printf("auto-negotiated link speed: %d\r\n", link_speed);
#elif	defined(CONFIG_LINKSPEED1000)
	link_speed = 1000;
	configure_IEEE_phy_speed(xaxiemacp, link_speed);
	xil_printf("link speed: %d\r\n", link_speed);
#elif	defined(CONFIG_LINKSPEED100)
	link_speed = 100;
	configure_IEEE_phy_speed(xaxiemacp, link_speed);
	xil_printf("link speed: %d\r\n", link_speed);
#elif	defined(CONFIG_LINKSPEED10)
	link_speed = 10;
	configure_IEEE_phy_speed(xaxiemacp, link_speed);
	xil_printf("link speed: %d\r\n", link_speed);
#endif
	return link_speed;
}

static void __attribute__ ((noinline)) AxiEthernetUtilPhyDelay(unsigned int Seconds)
{
#if defined (__MICROBLAZE__)
	static int WarningFlag = 0;

	/* If MB caches are disabled or do not exist, this delay loop could
	 * take minutes instead of seconds (e.g., 30x longer).  Print a warning
	 * message for the user (once).  If only MB had a built-in timer!
	 */
	if (((mfmsr() & 0x20) == 0) && (!WarningFlag)) {
		WarningFlag = 1;
	}

#define ITERS_PER_SEC   (XPAR_CPU_CORE_CLOCK_FREQ_HZ / 6)
    asm volatile ("\n"
			"1:               \n\t"
			"addik r7, r0, %0 \n\t"
			"2:               \n\t"
			"addik r7, r7, -1 \n\t"
			"bneid  r7, 2b    \n\t"
			"or  r0, r0, r0   \n\t"
			"bneid %1, 1b     \n\t"
			"addik %1, %1, -1 \n\t"
			:: "i"(ITERS_PER_SEC), "d" (Seconds));
#else
    sleep(Seconds);
#endif
}
