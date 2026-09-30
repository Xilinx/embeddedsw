/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xemacps_example_util.c
* @addtogroup emacps Overview
* @{
*
* @brief Implements utility helpers for the EmacPs examples.
*
* Implements the utility functions for the XEmacPs example code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a wsy  01/10/10 First release
* 1.00a asa  01/24/12 A new version of EmacPsUtilEnterLoopback is used for
* 		      Zynq boards. Now there are two versions of
* 		      EmacPsUtilEnterLoopback, one for PEEP and one for Zynq.
* 		      If the example is to be run  on a PEEP board, define PEEP
* 		      in xemacps_example.h.
* 1.01a asa  02/27/12 The sleep value after PHY loopback is setup is reduced
*		      for Zynq.
* 3.0   kpc  01/23/15 Removed PEEP board related code
* 3.2	hk   09/30/15 Added support for TI PHY DP83867
* 3.2   mus  02/20/16 Added support for microblaze.
* 3.9   hk   02/12/19 Use selected speed in loopback mode.
* 3.14  sk   12/23/20 Correct variable name to delay in EmacpsDelay() function
* 		      to fix the doxygen warning.
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xemacps_example.h"
#ifndef __MICROBLAZE__
#include "sleep.h"
#endif

/************************** Variable Definitions ****************************/

XEmacPs EmacPsInstance;	/* XEmacPs instance used throughout examples */

/*
 * Local MAC address
 */
char EmacPsMAC[] = { 0x00, 0x0a, 0x35, 0x01, 0x02, 0x03 };

u32 Platform;

/****************************************************************************/
/**
* @brief Sets the MAC addresses in the frame.
*
* @param[in] FramePtr is the pointer to the frame.
* @param[in] DestAddr is the Destination MAC address.
*
*****************************************************************************/
void EmacPsUtilFrameHdrFormatMAC(EthernetFrame *FramePtr, char *DestAddr)
{
	char *Frame = (char *) FramePtr;
	char *SourceAddress = EmacPsMAC;
	unsigned int Index;

	/* Destination address */
	for (Index = 0; Index < XEMACPS_MAC_ADDR_SIZE; Index++) {
		*Frame++ = *DestAddr++;
	}

	/* Source address */
	for (Index = 0; Index < XEMACPS_MAC_ADDR_SIZE; Index++) {
		*Frame++ = *SourceAddress++;
	}
}

/****************************************************************************/
/**
* @brief Sets the frame type for the specified frame.
*
* @param[in] FramePtr is the pointer to the frame.
* @param[in] FrameType is the Type to set in frame.
*
*****************************************************************************/
void EmacPsUtilFrameHdrFormatType(EthernetFrame *FramePtr, u16 FrameType)
{
	char *Frame = (char *) FramePtr;

	/*
	 * Increment to type field
	 */
	Frame = Frame + 12;
	/*
	 * Do endian swap from little to big-endian.
	 */
	FrameType = Xil_EndianSwap16(FrameType);
	/*
	 * Set the type
	 */
	*(u16 *) Frame = FrameType;
}

/****************************************************************************/
/**
* @brief Places a pattern in the payload section of a frame.
*
* The pattern is an 8-bit incrementing series of numbers starting with 0.
* Once the pattern reaches 256, the pattern changes to a 16-bit
* incrementing pattern:
* <pre>
*   0, 1, 2, ... 254, 255, 00, 00, 00, 01, 00, 02, ...
* </pre>
*
* @param[in] FramePtr is a pointer to the frame to change.
* @param[in] PayloadSize is the number of bytes in the payload that will be set.
*
*****************************************************************************/
void EmacPsUtilFrameSetPayloadData(EthernetFrame *FramePtr, u32 PayloadSize)
{
	u32 BytesLeft = PayloadSize;
	u8 *Frame;
	u16 Counter = 0;

	/*
	 * Set the frame pointer to the start of the payload area
	 */
	Frame = (u8 *) FramePtr + XEMACPS_HDR_SIZE;

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

		if (!BytesLeft) {
			break;
		}

		*Frame++ = (u8) Counter++;	/* low */
		BytesLeft--;
	}
}

/****************************************************************************/
/**
* @brief Verifies the frame data against a CheckFrame.
*
* Validation occurs by comparing the ActualFrame to the header of the
* CheckFrame. If the headers match, then the payload of ActualFrame is
* verified for the same pattern Util_FrameSetPayloadData() generates.
*
* @param[in] CheckFrame is a pointer to a frame containing the 14 byte header
*           that should be present in the ActualFrame parameter.
* @param[in] ActualFrame is a pointer to a frame to validate.
*
* @return XST_SUCCESS if successful, else XST_FAILURE.
*
*****************************************************************************/
LONG EmacPsUtilFrameVerify(EthernetFrame *CheckFrame,
			   EthernetFrame *ActualFrame)
{
	char *CheckPtr = (char *) CheckFrame;
	char *ActualPtr = (char *) ActualFrame;
	u16 BytesLeft;
	u16 Counter;
	u32 Index;

	/*
	 * Compare the headers
	 */
	for (Index = 0; Index < XEMACPS_HDR_SIZE; Index++) {
		if (CheckPtr[Index] != ActualPtr[Index]) {
			return XST_FAILURE;
		}
	}

	/*
	 * Get the length of the payload
	 */
	BytesLeft = *(u16 *) &ActualPtr[12];
	/*
	 * Do endian swap from big back to little-endian.
	 */
	BytesLeft = Xil_EndianSwap16(BytesLeft);
	/*
	 * Validate the payload
	 */
	Counter = 0;
	ActualPtr = &ActualPtr[14];

	/*
	 * Check 8 bit incrementing pattern
	 */
	while (BytesLeft && (Counter < 256)) {
		if (*ActualPtr++ != (char) Counter++) {
			return XST_FAILURE;
		}
		BytesLeft--;
	}

	/*
	 * Check 16 bit incrementing pattern
	 */
	while (BytesLeft) {
		if (*ActualPtr++ != (char) (Counter >> 8)) {	/* high */
			return XST_FAILURE;
		}

		BytesLeft--;

		if (!BytesLeft) {
			break;
		}

		if (*ActualPtr++ != (char) Counter++) {	/* low */
			return XST_FAILURE;
		}

		BytesLeft--;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief Clears the frame buffer by writing a fill pattern.
*
* @param[in] FramePtr is a pointer to the frame itself.
*
*****************************************************************************/
void EmacPsUtilFrameMemClear(EthernetFrame *FramePtr)
{
	u32 *Data32Ptr = (u32 *) FramePtr;
	u32 WordsLeft = sizeof(EthernetFrame) / sizeof(u32);

	/* frame should be an integral number of words */
	while (WordsLeft--) {
		*Data32Ptr++ = 0xDEADBEEF;
	}
}


/****************************************************************************/
/**
* @brief Copies data from source to destination for n bytes.
*
* @param[in] Destination is the targeted string to copy to.
* @param[in] Source is the source string to copy from.
* @param[in] n is number of bytes to be copied.
*
* @note Copies n bytes without checking for a null byte.
*       Behaves like strncpy() but does not stop at a null terminator.
*
*****************************************************************************/
void EmacPsUtilstrncpy(char *Destination, const char *Source, u32 n)
{
	do {
		*Destination++ = *Source++;
	}
	while (--n != 0);
}


/****************************************************************************/
/**
* @brief Sets the EmacPs MAC into local loopback mode.
*
* @param[in] EmacPsInstancePtr is the XEmacPs driver instance.
*
*****************************************************************************/
void EmacPsUtilEnterLocalLoopback(XEmacPs *EmacPsInstancePtr)
{
	u32 reg;

	reg = XEmacPs_ReadReg(EmacPsInstancePtr->Config.BaseAddress,
			      XEMACPS_NWCTRL_OFFSET);
	XEmacPs_WriteReg(EmacPsInstancePtr->Config.BaseAddress,
			 XEMACPS_NWCTRL_OFFSET,
			 reg | XEMACPS_NWCTRL_LOOPEN_MASK);
}


#define PHY_DETECT_REG1 2	/**< PHY identifier register 1 address. */
#define PHY_DETECT_REG2 3	/**< PHY identifier register 2 address. */

#define PHY_ID_MARVELL	0x141	/**< Marvell PHY OUI identifier value. */
#define PHY_ID_TI		0x2000	/**< TI PHY OUI identifier value. */

/****************************************************************************/
/**
* @brief Detects the PHY address by reading valid MII status register contents.
*
* @param[in] EmacPsInstancePtr is a pointer to the XEmacPs driver instance.
*
* @return The address of the PHY (defaults to 32 if none detected).
*
*****************************************************************************/
u32 XEmacPsDetectPHY(XEmacPs *EmacPsInstancePtr)
{
	u32 PhyAddr;
	u32 Status;
	u16 PhyReg1;
	u16 PhyReg2;

	for (PhyAddr = 0; PhyAddr <= 31; PhyAddr++) {
		Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr,
					 PHY_DETECT_REG1, &PhyReg1);

		Status |= XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr,
					  PHY_DETECT_REG2, &PhyReg2);

		if ((Status == XST_SUCCESS) &&
		    (PhyReg1 > 0x0000) && (PhyReg1 < 0xffff) &&
		    (PhyReg2 > 0x0000) && (PhyReg2 < 0xffff)) {
			/* Found a valid PHY address */
			return PhyAddr;
		}
	}

	return PhyAddr;		/* default to 32(max of iteration) */
}


#define PHY_REG0_RESET    0x8000	/**< PHY control register reset bit mask. */
#define PHY_REG0_LOOPBACK 0x4000	/**< PHY control register loopback bit mask. */
#define PHY_REG0_10       0x0100	/**< PHY control register value for 10 Mbps. */
#define PHY_REG0_100      0x2100	/**< PHY control register value for 100 Mbps. */
#define PHY_REG0_1000     0x0140	/**< PHY control register value for 1000 Mbps. */
#define PHY_REG21_10      0x0030	/**< Marvell page-2 register 21 value for 10 Mbps. */
#define PHY_REG21_100     0x2030	/**< Marvell page-2 register 21 value for 100 Mbps. */
#define PHY_REG21_1000    0x0070	/**< Marvell page-2 register 21 value for 1000 Mbps. */
#define PHY_LOOPCR		0xFE	/**< TI PHY loopback control register address. */
#define PHY_REGCR		0x0D	/**< TI PHY register control address. */
#define PHY_ADDAR		0x0E	/**< TI PHY address or data register. */
#define PHY_RGMIIDCTL	0x86	/**< TI PHY RGMII delay control register. */
#define PHY_RGMIICTL	0x32	/**< TI PHY RGMII control register. */

#define PHY_REGCR_ADDR	0x001F	/**< TI PHY REGCR address-mode select value. */
#define PHY_REGCR_DATA	0x401F	/**< TI PHY REGCR data-mode select value. */

/* RGMII RX and TX tuning values */
#define PHY_TI_RGMII_ZCU102	0xA8	/**< TI RGMII tuning value for ZCU102. */
#define PHY_TI_RGMII_VERSALEMU	0xAB	/**< TI RGMII tuning value for Versal emulation. */

/****************************************************************************/
/**
* @brief Sets a Marvell PHY into loopback mode at the requested speed.
*
* @param[in] EmacPsInstancePtr is a pointer to the XEmacPs driver instance.
* @param[in] Speed is the loopback speed in Mbps.
* @param[in] PhyAddr is the PHY address on the MDIO bus.
*
* @return XST_SUCCESS if successful, else XST_FAILURE.
*
*****************************************************************************/
LONG EmacPsUtilMarvellPhyLoopback(XEmacPs *EmacPsInstancePtr,
				  u32 Speed, u32 PhyAddr)
{
	LONG Status;
	u16 PhyReg0  = 0;
	u16 PhyReg21  = 0;
	u16 PhyReg22  = 0;

	/*
	 * Setup speed and duplex
	 */
	switch (Speed) {
		case 10:
			PhyReg0 |= PHY_REG0_10;
			PhyReg21 |= PHY_REG21_10;
			break;
		case 100:
			PhyReg0 |= PHY_REG0_100;
			PhyReg21 |= PHY_REG21_100;
			break;
		case 1000:
			PhyReg0 |= PHY_REG0_1000;
			PhyReg21 |= PHY_REG21_1000;
			break;
		default:
			EmacPsUtilErrorTrap("Error: speed not recognized ");
			return XST_FAILURE;
	}

	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0, PhyReg0);
	/*
	 * Make sure new configuration is in effect
	 */
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy speed");
		return XST_FAILURE;
	}

	/*
	 * Switching to PAGE2
	 */
	PhyReg22 = 0x2;
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 22, PhyReg22);

	/*
	 * Adding Tx and Rx delay. Configuring loopback speed.
	 */
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 21, PhyReg21);
	/*
	 * Make sure new configuration is in effect
	 */
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 21, &PhyReg21);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setting Reg 21 in Page 2");
		return XST_FAILURE;
	}
	/*
	 * Switching to PAGE0
	 */
	PhyReg22 = 0x0;
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 22, PhyReg22);

	/*
	 * Issue a reset to phy
	 */
	Status  = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	PhyReg0 |= PHY_REG0_RESET;
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0, PhyReg0);

	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error reset phy");
		return XST_FAILURE;
	}

	/*
	 * Enable loopback
	 */
	PhyReg0 |= PHY_REG0_LOOPBACK;
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0, PhyReg0);

	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy loopback");
		return XST_FAILURE;
	}

	EmacpsDelay(1);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief Sets a TI PHY into loopback mode at the requested speed.
*
* @param[in] EmacPsInstancePtr is a pointer to the XEmacPs driver instance.
* @param[in] Speed is the loopback speed in Mbps.
* @param[in] PhyAddr is the PHY address on the MDIO bus.
*
* @return XST_SUCCESS if successful, else XST_FAILURE.
*
*****************************************************************************/
LONG EmacPsUtilTiPhyLoopback(XEmacPs *EmacPsInstancePtr,
			     u32 Speed, u32 PhyAddr)
{
	LONG Status;
	u16 PhyReg0  = 0, LoopbackSpeed = 0;
	u16 RgmiiTuning = PHY_TI_RGMII_ZCU102;

	/*
	 * Setup speed and duplex
	 */
	switch (Speed) {
		case 10:
			PhyReg0 |= PHY_REG0_10;
			break;
		case 100:
			PhyReg0 |= PHY_REG0_100;
			break;
		case 1000:
			PhyReg0 |= PHY_REG0_1000;
			break;
		default:
			EmacPsUtilErrorTrap("Error: speed not recognized ");
			return XST_FAILURE;
	}
	LoopbackSpeed = PhyReg0;

	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0, PhyReg0);
	/*
	 * Make sure new configuration is in effect
	 */
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy speed");
		return XST_FAILURE;
	}

	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 1, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy speed");
		return XST_FAILURE;
	}

	/* Write PHY_LOOPCR */
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_ADDR);
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, PHY_LOOPCR);
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_DATA);
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, 0xEF20);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy speed");
		return XST_FAILURE;
	}

	/* Read PHY_LOOPCR */
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_ADDR);
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, PHY_LOOPCR);
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_DATA);
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy speed");
		return XST_FAILURE;
	}

	/* SW reset */
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0x1F, 0x4000);
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy speed");
		return XST_FAILURE;
	}

	/* issue a reset to phy */
	Status  = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	PhyReg0 |= PHY_REG0_RESET;
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0, PhyReg0);

	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error reset phy");
		return XST_FAILURE;
	}

	EmacpsDelay(1);

	/* enable loopback */
	PhyReg0 = LoopbackSpeed | PHY_REG0_LOOPBACK;
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0, PhyReg0);
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy loopback");
		return XST_FAILURE;
	}

	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0x10, 0x5048);
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0x10, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy loopback");
		return XST_FAILURE;
	}

	/* Write to PHY_RGMIIDCTL */
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_ADDR);
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, PHY_RGMIIDCTL);
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_DATA);
	if ((Platform & PLATFORM_MASK_VERSAL) == PLATFORM_VERSALEMU) {
		RgmiiTuning = PHY_TI_RGMII_VERSALEMU;
	}
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, RgmiiTuning);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error in tuning");
		return XST_FAILURE;
	}

	/* Read PHY_RGMIIDCTL */
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_ADDR);
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, PHY_RGMIIDCTL);
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_DATA);
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error in tuning");
		return XST_FAILURE;
	}

	/* Write PHY_RGMIICTL */
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_ADDR);
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, PHY_RGMIICTL);
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_DATA);
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, 0xD3);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error in tuning");
		return XST_FAILURE;
	}

	/* Read PHY_RGMIICTL */
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_ADDR);
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, PHY_RGMIICTL);
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_DATA);
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error in tuning");
		return XST_FAILURE;
	}

	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0x11, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy loopback");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief Provides a delay for the given number of seconds.
*
* @param[in] delay is the delay duration in seconds.
*
* @note For MicroBlaze the delay is not accurate and needs to be tuned.
*
*****************************************************************************/
void EmacpsDelay(u32 delay)
{

#ifndef __MICROBLAZE__
	sleep(delay);
#else
	unsigned long count = 0;
	while (count < (delay * 0xffff)) {
		count++;
	}
#endif

}


/****************************************************************************/
/**
* @brief Configures the detected PHY for loopback at the requested speed.
*
* @param[in] EmacPsInstancePtr is a pointer to the XEmacPs driver instance.
* @param[in] Speed is the loopback speed in Mbps.
*
* @return XST_SUCCESS if successful, else XST_FAILURE.
*
*****************************************************************************/
LONG EmacPsUtilEnterLoopback(XEmacPs *EmacPsInstancePtr, u32 Speed)
{
	LONG Status;
	u16 PhyIdentity;
	u32 PhyAddr;

	/*
	 * Detect the PHY address
	 */
	PhyAddr = XEmacPsDetectPHY(EmacPsInstancePtr);

	if (PhyAddr >= 32) {
		EmacPsUtilErrorTrap("Error detect phy");
		return XST_FAILURE;
	}

	XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, PHY_DETECT_REG1, &PhyIdentity);

	if (PhyIdentity == PHY_ID_MARVELL) {
		Status = EmacPsUtilMarvellPhyLoopback(EmacPsInstancePtr, Speed, PhyAddr);
	}

	if (PhyIdentity == PHY_ID_TI) {
		Status = EmacPsUtilTiPhyLoopback(EmacPsInstancePtr, Speed, PhyAddr);
	}

	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy loopback");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief Reports an error detected by the example code.
*
* Can be set as a breakpoint with a debugger or used to print the given
* message if a UART or STDIO device is available.
*
* @param[in] Message is the text explaining the error.
*
*****************************************************************************/
void EmacPsUtilErrorTrap(const char *Message)
{
	static u32 Count = 0;

	Count++;

#ifdef STDOUT_BASEADDRESS
	xil_printf("%s\r\n", Message);
#else
	(void) Message;
#endif
}

/** @} */
