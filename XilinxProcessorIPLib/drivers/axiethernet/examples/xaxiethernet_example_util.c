/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
*
* @file xaxiethernet_example_util.c
*
* This file implements the utility functions for the Axi Ethernet example code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a asa  6/30/10 First release based on the ll temac driver
* 3.01a srt  02/03/13 Added support for SGMII mode (CR 676793).
*	     02/14/13 Added support for Zynq (CR 681136).
* 3.02a srt  04/24/13 Modified parameter *_SGMII_PHYADDR to *_PHYADDR, the
*                     config parameter C_PHYADDR applies to SGMII/1000BaseX
*	              modes of operation and added support for 1000BaseX mode
*		      (CR 704195). Added function *_ConfigureInternalPhy()
*		      for this purpose.
*	     04/24/13 Added support for RGMII mode.
* 3.02a srt  08/06/13 Fixed CR 717949:
*			Configures external Marvel 88E1111 PHY based on the
*			axi ethernet physical interface type and allows to
*			operate in specific interface mode without changing
*			jumpers on the Microblaze board.
* 5.4	adk  07/12/16  Added Support for TI PHY DP83867.
*       ms   04/05/17  Added tabspace for return statements in functions
*                      for proper documentation while generating doxygen.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xaxiethernet_example.h"
#if !defined (__MICROBLAZE__) && !defined(__PPC__)
#include "sleep.h"
#endif

/************************** Variable Definitions ****************************/

/*
 * Local MAC address
 */
char AxiEthernetMAC[6] = { 0x00, 0x0A, 0x35, 0x01, 0x02, 0x03 };

/******************************************************************************/
/**
*
* Set the MAC addresses in the frame.
*
* @param	FramePtr is the pointer to the frame.
* @param	DestAddr is the Destination MAC address.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void AxiEthernetUtilFrameHdrFormatMAC(EthernetFrame *FramePtr, char *DestAddr)
{
	char *Frame = (char *) FramePtr;
	char *SourceAddress = AxiEthernetMAC;
	int Index;

	if (Padding) {
		for (Index = 0; Index < 8; Index++) {
			*Frame++ = 0;
		}
	}

	/*
	 * Destination address
	 */
	for (Index = 0; Index < XAE_MAC_ADDR_SIZE; Index++) {
		*Frame++ = *DestAddr++;
	}

	/*
	 * Source address
	 */
	for (Index = 0; Index < XAE_MAC_ADDR_SIZE; Index++) {
		*Frame++ = *SourceAddress++;
	}
}

/******************************************************************************/
/**
*
* Set the frame type for the specified frame.
*
* @param	FramePtr is the pointer to the frame.
* @param	FrameType is the Type to set in frame.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void AxiEthernetUtilFrameHdrFormatType(EthernetFrame *FramePtr, u16 FrameType)
{
	char *Frame = (char *) FramePtr;

	/*
	 * Increment to type field
	 */
	Frame = Frame + 12 + Padding;

	FrameType = Xil_Htons(FrameType);
	/*
	 * Set the type
	 */
	*(u16 *) Frame = FrameType;
}

/******************************************************************************/
/**
* This function places a pattern in the payload section of a frame. The pattern
* is a  8 bit incrementing series of numbers starting with 0.
* Once the pattern reaches 256, then the pattern changes to a 16 bit
* incrementing pattern:
* <pre>
*   0, 1, 2, ... 254, 255, 00, 00, 00, 01, 00, 02, ...
* </pre>
*
* @param	FramePtr is a pointer to the frame to change.
* @param	PayloadSize is the number of bytes in the payload that will be
*		set.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void AxiEthernetUtilFrameSetPayloadData(EthernetFrame *FramePtr,
							int PayloadSize)
{
	unsigned BytesLeft = PayloadSize;
	u8 *Frame;
	u16 Counter = 0;

	/*
	 * Set the frame pointer to the start of the payload area
	 */
	Frame = (u8 *) FramePtr + XAE_HDR_SIZE + Padding;

	/*
	 * Insert 8 bit incrementing pattern
	 */
	while (BytesLeft && (Counter < 256)) {
		*Frame++ = (u8) Counter++;
		BytesLeft--;
	}

	/*
	 * Switch to 16 bit incrementing pattern
	 */
	while (BytesLeft) {
		*Frame++ = (u8) (Counter >> 8);	/* high */
		BytesLeft--;

		if (!BytesLeft)
			break;

		*Frame++ = (u8) Counter++;	/* low */
		BytesLeft--;
	}
}

/******************************************************************************/
/**
*
* Set the frame VLAN info for the specified frame.
*
* @param	FramePtr is the pointer to the frame.
* @param	VlanNumber is the VlanValue insertion position to set in frame.
* @param	Vid  is the 4 bytes Vlan value (TPID, Priority, CFI, VID)
*		to be set in frame.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void AxiEthernetUtilFrameHdrVlanFormatVid(EthernetFrame *FramePtr,
						u32 VlanNumber,	u32 Vid)
{
	char *Frame = (char *) FramePtr;

	/*
	 * Increment to type field
	 */
	Frame = Frame + 12 + (VlanNumber * 4);

	Vid = Xil_Htonl(Vid);

	/*
	 * Set the type
	 */
	*(u32 *) Frame = Vid;
}

/******************************************************************************/
/**
*
* Set the frame type for the specified frame.
*
* @param	FramePtr is the pointer to the frame.
* @param	FrameType is the Type to set in frame.
* @param	VlanNumber is the VLAN friendly adjusted insertion position to
*		set in frame.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void AxiEthernetUtilFrameHdrVlanFormatType(EthernetFrame *FramePtr,
						u16 FrameType, u32 VlanNumber)
{
	char *Frame = (char *) FramePtr;

	/*
	 * Increment to type field
	 */
	Frame = Frame + 12 + (VlanNumber * 4);

	FrameType = Xil_Htons(FrameType);

	/*
	 * Set the type
	 */
	*(u16 *) Frame = FrameType;
}

/******************************************************************************/
/**
* This function places a pattern in the payload section of a frame. The pattern
* is a  8 bit incrementing series of numbers starting with 0.
* Once the pattern reaches 256, then the pattern changes to a 16 bit
* incrementing pattern:
* <pre>
*   0, 1, 2, ... 254, 255, 00, 00, 00, 01, 00, 02, ...
* </pre>
*
* @param	FramePtr is a pointer to the frame to change.
* @param	PayloadSize is the number of bytes in the payload that will be set.
* @param	VlanNumber is the VLAN friendly adjusted insertion position to
*		set in frame.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void AxiEthernetUtilFrameSetVlanPayloadData(EthernetFrame *FramePtr,
					int PayloadSize, u32 VlanNumber)
{
	unsigned BytesLeft = PayloadSize;
	u8 *Frame;
	u16 Counter = 0;

	/*
	 * Set the frame pointer to the start of the payload area
	 */
	Frame = (u8 *) FramePtr + XAE_HDR_SIZE + (VlanNumber * 4);

	/*
	 * Insert 8 bit incrementing pattern
	 */
	while (BytesLeft && (Counter < 256)) {
		*Frame++ = (u8) Counter++;
		BytesLeft--;
	}

	/*
	 * Switch to 16 bit incrementing pattern
	 */
	while (BytesLeft) {
		*Frame++ = (u8) (Counter >> 8);	/* high */
		BytesLeft--;

		if (!BytesLeft)
			break;

		*Frame++ = (u8) Counter++;	/* low */
		BytesLeft--;
	}
}

/******************************************************************************/
/**
* This function verifies the frame data against a CheckFrame.
*
* Validation occurs by comparing the ActualFrame to the header of the
* CheckFrame. If the headers match, then the payload of ActualFrame is
* verified for the same pattern Util_FrameSetPayloadData() generates.
*
* @param	CheckFrame is a pointer to a frame containing the 14 byte header
*		that should be present in the ActualFrame parameter.
* @param	ActualFrame is a pointer to a frame to validate.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE in case of failure.
*
* @note		None.
*
******************************************************************************/
int AxiEthernetUtilFrameVerify(EthernetFrame * CheckFrame,
			 EthernetFrame * ActualFrame)
{
	unsigned char *CheckPtr = (unsigned char *) CheckFrame;
	unsigned char *ActualPtr = (unsigned char *) ActualFrame;
	u16 BytesLeft;
	u16 Counter;
	int Index;

	CheckPtr = CheckPtr + Padding;
	ActualPtr = ActualPtr + Padding;

	/*
	 * Compare the headers
	 */
	for (Index = 0; Index < XAE_HDR_SIZE; Index++) {
		if (CheckPtr[Index] != ActualPtr[Index]) {
			return XST_FAILURE;
		}
	}

	Index = 0;

	BytesLeft = *(u16 *) &ActualPtr[12];
	BytesLeft = Xil_Ntohs(BytesLeft);
	/*
	 * Get the length of the payload, do not use VLAN TPID here.
	 * TPID needs to be verified.
	 */
	while ((0x8100 == BytesLeft) || (0x88A8 == BytesLeft) ||
	       (0x9100 == BytesLeft) || (0x9200 == BytesLeft)) {
		Index++;
		BytesLeft = *(u16 *) &ActualPtr[12+(4*Index)];
		BytesLeft = Xil_Ntohs(BytesLeft);
	}

	/*
	 * Validate the payload
	 */
	Counter = 0;
	ActualPtr = &ActualPtr[14+(4*Index)];

	/*
	 * Check 8 bit incrementing pattern
	 */
	while (BytesLeft && (Counter < 256)) {
		if (*ActualPtr++ != (u8) Counter++) {

			return XST_FAILURE;
		}
		BytesLeft--;
	}

	/*
	 * Check 16 bit incrementing pattern
	 */
	while (BytesLeft) {
		if (*ActualPtr++ != (u8) (Counter >> 8)) {	/* high */
			return XST_FAILURE;
		}

		BytesLeft--;

		if (!BytesLeft)
			break;

		if (*ActualPtr++ != (u8) Counter++) {	/* low */
			return XST_FAILURE;
		}

		BytesLeft--;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
* This function sets all bytes of a frame to 0.
*
* @param	FramePtr is a pointer to the frame itself.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void AxiEthernetUtilFrameMemClear(EthernetFrame * FramePtr)
{
	u32 *Data32Ptr = (u32 *) FramePtr;
	u32 WordsLeft = sizeof(EthernetFrame) / sizeof(u32);

	/*
	 * Frame should be an integral number of words
	 */
	while (WordsLeft--) {
		*Data32Ptr++ = 0;
	}
}

/******************************************************************************/
/**
*
* This function detects the PHY address by looking for successful MII status
* register contents (PHY register 1). It looks for a PHY that supports
* auto-negotiation and 10Mbps full-duplex and half-duplex.  So, this code
* won't work for PHYs that don't support those features, but it's a bit more
* general purpose than matching a specific PHY manufacturer ID.
*
* Note also that on some (older) Xilinx ML4xx boards, PHY address 0 does not
* properly respond to this query.  But, since the default is 0 and assuming
* no other address responds, then it seems to work OK.
*
* @param	The Axi Ethernet driver instance
*
* @return	The address of the PHY (defaults to 0 if none detected)
*
* @note		None.
*
******************************************************************************/
/* Use MII register 1 (MII status register) to detect PHY */
#define PHY_DETECT_REG  1

/* Mask used to verify certain PHY features (or register contents)
 * in the register above:
 *  0x1000: 10Mbps full duplex support
 *  0x0800: 10Mbps half duplex support
 *  0x0008: Auto-negotiation support
 */
#define PHY_DETECT_MASK 0x1808

u32 AxiEthernetDetectPHY(XAxiEthernet * AxiEthernetInstancePtr)
{
	u16 PhyReg;
	int PhyAddr;

	for (PhyAddr = 31; PhyAddr >= 0; PhyAddr--) {
		XAxiEthernet_PhyRead(AxiEthernetInstancePtr, PhyAddr,
						PHY_DETECT_REG, &PhyReg);

		if ((PhyReg != 0xFFFF) &&
		   ((PhyReg & PHY_DETECT_MASK) == PHY_DETECT_MASK)) {
			/* Found a valid PHY address */
			return PhyAddr;
		}
	}

	return 0;		/* Default to zero */
}

/******************************************************************************/
/**
* Set PHY to loopback mode. This works with the marvell PHY common on ML40x
* evaluation boards
*
* @param Speed is the loopback speed 10, 100, or 1000 Mbit
*
******************************************************************************/
/* IEEE PHY Specific definitions */
#define PHY_R0_CTRL_REG		0
#define PHY_R3_PHY_IDENT_REG	3

#define PHY_R0_RESET         0x8000
#define PHY_R0_LOOPBACK      0x4000
#define PHY_R0_ANEG_ENABLE   0x1000
#define PHY_R0_DFT_SPD_MASK  0x2040
#define PHY_R0_DFT_SPD_10    0x0000
#define PHY_R0_DFT_SPD_100   0x2000
#define PHY_R0_DFT_SPD_1000  0x0040
#define PHY_R0_DFT_SPD_2500  0x0040
#define PHY_R0_ISOLATE       0x0400

/* Marvel PHY 88E1111 Specific definitions */
#define PHY_R20_EXTND_CTRL_REG	20
#define PHY_R27_EXTND_STS_REG	27

#define PHY_R20_DFT_SPD_10    	0x20
#define PHY_R20_DFT_SPD_100   	0x50
#define PHY_R20_DFT_SPD_1000  	0x60
#define PHY_R20_RX_DLY		0x80

#define PHY_R27_MAC_CONFIG_GMII      0x000F
#define PHY_R27_MAC_CONFIG_MII       0x000F
#define PHY_R27_MAC_CONFIG_RGMII     0x000B
#define PHY_R27_MAC_CONFIG_SGMII     0x0004

/* Marvel PHY 88E1116R Specific definitions */
#define PHY_R22_PAGE_ADDR_REG	22
#define PHY_PG2_R21_CTRL_REG	21

#define PHY_REG21_10      0x0030
#define PHY_REG21_100     0x2030
#define PHY_REG21_1000    0x0070

/* Marvel PHY flags */
#define MARVEL_PHY_88E1111_MODEL	0xC0
#define MARVEL_PHY_88E1116R_MODEL	0x240
#define PHY_MODEL_NUM_MASK		0x3F0

/* TI PHY flags */
#define TI_PHY_IDENTIFIER		0x2000
#define TI_PHY_MODEL			0x230
#define TI_PHY_CR			0xD
#define TI_PHY_PHYCTRL			0x10
#define TI_PHY_CR_SGMII_EN		0x0800
#define TI_PHY_ADDDR			0xE
#define TI_PHY_CFGR2			0x14
#define TI_PHY_SGMIITYPE		0xD3
#define TI_PHY_CFGR2_SGMII_AUTONEG_EN	0x0080
#define TI_PHY_SGMIICLK_EN		0x4000
#define TI_PHY_CR_DEVAD_EN		0x001F
#define TI_PHY_CR_DEVAD_DATAEN		0x4000

/******************************************************************************/
/**
*
* This function sets the PHY to loopback mode. This works with the marvell PHY
* common on ML40x evaluation boards.
*
* @param	AxiEthernetInstancePtr is a pointer to the instance of the
*		AxiEthernet component.
* @param	Speed is the loopback speed 10, 100, or 1000 Mbit.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE, in case of failure..
*
* @note		None.
*
******************************************************************************/
int AxiEthernetUtilEnterLoopback(XAxiEthernet *AxiEthernetInstancePtr,
								int Speed)
{
	u16 PhyReg0;
	signed int PhyAddr;
	u8 PhyType;
	u16 PhyModel;
	u16 PhyReg20;	/* Extended PHY specific Register (Reg 20)
			   of Marvell 88E1111 PHY */
	u16 PhyReg21;	/* Control Register MAC (Reg 21)
			   of Marvell 88E1116R PHY */

	/* Get the Phy Interface */
	PhyType = XAxiEthernet_GetPhysicalInterface(AxiEthernetInstancePtr);

	/* Detect the PHY address */
	if (PhyType != XAE_PHY_TYPE_1000BASE_X) {
		PhyAddr = AxiEthernetDetectPHY(AxiEthernetInstancePtr);
	} else {
		PhyAddr = XPAR_AXIETHERNET_0_PHYADDR;
	}

	XAxiEthernet_PhyRead(AxiEthernetInstancePtr, PhyAddr,
				PHY_R3_PHY_IDENT_REG, &PhyModel);
	PhyModel = PhyModel & PHY_MODEL_NUM_MASK;

	/* Clear the PHY of any existing bits by zeroing this out */
	PhyReg0 = PhyReg20 = PhyReg21 = 0;

	switch (Speed) {
	case XAE_SPEED_10_MBPS:
		PhyReg0 |= PHY_R0_DFT_SPD_10;
		PhyReg20 |= PHY_R20_DFT_SPD_10;
		PhyReg21 |= PHY_REG21_10;
		break;

	case XAE_SPEED_100_MBPS:
		PhyReg0 |= PHY_R0_DFT_SPD_100;
		PhyReg20 |= PHY_R20_DFT_SPD_100;
		PhyReg21 |= PHY_REG21_100;
		break;

	case XAE_SPEED_1000_MBPS:
		PhyReg0 |= PHY_R0_DFT_SPD_1000;
		PhyReg20 |= PHY_R20_DFT_SPD_1000;
		PhyReg21 |= PHY_REG21_1000;
		break;

	case XAE_SPEED_2500_MBPS:
		PhyReg0 |= PHY_R0_DFT_SPD_2500;
		PhyReg20 |= PHY_R20_DFT_SPD_1000;
		PhyReg21 |= PHY_REG21_1000;
		break;

	default:
		AxiEthernetUtilErrorTrap("Intg_LinkSpeed not 10, 100, or 1000 mbps");
		return XST_FAILURE;
	}

	/* RGMII mode Phy specific registers initialization */
	if ((PhyType == XAE_PHY_TYPE_RGMII_2_0) ||
		(PhyType == XAE_PHY_TYPE_RGMII_1_3)) {
		if (PhyModel == MARVEL_PHY_88E1111_MODEL) {
			PhyReg20 |= PHY_R20_RX_DLY;
			/*
			 * Adding Rx delay. Configuring loopback speed.
			 */
			XAxiEthernet_PhyWrite(AxiEthernetInstancePtr,
						PhyAddr, PHY_R20_EXTND_CTRL_REG,
						PhyReg20);
		} else if (PhyModel == MARVEL_PHY_88E1116R_MODEL) {
			/*
			 * Switching to PAGE2
			 */
			XAxiEthernet_PhyWrite(AxiEthernetInstancePtr,
						PhyAddr,
						PHY_R22_PAGE_ADDR_REG, 2);
			/*
			 * Adding Tx and Rx delay. Configuring loopback speed.
			 */
			XAxiEthernet_PhyWrite(AxiEthernetInstancePtr,
						PhyAddr,
						PHY_PG2_R21_CTRL_REG, PhyReg21);
			/*
			 * Switching to PAGE0
			 */
			XAxiEthernet_PhyWrite(AxiEthernetInstancePtr,
						PhyAddr,
						PHY_R22_PAGE_ADDR_REG, 0);
		}
		PhyReg0 &= (~PHY_R0_ANEG_ENABLE);
	}

	/* Configure interface modes */
	if (PhyModel == MARVEL_PHY_88E1111_MODEL) {
		if ((PhyType == XAE_PHY_TYPE_RGMII_2_0) ||
				(PhyType == XAE_PHY_TYPE_RGMII_1_3))  {
			XAxiEthernet_PhyWrite(AxiEthernetInstancePtr,
					PhyAddr, PHY_R27_EXTND_STS_REG,
					PHY_R27_MAC_CONFIG_RGMII);
		} else if (PhyType == XAE_PHY_TYPE_SGMII) {
			XAxiEthernet_PhyWrite(AxiEthernetInstancePtr,
					PhyAddr, PHY_R27_EXTND_STS_REG,
					PHY_R27_MAC_CONFIG_SGMII);
		} else if ((PhyType == XAE_PHY_TYPE_GMII) ||
				(PhyType == XAE_PHY_TYPE_MII)) {
			XAxiEthernet_PhyWrite(AxiEthernetInstancePtr,
					PhyAddr, PHY_R27_EXTND_STS_REG,
					PHY_R27_MAC_CONFIG_GMII );
		}
	}

	/* Set the speed and put the PHY in reset, then put the PHY in loopback */
	XAxiEthernet_PhyWrite(AxiEthernetInstancePtr, PhyAddr,
				PHY_R0_CTRL_REG,
				PhyReg0 | PHY_R0_RESET);
	AxiEthernetUtilPhyDelay(AXIETHERNET_PHY_DELAY_SEC);
	XAxiEthernet_PhyRead(AxiEthernetInstancePtr,PhyAddr,
				PHY_R0_CTRL_REG, &PhyReg0);
	if (!ExternalLoopback) {
		XAxiEthernet_PhyWrite(AxiEthernetInstancePtr, PhyAddr,
					PHY_R0_CTRL_REG,
					PhyReg0 | PHY_R0_LOOPBACK);
	}

	if ((PhyModel == TI_PHY_MODEL) && (PhyType == XAE_PHY_TYPE_SGMII)) {
		XAxiEthernet_PhyRead(AxiEthernetInstancePtr,PhyAddr,
				     PHY_R0_CTRL_REG, &PhyReg0);
		PhyReg0 &= (~PHY_R0_ANEG_ENABLE);
		XAxiEthernet_PhyWrite(AxiEthernetInstancePtr, PhyAddr,
				      PHY_R0_CTRL_REG, PhyReg0 );
		AxiEtherentConfigureTIPhy(AxiEthernetInstancePtr, PhyAddr);
	}

	if ((PhyType == XAE_PHY_TYPE_SGMII) ||
		(PhyType == XAE_PHY_TYPE_1000BASE_X)) {
		AxiEthernetUtilConfigureInternalPhy(AxiEthernetInstancePtr, Speed);
	}

	AxiEthernetUtilPhyDelay(1);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
*
* This function is called by example code when an error is detected. It
* can be set as a breakpoint with a debugger or it can be used to print out the
* given message if there is a UART or STDIO device.
*
* @param	Message is the text explaining the error
*
* @return	None
*
* @note		None
*
******************************************************************************/
void AxiEthernetUtilErrorTrap(char *Message)
{
	static int Count = 0;

	Count++;

#ifdef STDOUT_BASEADDRESS
	xil_printf("%s\r\n", Message);
#endif
}

/******************************************************************************/
/**
*
* For Microblaze we use an assembly loop that is roughly the same regardless of
* optimization level, although caches and memory access time can make the delay
* vary.  Just keep in mind that after resetting or updating the PHY modes,
* the PHY typically needs time to recover.
*
* @return	None
*
* @note		None
*
******************************************************************************/
void AxiEthernetUtilPhyDelay(unsigned int Seconds)
{
#if defined (__MICROBLAZE__) || defined(__PPC__)
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

/******************************************************************************/
/**
*
* This function configures the internal phy for SGMII and 1000baseX modes.
* *
* @param	AxiEthernetInstancePtr is a pointer to the instance of the
*		AxiEthernet component.
* @param	Speed is the loopback speed 10, 100, or 1000 Mbit.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE, in case of failure..
*
* @note		None.
*
******************************************************************************/
int AxiEthernetUtilConfigureInternalPhy(XAxiEthernet *AxiEthernetInstancePtr,
					int Speed)
{
	u16 PhyReg0;
	signed int PhyAddr;

	PhyAddr = XPAR_AXIETHERNET_0_PHYADDR;

	/* Clear the PHY of any existing bits by zeroing this out */
	PhyReg0 = 0;
	XAxiEthernet_PhyRead(AxiEthernetInstancePtr, PhyAddr,
				 PHY_R0_CTRL_REG, &PhyReg0);

	PhyReg0 &= (~PHY_R0_ANEG_ENABLE);
	PhyReg0 &= (~PHY_R0_ISOLATE);

	switch (Speed) {
		case XAE_SPEED_10_MBPS:
			PhyReg0 |= PHY_R0_DFT_SPD_10;
			break;
		case XAE_SPEED_100_MBPS:
			PhyReg0 |= PHY_R0_DFT_SPD_100;
			break;
		case XAE_SPEED_1000_MBPS:
			PhyReg0 |= PHY_R0_DFT_SPD_1000;
			break;
		case XAE_SPEED_2500_MBPS:
			PhyReg0 |= PHY_R0_DFT_SPD_2500;
			break;
		default:
			AxiEthernetUtilErrorTrap(
				"Intg_LinkSpeed not 10, 100, or 1000 mbps\n\r");
				return XST_FAILURE;
	}

	AxiEthernetUtilPhyDelay(1);
	XAxiEthernet_PhyWrite(AxiEthernetInstancePtr, PhyAddr,
				PHY_R0_CTRL_REG, PhyReg0);
	return XST_SUCCESS;
}

int AxiEtherentConfigureTIPhy(XAxiEthernet *AxiEthernetInstancePtr, u32 PhyAddr)
{
	u16 PhyReg14;

	/* Enable SGMII Clock */
	XAxiEthernet_PhyWrite(AxiEthernetInstancePtr, PhyAddr, TI_PHY_CR,
			      TI_PHY_CR_DEVAD_EN);
	XAxiEthernet_PhyWrite(AxiEthernetInstancePtr, PhyAddr, TI_PHY_ADDDR,
			      TI_PHY_SGMIITYPE);
	XAxiEthernet_PhyWrite(AxiEthernetInstancePtr, PhyAddr, TI_PHY_CR,
			      TI_PHY_CR_DEVAD_EN | TI_PHY_CR_DEVAD_DATAEN);
	XAxiEthernet_PhyWrite(AxiEthernetInstancePtr, PhyAddr, TI_PHY_ADDDR,
			      TI_PHY_SGMIICLK_EN);

	/* Enable SGMII */
	XAxiEthernet_PhyWrite(AxiEthernetInstancePtr, PhyAddr, TI_PHY_PHYCTRL,
	                      TI_PHY_CR_SGMII_EN);
	XAxiEthernet_PhyRead(AxiEthernetInstancePtr, PhyAddr, TI_PHY_CFGR2,
			     &PhyReg14);
	XAxiEthernet_PhyWrite(AxiEthernetInstancePtr, PhyAddr, TI_PHY_CFGR2,
			      PhyReg14 & (~TI_PHY_CFGR2_SGMII_AUTONEG_EN));
	XAxiEthernet_PhyRead(AxiEthernetInstancePtr, PhyAddr, TI_PHY_CFGR2,
			     &PhyReg14);

	return XST_SUCCESS;
}
