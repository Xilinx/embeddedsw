/******************************************************************************
*
* Copyright (C) 2010 - 2018 Xilinx, Inc.  All rights reserved.
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
******************************************************************************/
/**
*
* @file xaxiethernet.c
* @addtogroup axiethernet_v5_8
* @{
*
* The APIs in this file takes care of the primary functionalities of the driver.
* The APIs in this driver take care of the following:
*	- Starting or stopping the Axi Ethernet device
*	- Initializing and resetting the Axi Ethernet device
*	- Setting MAC address and speed/duplex of the device
*	- Provide means for controlling the PHY and communicating with it.
*	- Turn on/off various features/options provided by the Axi Ethernet
*	  device.
* See xaxiethernet.h for a detailed description of the driver.
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a asa  6/30/10 First release based on the ll temac driver
* 1.02a asa  2/16/11 Made changes in XAxiEthernet_Reset to insert delays.
* 3.02a srt  4/13/13 Removed Warnings (CR 704998).
* 5.1   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
*                     Changed the prototype of XAxiEthernet_CfgInitialize API.
* 5.7   srm  01/16/18 Implemented poll timeout API which replaces while loops
*                     to ensure a deterministic time delay.
* 5.8   rsp  07/20/18 Fix cppcheck warning in Aptr assignment.
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/
#include "xaxiethernet.h"
#include "sleep.h"
/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

static void InitHw(XAxiEthernet *InstancePtr);	/* HW reset */

/************************** Variable Definitions *****************************/

xdbg_stmnt(int indent_on = 0;)
xdbg_stmnt(u32 _xaxiethernet_rir_value;)

/*****************************************************************************/
/**
*
* XAxiEthernet_CfgInitialize initializes an AXI Ethernet device along with the
* <i>InstancePtr</i> that references it.
*
* The PHY is setup independently from the Ethernet core. Use the MII or
* whatever other interface may be present for setup.
*
* @param	InstancePtr references the memory instance to be associated
*		with the AXI Ethernet core instance upon initialization.
* @param	CfgPtr references the structure holding the hardware
*		configuration for the Axi Ethernet core to initialize.
* @param	EffectiveAddress is the processor address used to access the
*		base address of the AXI Ethernet instance. In systems with an
*		MMU and virtual memory, <i>EffectiveAddress</i> is the
*		virtual address mapped to the physical in
*		<code>ConfigPtr->Config.BaseAddress</code>. In systems without
*		an active MMU, <i>EffectiveAddress</i> should be set to the
*		same value as <code>ConfigPtr->Config.BaseAddress</code>.
*
* @return	XST_SUCCESS.
*
* @note		None.
*
******************************************************************************/
int XAxiEthernet_CfgInitialize(XAxiEthernet *InstancePtr,
				XAxiEthernet_Config *CfgPtr,
				UINTPTR EffectiveAddress)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Clear instance memory and make copy of configuration */
	memset(InstancePtr, 0, sizeof(XAxiEthernet));
	memcpy(&InstancePtr->Config, CfgPtr, sizeof(XAxiEthernet_Config));

	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_CfgInitialize\n");

	/* Set device base address */
	InstancePtr->Config.BaseAddress = EffectiveAddress;

	/* Reset the hardware and set default options */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	XAxiEthernet_Reset(InstancePtr);

	xdbg_printf(XDBG_DEBUG_GENERAL,
			"AxiTEthernet_CfgInitialize: returning SUCCESS\n");
	return XST_SUCCESS;
}


/*****************************************************************************/
/**
*
* XAxiEthernet_Initialize initializes an AXI Ethernet device along with the
* <i>InstancePtr</i> that references it.
*
* The PHY is setup independently from the Ethernet core. Use the MII or
* whatever other interface may be present for setup.
*
* @param	InstancePtr references the memory instance to be associated
*		with the AXI Ethernet core instance upon initialization.
* @param	CfgPtr references the structure holding the hardware
*		configuration for the Axi Ethernet core to initialize.
* @param	EffectiveAddress is the processor address used to access the
*		base address of the AXI Ethernet instance. In systems with an
*		MMU and virtual memory, <i>EffectiveAddress</i> is the
*		virtual address mapped to the physical in
*		<code>ConfigPtr->Config.BaseAddress</code>. In systems without
*		an active MMU, <i>EffectiveAddress</i> should be set to the
*		same value as <code>ConfigPtr->Config.BaseAddress</code>.
*
* @return	XST_SUCCESS.
*
* @note		When user calls this function he should ensure the hardware
*		is in a quiescent state by reseting all the hardware
*		Configurations.
*
******************************************************************************/
int XAxiEthernet_Initialize(XAxiEthernet *InstancePtr,
			    XAxiEthernet_Config *CfgPtr,
			    UINTPTR EffectiveAddress)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Clear instance memory and make copy of configuration */
	memset(InstancePtr, 0, sizeof(XAxiEthernet));
	memcpy(&InstancePtr->Config, CfgPtr, sizeof(XAxiEthernet_Config));

	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_CfgInitialize\n");

	/* Set device base address */
	InstancePtr->Config.BaseAddress = EffectiveAddress;

	/* Set default options */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* XAxiEthernet_Start starts the Axi Ethernet device as follows:
*	- Enable transmitter if XAE_TRANSMIT_ENABLE_OPTION is set
*	- Enable receiver if XAE_RECEIVER_ENABLE_OPTION is set
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @return	None.
*
* @note		None.
*
*
******************************************************************************/
void XAxiEthernet_Start(XAxiEthernet *InstancePtr)
{
	u32 Reg;

	/* Assert bad arguments and conditions */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);


	/* If already started, then there is nothing to do */
	if (InstancePtr->IsStarted == XIL_COMPONENT_IS_STARTED) {
		return;
	}

	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_Start\n");

	/* Enable transmitter if not already enabled */
	if (InstancePtr->Options & XAE_TRANSMITTER_ENABLE_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL, "enabling transmitter\n");
		Reg = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_TC_OFFSET);
		if (!(Reg & XAE_TC_TX_MASK)) {
			xdbg_printf(XDBG_DEBUG_GENERAL,
				"transmitter not enabled, enabling now\n");
			XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
							XAE_TC_OFFSET,
							Reg | XAE_TC_TX_MASK);
		}
		xdbg_printf(XDBG_DEBUG_GENERAL, "transmitter enabled\n");
	}

	/* Enable receiver */
	if (InstancePtr->Options & XAE_RECEIVER_ENABLE_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL, "enabling receiver\n");
		Reg = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_RCW1_OFFSET);
		if (!(Reg & XAE_RCW1_RX_MASK)) {
			xdbg_printf(XDBG_DEBUG_GENERAL,
				"receiver not enabled, enabling now\n");

			XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
							XAE_RCW1_OFFSET,
							Reg | XAE_RCW1_RX_MASK);
		}
		xdbg_printf(XDBG_DEBUG_GENERAL, "receiver enabled\n");
	}

	/* Mark as started */
	InstancePtr->IsStarted = XIL_COMPONENT_IS_STARTED;
	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_Start: done\n");
}

/*****************************************************************************/
/**
* XAxiEthernet_Stop gracefully stops the Axi Ethernet device as follows:
*	- Disable all interrupts from this device
*	- Disable the receiver
*
* XAxiEthernet_Stop does not modify any of the current device options.
*
* Since the transmitter is not disabled, frames currently in internal buffers
* or in process by a DMA engine are allowed to be transmitted.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @return	None
*
* @note		None.
*
*
******************************************************************************/
void XAxiEthernet_Stop(XAxiEthernet *InstancePtr)
{
	u32 Reg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);


	/* If already stopped, then there is nothing to do */
	if (InstancePtr->IsStarted == 0) {
		return;
	}

	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_Stop\n");
	xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_Stop: disabling interrupts\n");

	/* Disable interrupts */
	XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
							XAE_IE_OFFSET, 0);

	xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_Stop: disabling receiver\n");

	/* Disable the receiver */
	Reg = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_RCW1_OFFSET);
	Reg &= ~XAE_RCW1_RX_MASK;
	XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
							XAE_RCW1_OFFSET, Reg);

	/*
	 * Stopping the receiver in mid-packet causes a dropped packet
	 * indication from HW. Clear it.
	 */
	/* get the interrupt pending register */
	Reg = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_IP_OFFSET);
	if (Reg & XAE_INT_RXRJECT_MASK) {
		/* set the interrupt status register to clear the interrupt */
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
				XAE_IS_OFFSET, XAE_INT_RXRJECT_MASK);
	}

	/* Mark as stopped */
	InstancePtr->IsStarted = 0;
	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_Stop: done\n");
}


/*****************************************************************************/
/**
* XAxiEthernet_Reset does not perform a soft reset of the AxiEthernet core.
* AxiEthernet hardware is reset by the device connected to the AXI4-Stream
* interface.
* This function inserts some delay before proceeding to check for MgtRdy bit.
* The delay is necessary to be at a safe side. It takes a while for the reset
* process to complete and for any of the AxiEthernet registers to be accessed.
* It then checks for MgtRdy bit in IS register to know if AxiEthernet reset
* is completed or not. Subsequently it calls one more driver function to
* complete the AxiEthernet hardware initialization.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @note		It is the responsibility of the user to reset the AxiEthernet
*		hardware before using it. AxiEthernet hardware should be reset
*		through the device connected to the AXI4-Stream interface of
*		AxiEthernet.
*
******************************************************************************/
void XAxiEthernet_Reset(XAxiEthernet *InstancePtr)
{
	volatile s32 TimeoutLoops;
	u32 value=0U;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Add delay of 1 sec to give enough time to the core to come
	 * out of reset. Till the time core comes out of reset none of the
	 * AxiEthernet registers are accessible including the IS register.
	 */
	sleep(1);
	/*
	 * Check the status of the MgtRdy bit in the interrupt status
	 * registers. This must be done to allow the MGT clock to become stable
	 * for the Sgmii and 1000BaseX PHY interfaces. No other register reads
	 * will be valid until this bit is valid.
	 * The bit is always a 1 for all other PHY interfaces.
	 */
	TimeoutLoops = Xil_poll_timeout(Xil_In32,InstancePtr->Config.BaseAddress+
	      XAE_IS_OFFSET, value, (value&XAE_INT_MGTRDY_MASK)!=0,
		           XAE_RST_DEFAULT_TIMEOUT_VAL);

	if(-1 == TimeoutLoops) {
		Xil_AssertVoidAlways();
	}

	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_Reset\n");

	/* Stop the device and reset HW */
	XAxiEthernet_Stop(InstancePtr);
	InstancePtr->Options = XAE_DEFAULT_OPTIONS;

	/* Setup HW */
	InitHw(InstancePtr);
}


/******************************************************************************
*
* InitHw (internal use only) performs a one-time setup of a Axi Ethernet device.
* The setup performed here only need to occur once after any reset.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @note		None.
*
*
******************************************************************************/
static void InitHw(XAxiEthernet *InstancePtr)
{
	u32 Reg;

	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet InitHw\n");


	/* Disable the receiver */
	Reg = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_RCW1_OFFSET);
	Reg &= ~XAE_RCW1_RX_MASK;
	XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
							XAE_RCW1_OFFSET, Reg);

	/*
	 * Stopping the receiver in mid-packet causes a dropped packet
	 * indication from HW. Clear it.
	 */
	/* Get the interrupt pending register */
	Reg = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
								XAE_IP_OFFSET);
	if (Reg & XAE_INT_RXRJECT_MASK) {
		/*
		 * Set the interrupt status register to clear the pending
		 * interrupt
		 */
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
					XAE_IS_OFFSET, XAE_INT_RXRJECT_MASK);
	}

	/*
	 * Sync default options with HW but leave receiver and transmitter
	 * disabled. They get enabled with XAxiEthernet_Start() if
	 * XAE_TRANSMITTER_ENABLE_OPTION and XAE_RECEIVER_ENABLE_OPTION
	 * are set
	 */
	XAxiEthernet_SetOptions(InstancePtr, InstancePtr->Options &
					~(XAE_TRANSMITTER_ENABLE_OPTION |
					XAE_RECEIVER_ENABLE_OPTION));

	XAxiEthernet_ClearOptions(InstancePtr, ~InstancePtr->Options);

	/* Set default MDIO divisor */
	XAxiEthernet_PhySetMdioDivisor(InstancePtr, XAE_MDIO_DIV_DFT);

	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet InitHw: done\n");
}

/*****************************************************************************/
/**
 * XAxiEthernet_SetMacAddress sets the MAC address for the Axi Ethernet device,
 * specified by <i>InstancePtr</i> to the MAC address specified by
 * <i>AddressPtr</i>.
 * The Axi Ethernet device must be stopped before calling this function.
 *
 * @param	InstancePtr is a pointer to the Axi Ethernet instance to be
 *		worked on.
 * @param	AddressPtr is a reference to the 6-byte MAC address to set.
 *
 * @return
 *		- XST_SUCCESS on successful completion.
 *		- XST_DEVICE_IS_STARTED if the Axi Ethernet device has not
 *		  stopped,
 *
 * @note
 * This routine also supports the extended/new VLAN and multicast mode. The
 * XAE_RAF_NEWFNCENBL_MASK bit dictates which offset will be configured.
 *
 ******************************************************************************/
int XAxiEthernet_SetMacAddress(XAxiEthernet *InstancePtr, void *AddressPtr)
{
	u32 MacAddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(AddressPtr != NULL);

	u8 *Aptr = (u8 *) AddressPtr;

	/* Be sure device has been stopped */
	if (InstancePtr->IsStarted == XIL_COMPONENT_IS_STARTED) {
		return (XST_DEVICE_IS_STARTED);
	}

	xdbg_printf(XDBG_DEBUG_GENERAL,
	"XAxiEthernet_SetMacAddress: setting mac address to:0x%08x%8x%8x%8x%8x%8x\n",
		Aptr[0],  Aptr[1], Aptr[2], Aptr[3], Aptr[4], Aptr[5]);

	/* Prepare MAC bits in either UAW0/UAWL */
	MacAddr = Aptr[0];
	MacAddr |= Aptr[1] << 8;
	MacAddr |= Aptr[2] << 16;
	MacAddr |= Aptr[3] << 24;
	/* Check to see if it is in extended/new mode. */
	if (!(XAxiEthernet_IsExtFuncCap(InstancePtr))) {
		/*
		 * Set the MAC bits [31:0] in UAW0.
		 * Having Aptr be unsigned type prevents the following
		 * operations from sign extending.
		 */
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
						XAE_UAW0_OFFSET, MacAddr);

		/* There are reserved bits in UAW1 so don't affect them */
		MacAddr = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
						XAE_UAW1_OFFSET);
		MacAddr &= ~XAE_UAW1_UNICASTADDR_MASK;

		/* Set MAC bits [47:32] in UAW1 */
		MacAddr |= Aptr[4];
		MacAddr |= Aptr[5] << 8;
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
				XAE_UAW1_OFFSET, MacAddr);

		return (XST_SUCCESS);
	} else { /* Extended mode */
		/*
		 * Set the MAC bits [31:0] in UAWL register.
		 * Having Aptr be unsigned type prevents the following
		 * operations from sign extending.
		 */
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
						XAE_UAWL_OFFSET, MacAddr);

		/* Set MAC bits [47:32] in UAWU register. */
		MacAddr  = 0;
		MacAddr |= Aptr[4];
		MacAddr |= Aptr[5] << 8;
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
						XAE_UAWU_OFFSET, MacAddr);

		return (XST_SUCCESS);
	}
}


/*****************************************************************************/
/**
 * XAxiEthernet_GetMacAddress gets the MAC address for the Axi Ethernet,
 * specified by <i>InstancePtr</i> into the memory buffer specified by
 * <i>AddressPtr</i>.
 *
 * @param	InstancePtr is a pointer to the Axi Ethernet instance to be
 *		worked on.
 * @param	AddressPtr references the memory buffer to store the retrieved
 *		MAC address. This memory buffer must be at least 6 bytes in
 *		length.
 *
 * @return	None.
 *
 * @note
 *
 * This routine also supports the extended/new VLAN and multicast mode. The
 * XAE_RAF_NEWFNCENBL_MASK bit dictates which offset will be configured.
 *
 ******************************************************************************/
void XAxiEthernet_GetMacAddress(XAxiEthernet *InstancePtr, void *AddressPtr)
{
	u32 MacAddr;
	u8 *Aptr = (u8 *) AddressPtr;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(AddressPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);


	/* Check to see if it is in extended/new mode. */
	if (!(XAxiEthernet_IsExtFuncCap(InstancePtr))) {
		/* Read MAC bits [31:0] in UAW0 */
		MacAddr = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_UAW0_OFFSET);
		Aptr[0] = (u8) MacAddr;
		Aptr[1] = (u8) (MacAddr >> 8);
		Aptr[2] = (u8) (MacAddr >> 16);
		Aptr[3] = (u8) (MacAddr >> 24);

		/* Read MAC bits [47:32] in UAW1 */
		MacAddr = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_UAW1_OFFSET);
		Aptr[4] = (u8) MacAddr;
		Aptr[5] = (u8) (MacAddr >> 8);
	} else { /* Extended/new mode */
		/* Read MAC bits [31:0] in UAWL register */
		MacAddr = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_UAWL_OFFSET);
		Aptr[0] = (u8) MacAddr;
		Aptr[1] = (u8) (MacAddr >> 8);
		Aptr[2] = (u8) (MacAddr >> 16);
		Aptr[3] = (u8) (MacAddr >> 24);

		/* Read MAC bits [47:32] in UAWU register */
		MacAddr = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_UAWU_OFFSET);
		Aptr[4] = (u8) MacAddr;
		Aptr[5] = (u8) (MacAddr >> 8);
	}
}


/*****************************************************************************/
/**
 * XAxiEthernet_UpdateDepOption check and update dependent options for
 * new/extended features. This is a helper function that is meant to be called
 * by XAxiEthernet_SetOptions() and XAxiEthernet_ClearOptions().
 *
 * @param	InstancePtr is a pointer to the Axi Ethernet instance to be
 *		worked on.
 *
 * @return	Dependent options that are required to set/clear per
 *		hardware requirement.
 *
 * @note
 *
 * This helper function collects the dependent OPTION(s) per hardware design.
 * When conflicts arises, extended features have precedence over legacy ones.
 * Two operations to be considered,
 * 1. Adding extended options. If XATE_VLAN_OPTION is enabled and enable one of
 *	extended VLAN options, XATE_VLAN_OPTION should be off and configure to
 *	hardware.
 *	However, axi-ethernet instance Options variable still holds
 *	XATE_VLAN_OPTION so when all of the extended feature are removed,
 *	XATE_VLAN_OPTION can be effective and configured to hardware.
 * 2. Removing extended options. Remove extended option can not just remove
 *	the selected extended option and dependent options. All extended
 *	options need to be verified and remained when one or more extended
 *	options are enabled.
 *
 * Dependent options are :
 *	- XAE_VLAN_OPTION,
 *	- XAE_JUMBO_OPTION
 *	- XAE_FCS_INSERT_OPTION,
 *	- XAE_FCS_STRIP_OPTION
 *	- XAE_PROMISC_OPTION.
 *
 ******************************************************************************/
static u32 XAxiEthernet_UpdateDepOptions(XAxiEthernet *InstancePtr)
{
	/*
	 * This is a helper function for XAxiEthernet_[Set|Clear]Options(),
	 * verification has been done before invoke this function.
	 */
	u32 DepOptions = InstancePtr->Options;

	/*
	 * The extended/new features require some OPTIONS to be on/off per
	 * hardware design. We determine these extended/new functions here
	 * first and also on/off other OPTIONS later. So that dependent
	 * OPTIONS are in sync and _[Set|Clear]Options() can be performed
	 * seamlessly.
	 */

	/* Enable extended multicast option */
	if (DepOptions & XAE_EXT_MULTICAST_OPTION) {
		/*
		 * When extended multicast module is enabled in HW,
		 * XAE_PROMISC_OPTION is required to be enabled.
		 */
		if (XAxiEthernet_IsExtMcast(InstancePtr)) {
			DepOptions |= XAE_PROMISC_OPTION;
			xdbg_printf(XDBG_DEBUG_GENERAL,
				"CheckDepOptions: enabling ext multicast\n");
		} else {
			xdbg_printf(XDBG_DEBUG_ERROR,
			"EXT MULTICAST is not built in hardware\n");
		}
	}

	/* Enable extended transmit VLAN translation option */
	if (DepOptions & XAE_EXT_TXVLAN_TRAN_OPTION) {
		/*
		 * Check if hardware is built with extend TX VLAN translation.
		 * if not, output an information message.
		 */
		if (XAxiEthernet_IsTxVlanTran(InstancePtr)) {
			DepOptions |= XAE_FCS_INSERT_OPTION;
			DepOptions &= ~XAE_VLAN_OPTION;
			xdbg_printf(XDBG_DEBUG_GENERAL,
			"CheckDepOptions: enabling ext Tx VLAN trans\n");
		} else {
			xdbg_printf(XDBG_DEBUG_GENERAL,
			"TX VLAN TRANSLATION is not built in hardware\n");
		}
	}

	/* Enable extended receive VLAN translation option */
	if (DepOptions & XAE_EXT_RXVLAN_TRAN_OPTION) {
		/*
		 * Check if hardware is built with extend RX VLAN translation.
		 * if not, output an information message.
		 */
		if (XAxiEthernet_IsRxVlanTran(InstancePtr)) {
			DepOptions |= XAE_FCS_STRIP_OPTION;
			DepOptions &= ~XAE_VLAN_OPTION;
			xdbg_printf(XDBG_DEBUG_GENERAL,
			"CheckDepOptions: enabling ext Rx VLAN trans\n");
		} else {
			xdbg_printf(XDBG_DEBUG_GENERAL,
			"RX VLAN TRANSLATION is not built in hardware\n");
		}
	}

	/* Enable extended transmit VLAN tag option */
	if (DepOptions & XAE_EXT_TXVLAN_TAG_OPTION) {
		/*
		 * Check if hardware is built with extend TX VLAN tagging.
		 * if not, output an information message.
		 */
		if (XAxiEthernet_IsTxVlanTag(InstancePtr)) {
			DepOptions |= XAE_FCS_INSERT_OPTION;
			DepOptions &= ~XAE_VLAN_OPTION;
			DepOptions |= XAE_JUMBO_OPTION;
			xdbg_printf(XDBG_DEBUG_GENERAL,
			"CheckDepOptions: enabling ext Tx VLAN tag\n");
		} else {
			xdbg_printf(XDBG_DEBUG_ERROR,
			"TX VLAN TAG is not built in hardware\n");
		}
	}

	/* Enable extended receive VLAN tag option */
	if (DepOptions & XAE_EXT_RXVLAN_TAG_OPTION) {
		/*
		 * Check if hardware is built with extend RX VLAN tagging.
		 * if not, output an information message.
		 */
		if (XAxiEthernet_IsRxVlanTag(InstancePtr)) {
			DepOptions |= XAE_FCS_STRIP_OPTION;
			DepOptions &= ~XAE_VLAN_OPTION;
			DepOptions |= XAE_JUMBO_OPTION;
			xdbg_printf(XDBG_DEBUG_GENERAL,
			"CheckDepOptions: enabling ext Rx VLAN tag\n");
		} else {
			xdbg_printf(XDBG_DEBUG_GENERAL,
			"RX VLAN TAG is not built in hardware\n");
		}
	}

	/* Enable extended transmit VLAN strip option */
	if (DepOptions & XAE_EXT_TXVLAN_STRP_OPTION) {
		/*
		 * Check if hardware is built with extend TX VLAN stripping.
		 * if not, output an information message.
		 */
		if (XAxiEthernet_IsTxVlanStrp(InstancePtr)) {
			DepOptions |= XAE_FCS_INSERT_OPTION;
			DepOptions &= ~XAE_VLAN_OPTION;
			DepOptions |= XAE_JUMBO_OPTION;
			xdbg_printf(XDBG_DEBUG_GENERAL,
			"CheckDepOptions: enabling ext Tx VLAN strip\n");
		} else {
			xdbg_printf(XDBG_DEBUG_ERROR,
			"TX VLAN STRIP is not built in hardware\n");
		}
	}

	/* Enable extended receive VLAN strip option */
	if (DepOptions & XAE_EXT_RXVLAN_STRP_OPTION) {
		/*
		 * Check if hardware is built with extend RX VLAN stripping.
		 * if not, output an information message.
		 */
		if (XAxiEthernet_IsRxVlanStrp(InstancePtr)) {
			DepOptions |= XAE_FCS_STRIP_OPTION;
			DepOptions &= ~XAE_VLAN_OPTION;
			DepOptions |= XAE_JUMBO_OPTION;
			xdbg_printf(XDBG_DEBUG_GENERAL,
			"CheckDepOptions: enabling ext Rx VLAN strip\n");
		} else {
			xdbg_printf(XDBG_DEBUG_ERROR,
			"RX VLAN STRIP is not built in hardware\n");
		}
	}

	/*
	 * Options and dependent Options together is what hardware and user
	 * are happy with. But we still need to keep original options
	 * in case option(s) are set/cleared, overall options can be managed.
	 * Return DepOptions to XAxiEthernet_[Set|Clear]Options for final
	 * configuration.
	 */
	return(DepOptions);
}

/*****************************************************************************/
/**
* XAxiEthernet_SetOptions enables the options, <i>Options</i> for the
* Axi Ethernet, specified by <i>InstancePtr</i>. Axi Ethernet should be
* stopped with XAxiEthernet_Stop() before changing options.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @param	Options is a bitmask of OR'd XAE_*_OPTION values for options to
*		set. Options not specified are not affected.
*
* @return
*		- XST_SUCCESS on successful completion.
*		- XST_DEVICE_IS_STARTED if the device has not been stopped.
*
*
* @note
* See xaxiethernet.h for a description of the available options.
*
*
******************************************************************************/
int XAxiEthernet_SetOptions(XAxiEthernet *InstancePtr, u32 Options)
{
	u32 Reg;	/* Generic register contents */
	u32 RegRcw1;	/* Reflects original contents of RCW1 */
	u32 RegTc;	/* Reflects original contents of TC  */
	u32 RegNewRcw1;	/* Reflects new contents of RCW1 */
	u32 RegNewTc;	/* Reflects new contents of TC  */
	u32 DepOptions;	/* Required dependent options for new features */

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Be sure device has been stopped */
	if (InstancePtr->IsStarted == XIL_COMPONENT_IS_STARTED) {
		return (XST_DEVICE_IS_STARTED);
	}

	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_SetOptions\n");

	/*
	 * Set options word to its new value.
	 * The step is required before calling _UpdateDepOptions() since
	 * we are operating on updated options.
	 */
	InstancePtr->Options |= Options;

	/*
	 * There are options required to be on/off per hardware requirement.
	 * Invoke _UpdateDepOptions to check hardware availability and update
	 * options accordingly.
	 */
	DepOptions = XAxiEthernet_UpdateDepOptions(InstancePtr);

	/*
	 * New/extended function bit should be on if any new/extended features
	 * are on and hardware is built with them.
	 */
	if (DepOptions & (XAE_EXT_MULTICAST_OPTION   |
			  XAE_EXT_TXVLAN_TRAN_OPTION |
			  XAE_EXT_RXVLAN_TRAN_OPTION |
			  XAE_EXT_TXVLAN_TAG_OPTION  |
			  XAE_EXT_RXVLAN_TAG_OPTION  |
			  XAE_EXT_TXVLAN_STRP_OPTION |
			  XAE_EXT_RXVLAN_STRP_OPTION)) {

		XAxiEthernet_WriteReg((InstancePtr)->Config.BaseAddress,
			XAE_RAF_OFFSET,
			XAxiEthernet_ReadReg((InstancePtr)->Config.BaseAddress,
			XAE_RAF_OFFSET) | XAE_RAF_NEWFNCENBL_MASK);
	}

	/*
	 * Many of these options will change the RCW1 or TC registers.
	 * To reduce the amount of IO to the device, group these options here
	 * and change them all at once.
	 */
	/* Get current register contents */
	RegRcw1 = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_RCW1_OFFSET);
	RegTc = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_TC_OFFSET);
	RegNewRcw1 = RegRcw1;
	RegNewTc = RegTc;

	xdbg_printf(XDBG_DEBUG_GENERAL,
			"current control regs: RCW1: 0x%0x; TC: 0x%0x\n",
			RegRcw1, RegTc);
	xdbg_printf(XDBG_DEBUG_GENERAL,
			"Options: 0x%0x; default options: 0x%0x\n",Options,
							XAE_DEFAULT_OPTIONS);

	/* Turn on jumbo packet support for both Rx and Tx */
	if (DepOptions & XAE_JUMBO_OPTION) {
		RegNewTc |= XAE_TC_JUM_MASK;
		RegNewRcw1 |= XAE_RCW1_JUM_MASK;
	}

	/* Turn on VLAN packet support for both Rx and Tx */
	if (DepOptions & XAE_VLAN_OPTION) {
		RegNewTc |= XAE_TC_VLAN_MASK;
		RegNewRcw1 |= XAE_RCW1_VLAN_MASK;
	}

	/* Turn on FCS stripping on receive packets */
	if (DepOptions & XAE_FCS_STRIP_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
				"setOptions: enabling fcs stripping\n");
		RegNewRcw1 &= ~XAE_RCW1_FCS_MASK;
	}

	/* Turn on FCS insertion on transmit packets */
	if (DepOptions & XAE_FCS_INSERT_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
				"setOptions: enabling fcs insertion\n");
		RegNewTc &= ~XAE_TC_FCS_MASK;
	}

	/* Turn on length/type field checking on receive packets */
	if (DepOptions & XAE_LENTYPE_ERR_OPTION) {
		RegNewRcw1 &= ~XAE_RCW1_LT_DIS_MASK;
	}

	/* Enable transmitter */
	if (DepOptions & XAE_TRANSMITTER_ENABLE_OPTION) {
		RegNewTc |= XAE_TC_TX_MASK;
	}

	/* Enable receiver */
	if (DepOptions & XAE_RECEIVER_ENABLE_OPTION) {
		RegNewRcw1 |= XAE_RCW1_RX_MASK;
	}

	/* Change the TC or RCW1 registers if they need to be modified */
	if (RegTc != RegNewTc) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
				"setOptions: writing tc: 0x%0x\n", RegNewTc);
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
						XAE_TC_OFFSET, RegNewTc);
	}

	if (RegRcw1 != RegNewRcw1) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
			"setOptions: writing rcw1: 0x%0x\n", RegNewRcw1);
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
						XAE_RCW1_OFFSET, RegNewRcw1);
	}

	/*
	 * Rest of options twiddle bits of other registers. Handle them one at
	 * a time
	 */

	/* Turn on flow control */
	if (DepOptions & XAE_FLOW_CONTROL_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
				"setOptions: enabling flow control\n");
		Reg = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_FCC_OFFSET);
		Reg |= XAE_FCC_FCRX_MASK;
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
							XAE_FCC_OFFSET, Reg);
	}
	xdbg_printf(XDBG_DEBUG_GENERAL,
	"setOptions: rcw1 is now (fcc):0x%0x\n",
	XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_RCW1_OFFSET));

	/* Turn on promiscuous frame filtering (all frames are received ) */
	if (DepOptions & XAE_PROMISC_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
				"setOptions: enabling promiscuous mode\n");
		Reg = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_FMI_OFFSET);
		Reg |= XAE_FMI_PM_MASK;
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
							XAE_FMI_OFFSET, Reg);
	}
	xdbg_printf(XDBG_DEBUG_GENERAL,
	"setOptions: rcw1 is now (afm):0x%0x\n",
	XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_RCW1_OFFSET));

	/* Allow broadcast address filtering */
	if (DepOptions & XAE_BROADCAST_OPTION) {
		Reg = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_RAF_OFFSET);
		Reg &= ~XAE_RAF_BCSTREJ_MASK;
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
							XAE_RAF_OFFSET, Reg);
	}
	xdbg_printf(XDBG_DEBUG_GENERAL,
	"setOptions: rcw1 is now (raf):0x%0x\n",
	XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_RCW1_OFFSET));

	/* Allow multicast address filtering */
	if (DepOptions & (XAE_MULTICAST_OPTION | XAE_EXT_MULTICAST_OPTION)) {
		Reg = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_RAF_OFFSET);
		Reg &= ~XAE_RAF_MCSTREJ_MASK;
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
							XAE_RAF_OFFSET, Reg);
	}
	xdbg_printf(XDBG_DEBUG_GENERAL,
	"setOptions: rcw1 is now (raf2): 0x%0x\n",
	XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_RCW1_OFFSET));

	xdbg_printf(XDBG_DEBUG_GENERAL,
	"setOptions: rcw1 is now (raf2):0x%0x\n",
	XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_RCW1_OFFSET));

	/*
	 * The remaining options not handled here are managed elsewhere in the
	 * driver. No register modifications are needed at this time.
	 * Reflecting the option in InstancePtr->Options is good enough for
	 * now.
	 */
	/* Enable extended multicast option */
	if (DepOptions & XAE_EXT_MULTICAST_OPTION) {
		XAxiEthernet_WriteReg((InstancePtr)->Config.BaseAddress,
			XAE_RAF_OFFSET,
			XAxiEthernet_ReadReg((InstancePtr)->Config.BaseAddress,
			XAE_RAF_OFFSET) | XAE_RAF_EMULTIFLTRENBL_MASK);
	}

	/*
	 * New/extended [TX|RX] VLAN translation option does not have specific
	 * bits to on/off.
	 */

	/* Enable extended transmit VLAN tag option */
	if (DepOptions & XAE_EXT_TXVLAN_TAG_OPTION) {
		Reg = XAxiEthernet_ReadReg((InstancePtr)->Config.BaseAddress,
							XAE_RAF_OFFSET);
		Reg = (Reg & ~XAE_RAF_TXVTAGMODE_MASK) |
			  (XAE_DEFAULT_TXVTAG_MODE <<
						XAE_RAF_TXVTAGMODE_SHIFT);
		XAxiEthernet_WriteReg((InstancePtr)->Config.BaseAddress,
							XAE_RAF_OFFSET, Reg);
	}

	/* Enable extended receive VLAN tag option */
	if (DepOptions & XAE_EXT_RXVLAN_TAG_OPTION) {
		Reg = XAxiEthernet_ReadReg((InstancePtr)->Config.BaseAddress,
							XAE_RAF_OFFSET);
		Reg = (Reg & ~XAE_RAF_RXVTAGMODE_MASK) |
			  (XAE_DEFAULT_RXVTAG_MODE <<
						XAE_RAF_RXVTAGMODE_SHIFT);
		XAxiEthernet_WriteReg((InstancePtr)->Config.BaseAddress,
							XAE_RAF_OFFSET, Reg);
	}

	/* Enable extended transmit VLAN strip option */
	if (Options & XAE_EXT_TXVLAN_STRP_OPTION) {
		Reg = XAxiEthernet_ReadReg((InstancePtr)->Config.BaseAddress,
							XAE_RAF_OFFSET);
		Reg = (Reg & ~XAE_RAF_TXVSTRPMODE_MASK) |
			  (XAE_DEFAULT_TXVSTRP_MODE <<
						XAE_RAF_TXVSTRPMODE_SHIFT);
		XAxiEthernet_WriteReg((InstancePtr)->Config.BaseAddress,
							XAE_RAF_OFFSET, Reg);
	}

	/* Enable extended receive VLAN strip option */
	if (Options & XAE_EXT_RXVLAN_STRP_OPTION) {
		Reg = XAxiEthernet_ReadReg((InstancePtr)->Config.BaseAddress,
							XAE_RAF_OFFSET);
		Reg = (Reg & ~XAE_RAF_RXVSTRPMODE_MASK) |
			  (XAE_DEFAULT_RXVSTRP_MODE <<
						XAE_RAF_RXVSTRPMODE_SHIFT);
		XAxiEthernet_WriteReg((InstancePtr)->Config.BaseAddress,
							XAE_RAF_OFFSET, Reg);
	}



	/*
	 * The remaining options not handled here are managed elsewhere in the
	 * driver. No register modifications are needed at this time.
	 * Reflecting the option in InstancePtr->Options is good enough for
	 * now.
	 */
	xdbg_printf(XDBG_DEBUG_GENERAL, "setOptions: returning SUCCESS\n");
	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
* XAxiEthernet_ClearOptions clears the options, <i>Options</i> for the
* Axi Ethernet, specified by <i>InstancePtr</i>. Axi Ethernet should be stopped
* with XAxiEthernet_Stop() before changing options.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @param	Options is a bitmask of OR'd XAE_*_OPTION values for options to
*		clear. Options not specified are not affected.
*
* @return
*		- XST_SUCCESS on successful completion.
*		- XST_DEVICE_IS_STARTED if the device has not been stopped.
*
* @note
* See xaxiethernet.h for a description of the available options.
*
******************************************************************************/
int XAxiEthernet_ClearOptions(XAxiEthernet *InstancePtr, u32 Options)
{
	u32 Reg;	/* Generic */
	u32 RegRcw1;	/* Reflects original contents of RCW1 */
	u32 RegTc;	/* Reflects original contents of TC  */
	u32 RegNewRcw1;	/* Reflects new contents of RCW1 */
	u32 RegNewTc;	/* Reflects new contents of TC  */
	u32 DepOptions;	/* Required dependent options for new features */

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);


	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_ClearOptions: 0x%08x\n",
								Options);
	/* Be sure device has been stopped */
	if (InstancePtr->IsStarted == XIL_COMPONENT_IS_STARTED) {
		return (XST_DEVICE_IS_STARTED);
	}

	/*
	 * Set options word to its new value.
	 * The step is required before calling _UpdateDepOptions() since
	 * we are operating on updated options.
	 */
	InstancePtr->Options &= ~Options;

	/*
	 * There are options required to be on/off per hardware requirement.
	 * Invoke _UpdateDepOptions to check hardware availability and update
	 * options accordingly.
	 */
	DepOptions = ~(XAxiEthernet_UpdateDepOptions(InstancePtr));

	/*
	 * New/extended function bit should be off if none of new/extended
	 * features is on after hardware is validated and gone through
	 * _UpdateDepOptions().
	 */
	if (!(~(DepOptions) &  (XAE_EXT_MULTICAST_OPTION   |
				XAE_EXT_TXVLAN_TRAN_OPTION |
				XAE_EXT_RXVLAN_TRAN_OPTION |
				XAE_EXT_TXVLAN_TAG_OPTION  |
				XAE_EXT_RXVLAN_TAG_OPTION  |
				XAE_EXT_TXVLAN_STRP_OPTION |
				XAE_EXT_RXVLAN_STRP_OPTION))) {
		XAxiEthernet_WriteReg((InstancePtr)->Config.BaseAddress,
			XAE_RAF_OFFSET,
			XAxiEthernet_ReadReg((InstancePtr)->Config.BaseAddress,
			XAE_RAF_OFFSET) & ~XAE_RAF_NEWFNCENBL_MASK);
	}

	/*
	 * Many of these options will change the RCW1 or TC registers.
	 * Group these options here and change them all at once. What we are
	 * trying to accomplish is to reduce the amount of IO to the device
	 */

	/* Get the current register contents */
	RegRcw1 = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_RCW1_OFFSET);
	RegTc = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_TC_OFFSET);
	RegNewRcw1 = RegRcw1;
	RegNewTc = RegTc;

	/* Turn off jumbo packet support for both Rx and Tx */
	if (DepOptions & XAE_JUMBO_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_ClearOptions: disabling jumbo\n");
		RegNewTc &= ~XAE_TC_JUM_MASK;
		RegNewRcw1 &= ~XAE_RCW1_JUM_MASK;
	}

	/* Turn off VLAN packet support for both Rx and Tx */
	if (DepOptions & XAE_VLAN_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_ClearOptions: disabling vlan\n");
		RegNewTc &= ~XAE_TC_VLAN_MASK;
		RegNewRcw1 &= ~XAE_RCW1_VLAN_MASK;
	}

	/* Turn off FCS stripping on receive packets */
	if (DepOptions & XAE_FCS_STRIP_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_ClearOptions: disabling fcs strip\n");
		RegNewRcw1 |= XAE_RCW1_FCS_MASK;
	}

	/* Turn off FCS insertion on transmit packets */
	if (DepOptions & XAE_FCS_INSERT_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_ClearOptions: disabling fcs insert\n");
		RegNewTc |= XAE_TC_FCS_MASK;
	}

	/* Turn off length/type field checking on receive packets */
	if (DepOptions & XAE_LENTYPE_ERR_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_ClearOptions: disabling lentype err\n");
		RegNewRcw1 |= XAE_RCW1_LT_DIS_MASK;
	}

	/* Disable transmitter */
	if (DepOptions & XAE_TRANSMITTER_ENABLE_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_ClearOptions: disabling transmitter\n");
		RegNewTc &= ~XAE_TC_TX_MASK;
	}

	/* Disable receiver */
	if (DepOptions & XAE_RECEIVER_ENABLE_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_ClearOptions: disabling receiver\n");
		RegNewRcw1 &= ~XAE_RCW1_RX_MASK;
	}

	/* Change the TC and RCW1 registers if they need to be
	 * modified
	 */
	if (RegTc != RegNewTc) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_ClearOptions: setting TC: 0x%0x\n", RegNewTc);
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
						XAE_TC_OFFSET, RegNewTc);
	}

	if (RegRcw1 != RegNewRcw1) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_ClearOptions: setting RCW1: 0x%0x\n",RegNewRcw1);
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
						XAE_RCW1_OFFSET, RegNewRcw1);
	}

	/*
	 * Rest of options twiddle bits of other registers. Handle them one at
	 * a time
	 */

	/* Turn off flow control */
	if (DepOptions & XAE_FLOW_CONTROL_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_ClearOptions: disabling flow control\n");
		Reg = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_FCC_OFFSET);
		Reg &= ~XAE_FCC_FCRX_MASK;
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
							XAE_FCC_OFFSET, Reg);
	}

	/* Turn off promiscuous frame filtering */
	if (DepOptions & XAE_PROMISC_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_ClearOptions: disabling promiscuous mode\n");
		Reg = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_FMI_OFFSET);
		Reg &= ~XAE_FMI_PM_MASK;
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
							XAE_FMI_OFFSET, Reg);
	}

	/* Disable broadcast address filtering */
	if (DepOptions & XAE_BROADCAST_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_ClearOptions: disabling broadcast mode\n");
		Reg = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_RAF_OFFSET);
		Reg |= XAE_RAF_BCSTREJ_MASK;
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
							XAE_RAF_OFFSET, Reg);
	}

	/* Disable multicast address filtering */
	if ((DepOptions & XAE_MULTICAST_OPTION) &&
		(DepOptions & XAE_EXT_MULTICAST_OPTION)) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_ClearOptions:disabling multicast mode\n");
		Reg = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_RAF_OFFSET);
		Reg |= XAE_RAF_MCSTREJ_MASK;
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
							XAE_RAF_OFFSET, Reg);
	}

	/* Disable extended multicast option */
	if (DepOptions & XAE_EXT_MULTICAST_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_ClearOptions:disabling extended multicast mode\n");
		XAxiEthernet_WriteReg((InstancePtr)->Config.BaseAddress,
			XAE_RAF_OFFSET,
			XAxiEthernet_ReadReg((InstancePtr)->Config.BaseAddress,
			XAE_RAF_OFFSET) & ~XAE_RAF_EMULTIFLTRENBL_MASK);
	}

	/* Disable extended transmit VLAN tag option */
	if (DepOptions & XAE_EXT_TXVLAN_TAG_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_ClearOptions:disabling extended TX VLAN tag mode\n");
		XAxiEthernet_WriteReg((InstancePtr)->Config.BaseAddress,
			XAE_RAF_OFFSET,
			XAxiEthernet_ReadReg((InstancePtr)->Config.BaseAddress,
			XAE_RAF_OFFSET) & ~XAE_RAF_TXVTAGMODE_MASK);
	}

	/* Disable extended receive VLAN tag option */
	if (DepOptions & XAE_EXT_RXVLAN_TAG_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_ClearOptions:disabling extended RX VLAN tag mode\n");
		XAxiEthernet_WriteReg((InstancePtr)->Config.BaseAddress,
			XAE_RAF_OFFSET,
			XAxiEthernet_ReadReg((InstancePtr)->Config.BaseAddress,
			XAE_RAF_OFFSET) & ~XAE_RAF_RXVTAGMODE_MASK);
	}

	/* Disable extended transmit VLAN strip option */
	if (DepOptions & XAE_EXT_TXVLAN_STRP_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_ClearOptions:disabling extended TX VLAN strip mode\n");
		XAxiEthernet_WriteReg((InstancePtr)->Config.BaseAddress,
			XAE_RAF_OFFSET,
			XAxiEthernet_ReadReg((InstancePtr)->Config.BaseAddress,
			XAE_RAF_OFFSET) & ~XAE_RAF_TXVSTRPMODE_MASK);
	}

	/* Disable extended receive VLAN strip option */
	if (DepOptions & XAE_EXT_RXVLAN_STRP_OPTION) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_ClearOptions:disabling extended RX VLAN strip mode\n");
		XAxiEthernet_WriteReg((InstancePtr)->Config.BaseAddress,
			XAE_RAF_OFFSET,
			XAxiEthernet_ReadReg((InstancePtr)->Config.BaseAddress,
			XAE_RAF_OFFSET) & ~XAE_RAF_RXVSTRPMODE_MASK);
	}

	/*
	 * The remaining options not handled here are managed elsewhere in the
	 * driver. No register modifications are needed at this time.
	 * Reflecting the option in InstancePtr->Options is good enough for
	 * now.
	 */
	xdbg_printf(XDBG_DEBUG_GENERAL, "ClearOptions: returning SUCCESS\n");
	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
* XAxiEthernet_GetOptions returns the current option settings.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @return	Returns a bitmask of XAE_*_OPTION constants,
*		each bit specifying an option that is currently active.
*
* @note
* See xaxiethernet.h for a description of the available options.
*
******************************************************************************/
u32 XAxiEthernet_GetOptions(XAxiEthernet *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Options);
}

/*****************************************************************************/
/**
 * XAxiEthernet_GetOperatingSpeed gets the current operating link speed. This
 * may be the value set by XAxiEthernet_SetOperatingSpeed() or a hardware
 * default.
 *
 * @param	InstancePtr is a pointer to the Axi Ethernet instance to be
 *		worked on.
 *
 * @return	Returns the link speed in units of megabits per second (10 /
 *		100 / 1000).
 *		Can return a value of 0, in case it does not get a valid
 *		speed from EMMC.
 *
 * @note	None.
 *
 ******************************************************************************/
u16 XAxiEthernet_GetOperatingSpeed(XAxiEthernet *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_GetOperatingSpeed\n");
	switch (XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
			XAE_EMMC_OFFSET) & XAE_EMMC_LINKSPEED_MASK) {

	case XAE_EMMC_LINKSPD_1000:
		xdbg_printf(XDBG_DEBUG_GENERAL,
			"XAxiEthernet_GetOperatingSpeed: returning 1000\n");
		return XAE_SPEED_1000_MBPS;

	case XAE_EMMC_LINKSPD_100:
		xdbg_printf(XDBG_DEBUG_GENERAL,
			"XAxiEthernet_GetOperatingSpeed: returning 100\n");
		return XAE_SPEED_100_MBPS;

	case XAE_EMMC_LINKSPD_10:
		xdbg_printf(XDBG_DEBUG_GENERAL,
			"XAxiEthernet_GetOperatingSpeed: returning 10\n");
		return (XAE_SPEED_10_MBPS);

	default:
		return (0);
	}
}


/*****************************************************************************/
/**
 * XAxiEthernet_SetOperatingSpeed sets the current operating link speed. For
 * any traffic to be passed, this speed must match the current
 * MII/GMII/SGMII/RGMII link speed.
 *
 * @param	InstancePtr references the Axi Ethernet on which to
 *		operate.
 * @param	Speed is the speed to set in units of Mbps.
 *		Valid values are 10, 100, or 1000.
 *
 * @return	- XST_SUCCESS on successful setting of speed.
 *		- XST_FAILURE, if the speed cannot be set for the present
 *		  harwdare configuration.
 *
 * @note	None.
 *
 *
 ******************************************************************************/
int XAxiEthernet_SetOperatingSpeed(XAxiEthernet *InstancePtr, u16 Speed)
{
	u32 EmmcReg;
	u8  TemacType;
	u8  PhyType;
	u8  SetSpeed = TRUE;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((Speed == XAE_SPEED_10_MBPS) ||
			(Speed == XAE_SPEED_100_MBPS) ||
			(Speed == XAE_SPEED_1000_MBPS) ||
			(Speed == XAE_SPEED_2500_MBPS));


	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_SetOperatingSpeed\n");
	xdbg_printf(XDBG_DEBUG_GENERAL,
	"XAxiEthernet_SetOperatingSpeed: setting speed to:%d (0x%0x)\n",
								Speed, Speed);

	TemacType = XAxiEthernet_GetTemacType(InstancePtr);
	PhyType = XAxiEthernet_GetPhysicalInterface(InstancePtr);
	(void) (TemacType);

	/*
	 * The following code checks for all allowable speed conditions before
	 * writing to the register. Please refer to the hardware specs for
	 * more information on it.
	 * For PHY type 1000Base-x, 10 and 100 Mbps are not supported.
	 * For soft/hard Axi Ethernet core, 1000 Mbps is supported in all PHY
	 * types except MII.
	 */
	if((Speed == XAE_SPEED_10_MBPS) || (Speed == XAE_SPEED_100_MBPS)) {
		if(PhyType == XAE_PHY_TYPE_1000BASE_X) {
			SetSpeed = FALSE;
		}
	}
	else {
		if((Speed == XAE_SPEED_1000_MBPS) &&
					(PhyType == XAE_PHY_TYPE_MII)) {
			SetSpeed = FALSE;
		}
	}

	if(SetSpeed == TRUE) {
		/*
		 * Get the current contents of the EMAC config register and
		 * zero out speed bits
		 */
		EmmcReg = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
				XAE_EMMC_OFFSET) & ~XAE_EMMC_LINKSPEED_MASK;

		xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_SetOperatingSpeed: current speed: 0x%0x\n",
								EmmcReg);
		switch (Speed) {
		case XAE_SPEED_10_MBPS:
			break;

		case XAE_SPEED_100_MBPS:
			EmmcReg |= XAE_EMMC_LINKSPD_100;
			break;

		case XAE_SPEED_1000_MBPS:
			EmmcReg |= XAE_EMMC_LINKSPD_1000;
			break;

		case XAE_SPEED_2500_MBPS:
			EmmcReg |= XAE_EMMC_LINKSPD_1000;
			break;

		default:
			return (XST_FAILURE);
		}

		xdbg_printf(XDBG_DEBUG_GENERAL,
			"XAxiEthernet_SetOperatingSpeed: new speed: 0x%0x\n",
								EmmcReg);
		/* Set register and return */
		XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
				XAE_EMMC_OFFSET, EmmcReg);
		xdbg_printf(XDBG_DEBUG_GENERAL,
				"XAxiEthernet_SetOperatingSpeed: done\n");
		return (XST_SUCCESS);
	}
	else {
		xdbg_printf(XDBG_DEBUG_ERROR,
		"Speed not compatible with the Axi Ethernet Phy type\n");
		return (XST_FAILURE);
	}

}

/*****************************************************************************/
/**
* XAxiEthernet_SetBadFrmRcvOption is used to enable the bad frame receive
* option. If enabled, this option ensures that bad receive frames are allowed
* and passed to the AXI4-Stream interface as if they are good frames.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @return	None.
*
* @note		None
*
******************************************************************************/
void XAxiEthernet_SetBadFrmRcvOption(XAxiEthernet *InstancePtr)
{
	u32 Reg;

	Reg = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_RAF_OFFSET);
	Reg |= XAE_RAF_RXBADFRMEN_MASK;
	XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
							XAE_RAF_OFFSET, Reg);
}

/*****************************************************************************/
/**
* XAxiEthernet_ClearBadFrmRcvOption is used to disable the bad frame receive
* option.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XAxiEthernet_ClearBadFrmRcvOption(XAxiEthernet *InstancePtr)
{
	u32 Reg;

	Reg = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_RAF_OFFSET);
	Reg &= ~XAE_RAF_RXBADFRMEN_MASK;
	XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
							XAE_RAF_OFFSET, Reg);
}

/*****************************************************************************/
/**
* XAxiEthernet_DisableControlFrameLenCheck is used to disable the length check
* for control frames (pause frames). This means once the API is called, control
* frames larger than the minimum frame length are accepted.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XAxiEthernet_DisableControlFrameLenCheck(XAxiEthernet *InstancePtr)
{
	u32 RegRcw1;

	/* Get the current register contents */
	RegRcw1 = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_RCW1_OFFSET);
	RegRcw1 |= XAE_RCW1_CL_DIS_MASK;
	XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
						XAE_RCW1_OFFSET, RegRcw1);
}

/*****************************************************************************/
/**
* XAxiEthernet_EnableControlFrameLenCheck is used to enable the length check
* for control frames (pause frames). After calling the API, all control frames
* received will be checked for proper length (less than minimum frame length).
* By default, upon normal start up, control frame length check is enabled.
* Hence this API needs to be called only if previously the control frame length
* check has been disabled by calling the API
* XAxiEthernet_DisableControlFrameLenCheck.
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XAxiEthernet_EnableControlFrameLenCheck(XAxiEthernet *InstancePtr)
{
	u32 RegRcw1;

	/* Get the current register contents */
	RegRcw1 = XAxiEthernet_ReadReg(InstancePtr->Config.BaseAddress,
							XAE_RCW1_OFFSET);
	RegRcw1 &= ~XAE_RCW1_CL_DIS_MASK;
	XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
						XAE_RCW1_OFFSET, RegRcw1);
}

/*****************************************************************************/
/**
* XAxiEthernet_PhySetMdioDivisor sets the MDIO clock divisor in the
* Axi Ethernet,specified by <i>InstancePtr</i> to the value, <i>Divisor</i>.
* This function must be called once after each reset prior to accessing
* MII PHY registers.
*
* From the Virtex-6(TM) and Spartan-6 (TM) Embedded Tri-Mode Ethernet
* MAC User's Guide, the following equation governs the MDIO clock to the PHY:
*
* <pre>
* 			f[HOSTCLK]
*	f[MDC] = -----------------------
*			(1 + Divisor) * 2
* </pre>
*
* where f[HOSTCLK] is the bus clock frequency in MHz, and f[MDC] is the
* MDIO clock frequency in MHz to the PHY. Typically, f[MDC] should not
* exceed 2.5 MHz. Some PHYs can tolerate faster speeds which means faster
* access.
*
* @param	InstancePtr references the Axi Ethernet instance on which to
*		operate.
* @param	Divisor is the divisor value to set within the range of 0 to
*		XAE_MDIO_MC_CLK_DVD_MAX.
*
* @note	None.
*
******************************************************************************/
void XAxiEthernet_PhySetMdioDivisor(XAxiEthernet *InstancePtr, u8 Divisor)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY)
	Xil_AssertVoid(Divisor <= XAE_MDIO_MC_CLOCK_DIVIDE_MAX);


	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_PhySetMdioDivisor\n");

	XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
				XAE_MDIO_MC_OFFSET,
				(u32) Divisor | XAE_MDIO_MC_MDIOEN_MASK);
}

/*****************************************************************************/
/*
* XAxiEthernet_PhyRead reads the specified PHY register, <i>RegiseterNum</i>
* on the PHY specified by <i>PhyAddress</i> into <i>PhyDataPtr</i>.
* This Ethernet driver does not require the device to be stopped before reading
* from the PHY. It is the responsibility of the calling code to stop the
* device if it is deemed necessary.
*
* Note that the Axi Ethernet hardware provides the ability to talk to a PHY
* that adheres to the Media Independent Interface (MII) as defined in the
* IEEE 802.3 standard.
*
* <b>It is important that calling code set up the MDIO clock with
* XAxiEthernet_PhySetMdioDivisor() prior to accessing the PHY with this
* function.
* </b>
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @param	PhyAddress is the address of the PHY to be written (multiple
*		PHYs supported).
* @param	RegisterNum is the register number, 0-31, of the specific PHY
*		register to write.
* @param	PhyDataPtr is a reference to the location where the 16-bit
*		result value is stored.
*
* @return	None.
*
*
* @note
*
* There is the possibility that this function will not return if the hardware
* is broken (i.e., it never sets the status bit indicating that the write is
* done). If this is of concern, the calling code should provide a mechanism
* suitable for recovery.
*
******************************************************************************/
void XAxiEthernet_PhyRead(XAxiEthernet *InstancePtr, u32 PhyAddress,
			   u32 RegisterNum, u16 *PhyDataPtr)
{
	u32 MdioCtrlReg = 0;
	u32 value=0U;
	volatile s32 TimeoutLoops;

	/*
	 * Verify that each of the inputs are valid.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(PhyAddress <= XAE_PHY_ADDR_LIMIT);
	Xil_AssertVoid(RegisterNum <= XAE_PHY_REG_NUM_LIMIT);

	xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_PhyRead: BaseAddress: 0x%08x\n",
		InstancePtr->Config.BaseAddress);

	/*
	 * Wait till MDIO interface is ready to accept a new transaction.
	 */
	TimeoutLoops = Xil_poll_timeout(Xil_In32,InstancePtr->Config.BaseAddress+
	   XAE_MDIO_MCR_OFFSET, value,(value&XAE_MDIO_MCR_READY_MASK)!=0,
			   XAE_RST_DEFAULT_TIMEOUT_VAL);
	if(-1 == TimeoutLoops) {
		Xil_AssertVoidAlways();
	}

	MdioCtrlReg =   ((PhyAddress << XAE_MDIO_MCR_PHYAD_SHIFT) &
			XAE_MDIO_MCR_PHYAD_MASK) |
			((RegisterNum << XAE_MDIO_MCR_REGAD_SHIFT)
			& XAE_MDIO_MCR_REGAD_MASK) |
			XAE_MDIO_MCR_INITIATE_MASK |
			XAE_MDIO_MCR_OP_READ_MASK;

	XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
			XAE_MDIO_MCR_OFFSET, MdioCtrlReg);


	/*
	 * Wait till MDIO transaction is completed.
	 */
	TimeoutLoops = Xil_poll_timeout(Xil_In32,InstancePtr->Config.BaseAddress+
	   XAE_MDIO_MCR_OFFSET, value,(value&XAE_MDIO_MCR_READY_MASK)!=0,
		   XAE_RST_DEFAULT_TIMEOUT_VAL);
	if(-1 == TimeoutLoops) {
		Xil_AssertVoidAlways();
	}

	/* Read data */
	*PhyDataPtr = (u16) XAxiEthernet_ReadReg
			(InstancePtr->Config.BaseAddress,XAE_MDIO_MRD_OFFSET);
	xdbg_printf(XDBG_DEBUG_GENERAL,
		"XAxiEthernet_PhyRead: Value retrieved: 0x%0x\n", *PhyDataPtr);

}


/*****************************************************************************/
/*
* XAxiEthernet_PhyWrite writes <i>PhyData</i> to the specified PHY register,
* <i>RegiseterNum</i> on the PHY specified by <i>PhyAddress</i>. This Ethernet
* driver does not require the device to be stopped before writing to the PHY.
* It is the responsibility of the calling code to stop the device if it is
* deemed necessary.
*
* Note that the Axi Ethernet hardware provides the ability to talk to a PHY
* that adheres to the Media Independent Interface (MII) as defined in the
* IEEE 802.3 standard.
*
* <b>It is important that calling code set up the MDIO clock with
* XAxiEthernet_PhySetMdioDivisor() prior to accessing the PHY with this
* function.</b>
*
* @param	InstancePtr is a pointer to the Axi Ethernet instance to be
*		worked on.
* @param	PhyAddress is the address of the PHY to be written (multiple
*		PHYs supported).
* @param	RegisterNum is the register number, 0-31, of the specific PHY
*		register to write.
* @param	PhyData is the 16-bit value that will be written to the
*		register.
*
* @return	None.
*
* @note
*
* There is the possibility that this function will not return if the hardware
* is broken (i.e., it never sets the status bit indicating that the write is
* done). If this is of concern, the calling code should provide a mechanism
* suitable for recovery.
*
******************************************************************************/
void XAxiEthernet_PhyWrite(XAxiEthernet *InstancePtr, u32 PhyAddress,
			u32 RegisterNum, u16 PhyData)
{
	u32 MdioCtrlReg = 0;
	u32 value=0U;
	volatile s32 TimeoutLoops;

	/*
	 * Verify that each of the inputs are valid.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(PhyAddress <= XAE_PHY_ADDR_LIMIT);
	Xil_AssertVoid(RegisterNum <= XAE_PHY_REG_NUM_LIMIT);


	xdbg_printf(XDBG_DEBUG_GENERAL, "XAxiEthernet_PhyWrite\n");

	/*
	 * Wait till the MDIO interface is ready to accept a new transaction.
	 */
	TimeoutLoops = Xil_poll_timeout(Xil_In32, InstancePtr->Config.BaseAddress +
	   XAE_MDIO_MCR_OFFSET, value,(value&XAE_MDIO_MCR_READY_MASK)!=0,
	                 XAE_RST_DEFAULT_TIMEOUT_VAL);
	if(-1 == TimeoutLoops) {
		Xil_AssertVoidAlways();
	}

	MdioCtrlReg =   ((PhyAddress << XAE_MDIO_MCR_PHYAD_SHIFT) &
			XAE_MDIO_MCR_PHYAD_MASK) |
			((RegisterNum << XAE_MDIO_MCR_REGAD_SHIFT) &
			XAE_MDIO_MCR_REGAD_MASK) |
			XAE_MDIO_MCR_INITIATE_MASK |
			XAE_MDIO_MCR_OP_WRITE_MASK;

	XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
			XAE_MDIO_MWD_OFFSET, PhyData);

	XAxiEthernet_WriteReg(InstancePtr->Config.BaseAddress,
			XAE_MDIO_MCR_OFFSET, MdioCtrlReg);

	/*
	 * Wait till the MDIO interface is ready to accept a new transaction.
	 */
	TimeoutLoops = Xil_poll_timeout(Xil_In32,InstancePtr->Config.BaseAddress+
	   XAE_MDIO_MCR_OFFSET, value,(value&XAE_MDIO_MCR_READY_MASK)!=0,
		   XAE_RST_DEFAULT_TIMEOUT_VAL);
	if(-1 == TimeoutLoops) {
		Xil_AssertVoidAlways();
	}

}
/** @} */
