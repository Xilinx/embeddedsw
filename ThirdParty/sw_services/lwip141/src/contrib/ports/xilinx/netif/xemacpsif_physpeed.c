/******************************************************************************
*
* Copyright (C) 2010 - 2015 Xilinx, Inc.  All rights reserved.
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

/*****************************************************************************
* This file xemacpsif_physpeed.c implements functionalities to:
* - Detect the available PHYs connected to a MAC
* - Negotiate speed
* - Configure speed
* - Configure the SLCR registers for the negotiated speed
*
* In a typical use case, users of the APIs implemented in this file need to
* do the following.
* - Call the API detect_phy. It probes for the available PHYs connected to a MAC.
*   The MACs can be Emac0 (XPAR_XEMACPS_0_BASEADDR, 0xE000B000) or Emac1
*   (XPAR_XEMACPS_0_BASEADDR, 0xE000C000). It populates an array to notify
*   about the detected PHYs. The array phymapemac0 is used for Emac0 and
*   phymapemac1 is for Emac1.
* - The users need to parse the corresponding arrays, phymapemac0 or phymapemac1
*   to know the available PHYs for a MAC. The users then need to call phy_setup
*   to setup the PHYs for proper speed setting. The API phy_setup should be called
*   with the PHY address for which the speed needs to be negotiated or configured.
*   In a specific use case, if 2 PHYs are connected to Emac0 with addresses of 7
*   and 11, then users get these address details from phymapemac0 (after calling
*   detect_phy) and then call phy_setup twice, with ab address of 7 and 11.
* - Points to note: The MAC can operate at only one speed. If a MAC is connected
*   to multiple PHYs, then all PHYs must negotiate and configured for the same
*   speed.
* - This file implements static functions to set proper SLCR clocks. As stated
*   above, all PHYs connected to a PHY must operate at same speed and the SLCR
*   clock will be setup accordingly.
*
* This file implements the following PHY types.
* - The standard RGMII.
* - It provides support for GMII to RGMII converter Xilinx IP. This Xilinx IP
*   sits on the MDIO bus with a predefined PHY address. This IP exposes register
*   that needs to be programmed with the negotiated speed.
*   For example, in a typical design, the Emac0 or Emac1 exposes GMII interface.
*   The user can then use the Xilinx IP that converts GMII to RGMII.
*   The external PHY (most typically Marvell 88E1116R) negotiates for speed
*   with the remote PHY. The implementation in this file then programs the
*   Xilinx IP with this negotiated speed. The Xilinx IP has a predefined IP
*   address exposed through xparameters.h
* - The SGMII and 1000 BaseX PHY interfaces.
*   If the PHY interface is SGMII or 1000 BaseX a separate "get_IEEE_phy_speed"
*   is used which is different from standard RGMII "get_IEEE_phy_speed".
*   The 1000 BaseX always operates at 1000 Mbps. The SGMII interface can
*   negotiate speed accordingly.
*   For SGMII or 1000 BaseX interfaces, the detect_phy should not be called.
*   The phy addresses for these interfaces are fixed at the design time.
*
* Point to note:
* A MAC can not be connected to PHYs where there is a mix between
* SGMII or 1000 Basex or GMII/MII/RGMII.
* In a typical multiple PHY designs, it is expected that the PHYs connected
* will be RGMII or GMII.
*
* The users can choose not to negotiate speed from lwip settings GUI.
* If they opt to choose a particular PHY speed, then the PHY will hard code
* the speed to operate only at the corresponding speed. It will not advertise
* any other speeds. It is users responsibility to ensure that the remote PHY
* supports the speed programmed through the lwip gui.
*
* The following combination of MDIO/PHY are supported:
* - Multiple PHYs connected to the MDIO bus of a MAC. If Emac0 MDIO is connected
*   to single/multiple PHYs, it is supported. Similarly Emac1 MDIO connected to
*   single/multiple PHYs is supported.
* - A design where both the interfaces are present and are connected to their own
*   MDIO bus is supported.
*
* The following MDIO/PHY setup is not supported:
* - A design has both the MACs present. MDIO bus is available only for one MAC
*   (Emac0 or Emac1). This MDIO bus has multiple PHYs available for both the
*   MACs. The negotiated speed for PHYs sitting on the MDIO bus of one MAC will
*   not be see for the other MAC and hence the speed/SLCR settings of the other
*   MAC cannot be programmed. Hence this kind of design will not work for
*   this implementation.
*
********************************************************************************/

#include "netif/xemacpsif.h"
#include "lwipopts.h"
#include "xparameters_ps.h"
#include "xparameters.h"

/* Advertisement control register. */
#define ADVERTISE_10HALF		0x0020  /* Try for 10mbps half-duplex  */
#define ADVERTISE_10FULL		0x0040  /* Try for 10mbps full-duplex  */
#define ADVERTISE_100HALF		0x0080  /* Try for 100mbps half-duplex */
#define ADVERTISE_100FULL		0x0100  /* Try for 100mbps full-duplex */

#define ADVERTISE_100			(ADVERTISE_100FULL | ADVERTISE_100HALF)
#define ADVERTISE_10			(ADVERTISE_10FULL | ADVERTISE_10HALF)
#define ADVERTISE_1000			0x0300

#define IEEE_CONTROL_REG_OFFSET				0
#define IEEE_STATUS_REG_OFFSET				1
#define IEEE_AUTONEGO_ADVERTISE_REG			4
#define IEEE_PARTNER_ABILITIES_1_REG_OFFSET	5
#define IEEE_1000_ADVERTISE_REG_OFFSET		9
#define IEEE_COPPER_SPECIFIC_CONTROL_REG	16
#define IEEE_SPECIFIC_STATUS_REG			17
#define IEEE_COPPER_SPECIFIC_STATUS_REG_2	19
#define IEEE_CONTROL_REG_MAC				21
#define IEEE_PAGE_ADDRESS_REGISTER			22
#define IEEE_CTRL_1GBPS_LINKSPEED_MASK		0x2040
#define IEEE_CTRL_LINKSPEED_MASK			0x0040
#define IEEE_CTRL_LINKSPEED_1000M			0x0040
#define IEEE_CTRL_LINKSPEED_100M			0x2000
#define IEEE_CTRL_LINKSPEED_10M				0x0000
#define IEEE_CTRL_RESET_MASK				0x8000

#define IEEE_SPEED_MASK		0xC000
#define IEEE_SPEED_1000		0x8000
#define IEEE_SPEED_100		0x4000

#define IEEE_CTRL_RESET_MASK				0x8000
#define IEEE_CTRL_AUTONEGOTIATE_ENABLE		0x1000
#define IEEE_STAT_AUTONEGOTIATE_COMPLETE	0x0020
#define IEEE_STAT_AUTONEGOTIATE_RESTART		0x0200
#define IEEE_RGMII_TXRX_CLOCK_DELAYED_MASK	0x0030
#define IEEE_ASYMMETRIC_PAUSE_MASK			0x0800
#define IEEE_PAUSE_MASK						0x0400
#define IEEE_AUTONEG_ERROR_MASK				0x8000

#define PHY_DETECT_REG  						1
#define PHY_IDENTIFIER_1_REG					2
#define PHY_DETECT_MASK 					0x1808
#define PHY_MARVELL_IDENTIFIER				0x0141
#define PHY_TI_IDENTIFIER					0x2000

#define XEMACPS_GMII2RGMII_SPEED1000_FD		0x140
#define XEMACPS_GMII2RGMII_SPEED100_FD		0x2100
#define XEMACPS_GMII2RGMII_SPEED10_FD		0x100
#define XEMACPS_GMII2RGMII_REG_NUM			0x10

#define PHY_REGCR		0x0D
#define PHY_ADDAR		0x0E
#define PHY_RGMIIDCTL	0x86
#define PHY_RGMIICTL	0x32
#define PHY_STS			0x11

#define PHY_REGCR_ADDR	0x001F
#define PHY_REGCR_DATA	0x401F

/* Frequency setting */
#define SLCR_LOCK_ADDR			(XPS_SYS_CTRL_BASEADDR + 0x4)
#define SLCR_UNLOCK_ADDR		(XPS_SYS_CTRL_BASEADDR + 0x8)
#define SLCR_GEM0_CLK_CTRL_ADDR	(XPS_SYS_CTRL_BASEADDR + 0x140)
#define SLCR_GEM1_CLK_CTRL_ADDR	(XPS_SYS_CTRL_BASEADDR + 0x144)
#define SLCR_LOCK_KEY_VALUE 	0x767B
#define SLCR_UNLOCK_KEY_VALUE	0xDF0D
#define SLCR_ADDR_GEM_RST_CTRL	(XPS_SYS_CTRL_BASEADDR + 0x214)
#define EMACPS_SLCR_DIV_MASK	0xFC0FC0FF

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

u32_t phymapemac0[32];
u32_t phymapemac1[32];

static u32_t get_IEEE_phy_speed(XEmacPs *xemacpsp, u32_t phy_addr);
static void SetUpSLCRDivisors(u32_t mac_baseaddr, s32_t speed);
static u32_t configure_IEEE_phy_speed(XEmacPs *xemacpsp, u32_t phy_addr, u32_t speed);

#ifdef PCM_PMA_CORE_PRESENT
u32_t phy_setup (XEmacPs *xemacpsp, u32_t phy_addr)
{
	u32_t link_speed;
	u16_t regval;

	link_speed = get_IEEE_phy_speed(xemacpsp, phy_addr);
	if (link_speed == 1000)
		SetUpSLCRDivisors(xemacpsp->Config.BaseAddress,1000);
	else if (link_speed == 100)
		SetUpSLCRDivisors(xemacpsp->Config.BaseAddress,100);
	else
		SetUpSLCRDivisors(xemacpsp->Config.BaseAddress,10);

	xil_printf("link speed for phy address %d: %d\r\n", phy_addr, link_speed);
	return link_speed;
}

static u32_t get_IEEE_phy_speed(XEmacPs *xemacpsp, u32_t phy_addr)
{
	u16_t temp;
	u16_t control;
	u16_t status;
	u16_t partner_capabilities;

	xil_printf("Start PHY autonegotiation \r\n");

	XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
	control |= IEEE_CTRL_AUTONEGOTIATE_ENABLE;
	control |= IEEE_STAT_AUTONEGOTIATE_RESTART;
	control &= IEEE_CTRL_ISOLATE_DISABLE;
	XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, control);

	xil_printf("Waiting for PHY to complete autonegotiation.\r\n");

	XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_STATUS_REG_OFFSET, &status);
	while ( !(status & IEEE_STAT_AUTONEGOTIATE_COMPLETE) ) {
		sleep(1);
		XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_STATUS_REG_OFFSET,
																&status);
	}
	xil_printf("autonegotiation complete \r\n");

#if XPAR_GIGE_PCS_PMA_1000BASEX_CORE_PRESENT == 1
	XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_PAGE_ADDRESS_REGISTER, 1);
	XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_PARTNER_ABILITIES_1_REG_OFFSET, &temp);
	if ((temp & 0x0020) == 0x0020) {
		XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_PAGE_ADDRESS_REGISTER, 0);
		return 1000;
	}
	else {
		XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_PAGE_ADDRESS_REGISTER, 0);
		xil_printf("Link error, temp = %x\r\n", temp);
		return 0;
	}
#elif XPAR_GIGE_PCS_PMA_SGMII_CORE_PRESENT == 1
	xil_printf("Waiting for Link to be up; Polling for SGMII core Reg \r\n");
	XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_PARTNER_ABILITIES_1_REG_OFFSET, &temp);
	while(!(temp & 0x8000)) {
		XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_PARTNER_ABILITIES_1_REG_OFFSET, &temp);
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
		XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, &temp);
		XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, 0x0100);
		return 10;
	}
#endif

}

#else /*PCM_PMA_CORE_PRESENT not defined, GMII/RGMII case*/
void detect_phy(XEmacPs *xemacpsp)
{
	u16_t phy_reg;
	u32_t phy_addr;
	u32_t emacnum;

	if (xemacpsp->Config.BaseAddress == XPAR_XEMACPS_0_BASEADDR)
		emacnum = 0;
	else
		emacnum = 1;
	for (phy_addr = 31; phy_addr > 0; phy_addr--) {
		XEmacPs_PhyRead(xemacpsp, phy_addr, PHY_DETECT_REG,
							&phy_reg);

		if ((phy_reg != 0xFFFF) &&
			((phy_reg & PHY_DETECT_MASK) == PHY_DETECT_MASK)) {
			/* Found a valid PHY address */
			LWIP_DEBUGF(NETIF_DEBUG, ("XEmacPs detect_phy: PHY detected at address %d.\r\n",
																	phy_addr));
			if (emacnum == 0)
				phymapemac0[phy_addr] = TRUE;
			else
				phymapemac1[phy_addr] = TRUE;

			XEmacPs_PhyRead(xemacpsp, phy_addr, PHY_IDENTIFIER_1_REG,
							&phy_reg);
			if ((phy_reg != PHY_MARVELL_IDENTIFIER) &&
				(phy_reg != PHY_TI_IDENTIFIER)) {
				xil_printf("WARNING: Not a Marvell or TI Ethernet PHY. Please verify the initialization sequence\r\n");
			}
		}
	}
}

u32_t phy_setup (XEmacPs *xemacpsp, u32_t phy_addr)
{
	u32_t link_speed;
	u16_t regval;
	u32_t conv_present = 0;
	u32_t convspeeddupsetting = 0;
	u32_t convphyaddr = 0;

#ifdef XPAR_GMII2RGMIICON_0N_ETH0_ADDR
	convphyaddr = XPAR_GMII2RGMIICON_0N_ETH0_ADDR;
	conv_present = 1;
#endif
#ifdef XPAR_GMII2RGMIICON_0N_ETH1_ADDR
	convphyaddr = XPAR_GMII2RGMIICON_0N_ETH1_ADDR;
	conv_present = 1;
#endif

#ifdef  CONFIG_LINKSPEED_AUTODETECT
	link_speed = get_IEEE_phy_speed(xemacpsp, phy_addr);
	if (link_speed == 1000) {
		SetUpSLCRDivisors(xemacpsp->Config.BaseAddress,1000);
		convspeeddupsetting = XEMACPS_GMII2RGMII_SPEED1000_FD;
	} else if (link_speed == 100) {
		SetUpSLCRDivisors(xemacpsp->Config.BaseAddress,100);
		convspeeddupsetting = XEMACPS_GMII2RGMII_SPEED100_FD;
	} else if (link_speed != XST_FAILURE){
		SetUpSLCRDivisors(xemacpsp->Config.BaseAddress,10);
		convspeeddupsetting = XEMACPS_GMII2RGMII_SPEED10_FD;
	} else {
		xil_printf("Phy setup error \r\n");
		return XST_FAILURE;
	}
#elif	defined(CONFIG_LINKSPEED1000)
	SetUpSLCRDivisors(xemacpsp->Config.BaseAddress,1000);
	link_speed = 1000;
	configure_IEEE_phy_speed(xemacpsp, phy_addr, link_speed);
	convspeeddupsetting = XEMACPS_GMII2RGMII_SPEED1000_FD;
	sleep(1);
#elif	defined(CONFIG_LINKSPEED100)
	SetUpSLCRDivisors(xemacpsp->Config.BaseAddress,100);
	link_speed = 100;
	configure_IEEE_phy_speed(xemacpsp, phy_addr, link_speed);
	convspeeddupsetting = XEMACPS_GMII2RGMII_SPEED100_FD;
	sleep(1);
#elif	defined(CONFIG_LINKSPEED10)
	SetUpSLCRDivisors(xemacpsp->Config.BaseAddress,10);
	link_speed = 10;
	configure_IEEE_phy_speed(xemacpsp, phy_addr, link_speed);
	convspeeddupsetting = XEMACPS_GMII2RGMII_SPEED10_FD;
	sleep(1);
#endif
	if (conv_present) {
		XEmacPs_PhyWrite(xemacpsp, convphyaddr,
		XEMACPS_GMII2RGMII_REG_NUM, convspeeddupsetting);
	}

	xil_printf("link speed for phy address %d: %d\r\n", phy_addr, link_speed);
	return link_speed;
}

static u32_t get_TI_phy_speed(XEmacPs *xemacpsp, u32_t phy_addr)
{
	u16_t temp;
	u16_t control;
	u16_t status;
	u16_t status_speed;
	u32_t timeout_counter = 0;
	u32_t temp_speed;
	u32_t phyregtemp;
	int i;
	u32_t RetStatus;

	xil_printf("Start PHY autonegotiation \r\n");

	XEmacPs_PhyRead(xemacpsp, phy_addr, 0x1F, (u16_t *)&phyregtemp);
	phyregtemp |= 0x4000;
	XEmacPs_PhyWrite(xemacpsp, phy_addr, 0x1F, phyregtemp);
	RetStatus = XEmacPs_PhyRead(xemacpsp, phy_addr, 0x1F, (u16_t *)&phyregtemp);
	if (RetStatus != XST_SUCCESS) {
		xil_printf("Error during sw reset \n\r");
		return XST_FAILURE;
	}

	XEmacPs_PhyRead(xemacpsp, phy_addr, 0, (u16_t *)&phyregtemp);
	phyregtemp |= 0x8000;
	XEmacPs_PhyWrite(xemacpsp, phy_addr, 0, phyregtemp);

	/*
	 * Delay
	 */
	for(i=0;i<1000000000;i++);

	RetStatus = XEmacPs_PhyRead(xemacpsp, phy_addr, 0, (u16_t *)&phyregtemp);
	if (RetStatus != XST_SUCCESS) {
		xil_printf("Error during reset \n\r");
		return XST_FAILURE;
	}

	/* FIFO depth */
	XEmacPs_PhyWrite(xemacpsp, phy_addr, 0x10, 0x5048);
	XEmacPs_PhyWrite(xemacpsp, phy_addr, 0x10, phyregtemp);
	RetStatus = XEmacPs_PhyRead(xemacpsp, phy_addr, 0x10, (u16_t *)&phyregtemp);
	if (RetStatus != XST_SUCCESS) {
		xil_printf("Error writing to 0x10 \n\r");
		return XST_FAILURE;
	}

	/* TX/RX tuning */
	/* Write to PHY_RGMIIDCTL */
	XEmacPs_PhyWrite(xemacpsp, phy_addr, PHY_REGCR, PHY_REGCR_ADDR);
	XEmacPs_PhyWrite(xemacpsp, phy_addr, PHY_ADDAR, PHY_RGMIIDCTL);
	XEmacPs_PhyWrite(xemacpsp, phy_addr, PHY_REGCR, PHY_REGCR_DATA);
	RetStatus = XEmacPs_PhyWrite(xemacpsp, phy_addr, PHY_ADDAR, 0xA8);
	if (RetStatus != XST_SUCCESS) {
		xil_printf("Error in tuning");
		return XST_FAILURE;
	}

	/* Read PHY_RGMIIDCTL */
	XEmacPs_PhyWrite(xemacpsp, phy_addr, PHY_REGCR, PHY_REGCR_ADDR);
	XEmacPs_PhyWrite(xemacpsp, phy_addr, PHY_ADDAR, PHY_RGMIIDCTL);
	XEmacPs_PhyWrite(xemacpsp, phy_addr, PHY_REGCR, PHY_REGCR_DATA);
	RetStatus = XEmacPs_PhyRead(xemacpsp, phy_addr, PHY_ADDAR, (u16_t *)&phyregtemp);
	if (RetStatus != XST_SUCCESS) {
		xil_printf("Error in tuning");
		return XST_FAILURE;
	}

	/* Write PHY_RGMIICTL */
	XEmacPs_PhyWrite(xemacpsp, phy_addr, PHY_REGCR, PHY_REGCR_ADDR);
	XEmacPs_PhyWrite(xemacpsp, phy_addr, PHY_ADDAR, PHY_RGMIICTL);
	XEmacPs_PhyWrite(xemacpsp, phy_addr, PHY_REGCR, PHY_REGCR_DATA);
	RetStatus = XEmacPs_PhyWrite(xemacpsp, phy_addr, PHY_ADDAR, 0xD3);
	if (RetStatus != XST_SUCCESS) {
		xil_printf("Error in tuning");
		return XST_FAILURE;
	}

	/* Read PHY_RGMIICTL */
	XEmacPs_PhyWrite(xemacpsp, phy_addr, PHY_REGCR, PHY_REGCR_ADDR);
	XEmacPs_PhyWrite(xemacpsp, phy_addr, PHY_ADDAR, PHY_RGMIICTL);
	XEmacPs_PhyWrite(xemacpsp, phy_addr, PHY_REGCR, PHY_REGCR_DATA);
	RetStatus = XEmacPs_PhyRead(xemacpsp, phy_addr, PHY_ADDAR, (u16_t *)&phyregtemp);
	if (RetStatus != XST_SUCCESS) {
		xil_printf("Error in tuning");
		return XST_FAILURE;
	}

	XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, &control);
	control |= IEEE_ASYMMETRIC_PAUSE_MASK;
	control |= IEEE_PAUSE_MASK;
	control |= ADVERTISE_100;
	control |= ADVERTISE_10;
	XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, control);

	XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET,
					&control);
	control |= ADVERTISE_1000;
	XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET,
					control);

	XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
	control |= IEEE_CTRL_AUTONEGOTIATE_ENABLE;
	control |= IEEE_STAT_AUTONEGOTIATE_RESTART;
	XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, control);

	XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
	XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_STATUS_REG_OFFSET, &status);

	xil_printf("Waiting for PHY to complete autonegotiation.\r\n");

	while ( !(status & IEEE_STAT_AUTONEGOTIATE_COMPLETE) ) {
		sleep(1);
		timeout_counter++;

		if (timeout_counter == 30) {
			xil_printf("Auto negotiation error \r\n");
			return XST_FAILURE;
		}
		XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_STATUS_REG_OFFSET, &status);
	}
	xil_printf("autonegotiation complete \r\n");

	XEmacPs_PhyRead(xemacpsp, phy_addr, PHY_STS, &status_speed);
	if ((status_speed & 0xC000) == 0x8000) {
		return 1000;
	} else if ((status_speed & 0xC000) == 0x4000) {
		return 100;
	} else {
		return 10;
	}

	return XST_SUCCESS;
}

static u32_t get_Marvell_phy_speed(XEmacPs *xemacpsp, u32_t phy_addr)
{
	u16_t temp;
	u16_t control;
	u16_t status;
	u16_t status_speed;
	u32_t timeout_counter = 0;
	u32_t temp_speed;
	u32_t phyregtemp;

	xil_printf("Start PHY autonegotiation \r\n");

	XEmacPs_PhyWrite(xemacpsp,phy_addr, IEEE_PAGE_ADDRESS_REGISTER, 2);
	XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_CONTROL_REG_MAC, &control);
	control |= IEEE_RGMII_TXRX_CLOCK_DELAYED_MASK;
	XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_CONTROL_REG_MAC, control);

	XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_PAGE_ADDRESS_REGISTER, 0);

	XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, &control);
	control |= IEEE_ASYMMETRIC_PAUSE_MASK;
	control |= IEEE_PAUSE_MASK;
	control |= ADVERTISE_100;
	control |= ADVERTISE_10;
	XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, control);

	XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET,
					&control);
	control |= ADVERTISE_1000;
	XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET,
					control);

	XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_PAGE_ADDRESS_REGISTER, 0);
	XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_COPPER_SPECIFIC_CONTROL_REG,
																&control);
	control |= (7 << 12);	/* max number of gigabit attempts */
	control |= (1 << 11);	/* enable downshift */
	XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_COPPER_SPECIFIC_CONTROL_REG,
																control);
	XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
	control |= IEEE_CTRL_AUTONEGOTIATE_ENABLE;
	control |= IEEE_STAT_AUTONEGOTIATE_RESTART;
	XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, control);

	XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
	control |= IEEE_CTRL_RESET_MASK;
	XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, control);

	while (1) {
		XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
		if (control & IEEE_CTRL_RESET_MASK)
			continue;
		else
			break;
	}

	XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_STATUS_REG_OFFSET, &status);

	xil_printf("Waiting for PHY to complete autonegotiation.\r\n");

	while ( !(status & IEEE_STAT_AUTONEGOTIATE_COMPLETE) ) {
		sleep(1);
		XEmacPs_PhyRead(xemacpsp, phy_addr,
						IEEE_COPPER_SPECIFIC_STATUS_REG_2,  &temp);
		timeout_counter++;

		if (timeout_counter == 30) {
			xil_printf("Auto negotiation error \r\n");
			return XST_FAILURE;
		}
		XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_STATUS_REG_OFFSET, &status);
	}
	xil_printf("autonegotiation complete \r\n");

	XEmacPs_PhyRead(xemacpsp, phy_addr,IEEE_SPECIFIC_STATUS_REG,
					&status_speed);
	if (status_speed & 0x400) {
		temp_speed = status_speed & IEEE_SPEED_MASK;

		if (temp_speed == IEEE_SPEED_1000)
			return 1000;
		else if(temp_speed == IEEE_SPEED_100)
			return 100;
		else
			return 10;
	}

	return XST_SUCCESS;
}

static u32_t get_IEEE_phy_speed(XEmacPs *xemacpsp, u32_t phy_addr)
{
	u16_t phy_identity;
	u32_t RetStatus;

	XEmacPs_PhyRead(xemacpsp, phy_addr, PHY_IDENTIFIER_1_REG,
					&phy_identity);
	if (phy_identity == PHY_TI_IDENTIFIER) {
		RetStatus = get_TI_phy_speed(xemacpsp, phy_addr);
	} else {
		RetStatus = get_Marvell_phy_speed(xemacpsp, phy_addr);
	}

	return RetStatus;
}

static u32_t configure_IEEE_phy_speed(XEmacPs *xemacpsp, u32_t phy_addr, u32_t speed)
{
	u16_t control;
	u16_t autonereg;

	XEmacPs_PhyWrite(xemacpsp,phy_addr, IEEE_PAGE_ADDRESS_REGISTER, 2);
	XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_CONTROL_REG_MAC, &control);
	control |= IEEE_RGMII_TXRX_CLOCK_DELAYED_MASK;
	XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_CONTROL_REG_MAC, control);

	XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_PAGE_ADDRESS_REGISTER, 0);

	XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, &autonereg);
	autonereg |= IEEE_ASYMMETRIC_PAUSE_MASK;
	autonereg |= IEEE_PAUSE_MASK;
	XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, autonereg);

	XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
	control &= ~IEEE_CTRL_LINKSPEED_1000M;
	control &= ~IEEE_CTRL_LINKSPEED_100M;
	control &= ~IEEE_CTRL_LINKSPEED_10M;

	if (speed == 1000) {
		control |= IEEE_CTRL_LINKSPEED_1000M;

		/* Dont advertise PHY speed of 100 Mbps */
		XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, &autonereg);
		autonereg &= (~ADVERTISE_100);
		XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, autonereg);

		/* Dont advertise PHY speed of 10 Mbps */
		XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, &autonereg);
		autonereg &= (~ADVERTISE_10);
		XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, autonereg);

		/* Advertise PHY speed of 1000 Mbps */
		XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET, &autonereg);
		autonereg |= ADVERTISE_1000;
		XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET, autonereg);
	}

	else if (speed == 100) {
		control |= IEEE_CTRL_LINKSPEED_100M;

		/* Dont advertise PHY speed of 1000 Mbps */
		XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET, &autonereg);
		autonereg &= (~ADVERTISE_1000);
		XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET, autonereg);

		/* Dont advertise PHY speed of 10 Mbps */
		XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, &autonereg);
		autonereg &= (~ADVERTISE_10);
		XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, autonereg);

		/* Advertise PHY speed of 100 Mbps */
		XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, &autonereg);
		autonereg |= ADVERTISE_100;
		XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, autonereg);
	}

	else if (speed == 10) {
		control |= IEEE_CTRL_LINKSPEED_10M;

		/* Dont advertise PHY speed of 1000 Mbps */
		XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET, &autonereg);
		autonereg &= (~ADVERTISE_1000);
		XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET, autonereg);

		/* Dont advertise PHY speed of 100 Mbps */
		XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, &autonereg);
		autonereg &= (~ADVERTISE_100);
		XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, autonereg);

		/* Advertise PHY speed of 10 Mbps */
		XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, &autonereg);
		autonereg |= ADVERTISE_10;
		XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, autonereg);
	}

	XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET,
											control | IEEE_CTRL_RESET_MASK);
	{
		volatile s32_t wait;
		for (wait=0; wait < 100000; wait++);
	}
	return 0;
}
#endif /*PCM_PMA_CORE_PRESENT*/

static void SetUpSLCRDivisors(u32_t mac_baseaddr, s32_t speed)
{
	volatile u32_t slcrBaseAddress;
	u32_t SlcrDiv0;
	u32_t SlcrDiv1;
	u32_t SlcrTxClkCntrl;
	u32_t gigeversion;

	gigeversion = ((Xil_In32(mac_baseaddr + 0xFC)) >> 16) & 0xFFF;
	if (gigeversion == 2) {

		*(volatile u32_t *)(SLCR_UNLOCK_ADDR) = SLCR_UNLOCK_KEY_VALUE;

		if (mac_baseaddr == ZYNQ_EMACPS_0_BASEADDR) {
			slcrBaseAddress = SLCR_GEM0_CLK_CTRL_ADDR;
		} else {
			slcrBaseAddress = SLCR_GEM1_CLK_CTRL_ADDR;
		}
		if (speed == 1000) {
			if (mac_baseaddr == XPAR_XEMACPS_0_BASEADDR) {
#ifdef XPAR_PS7_ETHERNET_0_ENET_SLCR_1000MBPS_DIV0
				SlcrDiv0 = XPAR_PS7_ETHERNET_0_ENET_SLCR_1000MBPS_DIV0;
				SlcrDiv1 = XPAR_PS7_ETHERNET_0_ENET_SLCR_1000MBPS_DIV1;
#endif
			} else {
#ifdef XPAR_PS7_ETHERNET_1_ENET_SLCR_1000MBPS_DIV0
				SlcrDiv0 = XPAR_PS7_ETHERNET_1_ENET_SLCR_1000MBPS_DIV0;
				SlcrDiv1 = XPAR_PS7_ETHERNET_1_ENET_SLCR_1000MBPS_DIV1;
#endif
			}
		} else if (speed == 100) {
			if (mac_baseaddr == XPAR_XEMACPS_0_BASEADDR) {
#ifdef XPAR_PS7_ETHERNET_0_ENET_SLCR_100MBPS_DIV0
				SlcrDiv0 = XPAR_PS7_ETHERNET_0_ENET_SLCR_100MBPS_DIV0;
				SlcrDiv1 = XPAR_PS7_ETHERNET_0_ENET_SLCR_100MBPS_DIV1;
#endif
			} else {
#ifdef XPAR_PS7_ETHERNET_1_ENET_SLCR_100MBPS_DIV0
				SlcrDiv0 = XPAR_PS7_ETHERNET_1_ENET_SLCR_100MBPS_DIV0;
				SlcrDiv1 = XPAR_PS7_ETHERNET_1_ENET_SLCR_100MBPS_DIV1;
#endif
			}
		} else {
			if (mac_baseaddr == XPAR_XEMACPS_0_BASEADDR) {
#ifdef XPAR_PS7_ETHERNET_0_ENET_SLCR_10MBPS_DIV0
				SlcrDiv0 = XPAR_PS7_ETHERNET_0_ENET_SLCR_10MBPS_DIV0;
				SlcrDiv1 = XPAR_PS7_ETHERNET_0_ENET_SLCR_10MBPS_DIV1;
#endif
			} else {
#ifdef XPAR_PS7_ETHERNET_1_ENET_SLCR_10MBPS_DIV0
				SlcrDiv0 = XPAR_PS7_ETHERNET_1_ENET_SLCR_10MBPS_DIV0;
				SlcrDiv1 = XPAR_PS7_ETHERNET_1_ENET_SLCR_10MBPS_DIV1;
#endif
			}
		}
		SlcrTxClkCntrl = *(volatile u32_t *)(UINTPTR)(slcrBaseAddress);
		SlcrTxClkCntrl &= EMACPS_SLCR_DIV_MASK;
		SlcrTxClkCntrl |= (SlcrDiv1 << 20);
		SlcrTxClkCntrl |= (SlcrDiv0 << 8);
		*(volatile u32_t *)(UINTPTR)(slcrBaseAddress) = SlcrTxClkCntrl;
		*(volatile u32_t *)(SLCR_LOCK_ADDR) = SLCR_LOCK_KEY_VALUE;
	}
	return;
}
