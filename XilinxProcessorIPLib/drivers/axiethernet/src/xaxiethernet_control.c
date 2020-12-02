/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
*
* @file xaxiethernet_control.c
* @addtogroup axiethernet_v5_11
* @{
*
* This file has driver APIs related to the controlling of the extended
* features of the AXI Ethernet device. Please note that APIs for turning on/off
* any of the driver features are present in axiethernet.c. This file takes care
* of controlling these features.
*	- Normal/extended multicast filtering
*	- Normal/extended VLAN features
*	- RGMII/SGMII features
*
* See xaxiethernet.h for a detailed description of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a asa  6/30/10 First release based on the ll temac driver
* 5.7   rsp 01/09/18 Instead of #define XAE_MULTI_MAT_ENTRIES derive multicast table
*                    entries max count from ethernet config structure.
* 5.8   rsp 07/20/18 Fix cppcheck warning in Aptr assignment.
* </pre>
*****************************************************************************/

/***************************** Include Files *********************************/

#include "xaxiethernet.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* XAxiEthernet_MulticastAdd adds the Ethernet address, <i>AddressPtr</i> to the
* Axi Ethernet device's multicast filter list, at list index <i>Entry</i>. The
* address referenced by <i>AddressPtr</i> may be of any unicast, multicast, or
* broadcast address form. The hardware for the Axi Ethernet device can hold up
* to C_Number_of_Table_Entries addresses in this filter list.<br><br>
*
* The device must be stopped to use this function.<br><br>
*
* Once an Ethernet address is programmed, the Axi Ethernet device will begin
* receiving data sent from that address. The Axi Ethernet hardware does not
* have a control bit to disable multicast filtering. The only way to prevent
* the Axi Ethernet device from receiving messages from an Ethernet address in
* the Multicast Address Table (MAT) is to clear it with
* XAxiEthernet_MulticastClear().
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @param	AddressPtr is a pointer to the 6-byte Ethernet address to set.
*		The previous address at the location <i>Entry</i> (if any) is
*		overwritten with the value at <i>AddressPtr</i>.
* @param	Entry is the hardware storage location to program this address
*		and must be between 0 to (C_Number_of_Table_Entries - 1).
*
* @return
*		- XST_SUCCESS on successful completion.
*		- XST_DEVICE_IS_STARTED.if the Axi Ethernet device is not
*		  stopped.
*
* @note
*
* This routine works only with normal multicast filtering feature. A maximum
* of 4 multicast addresses can be stored in the HW provided multicast table.
*
* To use the extended multicast feature, extended multicast filtering must
* be enabled by using driver API XAxiEthernet_SetOptions with proper option
* fields set. Once extended multicast filtering is enabled, the APIs
* XAxiEthernet_[Add|Clear|Get]ExtMulticastGroup() must be used to manage
* multicast address groups.
*
******************************************************************************/
int XAxiEthernet_MulticastAdd(XAxiEthernet *InstancePtr, void *AddressPtr,
								int Entry)
{
	u32 Af0Reg;
	u32 Af1Reg;
	u32 FmiReg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(AddressPtr != NULL);
	Xil_AssertNonvoid(Entry < InstancePtr->Config.NumTableEntries);

	u8 *Aptr = (u8 *) AddressPtr;
	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_MulticastAdd\n");

	/* The device must be stopped before clearing the multicast hash
	 * table.
	 */
	if (InstancePtr->IsStarted == XIL_COMPONENT_IS_STARTED) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_MulticastAdd: returning DEVICE_IS_STARTED\n");
		return (XST_DEVICE_IS_STARTED);
	}

	/* Set MAC bits [31:0] */
	Af0Reg = Aptr[0];
	Af0Reg |= Aptr[1] << 8;
	Af0Reg |= Aptr[2] << 16;
	Af0Reg |= Aptr[3] << 24;

	/* Set MAC bits [47:32] */
	Af1Reg = Aptr[4];
	Af1Reg |= Aptr[5] << 8;


	FmiReg = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_FMI_OFFSET);
	FmiReg &= 0xFFFFFF00;
	FmiReg |= (Entry);
	XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
						XAE_FMI_OFFSET, FmiReg);

	/* Add in MAT address */
	xdbg_printf(XDBG_DEBUG_GENERAL, "Setting MAT entry: %d\n", Entry);
	XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
						XAE_AF0_OFFSET, Af0Reg);

	XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
						XAE_AF1_OFFSET, Af1Reg);

	return (XST_SUCCESS);
}


/*****************************************************************************/
/**
* XAxiEthernet_MulticastGet gets the Ethernet address stored at
* index <i>Entry</i> in the Axi Ethernet device's multicast filter list.<br><br>
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @param	AddressPtr references the memory buffer to store the retrieved
*		Ethernet address. This memory buffer must be at least 6 bytes
*		in length.
* @param	Entry is the hardware storage location from which to retrieve
*		the address and must be between 0 to (C_Number_of_Table_Entries - 1)
*
* @return	None.
*
* @note
*
* This routine works only with normal multicast filtering feature. A maximum
* of 4 multicast addresses can be stored in the HW provided multicast table.
*
* To use the extended multicast feature, extended multicast filtering must
* be enabled by using driver API XAxiEthernet_SetOptions with proper option
* fields set. Once extended multicast filtering is enabled, the APIs
* XAxiEthernet_[Add|Clear|Get]ExtMulticastGroup() must be used to manage
* multicast address groups.
*
******************************************************************************/
void XAxiEthernet_MulticastGet(XAxiEthernet *InstancePtr, void *AddressPtr,
								int Entry)
{
	u32 Af0Reg;
	u32 Af1Reg;
	u32 FmiReg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(AddressPtr != NULL);
	Xil_AssertVoid(Entry < InstancePtr->Config.NumTableEntries);

	u8 *Aptr = (u8 *) AddressPtr;
	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_MulticastGet\n");


	FmiReg = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_FMI_OFFSET);
	FmiReg &= 0xFFFFFF00;
	FmiReg |= (Entry);
	XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
						XAE_FMI_OFFSET, FmiReg);


	Af0Reg = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_AF0_OFFSET);
	Af1Reg = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_AF1_OFFSET);

	/* Copy the address to the user buffer */
	Aptr[0] = (u8) Af0Reg;
	Aptr[1] = (u8) (Af0Reg >> 8);
	Aptr[2] = (u8) (Af0Reg >> 16);
	Aptr[3] = (u8) (Af0Reg >> 24);
	Aptr[4] = (u8) Af1Reg;
	Aptr[5] = (u8) (Af1Reg >> 8);

	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_MulticastGet: done\n");
}

/*****************************************************************************/
/**
* XAxiEthernet_MulticastClear clears the Ethernet address stored at index
* <i>Entry</i> in the Axi Ethernet device's multicast filter list.<br><br>
*
* The device must be stopped to use this function.<br><br>
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @param	Entry is the HW storage location used when this address was
*		added. It must be between 0 to (C_Number_of_Table_Entries - 1).
*
* @return
*		- XST_SUCCESS on successful completion.
*		- XST_DEVICE_IS_STARTED.if the Axi Ethernet device is not
*		  stopped.
*
* @note
*
* This routine works only with normal multicast filtering feature. A maximum
* of 4 multicast addresses can be stored in the HW provided multicast table.
*
* To use the extended multicast feature, extended multicast filtering must
* be enabled by using driver API XAxiEthernet_SetOptions with proper option
* fields set. Once extended multicast filtering is enabled, the APIs
* XAxiEthernet_[Add|Clear|Get]ExtMulticastGroup() must be used to manage
* multicast address groups.
*
******************************************************************************/
int XAxiEthernet_MulticastClear(XAxiEthernet *InstancePtr, int Entry)
{
	u32 FmiReg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(Entry < InstancePtr->Config.NumTableEntries);

	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_MulticastClear\n");

	/*
	 * The device must be stopped before clearing the multicast hash
	 * table.
	 */
	if (InstancePtr->IsStarted == XIL_COMPONENT_IS_STARTED) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_MulticastClear:returning DEVICE_IS_STARTED\n");
		return (XST_DEVICE_IS_STARTED);
	}

	FmiReg = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_FMI_OFFSET);
	FmiReg &= 0xFFFFFF00;
	FmiReg |= (Entry);
	XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
						XAE_FMI_OFFSET, FmiReg);


	/* Clear the entry by writing 0:0:0:0:0:0 to it */
	XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
						XAE_AF0_OFFSET, 0);

	XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
						XAE_AF1_OFFSET, 0);

	xdbg_printf(XDBG_DEBUG_GENERAL,
			"XAxiEthernet_MulticastClear: returning SUCCESS\n");
	return (XST_SUCCESS);
}


/*****************************************************************************/
/**
* XAxiEthernet_SetMacPauseAddress sets the MAC address used for pause frames
* to <i>AddressPtr</i>. <i>AddressPtr</i> will be the address the Axi Ethernet
* device will recognize as being for pause frames. Pause frames transmitted
* with XAxiEthernet_SendPausePacket() will also use this address.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @param	AddressPtr is a pointer to the 6-byte Ethernet address to set.
*
* @return
*		- XST_SUCCESS on successful completion.
*		- XST_DEVICE_IS_STARTED if the Axi Ethernet device is not
*		  stopped.
*
* @note		None.
*
******************************************************************************/
int XAxiEthernet_SetMacPauseAddress(XAxiEthernet *InstancePtr,
							void *AddressPtr)
{
	u32 MacAddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(AddressPtr != NULL);

	u8 *Aptr = (u8 *) AddressPtr;
	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_SetMacPauseAddress\n");

	/* Be sure device has been stopped */
	if (InstancePtr->IsStarted == XIL_COMPONENT_IS_STARTED) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_SetMacPauseAddress:returning DEVICE_IS_STARTED\n");
		return (XST_DEVICE_IS_STARTED);
	}

	/* Set the MAC bits [31:0] in RCW0 register */
	MacAddr = Aptr[0];
	MacAddr |= Aptr[1] << 8;
	MacAddr |= Aptr[2] << 16;
	MacAddr |= Aptr[3] << 24;
	XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
						XAE_RCW0_OFFSET, MacAddr);

	/* RCW1 contains other info that must be preserved */
	MacAddr = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_RCW1_OFFSET);
	MacAddr &= ~XAE_RCW1_PAUSEADDR_MASK;

	/* Set MAC bits [47:32] */
	MacAddr |= Aptr[4];
	MacAddr |= Aptr[5] << 8;
	XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
						XAE_RCW1_OFFSET, MacAddr);

	xdbg_printf(XDBG_DEBUG_GENERAL,
		   "XAxiEthernet_SetMacPauseAddress: returning SUCCESS\n");

	return (XST_SUCCESS);
}


/*****************************************************************************/
/**
* XAxiEthernet_GetMacPauseAddress gets the MAC address used for pause frames
* for the Axi Ethernet device specified by <i>InstancePtr</i>.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @param	AddressPtr references the memory buffer to store the retrieved
*		MAC address. This memory buffer must be at least 6 bytes in
*		length.
*
* @return 	None.
*
* @note		None.
*
******************************************************************************/
void XAxiEthernet_GetMacPauseAddress(XAxiEthernet *InstancePtr,
							void *AddressPtr)
{
	u32 MacAddr;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(AddressPtr != NULL);


	u8 *Aptr = (u8 *) AddressPtr;
	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_SetMacPauseAddress\n");

	/* Read MAC bits [31:0] in ERXC0 */
	MacAddr = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_RCW0_OFFSET);
	Aptr[0] = (u8) MacAddr;
	Aptr[1] = (u8) (MacAddr >> 8);
	Aptr[2] = (u8) (MacAddr >> 16);
	Aptr[3] = (u8) (MacAddr >> 24);

	/* Read MAC bits [47:32] in RCW1 */
	MacAddr = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_RCW1_OFFSET);
	Aptr[4] = (u8) MacAddr;
	Aptr[5] = (u8) (MacAddr >> 8);

	xdbg_printf(XDBG_DEBUG_GENERAL,
				"XAxiEthernet_SetMacPauseAddress: done\n");
}

/*****************************************************************************/
/**
* XAxiEthernet_SendPausePacket sends a pause packet with the value of
* <i>PauseValue</i>.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @param	PauseValue is the pause value in units of 512 bit times.
*
* @return
*		- XST_SUCCESS on successful completion.
*		- XST_DEVICE_IS_STARTED if the Axi Ethernet device is not
*		  stopped.
*
* @note		None.
*
******************************************************************************/
int XAxiEthernet_SendPausePacket(XAxiEthernet *InstancePtr, u16 PauseValue)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);


	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_SetMacPauseAddress\n");

	/* Make sure device is ready for this operation */
	if (InstancePtr->IsStarted != XIL_COMPONENT_IS_STARTED) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_SendPausePacket:returning DEVICE_IS_STOPPED\n");
		return (XST_DEVICE_IS_STOPPED);
	}

	/* Send flow control frame */
	XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress, XAE_TPF_OFFSET,
				 (u32) PauseValue & XAE_TPF_TPFV_MASK);

	xdbg_printf(XDBG_DEBUG_GENERAL,
		   "XAxiEthernet_SendPausePacket: returning SUCCESS\n");
	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
* XAxiEthernet_GetSgmiiStatus get the state of the link when using the SGMII
* media interface.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @param	SpeedPtr references the location to store the result, which is
*		the auto negotiated link speed in units of Mbits/sec, either 0,
*		10, 100, or 1000.
*
* @return
*		- XST_SUCCESS on successful completion.
*		- XST_NO_FEATURE if the Axi Ethernet device is not using an
*		  SGMII interface,
*
* @note		Currently SGMII PHY does not support half duplex mode.
*
******************************************************************************/
int XAxiEthernet_GetSgmiiStatus(XAxiEthernet *InstancePtr, u16 *SpeedPtr)
{
	int PhyType;
	u32 EgmicReg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(SpeedPtr != NULL);

	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_GetSgmiiStatus\n");

	/* Make sure PHY is SGMII */
	PhyType = XAxiEthernet_GetPhysicalInterface(InstancePtr);
	if (PhyType != XAE_PHY_TYPE_SGMII) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_GetSgmiiStatus: returning NO_FEATURE\n");
		return (XST_NO_FEATURE);
	}

	/* Get the current contents of RGMII/SGMII config register */
	EgmicReg = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
						XAE_PHYC_OFFSET);

	/* Extract speed */
	switch (EgmicReg & XAE_PHYC_SGMIILINKSPEED_MASK) {
	case XAE_PHYC_SGLINKSPD_10:
		*SpeedPtr = XAE_SPEED_10_MBPS;
		break;

	case XAE_PHYC_SGLINKSPD_100:
		*SpeedPtr = XAE_SPEED_100_MBPS;
		break;

	case XAE_PHYC_SGLINKSPD_1000:
		*SpeedPtr = XAE_SPEED_1000_MBPS;
		break;

	default:
		*SpeedPtr = 0;
	}

	xdbg_printf(XDBG_DEBUG_GENERAL,
		   "XAxiEthernet_GetSgmiiStatus: returning SUCCESS\n");
	return (XST_SUCCESS);
}


/*****************************************************************************/
/**
* XAxiEthernet_GetRgmiiStatus get the state of the link when using the RGMII
* media interface.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @param	SpeedPtr references the location to store the result, which is
*		the auto negotiated link speed in units of Mbits/sec,
*		either 0, 10, 100, or 1000.
* @param	IsFullDuplexPtr references the value that is set by this
*		function to indicate full duplex operation.
*		<i>IsFullDuplexPtr</i> is set to TRUE when the RGMII link is
*		operating in full duplex mode, otherwise it is set to FALSE.
* @param	IsLinkUpPtr references the value that is set by this function
*		to indicate the link status.<i>IsLinkUpPtr</i> is set to TRUE
*		when the RGMII link up, otherwise it is set to FALSE.
*
* @return
*		- XST_SUCCESS on successful completion.
*		- XST_NO_FEATURE if the Axi Ethernet device is not using an
*		  RGMII interface,
*
* @note		None.
*
******************************************************************************/
int XAxiEthernet_GetRgmiiStatus(XAxiEthernet *InstancePtr, u16 *SpeedPtr,
				int *IsFullDuplexPtr, int *IsLinkUpPtr)
{
	int PhyType;
	u32 EgmicReg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(SpeedPtr != NULL);
	Xil_AssertNonvoid(IsFullDuplexPtr != NULL);
	Xil_AssertNonvoid(IsLinkUpPtr != NULL);


	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_GetRgmiiStatus\n");

	/* Make sure PHY is RGMII */
	PhyType = XAxiEthernet_GetPhysicalInterface(InstancePtr);
	if ((PhyType != XAE_PHY_TYPE_RGMII_1_3) &&
		(PhyType != XAE_PHY_TYPE_RGMII_2_0)) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
			"XAxiEthernet_GetRgmiiStatus: returning NO_FEATURE\n");
		return (XST_NO_FEATURE);
	}

	/* Get the current contents of RGMII/SGMII config register */
	EgmicReg = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
						XAE_PHYC_OFFSET);

	/* Extract speed */
	switch (EgmicReg & XAE_PHYC_RGMIILINKSPEED_MASK) {
	case XAE_PHYC_RGLINKSPD_10:
		*SpeedPtr = XAE_SPEED_10_MBPS;
		break;

	case XAE_PHYC_RGLINKSPD_100:
		*SpeedPtr = XAE_SPEED_100_MBPS;
		break;

	case XAE_PHYC_RGLINKSPD_1000:
		*SpeedPtr = XAE_SPEED_1000_MBPS;
		break;

	default:
		*SpeedPtr = 0;
	}

	/* Extract duplex and link status */
	if (EgmicReg & XAE_PHYC_RGMIIHD_MASK) {
		*IsFullDuplexPtr = FALSE;
	} else {
		*IsFullDuplexPtr = TRUE;
	}

	if (EgmicReg & XAE_PHYC_RGMIILINK_MASK) {
		*IsLinkUpPtr = TRUE;
	} else {
		*IsLinkUpPtr = FALSE;
	}

	xdbg_printf(XDBG_DEBUG_GENERAL,
		   "XAxiEthernet_GetRgmiiStatus: returning SUCCESS\n");
	return (XST_SUCCESS);
}


/*****************************************************************************/
/**
* XAxiEthernet_SetTpid sets the VLAN Tag Protocol Identifier(TPID).
*
* Four values can be configured - 0x8100, 0x9100, 0x9200, 0x88A8.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @param	Tpid is a hex value to be added to the TPID table. The four
*		values that can be added are 0x8100, 0x9100, 0x9200, 0x88A8.
* @param	Entry is the hardware storage location to program this address
*		and must be between 0..XAE_TPID_MAX_ENTRIES.
*
* @return
*		- XST_SUCCESS on successful completion.
*		- XST_DEVICE_IS_STARTED, if the Axi Ethernet device is not
*		  stopped.
*		- XST_NO_FEATURE if the Axi Ethernet does not enable or have
*		  the VLAN tag capability.
*		- XST_INVALID_PARAM if Tpid is not one of supported values.
*
* @note		The device must be stopped to use this function.
*
*****************************************************************************/
int XAxiEthernet_SetTpid(XAxiEthernet *InstancePtr, u16 Tpid, u8 Entry)
{
	u32 RegTpid;
	u32 RegTpidOffset;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(Entry < XAE_TPID_MAX_ENTRIES);

	/* The device must be stopped before modify VLAN TPID */
	if (InstancePtr->IsStarted == XIL_COMPONENT_IS_STARTED) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_SetTpid: returning DEVICE_IS_STARTED\n");
		return (XST_DEVICE_IS_STARTED);
	}

	/* Check hw capability */
	if (!XAxiEthernet_IsExtFuncCap(InstancePtr)) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_SetTpid: returning DEVICE_NO_FEATURE\n");
		return (XST_NO_FEATURE);
	}

	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_SetTpid\n");

	/* Verify TPID */
	switch (Tpid) {
		case 0x8100:
		case 0x88a8:
		case 0x9100:
		case 0x9200:
			break;
		default:
			return (XST_INVALID_PARAM);
	}

	/* Determine which register to operate on */
	if (Entry < 2) {
		RegTpidOffset = XAE_TPID0_OFFSET;
	} else {
		RegTpidOffset = XAE_TPID1_OFFSET;
	}

	RegTpid = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
								RegTpidOffset);

	/* Determine upper/lower 16 bits to operate on */
	if (Entry % 2) {
		/* Program HW */
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
			RegTpidOffset, (RegTpid & XAE_TPID_0_MASK) |
			(Tpid << 16));
	} else {
		/* Program HW */
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
			RegTpidOffset, (RegTpid & XAE_TPID_1_MASK) |
			Tpid);
	}
	xdbg_printf(XDBG_DEBUG_GENERAL,
			"XAxiEthernet_SetTpid: returning SUCCESS\n");
	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
* XAxiEthernet_ClearTpid clears the VLAN Tag Protocol Identifier(TPID).
*
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @param	Entry is the hardware storage location to program this address
*		and must be between 0..XAE_TPID_MAX_ENTRIES.
*
* @return
*		 - XST_SUCCESS on successful completion.
*		 - XST_DEVICE_IS_STARTED if the Axi Ethernet device is not
*		   stopped.
*		 - XST_NO_FEATURE if the Axi Ethernet does not enable or have
*		   the VLAN tag capability.
*
* @note		The device must be stopped to use this function.
*
*****************************************************************************/
int XAxiEthernet_ClearTpid(XAxiEthernet *InstancePtr, u8 Entry)
{
	u32 RegTpid;
	u32 RegTpidOffset;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(Entry < XAE_TPID_MAX_ENTRIES);

	/* The device must be stopped before modify VLAN TPID */
	if (InstancePtr->IsStarted == XIL_COMPONENT_IS_STARTED) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_ClearTpid: returning DEVICE_IS_STARTED\n");
		return (XST_DEVICE_IS_STARTED);
	}

	/* Check hw capability */
	if (!XAxiEthernet_IsExtFuncCap(InstancePtr)) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_ClearTpid: returning DEVICE_NO_FEATURE\n");
		return (XST_NO_FEATURE);
	}

	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_ClearExtTpid\n");

	/* Determine which register to operate on */
	if (Entry < 2) {
		RegTpidOffset = XAE_TPID0_OFFSET;
	} else {
		RegTpidOffset = XAE_TPID1_OFFSET;
	}

	RegTpid = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
				RegTpidOffset);

	/* Determine upper/lower 16 bits to operate on */
	if (Entry % 2) {
		/* Program HW */
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
			RegTpidOffset, (RegTpid & XAE_TPID_1_MASK));
	} else {
		/* Program HW */
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
			RegTpidOffset, (RegTpid & XAE_TPID_0_MASK));
	}
	xdbg_printf(XDBG_DEBUG_GENERAL,
			"XAxiEthernet_ClearTpid: returning SUCCESS\n");

	return (XST_SUCCESS);
}


/*****************************************************************************/
/**
* XAxiEthernet_GetTpid gets the VLAN Tag Protocol Identifier value (TPID).
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @param	TpidPtr references the location to store the result.
* @param	Entry is the hardware storage location to program this address
*		and must be between 0..XAE_TPID_MAX_ENTRIES.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XAxiEthernet_GetTpid(XAxiEthernet *InstancePtr, u16 *TpidPtr, u8 Entry)
{
	u32 RegTpid;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(TpidPtr != NULL);
	Xil_AssertVoid(Entry < XAE_TPID_MAX_ENTRIES);

	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_GetTpid\n");

	if (Entry < 2) {
		RegTpid = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
					XAE_TPID0_OFFSET);
	} else {
		RegTpid = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
					XAE_TPID1_OFFSET);
	}

	if (Entry % 2) {
		*TpidPtr = (RegTpid >> 16);
	} else {
		*TpidPtr = (RegTpid & XAE_TPID_0_MASK);
	}
	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_GetTpid: done\n");
}


/*****************************************************************************/
/**
* XAxiEthernet_SetVTagMode configures the VLAN tagging mode.
*
* Four modes can be configured,
*	- XAE_VTAG_NONE for no tagging.
*	- XAE_VTAG_ALL to tag all frames.
*	- XAE_VTAG_EXISTED to tag already tagged frames.
*	- XAE_VTAG_SELECT to tag selected already tagged frames based on VID
*	  value.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @param	Mode is the VLAN tag mode. Value must be between b'00-b'11.
* @param	Dir must be either XAE_TX or XAE_RX.
*
* @return
*		- XST_SUCCESS. on successful completion.
*		- XST_DEVICE_IS_STARTED if the Axi Ethernet device is not
*                 stopped.
*		- XST_NO_FEATURE if the Axi Ethernet does not enable or have
*		  the TX VLAN tag capability.
*		- XST_INVALID_PARAM if Mode is not one of supported modes.
*
* @note
*
* The device must be stopped to use this function.<br><br>
*
* The fourth mode (specified by XAE_VTAG_SELECT) requires a method for
* specifying which tagged frames should receive an additional VLAN tag.
* The VLAN translation table 'tag enabled' is referenced. That configuration
* is handled in XAxiEthernet_SetVidTable().
*
* Mode value shifting is handled in this function. No shifting is required to
* call this function.
*
*****************************************************************************/
int XAxiEthernet_SetVTagMode(XAxiEthernet *InstancePtr, u32 Mode, int Dir)
{
	u32 RegRaf;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((Dir == XAE_TX) || (Dir == XAE_RX));

	/* The device must be stopped before modify TX VLAN Tag mode */
	if (InstancePtr->IsStarted == XIL_COMPONENT_IS_STARTED) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_SetVTagMode:returning DEVICE_IS_STARTED\n");
		return (XST_DEVICE_IS_STARTED);
	}

	/* Check hw capability */
	if (!XAxiEthernet_IsExtFuncCap(InstancePtr)) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_SetVTagMode: returning DEVICE_NO_FEATURE\n");
		return (XST_NO_FEATURE);
	}

	/* Mode has to be one of the supported values */
	switch (Mode) {
		case XAE_VTAG_NONE:
		case XAE_VTAG_ALL:
		case XAE_VTAG_EXISTED:
		case XAE_VTAG_SELECT:
			break;
		default:
			return (XST_INVALID_PARAM);
	}

	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_SetVTagMode\n");

	/* Program HW */
	RegRaf = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_RAF_OFFSET);
	/* Transmit direction */
	if (XAE_TX == Dir) {
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
			XAE_RAF_OFFSET, ((RegRaf & ~XAE_RAF_TXVTAGMODE_MASK) |
			(Mode << XAE_RAF_TXVTAGMODE_SHIFT)));
	} else { /* Receive direction */
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
			XAE_RAF_OFFSET, ((RegRaf & ~XAE_RAF_RXVTAGMODE_MASK) |
			(Mode << XAE_RAF_RXVTAGMODE_SHIFT)));
	}

	xdbg_printf(XDBG_DEBUG_GENERAL,
			"XAxiEthernet_SetVTagMode: returning SUCCESS\n");

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
* XAxiEthernet_GetVTagMode gets VLAN tagging mode.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
		worked on.
* @param	ModePtr references the location to store the VLAN tag mode.
*		Value is between b'00-b'11.
* @param 	Dir must be either XAE_TX or XAE_RX.
*
* @return	None.
*
* @note
*
* The device must be stopped to use this function.<br><br>
* Mode value shifting is handled in this function. No shifting is required to
* call this function.
*
*****************************************************************************/
void XAxiEthernet_GetVTagMode(XAxiEthernet *InstancePtr, u8 *ModePtr, int Dir)
{
	u32 RegRaf;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(ModePtr != NULL);
	Xil_AssertVoid((Dir == XAE_TX) || (Dir == XAE_RX));

	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_GetVTagMode\n");

	/* Access HW configuration */
	RegRaf = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_RAF_OFFSET);
	/* Transmit direction */
	if (XAE_TX == Dir) {
		*ModePtr = (RegRaf & XAE_RAF_TXVTAGMODE_MASK) >>
				XAE_RAF_TXVTAGMODE_SHIFT;
	} else { /* Receive direction */
		*ModePtr = (RegRaf & XAE_RAF_RXVTAGMODE_MASK) >>
				XAE_RAF_RXVTAGMODE_SHIFT;
	}

	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_GetVTagMode: done\n");

}


/*****************************************************************************/
/**
* XAxiEthernet_SetVStripMode configures the VLAN strip mode.
*
* Three modes can be configured :
*	- XAE_VSTRP_NONE for no stripping.
*	- XAE_VSTRP_ALL	to strip one tag from all frames.
*	- XAE_VSTRP_SELECT to strip one tag from already selected tagged frames
*	  based on VID value.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @param	Mode is the VLAN strip mode. Value must be b'00, b'01, or b'11.
* @param	Dir must be either XAE_TX or XAE_RX.
*
* @return
*		- XST_SUCCESS on successful completion., returns XST_SUCCESS.
*		- XST_DEVICE_IS_STARTED if the Axi Ethernet device is not
*                 stopped.
*		- XST_NO_FEATURE if the Axi Ethernet does not enable or have
*		  the TX VLAN strip capability.
*		- XST_INVALID_PARAM if Mode is not one of supported modes.
*
* @note
*
* The device must be stopped to use this function.<br><br>
* The third mode (specified by XAE_VSTRP_SELECT) requires a method for
* specifying which tagged frames should be stripped. The VLAN translation
* table 'stripped enabled' is referenced. That configuration is handled in
* XAxiEthernet_SetVidTable().
*
* Mode value shifting is handled in this function. No shifting is required to
* call this function.
*
*****************************************************************************/
int XAxiEthernet_SetVStripMode(XAxiEthernet *InstancePtr, u32 Mode, int Dir)
{
	u32 RegRaf;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((Dir == XAE_TX) || (Dir == XAE_RX));

	/* The device must be stopped before modify TX VLAN Tag mode */
	if (InstancePtr->IsStarted == XIL_COMPONENT_IS_STARTED) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_SetVStripMode: returning DEVICE_IS_STARTED\n");
		return (XST_DEVICE_IS_STARTED);
	}

	/* Check HW capability */
	if (!XAxiEthernet_IsExtFuncCap(InstancePtr)) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_SetVStripMode:returning DEVICE_NO_FEATURE\n");
		return (XST_NO_FEATURE);
	}

	/* Mode has to be one of the supported values */
	switch (Mode) {
		case XAE_VSTRP_NONE:
		case XAE_VSTRP_ALL:
		case XAE_VSTRP_SELECT:
			break;
		default:
			return (XST_INVALID_PARAM);
	}

	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_SetStripMode\n");

	/* Program HW */
	RegRaf = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_RAF_OFFSET);
	/* Transmit direction */
	if (XAE_TX == Dir) {
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
			XAE_RAF_OFFSET, ((RegRaf & ~XAE_RAF_TXVSTRPMODE_MASK) |
			(Mode << XAE_RAF_TXVSTRPMODE_SHIFT)));
	} else { /* Receive direction */
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
			XAE_RAF_OFFSET, ((RegRaf & ~XAE_RAF_RXVSTRPMODE_MASK) |
			(Mode << XAE_RAF_RXVSTRPMODE_SHIFT)));
	}

	xdbg_printf(XDBG_DEBUG_GENERAL,
			"XAxiEthernet_SetVStripMode:returning SUCCESS\n");

	return (XST_SUCCESS);
}


/*****************************************************************************/
/**
* XAxiEthernet_GetVStripMode gets the VLAN stripping mode.
*
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @param	ModePtr references the location to store the VLAN strip mode
*		returned by this function. Value is b'00, b'01 or b'11.
*		Refer XAE_VTSRAP_* in xaxiethernet.h file for the details.
* @param	Dir must be either XAE_TX or XAE_RX.
*
* @return	None.
*
* @note
*
* Mode value shifting is handled in this function. No shifting is required to
* call this function.
*
*****************************************************************************/
void XAxiEthernet_GetVStripMode(XAxiEthernet *InstancePtr, u8 *ModePtr,
								int Dir)
{
	u32 RegRaf;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(ModePtr != NULL);
	Xil_AssertVoid((Dir == XAE_TX) || (Dir == XAE_RX));

	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_GetVStripMode\n");

	/* Access HW configuration */
	RegRaf = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_RAF_OFFSET);
	/* Transmit direction */
	if (XAE_TX == Dir) {
		*ModePtr = (RegRaf & XAE_RAF_TXVSTRPMODE_MASK) >>
					XAE_RAF_TXVSTRPMODE_SHIFT;
	} else { /* Receive direction */
		*ModePtr = (RegRaf & XAE_RAF_RXVSTRPMODE_MASK) >>
					XAE_RAF_RXVSTRPMODE_SHIFT;
	}

	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_GetVStripMode: done\n");
}


/*****************************************************************************/
/**
* XAxiEthernet_SetVTagValue configures the VLAN tagging value.
*
* The device must be stopped to use this function.<br><br>
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @param	VTagValue is the VLAN tag value to be configured. A 32bit
*		value.
*		TPID, one of the following 16 bit values,
*		0x8100, 0x88a8, 0x9100, 0x9200.
*		Priority, 3  bits
*		CFI,	  1  bit
*		VID,	  12 bits
* @param	Dir must be either XAE_TX or XAE_RX.
*
* @return
*		- XST_SUCCESS on successful completion, returns .
*		- XST_DEVICE_IS_STARTED if the Axi Ethernet device is not
*		  stopped.
*		- XST_NO_FEATURE if the Axi Ethernet does not enable/have
*		  TX VLAN tag capability.
*		- XST_INVALID_PARAM, if the TPID is not one the four supported
*		  values.
*
* @note
*
* The four supported TPID values are 0x8100, 0x88a8, 0x9100, 0x9200.
* XAxiEthernet_SetVTagValue performs verification on TPID only.
*
* Ethernet VLAN frames' VLAN type/length(2B) and tag control information(2B).
* Bit layout : bbbb bbbb bbbb bbbb bbb b bbbb bbbb bbbb
*              \                 /  |  | \ VID (12b)  /
*               \               /   |  CFI bit (1b)
*                   TPID (16b)      priority bit (3b)
*
*****************************************************************************/
int XAxiEthernet_SetVTagValue(XAxiEthernet *InstancePtr, u32 VTagValue,
								int Dir)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((Dir == XAE_TX) || (Dir == XAE_RX));

	/* The device must be stopped before modifying TX VLAN Tag value */
	if (InstancePtr->IsStarted == XIL_COMPONENT_IS_STARTED) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_SetVTagValue:returning DEVICE_IS_STARTED\n");
		return (XST_DEVICE_IS_STARTED);
	}

	/* Check HW capability */
	if (!XAxiEthernet_IsExtFuncCap(InstancePtr)) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_SetVTagValue:returning DEVICE_NO_FEATURE\n");
		return (XST_NO_FEATURE);
	}

	/* Verify TPID */
	switch (VTagValue >> 16) {
		case 0x8100:
		case 0x88a8:
		case 0x9100:
		case 0x9200:
			break;
		default:
			return (XST_INVALID_PARAM);
	}

	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_SetVTagValue\n");

	/* Program HW */
	/* Transmit direction */
	if (XAE_TX == Dir) {
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
					XAE_TTAG_OFFSET, VTagValue);
	} else { /* Receive direction */
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
					XAE_RTAG_OFFSET, VTagValue);
	}

	xdbg_printf(XDBG_DEBUG_GENERAL,
			"XAxiEthernet_SetVTagValue:returning SUCCESS\n");

	return (XST_SUCCESS);
}


/*****************************************************************************/
/**
* XAxiEthernet_GetVTagValue gets the configured VLAN tagging value.
*
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @param	VTagValuePtr references the location to store the result.
*		Format is TPID, one of the following 16 bit values,
*			  0x8100, 0x88a8, 0x9100, 0x9200.
*		Priority, 3  bits
*		CFI,	  1  bit
*		VID,	  12 bits
* @param	Dir must be either XAE_TX or XAE_RX.
*
* @return	None.
*
* @note
*
* Ethernet VLAN frames' VLAN type/length(2B) and tag control information(2B).
* Bit layout : bbbb bbbb bbbb bbbb bbb b bbbb bbbb bbbb
*              \                 /  |  | \ VID (12b)  /
*               \               /   |  CFI bit (1b)
*                   TPID (16b)      priority bit (3b)
*
*****************************************************************************/
void XAxiEthernet_GetVTagValue(XAxiEthernet *InstancePtr, u32 *VTagValuePtr,
								int Dir)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(VTagValuePtr != NULL);
	Xil_AssertVoid((Dir == XAE_TX) || (Dir == XAE_RX));

	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_GetVTagValue\n");

	/* Transmit direction */
	if (XAE_TX == Dir) {
		*VTagValuePtr = XAxiEthernet_ReadReg
					(InstancePtr->Config.BaseAddress,
							XAE_TTAG_OFFSET);
	}
	else { /* Receive direction */
		*VTagValuePtr = XAxiEthernet_ReadReg(
					InstancePtr->Config.BaseAddress,
							XAE_RTAG_OFFSET);
	}

	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_GetVTagValue: done\n");
}


/*****************************************************************************/
/**
* XAxiEthernet_SetVidTable sets VID table includes new VLAN ID, strip
* and tag enable bits.
*
* The device must be stopped to use this function.<br><br>
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @param	Entry is the hardware storage location/index to program updated
*		VID value, strip, or tag value.
*		The value must be between 0..0xFFF.
* @param	Vid is updated/translated Vid value to be programmed.
* @param	Strip is strip enable indication for Vid.
* @param	Tag is tag enable indication for Vid.
* @param	Dir must be either XAE_TX or XAE_RX.
*
* @return
*		- XST_SUCCESS on successful completion.
*		- XST_DEVICE_IS_STARTED if the Axi Ethernet device is not
*		  stopped.
*		- XST_NO_FEATURE if the Axi Ethernet does not enable/have
*		  extended functionalities.
*
* @note
*
* The hardware requires the table to be 'indexed' with Entry and must be
* 0x000..0xFFF.
*
* Bits layout is bbbb bbbb bbbb b b
*                VLAN ID (12b), | |
*                               | VLAN double tag enable bit
*                               VLAN strip enable bit
*
* To disable translation indexed by Entry, Set Vid = Entry.
*
*****************************************************************************/
int XAxiEthernet_SetVidTable(XAxiEthernet *InstancePtr, u32 Entry, u32 Vid,
				u8 Strip, u8 Tag, int Dir)
{
	u32 Reg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(Entry <= XAE_MAX_VLAN_TABL_ENTRY);
	Xil_AssertNonvoid((Dir == XAE_TX) || (Dir == XAE_RX));
	Xil_AssertNonvoid(Vid <= XAE_MAX_VLAN_TABL_ENTRY);
	Xil_AssertNonvoid(Strip <= XAE_VLAN_TABL_STRP_FLD_LEN);
	Xil_AssertNonvoid(Tag <= XAE_VLAN_TABL_TAG_FLD_LEN);

	/* The device must be stopped before modify TX VLAN Tag value */
	if (InstancePtr->IsStarted == XIL_COMPONENT_IS_STARTED) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_SetVidTable:returning DEVICE_IS_STARTED\n");
		return (XST_DEVICE_IS_STARTED);
	}

	/* Check HW capability */
	if (!XAxiEthernet_IsExtFuncCap(InstancePtr)) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_SetVidTable:returning DEVICE_NO_FEATURE\n");
		return (XST_NO_FEATURE);
	}

	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_SetVidTable\n");

	/* Program HW */
	Reg = (Vid << XAE_VLAN_TABL_VID_START_OFFSET) |
			(Strip << XAE_VLAN_TABL_STRP_STRT_OFFSET) | Tag;
	/* Transmit direction */
	if (XAE_TX == Dir) {
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
			XAE_TX_VLAN_DATA_OFFSET +
			(Entry << XAE_VLAN_TABL_VID_START_OFFSET), Reg);
	} else { /* Receive direction */
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
			XAE_RX_VLAN_DATA_OFFSET +
			(Entry << XAE_VLAN_TABL_VID_START_OFFSET), Reg);
	}

	xdbg_printf(XDBG_DEBUG_GENERAL,
			"XAxiEthernet_SetVidTable: returning SUCCESS\n");
	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
* XAxiEthernet_GetVidTable gets VID table content includes new VLAN ID, strip
* and tag enable bits.
*
* The device must be stopped to use this function.<br><br>
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @param	Entry is the hardware storage location/index to program
*		updated VID value, strip, or tag value. The value must be
*		between 0..0xFFF.
* @param	VidPtr references the location to store the result.
*		This function stores the Vid value indexed by Entry into
*		this location.
* @param	StripPtr references the location to store the result.
*		This function stores the strip enable bit value indexed
*		by Entry into this location.
* @param	TagPtr references the location to store the result. This
*		function stores the tag enable bit value indexed by Entry
*		into this location.
* @param	Dir must be either XAE_TX or XAE_RX.
*
* @return	None.
*
* @note
*
* The hardware requires the table to be 'indexed' with Entry and
* must be 0x000..0xFFF.
*
* Bits layout is bbbb bbbb bbbb b b
*                VLAN ID (12b), | |
*                               | VLAN double tag enable bit
*                               VLAN strip enable bit
*
*****************************************************************************/
void XAxiEthernet_GetVidTable(XAxiEthernet *InstancePtr, u32 Entry,
			u32 *VidPtr, u8 *StripPtr, u8 *TagPtr, int Dir)
{
	u32 Reg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(Entry <= XAE_MAX_VLAN_TABL_ENTRY);
	Xil_AssertVoid(VidPtr != NULL);
	Xil_AssertVoid(StripPtr != NULL);
	Xil_AssertVoid(TagPtr != NULL);
	Xil_AssertVoid((Dir == XAE_TX) || (Dir == XAE_RX));

	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_GetVidTable\n");

	/* Transmit direction */
	if (XAE_TX == Dir) {
		Reg = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
			XAE_TX_VLAN_DATA_OFFSET +
				(Entry << XAE_VLAN_TABL_VID_START_OFFSET));
	} else { /* Receive direction */
		Reg = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
			XAE_RX_VLAN_DATA_OFFSET +
				(Entry << XAE_VLAN_TABL_VID_START_OFFSET));
	}

	*VidPtr   = (Reg >> XAE_VLAN_TABL_VID_START_OFFSET);
	*StripPtr = (Reg >> XAE_VLAN_TABL_STRP_STRT_OFFSET) &
					XAE_VLAN_TABL_STRP_ENTRY_MASK;
	*TagPtr   = Reg & XAE_VLAN_TABL_TAG_ENTRY_MASK;

	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_GetVidTable: done\n");
}


/*****************************************************************************/
/**
* XAxiEthernet_AddExtMulticastGroup adds an entry to the multicast Ethernet
* address table. The new entry, represents a group of MAC addresses
* based on the contents of AddressPtr. AddressPtr is one member of the MAC
* address set in the newly added entry.
*
* The device must be stopped to use this function.<br><br>
*
* Once an Ethernet address is programmed, the Axi Ethernet device will begin
* receiving data sent from that address. The Axi Ethernet hardware does not
* have a control bit to disable multicast filtering. The only way to prevent
* the Axi Ethernet device from receiving messages from an Ethernet address in
* the multicast table is to clear it with
* XAxiEthernet_ClearExtMulticastGroup().
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @param	AddressPtr is a pointer to the 6-byte Ethernet address to add.
*
* @return
*		- XST_SUCCESS.on successful completion.
*		- XST_DEVICE_IS_STARTED if the Axi Ethernet device is not
*		  stopped.
*		- XST_INVALID_PARAM if the input MAC address is not between
*		  01:00:5E:00:00:00 and 01:00:5E:7F:FF:FF per RFC1112.
*
* @note
*
* This routine consider all 2**23 possible multicast Ethernet addresses to be
* 8Mx1 bit or 1M bytes memory area. All defined multicast addresses are from
* 01.00.5E.00.00.00 to 01.00.5E.7F.FF.FF
* The most significant 25 bit out of 48 bit are static, so they will not be
* part of calculation.
*
* The hardware requires to 'index' with bit 22-8, 15 bits in
* total. The least significant byte/8 bits are considered a group.
*
* This API operates at a group (256 MAC addresses) for hardware to do the
* first layer address filtering. It is user's responsibility to provision
* this table appropriately.
*
*****************************************************************************/
int XAxiEthernet_AddExtMulticastGroup(XAxiEthernet *InstancePtr,
							void *AddressPtr)
{
	u32 Loc;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(AddressPtr != NULL);

	u8 *Aptr = (u8 *) AddressPtr;
	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_AddExtMulticastGroup\n");

	/*
	 * The device must be stopped before setting the multicast table.
	 */
	if (InstancePtr->IsStarted == XIL_COMPONENT_IS_STARTED) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_AddExtMulticastGroup:returning DEVICE_IS_STARTED\n");

		return (XST_DEVICE_IS_STARTED);
	}

	/* Check HW capability */
	if (!XAxiEthernet_IsExtFuncCap(InstancePtr) ||
		!XAxiEthernet_IsExtMcastEnable(InstancePtr)) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_AddExtMulticastGroup:returning DEVICE_NO_FEATURE\n");
		return (XST_NO_FEATURE);
	}

	/*
	 * Verify if address is a good/valid multicast address, between
	 * 01:00:5E:00:00:00 to 01:00:5E:7F:FF:FF per RFC1112.
	 * This address is referenced to be index to the table.
	 */
	if ((0x01 != Aptr[0]) || (0x00 != Aptr[1]) || (0x5e != Aptr[2]) ||
		(0x0 != (Aptr[3] & 0x80)))
		return (XST_INVALID_PARAM);

	/*
	 * Program hardware table, index : bit 22-8. Bit 23 is 0,
	 * when passed the if statement above.
	 * note: if the index/bits changed, need to revisit calculation.
	 */
	Loc  = Aptr[3];
	Loc  = Loc << 8;
	Loc |= Aptr[4];

	/* Word aligned address access */
	Loc  = Loc << 2;

	XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
			XAE_MCAST_TABLE_OFFSET + Loc, 0x01);

	xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_AddExtMulticastGroup: returning SUCCESS\n");

	return (XST_SUCCESS);
}


/*****************************************************************************/
/**
* XAxiEthernet_ClearExtMulticastGroup clears input multicast Ethernet address
* group from table.
*
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @param	AddressPtr is a pointer to the 6-byte Ethernet address to clear.
*
* @return
*		- XST_SUCCESS on successful completion, returns XST_SUCCESS.
*		- XST_DEVICE_IS_STARTED if the Axi Ethernet device is not
*                 stopped
*		- XST_INVALID_PARAM if input MAC address is not between
*		  01:00:5E:00:00:00 and 01:00:5E:7F:FF:FF per RFC1112.
*
* @note
*
* Please reference XAxiEthernet_AddExtMulticastGroup for multicast address
* index and bit value calculation.
*
* In table, hardware requires to 'index' with bit 22-8, 15 bits in
* total. The least significant byte/8 bits are considered a group.
*
* There is a scenario that might introduce issues:
* When multicast tables are programmed initially to accept
* 01:00:5E:12:34:56 and 01:00:5E:12:34:78 but later decided to clear
* 01:00:5E:12:34:78. Without validating all possible combinations at the
* indexed entry, multicast table might be misconfigured and drop
* frames.
*
* When clearing a multicast address table entry, note that a whole group of
* mac addresses will no longer be accepted - this because an entry in the
* table represents multiple(256) mac addresses.
*
* The device must be stopped to use this function.<br><br>
* This API operates at a group (256 MAC addresses) level for hardware to
* perform the first layer address filtering. It is user's responsibility to
* provision this table appropriately.
*
*****************************************************************************/
int XAxiEthernet_ClearExtMulticastGroup(XAxiEthernet *InstancePtr,
							void *AddressPtr)
{
	u32 Loc;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(AddressPtr != NULL);

	u8 *Aptr = (u8 *) AddressPtr;
	xdbg_printf(XDBG_DEBUG_GENERAL,
				"XAxiEthernet_ClearExtMulticastGroup\n");

	/*
	 * The device must be stopped before clearing the multicast table.
	 */
	if (InstancePtr->IsStarted == XIL_COMPONENT_IS_STARTED) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_ClearExtMulticastGroup:returning DEVICE_IS_STARTED\n");
		return (XST_DEVICE_IS_STARTED);
	}

	/* Check HW capability */
	if (!XAxiEthernet_IsExtFuncCap(InstancePtr) ||
		!XAxiEthernet_IsExtMcastEnable(InstancePtr)) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_ClearExtMulticastGroup:returning DEVICE_NO_FEATURE\n");
		return (XST_NO_FEATURE);
	}

	/*
	 * Verify if address is a good/valid multicast address, between
	 * 01:00:5E:00:00:00 to 01:00:5E:7F:FF:FF per RFC1112.
	 * This address is referenced to be index to the table.
	 */
	if ((0x01 != Aptr[0]) || (0x00 != Aptr[1]) || (0x5e != Aptr[2]) ||
		(0x0 != (Aptr[3] & 0x80)))
		return (XST_INVALID_PARAM);

	Loc  = Aptr[3];
	Loc  = Loc << 8;
	Loc |= Aptr[4];

	/* Word aligned address access */
	Loc  = Loc << 2;

	XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
		XAE_MCAST_TABLE_OFFSET + Loc, 0x00);

	xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_ClearExtMulticastGroup: returning SUCCESS\n");

	return (XST_SUCCESS);
}


/*****************************************************************************/
/**
* XAxiEthernet_GetExtMulticastGroup returns whether the given Ethernet address
* group is stored in the table.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @param	AddressPtr is a pointer to the 6-byte Ethernet address.
*
* @return
*		- TRUE if it is an acceptable multicast MAC address
*		  and the group is present in the table.
*		- FALSE if it is not a valid multicast MAC address
*		  or the group was not found in the table.
*
* @note
*
* In the table, hardware requires to 'index' with bit 22-8, 15 bits in
* total. The least significant byte/8 bits are considered a group.
* This API operates at a group (256 MAC addresses) level.
*
*****************************************************************************/
int XAxiEthernet_GetExtMulticastGroup(XAxiEthernet *InstancePtr,
							void *AddressPtr)
{
	u32 Loc;
	u8 Bit;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(AddressPtr != NULL);

	u8 *Aptr = (u8 *) AddressPtr;
	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_GetExtMulticastGroup\n");

	/*
	 * Verify if address is a good/valid multicast address, between
	 * 01:00:5E:00:00:00 to 01:00:5E:7F:FF:FF per RFC1112.
	 * This address is referenced to be index to the table.
	 */
	if ((0x01 != Aptr[0]) || (0x00 != Aptr[1]) || (0x5e != Aptr[2]) ||
		(0x0 != (Aptr[3] & 0x80)))
		return (FALSE);

	Loc  = Aptr[3];
	Loc  = Loc << 8;
	Loc |= Aptr[4];

	/*
	 * Word aligned address access
	 */
	Loc  = Loc << 2;

	Bit = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
		XAE_MCAST_TABLE_OFFSET + Loc);

	if (Bit) {
		return (TRUE);
	} else {
		return (FALSE);
	}
	xdbg_printf(XDBG_DEBUG_GENERAL,
				"XAxiEthernet_GetExtMulticastGroup:done\n");
}


/*****************************************************************************/
/**
* XAxiEthernet_DumpExtMulticastGroup dumps ALL provisioned acceptable multicast
* MAC in the Axi Ethernet device's multicast table.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @return	None.
*
* @note
*
* Hardware requires to 'index' with bit 22-8, 15 bits in
* total. The least significant byte/8 bits are considered a set.
*
* This API operates at a set (256 MAC addresses) level.
*
*****************************************************************************/
void XAxiEthernet_DumpExtMulticastGroup(XAxiEthernet *InstancePtr)
{
	u32 Loc;
	u32 Index;
	u8  Bit;
	char MacAddr[6];

	Xil_AssertVoid(InstancePtr != NULL);

	/*
	 * Pre-populated these bytes, we know and guarantee these if
	 * provisioned through the XAxiEthernet_AddExtMulticastGroup().
	 */
	MacAddr[0] = 0x01;
	MacAddr[1] = 0x00;
	MacAddr[2] = 0x5E;

	for (Index = 0; Index < (1 << 15); Index++) {
		MacAddr[3] = Index << 16;
		MacAddr[4] = Index << 8;
		MacAddr[5] = 0;

		Loc  = MacAddr[3];
		Loc |= MacAddr[4] << 8;

		/* Word aligned address access */
		Loc  = Loc << 2;

		Bit = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
			XAE_MCAST_TABLE_OFFSET + Loc);
		if (Bit) {
			xdbg_printf(XDBG_DEBUG_GENERAL,
			"%x:%x:%x:%x:%x:%x\n", MacAddr[5], MacAddr[4],
			MacAddr[3], MacAddr[2], MacAddr[1], MacAddr[0]);
		}
	}
}
/** @} */
